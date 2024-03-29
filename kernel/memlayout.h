// Physical memory layout

// 0x00200000 -- bios loads kernel here and jumps here
// 0x10000000 -- 
// 0x1c000000 -- reset address
// 0x1fe00000 -- I/O interrupt base address
// 0x1fe001e0 -- uart16550 serial port (3a5000 platform) : while 0x1fe20000 in 2k1000 platform 
// 0x90000000 -- RAM used by user pages

#define DMWIN_MASK 0x9000000000000000

// qemu puts UART registers here in virtual memory.
#define UART0 (0x1fe20000UL | DMWIN_MASK)
#define UART0_IRQ 2

/* ============== LS7A registers =============== */
#define LS7A_PCH_REG_BASE		(0x10000000UL | DMWIN_MASK)

#define LS7A_INT_MASK_REG		LS7A_PCH_REG_BASE + 0x020
#define LS7A_INT_EDGE_REG		LS7A_PCH_REG_BASE + 0x060
#define LS7A_INT_CLEAR_REG		LS7A_PCH_REG_BASE + 0x080
#define LS7A_INT_HTMSI_VEC_REG		LS7A_PCH_REG_BASE + 0x200
#define LS7A_INT_STATUS_REG		LS7A_PCH_REG_BASE + 0x3a0
#define LS7A_INT_POL_REG		LS7A_PCH_REG_BASE + 0x3e0

/* ============== 2K1000LA registers =========== */
#define LS2K_CONFIG_REG_BASE	( 0x1fe00000UL | DMWIN_MASK )

// ---- interrupt ----
//   | ---- low bits base address
#define LS2K_INT_LOW_BIT	( 0X1420 )
//   | ---- high bits base address
#define LS2K_INT_HIG_BIT	( 0X1460 )
//   | ---- intterupt bit 
#define LS2K_INT_BIT_UART0	( 0X1U << 0 )
//   | ---- registers offset
#define LS2K_INT_ISR		( 0X00 )		// RO interrupt status
#define LS2K_INT_ESR		( 0X04 )		// RO interrupt enable
#define LS2K_INT_ENR		( 0X08 )		// WO interrupt enable 
#define LS2K_INT_CLR		( 0X0C )		// WO clear interrupt enable and status 
#define LS2K_INT_POL 		( 0X10 )		// WR interrupt polarity control 
#define LS2K_INT_EDG		( 0X14 )		// WR interrupt trigger way ( edge or level )
#define LS2K_INT_BOU		( 0X18 ) 		// WO interrupt distribute way
#define LS2k_INT_AUT		( 0X1C ) 		// WO interrupt distribute way
//   | ---- routing registers base address 
#define LS2K_INT_ROU_BASE	( 0X1400 )
//   | ---- routing registers offset
#define LS2K_INT_ROU_UART0	( 0X00 )
//   | ---- config routing register
//		: x -> core number
//		: y -> interrupt pin number
#define INT_ROUTE_XY( x, y )	( ((uchar)x<<0) | ((uchar)y<<4) )


// the kernel expects there to be RAM
// for use by user pages
// from physical address 0x90000000 to PHYSTOP.
#define RAMBASE (0x90000000UL | DMWIN_MASK)
#define RAMSTOP (RAMBASE + 128*1024*1024)

// map kernel stacks beneath the trampframe,
// each surrounded by invalid guard pages.
#define KSTACK(p) (TRAPFRAME - ((p)+1)* 2*PGSIZE)

// User memory layout.
// Address zero first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
//   ...
//   invalid guard page
//   KSRACK (used for kernel thread)
//   TRAPFRAME (p->trapframe, used by the uservec)
#define TRAPFRAME (MAXVA - PGSIZE)
