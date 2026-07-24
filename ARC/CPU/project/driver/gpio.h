/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        gpio.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-07-24
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-07-24    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#ifndef __GPIO_H_
#define __GPIO_H_



#include "stdbool.h"
#include "drv_opt.h"


typedef struct _gpio_config_stru_t
{
    GPIO_Module*            gpiox;
    uint32_t                gpio_pinx;
    GPIO_ModeType           gpio_mode;
    
    bool                    en_exti;
    uint8_t                 gpio_exti_port;
    uint8_t                 gpio_exti_pin;
    EXTI_ModeType           gpio_exti_mode;
    EXTI_TriggerType        gpio_exti_triger;
    
    uint8_t                 irq_prio;
    uint8_t                 irq_sub_prio;
}Gpio_Config;


typedef struct _Gpio_Handle_stru_t
{
    Gpio_Config     *gpio_cfg;
    
    bool            (*irq_process)();/*对于外部中断而言，同一个中断可能不是一个GPIO引发，例如PA0和PB0，代表不同业务，会触发同一中断，返回值表示是否为当前业务触发*/
}Gpio_HandleType;

void GpioInit(Gpio_HandleType *pHandle);


void GpioClkInit();

#define GpioBitSet(pHandle)           GPIO_SetBits((pHandle)->gpio_cfg->gpiox, (pHandle)->gpio_cfg->gpio_pinx)
#define GpioBitReset(pHandle)         GPIO_ResetBits((pHandle)->gpio_cfg->gpiox, (pHandle)->gpio_cfg->gpio_pinx)
#define GpioBitToogle(pHandle)        GpioToggleBits((pHandle)->gpio_cfg->gpiox, (pHandle)->gpio_cfg->gpio_pinx)
#define GpioBitGet(pHandle)           (uint8_t)GPIO_ReadInputDataBit((pHandle)->gpio_cfg->gpiox, (pHandle)->gpio_cfg->gpio_pinx)
#define GpioOutputBitGet(pHandle)     (uint8_t)GPIO_ReadOutputDataBit((pHandle)->gpio_cfg->gpiox, (pHandle)->gpio_cfg->gpio_pinx)

#define GpioBitWrite(pHandle, value)  GPIO_WriteBit((pHandle)->gpio_cfg->gpiox, (pHandle)->gpio_cfg->gpio_pinx, (Bit_OperateType)value)

#define GpioPortWrite(pHandle, data)  GPIO_Write((pHandle)->gpio_cfg->gpiox, (pHandle)->gpio_cfg->gpio_pinx, data) 
#define GpioPortRead(pHandle)              GPIO_ReadInputData((pHandle)->gpio_cfg->gpiox)
//#define GpioPortToogle(pHandle)            gpio_port_toggle((pHandle)->gpio_cfg->gpiox)



#endif









