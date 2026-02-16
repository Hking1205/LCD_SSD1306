/*
 * ssd1306.h
 *
 *  Created on: Jan 18, 2026
 *      Author: H
 */

#ifndef __SSD1306_H__
#define __SSD1306_H__

#include "stm32f3xx_hal.h"   // <-- change to your family header if needed
#include <stdint.h>
#include <stddef.h>

/* -------- Display config -------- */
#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  64

/* Common I2C addresses: 0x3C or 0x3D (7-bit) */
#define SSD1306_I2C_ADDR  (0x3C << 1)   // HAL uses 8-bit address

typedef enum {
    SSD1306_COLOR_BLACK = 0,
    SSD1306_COLOR_WHITE = 1
} ssd1306_color_t;

/* Public API */
void SSD1306_Init(I2C_HandleTypeDef *hi2c);
void SSD1306_Fill(ssd1306_color_t color);
void SSD1306_UpdateScreen(void);

void SSD1306_SetCursor(uint8_t x, uint8_t y);
void SSD1306_DrawPixel(uint8_t x, uint8_t y, ssd1306_color_t color);

/* ---- Text scaling API ---- */
void SSD1306_SetTextSize(uint8_t scale);  // 1 = normal, 2..10 = bigger
void SSD1306_WriteStringSize(uint8_t x, uint8_t y, const char *str, ssd1306_color_t color);

/* Simple text (5x7) */
uint8_t SSD1306_WriteChar(char ch, ssd1306_color_t color);
uint8_t SSD1306_WriteString(const char* str, ssd1306_color_t color);

#endif
