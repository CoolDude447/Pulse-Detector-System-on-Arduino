//////////////////////////////////////////////////////////////////////////
// Include and defines
//////////////////////////////////////////////////////////////////////////
#include "app.h"
#include "clock.h"
#include <stdio.h>
#include <stdlib.h>

#define F_CPU 48000000UL
#include "delay.h"

#include "adc.h"
#include "USART3.h"

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

	// Debug message to indicate initialization is complete
	UART3_Write_Text("UART Initialized successfully at 9600 baud.\r\n");

	// Initialize the ADC
	adc_init();
	delay_ms(100);
	UART3_Write_Text("ADC Initialized successfully.\r\n");

	// Variable to store the result of ADC conversion
	int result;

	while(1)
	{
		// Read the ADC channel connected to A0
		result = adc_readchannel(19); // A0 on the MKR Zero is connected to ADC channel 19 (PA11)

		// Check if the result is greater than zero before printing
		if (result >= 0) {
			// Convert the ADC result to a string
			char buffer[10];
			itoa(result, buffer, 10);

			// Send ADC reading over UART
			UART3_Write_Text("ADC Reading: ");
			UART3_Write_Text(buffer);
			UART3_Write_Text("\r\n");
		}

		// Add a small delay between readings
		delay_ms(100);
	}
}



