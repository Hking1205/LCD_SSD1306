/*
 * oled_ui.c
 *
 *  Created on: Jan 18, 2026
 *      Author: H
 */
#include "oled_ui.h"
#include "ssd1306.h"
#include "main.h"

/* CubeMX I2C handle */
extern I2C_HandleTypeDef hi2c1;

/* Internal state */
static uint8_t  s_boot_done = 0;
static uint32_t s_last_ms   = 0;

/* ---- HELLO screen (centered, big) ---- */
static void OLED_DrawHello(void)
{
    const uint8_t scale = 4;

    /* Calculated centering */
    const uint8_t text_width  = 5 * 6 * scale;   // "HELLO"
    const uint8_t text_height = 7 * scale;

    const uint8_t x = (SSD1306_WIDTH  - text_width)  / 2;
    const uint8_t y = (SSD1306_HEIGHT - text_height) / 2;

    SSD1306_Fill(SSD1306_COLOR_BLACK);

    SSD1306_SetTextSize(scale);
    SSD1306_WriteStringSize(x, y, "HELLO", SSD1306_COLOR_WHITE);

    SSD1306_UpdateScreen();

    /* Restore normal text size for dashboard */
    SSD1306_SetTextSize(1);
}

/* ---- Dashboard (labels only) ---- */
static void OLED_DrawDashboard(void)
{
    SSD1306_Fill(SSD1306_COLOR_BLACK);

    SSD1306_SetCursor(0, 0);
    SSD1306_WriteString("MODE_REF :", SSD1306_COLOR_WHITE);

    SSD1306_SetCursor(0, 16);
    SSD1306_WriteString("ALTITUDE :", SSD1306_COLOR_WHITE);

    SSD1306_SetCursor(0, 32);
    SSD1306_WriteString("THROTTLE :", SSD1306_COLOR_WHITE);

    SSD1306_SetCursor(0, 48);
    SSD1306_WriteString("BATTERY  :", SSD1306_COLOR_WHITE);

    SSD1306_UpdateScreen();
}

void OLED_UI_Init(void)
{
    SSD1306_Init(&hi2c1);

    /* Boot screen */
    OLED_DrawHello();
    HAL_Delay(1500);

    /* Draw dashboard immediately after */
    OLED_DrawDashboard();
    s_boot_done = 1;

    s_last_ms = HAL_GetTick();
}

void OLED_UI_Update(void)
{
    if (!s_boot_done) return;

    /* Refresh at 2 Hz */
    uint32_t now = HAL_GetTick();
    if ((now - s_last_ms) < 500) return;
    s_last_ms = now;

    OLED_DrawDashboard();
}



