#include "types.h"
#include "loongarch.h"
#include "spinlock.h"
#include "sem.h"
#include "defs.h"

int sem_used_count = 0;
struct sem sems[ SEM_MAX_NUM ];

void seminit()
{
	int i;
	for ( i = 0; i < SEM_MAX_NUM; ++i )
	{
		initlock( &( sems[ i ].lock ), "semaphore" );
		sems[ i ].allocated = 0;
	}
}