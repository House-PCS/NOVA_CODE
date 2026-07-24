/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __BSP_H_
#define __BSP_H_

#include "gpio.h"
//#include "exti.h"
#include "spi.h"
#include "uart.h"
#include "dma.h"
#include "timer.h"
#include "string.h"
#include "stdint.h" 
#include "flash.h"

/////////////////////UART/////////////////////////////////////
typedef enum
{
    UART_IDLE=0,
    UART_SEND_ING,
    UART_SEND_STOP,
}EN_SEND_STATE;

typedef struct {
	EN_SEND_STATE   Sending;		//发送状态，0:空闲，1:发送中，2:发送结束中（485延时切换状态）
	bool            ReceiveFlg;	//接收完成标志
	uint16_t        ReceiveNum;
    uint32_t        send_time;
}UART_RXTX_STATE;


//////////////IO_IN/////////////////
typedef enum
{
    IO2=0,
    IO4,
    
	DI1,	//
	DI2,
    
    DIH3,
    DIH4,
    DIH5,
    
    DIL6,
    DIL7,
    DIL8,
    
    DI_NUM,
}DIx_PORT;

extern bool gDI_State[DI_NUM];

#define DI_GET(DIx)     (gDI_State[DIx])
#define IO2_GET()       (gDI_State[IO2])

//////////////IO_IN END/////////////////

//////////////IO_OUT/////////////////
typedef enum
{
	RES_KG=0,	//
	RES_KP,
    RES_KN,
    
    S0,
    S1,
//	A0,      //485发送、接收引脚
	B3,
    
//    DO1,
//    DO2,
    
//    DO3,
//    DO4,
//    DO5,
//    DO9,
//    
//    DO6,
//    DO7,
//    DO8,
//    DO10,
    
    DO_NUM,
}DOx_PORT;
//DO_SET(DOx,val) 
//{
//	do{if(!gDO_ForceFlg) {gDO_State[DOx]=val; 
//	GpioBitWrite((&DO_Handle[DOx]),val);} } while(0)
//} 

#define DO_GET(DOx)     (gDO_State[DOx])

#define DO_SET(DOx,val)  do{if(!gDO_ForceFlg) {gDO_State[DOx]=val; GpioBitWrite((&DO_Handle[DOx]),val);} } while(0) 
#define RES_KG_SET(val)    gDO_State[RES_KG]=val; GpioBitWrite((&DO_Handle[RES_KG]),val)
#define RES_KP_SET(val)    gDO_State[RES_KP]=val;  GpioBitWrite((&DO_Handle[RES_KP]),val)
#define RES_KN_SET(val)    gDO_State[RES_KN]=val; GpioBitWrite((&DO_Handle[RES_KN]),val)
#define DO_RESET(DOx)    gDO_State[DOx]=false; GpioBitWrite((&DO_Handle[DOx]),false)

#define DO_FORCE_SET(DOx,val)    do{gDO_ForceFlg = true; gDO_State[DOx]=val; GpioBitWrite((&DO_Handle[DOx]),val);} while(0) 
extern bool gDO_State[DO_NUM];
extern bool gDO_ForceFlg;
extern Gpio_HandleType DO_Handle[DO_NUM];

//////////////IO_OUT END/////////////////





extern Uart_HandleType gUart1ModbusHandle;
extern Gpio_HandleType LedSysHandle;
extern Gpio_HandleType LedPcsHandle;
extern Gpio_HandleType LedConnHandle;

extern Gpio_HandleType gGpioDirModbusHandle;

void BaseBspInit();

void DevModbusSlaveBspInit();
void LedBspInit();
void DetectIdBspInit();
void DO_BspInit();

#endif

