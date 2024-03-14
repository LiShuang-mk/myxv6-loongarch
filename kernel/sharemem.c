#include "types.h"
#include "param.h"
#include "loongarch.h"
#include "spinlock.h" 
#include "proc.h"
#include "defs.h"
#include "memlayout.h"

struct spinlock shmlock;
struct sharemem shmtab[ MAX_SHM_NUM ];

void
sharememinit()
{
	initlock( &shmlock, "shmlock" );
	for ( int i = 0; i < MAX_SHM_NUM; i++ )
	{
		shmtab[ i ].refcount = 0;
	}
	printf( "shm init finished.\n" );
}

int
shmkeyused( uint key, uint mask )
{
	if ( key < 0 || key >= MAX_SHM_NUM )
		return 0;
	return ( mask >> key ) & 0x1;
}

void
shmaddcount( uint mask )
{
	acquire( &shmlock );
	for ( int key = 0; key < MAX_SHM_NUM; key++ )
	{
		if ( shmkeyused( key, mask ) )
		{
			shmtab[ key ].refcount++;
		}
	}
	release( &shmlock );
}

uint64
deallocshm( pde_t *pgdir, uint64 oldshm, uint64 newshm )
{
	if ( newshm <= oldshm )
		return oldshm;
	if ( PGROUNDUP( newshm ) > PGROUNDUP( oldshm ) )
	{
		int npages = ( PGROUNDUP( newshm ) - PGROUNDUP( oldshm ) ) / PGSIZE;
		uvmunmap( pgdir, PGROUNDUP( oldshm ), npages, 0 );
	}
	return oldshm;
}

uint64
allocshm(
	pde_t *pgdir
	, uint64 oldshm
	, uint64 newshm
	, uint64 sz
	, void *phyadddr[ MAX_SHM_PGNUM ] )
{
	char *mem;
	uint64 a;

	if ( oldshm & 0xFFF
		|| newshm & 0xFFF
		|| oldshm > ( TRAPFRAME - 64 * 2 * PGSIZE )
		|| newshm < sz )
	{
		return 0;
	}
	a = newshm;
	for ( int i = 0; a < oldshm; a += PGSIZE, ++i )
	{
		mem = kalloc();
		if ( mem == 0 )
		{
			printf( "allocshm out of memory\n" );
			deallocshm( pgdir, newshm, oldshm );
			return 0;
		}
		memset( mem, 0, PGSIZE );
		mappages(
			pgdir
			, a
			, PGSIZE
			, ( uint64 ) mem
			, PTE_P | PTE_W | PTE_PLV | PTE_MAT | PTE_D );
		phyadddr[ i ] = ( void* ) mem;
		printf( "allocshm: %x\n", a );
	}
	return newshm;
}

uint64
mapshm( pde_t *pgdir, uint64 oldshm, uint64 newshm, uint sz, void **physaddr )
{
	uint64 a;
	if ( oldshm & 0xFFF
		|| newshm & 0xFFF
		|| oldshm > ( TRAPFRAME - 64 * 2 * PGSIZE )
		|| newshm < sz )
	{
		return 0;
	}
	a = newshm;
	for ( int i = 0; a < oldshm; a += PGSIZE, ++i )
	{
		mappages(
			pgdir
			, a
			, PGSIZE
			, ( uint64 ) physaddr[ i ]
			, PTE_P | PTE_W | PTE_PLV | PTE_MAT | PTE_D );
	}
	return newshm;
}

int
shmadd( uint key, uint pagenum, void *physaddr[ MAX_SHM_PGNUM ] )
{
	if ( key<0 || key >= MAX_SHM_NUM || pagenum >MAX_SHM_PGNUM )
	{
		return -1;
	}
	shmtab[ key ].refcount = 1;
	shmtab[ key ].pagenum = pagenum;
	for ( int i = 0; i < pagenum; ++i )
	{
		shmtab[ key ].physaddr[ i ] = physaddr[ i ];
	}
	return 0;
}

void*
shmgetat( uint key, uint num /*page num*/ )
{
	pde_t *pgdir;
	struct proc* p;
	void *phyaddr[ MAX_SHM_PGNUM ];
	uint64 shm = 0;
	if ( key < 0
		|| key >= MAX_SHM_NUM
		|| num < 0
		|| num > MAX_SHM_PGNUM )
	{
		return ( void* ) -1;
	}
	acquire( &shmlock );
	p = myproc();
	pgdir = p->pagetable;
	shm = p->shm;

	// Case 1: key has been mapped 
	if ( p->shmkeymask >> key & 0x1 )
	{
		release( &shmlock );
		return p->shmva[ key ];
	}

	// Case 2: shared memory has not been created 
	if ( shmtab[ key ].refcount == 0 )
	{
		shm = allocshm( pgdir, shm, shm - num * PGSIZE, p->sz, phyaddr );
		if ( shm == 0 )
		{
			release( &shmlock );
			return ( void* ) -1;
		}
		p->shmva[ key ] = ( void* ) shm;
		shmadd( key, num, phyaddr );
	}

	// Case 3: shared memory has been created but not been mapped 
	else
	{
		for ( int i = 0; i < num; ++i )
		{
			phyaddr[ i ] = shmtab[ key ].physaddr[ i ];
		}
		num = shmtab[ key ].pagenum;
		if ( ( shm = mapshm( pgdir, shm, shm - num * PGSIZE, p->sz, phyaddr ) ) == 0 )
		{
			release( &shmlock );
			return ( void * ) -1;
		}
		p->shmva[ key ] = ( void* ) shm;
		shmtab[ key ].refcount++;
	}


	p->shm = shm;
	p->shmkeymask |= 1 << key;
	release( &shmlock );
	return ( void* ) shm;
}

int
shmrm( int key )
{
	if ( key < 0 || key >= MAX_SHM_NUM )
	{
		return -1;
	}
	struct sharemem *shmem = &shmtab[ key ];
	for ( int i = 0; i < shmem->pagenum; ++i )
	{
		kfree( ( char* ) ( shmem->physaddr[ i ] ) );
	}
	shmem->refcount = 0;
	return 0;
}

int
shmrelease( pde_t *pgdir, uint64 shm, uint keymask )
{
	acquire( &shmlock );
	deallocshm( pgdir, shm, TRAPFRAME - 64 * 2 * PGSIZE );
	for ( int k = 0; k < MAX_SHM_NUM; ++k )
	{
		if ( shmkeyused( k, keymask ) )
		{
			shmtab[ k ].refcount--;
			if ( shmtab[ k ].refcount == 0 )
				shmrm( k );
		}
	}
	release( &shmlock );
	return 0;
}

int
shmrefcount( uint key )
{
	acquire( &shmlock );
	int count;
	count = ( key < 0 || key >= MAX_SHM_NUM ) ?
		-1 : shmtab[ key ].refcount;
	release( &shmlock );
	return count;
}
