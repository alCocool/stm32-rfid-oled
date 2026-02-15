#ifndef SSD1306_H
#define SSD1306_H

#include "main.h"
#define SSD1306_I2C_ADDR 0x3C 

void SSD1306_Init(void);
void SSD1306_Fill(uint8_t color);
void SSD1306_UpdateScreen(void);
void SSD1306_WriteString(char* str, int x, int page, uint8_t color);

#endif