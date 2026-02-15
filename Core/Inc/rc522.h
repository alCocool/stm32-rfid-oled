#ifndef RC522_H
#define RC522_H

#include "main.h"

#define MAX_LEN 16 

// Codes de retour
#define MI_OK 0
#define MI_NOTAGERR 1
#define MI_ERR 2

// Commandes RFID
#define PICC_REQIDL 0x26

void MFRC522_Init(void);
uint8_t MFRC522_ReadRegister(uint8_t addr);
uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType);
uint8_t MFRC522_Anticoll(uint8_t *serNum);

#endif