	MOD-IO2
	----------
	
1. Discription
 MOD-IO2 is stackable development board which adds RELAYs and GPIOs to 
any of our development boards with UEXT. If you work with any of our 
development boards with UEXT connector and you need more GPIOs and/or 
RELAY outputs you can add these by connecting MOD-IO2 to your 
development board. This board allows easy way to interface up to 2 
relays and 7 GPIOs. MOD-IO2 is stackable and addressable, what does this 
means? These boards can stack together so you can add as many input and 
outputs as you want! E.g. 2-4-6-8 etc.! MOD-IO2 has PIC16F1503 
microcontroller and the firmware is available for modification. 

The firmware might be uploaded to your MOD-IO2 via a PIC programmer or
debugger. There is a prebuilt hex in the firmware archive for easier
upgrading.

Important! MOD-IO2 is compatible with the i2c-tools package and the 
commands i2cdetect, i2cset, i2cget, i2cdump. Visit the following two
web-sites for more info:

http://www.lm-sensors.org/wiki/I2CTools
http://www.lm-sensors.org/wiki/i2cToolsDocumentation

Before continue reading:
	* This file describe operation with the version3 of the firmware.
	* MOD-IO2 works only at 100kHz (or lower) I2C speed.
	* Host must support clock stretching for proper work.
	* When reading, after sending command, STOP-START condition must be
	send, not RESTART.
	
2. I2C Commands:		
		
	* SET_TRIS(0x01):
		Define port direction of the gpio pins. Can be INPUT or OUTPUT. 
		Note that GPIO3  can be only input with pullup always turned on. 
		This is because the alternative function is #MCLR. To set gpio
		as INPUT write 1 to the coresponding bitmask, or 0 for output.
	Example:
	--------
		START | ADDRESS | W | ACK | SET_TRIS | ACK | VALUE | ACK | STOP
				
		where:
			ADDRESS = 0x23 (this is the default address)
			SET_TRIS = 0x01
			VALUE = 0b0ddddddd - the bitmask with the coresponding 
			directions. GPIO0 - bit0, GPIO1 - bit1, etc ... 
			0x7F - all input, 0x00 - all outputs)
					
	* SET_LAT(0x02):
		Set the output level of gpio. In the bitmask 1 corespond to logical
		"1", and 0 - logical "0". Again, GPIO3 can be only input, so the 
		coresponding bit in the mask will be igrored.
	Example:
	--------
		START | ADDRESS | W | ACK | SET_LAT | ACK | VALUE | ACK | STOP
		
		where:
			ADDRESS = 0x23
			SET_TRIS = 0x02
			VALUE = 0b0dddxddd	the bitmask, where bit0 is the value for 
								GPIO0, bit1 - GPIO1, etc. bit3 is 
								x - doesn't care.

	
	* GET_PORT(0x03):
		Read current level of the GPIOs. The data will be valid if the 
		GPIO is configured as INPUT. 
	Example:
	--------
		START | ADDRESS | W | ACK | GET_PORT | ACK | STOP | START | ADDRESS | R | DATA | NACK | STOP
		
		where:
			ADDRESS = 0x23 (the default address)
			GET_PORT = 0x03
			DATA = 0x0ddddddd  - the bitmask containing the GPIOs status. bit0 - GPIO0, bit1 - GPIO1, etc.
			
			
	* SET_PU(0x04):
		Turn on or off weak pullup. GPIOs must be configured as INPUTS. Note that only GPIO0 to GPIO4 have pullups, and GPIO3 is with pullup always enabled.
	Example:
	--------
		START | ADDRESS | W | ACK | SET_PU | ACK | VALUE | ACK | STOP
		
		where:
			ADDRESS = 0x21  (the default address)
			SET_PU = 0x04
			VALUE = 0b000dxddd - bit0 coresponding to GPIO0, as "1" enables the pullup and "0" - disables it.
			
		
	* GET_AN0(0x10) on GPIO0:
	  GET_AN1(0x11) on GPIO1:
	  GET_AN2(0x12) on GPIO2:
	  GET_AN6(0x13) on GPIO3:
	  GET_AN7(0x15) on GPIO5:
		Read the voltage applied on any of the GPIOs mentioned above. If GPIO is configured as output, the execution the the command will set the GPIO as input. After the applied voltage is measured, the GPIO configuration will be as INPUT. After READ the master should read 2 bytes of data: HIGH and LOW byte.
	Example:
	--------
		START | ADDRESS | W | ACK | GET_ANx | ACK | STOP | START | ADDRESS | R | DATA_L | ACK | DATA_H | NACK | STOP
		
		where:
			ADDRESS = 0x23 (the default address)
			GET_ANx = 0x10, 0x11, 0x12, 0x13 or 0x15
			DATA_L = the byte that contains the low 8 bits of the adc value
			DATA_H = the byte that contains the high bits of the adc value
			
			
	* GET_ID(0x20):
		Read the ID of the MOD-IO2. By default it must be 0x23.
	Example:
	--------
	
		START | ADDRESS | W | ACK | GET_ID | ACK | STOP | START | ADDRESS | R | ACK | DATA | NACK | STOP
		
		where:
			ADDRESS = 0x21 (the default address)
			GET_ID = 0x20
			DATA = should be 0x23
			
	* SET_REL(0x40):
		Turn on or off the two relays. The data is the state of the relays. Bit0 is the state of RELAY1, and 
		bit1 - RELAY2. If 1 is writen the relays is turned on, 0 is off. For example to turn on both relays
		you must write 0x03.
		
		Example:
	--------
	
		START | ADDRESS | W | ACK | SET_REL | ACK | DATA | ACK | STOP

3.	Examples:

3.1 Turning on the relays:
 
	i2cset –y 2 0x21 0x40 0x03,
 
	where
 
	i2cset – command for sending data;
	-y – to skip the y/n confirmation prompt;
	2 – I2C number (usually either 1 or 2);
	0x21 – board address (0x21 should be used for writing);
	0x40 – relay operations (as seen in the README.txt);
	0x03 – should be interpreted as binary 011 – turns on both relays (0x02 would turn only second relay, 0x01 only the first, 0x00 would turn both off – 0x03 again would turn them off also btw);
 
	Expected result: a specific sound would occur and relay lights would turn on.
 
3.2. Read analog inputs/outputs:
 
	i2cset –y 2 0x21 0x10,
	and then the read command
	i2cget –y 2 0x21,
 
	where
 
	0x10 is the first analog IO;
 
	The big thing here is that to read you actually have to write (“that you would read”). Read is a combination of i2cset and i2cget!
 
	Expected results: on the terminal you would receive random and changing number or 0x00 or 0x08 or 0xFF whether you have the GPIO floating or set to 0V or set to 3.3V.
 
3.3. To read the ID of MOD-IO2
 
	i2cset –y 2 0x21 0x20,
 
	i2cget –y 2 0x21,
 
	where
 
	0x20 is the ID according to chapter 2.
 
	Expected result: on the terminal you would receive 0x23.
 
3.4. Set all analog IOs in high level

	i2cset –y 2 0x21 0x01 0x01,
 
	where 
 
	0x01 according to chapter 2, SET_TRIS is used to define port directions;
	0x01 is the high level (for low level use 0x00).
 
3.5. Read all analog IOs

	i2cset –y 2 0x21 0x01
	i2cget –y 2 0x21		
			
4.	Release history:
	24 JUL 2013	-	Released version3 firmware
	23 OCT 2013	-	Revision 2 of the README
	
5. Support:
	https://www.olimex.com/
	support@olimex.com