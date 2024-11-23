#ifndef SPI_H_
#define SPI_H_

//////////////////////////////////////////////////////////////////////////
// Include and defines
//////////////////////////////////////////////////////////////////////////
#include "app.h"


/**
 * \def SPI_Initialize_Slow
 * \brief Initializes the SPI module in Slow Mode 400 kHz Baud
 * \param none 
 */
void SPI_Initialize_Slow(void);


/**
 * \def SPI_Initialize_Fast
 * \brief Initializes the SPI module in Fast Mode 12 MHz Baud (Att.)
 * \param none 
 */
void SPI_Initialize_Fast(void);


/**
 * \def SPI_Exchange8bit
 * \brief Exchanges 8 bits of data on SPI bus
 * \param none 
 */
uint8_t SPI_Exchange8bit(uint8_t data);


/**
 * \def SPI_SD_Send_Byte
 * \brief Exchanges 8 bits of data on SPI bus
 * \param none 
 */
uint8_t SPI_SD_Send_Byte(uint8_t byte_val);



/**
 * \def SPI_SD_Read_Byte
 * \brief Exchanges 8 bits of data on SPI bus
 * \param none 
 */
uint8_t SPI_SD_Read_Byte(void);


#endif /* SPI_H_ */