//////////////////////////////////////////////////////////////////////////
// Include and defines
//////////////////////////////////////////////////////////////////////////
#include "app.h"
#include "clock.h"
#include <stdio.h>
#include "USART3.h"
#include <sys/types.h>  // Required for caddr_t

caddr_t _sbrk(int incr)
{
	extern char _end;  // Defined by the linker, marks end of the .bss section
	static char *heap_end;
	char *prev_heap_end;

	if (heap_end == 0)
	{
		heap_end = &_end;
	}

	prev_heap_end = heap_end;
	heap_end += incr;

	return (caddr_t)prev_heap_end;
}

/*******************************************************************************
 * Function:        void AppInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine takes care of all of the 1-time hardware/software
 *                  initialization that is required.
 *
 * Note:            This was taken from the _00_LED_ON provided by
 *                  Microchip Technology Inc. 
 *
 ******************************************************************************/
void AppInit(void)
{
	/*	Clock initialization (CPU, AHB, APBx, Asynchronous Peripheral Clocks)
		The System RC Oscillator (RCSYS) provides the source for the main clock
		at chip startup. It is set to 1MHz.
	*/
	ClocksInit();
	
	// Assign LED0 as OUTPUT
	REG_PORT_DIR0 = LED0_PIN_MASK;
	
	// Set LED0 OFF
	REG_PORT_OUTCLR0 = LED0_PIN_MASK;
	

} // AppInit()




/*******************************************************************************
 * Function:        void AppRun(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function contains your main application
 *                  
 *
 * Note:
 *
 ******************************************************************************/
void AppRun(void)
{
	// Initialize the UART at 9600 baud
	UART3_Init(9600);
	delay_ms(500);

	// Set up RTC
	UART3_Write_Text("RTC Initialization started\r\n");
	RTCInit();
	delay_ms(500);

	while (1)
	{
		// Wait for RTC to sync
		while (RTC->MODE2.STATUS.bit.SYNCBUSY);

		// Read the current time from RTC
		uint8_t hours = RTC->MODE2.CLOCK.bit.HOUR;
		uint8_t minutes = RTC->MODE2.CLOCK.bit.MINUTE;
		uint8_t seconds = RTC->MODE2.CLOCK.bit.SECOND;

		// Print the current time over UART
		char timeStr[20];
		sprintf(timeStr, "Time: %02d:%02d:%02d\r\n", hours, minutes, seconds);
		UART3_Write_Text(timeStr);

		// Delay for 5 second to avoid rapid updates
		delay_s(5);
	}
} // AppRun()


