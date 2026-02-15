#include "ssd1306.h"
#include <string.h>

extern I2C_HandleTypeDef hi2c1;
static uint8_t SSD1306_Buffer[1024];

// Police complète 5x7
const uint8_t Font5x7[][5] = {
    {0,0,0,0,0},       // Espace (Index 0)
    {0x3E,0x51,0x49,0x45,0x3E}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 6
    {0x01,0x71,0x09,0x05,0x03}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x06,0x49,0x49,0x29,0x1E}, // 9
    {0,0,0,0,0}, // :
    {0,0,0,0,0}, // ;
    {0,0,0,0,0}, // <
    {0,0,0,0,0}, // =
    {0,0,0,0,0}, // >
    {0,0,0,0,0}, // ?
    {0,0,0,0,0}, // @
    {0x7E,0x11,0x11,0x11,0x7E}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x09,0x01}, // F
    {0x3E,0x41,0x49,0x49,0x7A}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x0C,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x3F,0x40,0x38,0x40,0x3F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x07,0x08,0x70,0x08,0x07}, // Y
    {0x61,0x51,0x49,0x45,0x43}  // Z
};

void ssd1306_WriteCommand(uint8_t byte) {
    HAL_I2C_Mem_Write(&hi2c1, SSD1306_I2C_ADDR << 1, 0x00, 1, &byte, 1, 100);
}

void ssd1306_WriteData(uint8_t* buffer, size_t buff_size) {
    HAL_I2C_Mem_Write(&hi2c1, SSD1306_I2C_ADDR << 1, 0x40, 1, buffer, buff_size, 1000);
}

void SSD1306_Init(void) {
    HAL_Delay(100);
    ssd1306_WriteCommand(0xAE);
    ssd1306_WriteCommand(0x20); ssd1306_WriteCommand(0x00);
    ssd1306_WriteCommand(0xB0);
    ssd1306_WriteCommand(0xC8);
    ssd1306_WriteCommand(0x00); ssd1306_WriteCommand(0x10);
    ssd1306_WriteCommand(0x40);
    ssd1306_WriteCommand(0x81); ssd1306_WriteCommand(0xFF);
    ssd1306_WriteCommand(0xA1);
    ssd1306_WriteCommand(0xA6);
    ssd1306_WriteCommand(0xA8); ssd1306_WriteCommand(0x3F);
    ssd1306_WriteCommand(0xA4);
    ssd1306_WriteCommand(0xD3); ssd1306_WriteCommand(0x00);
    ssd1306_WriteCommand(0xD5); ssd1306_WriteCommand(0xF0);
    ssd1306_WriteCommand(0xD9); ssd1306_WriteCommand(0x22);
    ssd1306_WriteCommand(0xDA); ssd1306_WriteCommand(0x12);
    ssd1306_WriteCommand(0xDB); ssd1306_WriteCommand(0x20);
    ssd1306_WriteCommand(0x8D); ssd1306_WriteCommand(0x14);
    ssd1306_WriteCommand(0xAF);
    SSD1306_Fill(0);
    SSD1306_UpdateScreen();
}

void SSD1306_Fill(uint8_t color) {
    memset(SSD1306_Buffer, (color ? 0xFF : 0x00), sizeof(SSD1306_Buffer));
}

void SSD1306_UpdateScreen(void) {
    for (uint8_t i = 0; i < 8; i++) {
        ssd1306_WriteCommand(0xB0 + i);
        ssd1306_WriteCommand(0x00);
        ssd1306_WriteCommand(0x10);
        ssd1306_WriteData(&SSD1306_Buffer[128 * i], 128);
    }
}

// Affiche un caractère sur une PAGE spécifique (ligne 0 à 7)
// x : colonne (0 à 127)
// page : ligne de texte (0 à 7)
void SSD1306_DrawChar(char ch, int x, int page, uint8_t color) {
    // 1. On autorise jusqu'à 'Z'
    if (ch < 32 || ch > 'Z') ch = 32; 
    
    // 2. On gère les minuscules (a-z -> A-Z)
    if (ch >= 'a' && ch <= 'z') ch -= 32; 

    int index = -1;
    if (ch >= '0' && ch <= '9') index = ch - '0' + 1;
    else if (ch >= 'A' && ch <= 'Z') index = ch - 'A' + 18;
    else if (ch == ' ') index = 0;
    
    if (index < 0) return;

    // Protection pour ne pas sortir de l'écran
    if (x > 122 || page > 7) return; 

    for (int i = 0; i < 5; i++) {
        // On copie directement la colonne de la lettre dans la mémoire
        // Le format de la police correspond exactement au format mémoire de l'écran
        uint8_t pixels = Font5x7[index][i];
        
        if (color) {
            SSD1306_Buffer[x + i + (page * 128)] = pixels;
        } else {
            SSD1306_Buffer[x + i + (page * 128)] = 0x00;
        }
    }
}

void SSD1306_WriteString(char* str, int x, int page, uint8_t color) {
    while (*str) {
        SSD1306_DrawChar(*str, x, page, color);
        x += 6; // On avance de 6 pixels
        str++;
    }
}