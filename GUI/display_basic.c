/*
 * display_basic.c
 *
 *  Created on: 7 Jan 2021
 *      Author: remiaeschimann
 */

#include "display_private.h"
#include "lv_port_disp.h"

void Display_Init( void )
{
	/* Init the lvgl library */
	lv_init();

	/* Init the porting lvgl to lcd */
	lv_port_disp_init();
}
