#define SEM_MAX_NUM 128 

struct sem
{
	struct spinlock lock;
	int resource_count;
	int allocated;
};

extern int sem_used_count;
extern struct sem sems[ SEM_MAX_NUM ];

