/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/
/** 
 * _____________________________________________________________________________
 * TODO:
 * 
 * DC Motor_____________________________________________________________________
 * Setup input for selection between stepper and DC motor usage.
 * Supply voltage, plus to REL1_com and minus to REL2_com
 * DC_CW, REL1=1, REL2=0 (Motor+ to REL1_no and Motor- to REL2_nc)
 * DC_CcW, REL1=0, REL2=1 (Motor+ to REL1_nc and Motor- to REL2_no)
 * 
 * Frequency setup______________________________________________________________
 * Frequency. PWM freq is not exact!
 * 
 * Temperature monitoring:______________________________________________________
 * Use analog input for temperature measurement and compensate overload 
 * calculations with actual temperature.
 * 
 * Datalogging:_________________________________________________________________
 * Use analog out to show distribution between CW and CCW 
 * Monitor runtime and no. of activations.
 * 
 *______________________________________________________________________________ 
 * DOCUMENTATION:_______________________________________________________________
 * 
 * Data size maximum check:
 * unsigned long 32 bit +-2147483648
 * 600 seconds is           18072000
 * 
 * Info regarding PWM out:
 * Two formulaes for Freq <=920  or above
 * Range is [???..???]Hz
 * periode = (PR2+1)×4×Tosc×TMR2
 * TMR2 er prescale value
 * Tosc = 1 / Fosc
 * (PR2+1) * 4 * 1/16M * TMR2
 * Where PR2 is 200 (0xC8)
 * 200*4*1/16M*TMR2 =
 * 5.025*10E-5 * 64 = 0.003216
 * 
 * 1/0.003216 = 310 Hz
 * Page 221
 * Freq = 1 / (PR2+1)*4*1/16M*64
 * 
 * PR2 = ( 1 / Freq * 0.000016) -1
 *  
 */


#include "cleankeeper.h"

//GENERAL SETTINGS
#define false 0
#define true 1
#define CW 1
#define CCW 0

//MOTOR SETTINGS
#define MOTOR_STEP 200 // Stepper motor 1,8°
#define MICRO_STEPS 4  // Stepper driver microsteps

//TIME DEFINITIONS
#define SECOND 30120l // 30120 long definition of ONE second 

//USER SETTINGS
#define RUN_TIME (SECOND*20)                 // 20 seconds run time 
#define OVER_LOAD_TIME (SECOND*600)          // Wanted over load time 600
#define OVER_LOAD_TIME_CONTINUE (SECOND*540) // When to re-activate overload 540
#define MOTOR_RPM 51                         // Stepper motor speed (approx.)



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
 * 
 * Reserved:
 * Relay2 for DC motor usage
 * Input
 *
 */



/** Converting frequency to rpm for PL23 moons motor for CleanKeeper
 * 1,8° step => 200 steps/rev
 * Microstepping 4
 * Equation: Hertz = (RPM/60)*Steps*Microsteps
 * Hertz = RPM*K (where K=13,33 with acual constants)
 */
int ConvertRpm2Hz(int iRpm)
{
    long int iHertz;
    const long int iScale=100;

    iHertz = ((iRpm*iScale)/60)*MOTOR_STEP*MICRO_STEPS; // Scaled
    iHertz = iHertz/iScale;

    return iHertz;
}



/** Init for CleanKeeper
 */
void CleanKeeperInit()
{
    int iHertz;
    
    // Set output to init states
    GPIO3_LAT = 0; // OVERLOAD lamp OFF

    //Start pulse out
    iHertz = ConvertRpm2Hz(MOTOR_RPM);
    if (GPIO0_PORT == 1) // ####################################
        iHertz*=4;
    if (GPIO1_PORT == 1) // ####################################
        iHertz*=16;
    PulseOut(iHertz); // Output PWM pulse (duty cycle 50%)
}



/******************************************************************************
 * CleanKeeper sequence     
 * Main Loop
 * 
 *         *  *   
 *     *          *
 *  *                *
 **                    * 
 *         MAIN         *
 *         LOOP         *
 **                    *
 *  *                *
 *     *          *
 *         *  *
 ******************************************************************************/
void CleanKeeperController()
{
    static unsigned int iState;
    static long iTimer;
    static unsigned char bSignalRelay;
    static unsigned char bSignalDirection;
    static unsigned char bSignalOverload;
    unsigned char bGreen, bRed;
    static long iOverLoadTimer;
    
//*****************************************************************************
// COMMON PART PRE
//*****************************************************************************
    // Read input
    bGreen = GPIO0_PORT;
    bRed = GPIO1_PORT;
      
//*****************************************************************************
// STATE MACHINE PART
//*****************************************************************************
    // Default - catch if state out of bounds
    if (iState == 0 )  
    {
        if ((bGreen == 0) && (bRed == 0)) // Both GREEN and RED released
        {
            iState = 1;
        }
    }

    //STATE START (or RESTART) 
    if (iState == 1)
    {
        if ((bGreen == 1) || (bRed == 1)) //only active if GREEN or RED is enabled
        {
            // Button has been activated.
            // Set direction
            if (bGreen == 1)    // GREEN
            {
                bSignalDirection = CCW;  // Set for CW
            }
            else                // RED
            {
                bSignalDirection = CW;  // Set for CCW            
            }
            //Set next state
            iState = 2; 
        }
    }
    // STATE SET 
    if (iState == 2)
    {
        // Start Timer
        iTimer = RUN_TIME; // 20 seconds
        // Start relay
        bSignalRelay = 1;
        // Set next state
        if ((bGreen == 0) && (bRed == 0)) // Both GREEN or RED NOT enabled
        {
            iState = 3; 
        }
    }

    // STATE COUNT-DOWN 
    if (iState == 3)
    {
        //Decrease timer until elapsed
        iTimer--;
        if (iTimer <= 0) // Timer elapsed
        {
            iState = 4;                 
        }
        // If buttons pressed again:
        if ((bGreen == 1) || (bRed == 1)) // Either GREEN or RED enabled
        {   
            iState = 1; //Restart 
        }
    }
    // STATE CLEAR 
    if (iState == 4)
    {
        // Stop relay
        bSignalRelay = 0;
        // Set next state
        iState = 1; // Restart 
    }

    // STATE OVERLOAD 
    if (iState == 99)
    {
        bSignalOverload = true; // Request OVERLOAD LAMP ON
        bSignalRelay = 0; // Request motor driver off
        
        // Continue when motor is cooled down ...
        if ( iOverLoadTimer <= OVER_LOAD_TIME_CONTINUE ) 
        {
            bSignalOverload = false; // Request OVERLOAD LAMP OFF
            iState = 0;     // Restart      
        }
    }
   
//*****************************************************************************
// COMMON PART POST
//*****************************************************************************

    // Check for overload
    if ( iOverLoadTimer >= OVER_LOAD_TIME ) 
    {
        iState = 99;     // Enter overload state     
    }

    // Handle Output based on signal settings
    if (bSignalRelay == 1)
    {
        // Start relay
        REL1 = 1; // Set relay (start motor driver)
        
        // Setup direction CW/CCW
        if (bSignalDirection == CW)
        {
            GPIO2_LAT = 1;  // Set GPIO for CW
        }
        else
        {
            GPIO2_LAT = 0;  // Set GPIO for CCW            
        }

        // Increase overload timer when motor is active
        iOverLoadTimer++;
        if (iOverLoadTimer > OVER_LOAD_TIME)
        {
            iOverLoadTimer = OVER_LOAD_TIME; // Limit
        }
    }

    if (bSignalRelay == 0)
    {
        // Stop relay
        REL1 = 0; // Stop relay

        // Decrease overload timer when motor is stopped
        iOverLoadTimer--;
        if (iOverLoadTimer <= 0)
        {
            iOverLoadTimer = 0; // Lower limit
        }
    }

    if (bSignalOverload)
    {
            GPIO3_LAT = 1;  // OVERLOAD LAMP ON
    }
    else
    {
            GPIO3_LAT = 0;  // OVERLOAD LAMP OFF
    }
}



