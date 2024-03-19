#include "types.h"
#include "param.h"
#include "loongarch.h"
#include "defs.h"
#include "memlayout.h"

// entry.S needs one stack per CPU.
__attribute__( ( aligned( 16 ) ) ) char stack0[ 4096 * NCPU ];

volatile static int started = 0;

// entry.S jumps here on stack0.
void
main()
{
	if ( cpuid() == 0 )
	{
		consoleinit();
		printfinit();

		// uint32 apbh[ 64 ];
		// uint64 addr = ( ( 0xFE0UL << 28 ) | ( 0x0UL << 16 ) | ( 0x2UL << 11 ) | ( 0x0UL << 8 ) );
		// printf( "addr: \n%p\n", addr | DMWIN_MASK );
		// printf( "head: \n" );
		// volatile uint32 * p = ( volatile uint32 * ) ( addr | DMWIN_MASK );
		// for ( int i = 0; i < 16; i++, p++ )
		// 	apbh[ i ] = *p;
		// for ( int i = 0; i < 16; i++ )
		// 	printf( "%x\n", apbh[ i ] );

		// printf( "win addr:\n" );
		// for ( int i = 0; i < 8; ++i )
		// {
		// 	printf( "%x\t%x\t%x\n"
		// 		, *( ( volatile unsigned long long * ) ( ( 0x1fe02000 | DMWIN_MASK ) + i * 8 ) )
		// 		, *( ( volatile unsigned long long * ) ( ( 0x1fe02040 | DMWIN_MASK ) + i * 8 ) )
		// 		, *( ( volatile unsigned long long * ) ( ( 0x1fe02080 | DMWIN_MASK ) + i * 8 ) )
		// 	);
		// }

		kinit();         // physical page allocator
	//printf("kinit\n");
		vminit();        // create kernel page table
	//printf("vminit\n");
		procinit();      // process table
	//printf("procinit\n");
		trapinit();      // trap vectors
	//printf("trapinit\n");
		apic_init();     // set up LS7A1000 interrupt controller
	//printf("apicinit\n");
		extioi_init();   // extended I/O interrupt controller
	//printf("extioi_init\n");
		binit();         // buffer cache
	//printf("binit\n");
		iinit();         // inode table
	//printf("iinit\n");
		fileinit();      // file table
	//printf("fileinit\n");
		ramdiskinit();   // emulated hard disk
	//printf("ramdiskinit\n");
		sharememinit();
	//printf("sharemem init\n")
		seminit();		// semaphore init 
	//printf("seminit\n");
		mqinit();		// msg queue init 
	//printf("mqinit\n");
		userinit();      // first user process
	//printf("userinit\n");
		__sync_synchronize();
		started = 1;
	}
	else
	{
		while ( started == 0 )
			;
		__sync_synchronize();
		printf( "hart %d starting\n", cpuid() );
	}
	scheduler();
}

