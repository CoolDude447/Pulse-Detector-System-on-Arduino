//////////////////////////////////////////////////////////////////////////
// Include and defines
//////////////////////////////////////////////////////////////////////////
#include "app.h"
#include "clock.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  // Required for caddr_t
#include "adc.h"
#include "USART3.h"
#include "SPI.h"
#include "SD.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#define F_CPU 48000000UL


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

//////////////////////////////////////////////////////////////////////////
// FreeRTOS Task Handles
//////////////////////////////////////////////////////////////////////////
TaskHandle_t SensorReadTaskHandle = NULL;
TaskHandle_t AlertTaskHandle = NULL;
TaskHandle_t DataLogTaskHandle = NULL;
TaskHandle_t CommunicationTaskHandle = NULL;

//////////////////////////////////////////////////////////////////////////
// Peripheral Initialization
//////////////////////////////////////////////////////////////////////////
void AppInit(void)
{
	// Initialize system clocks
	ClocksInit();

	// Initialize peripherals
	adc_init();       // ADC for SpO2 and PPG measurement (FR1)
	UART3_Init();    // UART for debugging and serial communication (FR5)
	SPI_Initialize_Slow();       // SPI for SD card communication (FR4)
	RTCInit();       // RTC for timestamping (FR3)
	PWM_Init();       // PWM for alerts (FR2)

	// Debugging LED initialization (optional)
	REG_PORT_DIR0 |= (1 << 27); // Assume LED is PA27
	REG_PORT_OUTCLR0 |= (1 << 27);
}

//////////////////////////////////////////////////////////////////////////
// FreeRTOS Tasks
//////////////////////////////////////////////////////////////////////////

// Task 1: Read sensor data and process SpO2 and PPG signals (FR1)
void vSensorReadTask(void *pvParameters)
{
	uint16_t adcValue = 0;

	while (1)
	{
		// Read ADC for PPG data
		adcValue = adc_readchannel();
		// Send processed value over UART for debugging
		char buffer[50];
		sprintf(buffer, "ADC Value: %u\r\n", adcValue);
		UART3_Write_Text(buffer);

		// Simulate delay between reads
		vTaskDelay(pdMS_TO_TICKS(500)); // 500 ms
	}
}

// Task 2: Trigger audible alerts if thresholds are exceeded (FR2)
void vAlertTask(void *pvParameters)
{
	// Threshold values
	const uint8_t SPO2_THRESHOLD = 90;  // SpO2 threshold in percentage
	const uint16_t PPG_THRESHOLD = 300; // Example PPG fluctuation threshold (adjust as needed)

	// Variables to hold current sensor data
	uint8_t spo2Level = 0;  // Placeholder for SpO2 percentage
	uint16_t ppgValue = 0;  // Placeholder for PPG signal amplitude

	while (1)
	{
		// Retrieve SpO2 and PPG values
		// Replace these placeholders with actual data retrieval logic
		spo2Level = 92;  // Example SpO2 level (replace with real data)
		ppgValue = 250;  // Example PPG value (replace with real data)

		// Check if alert conditions are met
		if (spo2Level < SPO2_THRESHOLD || ppgValue > PPG_THRESHOLD)
		{
			// Trigger alert (e.g., turn on a buzzer or LED using PWM)
			PWM_SetDuty(255); // Set PWM to 100% duty cycle for maximum alert
		}
		else
		{
			// Turn off alert
			PWM_SetDuty(0); // Set PWM to 0% duty cycle to stop the alert
		}

		// Check alert conditions every second
		vTaskDelay(pdMS_TO_TICKS(1000)); // 1-second delay
	}
}


// Task 3: Log data to SD card (FR3, FR4)
void vDataLogTask(void *pvParameters)
{
	while (1)
	{
		// Placeholder for SD card data logging
		// E.g., log ADC data with timestamp
		UART3_Write_Text("Logging data to SD card...\r\n");
		// Simulate SD card write
		vTaskDelay(pdMS_TO_TICKS(2000)); // Log every 2 seconds
	}
}

// Task 4: Communicate data via UART or wireless (FR5)
void vCommunicationTask(void *pvParameters)
{
	// Variables for storing data
	uint16_t pulseRate = 0;     // Placeholder for pulse rate value
	uint16_t spo2Level = 0;     // Placeholder for SpO2 percentage
	uint8_t hours = 12, minutes = 0, seconds = 0; // Initial time set in RTC
	char buffer[100];           // Buffer for formatted output

	while (1)
	{
		// Retrieve Pulse Rate and SpO2 Level (placeholders for actual values)
		// These should be calculated in the sensor task or a separate processing function
		pulseRate = 72;         // Example pulse rate (replace with actual data)
		spo2Level = 98;         // Example SpO2 level (replace with actual data)

		// Increment the time manually
		seconds++;
		if (seconds == 60)
		{
			seconds = 0;
			minutes++;
			if (minutes == 60)
			{
				minutes = 0;
				hours++;
				if (hours == 24)
				{
					hours = 0;
				}
			}
		}

		// Format the data into a string
		snprintf(buffer, sizeof(buffer),
		"Time: %02u:%02u:%02u | Pulse Rate: %u bpm | SpO2: %u%%\r\n",
		hours, minutes, seconds, pulseRate, spo2Level);

		// Transmit the data via UART
		UART3_Write_Text(buffer);

		// Simulate transmission delay (1 second)
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}


//////////////////////////////////////////////////////////////////////////
// Application Run (FreeRTOS Scheduler and Tasks)
//////////////////////////////////////////////////////////////////////////
void AppRun(void)
{
	// Create FreeRTOS tasks
	xTaskCreate(vSensorReadTask, "SensorRead", configMINIMAL_STACK_SIZE, NULL, 2, &SensorReadTaskHandle);
	xTaskCreate(vAlertTask, "Alert", configMINIMAL_STACK_SIZE, NULL, 2, &AlertTaskHandle);
	xTaskCreate(vDataLogTask, "DataLog", configMINIMAL_STACK_SIZE, NULL, 1, &DataLogTaskHandle);
	xTaskCreate(vCommunicationTask, "Communication", configMINIMAL_STACK_SIZE, NULL, 1, &CommunicationTaskHandle);

	// Start FreeRTOS scheduler
	vTaskStartScheduler();

	// Should never reach here
	while (1)
	{
	}
}


////////////////////////
// Initialize the PWM 
////////////////////////
void PWM_Init()
{
   /* -------------------------------------------------
	* 1) Enable bus clock to APBC mask
	*/
	REG_PM_APBCMASK |=  PM_APBCMASK_TCC0;
	
    /* -------------------------------------------------
	* 2) select TCC0 clock
	*/
	GCLK->CLKCTRL.reg = 
	GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC0_TCC1) |  // select TCC0 clock
	GCLK_CLKCTRL_CLKEN |                         // enable the clock
	GCLK_CLKCTRL_GEN(0);                         //  select GCLK GEN0
	
	while (GCLK->STATUS.bit.SYNCBUSY) {}
	
	/* -------------------------------------------------
	* 3) select PA05 as output
	*/
    PORT->Group[0].DIRSET.reg = PORT_PA05;
    PORT->Group[0].OUTCLR.reg = PORT_PA05;
	PORT->Group[0].PINCFG[5].reg |= PORT_PINCFG_PMUXEN;
	PORT->Group[0].PMUX[4 >> 1].reg =  PORT_PMUX_PMUXO_E;
	
	/* -------------------------------------------------
	* 4) Reset the PWM Module
	*/
	TCC0->CTRLA.reg = TCC_CTRLA_SWRST;
	while (TCC0->SYNCBUSY.reg & TCC_SYNCBUSY_SWRST) {}
	 
	/* -------------------------------------------------
	* 5) Configure the prescaler
	*/
	TCC0->CTRLA.reg = (TCC_CTRLA_PRESCSYNC_GCLK_Val | TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV8_Val));
	 
	/* -------------------------------------------------
	* 6) Set the waveform to normal PWM
	*/
	TCC0->WAVE.reg = (TCC_WAVE_WAVEGEN_NPWM);
	while (TCC0->SYNCBUSY.reg & TCC_SYNCBUSY_WAVE) {}
	 
	/* -------------------------------------------------
	* 7) Set the period
	*/
	TCC0->PER.reg = (255 - 1);
	while (TCC0->SYNCBUSY.reg & TCC_SYNCBUSY_PER) {}
	 
	/* -------------------------------------------------
	* 8) Initial duty cycle is zero
	*/
	TCC0->CC[1].reg = 0;
	 
	/* -------------------------------------------------
	* 9) Enable the TCC module
	*/ 
	TCC0->CTRLA.reg |= (TCC_CTRLA_ENABLE);
}


////////////////////////
// Set the Duty Cycle
////////////////////////
void PWM_SetDuty(uint8_t duty)
{
	 // Set the duty cycle for TCC0
	 TCC0->CC[1].reg = duty;
}

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
	RTCInit();

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
		
		// Wait for RTC to sync
		while (RTC->MODE2.STATUS.bit.SYNCBUSY);

		// Read the current time from RTC
		uint8_t hours = RTC->MODE2.CLOCK.bit.HOUR;
		uint8_t minutes = RTC->MODE2.CLOCK.bit.MINUTE;
		uint8_t seconds = RTC->MODE2.CLOCK.bit.SECOND;

		// Check if the result is greater than zero before printing
		if (result >= 0) {
			// Convert the ADC result to a string
			char buffer[10];
			itoa(result, buffer, 10);

			// Send ADC reading over UART
			UART3_Write_Text("Pulse: ");
			UART3_Write_Text(buffer);
			UART3_Write_Text(" ");
			
			// Print the current time over UART
			char timeStr[20];
			sprintf(timeStr, "Timestamp: %02d:%02d:%02d\r\n", hours, minutes, seconds);
			UART3_Write_Text(timeStr);
			
			UART3_Write_Text("\r\n");
			
			// Send ADC reading over UART
			UART3_Write_Text("SPo2: ");
			UART3_Write_Text(buffer);
			UART3_Write_Text(" ");
			
			// Print the current time over UART
			sprintf(timeStr, "Timestamp: %02d:%02d:%02d\r\n", hours, minutes, seconds);
			UART3_Write_Text(timeStr);
			UART3_Write_Text("\r\n");
			UART3_Write_Text("\r\n");
		}

		// Add a small delay between readings
		delay_s(3);
	}
}



