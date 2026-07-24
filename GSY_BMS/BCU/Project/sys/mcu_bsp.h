#ifndef __MCUBSP_H_
#define __MCUBSP_H_


//#include "PublicTypedef.h"
#include "adc.h"
#include "gpio.h"
#include "uart.h"
#include "dma.h"
#include "can.h"
#include "queue.h"
#include "i2c.h"
#include "sim_i2c.h"
#include "enet.h"
#include "spi.h"
#include "timer.h"

    

//SPIFLASH
extern Gpio_HandleType gSpiFlashCsHandle;
extern Spi_HandleType gSpiFlashHandle;
void SpiFlashBspInit(void);

//RTC
extern I2C_HandleType gRtcI2CHandle;
extern SimI2C_HandleType gSimRtcI2CHandle;
extern Gpio_HandleType gRtcIntHandle;
void RtcBspInit(void);

//EEPROM
extern I2C_HandleType gEepromI2CHandle;
extern SimI2C_HandleType gEepromSimI2CHandle;
void E2promBspInit(void);

//SGM58031
extern SimI2C_HandleType gSGMI2CHandle;
extern void SGMI2CBspInit(void);

//HT7017
extern Uart_HandleType gUartHT7017Handle;
extern void HT7017BspInit(void);

//COMM UART
extern Uart_HandleType gUart1ModbusHandle;
extern Gpio_HandleType gGpioDE1ModbusHandle;
extern Uart_HandleType gUart2ModbusHandle;
extern Gpio_HandleType gGpioDE2ModbusHandle;
extern Uart_HandleType gUart0TTLHandle;
void UartCommBspInit(void);

//CAN
extern Can_HandleType gCanHandle[];
void CanBspInit(void);

//ADC
#define ADC_USE_CONT_MODE   0//连续采样模式，使能DMA
extern Adc_HandleType gAdc2Handle;
void Adc2BspInit(void);

//IO
#define IO_2WAY_NUM                     4 //双向IO

#define IO_ACT_LOW_INPUT_NUM            3 //低有效输入
#define IO_ACT_HIGH_INPUT_NUM           4 //高有效输入
#define IO_ISO_INPUT_NUM                1 //隔离输入

#define IO_LOW_SIDE_OUTPUT_NUM          4 //低边输出
#define IO_HIGH_SIDE_OUTPUT_NUM         4 //高边输出
#define IO_ISO_OUTPUT_NUM               3 //隔离输出
#define IO_DRY_OUTPUT_NUM               2 //干接点输出

extern Gpio_HandleType gTwoWayOutputHandle[IO_2WAY_NUM];
extern Gpio_HandleType gTwoWayInputHandle[IO_2WAY_NUM];
extern Gpio_HandleType gActLowInputHandle[IO_ACT_LOW_INPUT_NUM];
extern Gpio_HandleType gActHighInputHandle[IO_ACT_HIGH_INPUT_NUM];
extern Gpio_HandleType gIsoInputHandle[IO_ISO_INPUT_NUM];
extern Gpio_HandleType gLowSideOutputHandle[IO_LOW_SIDE_OUTPUT_NUM];
extern Gpio_HandleType gHighSideOutputHandle[IO_HIGH_SIDE_OUTPUT_NUM];
extern Gpio_HandleType gIsoOutputHandle[IO_ISO_OUTPUT_NUM];
extern Gpio_HandleType gDryOutputHandle[IO_DRY_OUTPUT_NUM];
void IOCtlBspInit(void);

//TIMER5 轮询
extern Timer_HandleType gTimerPollingHandle;
void TimerPollingBspInit(void);

//LED灯
extern Gpio_HandleType gGpioLedHandle;
void GpioLedBspInit(void);

void McuBspInit(void);






	
#endif
