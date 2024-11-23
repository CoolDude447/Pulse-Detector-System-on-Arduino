#include "app.h"
#include "SPI.h"

/*******************************************************************************
 * Function:        void SPI_Initialize_Fast(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function initializes the SPI bus at 12 MHz baud
 *                  
 * Note:            Needed for SPI initial slow function
 ******************************************************************************/
void SPI_Initialize_Fast(void)
{
    // Wait Sync
    while(SERCOM4->SPI.SYNCBUSY.bit.ENABLE);

    /* -------------------------------------------------
     * 1) Enable bus clock to APBC mask
     */
    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM4;

    /* -------------------------------------------------
     * 2) select SPI clock
     */
    GCLK->CLKCTRL.reg = 
        GCLK_CLKCTRL_ID(SERCOM4_GCLK_ID_CORE) |
        GCLK_CLKCTRL_CLKEN |
        GCLK_CLKCTRL_GEN(0);

    while(GCLK->STATUS.bit.SYNCBUSY);

    /* -------------------------------------------------
     * 3) setup pins
     */
    // Assign SS, MOSI, SCK as OUTPUT and MISO as INPUT
    PORT->Group[0].DIRSET.reg = PORT_PA13 | PORT_PA10 | PORT_PA11;  // Set SS, MOSI, SCK as output
    PORT->Group[0].DIRCLR.reg = PORT_PA12;  // Set MISO as input

    // Set SS OFF
    PORT->Group[0].OUTSET.reg = PORT_PA13;

    // Set the PMUX for PA10 (MOSI), PA11 (SCK), PA12 (MISO), PA13 (SS)
    PORT->Group[0].PINCFG[10].reg |= PORT_PINCFG_PMUXEN;  // Enable PMUX for PA10 (MOSI)
    PORT->Group[0].PINCFG[11].reg |= PORT_PINCFG_PMUXEN;  // Enable PMUX for PA11 (SCK)
    PORT->Group[0].PINCFG[12].reg |= PORT_PINCFG_PMUXEN;  // Enable PMUX for PA12 (MISO)
    PORT->Group[0].PINCFG[13].reg |= PORT_PINCFG_PMUXEN;  // Enable PMUX for PA13 (SS)

    // Set the PMUX value for pins (each pair shares a PMUX register)
    PORT->Group[0].PMUX[5].reg = PORT_PMUX_PMUXO_C | PORT_PMUX_PMUXE_C; // PA10 (MOSI) and PA11 (SCK) to SERCOM4
    PORT->Group[0].PMUX[6].reg = PORT_PMUX_PMUXE_C;                     // PA12 (MISO) to SERCOM4
    PORT->Group[0].PMUX[6].reg |= PORT_PMUX_PMUXO_C;                    // PA13 (SS) to SERCOM4

    /* -------------------------------------------------
     * 4) Configure SPI Module
     */
    // Disable the SERCOM SPI module
    SERCOM4->SPI.CTRLA.bit.ENABLE = 0;

    // Wait for synchronization
    while(SERCOM4->SPI.SYNCBUSY.bit.SWRST)
    {
    }

    // Perform a software reset
    SERCOM4->SPI.CTRLA.bit.SWRST = 1;

    // Wait for synchronization
    while(SERCOM4->SPI.CTRLA.bit.SWRST)
    {
    }

    // Wait for synchronization
    while(SERCOM4->SPI.SYNCBUSY.bit.SWRST || SERCOM4->SPI.SYNCBUSY.bit.ENABLE);

    SERCOM4->SPI.CTRLA.reg = 
        SERCOM_SPI_CTRLA_MODE_SPI_MASTER |   // set SPI Master Mode
        SERCOM_SPI_CTRLA_DIPO(0) |           // PAD0 is used as data input (MISO)
        SERCOM_SPI_CTRLA_DOPO(1);            // PAD2 is used as data output (MOSI), PAD3 is SCK

    SERCOM4->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;      // Enable SPI Receive enable

    /* -------------------------------------------------
     * 5) Set the baud rate
     */
    uint32_t BAUD_REG =  ((float)48000000 / (float)(2 * 12000000)) - 1; // Calculate BAUD value
    SERCOM4->SPI.BAUD.reg = SERCOM_SPI_BAUD_BAUD(BAUD_REG);

    /* -------------------------------------------------
     * 6) Enable SPI Module
     */
    // Enable SPI receive
    SERCOM4->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_ENABLE; // Enable the SPI
    while(SERCOM4->SPI.SYNCBUSY.bit.ENABLE);
}

/*******************************************************************************
 * Function:        void SPI_Initialize_Slow(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function initializes the SPI bus at 400 kHz baud
 ******************************************************************************/
void SPI_Initialize_Slow(void)
{
    // Wait Sync
    while(SERCOM4->SPI.SYNCBUSY.bit.ENABLE);

    /* -------------------------------------------------
     * 1) Enable bus clock to APBC mask
     */
    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM4;

    /* -------------------------------------------------
     * 2) select SPI clock
     */
    GCLK->CLKCTRL.reg = 
        GCLK_CLKCTRL_ID(SERCOM4_GCLK_ID_CORE) |
        GCLK_CLKCTRL_CLKEN |
        GCLK_CLKCTRL_GEN(0);

    while(GCLK->STATUS.bit.SYNCBUSY);

    /* -------------------------------------------------
     * 3) setup pins
     */
    // Assign SS, MOSI, SCK as OUTPUT and MISO as INPUT
    PORT->Group[0].DIRSET.reg = PORT_PA13 | PORT_PA10 | PORT_PA11;  // Set SS, MOSI, SCK as output
    PORT->Group[0].DIRCLR.reg = PORT_PA12;  // Set MISO as input

    // Set SS OFF
    PORT->Group[0].OUTSET.reg = PORT_PA13;

    // Set the PMUX for PA10 (MOSI), PA11 (SCK), PA12 (MISO), PA13 (SS)
    PORT->Group[0].PINCFG[10].reg |= PORT_PINCFG_PMUXEN;  // Enable PMUX for PA10 (MOSI)
    PORT->Group[0].PINCFG[11].reg |= PORT_PINCFG_PMUXEN;  // Enable PMUX for PA11 (SCK)
    PORT->Group[0].PINCFG[12].reg |= PORT_PINCFG_PMUXEN;  // Enable PMUX for PA12 (MISO)
    PORT->Group[0].PINCFG[13].reg |= PORT_PINCFG_PMUXEN;  // Enable PMUX for PA13 (SS)

    // Set the PMUX value for pins (each pair shares a PMUX register)
    PORT->Group[0].PMUX[5].reg = PORT_PMUX_PMUXO_C | PORT_PMUX_PMUXE_C; // PA10 (MOSI) and PA11 (SCK) to SERCOM4
    PORT->Group[0].PMUX[6].reg = PORT_PMUX_PMUXE_C;                     // PA12 (MISO) to SERCOM4
    PORT->Group[0].PMUX[6].reg |= PORT_PMUX_PMUXO_C;                    // PA13 (SS) to SERCOM4

    /* -------------------------------------------------
     * 4) Configure SPI Module
     */
    // Disable the SERCOM SPI module
    SERCOM4->SPI.CTRLA.bit.ENABLE = 0;

    // Wait for synchronization
    while(SERCOM4->SPI.SYNCBUSY.bit.SWRST)
    {
    }

    // Perform a software reset
    SERCOM4->SPI.CTRLA.bit.SWRST = 1;

    // Wait for synchronization
    while(SERCOM4->SPI.CTRLA.bit.SWRST)
    {
    }

    // Wait for synchronization
    while(SERCOM4->SPI.SYNCBUSY.bit.SWRST || SERCOM4->SPI.SYNCBUSY.bit.ENABLE);

    SERCOM4->SPI.CTRLA.reg = 
        SERCOM_SPI_CTRLA_MODE_SPI_MASTER |   // set SPI Master Mode
        SERCOM_SPI_CTRLA_DIPO(0) |           // PAD0 is used as data input (MISO)
        SERCOM_SPI_CTRLA_DOPO(1);            // PAD2 is used as data output (MOSI), PAD3 is SCK

    SERCOM4->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;      // Enable SPI Receive enable

    /* -------------------------------------------------
     * 5) Set the baud rate
     */
    uint32_t BAUD_REG =  ((float)48000000 / (float)(2 * 400000)) - 1; // Calculate BAUD value
    SERCOM4->SPI.BAUD.reg = SERCOM_SPI_BAUD_BAUD(BAUD_REG);

    /* -------------------------------------------------
     * 6) Enable SPI Module
     */
    // Enable SPI receive
    SERCOM4->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_ENABLE; // Enable the SPI
    while(SERCOM4->SPI.SYNCBUSY.bit.ENABLE);
}

/*******************************************************************************
 * Function:        uint8_t SPI_Exchange8bit(uint8_t data)
 *
 * PreCondition:    The SPI Bus Must be Initialized
 *
 * Input:           The data we want to send
 *
 * Output:          The data Exchanged via SPI
 *
 * Side Effects:    None
 *
 * Overview:        This function exchanges a byte of data on the SPI Bus
 ******************************************************************************/
uint8_t SPI_Exchange8bit(uint8_t data)
{
    while(SERCOM4->SPI.INTFLAG.bit.DRE == 0);
    SERCOM4->SPI.DATA.reg = data;

    while(SERCOM4->SPI.INTFLAG.bit.RXC == 0);
    return (uint8_t)SERCOM4->SPI.DATA.reg;
}

/*******************************************************************************
 * Function:        uint8_t SPI_SD_Send_Byte(uint8_t byte_val)
 *
 * PreCondition:    The SPI Bus Must be Initialized
 *
 * Input:           The data we want to send
 *
 * Output:          Data Exchanged on SPI Transfer
 *
 * Side Effects:    None
 *
 * Overview:        This function sends a byte of data to the SD Card
 ******************************************************************************/
uint8_t SPI_SD_Send_Byte(uint8_t byte_val)
{
    uint8_t data;

    data = SPI_Exchange8bit(byte_val);
    return data;
}

/*******************************************************************************
 * Function:        uint8_t SPI_SD_Read_Byte(void)
 *
 * PreCondition:    SPI Bus Must Be Initialized
 *
 * Input:           None
 *
 * Output:          Data Read from SPI Bus
 *
 * Side Effects:    None
 *
 * Overview:        This function reads a byte of data from the SPI Module
 ******************************************************************************/
uint8_t SPI_SD_Read_Byte(void)
{
    uint8_t data;

    data = SPI_Exchange8bit(0xff);
    return data;
}
