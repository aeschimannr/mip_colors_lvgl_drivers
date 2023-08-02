/*
 * lv_port_disp.h
 *
 *  Created on: Dec 7, 2020
 *      Author: remiaeschimann
 */

#ifndef LPM013M126A_LV_PORT_DISP_H_
#define LPM013M126A_LV_PORT_DISP_H_


#include "lvgl/lvgl.h"

#define LCD_SPI_HANDLE 		SPI1


#define JDI_MIP_VER_RES			176
#define JDI_MIP_HOR_RES			176

#define JDI_MIP_LINE_SIZE			( JDI_MIP_HOR_RES / 2 )
#define JDI_MIP_FRAMEBUFFER_SIZE	( JDI_MIP_VER_RES * LCD_LINE_SIZE )



void lv_port_disp_init(void);


#endif /* LPM013M126A_LV_PORT_DISP_H_ */
