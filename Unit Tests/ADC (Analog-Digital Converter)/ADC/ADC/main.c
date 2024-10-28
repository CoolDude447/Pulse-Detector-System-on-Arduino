#include "sam.h"
#include "definitions.h"
#include "app.h"

//////////////////////////////////////////////////////////////////////////
// Function Prototypes
//////////////////////////////////////////////////////////////////////////
void ClocksInit(void);	// Configure Clock, Wait States and synch, bus clocks for 48MHz operation

int main(void)
{
	AppInit();
	
    /* Replace with your application code */
    while (1) 
    {
		AppRun();
    }
}
