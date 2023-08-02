/*
 * lpm013m126a.h
 *
 *  Created on: 31 Aug 2021
 *      Author: remiaeschimann
 */

#ifndef DISPLAYS_LPM013M126A_LPM013M126A_H_
#define DISPLAYS_LPM013M126A_LPM013M126A_H_

#ifdef __cplusplus
 extern "C" {
#endif



/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "lvgl/lvgl.h"

#define LCD_EXTCOMIN								PB_2
#define LCD_CS										PA_15
#define LCD_DISP_ON_nOFF							PB_9
#define LCD_PWM										PA_8


#define NO_UPDATE_BLINK 0x00	// 00000000
#define BLINK_BLACK 0x10		// 00010000
#define BLINK_WHITE 0x18		// 00011000
#define BLINK_INVERSION 0x14	// 00010100
#define ALL_CLEAR_COMH 0x60			// 01100000
#define ALL_CLEAR_COML 0x20			// 01100000
#define DATA_UPDATE_3BIT_COMH 0xC0	// 11000000
#define DATA_UPDATE_3BIT_COML 0x80	// 10000000
#define DATA_UPDATE_1BIT 0x88	// 10001000
#define DATA_UPDATE_4BIT 0x90	// 10010000
#define NO_UPDATE_COML 0xA0			// 10100000
#define NO_UPDATE_COMH 0xE0			// 11100000

 /*********************
  *      DEFINES
  *********************/
 /** @def
  * LCD_Color SPI commands
  */
 #define LCD_COLOR_CMD_UPDATE            (0x90) //!< Update Mode (4bit Data Mode)
 #define LCD_COLOR_CMD_ALL_CLEAR         (0x20) //!< All Clear Mode
 #define LCD_COLOR_CMD_NO_UPDATE         (0x00) //!< No Update Mode
 #define LCD_COLOR_CMD_BLINKING_WHITE    (0x18) //!< Display Blinking Color Mode (White)
 #define LCD_COLOR_CMD_BLINKING_BLACK    (0x10) //!< Display Blinking Color Mode (Black)
 #define LCD_COLOR_CMD_INVERSION         (0x14) //!< Display Inversion Mode

 //defines the VCOM bit in the command word that goes to the LCD
 #define VCOM_HI 0x40
 #define VCOM_LO 0x00

#define LPM013M126A_LCD_ID             0x34

#define configUSE_MULTIPLELINESDISP 	1	// 1 to write lines block with multiple lines driver fct !not working in single line update with DMA enable!
#define configUSE_BUFFERDISP 			1 	// 1 to write lines in a buffer before sending !need to be enable if DMA is used!
#define configUSE_DMADISP				1	/*! Enable or disable DMA transmission for the display */
#define configUSE_EXTMODE				1   /*! 1 if EXTMODE is HIGH on the hardware (enable EXTCOMIN signal) */


#define LCD_CMD_UPDATE		(0x90)	// Update Mode ( 4bit Data Mode)
#define LCD_CMD_ALL_CLEAR	(0x20) 	// All Clear Mode
#define LCD_CMD_NO_UPDATE	(0x00)	// No Update Mode

/*
* Return the VDB byte index corresponding to the pixel
* relatives coordinates (x, y) in the area.
* The area is rounded to a whole screen line.
*/
#define BUFIDX( x, y )  ( ( ( x ) >> 3 ) + ( ( y ) * ( 2 + ( LV_HOR_RES_MAX >> 3 ) ) ) + 2 )

#define HW_COLUMNS 176

void lpm013m126a_Init( void );
void lpm013m126a_DeInit( void );
uint16_t lpm013m126a_GetLcdPixelWidth( void );
uint16_t lpm013m126a_GetLcdPixelHeight(void);
uint16_t lpm013m126a_ReadID(void);
void lpm013m126a_WriteLine( char* line_cmd, int line );
void lpm013m126a_WriteMultipleLines( lv_color_t* line_cmd, int firstLine, int linesNb, int linelen  );
void lpm013m126a_Cmd( uint8_t data );
void lpm013m126a_StartUpdate( void );
uint8_t lpm013m126a_StopUpdate( void );
void lpm013m126a_comInversion( void );
void lpm013m126a_writeCs(uint8_t);
uint8_t lpm013m126a_status(void);

#ifdef __cplusplus
}
#endif


#endif /* DISPLAYS_LPM013M126A_LPM013M126A_H_ */
