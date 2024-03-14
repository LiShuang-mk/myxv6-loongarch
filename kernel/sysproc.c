#include "types.h"
#include "loongarch.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sem.h"
#include "proc.h"

uint64
sys_exit( void )
{
	int n;
	if ( argint( 0, &n ) < 0 )
		return -1;
	exit( n );
	return 0;  // not reached
}

uint64
sys_getpid( void )
{
	return myproc()->pid;
}

uint64
sys_fork( void )
{
	return fork();
}

uint64
sys_wait( void )
{
	uint64 p;
	if ( argaddr( 0, &p ) < 0 )
		return -1;
	return wait( p );
}

uint64
sys_sbrk( void )
{
	int addr;
	int n;

	if ( argint( 0, &n ) < 0 )
		return -1;
	addr = myproc()->sz;
	if ( growproc( n ) < 0 )
		return -1;
	return addr;
}

uint64
sys_sleep( void )
{
	int n;
	uint ticks0;

	if ( argint( 0, &n ) < 0 )
		return -1;
	acquire( &tickslock );
	ticks0 = ticks;
	while ( ticks - ticks0 < n )
	{
		if ( myproc()->killed )
		{
			release( &tickslock );
			return -1;
		}
		sleep( &ticks, &tickslock );
	}
	release( &tickslock );
	return 0;
}

uint64
sys_kill( void )
{
	int pid;

	if ( argint( 0, &pid ) < 0 )
		return -1;
	return kill( pid );
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime( void )
{
	uint xticks;

	acquire( &tickslock );
	xticks = ticks;
	release( &tickslock );
	return xticks;
}

uint64
sys_getcpuid( void )
{
	return cpuid();
}

uint64 sh_var = 0;

uint64
sys_read_sh_var( void )
{
	return sh_var;
}

uint64
sys_write_sh_var( void )
{
	int old_var = sh_var;
	int new_var;
	if ( argint( 0, &new_var ) < 0 )
		return -1;
	sh_var = new_var;
	return old_var;
}

uint64
sys_chpri( void )
{
	int pid, pr;
	if ( argint( 0, &pid ) < 0 )
		return -1;
	if ( argint( 1, &pr ) < 0 )
		return -1;
	if ( pr > MAX_PRIORITY )
		return -1;
	return chpri( pid, pr );
}

uint64
sys_sem_create( void )
{
	int n_sem, i;
	if ( argint( 0, &n_sem ) < 0 )
		return -1;
	for ( i = 0; i < SEM_MAX_NUM; ++i )
	{
		acquire( &sems[ i ].lock );
		if ( sems[ i ].allocated == 0 )
		{
			sems[ i ].allocated = 1;
			sems[ i ].resource_count = n_sem;
			// printf( "create sem %d\n", i );
			release( &sems[ i ].lock );
			return i;
		}
		release( &sems[ i ].lock );
	}
	return -1;
}

uint64
sys_sem_free()
{
	int id;
	if ( argint( 0, &id ) < 0 )
		return -1;
	acquire( &sems[ id ].lock );
	if ( sems[ id ].allocated == 1
		&& sems[ id ].resource_count > 0 )
	{
		sems[ id ].allocated = 0;
		// printf( "free sem %d\n" );
	}
	release( &sems[ id ].lock );
	return 0;
}

uint64
sys_sem_p()
{
	int id;
	if ( argint( 0, &id ) < 0 )
		return -1;
	acquire( &sems[ id ].lock );
	sems[ id ].resource_count--;
	if ( sems[ id ].resource_count < 0 )
		sleep( &sems[ id ], &sems[ id ].lock );
	release( &sems[ id ].lock );
	return 0;
}

uint64
sys_sem_v()
{
	int id;
	if ( argint( 0, &id ) < 0 )
		return -1;
	acquire( &sems[ id ].lock );
	sems[ id ].resource_count++;
	if ( sems[ id ].resource_count < 1 )
		wakeup1p( &sems[ id ] );
	release( &sems[ id ].lock );
	return 0;
}

uint64
sys_shmgetat( void )
{
	int key, num;
	if ( argint( 0, &key ) < 0 || argint( 1, &num ) < 0 )
		return -1;
	return ( uint64 ) shmgetat( key, num );
}

uint64
sys_shmrefcount( void )
{
	int key;
	if ( argint( 0, &key ) < 0 )
		return -1;
	return ( uint64 ) shmrefcount( key );
}

uint64
sys_debug_v2p( void )
{
	uint64 va;
	if ( argaddr( 0, &va ) < 0 )
		return -1;
	struct proc *p = myproc();
	pagetable_t pagetable = p->pagetable;
	return walkaddr( pagetable, va );
}

uint64
sys_mqget( void )
{
	int key;
	if ( argint( 0, &key ) < 0 )
		return -1;
	return mqget( key );
}

uint64
sys_msgsnd( void )
{
	int mqid, type, sz;
	if ( argint( 0, &mqid ) < 0 )
		return -1;
	if ( argint( 1, &type ) < 0 )
		return -1;
	if ( argint( 2, &sz ) < 0 )
		return -1;
	char msg[ sz + 1 ];
	if ( argstr( 3, msg, sz ) < 0 )
		return -1;
	return msgsnd( mqid, type, sz, msg );
}

uint64
sys_msgrcv( void )
{
	int mqid, type, sz;
	uint64 addr;
	if ( argint( 0, &mqid ) < 0 )
		return -1;
	if ( argint( 1, &type ) < 0 )
		return -1;
	if ( argint( 2, &sz ) < 0 )
		return -1;
	if ( argaddr( 3, &addr ) < 0 )
		return -1;
	return msgrcv( mqid, type, sz, addr );
}
