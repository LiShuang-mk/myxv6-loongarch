#define MQMAX 8

struct msg
{
	struct msg *next;
	long type;
	char *dataaddr;
	int datasize;
};

struct mq
{
	int key;
	int status;
	struct msg *msgs;
	int maxbytes;
	int curbytes;
	int refcount;
};

extern struct spinlock mqlock;
extern struct mq mqs[ MQMAX ];

extern struct proc *wqueue[ NPROC ];
extern int wstart;

extern struct proc *rqueue[ NPROC ];
extern int rstart;