
#ifndef __DEV_LCD_H_
#define __DEV_LCD_H_


#include "bsp.h"
#include "queue.h"




static void DevIli9486Init(void);
void DevLcdRst(void);
void DevLcdBLClose(void);
void DevLcdBLOpen(void);

void DevLcdWriteReg(uint16_t reg);
void DevLcdWriteData(uint16_t data);
uint16_t DevLcdReadData(void);
void DevLcdWriteMultiData(uint16_t * pData, int num);
void DevLcdReadMultiData(uint16_t * pData, int num);
void DevLcdSetCursor(uint16_t x_pos, uint16_t y_pos);
void DevLcdWriteRAMCmd(void);
void DevLcdReadRAMCmd(void);

void DevLcdInit(void);

#endif








