/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        timer.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-07-24
  * @brief       GD32F30x
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
#include "timer.h"



static void _TimerGpioInit(Timer_HandleType *pHandle)
{
    uint8_t i;
    GPIO_InitType GPIO_InitStructure;
    for(i = 0; i < TIMER_CHANNEL_NUM; i++)
    {
        if(pHandle->timer_cfg->channel_config[i].enable == true)
        {
            if(pHandle->timer_cfg->channel_config[i].alt_rmp_mode == 3)
            {
                GPIO_ConfigPinRemap(pHandle->timer_cfg->channel_config[i].alt_rmp_function, ENABLE);
            }
            GPIO_InitStructure.Pin        = pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx;
            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

            GPIO_InitPeripheral(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, &GPIO_InitStructure);
            
            
            if(pHandle->timer_cfg->timer_peri == TIM1 || pHandle->timer_cfg->timer_peri == TIM8) 
            {
                if(pHandle->timer_cfg->channel_config[i].complementary_enable == true)
                {
                    if(pHandle->timer_cfg->channel_config[i].comp_alt_rmp_mode == 3)
                    {
                        GPIO_ConfigPinRemap(pHandle->timer_cfg->channel_config[i].comp_alt_rmp_function, ENABLE);
                    }
                    GPIO_InitStructure.Pin        = pHandle->timer_cfg->channel_config[i].timer_ch_on_gpio_pinx;
                    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

                    GPIO_InitPeripheral(pHandle->timer_cfg->channel_config[i].timer_ch_on_gpiox, &GPIO_InitStructure);
                }
            }
        }
    }
}


void TimerInit(Timer_HandleType *pHandle)
{
    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    OCInitType TIM_OCInitStructure;
    TIM_ICInitType TIM_ICInitStructure;
    TIM_BDTRInitType TIM_BRKInitStructure;
    
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
    TIM_InitOcStruct(&TIM_OCInitStructure);
    TIM_InitIcStruct(&TIM_ICInitStructure);
     
    uint8_t i;
    uint8_t oc_existed = 0;
    
    if(pHandle->timer_cfg->timer_peri == TIM1 || pHandle->timer_cfg->timer_peri == TIM8)
    {
        RCC_EnableAPB2PeriphClk(pHandle->timer_cfg->rcc_timer, ENABLE);
    }
    else
    {
        RCC_EnableAPB1PeriphClk(pHandle->timer_cfg->rcc_timer, ENABLE);
    }

    _TimerGpioInit(pHandle);

    TIM_DeInit(pHandle->timer_cfg->timer_peri);
    
    TIM_TimeBaseStructure.Prescaler         = pHandle->timer_cfg->prescaler;
    TIM_TimeBaseStructure.CntMode           = pHandle->timer_cfg->counterdirection;
    TIM_TimeBaseStructure.Period            = pHandle->timer_cfg->period;
    TIM_TimeBaseStructure.ClkDiv            = pHandle->timer_cfg->clockdivision;
    TIM_TimeBaseStructure.RepetCnt          = pHandle->timer_cfg->repetitioncounter;
    TIM_InitTimeBase(pHandle->timer_cfg->timer_peri, &TIM_TimeBaseStructure);

    for(i = 0; i < TIMER_CHANNEL_NUM; i++)
    {
        if(pHandle->timer_cfg->channel_config[i].enable == true)
        {
            if(pHandle->timer_cfg->channel_config[i].ic_or_oc == 0)
            {
                TIM_ICInitStructure.IcPolarity      = pHandle->timer_cfg->channel_config[i].icpolarity;
                TIM_ICInitStructure.IcSelection     = pHandle->timer_cfg->channel_config[i].icselection;
                TIM_ICInitStructure.IcPrescaler     = pHandle->timer_cfg->channel_config[i].icprescaler;
                TIM_ICInitStructure.IcFilter        = pHandle->timer_cfg->channel_config[i].icfilter;
                TIM_ICInitStructure.Channel         = pHandle->timer_cfg->channel_config[i].chl_num;
                
                TIM_ConfigPwmIc(pHandle->timer_cfg->timer_peri, &TIM_ICInitStructure);
            }
            else
            {
                TIM_OCInitStructure.OcPolarity      = pHandle->timer_cfg->channel_config[i].ocpolarity;
                TIM_OCInitStructure.OcNPolarity     = pHandle->timer_cfg->channel_config[i].ocnpolarity;
                TIM_OCInitStructure.OutputState     = pHandle->timer_cfg->channel_config[i].outputstate;
                TIM_OCInitStructure.OutputNState    = pHandle->timer_cfg->channel_config[i].outputnstate;
                TIM_OCInitStructure.OcIdleState     = pHandle->timer_cfg->channel_config[i].ocidlestate;
                TIM_OCInitStructure.OcNIdleState    = pHandle->timer_cfg->channel_config[i].ocnidlestate;
                TIM_OCInitStructure.OcMode          = pHandle->timer_cfg->channel_config[i].oc_mode;
                TIM_OCInitStructure.Pulse           = pHandle->timer_cfg->channel_config[i].oc_duty;
                
                if(pHandle->timer_cfg->channel_config[i].chl_num == TIM_CH_1)
                {
                    TIM_InitOc1(pHandle->timer_cfg->timer_peri, &TIM_OCInitStructure);
                    TIM_ConfigOc1Preload(pHandle->timer_cfg->timer_peri, TIM_OC_PRE_LOAD_ENABLE);
                }
                else if(pHandle->timer_cfg->channel_config[i].chl_num == TIM_CH_2)
                {
                    TIM_InitOc2(pHandle->timer_cfg->timer_peri, &TIM_OCInitStructure);
                    TIM_ConfigOc2Preload(pHandle->timer_cfg->timer_peri, TIM_OC_PRE_LOAD_ENABLE);
                }
                else if(pHandle->timer_cfg->channel_config[i].chl_num == TIM_CH_3)
                {
                    TIM_InitOc3(pHandle->timer_cfg->timer_peri, &TIM_OCInitStructure);
                    TIM_ConfigOc3Preload(pHandle->timer_cfg->timer_peri, TIM_OC_PRE_LOAD_ENABLE);
                }
                else if(pHandle->timer_cfg->channel_config[i].chl_num == TIM_CH_4)
                {
                    TIM_InitOc4(pHandle->timer_cfg->timer_peri, &TIM_OCInitStructure);
                    TIM_ConfigOc4Preload(pHandle->timer_cfg->timer_peri, TIM_OC_PRE_LOAD_ENABLE);
                }

                oc_existed = 1;
            }
        }
    }
    if(pHandle->timer_cfg->quadrature_decoder_mode_enable == 1)
    {
        TIM_ConfigEncoderInterface(pHandle->timer_cfg->timer_peri, TIM_ENCODE_MODE_TI12, TIM_IC_POLARITY_RISING, TIM_IC_POLARITY_RISING);
    }
    else
    {
        TIM_ConfigArPreload(pHandle->timer_cfg->timer_peri, ENABLE);
    }
    

//    if(pHandle->timer_cfg->timer_peri == TIMER0 || pHandle->timer_cfg->timer_peri == TIMER7)
//    {
//        timer_breakpara.runoffstate      = TIMER_ROS_STATE_DISABLE;
//        timer_breakpara.ideloffstate     = TIMER_IOS_STATE_DISABLE ;
//        timer_breakpara.deadtime         = pHandle->timer_cfg->dead_time;
//        timer_breakpara.breakpolarity    = pHandle->timer_cfg->breakpolarity;
//        timer_breakpara.outputautostate  = TIMER_OUTAUTO_ENABLE;
//        timer_breakpara.protectmode      = TIMER_CCHP_PROT_0;
//        if(pHandle->timer_cfg->break_enable == true)
//        {
//            timer_breakpara.breakstate       = TIMER_BREAK_ENABLE;
//        }
//        else
//        {
//            timer_breakpara.breakstate       = TIMER_BREAK_DISABLE;
//        }
//        timer_break_config(pHandle->timer_cfg->timer_peri, &timer_breakpara);
//        
//        if(oc_existed == 1)
//        {
//            timer_primary_output_config(pHandle->timer_cfg->timer_peri, ENABLE);
//        }
//    }
    

    if(pHandle->timer_cfg->en_irq == true)
    {
        NVIC_InitType NVIC_InitStructure;
        
        TIM_ClrIntPendingBit(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->irq_type);/* 清除更新中断标志位 */
        TIM_ConfigInt(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->irq_type, ENABLE);/* 使能定时器更新中断 */

        /* Enable the USARTz Interrupt */
        NVIC_InitStructure.NVIC_IRQChannel                   = pHandle->timer_cfg->nvic_irq;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pHandle->timer_cfg->irq_prio;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = pHandle->timer_cfg->irq_sub_prio;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }


    TIM_Enable(pHandle->timer_cfg->timer_peri, ENABLE);
}



void TimerCntSet(Timer_HandleType *pHandle)
{
    TIM_SetCnt(pHandle->timer_cfg->timer_peri, pHandle->current_cnt);
}
void TimerCntGet(Timer_HandleType *pHandle)
{
    pHandle->current_cnt = TIM_GetCnt(pHandle->timer_cfg->timer_peri);
}

uint32_t TimPeriodDefaultGet(Timer_HandleType *pHandle)
{
    return pHandle->timer_cfg->period;
}

void TimerPWMDutySet(Timer_HandleType *pHandle, uint16_t duty_particle)/*duty：由duty_particle颗粒度决定，如duty_particle = 100，则duty范围为0~100*/
{
    uint32_t period = pHandle->timer_cfg->period;
    uint32_t pulse_value;
    uint16_t duty;
    uint8_t i = 0;
    
    duty = pHandle->oc_pwm_duty;

    if(duty > duty_particle)
    {
        return;
    }
    pulse_value = period * duty / duty_particle;
    
    
    
    for(i = 0; i < TIMER_CHANNEL_NUM; i++)
    {
        if(pHandle->timer_cfg->channel_config[i].chl_num == pHandle->current_chl)
        {
            pHandle->timer_cfg->channel_config[i].oc_duty = pulse_value;
            if(pHandle->current_chl == TIM_CH_1)
            {
                TIM_SetCmp1(pHandle->timer_cfg->timer_peri, pulse_value);
            }
            else if(pHandle->current_chl == TIM_CH_2)
            {
                TIM_SetCmp2(pHandle->timer_cfg->timer_peri, pulse_value);
            }
            else if(pHandle->current_chl == TIM_CH_3)
            {
                TIM_SetCmp3(pHandle->timer_cfg->timer_peri, pulse_value);
            }
            else if(pHandle->current_chl == TIM_CH_4)
            {
                TIM_SetCmp4(pHandle->timer_cfg->timer_peri, pulse_value);
            }
            break;
        }
    }
}


void TimerIrqProcess(Timer_HandleType *pHandle)
{
    if(TIM_GetIntStatus(pHandle->timer_cfg->timer_peri, TIM_INT_UPDATE) == SET)
    {
        TIM_ClrIntPendingBit(pHandle->timer_cfg->timer_peri, TIM_INT_UPDATE);

        //
        pHandle->irq_process();
    }
}


