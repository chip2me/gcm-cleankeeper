/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#include "cleankeeper.h"   /* System funct/params, like osc/peripheral config */
#include <htc.h>           /* Global Header File */
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */


/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/

//unsigned char DATA_BUFFER[6];
//unsigned char DATA_OUT[2];
//unsigned int DATA_INDEX = 0;
unsigned char ADDRESS;

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/


void main(void)
{
	/* Configure the oscillator for the device */
	ConfigureOscillator();

	/* Initialize I/O and Peripherals for application */
	InitApp();
    CleanKeeperInit(); //Overwrites setup done in InitApp())

    //Endless loop
    while (1)
    {
        CleanKeeperController(); //Application main loop
    }
}

