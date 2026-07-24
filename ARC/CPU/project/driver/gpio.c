/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        gpio.c
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


#include "gpio.h"


void GpioClkInit()
{
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);
    GPIO_ConfigPinRemap(GPIO_RMP_SW_JTAG_SW_ENABLE, ENABLE);
#if USE_GPIOA
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
#endif
#if USE_GPIOB
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
#endif
#if USE_GPIOC
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOC, ENABLE);
#endif
#if USE_GPIOD
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOD, ENABLE);
#endif
#if USE_GPIOE
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOE, ENABLE);
#endif
#if USE_GPIOF
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOF, ENABLE);
#endif
#if USE_GPIOG
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOG, ENABLE);
#endif
}

static IRQn_Type _GpioExtiIRQn[] = {EXTI0_IRQn,   EXTI1_IRQn,   EXTI2_IRQn,     EXTI3_IRQn,     EXTI4_IRQn,     EXTI9_5_IRQn,   EXTI9_5_IRQn,   EXTI9_5_IRQn,
                                    EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn
                                   };




void GpioInit(Gpio_HandleType *pHandle)
{
    GPIO_InitType GPIO_InitStructure;
    EXTI_InitType EXTI_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    GPIO_InitStructure.Pin        = pHandle->gpio_cfg->gpio_pinx;
    GPIO_InitStructure.GPIO_Mode  = pHandle->gpio_cfg->gpio_mode;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitPeripheral(pHandle->gpio_cfg->gpiox, &GPIO_InitStructure);

    if(pHandle->gpio_cfg->en_exti == true)
    {
        GPIO_ConfigEXTILine(pHandle->gpio_cfg->gpio_exti_port, pHandle->gpio_cfg->gpio_exti_pin);

        EXTI_InitStructure.EXTI_Line    = (uint32_t)pHandle->gpio_cfg->gpio_pinx;
        EXTI_InitStructure.EXTI_Mode    = pHandle->gpio_cfg->gpio_exti_mode;
        EXTI_InitStructure.EXTI_Trigger = pHandle->gpio_cfg->gpio_exti_triger;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_InitPeripheral(&EXTI_InitStructure);
        if(pHandle->gpio_cfg->gpio_exti_mode == EXTI_Mode_Interrupt)
        {
            NVIC_InitStructure.NVIC_IRQChannel                   = _GpioExtiIRQn[pHandle->gpio_cfg->gpio_exti_pin];
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pHandle->gpio_cfg->irq_prio;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = pHandle->gpio_cfg->irq_sub_prio;
            NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
            EXTI_ClrITPendBit((uint32_t)(pHandle->gpio_cfg->gpio_pinx));
        }
    }
}


void GpioToggleBits(GPIO_Module* GPIOx, uint16_t Pin)
{
    if(GPIO_ReadInputDataBit(GPIOx, Pin) == Bit_SET)
    {
        GPIO_ResetBits(GPIOx, Pin);
    }
    else
    {
        GPIO_SetBits(GPIOx, Pin);
    }
}

void GpioTooglePort(GPIO_Module* GPIOx)
{

}


void GpioIrqProcess(Gpio_HandleType *pHandle)
{
    if(EXTI_GetITStatus((uint32_t)(pHandle->gpio_cfg->gpio_pinx)) != RESET)
    {
        if(pHandle->irq_process() == true)
        {
            EXTI_ClrITPendBit((uint32_t)(pHandle->gpio_cfg->gpio_pinx));
        }
    }
}



