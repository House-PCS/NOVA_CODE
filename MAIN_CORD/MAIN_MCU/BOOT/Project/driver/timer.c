/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        timer.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-06-19
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-06-19    <td>V1.0.0    <td>           <td>InitialVersion 
  * </table>
  ******************************************************************************
  */
#include "timer.h"



void TimerClkInit()
{
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
}



static void _TimerGpioInit(Timer_HandleType *pHandle)
{
    uint8_t i;
    for(i = 0; i < TIMER_CHANNEL_NUM; i++)
    {
        if(pHandle->timer_cfg->channel_config[i].enable == true)
        {
            gpio_mode_set(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, GPIO_MODE_AF, GPIO_PUPD_NONE, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx);
            gpio_output_options_set(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx);

            gpio_af_set(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_alt_num, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx);
            if(pHandle->timer_cfg->timer_peri == TIMER0 || pHandle->timer_cfg->timer_peri == TIMER7) 
            {
                if(pHandle->timer_cfg->channel_config[i].complementary_enable == true)
                {
                    gpio_mode_set(pHandle->timer_cfg->channel_config[i].timer_ch_on_gpiox, GPIO_MODE_AF, GPIO_PUPD_NONE, pHandle->timer_cfg->channel_config[i].timer_ch_on_gpio_pinx);
                    gpio_output_options_set(pHandle->timer_cfg->channel_config[i].timer_ch_on_gpiox, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pHandle->timer_cfg->channel_config[i].timer_ch_on_gpio_pinx);

                    gpio_af_set(pHandle->timer_cfg->channel_config[i].timer_ch_on_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_on_alt_num, pHandle->timer_cfg->channel_config[i].timer_ch_on_gpio_pinx);
                }
                if(pHandle->timer_cfg->break_enable == true)
                {
                    gpio_mode_set(pHandle->timer_cfg->timer_brkin_gpiox, GPIO_MODE_AF, GPIO_PUPD_NONE, pHandle->timer_cfg->timer_brkin_gpio_pinx);
                    gpio_output_options_set(pHandle->timer_cfg->timer_brkin_gpiox, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pHandle->timer_cfg->timer_brkin_gpio_pinx);

                    gpio_af_set(pHandle->timer_cfg->timer_brkin_gpiox, pHandle->timer_cfg->brkin_alt_num, pHandle->timer_cfg->timer_brkin_gpio_pinx);
                }
            }
        }
    }
}


void TimerDeInit(Timer_HandleType *pHandle)
{
    timer_deinit(pHandle->timer_cfg->timer_peri);    
}


void TimerInit(Timer_HandleType *pHandle)
{
    timer_parameter_struct      timer_initpara;
    timer_ic_parameter_struct   timer_icinitpara;
    timer_oc_parameter_struct   timer_ocinitpara;
    timer_break_parameter_struct timer_breakpara;
    
    uint8_t i;
    uint8_t oc_existed = 0;

    rcu_periph_clock_enable(pHandle->timer_cfg->rcu_timer);
    _TimerGpioInit(pHandle);

    timer_deinit(pHandle->timer_cfg->timer_peri);

    timer_initpara.prescaler                = pHandle->timer_cfg->prescaler;
    timer_initpara.alignedmode              = pHandle->timer_cfg->alignedmode;
    timer_initpara.counterdirection         = pHandle->timer_cfg->counterdirection;
    timer_initpara.period                   = pHandle->timer_cfg->period;
    timer_initpara.clockdivision            = pHandle->timer_cfg->clockdivision;
    timer_initpara.repetitioncounter        = pHandle->timer_cfg->repetitioncounter;
    timer_init(pHandle->timer_cfg->timer_peri, &timer_initpara);

    for(i = 0; i < TIMER_CHANNEL_NUM; i++)
    {
        if(pHandle->timer_cfg->channel_config[i].enable == true)
        {
            if(pHandle->timer_cfg->channel_config[i].ic_or_oc == 0)
            {
                timer_icinitpara.icpolarity         = pHandle->timer_cfg->channel_config[i].icpolarity;
                timer_icinitpara.icselection        = pHandle->timer_cfg->channel_config[i].icselection;
                timer_icinitpara.icprescaler        = pHandle->timer_cfg->channel_config[i].icprescaler;
                timer_icinitpara.icfilter           = pHandle->timer_cfg->channel_config[i].icfilter;

                if(pHandle->timer_cfg->quadrature_decoder_mode_enable == 1 && pHandle->timer_cfg->channel_config[i].chl_num <= TIMER_CH_1)
                {
                    timer_input_pwm_capture_config(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->channel_config[i].chl_num, &timer_icinitpara);
                }
                else
                {
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->channel_config[i].chl_num, &timer_icinitpara);
                }
            }
            else
            {
                timer_ocinitpara.ocpolarity         = pHandle->timer_cfg->channel_config[i].ocpolarity;
                timer_ocinitpara.ocnpolarity        = pHandle->timer_cfg->channel_config[i].ocnpolarity;
                timer_ocinitpara.outputstate        = pHandle->timer_cfg->channel_config[i].outputstate;
                timer_ocinitpara.outputnstate       = pHandle->timer_cfg->channel_config[i].outputnstate;
                timer_ocinitpara.ocidlestate        = pHandle->timer_cfg->channel_config[i].ocidlestate;
                timer_ocinitpara.ocnidlestate       = pHandle->timer_cfg->channel_config[i].ocnidlestate;
                timer_channel_output_config(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->channel_config[i].chl_num, &timer_ocinitpara);


                timer_channel_output_mode_config(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->channel_config[i].chl_num, TIMER_OC_MODE_PWM0);
                timer_channel_output_shadow_config(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->channel_config[i].chl_num, TIMER_OC_SHADOW_ENABLE);
                timer_channel_output_pulse_value_config(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->channel_config[i].chl_num, pHandle->timer_cfg->channel_config[i].oc_duty);
                
                oc_existed = 1;
            }
        }
    }
    if(pHandle->timer_cfg->quadrature_decoder_mode_enable == 1)
    {
        timer_quadrature_decoder_mode_config(pHandle->timer_cfg->timer_peri , TIMER_QUAD_DECODER_MODE2, TIMER_IC_POLARITY_RISING, TIMER_IC_POLARITY_RISING);
    }
    else
    {
        timer_auto_reload_shadow_enable(pHandle->timer_cfg->timer_peri);
    }
    
    if(pHandle->timer_cfg->timer_peri == TIMER0 || pHandle->timer_cfg->timer_peri == TIMER7)
    {
        timer_breakpara.runoffstate      = TIMER_ROS_STATE_DISABLE;
        timer_breakpara.ideloffstate     = TIMER_IOS_STATE_DISABLE ;
        timer_breakpara.deadtime         = pHandle->timer_cfg->dead_time;
        timer_breakpara.breakpolarity    = pHandle->timer_cfg->breakpolarity;
        timer_breakpara.outputautostate  = TIMER_OUTAUTO_ENABLE;
        timer_breakpara.protectmode      = TIMER_CCHP_PROT_0;
        if(pHandle->timer_cfg->break_enable == true)
        {
            timer_breakpara.breakstate       = TIMER_BREAK_ENABLE;
        }
        else
        {
            timer_breakpara.breakstate       = TIMER_BREAK_DISABLE;
        }
        timer_break_config(pHandle->timer_cfg->timer_peri, &timer_breakpara);
        
        if(oc_existed == 1)
        {
            timer_primary_output_config(pHandle->timer_cfg->timer_peri, ENABLE);
        }
    }

    if(pHandle->timer_cfg->en_irq == true)
    {
        timer_interrupt_flag_clear(pHandle->timer_cfg->timer_peri, TIMER_INT_FLAG(pHandle->timer_cfg->irq_type));/* 清除更新中断标志位 */
        timer_interrupt_enable(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->irq_type);/* 使能定时器更新中断 */

        nvic_irq_enable(pHandle->timer_cfg->nvic_irq, pHandle->timer_cfg->irq_prio, pHandle->timer_cfg->irq_sub_prio);
    }


    timer_enable(pHandle->timer_cfg->timer_peri);


}

void TimerCntSet(Timer_HandleType *pHandle)
{
    timer_counter_value_config(pHandle->timer_cfg->timer_peri, pHandle->current_cnt);
}
void TimerCntGet(Timer_HandleType *pHandle)
{
    pHandle->current_cnt = timer_counter_read(pHandle->timer_cfg->timer_peri);
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
            timer_channel_output_pulse_value_config(pHandle->timer_cfg->timer_peri, pHandle->current_chl, pulse_value);
            break;
        }
    }
}




void TimerIrqProcess(Timer_HandleType *pHandle)
{
    if(timer_interrupt_flag_get(pHandle->timer_cfg->timer_peri, TIMER_INT_FLAG_UP) == SET)
    {
        timer_interrupt_flag_clear(pHandle->timer_cfg->timer_peri, TIMER_INT_FLAG_UP);

        //
        pHandle->irq_process();
    }
}














