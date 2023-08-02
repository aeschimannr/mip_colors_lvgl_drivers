/*
 * lpm013m126a.c
 *
 *  Created on: 31 Aug 2021
 *      Author: remiaeschimann
 */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "spi.h"
#include "dma.h"
#include "lpm013m126a.h"
#include "gpio.h"
#include "lv_conf.h"



// LCD commands - Note: the bits are reversed per the memory LCD data
// sheets because of the order the bits are shifted out in the SPI
// port.
#define MLCD_WR 0x80 //MLCD write line command
#define MLCD_CM 0x20 //MLCD clear memory command
#define MLCD_NO 0x00 //MLCD NOP command (used to switch VCOM)
//LCD resolution
#define MLCD_XRES 176 //pixels per horizontal line
#define MLCD_YRES 176 //pixels per vertical line
#define MLCD_BYTES_LINE MLCD_XRES / 8 //number of bytes in a line
#define MLCD_BUF_SIZE MLCD_YRES * MLCD_BYTES_LINE


//stage 0: first line (has command in it)
//stage 1: 2nd through last line (no command)
//stage 2: null byte trailer
extern char vcom; //current state of vcom. This should alternate
//between VCOM _ HI and VCOM _ LO on a 1-30 second
//period.

/* actual status of the screen
 * false screen OFF and not INIT
 * 1 screen ON and INIT
 */
static bool isScreenOn = false;

uint8_t lookup[16] = {
		0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
		0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF
};

Gpio_t LcdOnOff;
Gpio_t LcdCs;
Gpio_t LcdExtcomin;
Gpio_t lcdPwm;

static void lpm013m126a_SpiInit(void);
static void lpm013m126a_SendByte(uint8_t data);
static void lpm013m126a_sendByteLSB(uint8_t data);

static uint8_t bit_reverse(uint8_t data);

#if configUSE_BUFFERDISP
/* Counter & buffer for write the whole display to fill */
static int32_t y_fill_act = 0;
//static uint8_t my_fb[ LV_VER_RES_MAX * ( ( LV_HOR_RES_MAX / 2 ) + 4 ) ] = {0};
static uint8_t my_fb[ ((LV_VER_RES_MAX / 2) + 4) * 10 ] = {0};
#endif // configUSE_BUFFERDISP


/**
  * @brief  Initialize the lpm013m126a LCD Component.
  * @param  None
  * @retval None
  */
void lpm013m126a_Init(void)
{
	uint8_t cmd = LCD_COLOR_CMD_ALL_CLEAR;

	/* T2 Pixel memory initialization. 1ms or more initialize with M2 (all clear flag). */
	lpm013m126a_SpiInit(); // init spi for the lcd

	GpioInit(&LcdCs, LCD_CS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

	GpioWrite(&LcdCs, 1); // start com
	lpm013m126a_SendByte(cmd);
	lpm013m126a_sendByteLSB(0x00);
	GpioWrite(&LcdCs, 0); // end com
	HAL_Delay(1);

	/* T3 : Release time for internal latch circuits. 30us or more */
	GpioInit(&LcdOnOff, LCD_DISP_ON_nOFF, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
	GpioWrite(&LcdOnOff, 1);
	HAL_Delay(1);

	/* T4 : COM polarity initialization time. 30us or more */
	GpioInit(&LcdExtcomin, LCD_EXTCOMIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit(&lcdPwm, LCD_PWM, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1);
	isScreenOn = true;
}

void lpm013m126a_DeInit( void )
{
	GpioInit(&LcdExtcomin, LCD_EXTCOMIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
	GpioInit(&LcdOnOff, LCD_DISP_ON_nOFF, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
	GpioInit(&LcdCs, LCD_CS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );// SPI_CS

}

static void lpm013m126a_SpiInit( void )
{
	MX_LCD_Init();
}

static void lpm013m126a_SendByte(uint8_t data)
{
	HAL_SPI_Transmit(&lcdSpi, &data, 1, 200); // send data in MSB
}

static void lpm013m126a_sendByteLSB(uint8_t data)
{
	uint8_t temp;

	temp = bit_reverse(data);
	HAL_SPI_Transmit(&lcdSpi, &temp, 1, 200); // send data in LSB
}



/**
  * @brief  Get the LCD pixel Width.
  * @param  None
  * @retval The Lcd Pixel Width
  */
uint16_t lpm013m126a_GetLcdPixelWidth( void )
{
	return 176;
}

/**
  * @brief  Get the LCD pixel Height.
  * @param  None
  * @retval The Lcd Pixel Height
  */
uint16_t lpm013m126a_GetLcdPixelHeight(void)
{
	return 176;
}

/**
  * @brief  Get the lpm013m126a2 ID.
  * @param  None
  * @retval The lpm013m126a ID
  */
uint16_t lpm013m126a_ReadID(void)
{
  return LPM013M126A_LCD_ID;
}

/** send data packet */
void lpm013m126a_WriteLine( char* line_cmd, int line )
{
	if (!isScreenOn)
		return;

	int32_t j;

	if( ( line < 0 )||
		( line >= LV_VER_RES_MAX ) ) {
		/* out of device size */
		return;
	}

	lpm013m126a_StartUpdate();

//	lpm013m16a_Cmd( LCD_CMD_UPDATE ); // Command

	lpm013m126a_Cmd( line + 1 );       // line

	for( j = 0 ; j < (LV_HOR_RES_MAX/2) ; j++ ) {
		if( j >= (LV_HOR_RES_MAX/2) ) {
			/* out of device size */
			break;
		}
		lpm013m126a_Cmd(line_cmd[j]);        // data
	}
	for( ; j < (LV_HOR_RES_MAX/2) ; j++ ) {
		/* padding to device size */
		lpm013m126a_Cmd( 0x00 ); // TODO try without padding
	}

	lpm013m126a_Cmd( 0x00 );
	lpm013m126a_Cmd( 0x00 );

	lpm013m126a_StopUpdate();
}

#if 0
// Function to convert 8-bit color to 3-bit color
static uint8_t convertTo3BitColor(uint8_t color8Bit) {
    // Extract individual color components from the 8-bit color value
	uint8_t color4Bit = ((color8Bit & (1<<7)) >> 4) | // Extract the 3 most significant bits (Red), keep Red LSB shifted from bit-7 to bit-3
			((color8Bit & (1<<4)) >> 2) | // Extract the next 3 bits (Green), keep Green LSB shifted from bit-4 to bit-2
			(color8Bit & (1<<1)); // Extract the 2 least significant bits (Blue), // keep Blue LSB shifted from bit-1 to bit-1

    return color4Bit;
}
#endif

// Function to convert RGB565 16-bit color to 4-bit color with the desired bit arrangement
static uint8_t convertRGB565To4BitColor(lv_color_t colorRGB565) {
    // Extract the individual color components from the 16-bit RGB565 color value
	uint8_t color4Bit = ((lv_color_to8(colorRGB565) & (1<<7)) >> 4) | // keep Red LSB shifted from bit-7 to bit-3
			((lv_color_to8(colorRGB565) & (1<<4)) >> 2) | // keep Green LSB shifted from bit-4 to bit-2
			((lv_color_to8(colorRGB565) & (1<<1))) ; // keep Blue LSB shifted from bit-1 to bit-1

    return color4Bit;
}

/** send data packet */
void lpm013m126a_WriteMultipleLines( lv_color_t* line_cmd, int firstLine, int linesNb, int linelen  )
{
    int32_t j;
    uint8_t pixel = 0;

    lpm013m126a_StartUpdate();

	for (uint8_t line = firstLine; line < (firstLine + linesNb); line++)
    {
		lpm013m126a_Cmd(line + 1); // Gate line address (8 bits = 10bits - 2 dummy bits of address)

		for( j = 0 ; j < (LV_HOR_RES_MAX) ; j++ ) {

			if((j % 2) == 0) { // packt two pixel in one byte
				pixel |= (convertRGB565To4BitColor(line_cmd[j])) << 4; // Set the upper 4 bits
			} else {
				pixel |= (convertRGB565To4BitColor(line_cmd[j]) & 0x0F); // Set the lower 4 bits
				// flush the two pixel
				lpm013m126a_Cmd(pixel);
				pixel = 0;
			}
		}


        lpm013m126a_Cmd(0x00); // dummy (6bits + 2 dummy bits of address)
        line_cmd += (linelen * 2) + 2; // increment the buffer for going to the next line
    }

	lpm013m126a_Cmd(0x00); // dummy (8bits)
	lpm013m126a_Cmd(0x00); // dummy (8bits)

    lpm013m126a_StopUpdate();
}

void lpm013m126a_Cmd( uint8_t data )
{
#if !configUSE_BUFFERDISP // configUSE_BUFFERDISP == 0
#if !configUSE_DMADISP // configUSE_DMADISP == 0
	HAL_SPI_Transmit( &lcdSpi, &data, 1, 500 ); // Component is LSB first!
#else // configUSE_DMADISP == 1
	if( HAL_SPI_GetState( &lcdSpi ) == HAL_SPI_STATE_READY )
	{
#if configUSE_BUFFERDISP // configUSE_BUFFERDISP == 1
		if( HAL_SPI_Transmit_DMA( &lcdSpi, my_fb, y_fill_act) == HAL_OK )
		{
			return 1; // Succeed
		}
		else
		{
			GpioWrite(&LcdCs, 0); // end of com
			return 0;
		}
#else // configUSE_BUFFERDISP == 0

#endif // configUSE_BUFFERDISP
	}
#endif // configUSE_DMADISP
#else // configUSE_BUFFERDISP == 1
	/* Add data the the transmit buffer */
	my_fb[y_fill_act] = data;

	/* Increase counter */
	y_fill_act++;
#endif // configUSE_BUFFERDISP
}

void lpm013m126a_StartUpdate( void )
{
#if !configUSE_BUFFERDISP
	GpioWrite(&LcdCs, 1);
#else // configUSE_BUFFERDISP == 1
	/* reset buffer to 0 */
	memset(my_fb, 0, y_fill_act);
	/* Reset buffer counter to 0 */
	y_fill_act = 0;
#endif // configUSE_BUFFERDISP

#if configUSE_EXTMODE == 0
	/* send vcom command */
	if( vcom == VCOM_HI )
		vcom = VCOM_LO;
	else
		vcom = VCOM_HI;

	lpm013m126a_Cmd(LCD_CMD_UPDATE | vcom); // Command
#else // configUSE_EXTMODE == 1
	lpm013m126a_Cmd(LCD_CMD_UPDATE);
#endif // configUSE_EXTMODE
}

uint8_t lpm013m126a_StopUpdate( void )
{
#if !configUSE_BUFFERDISP // configUSE_BUFFERDISP == 0
	GpioWrite(&LcdCs, 0);

#else // configUSE_BUFFERDISP == 1
#if !configUSE_DMADISP // configUSE_DMADISP == 0
	GpioWrite(&LcdCs, 1);

	HAL_SPI_Transmit(&lcdSpi, my_fb, y_fill_act, 4000);

	GpioWrite(&LcdCs, 0);

#else // configUSE_DMADISP == 1
	GpioWrite(&LcdCs, 1);

	if( HAL_SPI_GetState(&lcdSpi ) == HAL_SPI_STATE_READY )
	{
		if( HAL_SPI_Transmit_DMA( &lcdSpi, my_fb, y_fill_act) == HAL_OK )
		{
			return 1; // Succeed
		}
		else
		{
			GpioWrite(&LcdCs, 0); // end of com
			return 0;
		}
	}
	else
	{
		return 0;
	}

#endif // configUSE_DMADISP
#endif // configUSE_BUFFERDISP
}

void lpm013m126a_comInversion( void )
{
	GpioToggle( &LcdExtcomin );
}

void lpm013m126a_writeCs(uint8_t state)
{
	GpioWrite(&LcdCs, state);
}

static uint8_t bit_reverse(uint8_t data)
{
	return lookup[data&0xF << 4] | lookup[data >> 4];
}
