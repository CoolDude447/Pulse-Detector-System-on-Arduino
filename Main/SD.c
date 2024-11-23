#include "app.h"
#include "SPI.h"
#include "SD.h"
#include "USART3.h"
#include "integer.h"
#include <stdio.h>

// setup for slave select
#define GPIO_MAP_SS                     PORT_PA13
#define GPIO_GROUP_SS                   0

#define SPI_CS_LOW()                    PORT->Group[GPIO_GROUP_SS].OUTCLR.reg = GPIO_MAP_SS;
#define SPI_CS_HIGH()                   PORT->Group[GPIO_GROUP_SS].OUTSET.reg = GPIO_MAP_SS;

// Global Variables
uint8_t tmp_0 = 0;
uint8_t tmp_1 = 0;
uint8_t tmp_8 = 0;
uint8_t tmp_9 = 0;
uint8_t tmp_16 = 0;
uint8_t tmp_55 = 0;
uint8_t tmp_41 = 0;
uint8_t tmp_58 = 0;
uint8_t tmpAMD = 0;
uint8_t tmp17 = 0;
uint8_t tmp17_2 = 0;
uint8_t tmp_24 = 0;

// custom loop variable
uint8_t lvar = 0;

// data buffer
uint8_t  dataBuff[512];

// variable for SD Card Type
uint8_t SD_Type=0;

/*-----------------------------------------------------------------------*/
/* Set Slow SPI Speed                                                    */
/*-----------------------------------------------------------------------*/
void SDCard_InitSpeed(void)
{
	SPI_Initialize_Slow();
}

/*-----------------------------------------------------------------------*/
/* Set Fast SPI Speed                                                    */
/*-----------------------------------------------------------------------*/
void SDCard_RunSpeed(void)
{
	SPI_Initialize_Fast();
} // SDCard_RunSpeed()

/*-----------------------------------------------------------------------*/
/* Slave Select Function                                                 */
/*-----------------------------------------------------------------------*/
void SDCard_SS(uint8_t cs)
{
	if (cs == 1)
	{
		SPI_CS_HIGH();
	}
	else
	{
		SPI_CS_LOW();
	}
} // SDCard_SS()

/*-----------------------------------------------------------------------*/
/* Wait for SD Card to Read                                              */
/*-----------------------------------------------------------------------*/
uint8_t SDCard_WaitRead(void)
{
	uint32_t cnt = 0x00FFFFF;
	uint8_t  res;

	do
	{
		res = SPI_SD_Send_Byte(0xFF);

		if (res == 0xFF)
		{
			return 0;
		}
		cnt--;

	} while (cnt);

	return 1;
} // SDCard_WaitRead()

/*-----------------------------------------------------------------------*/
/* Wait for SD Card to Be Ready                                          */
/*-----------------------------------------------------------------------*/
uint8_t SD_WaitReady(void)
{
	uint8_t data;
	uint16_t attempt = 0;

	do
	{
		data = SPI_SD_Send_Byte(0xFF);

		if(attempt == 0xFFFE)
		{
			return 1;
		}
	} while(data != 0xFF);

	return 0;
} //SDCard_WaitReady()

/*-----------------------------------------------------------------------*/
/* Initialize the SD Card                                                */
/*-----------------------------------------------------------------------*/
void itoa(int num, char *str)
{
	int i = 0;
	if (num == 0)
	{
		str[i++] = '0';
		str[i] = '\0';
		return;
	}

	while (num != 0)
	{
		str[i++] = (num % 10) + '0';
		num /= 10;
	}
	str[i] = '\0';

	// Reverse the string
	int start = 0;
	int end = i - 1;
	while (start < end)
	{
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		start++;
		end--;
	}
}

#include <stdio.h>

uint8_t SDCard_Init(void)
{
	// local variables
	uint8_t tvar = 0;
	uint16_t cnt = 0;
	char numStr[10];  // Buffer for converting integer to string

	UART3_Write_Text("Starting SD Card Initialization\r\n");

	// Initialize the SD Card with a low speed
	SDCard_InitSpeed();
	UART3_Write_Text("SD Card Initialized to Slow Speed\r\n");
	delay_ms(100);

	// Set CS high initially to deselect the card
	SDCard_SS(1);

	// Send 100 dummy clock cycles with CS high to ensure the card is powered up
	for (tvar = 0; tvar < 100; tvar++)
	{
		SPI_SD_Send_Byte(0xFF);  // send dummy clocks
	}
	UART3_Write_Text("Sent 100 dummy clocks\r\n");

	// Set CS to LOW before trying to reset
	SDCard_SS(0);
	UART3_Write_Text("CS set to LOW\r\n");

	// Add a longer delay to ensure SD card is ready after dummy clocks
	delay_ms(20);

	// Try to reset the SD card
	lvar = 0;
	do
	{
		// Try reset using CMD0
		tmp_0 = SDCard_WriteCmd(CMD0, 0x00, 0x95);
		lvar++;

		// Convert lvar to string and append to message
		itoa(lvar, numStr);
		UART3_Write_Text("Attempting CMD0 Reset, attempt: ");
		UART3_Write_Text(numStr);
		UART3_Write_Text("\r\n");

		// Add debug message to check CMD0 response
		UART3_Write_Text("CMD0 Response: ");
		itoa(tmp_0, numStr);
		UART3_Write_Text(numStr);
		UART3_Write_Text("\r\n");

		delay_ms(10); // Add a delay between each attempt
	} while ((tmp_0 != 1) && (lvar < 500));  // Retry limit 500

	// The card reset failed
	if (lvar == 500)
	{
		SDCard_SS(1);
		SPI_SD_Send_Byte(0xFF);
		UART3_Write_Text("Failed to reset card after 500 attempts\r\n");
		return 1;
	}
	UART3_Write_Text("Card Reset Successful\r\n");

	// Send CMD8 to check voltage range
	tmp_8 = SDCard_WriteCmd(CMD8, 0x1AA, 0x87);
	UART3_Write_Text("CMD8 Sent\r\n");

	// CMD8 Response Handling
	if (tmp_8 == 1)
	{
		// Read the R7 response (4 bytes)
		for (tvar = 0; tvar < 4; tvar++)
		{
			dataBuff[tvar] = SPI_SD_Read_Byte();
		}

		// Check if the response is valid (last byte should be 0xAA)
		if (dataBuff[3] != 0xAA)
		{
			UART3_Write_Text("CMD8 Failed: Card does not support required voltage\r\n");
			return 1;
		}

		UART3_Write_Text("CMD8 Response Valid\r\n");

		// Send ACMD41 with HCS (High Capacity Support) bit set
		cnt = 0xFFFF;
		do
		{
			// Send CMD55 before ACMD41
			tmp_55 = SDCard_WriteCmd(CMD55, 0, 0xFF);
			UART3_Write_Text("CMD55 Sent\r\n");

			// Send ACMD41 with HCS bit set
			tmp_41 = SDCard_WriteCmd(CMD41, 0x40000000, 0xFF);
			UART3_Write_Text("ACMD41 Sent\r\n");

			cnt--;
		} while ((tmp_41) && (cnt));

		if (cnt == 0)
		{
			UART3_Write_Text("ACMD41 Timeout\r\n");
			SDCard_SS(1);
			return 1;
		}

		UART3_Write_Text("Card Initialization Complete\r\n");

		// Read OCR register with CMD58 to check CCS (Card Capacity Status)
		tmp_58 = SDCard_WriteCmd(CMD58, 0, 0xFF);
		UART3_Write_Text("CMD58 Sent\r\n");

		if (tmp_58 != 0x00)
		{
			SDCard_SS(1);
			UART3_Write_Text("Problem reading OCR data\r\n");
			return 1;
		}

		// Read OCR response (4 bytes)
		for (tvar = 0; tvar < 4; tvar++)
		{
			dataBuff[tvar] = SPI_SD_Read_Byte();
		}

		// Determine card type based on OCR response
		if (dataBuff[0] & 0x40)
		{
			SD_Type = SD_TYPE_V2HC;
			UART3_Write_Text("Card: V2.0 SDHC\r\n");
		}
		else
		{
			SD_Type = SD_TYPE_V2;
			UART3_Write_Text("Card Type: V2.0 Standard Capacity\r\n");
		}
	}
	else
	{
		UART3_Write_Text("CMD8 Failed, Card Not Supported\r\n");
		return 1;
	}

	// We have finished our initialization
	UART3_Write_Text("Initialization Complete \r\n");
	SDCard_RunSpeed();
	UART3_Write_Text("Switched to Fast Speed\r\n");
	SDCard_SS(1);

	return 0;
}




// Disable select with inline expansion
__inline static void SDCard_DisableSelect(void)
{
	SDCard_SS(1);
	SPI_SD_Send_Byte(0xFF);
}

// Enable select with inline expansion
__inline static uint8_t SDCard_EnableSelect(void)
{
	UART3_Write_Text("SD_SS to 0\r\n");
	SDCard_SS(0);
	UART3_Write_Text("Check if SDCard_WaitRead = 0\r\n");
	if (SDCard_WaitRead() == 0)
	{
		return 0;
	}
	UART3_Write_Text("SDCard Disable Select\r\n");
	SDCard_DisableSelect();

	return 1;
}

/*-----------------------------------------------------------------------*/
/* Write a command to the card                                           */
/*-----------------------------------------------------------------------*/
uint8_t SDCard_WriteCmd(uint8_t cmd, uint32_t arg, uint8_t crc)
{
	UART3_Write_Text("SDCardWrite_Cmd started!\r\n");
	uint16_t cnt = 512;
	uint8_t  res;

	// toggle line and transmit data
	UART3_Write_Text("Toggle Line and transmit data\r\n");
	SDCard_SS(1);
	SPI_SD_Send_Byte(0xFF);
	SDCard_SS(0);
	
	UART3_Write_Text("Card is selected!\r\n");
	if (SDCard_EnableSelect()) // Select the card and wait for ready except to stop multiple block read
	{
		return 0xFF;
	}
	UART3_Write_Text("start command and index\r\n");
	SPI_SD_Send_Byte(cmd | 0x40);          // start command and index
	UART3_Write_Text("Argument [31..24]\r\n");
	SPI_SD_Send_Byte((uint8_t)(arg >> 24));  // argument [31..24]
	UART3_Write_Text("Argument [23..16]\r\n");
	SPI_SD_Send_Byte((uint8_t)(arg >> 16));  // argument [23..16]
	UART3_Write_Text("Argument [15..8]\r\n");
	SPI_SD_Send_Byte((uint8_t)(arg >> 8));   // argument [15..8]
	UART3_Write_Text("Argument [7..0]\r\n");
	SPI_SD_Send_Byte((uint8_t)(arg));      // argument [7..0]
	UART3_Write_Text("Send Valid CRC and Stop!\r\n");
	SPI_SD_Send_Byte(crc);                 // Send valid CRC and stop

	do                                     // Wait for a valid response
	{
		res = SPI_SD_Send_Byte(0xFF);
		cnt--;
	} while ((cnt) && (res == 0xFF));

	return res; // Return with a response value
} // SDCard_WriteCmd()

/*-----------------------------------------------------------------------*/
/* Get the SD Card ID                                                    */
/*-----------------------------------------------------------------------*/
uint8_t SDCard_CardID(uint8_t cmd, uint8_t *buf)
{
	uint8_t i;

	if (SDCard_WriteCmd(cmd, 0x00, 0xFF) != 0x00)
	{
		return 1;
	}

	for (i = 0; i < 16; i++)
	{
		*buf++ = SPI_SD_Send_Byte(0xFF);
	}

	return 0;
}

/*-----------------------------------------------------------------------*/
/* Read a single block of Data                                           */
/*-----------------------------------------------------------------------*/
uint8_t SDCard_ReadSingleBlock(uint32_t addr, uint8_t *buf)
{
	uint16_t i;

	if(SD_Type != SD_TYPE_V2HC)
	{
		UART3_Write_Text("Not SD_TYPE_V2HC\r\n");
		addr = addr << 9;
	}

	tmp17 = SDCard_WriteCmd(CMD17, addr, 0x01);

	while(tmp17 != 0)
	{
		UART3_Write_Text("Read block address\r\n");
		tmp17 = SDCard_WriteCmd(CMD17, addr, 0x01);
	}
	delay_ms(1);

	while (SPI_SD_Read_Byte() != 0xFE)
	{
	}

	if(tmp17 == 0)
	{
		UART3_Write_Text("Reading\r\n");
		for (i = 0; i < 512; i++)
		{
			buf[i] = SPI_SD_Send_Byte(0xFF);
		}
	}

	// Done reading
	UART3_Write_Text("Read complete\r\n");
	SPI_SD_Send_Byte(0xFF);
	SPI_SD_Send_Byte(0xFF);
	SDCard_SS(1);
	return 0;
}

/*-----------------------------------------------------------------------*/
/* Read multiple blocks of Data                                          */
/*-----------------------------------------------------------------------*/
uint8_t SDCard_ReadMultipleBlock(uint32_t addr, uint8_t *buf, uint8_t count)
{
	uint16_t i;

	if(SD_Type != SD_TYPE_V2HC)
	{
		addr = addr << 9;
	}

	if (SDCard_WriteCmd(CMD18, addr, 0xFF) != 0x00)
	{
		return 1;
	}

	SDCard_SS(0);
	do
	{
		while(SPI_SD_Send_Byte(0xFF) != 0xFE);
		for (i = 0; i < 512; i++)
		{
			*buf++ = SPI_SD_Send_Byte(0xFF);
		}
		SPI_SD_Send_Byte(0xFF);
		SPI_SD_Send_Byte(0xFF);

	} while (--count);
	SDCard_SS(1);

	// Force stop transmission after multiple block read
	SDCard_WriteCmd(CMD12, 0x00, 0xFF);
	SPI_SD_Send_Byte(0xFF);
	return 0;
}

/*-----------------------------------------------------------------------*/
/* Write Single Block of Data                                            */
/*-----------------------------------------------------------------------*/
uint8_t SDCard_WriteSingleBlock(uint32_t addr, const uint8_t *buf)
{
	uint8_t  temp;
	uint16_t retry = 0;
	uint16_t i;

	if(SD_Type != SD_TYPE_V2HC)
	{
		addr = addr << 9;
	}

	tmp_24 = SDCard_WriteCmd(CMD24, addr, 0x01);

	while(tmp_24 != 0)
	{
		tmp_24 = SDCard_WriteCmd(CMD24, addr, 0x01);
	}

	if (tmp_24 != 0x00)
	{
		UART3_Write_Text("Reading\r\n");
		return 1;
	}

	SDCard_SS(0);

	// Wait until SD Card is ready
	SPI_SD_Send_Byte(0xFF);
	SPI_SD_Send_Byte(0xFF);
	SPI_SD_Send_Byte(0xFF);
	SPI_SD_Send_Byte(0xFE);

	for (i = 0; i < 512; i++)
	{
		SPI_SD_Send_Byte(buf[i]);
	}

	// Send CRC Info (dummy)
	SPI_SD_Send_Byte(0xFF);
	SPI_SD_Send_Byte(0xFF);
	temp = SPI_SD_Send_Byte(0xFF);
	temp &= 0x1F;

	if (temp != 0x05)
	{
		UART3_Write_Text("CRC Write Fail\r\n");
		SDCard_SS(1);
		return 1;
	}

	while (SPI_SD_Send_Byte(0xFF) == 0x00)
	{
		retry++;
		if(retry > 0xfffe)
		{
			SDCard_SS(1);
			return 1;
		}
	}

	SDCard_SS(1);
	SPI_SD_Send_Byte(0xFF);
	UART3_Write_Text("Write Complete\r\n");
	return 0;
}

/*-----------------------------------------------------------------------*/
/* Write Multiple Blocks of Data                                         */
/*-----------------------------------------------------------------------*/
uint8_t SDCard_WriteMultipleBlock(uint32_t addr, const uint8_t *buf, uint8_t count)
{
	uint16_t i;
	uint8_t tmp;

	if(SD_Type != SD_TYPE_V2HC)
	{
		addr = addr << 9;
	}

	if(SD_Type != SD_TYPE_MMC)
	{
		// pre-erase write sector
		tmp = SDCard_WriteCmd(CMD23, count, 0x00);
	}

	if (SDCard_WriteCmd(CMD25, addr, 0xFF) != 0x00)
	{
		UART3_Write_Text("Write Multiple Block Address Failed\r\n");
		return 1;
	}

	SDCard_SS(0);

	// Wait until SD Card is ready
	SPI_SD_Send_Byte(0xFF);
	SPI_SD_Send_Byte(0xFF);

	do
	{
		// Start block
		SPI_SD_Send_Byte(0xFC);

		// Write Data
		for (i = 0; i < 512; i++)
		{
			SPI_SD_Send_Byte(*buf++);
		}

		// Send CRC info (dummy)
		SPI_SD_Send_Byte(0xFF);
		SPI_SD_Send_Byte(0xFF);

		tmp = SPI_SD_Send_Byte(0xFF);
		tmp &= 0x1F;

		if(tmp != 0x05)
		{
			UART3_Write_Text("CRC Multiple Write Failed\r\n");
			SDCard_SS(1);
			return 1;
		}
		while(SPI_SD_Send_Byte(0xFF) == 0x00);
	} while (--count);

	// Send 'Stop Tran' Token
	SPI_SD_Send_Byte(0xFD);
	while (SPI_SD_Send_Byte(0xFF) == 0x00)
	{
	}

	// done writing
	SDCard_SS(1);
	SPI_SD_Send_Byte(0xFF);
	UART3_Write_Text("Write Multiple Complete\r\n");
	return 0;
}
