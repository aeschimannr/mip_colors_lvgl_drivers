/*
 * lv_port_disp.c
 *
 *  Created on: Dec 7, 2020
 *      Author: remiaeschimann
 */




/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <string.h>
#include "spi.h"
#include "dma.h"
#include "lv_port_disp.h"
#include "gpio.h"
#include "display_private.h"
#if (LCD_TYPE_USE == LPM013M126A)
#include "lpm013m126a.h"
#endif
#if (LCD_TYPE_USE == ST7789)
#include "tft.h"
#endif
#if (LCD_TYPE_USE == SHARP_MIP)
#include "sharp_mip.h"
#endif

/**********************
 *      MACROS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_disp_drv_t * disp_p;
static uint8_t lcd_cmd_line = 1;


/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_port_flush( lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p );
#if 0
static void lv_port_set_px(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa);
static void lv_port_rounder_cb( lv_disp_drv_t * disp_drv, lv_area_t * area );
#endif
/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init( void )
{

#if (LCD_TYPE_USE == ST7789)
		tft_init();
		lv_disp_set_rotation(lv_disp_get_default(), LV_DISP_ROT_180);
#endif // ST7789

#if (LCD_TYPE_USE == LPM013M126A)
	/* init low level display */
	lpm013m126a_Init();
#endif


	/* LVGL will draw the display's content here and writes it to your display */
	static lv_disp_draw_buf_t disp_buf;

	static lv_color_t buf1[ LV_VER_RES_MAX * 10 ];
	static lv_color_t buf2[ LV_VER_RES_MAX * 10 ];


    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/
	/*Initialize `disp_buf` with the buffer(s) */
    lv_disp_draw_buf_init( &disp_buf, buf1, buf2, LV_VER_RES_MAX * 10 );


    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/
    /*Descriptor of a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;


    /*Used to copy the buffer's content to the display*/
    //    disp_drv.flush_cb = jdi_mip_flush;
    disp_drv.flush_cb = lv_port_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &disp_buf;
#if LV_USE_GPU
    /*Optionally add functions to access the GPU. (Only in buffered mode, LV_VDB_SIZE != 0)*/

    /*Blend two color array using opacity*/
    disp_drv.gpu_blend_cb = gpu_blend;

    /*Fill a memory array with a color*/
    disp_drv.gpu_fill_cb = gpu_fill;
#endif // LV_USE_GPU

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Flush a color buffer
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color_p pointer to an array of colors
 */
/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
static void lv_port_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
	int row = area->y1; // select the row
	int address = (unsigned int)row + area->x1 / 2; // select the address in the column
	uint16_t *buffer;

	int linelen = (area->x2 - area->x1) / 2;

	buffer = (uint16_t *)color_p;

#if configUSE_DMADISP
//	vUlpOnPeripheralsActive(ulpPERIPHERAL_SPI);
#endif // configUSE_DMADISP
#if configUSE_MULTIPLELINESDISP
	lpm013m126a_WriteMultipleLines(color_p, address, (area->y2 - area->y1 + 1), linelen);
#else
	for( row = area->y1; row <= area->y2; row++ ) // stay inside the row that we need to refresh
	{
		lpm013m126a_WriteLine(buffer, address);
		buffer += linelen+1;
		address += lcd_cmd_line;
	}
#endif // configUSE_MULTIPLELINESDISP

#if configUSE_DMADISP
	/* flush the entire buffer */
    disp_p = disp_drv; // copy the pointer disp_drv for use it in txclptcallback
#else // !configUSE_DMADISP

    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
#endif // configUSE_DMADISP
}

/**
  * @brief  TxRx Transfer completed callback.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report end of DMA TxRx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance == LCD_SPI_HANDLE)
	{
		lpm013m126a_writeCs(0); // end of com

		lv_disp_flush_ready(disp_p); // tell lvgl that the flush is finish
#if configUSE_DMADISP
//		vUlpOnPeripheralsInactiveFromISR(ulpPERIPHERAL_SPI);
#endif // configUSE_DMADISP
	}
}


/****************************************************************************
 * !
 	 	 @brief		Sends a single byte in SPI.

 	 	 !! Much slower then drawing with supported color formats. !!
*/
/***************************************************************************/
#if 0
static void lv_port_set_px(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa)
{

	buf += buf_w/2 * y;
	buf += x/2;

	uint8_t color4Bit = ((lv_color_to8(color) & (1<<7)) >> 4) | // keep Red LSB shifted from bit-7 to bit-3
			((lv_color_to8(color) & (1<<4)) >> 2) | // keep Green LSB shifted from bit-4 to bit-2
			((lv_color_to8(color) & (1<<1))) ; // keep Blue LSB shifted from bit-1 to bit-1

	if((x % 2) == 0)
	{
		(*buf) &= 0x0F;
		(*buf) |= (color4Bit << 4); // set color bits to 4 MSB

	}
	else
	{
		(*buf) &= 0xF0;
		(*buf) |= (color4Bit); // set color bits to 4 LSB

	}
}

static void lv_port_rounder_cb( lv_disp_drv_t * disp_drv, lv_area_t * area )
{
	/* Round area to a whole line */
	area->x1 = 0;
	area->x2 = LV_HOR_RES_MAX - 1;

}
#endif



