#include "../kernel/param.h"
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"
#include "../kernel/fs.h"
#include "../kernel/fcntl.h"
#include "../kernel/syscall.h"
#include "../kernel/memlayout.h"

struct msg
{
	int type;
	char *dataaddr;
};
struct msg s1, s2, g;

void msg_test()
{
	int mqid = mqget( 123 );
	int pid = fork();
	if ( pid == 0 )
	{
		s1.type = 1;
		s1.dataaddr = "This is the first message!\n";
		msgsnd( mqid, s1.type, 28, s1.dataaddr );

		s1.type = 2;
		s1.dataaddr = "Hello, another message comes!\n";
		msgsnd( mqid, s1.type, 31, s1.dataaddr );

		s1.type = 3;
		s1.dataaddr = "This is the third message, and this message has great many characters!\n";
		msgsnd( mqid, s1.type, 72, s1.dataaddr );

		printf( "all messages have been sent.\n" );
	}
	else if ( pid > 0 )
	{
		// sleep( 10 );
		char buf[ 128 ];
		g.dataaddr = buf;

		g.type = 2;
		msgrcv( mqid, g.type, 31, ( void* ) g.dataaddr );
		printf( "receive the %dth message: %s\n", 2, g.dataaddr );

		g.type = 1;
		msgrcv( mqid, g.type, 28, ( void* ) g.dataaddr );
		printf( "receive the %dth message: %s\n", 1, g.dataaddr );

		g.type = 3;
		msgrcv( mqid, g.type, 72, ( void* ) g.dataaddr );
		printf( "receive the %dth message: %s\n", 3, g.dataaddr );

		wait( 0 );
	}
	return;
}

int
main( int argc, char *argv[] )
{
	msg_test();
	exit( 0 );
}