#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main()
{
	int id = sem_create( 1 );
	int pid = fork();
	int i;
	for ( i = 0; i < 10000; ++i )
	{
		sem_p( id );
		write_sh_var( read_sh_var() + 1 );
		sem_v( id );
	}
	if ( pid > 0 )
	{
		wait( 0 );
		sem_free( id );
		printf( "parent get sum=%d\n", read_sh_var() );
	}
	else 
		printf( "child get sum=%d\n", read_sh_var() );
	exit( 0 );
}