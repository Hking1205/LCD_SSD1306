/*
 * ssd1306.c
 *
 *  Created on: Jan 18, 2026
 *      Author: H
 */

#include "ssd1306.h"

/* ---- Internal state ---- */
static I2C_HandleTypeDef *g_hi2c = NULL;

/* 1 bit per pixel => 128*64/8 = 1024 bytes */
static uint8_t g_buf[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

static uint8_t g_curX = 0;
static uint8_t g_curY = 0;
static uint8_t g_textScale = 1;


/* ---- Tiny 5x7 font (ASCII 32..127) ----
   Minimal font table: enough for debug text.
   If you want full font, tell me and I’ll paste the full 96-char table.
*/
static const uint8_t font5x7[][5] = {
    /* ASCII 32 ' ' */
    {0x00,0x00,0x00,0x00,0x00}, /* space */
    {0x00,0x00,0x5F,0x00,0x00}, /* ! */
    {0x00,0x07,0x00,0x07,0x00}, /* " */
    {0x14,0x7F,0x14,0x7F,0x14}, /* # */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* $ */
    {0x23,0x13,0x08,0x64,0x62}, /* % */
    {0x36,0x49,0x55,0x22,0x50}, /* & */
    {0x00,0x05,0x03,0x00,0x00}, /* ' */
    {0x00,0x1C,0x22,0x41,0x00}, /* ( */
    {0x00,0x41,0x22,0x1C,0x00}, /* ) */
    {0x14,0x08,0x3E,0x08,0x14}, /* * */
    {0x08,0x08,0x3E,0x08,0x08}, /* + */
    {0x00,0x50,0x30,0x00,0x00}, /* , */
    {0x08,0x08,0x08,0x08,0x08}, /* - */
    {0x00,0x60,0x60,0x00,0x00}, /* . */
    {0x20,0x10,0x08,0x04,0x02}, /* / */
    {0x3E,0x51,0x49,0x45,0x3E}, /* 0 */
    {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */
    {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10}, /* 4 */
    {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30}, /* 6 */
    {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */
    {0x06,0x49,0x49,0x29,0x1E}, /* 9 */
    {0x00,0x36,0x36,0x00,0x00}, /* : */
    {0x00,0x56,0x36,0x00,0x00}, /* ; */
    {0x08,0x14,0x22,0x41,0x00}, /* < */
    {0x14,0x14,0x14,0x14,0x14}, /* = */
    {0x00,0x41,0x22,0x14,0x08}, /* > */
    {0x02,0x01,0x51,0x09,0x06}, /* ? */
    {0x32,0x49,0x79,0x41,0x3E}, /* @ */
    {0x7E,0x11,0x11,0x11,0x7E}, /* A */
    {0x7F,0x49,0x49,0x49,0x36}, /* B */
    {0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C}, /* D */
    {0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x7F,0x09,0x09,0x09,0x01}, /* F */
    {0x3E,0x41,0x49,0x49,0x7A}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F}, /* H */
    {0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01}, /* J */
    {0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x7F,0x40,0x40,0x40,0x40}, /* L */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F}, /* N */
    {0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x7F,0x09,0x09,0x09,0x06}, /* P */
    {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46}, /* R */
    {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7F,0x01,0x01}, /* T */
    {0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F}, /* V */
    {0x3F,0x40,0x38,0x40,0x3F}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */
    {0x07,0x08,0x70,0x08,0x07}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}, /* Z */
    /* ... (table truncated intentionally) ... */
};

/* ---- Low-level I2C write helpers ---- */
static void ssd1306_write_cmd(uint8_t cmd)
{
    /* Control byte 0x00 => Co=0, D/C#=0 (command) */
    uint8_t data[2] = {0x00, cmd};
    HAL_I2C_Master_Transmit(g_hi2c, SSD1306_I2C_ADDR, data, 2, 100);
}

static void ssd1306_write_data(uint8_t *bytes, size_t len)
{
    /* Control byte 0x40 => Co=0, D/C#=1 (data) */
    /* We send in chunks: [0x40][payload...] */
    uint8_t tmp[17]; /* 1 control + 16 data */
    tmp[0] = 0x40;

    while (len)
    {
        size_t n = (len > 16) ? 16 : len;
        for (size_t i = 0; i < n; i++) tmp[1 + i] = bytes[i];

        HAL_I2C_Master_Transmit(g_hi2c, SSD1306_I2C_ADDR, tmp, (uint16_t)(1 + n), 100);

        bytes += n;
        len   -= n;
    }
}

/* ---- Public API ---- */
void SSD1306_Init(I2C_HandleTypeDef *hi2c)
{
    g_hi2c = hi2c;

    /* SSD1306 init sequence (128x64, page addressing) */
    ssd1306_write_cmd(0xAE); // Display OFF
    ssd1306_write_cmd(0xD5); ssd1306_write_cmd(0x80); // clock
    ssd1306_write_cmd(0xA8); ssd1306_write_cmd(0x3F); // multiplex 1/64
    ssd1306_write_cmd(0xD3); ssd1306_write_cmd(0x00); // display offset
    ssd1306_write_cmd(0x40); // start line 0
    ssd1306_write_cmd(0x8D); ssd1306_write_cmd(0x14); // charge pump ON
    ssd1306_write_cmd(0x20); ssd1306_write_cmd(0x00); // memory mode = horizontal
    ssd1306_write_cmd(0xA1); // segment remap (mirror X) - common modules
    ssd1306_write_cmd(0xC8); // COM scan dec (mirror Y)
    ssd1306_write_cmd(0xDA); ssd1306_write_cmd(0x12); // COM pins
    ssd1306_write_cmd(0x81); ssd1306_write_cmd(0x7F); // contrast
    ssd1306_write_cmd(0xD9); ssd1306_write_cmd(0xF1); // pre-charge
    ssd1306_write_cmd(0xDB); ssd1306_write_cmd(0x40); // VCOM detect
    ssd1306_write_cmd(0xA4); // resume RAM content display
    ssd1306_write_cmd(0xA6); // normal (not inverted)
    ssd1306_write_cmd(0xAF); // Display ON

    SSD1306_Fill(SSD1306_COLOR_BLACK);
    SSD1306_UpdateScreen();
    SSD1306_SetCursor(0, 0);
}

void SSD1306_Fill(ssd1306_color_t color)
{
    for (size_t i = 0; i < sizeof(g_buf); i++)
        g_buf[i] = (color == SSD1306_COLOR_WHITE) ? 0xFF : 0x00;
}

void SSD1306_UpdateScreen(void)
{
    /* Set column + page ranges */
    ssd1306_write_cmd(0x21); // COL addr
    ssd1306_write_cmd(0x00);
    ssd1306_write_cmd(SSD1306_WIDTH - 1);

    ssd1306_write_cmd(0x22); // PAGE addr
    ssd1306_write_cmd(0x00);
    ssd1306_write_cmd((SSD1306_HEIGHT / 8) - 1);

    ssd1306_write_data(g_buf, sizeof(g_buf));
}

void SSD1306_SetCursor(uint8_t x, uint8_t y)
{
    g_curX = x;
    g_curY = y;
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, ssd1306_color_t color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;

    if (color == SSD1306_COLOR_WHITE)
        g_buf[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    else
        g_buf[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
}

void SSD1306_SetTextSize(uint8_t scale)
{
    if (scale < 1)  scale = 1;
    if (scale > 10) scale = 10;
    g_textScale = scale;
}

/* Draw ONE scaled character at x,y (uses your existing font5x7 table) */
static void ssd1306_drawCharScaled(uint8_t x, uint8_t y, char ch, ssd1306_color_t color, uint8_t scale)
{
    if (ch < 32 || ch > 90) ch = '?';   // adjust if your font table is bigger
    uint8_t idx = (uint8_t)(ch - 32);

    for (uint8_t col = 0; col < 5; col++)
    {
        uint8_t line = font5x7[idx][col];

        for (uint8_t row = 0; row < 7; row++)
        {
            if (line & 0x01)
            {
                for (uint8_t dx = 0; dx < scale; dx++)
                {
                    for (uint8_t dy = 0; dy < scale; dy++)
                    {
                        SSD1306_DrawPixel(x + col * scale + dx,
                                          y + row * scale + dy,
                                          color);
                    }
                }
            }
            line >>= 1;
        }
    }
}

void SSD1306_WriteStringSize(uint8_t x, uint8_t y, const char *str, ssd1306_color_t color)
{
    uint8_t cx = x;
    uint8_t cy = y;
    uint8_t s  = g_textScale;

    while (*str)
    {
        /* Simple wrap: if next char would exceed width, go to next line */
        if ((uint16_t)cx + (uint16_t)(6 * s) >= SSD1306_WIDTH)
        {
            cx = x;
            cy = (uint8_t)(cy + (uint8_t)(8 * s));
            if ((uint16_t)cy + (uint16_t)(8 * s) >= SSD1306_HEIGHT) break;
        }

        ssd1306_drawCharScaled(cx, cy, *str, color, s);
        cx = (uint8_t)(cx + (uint8_t)(6 * s)); // 5 cols + 1 space, scaled

        str++;
    }
}


uint8_t SSD1306_WriteChar(char ch, ssd1306_color_t color)
{
    if (ch < 32 || ch > 90) ch = '?';   // because our mini table is limited here
    uint16_t idx = (uint16_t)(ch - 32);

    if (g_curX + 6 >= SSD1306_WIDTH) return 0;
    if (g_curY + 8 >= SSD1306_HEIGHT) return 0;

    for (uint8_t col = 0; col < 5; col++)
    {
        uint8_t line = font5x7[idx][col];
        for (uint8_t row = 0; row < 8; row++)
        {
            if (line & 0x01) SSD1306_DrawPixel(g_curX + col, g_curY + row, color);
            else             SSD1306_DrawPixel(g_curX + col, g_curY + row, (ssd1306_color_t)!color);
            line >>= 1;
        }
    }

    /* 1 column space */
    for (uint8_t row = 0; row < 8; row++)
        SSD1306_DrawPixel(g_curX + 5, g_curY + row, (ssd1306_color_t)!color);

    g_curX += 6;
    return 1;
}

uint8_t SSD1306_WriteString(const char* str, ssd1306_color_t color)
{
    while (*str)
    {
        if (!SSD1306_WriteChar(*str, color)) return 0;
        str++;
    }
    return 1;
}

