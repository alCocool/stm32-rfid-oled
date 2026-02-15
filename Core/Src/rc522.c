#include "rc522.h"
#include <stdio.h>

extern SPI_HandleTypeDef hspi1;

#define MFRC522_CS_LOW  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define MFRC522_CS_HIGH HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

// Commandes MFRC522
#define PCD_IDLE       0x00
#define PCD_AUTHENT    0x0E
#define PCD_RECEIVE    0x08
#define PCD_TRANSMIT   0x04
#define PCD_TRANSCEIVE 0x0C
#define PCD_RESETPHASE 0x0F
#define PCD_CALCCRC    0x03

#define PICC_REQIDL    0x26
#define PICC_ANTICOLL  0x93

void MFRC522_WriteRegister(uint8_t addr, uint8_t val) {
    uint8_t data[2] = {(addr << 1) & 0x7E, val};
    MFRC522_CS_LOW;
    HAL_SPI_Transmit(&hspi1, data, 2, 10);
    MFRC522_CS_HIGH;
}

uint8_t MFRC522_ReadRegister(uint8_t addr) {
    uint8_t val;
    uint8_t data = ((addr << 1) & 0x7E) | 0x80;
    MFRC522_CS_LOW;
    HAL_SPI_Transmit(&hspi1, &data, 1, 10);
    HAL_SPI_Receive(&hspi1, &val, 1, 10);
    MFRC522_CS_HIGH;
    return val;
}

void MFRC522_SetBitMask(uint8_t reg, uint8_t mask) {
    MFRC522_WriteRegister(reg, MFRC522_ReadRegister(reg) | mask);
}

void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask) {
    MFRC522_WriteRegister(reg, MFRC522_ReadRegister(reg) & (~mask));
}

void MFRC522_Init(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // Reset High
    HAL_Delay(10);
    MFRC522_WriteRegister(0x01, 0x0F); // Soft Reset
    HAL_Delay(10);
    
    MFRC522_WriteRegister(0x2A, 0x8D); // Timer: TPrescaler
    MFRC522_WriteRegister(0x2B, 0x3E); // Timer: TPrescaler
    MFRC522_WriteRegister(0x2D, 30);   // Timer: Reload
    MFRC522_WriteRegister(0x2C, 0);    // Timer: Reload
    MFRC522_WriteRegister(0x15, 0x40); // TxAuto=1
    MFRC522_WriteRegister(0x11, 0x3D); // Mode
    MFRC522_SetBitMask(0x14, 0x03);    // TxControl: Antenna On
}

uint8_t MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint16_t *backLen) {
    uint8_t status = MI_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint16_t i;

    if (command == PCD_AUTHENT) {
        irqEn = 0x12;
        waitIRq = 0x10;
    } else if (command == PCD_TRANSCEIVE) {
        irqEn = 0x77;
        waitIRq = 0x30;
    }

    MFRC522_WriteRegister(0x02, irqEn | 0x80);
    MFRC522_ClearBitMask(0x04, 0x80);
    MFRC522_SetBitMask(0x0A, 0x80);
    MFRC522_WriteRegister(0x01, PCD_IDLE);

    for (i = 0; i < sendLen; i++) MFRC522_WriteRegister(0x09, sendData[i]);

    MFRC522_WriteRegister(0x01, command);
    if (command == PCD_TRANSCEIVE) MFRC522_SetBitMask(0x0D, 0x80);

    i = 2000;
    do {
        n = MFRC522_ReadRegister(0x04);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    MFRC522_ClearBitMask(0x0D, 0x80);

    if (i != 0) {
        if (!(MFRC522_ReadRegister(0x06) & 0x1B)) {
            status = MI_OK;
            if (n & irqEn & 0x01) status = MI_NOTAGERR;
            if (command == PCD_TRANSCEIVE) {
                n = MFRC522_ReadRegister(0x0A);
                lastBits = MFRC522_ReadRegister(0x0C) & 0x07;
                if (lastBits) *backLen = (n - 1) * 8 + lastBits;
                else *backLen = n * 8;
                if (n == 0) n = 1;
                if (n > 16) n = 16;
                for (i = 0; i < n; i++) backData[i] = MFRC522_ReadRegister(0x09);
            }
        } else status = MI_ERR;
    }
    return status;
}

uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType) {
    uint8_t status;
    uint16_t backBits;
    MFRC522_WriteRegister(0x0D, 0x07);
    TagType[0] = reqMode;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
    if ((status != MI_OK) || (backBits != 0x10)) status = MI_ERR;
    return status;
}

uint8_t MFRC522_Anticoll(uint8_t *serNum) {
    uint8_t status;
    uint8_t i;
    uint8_t serNumCheck = 0;
    uint16_t unLen;

    MFRC522_WriteRegister(0x0D, 0x00);
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK) {
        for (i = 0; i < 4; i++) serNumCheck ^= serNum[i];
        if (serNumCheck != serNum[4]) status = MI_ERR;
    }
    return status;
}