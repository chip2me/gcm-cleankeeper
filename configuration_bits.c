/******************************************************************************/
/* Main Files to Include                                                      */
/******************************************************************************/

#include <htc.h>            /* HiTech General Includes */

/******************************************************************************/
/* Configuration Bits                                                         */
/*                                                                            */
/* Refer to your Hi-Tech User Manual in the PICC installation directory       */
/* /doc folder for more information on filling in configuration bits.         */
/* In addition, configuration bit mnemonics can be found in your              */
/* PICC\version\include\<processor name>.h file for your device.  For         */
/* additional information about what the hardware configurations mean in      */
/* terms of device operation, refer to the device datasheet.                  */
/*                                                                            */
/******************************************************************************/

__CONFIG(FOSC_INTOSC & WDTE_OFF & PWRTE_ON & MCLRE_ON & CP_OFF & BOREN_ON & CLKOUTEN_OFF);
//### __CONFIG(FOSC_INTOSC & WDTE_OFF & PWRTE_ON & MCLRE_ON & CP_OFF & BOREN_ON & CLKOUTEN_OFF);
//###__CONFIG(WRT_OFF & STVREN_ON & BORV_19 & LPBOR_OFF & LVP_OFF);

