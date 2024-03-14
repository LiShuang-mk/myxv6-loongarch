#define MAX_SHM_PGNUM (4)
#define MAX_SHM_NUM (8)

struct sharemem
{
	int refcount;		// 引用计数 
	int pagenum;		// 占用页数（0~4）
	void *physaddr[ MAX_SHM_PGNUM ]; 	// 物理地址映射
};

extern struct spinlock shmlock;
extern struct sharemem shmtab[ MAX_SHM_NUM ];