/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        gpio.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       GD32F30x
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


#include "gpio.h"

//#define PORT_RCU(rcu)       RCU_REGIDX_BIT(AHB1EN_REG_OFFSET, rcu)

void GpioClkInit(void)
{
    rcu_periph_clock_enable(RCU_AF);
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
#if USE_GPIOA
    rcu_periph_clock_enable(RCU_GPIOA);
#endif
#if USE_GPIOB
    rcu_periph_clock_enable(RCU_GPIOB);
#endif   
#if USE_GPIOC
    rcu_periph_clock_enable(RCU_GPIOC);
#endif
#if USE_GPIOD
    rcu_periph_clock_enable(RCU_GPIOD);
#endif
#if USE_GPIOE
    rcu_periph_clock_enable(RCU_GPIOE);
#endif
#if USE_GPIOF
    rcu_periph_clock_enable(RCU_GPIOF);
#endif
#if USE_GPIOG
    rcu_periph_clock_enable(RCU_GPIOG);
#endif
#if USE_GPIOH
    rcu_periph_clock_enable(RCU_GPIOH);
#endif
#if USE_GPIOI
    rcu_periph_clock_enable(RCU_GPIOI);
#endif
}

static IRQn_Type _GpioExtiIRQn[] = {EXTI0_IRQn,   EXTI1_IRQn,   EXTI2_IRQn,     EXTI3_IRQn,     EXTI4_IRQn,     EXTI5_9_IRQn,   EXTI5_9_IRQn,   EXTI5_9_IRQn, 
                                    EXTI5_9_IRQn, EXTI5_9_IRQn, EXTI10_15_IRQn, EXTI10_15_IRQn, EXTI10_15_IRQn, EXTI10_15_IRQn, EXTI10_15_IRQn, EXTI10_15_IRQn};


void GpioInit(Gpio_HandleType *pHandle)
{
    gpio_init(pHandle->gpio_cfg->gpiox, pHandle->gpio_cfg->gpio_mode, GPIO_OSPEED_50MHZ, pHandle->gpio_cfg->gpio_pinx);
    
    if(pHandle->gpio_cfg->en_exti == true)
    {
        /* Connect EXTI Line to pin */
        gpio_exti_source_select(pHandle->gpio_cfg->gpio_exti_port, pHandle->gpio_cfg->gpio_exti_pin);    
        exti_init((exti_line_enum)(pHandle->gpio_cfg->gpio_pinx), pHandle->gpio_cfg->gpio_exti_mode, pHandle->gpio_cfg->gpio_exti_triger);
        if(pHandle->gpio_cfg->gpio_exti_mode == EXTI_INTERRUPT)
        {
            nvic_irq_enable(_GpioExtiIRQn[pHandle->gpio_cfg->gpio_exti_pin], pHandle->gpio_cfg->irq_prio, pHandle->gpio_cfg->irq_sub_prio);
            exti_interrupt_flag_clear((exti_line_enum)(pHandle->gpio_cfg->gpio_pinx));
        }
    }    
}

void GpioIrqProcess(Gpio_HandleType *pHandle)
{
    if(exti_interrupt_flag_get((exti_line_enum)(pHandle->gpio_cfg->gpio_pinx)) != RESET)
    {
        if(pHandle->irq_process() == true)
        {
            exti_interrupt_flag_clear((exti_line_enum)(pHandle->gpio_cfg->gpio_pinx));
        }
    }
}
