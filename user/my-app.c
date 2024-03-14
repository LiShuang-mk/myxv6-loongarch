#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "kernel/msgqueue.h"
#include "user/user.h"

int
main( int argc, char *argv[] )
{
	printf( "Hello world from my own app!\n" );
	printf( "Current running CPU is: %d\n", getcpuid() );
	printf( "This process's PID is: %d\n", getpid() );

	printf( "\ntest msg size: %d %d %d %d %d\n"
		, sizeof( struct msg )
		, sizeof( struct msg * )
		, sizeof( long )
		, sizeof( char * )
		, sizeof( int )
	);

	int pid = fork();
	int i, n;
	for ( i = 0; i < 20000; ++i )
	{
		n = read_sh_var();
		write_sh_var( n + 1 );
	}
	printf( "sum=%d\n", read_sh_var() );
	if ( pid > 0 )
		wait( 0 );

	exit( 0 );
}