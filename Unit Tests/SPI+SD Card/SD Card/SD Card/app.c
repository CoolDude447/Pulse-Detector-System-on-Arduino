#include "app.h"
#include "SPI.h"
#include "SD.h"
#include "USART3.h"
#include "clock.h"
#include "diskio.h"
#include "ff.h"
#include "integer.h"
#include "ffconf.h"
#include "delay.h"

// file handlers
FATFS fs;     // our work area
FRESULT FR;   // error results
FIL fil;      // file object
UINT bw;      // variable for pointer to number of bytes

// our data file
char data_file[12] = "Data.csv";

/*-----------------------------------------------------------------------*/
/* Application Initialization                                            */
/*-----------------------------------------------------------------------*/
void AppInit(void)
{
	// Initialize clocks
	ClocksInit();

	// Assign SS as OUTPUT
	REG_PORT_DIR0 |= PORT_PA13;

	// Set SS OFF
	REG_PORT_OUTSET0 = PORT_PA13;

	// Initialize UART3 for debugging
	UART3_Init(9600);
	UART3_Write_Text("UART3 Initialized\r\n");

	// Initialize SPI
	SPI_Initialize_Slow();
	UART3_Write_Text("SPI Initialized (Slow)\r\n");

	// Initialize SD card
	if (SDCard_Init() == 0)
	{
		UART3_Write_Text("SD Card Initialized Successfully\r\n");
	}
	else
	{
		UART3_Write_Text("SD Card Initialization Failed\r\n");
		while (1);
	}
}

/*-----------------------------------------------------------------------*/
/* Application Run Loop                                                  */
/*-----------------------------------------------------------------------*/
void AppRun(void)
{
	// Mount the SD card
	UART3_Write_Text("Mounting SD card...\r\n");
	FR = f_mount(&fs, "", 0);
	if (FR)
	{
		UART3_Write_Text("Error mounting file system\r\n");
		while (1);
	}
	UART3_Write_Text("SD card mounted successfully\r\n");

	// Open the SD Card for writing
	UART3_Write_Text("Opening file for writing...\r\n");
	FR = f_open(&fil, data_file, FA_WRITE | FA_CREATE_ALWAYS);
	if (FR)
	{
		UART3_Write_Text("Error opening file for writing\r\n");
		while (1);
	}

	// Write some CSV data to the file
	UART3_Write_Text("Writing data to file...\r\n");
	FR = f_write(&fil, "Data1,Data2,Data3,Data4\r\n", 25, &bw);
	if (FR)
	{
		UART3_Write_Text("Error writing to file\r\n");
		while (1);
	}

	// Close the file after writing
	FR = f_close(&fil);
	if (FR)
	{
		UART3_Write_Text("Error closing file after writing\r\n");
		while (1);
	}
	UART3_Write_Text("File written and closed successfully\r\n");

	// Open the SD Card for reading
	UART3_Write_Text("Opening file for reading...\r\n");
	FR = f_open(&fil, data_file, FA_READ);
	if (FR)
	{
		UART3_Write_Text("Error opening file for reading\r\n");
		while (1);
	}

	// Read data from the file and print to UART
	char line[100];
	UART3_Write_Text("Reading file contents:\r\n");
	while (f_gets(line, sizeof(line), &fil))
	{
		UART3_Write_Text(line);
	}

	// Close the file after reading
	FR = f_close(&fil);
	if (FR)
	{
		UART3_Write_Text("Error closing file after reading\r\n");
		while (1);
	}
	UART3_Write_Text("File read and closed successfully\r\n");

	// Keep the application running
	while (1)
	{
	}
}