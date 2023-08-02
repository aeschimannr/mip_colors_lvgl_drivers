/**
 * @file SHARP_MIP.h
 *
 */

#ifndef SHARP_MIP_H
#define SHARP_MIP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include "cmsis_gcc.h"
#include "main.h"
#include "spi.h"

/*---------
 *  SPI
 *---------*/
#define LV_DRV_DISP_SPI_CS(val) 	    	HAL_GPIO_WritePin(LCD_nSS_SPI3_GPIO_Port, LCD_nSS_SPI3_Pin, val)  /*spi_cs_set(val)*/     /*Set the SPI's Chip select to 'val'*/
#define LV_DRV_DISP_SPI_WR_BYTE(data) 		HAL_SPI_Transmit(&hspi3, &data, 1, 500)    /*spi_wr(data)*/        /*Write a byte the SPI bus*/
#if configUSE_DMADISP
#define LV_DRV_DISP_SPI_WR_ARRAY(adr, n)	HAL_SPI_Transmit_DMA(&hspi3, adr, n) 	/*spi_wr_mem(adr, n)*/  /*Write 'n' bytes to SPI bus from 'adr'*/
#else
#define LV_DRV_DISP_SPI_WR_ARRAY(adr, n)	HAL_SPI_Transmit(&hspi3, adr, n, 500) 	/*spi_wr_mem(adr, n)*/  /*Write 'n' bytes to SPI bus from 'adr'*/
#endif

#  define SHARP_MIP_HOR_RES             176
#  define SHARP_MIP_VER_RES             176
#  define SHARP_MIP_SOFT_COM_INVERSION  0
#  define SHARP_MIP_REV_BYTE(b)         ((uint8_t) __REV(__RBIT(b)))  /*Architecture / compiler dependent byte bits order reverse*/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif


/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void sharp_mip_init(void);
void sharp_mip_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
void sharp_mip_rounder(lv_disp_drv_t * disp_drv, lv_area_t * area);
void sharp_mip_set_px(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa);
#if SHARP_MIP_SOFT_COM_INVERSION
void sharp_mip_com_inversion(void);
#endif

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SHARP_MIP_H */
