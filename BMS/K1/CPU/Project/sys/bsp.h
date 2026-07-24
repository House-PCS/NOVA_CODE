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
#include "sim_i2c.h"
#include "i2c.h"
#include "exti.h"
#include "spi.h"
#include "uart.h"
#include "adc.h"
#include "dma.h"
#include "timer.h"
#include "can.h"
#include "mid_shell.h"
#include "debug.h"
#include "dev_can_comm.h"
//#include "EventRecorder.h"

#define SK68XX_CH1_PORT         GPIOA      
#define SK68XX_CH1_PIN          GPIO_PIN_11


#define  TEST_ENDURANCE_MODE     (0)

typedef enum
{
    EN_BMS_A0 = 0,    // 
    EN_BMS_A1,         // 
//	EN_BMS_EXP,
//	EN_BMS_HOST,
    EN_BMS_NUM = 0xFF,
}EN_BMS_TYPE;

extern EN_BMS_TYPE geBMSType;

extern Spi_HandleType gSpiAdbms6830Handle;
extern Gpio_HandleType gCsAdbms6830Handle;
extern Timer_HandleType SQ52201SampleTimeHandle;

extern Uart_HandleType Uart0_Debug_Handle;

extern Uart_HandleType Uart1_ModS_Handle;
extern Gpio_HandleType Modbus485En1;

extern Uart_HandleType Uart2_ModS_Handle;
extern Gpio_HandleType Modbus485En2;

extern Spi_HandleType  SpiFlashHandle;
extern Gpio_HandleType CsSpiFlashHandle;

extern Adc_HandleType  PackAdcHandle;

extern SimI2C_HandleType   gSimI2CEepromHandleType;  



///*****************LED****************/
extern Gpio_HandleType McuLedHandle;//MCU运行指示灯

/*****************过压检测****************/
extern Gpio_HandleType OVCheckHandle;

extern Gpio_HandleType McuPowerEnHandle;

extern Gpio_HandleType AllPackPowerKeyHandle;
extern Gpio_HandleType SignalPackPowerKeyHandle;

extern Gpio_HandleType VbatAdcCtrlHandle;
/*****************限流模块****************/
extern Gpio_HandleType CCPowerEnHandle;
extern Gpio_HandleType CCEnHandle;

/*******************MOS******************/
extern Gpio_HandleType PMosEnHandle;
extern Gpio_HandleType DMosEnHandle;
extern Gpio_HandleType CMosEnHandle;


/******************过流检测****************/
extern Gpio_HandleType Dsg_OC_Check_Handle;
extern Gpio_HandleType Dsg_OC_Clear_Handle;
/******************INA226****************/
extern SimI2C_HandleType   SQ52201Handle;
/******************加热膜****************/
extern Gpio_HandleType HeatEnHandle;
extern Gpio_HandleType HeatCheckHandle;

/****************OVC****************/
extern Gpio_HandleType OvcEnHandle;
/****************泄放电路****************/
extern Gpio_HandleType ReleaseHandle;
/****************充电器检测****************/
extern Gpio_HandleType ChargerCheckHandle;

extern Can_HandleType gCanHandle[];

extern Timer_HandleType gDelayTimerHandle;
void DelayTimerInit(void);


void CanBspInit(void);
void BspInit(void);
void SpiAdbms6830BspInit(void);
void PowerKeyInit(void);
void Uart0DebugBspInit(void);
void SpiFlashBspInit(void);
void SimI2CEepromBspInit(void);
void SpiAdbms6830BspInit(void);
void AFETimerInit(void);

void McuLedBspInit(void);
void RunLedEnInit(void);
void MosBspInit(void);
void ADCBspInit(void);
void Uart1ModsBspInit(void);
void Uart2ModsBspInit(void);
void BoardIdBspInit(void);

void McuPowerEnInit(void);
void PowerKeyInit(void);

void CCEnInit(void);
void VbatAdcCtrlInit(void);
void Dsg_OC_ClearGpioBspInit(void);
void Dsg_OC_CheckGpioBspInit(void);
void OVCheckInit(void);
void HeatInit(void);
void ReleaseInit(void);
void ChargerCheckBspInit(void);

void SQ52201BspInit(void);
#endif












