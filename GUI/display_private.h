/*
 * display_private.h
 *
 *  Created on: 29 Dec 2020
 *      Author: remiaeschimann
 */

#ifndef GUI_DISPLAY_PRIVATE_H_
#define GUI_DISPLAY_PRIVATE_H_

#include <stdint.h>
#include <stdio.h>

/*!
 * Defines the id lcd
 */
#define LPM013M126A 					0
#define ST7789H2						1
#define ST7789							2
#define SHARP_MIP						3



#define LCD_TYPE_USE 					LPM013M126A


void Display_Init( void );

#endif /* GUI_DISPLAY_PRIVATE_H_ */
