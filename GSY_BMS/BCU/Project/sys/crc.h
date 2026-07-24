
#ifndef __CRC_H_
#define __CRC_H_

#include "stdbool.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"


uint16_t CRC16 (uint8_t *puchMsg, uint16_t usDataLen);
bool CrcJudge(uint8_t *p, uint16_t len);
    
#endif









