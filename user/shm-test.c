#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h" 
#include "kernel/fs.h" 

int
main( void )
{
	char *shm;
	int pid = fork();
	if ( pid == 0 )
	{
		sleep( 3 );
		shm = ( char* ) shmgetat( 1, 3 );
		printf( "child pid: %d -- shm(%x->%x) is %s -- refcount of 1 is: %d\n"
			, getpid()
			, shm
			, debug_v2p( shm )
			, shm
			, shmrefcount( 1 )
		);
		strcpy( shm, "hello_world!" );
		printf( "child pid: %d -- write %s into the shm\n"
			, getpid()
			, shm
		);
	}
	else if ( pid > 0 )
	{
		shm = ( char * ) shmgetat( 1, 3 );
		printf( "parent pid: %d -- before wait() shm(%x->%x) is %s -- refcount of 1 is: %d\n"
			, getpid()
			, shm
			, debug_v2p( shm )
			, shm
			, shmrefcount( 1 )
		);
		strcpy( shm, "share_memory!" );
		printf( "parent pid: %d -- write %s into the shm\n"
			, getpid()
			, shm
		);
		wait( 0 );
		printf( "parent pid: %d -- after wait() shm(%x->%x) is %s -- refcount of 1 is: %d\n"
			, getpid()
			, shm
			, debug_v2p( shm )
			, shm
			, shmrefcount( 1 )
		);
	}
	exit( 0 );
}