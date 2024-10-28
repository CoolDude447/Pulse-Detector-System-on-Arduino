//////////////////////////////////////////////////////////////////////////
// Include and defines
//////////////////////////////////////////////////////////////////////////
#include "adc.h"
#include "USART3.h"



/*******************************************************************************
 * Function:        void adc_init(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function initializes the ADC module
 *                  
 *
 * Note:            The default initialization is 10-bits resolution
 *
 ******************************************************************************/
void adc_init(void)
{
	// Enable APBC clock for ADC
	REG_PM_APBCMASK |= PM_APBCMASK_ADC;
	while (!(REG_PM_APBCMASK & PM_APBCMASK_ADC));

	// Assign clock source to ADC
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_ADC | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;
	while (GCLK->STATUS.bit.SYNCBUSY);

	// Reset ADC
	ADC->CTRLA.reg = ADC_CTRLA_SWRST;
	while (ADC->CTRLA.bit.SWRST || ADC->STATUS.bit.SYNCBUSY); // Wait for reset and sync to complete

	// Load calibration data
	ADC->CALIB.reg = ADC_CALIB_BIAS_CAL((*(uint32_t *)ADC_FUSES_BIASCAL_ADDR >> ADC_FUSES_BIASCAL_Pos)) |
	ADC_CALIB_LINEARITY_CAL((*(uint64_t *)ADC_FUSES_LINEARITY_0_ADDR >> ADC_FUSES_LINEARITY_0_Pos));

	// Set reference to VCC (3.3V)
	ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC0;

	// Set sample length and prescaler
	ADC->SAMPCTRL.reg = ADC_SAMPCTRL_SAMPLEN(10);
	ADC->CTRLB.bit.PRESCALER = ADC_CTRLB_PRESCALER_DIV32_Val;

	// Enable ADC
	ADC->CTRLA.reg = ADC_CTRLA_ENABLE;
	while (ADC->STATUS.bit.SYNCBUSY); // Wait for synchronization to complete
}


/*******************************************************************************
 * Function:        int32_t adc_readchannel(uint8_t channel)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function reads a channel number for ADC conversion
 *                  
 *
 * Note:            
 *
 ******************************************************************************/
int32_t adc_readchannel(uint8_t channel)
{
	// set positive MUX input selection
	ADC->INPUTCTRL.bit.MUXPOS = channel;

	// start conversion then flush
	// ADC will pick up where it left off
	ADC->SWTRIG.reg = ADC_SWTRIG_START | ADC_SWTRIG_FLUSH;

	// wait for analog conversion to complete
	while (ADC->INTFLAG.bit.RESRDY == 0)
	{

	}

	// return the result of the ADC
	return ADC->RESULT.reg;
}