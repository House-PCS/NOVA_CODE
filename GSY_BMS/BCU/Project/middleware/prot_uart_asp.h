#ifndef __PROT_UART_ASP_H_
#define __PROT_UART_ASP_H_

#include "PublicTypedef.h"
//#ifndef UART_NUM
//#define UART_NUM    1
//#endif

typedef enum{	
	PORT_SLAVE=0,
	PORT_HOST,
	PORT_FDX,			//全双工交互协议
}PORT_TYPE;		//接口协议模式类型

typedef struct {
	PORT_TYPE	PortType;		//端口模式
	uint8_t ProtModbusNum:4;		//
	uint8_t ProtUnusualNum:4;		//非标协议数量
//	MODBUS_SLAVE_CFG* 
//	bool Sending:1;		//发送状态，flash时发送空闲
//	bool ReceiveFlg:1;	//接收完成标志
//	uint16_t ReceiveNum;
}PROT_UART_ASP;

////////////////VOFA+/////////////
typedef struct {
	uint8_t vofa_en :1;
    uint8_t prot :2;
}VofaInfoType;
void VofaSend(uint8_t *buff,uint16_t len);



void ProtUartMain(void);
void ProtRs485Main(void);	

#endif
