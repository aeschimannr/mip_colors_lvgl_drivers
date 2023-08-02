## Introduction

Full project example to run Freertos with LVGL v8 GUI generated from SquareLineStudio and Memory In Pixel Displays produced by Japan Display Inc.

The library should also work with the Memory In Pixel Displays produced by Sharp (such as LS027B7DH01 or LS044Q7DH01) because they seem to share the same protocol. The only difference with the JDI Displays is that the latter are 8 bit colors while the Sharp ones are monochrome.

Please note that the library has been tested only with the JDI display model LPM013M126A as well as the models listed in Other supported displays section.

Reflective displays are readable under direct sunlight and require less power because in bright environments they don't need a backlight! However if you also want to use them in dark environments you may need a display with a backlight panel (like the model LPM027M128C or LPM013M126C).

There is also a different kind of display lighting produced by Azumo: they apply a special thin and transparent film to the front. Check out all the Azumo displays. Some of them are based on Sharp display, others on JDI display. This is the exact display which I bought and used to test the library.

## Installation
Import the project in stmcubeide adapt the low level hardware for your MCU (actual STM32L4P5CGU6). Adapt your pin assignement in for the SPI in spi.c file or with stm32cubeMX. 
flash!

## additional information
MIP reflective color display(Japan Display Inc.) product information.
https://os.mbed.com/teams/JapanDisplayInc/wiki/MIP-reflective-color-display
