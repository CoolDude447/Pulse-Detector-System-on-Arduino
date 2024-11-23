//////////////////////////////////////////////////////////////////////////
// Include and defines
//////////////////////////////////////////////////////////////////////////
#include "delay.h"


/*******************************************************************************
 * Function:        void delay_n_cycles(unsigned long n)
 *
 * PreCondition:    None
 *
 * Input:           Delay in clock cycles
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This functions allows for delay in CPU cycles
 *
 * Note:
 *
 ******************************************************************************/

void delay_n_cycles(unsigned long n)
{
	__asm (
	"loop: DMB	\n"
	"SUB r0, r0, #1 \n"
	"CMP r0, #0  \n"
	"BNE loop         "
	);
} // delay_n_cycles()



