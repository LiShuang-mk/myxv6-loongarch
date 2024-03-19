#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "loongarch.h"
#include "defs.h"

//
// the loongarch 7A1000 I/O Interrupt Controller Registers.
//

void
apic_init( void )
{
	// *( volatile uint64* ) ( LS7A_INT_MASK_REG ) = ~( 0x1UL << UART0_IRQ );

	// *( volatile uint64* ) ( LS7A_INT_EDGE_REG ) = ( 0x1UL << UART0_IRQ );

	// *( volatile uint8* ) ( LS7A_INT_HTMSI_VEC_REG + UART0_IRQ ) = UART0_IRQ;

	// *( volatile uint64* ) ( LS7A_INT_POL_REG ) = 0x0UL;

	*( volatile uint32* ) ( LS2K_CONFIG_REG_BASE | LS2K_INT_LOW_BIT | LS2K_INT_CLR ) = ~0x0U;
	*( volatile uint32* ) ( LS2K_CONFIG_REG_BASE | LS2K_INT_HIG_BIT | LS2K_INT_CLR ) = ~0x0U;
	*( volatile uint32* ) ( LS2K_CONFIG_REG_BASE | LS2K_INT_LOW_BIT | LS2K_INT_EDG ) = 0x0U;
	*( volatile uint32* ) ( LS2K_CONFIG_REG_BASE | LS2K_INT_HIG_BIT | LS2K_INT_EDG ) = 0x0U;

	*( volatile uint32* ) ( LS2K_CONFIG_REG_BASE | LS2K_INT_LOW_BIT | LS2K_INT_BOU ) = 0x0U;
	*( volatile uint32* ) ( LS2K_CONFIG_REG_BASE | LS2K_INT_LOW_BIT | LS2k_INT_AUT ) = 0x0U;
	*( volatile uint32* ) ( LS2K_CONFIG_REG_BASE | LS2K_INT_LOW_BIT | LS2K_INT_POL ) = LS2K_INT_BIT_UART0;
	*( volatile uint32* ) ( LS2K_CONFIG_REG_BASE | LS2K_INT_LOW_BIT | LS2K_INT_ENR ) = LS2K_INT_BIT_UART0;
	*( volatile uint32* ) ( LS2K_CONFIG_REG_BASE | LS2K_INT_ROU_BASE | LS2K_INT_ROU_UART0 ) = INT_ROUTE_XY( 0, 0 );
}

// tell the apic we've served this IRQ.
void
apic_complete( uint64 irq )
{
	*( volatile uint64* ) ( LS7A_INT_CLEAR_REG ) = ( irq );
}
