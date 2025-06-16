#pragma once

#include "esp_err.h"

#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL 1
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL

#define LCD_H_RES 320
#define LCD_V_RES 240
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

#define LCD_DRAW_BUFF_HEIGHT (50)
#define LCD_DRAW_BUFF_DOUBLE (1)

esp_err_t screen_init(void);
