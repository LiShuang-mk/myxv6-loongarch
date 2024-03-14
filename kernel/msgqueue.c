#include "types.h"
#include "param.h"
#include "loongarch.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "msgqueue.h"

struct spinlock mqlock;
struct mq mqs[ MQMAX ];

struct proc *wqueue[ NPROC ];
int wstart = 0;

struct proc *rqueue[ NPROC ];
int rstart = 0;

void
mqinit()
{
	printf( "mqinit.\n" );
	initlock( &mqlock, "mqlock" );
	for ( int i = 0; i < MQMAX; ++i )
	{
		mqs[ i ].status = 0;
	}
}

void
addmqcount( uint mask )
{
	acquire( &mqlock );
	for ( int key = 0; key < MQMAX; key++ )
	{
		if ( ( mask >> key ) & 0x01 )
		{
			mqs[ key ].refcount++;
		}
	}
	release( &mqlock );
}

int
findkey( uint key )
{
	int idx = -1;
	for ( int i = 0; i < MQMAX; ++i )
	{
		if ( mqs[ i ].status != 0 && mqs[ i ].key == key )
		{
			idx = i;
			break;
		}
	}
	return idx;
}

int
newmq( int key )
{
	struct proc *proc = myproc();
	int idx = -1;
	for ( int i = 0; i < MQMAX; ++i )
	{
		if ( mqs[ i ].status == 0 )
		{
			idx = i;
			break;
		}
	}
	if ( idx == -1 )
	{
		printf( "newmq failed: can not get idx.\n" );
		return -1;
	}
	mqs[ idx ].msgs = ( struct msg* ) kalloc();
	if ( mqs[ idx ].msgs == 0 )
	{
		printf( "newmq failed: can not alloc page.\n" );
		return -1;
	}
	mqs[ idx ].key = key;
	mqs[ idx ].status = 1;
	memset( mqs[ idx ].msgs, 0, PGSIZE );
	mqs[ idx ].msgs->next = 0;
	mqs[ idx ].msgs->datasize = 0;
	mqs[ idx ].maxbytes = PGSIZE;
	mqs[ idx ].curbytes = sizeof( struct msg );
	mqs[ idx ].refcount = 1;
	proc->mqmask |= 0x1u << idx;
	return idx;
}

int
mqget( uint key )
{
	struct proc *proc = myproc();
	acquire( &mqlock );

	int idx = findkey( key );
	if ( idx != -1 )
	{
		if ( !( proc->mqmask >> idx & 0x01 ) )
		{
			proc->mqmask |= 1 << idx;
			mqs[ idx ].refcount++;
		}
		release( &mqlock );
		return idx;
	}

	idx = newmq( key );
	release( &mqlock );
	return idx;
}

int
reloc( int mqid )
{
	struct msg *pages = mqs[ mqid ].msgs;
	struct msg *m = pages;
	struct msg *t;
	struct msg *pre = pages;
	while ( m != 0 )
	{
		t = m->next;
		memmove( pages, m, m->datasize + sizeof( struct msg ) );
		pages->next = ( struct msg* ) ( ( char * ) pages + pages->datasize + sizeof( struct msg ) );
		pages->dataaddr = ( ( char * ) pages + sizeof( struct msg ) );
		pre = pages;
		pages = pages->next;
		m = t;
	}
	pre->next = 0;
	return 0;
}

int
msgsnd( uint mqid, int type, int sz, char *msg )
{
	struct proc *proc = myproc();
	if ( mqid < 0 || MQMAX <= mqid || mqs[ mqid ].status == 0 )
	{
		return -1;
	}

	if ( mqs[ mqid ].msgs == 0 )
	{
		printf( "msgsnd failed: msgs == 0.\n" );
		return -1;
	}

	acquire( &mqlock );
	while ( 1 )
	{
		if ( mqs[ mqid ].curbytes + sz + sizeof( struct msg )
			<= mqs[ mqid ].maxbytes )
		{
			struct msg *m = mqs[ mqid ].msgs;
			while ( m->next != 0 )
			{
				m = m->next;
			}
			m->next = ( void* ) m + m->datasize + sizeof( struct msg );
			m = m->next;
			m->type = type;
			m->next = 0;
			m->dataaddr = ( char * ) m + sizeof( struct msg );
			m->datasize = sz;
			memmove( m->dataaddr, msg, sz );
			mqs[ mqid ].curbytes += ( sz + sizeof( struct msg ) );
			for ( int i = 0; i < rstart; ++i )
			{
				wakeup( rqueue[ i ] );
			}
			rstart = 0;

			release( &mqlock );
			return 0;
		}
		else
		{
			printf( "msgsnd: can not alloc: pthread: %d sleep.\n", proc->pid );
			wqueue[ wstart++ ] = proc;

			sleep( proc, &mqlock );
		}
	}

	return -1;
}

int
msgrcv( uint mqid, int type, int sz, uint64 addr )
{
	struct proc *proc = myproc();
	if ( mqid < 0 || MQMAX <= mqid || mqs[ mqid ].status == 0 )
	{
		return -1;
	}

	acquire( &mqlock );
	while ( 1 )
	{
		struct msg *m = mqs[ mqid ].msgs->next;
		struct msg *pre = mqs[ mqid ].msgs;
		while ( m != 0 )
		{
			if ( m->type == type )
			{
				copyoutstr( proc->pagetable, addr, m->dataaddr, sz );

				pre->next = m->next;
				mqs[ mqid ].curbytes -= ( m->datasize + sizeof( struct msg ) );
				reloc( mqid );

				for ( int i = 0; i < wstart; ++i )
				{
					wakeup( wqueue[ i ] );
				}
				wstart = 0;

				release( &mqlock );
				return 0;
			}
			pre = m;
			m = m->next;
		}
		printf( "msgrcv: can not read: pthread: %d sleep.\n", proc->pid );
		rqueue[ rstart++ ] = proc;
		sleep( proc, &mqlock );
	}
	return -1;
}

void
rmmq( int mqid )
{
	kfree( ( char * ) mqs[ mqid ].msgs );
	mqs[ mqid ].status = 0;
}

void
releasemq( uint key )
{
	int idx = findkey( key );
	if ( idx != -1 )
	{
		acquire( &mqlock );
		mqs[ idx ].refcount--;
		if ( mqs[ idx ].refcount == 0 )
			rmmq( idx );
		release( &mqlock );
	}
}

void
releasemq2( int mask )
{
	acquire( &mqlock );
	for ( int id = 0; id < MQMAX; ++id )
	{
		if ( mask >> id & 0x01 )
		{
			mqs[ id ].refcount--;
			if ( mqs[ id ].refcount == 0 )
			{
				rmmq( id );
			}
		}
	}
	release( &mqlock );
}
