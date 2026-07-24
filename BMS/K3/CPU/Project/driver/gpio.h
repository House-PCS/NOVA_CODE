/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        gpio.h
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


#ifndef __GPIO_H_
#define __GPIO_H_


#include "gd32f30x_gpio.h"
#include "stdbool.h"
#include "drv_opt.h"
#include "exti.h"





typedef struct _gpio_config_stru_t
{
    uint32_t                gpiox;
    uint32_t                gpio_pinx;
    uint32_t                gpio_mode;
    
    bool                    en_exti;
    uint8_t                 gpio_exti_port;
    uint8_t                 gpio_exti_pin;
    exti_mode_enum          gpio_exti_mode;
    exti_trig_type_enum     gpio_exti_triger;
    
    uint8_t                 irq_prio;
    uint8_t                 irq_sub_prio;
}Gpio_Config;


typedef struct _Gpio_Handle_stru_t
{
    Gpio_Config     *gpio_cfg;
    
    bool            (*irq_process)();/*对于外部中断而言，同一个中断可能不是一个GPIO引发，例如PA0和PB0，代表不同业务，会触发同一中断，返回值表示是否为当前业务触发*/
}Gpio_HandleType;







#define PORT(port)          (GPIO_BASE + 0x400 * port)
#define PIN(pin)            BIT(pin)
#define PIN_MODE(mode)      CTL_CLTR(mode)
#define PIN_PUPD(pupd)      PUD_PUPD(pupd)
#define PIN_OTYPE(otype)    ((uint8_t)(otype))

void GpioInit(Gpio_HandleType *pHandle);
void GpioClkInit(void);

#define GpioBitSet(pHandle)           gpio_bit_set(pHandle->gpio_cfg->gpiox, pHandle->gpio_cfg->gpio_pinx)
#define GpioBitReset(pHandle)         gpio_bit_reset(pHandle->gpio_cfg->gpiox, pHandle->gpio_cfg->gpio_pinx)
#define GpioBitToogle(pHandle)        gpio_bit_toggle(pHandle->gpio_cfg->gpiox, pHandle->gpio_cfg->gpio_pinx)
#define GpioBitGet(pHandle)           (uint8_t)gpio_input_bit_get(pHandle->gpio_cfg->gpiox, pHandle->gpio_cfg->gpio_pinx)
#define GpioOutputBitGet(pHandle)     (uint8_t)gpio_output_bit_get(pHandle->gpio_cfg->gpiox, pHandle->gpio_cfg->gpio_pinx)

#define GpioBitWrite(pHandle, value)  gpio_bit_write(pHandle->gpio_cfg->gpiox, pHandle->gpio_cfg->gpio_pinx, (bit_status)value)

#define GpioPortWrite(pHandle, data)  gpio_port_write(pHandle->gpio_cfg->gpiox, pHandle->gpio_cfg->gpio_pinx, data) 
#define GpioPortRead(pHandle)              gpio_output_port_get(pHandle->gpio_cfg->gpiox)
#define GpioPortToogle(pHandle)            gpio_port_toggle(pHandle->gpio_cfg->gpiox)





#endif
















