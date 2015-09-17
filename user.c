/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#include <xc.h>             /* HiTech General Includes */
#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

#include "user.h"
#include "i2c.h"
#include "OLIMEX.h"

/******************************************************************************/
/* User Functions                                                             */

/******************************************************************************/


void InitApp(void)
{
	InitGPIO();
	InitAddress();
	//CM InitI2C();
	//CM InitInterupts();
}

void InitI2C(void)
{
	SSPCON2 = 0x01; // Enable clock streching
	SSPCON3 = 0x40; // Enable interrupts on START and STOP
	SSPSTAT = 0x80; // Slew rate for 100kHz
	SSPADD = ADDRESS << 1;
	SSPCON1 = 0x3E;
}

void InitGPIO(void)
{
	ANSELC = 0x00;
	ANSELA = 0x00;

	/* Make all inputs */
	TRISA = 0x00; //CM? All port A as output
	TRISC = 0xFF;

	/* Clear LAT registers */
	LATA = 0x00;
	LATC = 0x00;

	SCL_TRIS = 1;
	SDA_TRIS = 1;
	SCL_LAT = 0;
	SDA_LAT = 0;

	REL1_TRIS = 0;
	REL2_TRIS = 0;
	REL1 = 0;
	REL2 = 0;

	OPTION_REGbits.nWPUEN = 0;
	WPUAbits.WPUA = 0x20; //CM? *0x20 or 0xFF
	JMP_TRIS = 1;
	JMP_LAT = 0;
}

void InitInterupts(void)
{
	PIR1bits.SSP1IF = 0;
	PIE1bits.SSP1IE = 1;
	INTCONbits.PEIE = 1;
	INTCONbits.GIE = 1;

}

void InitAddress(void)
{
	unsigned int addr;
	addr = ReadFlash(MEMORY_ADDRESS);
	if ((addr & 0xFF) == 0xFF)
		ADDRESS = DEFAULT_ADDRESS;
	else
		ADDRESS = addr & 0xFF;
}

unsigned int ReadFlash(unsigned int address)
{
	unsigned int data = 0;
	PMADRL = address & 0x00FF;
	address >>= 8;
	PMADRH = address & 0x00FF;
	PMCON1bits.CFGS = 0;
	PMCON1bits.RD = 1;
	Nop();
	Nop();

	data = PMDATH;
	data <<= 8;
	data |= PMDATL;
	return data;
}

void EraseFlash(unsigned int address)
{
	PMADRL = address & 0x00FF;
	address >>= 8;
	PMADRH = address & 0x00FF;
	PMCON1bits.CFGS = 0;
	PMCON1bits.FREE = 1;
	PMCON1bits.WREN = 1;
	UnlockFlash();
	PMCON1bits.WREN = 0;
}

void UnlockFlash(void)
{
	PMCON2 = 0x55;
	PMCON2 = 0xAA;
	PMCON1bits.WR = 1;
	Nop();
	Nop();
}

void WriteFlash(unsigned int data, unsigned int address)
{
	PMCON1bits.CFGS = 0;
	PMADRL = address & 0x00FF;
	address >>= 8;
	PMADRH = address & 0x00FF;
	PMCON1bits.FREE = 0;
	PMCON1bits.LWLO = 1;
	PMCON1bits.WREN = 1;
	PMDATL = data & 0x00FF;
	data >>= 8;
	PMDATH = data & 0x00FF;
	PMCON1bits.LWLO = 0;
	UnlockFlash();
	PMCON1bits.WREN = 0;
}

void CommandSetTris(unsigned char data)
{
	GPIO0_TRIS = data & 0x01;
	data >>= 1;
	GPIO1_TRIS = data & 0x01;
	data >>= 1;
	GPIO2_TRIS = data & 0x01;
	data >>= 1;
	GPIO3_TRIS = 1;
	data >>= 1;
	GPIO4_TRIS = data & 0x01;
	data >>= 1;
	GPIO5_TRIS = data & 0x01;
	data >>= 1;
	GPIO6_TRIS = data & 0x01;

}

void CommandSetLat(unsigned char data)
{
	GPIO0_LAT = data & 0x01;
	data >>= 1;
	GPIO1_LAT = data & 0x01;
	data >>= 1;
	GPIO2_LAT = data & 0x01;
	data >>= 1;
	//	GPIO3_LAT = command & 0x01;	GPIO3 is ALWAYS INPUT!!!
	data >>= 1;
	GPIO4_LAT = data & 0x01;
	data >>= 1;
	GPIO5_LAT = data & 0x01;
	data >>= 1;
	GPIO6_LAT = data & 0x01;

}

unsigned char CommandGetPort()
{
	unsigned char data = 0;

	data = GPIO6_PORT & 0x01;
	data <<= 1;
	data |= GPIO5_PORT & 0x01;
	data <<= 1;
	data |= GPIO4_PORT & 0x01;
	data <<= 1;
	data |= GPIO3_PORT & 0x01;
	data <<= 1;
	data |= GPIO2_PORT & 0x01;
	data <<= 1;
	data |= GPIO1_PORT & 0x01;
	data <<= 1;
	data |= GPIO0_PORT & 0x01;

	return data;
}

void CommandSetRelay(unsigned char data)
{
	REL1 = data & 0x01;
	data >>= 1;
	REL2 = data & 0x01;
	data >>= 1;
}

void CommandSetAddress(unsigned char addr)
{
	if (!JMP_PORT) {
		EraseFlash(MEMORY_ADDRESS);
		WriteFlash(addr, MEMORY_ADDRESS);
		ADDRESS = addr;
		SSPCON1bits.SSPEN = 0;
		SSPCON2 = 0x01;
		SSPCON3 = 0x40;
		SSPSTAT = 0x00;
		SSPADD = ADDRESS << 1;
		SSPCON1 = 0x3E;
	}
}

void CommandSetPullUps(unsigned char data)
{
	WPUAbits.WPUA0 = data & 0x01;
	data >>= 1;
	WPUAbits.WPUA1 = data & 0x01;
	data >>= 1;
	WPUAbits.WPUA2 = data & 0x01;
	data >>= 1;
	WPUAbits.WPUA3 = data & 0x01;
	data >>= 1;
	WPUAbits.WPUA5 = data & 0x01;
	data >>= 1;

}

unsigned char CommandGetTris(void)
{
	unsigned char data = 0;
	data |= GPIO6_TRIS;
	data <<= 1;
	data |= GPIO5_TRIS;
	data <<= 1;
	data |= GPIO4_TRIS;
	data <<= 1;
	data |= GPIO3_TRIS;
	data <<= 1;
	data |= GPIO2_TRIS;
	data <<= 1;
	data |= GPIO1_TRIS;
	data <<= 1;
	data |= GPIO0_TRIS;

	return data;
}

unsigned char CommandGetLat(void)
{
	unsigned char data = 0;
	data |= GPIO6_LAT;
	data <<= 1;
	data |= GPIO5_LAT;
	data <<= 1;
	data |= 0; // Note: GPIO4 can't be output!
	data <<= 1;
	data |= GPIO3_LAT;
	data <<= 1;
	data |= GPIO2_LAT;
	data <<= 1;
	data |= GPIO1_LAT;
	data <<= 1;
	data |= GPIO0_LAT;

	return data;
}

unsigned int CommandGetAnalog(unsigned char channel)
{
	unsigned int data = 0;
	unsigned char count = 0;

	// Configure port
	switch (channel) {
	case 0:
		GPIO0_TRIS = 1;
		ANSELAbits.ANSA0 = 1;
		break;
	case 1:
		GPIO1_TRIS = 1;
		ANSELAbits.ANSA1 = 1;
		break;
	case 2:
		GPIO2_TRIS = 1;
		ANSELAbits.ANSA2 = 1;
		break;
	case 3:
		GPIO3_TRIS = 1;
		ANSELAbits.ANSA4 = 1;
		break;
	case 7:
		GPIO5_TRIS = 1;
		ANSELCbits.ANSC3 = 1;
		break;
	default:
		break;
	}

	// Configure the ADC modules
	ADCON1bits.ADCS = 0x00;		// Fosc/2
	ADCON1bits.ADPREF = 0x00;	// Vref+
	ADCON1bits.ADFM = 0x01;		// Right justify
	ADCON0bits.CHS = channel;	// Select input channel
	ADCON0bits.ADON = 0x01;		// Turn on the ADC module


	/* Wait aprox 20us aquisition time */
	for(count = 0; count < 5; count++)
		asm("NOP");
	ADCON0bits.GO_nDONE = 1;	// Start conversation
	while(ADCON0bits.GO_nDONE);	// Wait conversation to complete

	data = ADRESH;
	data <<= 8;
	data |= ADRESL;

	// Disable analog function for ports
	switch (channel) {
	case 0:
		ANSELAbits.ANSA0 = 0;
		break;
	case 1:
		ANSELAbits.ANSA1 = 0;
		break;
	case 2:
		ANSELAbits.ANSA2 = 0;
		break;
	case 3:
		ANSELAbits.ANSA4 = 0;
		break;
	case 7:
		ANSELCbits.ANSC3 = 0;
		break;
	default:
		break;
	}
	
	return data;
}



// Delay function calibrated to approx. ms
// CM 24. april 2015
Wait_ms(int iDelay)
{
    int iCount, iInnerCount;
    const int iWAIT = 116; //Calibration constant
    // Wait
	for(iCount = 0; iCount < iDelay; iCount++)
    {
        asm("NOP");
            // Wait
            for(iInnerCount = 0; iInnerCount < iWAIT; iInnerCount++)
            {
                asm("NOP");
                asm("NOP");
                asm("NOP");
                asm("NOP");
                asm("NOP");
                asm("NOP");
            }
    }
}



/** Handle "illegal" frequencies 
    Could be solved by investigating register more deeply...
*/
unsigned int LimitFreq(unsigned int iFreq)
{
    unsigned int iLimited;
    
    if (iFreq <= 245)
        iLimited = 250;
    else if ((iFreq >= 920) && (iFreq <= 1000))
        iLimited = 1001;
    else if (iFreq >= 10000)
        iLimited = 10000;
    else
        iLimited = iFreq;
    return iLimited;
}



void SetPWMDutyCycle(unsigned int DutyCycle)    // Give a value in between 0 and 128 for DutyCycle 
{ 
    PWM2DCH = DutyCycle; // Put MSB 8 bits in PWMDCH    
    PWM2DCL = 0x00;      // Assign Last 2 LSBs to PWMDCL 
} 



// PWM out for Stepper Driver pulse
// CM 27. april 2015
void PulseOut(unsigned int iFreq)
{
    unsigned int iPR2;
    unsigned int iLimitedFreq;
    
    iLimitedFreq = LimitFreq(iFreq);
    
    //CM TRISC   = 0b11100111;      // Make C port as output
    // After line above removed the REL2 works again :-)

    TRISCbits.TRISC3 = 0; // PWM2 out
    PWM2CON = 0xC0;      // Configure PWM1 module in PWM mode.
    PIR2    = 0x00; 
//    SetPWMDutyCycle(95); // Intialize the PWM to 50 % duty cycle  (*64))

    //Calculate PR2 to give correct output frequency
    if (iLimitedFreq<=920)
    {
        PR2 = (1/(iLimitedFreq*0.000016)-1);
        T2CON   = 0x07;      // Bit 2 = timer1 enable, bit 0/1 = prescale 
    }
    else
    {
        PR2 = (1/(iLimitedFreq*0.000004)-1);
        T2CON   = 0x06;      // Bit 2 = timer1 enable, bit 0/1 = prescale 
    }
    iPR2 = PR2;
    iPR2=iPR2/2;
    SetPWMDutyCycle(iPR2); // Intialize the PWM to 50 % duty cycle  (*64))
} 


