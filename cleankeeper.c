/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/
#include "cleankeeper.h"

#define SECOND 30120l // 30120 long definition of ONE second 
#define RUN_TIME (SECOND*20) 
#define OVER_LOAD_TIME (SECOND*62)
#define PULSE_FREQ 850 // 850 Hz is approx. 55 rpm

/******************************************************************************/
/* In/OUT
/******************************************************************************/
/* INPUT
 * GPIO0/PIN3: Green button
 * GPIO1/PIN4: Red button
 * 
 * OUTPUT
 * Relay Con1 PIN 1-3 Enable motor driver
 * GPIO2/PIN5: Direction 
 * GPIO3/PIN6: Overload
 * GPIO3/PIN8: Pulse 
 */

// Init for CleanKeeper
void CleanKeeperInit()
{
    // Set output to init states
    GPIO3_LAT = 1; // Sluk OVERLOAD lampe

    //Start pulse out
    PulseOut(PULSE_FREQ); // Output PWM pulse (duty cycle 50%)
}

    
/******************************************************************************
 * CleanKeeper sequence     
 * Main Loop
 ******************************************************************************/
void CleanKeeperController()
{
    static unsigned int iState;
    static unsigned long iTimer;
    static unsigned char bSignalRelay;
    unsigned char bGreen, bRed;
    static long iOverLoadTimer;
    
    //COMMON
    //Read input
    bGreen = GPIO0_PORT;
    bRed = GPIO1_PORT;
      
    // Default - catch if state out of bounds
    if (iState == 0 )  
    {
        iState = 1;
    }

    //STATE START (or RESTART) 
    if (iState == 1)
    {
        if ((bGreen == 1) || (bRed == 1)) //only active if GREEN or RED is enabled
        {
            // Button has been activated.
            //Set direction
            if (bGreen == 1)    // GREEN
            {
                GPIO2_LAT = 1;  // Set GPIO for CW
            }
            else                // RED
            {
                GPIO2_LAT = 0;  // Set GPIO for CCW            
            }
            //Set next state
            iState = 2; 
        }
    }
    //STATE SET 
    if (iState == 2)
    {
        //Start Timer
        iTimer = RUN_TIME; //20 seconds
        //Start relay
        bSignalRelay = 1;
        //Set next state
        if ((bGreen == 0) && (bRed == 0)) //both GREEN or RED NOT enabled
            iState = 3; 
    }

    //STATE COUNT-DOWN 
    if (iState == 3)
    {
        
        //Decrease timer until elapsed
        iTimer--;
        if (iTimer == 0) // Timer elepsed
        {
            iState = 4;                 
        }
        // If buttons pressed again:
        if ((bGreen == 1) || (bRed == 1)) //only active if GREEN or RED is enabled
        {   
            //Set direction
            if (bGreen == 1)    // GREEN
                GPIO2_LAT = 1;  // Set GPIO for CW
            else                // RED
                GPIO2_LAT = 0;  // Set GPIO for CCW
            iState = 2; 
        }
    }
    //STATE CLEAR 
    if (iState == 4)
    {
        //Stop relay
        bSignalRelay = 0;
        //Set next state
        if ((bGreen == 0) && (bRed == 0)) //Ensure both GREEN or RED NOT enabled
        {   
            iState = 1; // Restart 
        }
    }

    //STATE OVERLOAD 
    if (iState == 99)
    {
        //Remark: iOverLoadTimer is decreased in general handle output state below
        if ( iOverLoadTimer > 0 ) // continue until motor cooled down ...
        {
            REL1 = 0; // Force motor driver OFF
            GPIO3_LAT = 0; // OVERLOAD LAMPE LYSER
        }
        else
        {
            GPIO3_LAT = 1;  // Sluk OVERLOAD lampe
            iState = 4;     // State Clear     
        }
    }
   
    // Handle Output
    if ((bSignalRelay == 1) && (iState != 99))
    {
        //Start relay
        REL1 = 1; // Set relay (start motor driver) CommandSetRelay(0x01); (0x03 if both rel1 and trel2 on))

        iOverLoadTimer++;
        if (iOverLoadTimer > OVER_LOAD_TIME)
        {
            iOverLoadTimer = OVER_LOAD_TIME; // Lower limit
        }
    }
    
    if ((bSignalRelay == 0) || (iState == 99))
    {
        //Stop relay
        REL1 = 0; // Stop relay
        iOverLoadTimer--;
        if (iOverLoadTimer <= 0)
        {
            iOverLoadTimer = 0; // Lower limit
        }
    }
    
    // If OVERLOAD then react using State 99
    if (iOverLoadTimer >= OVER_LOAD_TIME)
    {
        iState = 99; // Overload State 
    }
}
