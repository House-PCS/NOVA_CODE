/**********************************************************************
* Copyright: ShenZhen TOPBAND CO.LTD
* File Name: modbus_rtu.h
* Abstract : Modbus RTU
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
#ifndef _MODBUS_RTU_H
#define _MODBUS_RTU_H


#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
*                                 Head Files                            *
************************************************************************/
#include "public_proto.h"	
	
/************************************************************************
*                                Macro Definition                       *
************************************************************************/	
#define MBRTU_FC_READ_STANDREG		   0x03			/* 读保持寄存器功能码	  */
#define MBRTU_FC_READ_ORREG			     0x04			/* 读只读寄存器功能码	  */
#define MBRTU_FC_SWRITE_STANDREG	   0x06			/* 写单个寄存器功能码	  */
#define MBRTU_FC_WRITE_STANDREG		   0x10			/* 写保持寄存器功能码	  */
#define MBRTU_FC_READ_STANDREG_ERR	 0x83			/* 读保持寄存器异常功能码 */
#define MBRTU_FC_READ_ORREG_ERR		   0x84			/* 读只读寄存器异常功能码 */
#define MBRTU_FC_SWRITE_STANDREG_ERR 0x86     /* 写单个寄存器异常功能码 */
#define MBRTU_FC_WRITE_STANDREG_ERR	 0x90			/* 写保持寄存器异常功能码 */

#define INVALID_FUNC   1
#define INVALID_ADDR   2
#define INVALID_DATA   3
#define INVALID_OPER   4

#define MBRTU_OK                     0x5000   /* OK */
#define MBRTU_FC_ERR                 0x5001   /* 功能码错误 */
#define MBRTU_ADR_ERR                0x5002   /* 起始地址错误 */
#define MBRTU_LEN_ERR                0x5003   /* 输出长度错误 */
#define MBRTU_OPER_ERR               0x5004   /* 数据读或写失败 */
#define MBRTU_CRC_ERR                0x5005   /* 校验错误 */
#define MBRTU_PARA_INVALID           0x5006   /* 输入参数无效 */

/************************************************************************
*                                  Global Struct                         *
************************************************************************/
/* 数据域	*/
typedef struct
{
	uint16_t         uStartAddr;				/* 寄存器起始地址	*/
	uint16_t         uRegLen;				/* 寄存器长度		*/
	uint8_t          uData[256];				/* 数据					  */
}T_MBRTU_Dataspace;

/* modbus-tcp帧格式	*/
typedef struct
{

	uint8_t  uAddr;					              /* 从机地址	*/
	uint8_t  uFuc;					                /* 功能码		*/
  T_MBRTU_Dataspace tDataspace;			/* 数据域		*/        
	uint16_t         uCrc;
}T_ModbusRtuFrame;

/************************************************************************
*                         Global Function Definition                    *
************************************************************************/
/************************************************************************
@ Function Name: MBRTU_ParseUp
@ Description  : modbus parse data(BMS been slave)
@ Input    Para: Recive buf, len, frame
@ Output   Para: None
@ Return  Value: Success & Failed
@ Author   Data: Sophy & 2017.8.23
*************************************************************************/
uint16_t MBRTU_ParseUp(uint8_t *pRecvbuf, uint16_t uRecLen, T_ModbusRtuFrame *ptFrameInfo, uint16_t *pRealLen);
/************************************************************************
@ Function Name: MBRTU_PackUp
@ Description  : modbus pack data(BMS been slave)
@ Input    Para: Recive buf, len, frame
@ Output   Para: None
@ Return  Value: Success & Failed
@ Author   Data: Sophy & 2017.8.23
*************************************************************************/
uint16_t MBRTU_PackUp(uint8_t *pSendbuf, T_ModbusRtuFrame *ptFrameinfo);
void SetCommAddr(uint8_t addr);
uint8_t GetCommAddr(void);
#ifdef __cplusplus
}
#endif

#endif

