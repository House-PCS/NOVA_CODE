/**********************************************************************
* Copyright: ShenZhen TOPBAND CO.LTD
* File Name: public_proto.h
* Abstract : Public Func for Protocol
* Remark   : None
* Version  : V1.0
* Author   : sophy 
* Data     : 2017.9.27 
*
* Update   : None
* Date     : None
* Version  : None
* Author   : None
* Abstract : None
************************************************************************/
#ifndef _PROTO_H
#define _PROTO_H

#include "bsp.h"
/*
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
*/

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
*                                 Head Files                            *
************************************************************************/	
#include <string.h>
	
/************************************************************************
*                                Macro Definition                       *
************************************************************************/	
#define USING_MODE_BIG     1
#define USING_MODE_SMALL   0

/************************************************************************
*                         Global Function Definition                    *
************************************************************************/
/************************************************************************
@ Function Name: DataToUnshort
@ Description  : u8 to u16
@ Input    Para: Buf, Mode Flag
@ Output   Para: None
@ Return  Value: u16
@ Author   Data: Sophy & 2017.8.23
*************************************************************************/
uint16_t DataToUnshort(uint8_t *pBuffer, uint8_t uModeFlag);
/************************************************************************
@ Function Name: DataToUnchar
@ Description  : u16 to u8
@ Input    Para: Buf, u16, Mode Flag
@ Output   Para: None
@ Return  Value: None
@ Author   Data: Sophy & 2017.8.23
*************************************************************************/
void DataToUnchar(uint8_t *pBuffer, uint16_t uShort, uint8_t uModeFlag);
/************************************************************************
@ Function Name: DataToUnint
@ Description  : u8 to u32
@ Input    Para: Buf, Mode Flag
@ Output   Para: None
@ Return  Value: u32
@ Author   Data: Sophy & 2017.8.23
*************************************************************************/
uint32_t DataToUnint(uint8_t *pBuffer, uint8_t uModeFlag);
/************************************************************************
@ Function Name: UintToUnchar
@ Description  : u32 to u8
@ Input    Para: Buf, u32, Mode Flag
@ Output   Para: None
@ Return  Value: None
@ Author   Data: Sophy & 2017.8.23
*************************************************************************/
void UintToUnchar(uint8_t *pBuffer, uint32_t uInt, uint8_t ModeFlag);
/************************************************************************
@ Function Name: CalCrcShort
@ Description  : Calculate CRC16
@ Input    Para: Frame, Len
@ Output   Para: None
@ Return  Value: Crc16
@ Author   Data: Sophy & 2017.8.23
*************************************************************************/
uint16_t CalCrcShort(uint8_t *pFrame, uint16_t uLen);
/************************************************************************
@ Function Name: DataModeChange
@ Description  : u8 to u8
@ Input    Para: Buf, Buf, Mode Flag, Len
@ Output   Para: None
@ Return  Value: None
@ Author   Data: Sophy & 2017.8.23
*************************************************************************/
void DataModeChange(uint8_t *pBuffer, uint8_t *pData, uint8_t uModeFlag, uint8_t uLen);
/************************************************************************
@ Function Name: CalCrcValue
@ Description  : Calculate CRC32
@ Input    Para: Frame, Len
@ Output   Para: None
@ Return  Value: Crc32
@ Author   Data: Sophy & 2017.8.23
*************************************************************************/
uint32_t CalCrcValue(uint8_t *pFrame, uint32_t uLen);

#ifdef __cplusplus
}
#endif

#endif

