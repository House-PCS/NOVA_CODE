
#include "timer.h"



void TimerClkInit()
{
    rcu_periph_clock_enable(RCU_AF);
}



static void _TimerGpioInit(Timer_HandleType *pHandle)
{
    uint8_t i;
    for(i = 0; i < TIMER_CHANNEL_NUM; i++)
    {
        if(pHandle->timer_cfg->channel_config[i].enable == true)
        {
            gpio_init(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx);
        }
    }
}



void TimerInit(Timer_HandleType *pHandle)
{
    timer_parameter_struct      timer_initpara;
    timer_ic_parameter_struct   timer_icinitpara;
    timer_oc_parameter_struct   timer_ocinitpara;
    uint8_t i;

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
                timer_channel_output_pulse_value_config(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->channel_config[i].chl_num, pHandle->timer_cfg->channel_config[i].default_duty);
                
                if(pHandle->timer_cfg->timer_peri == TIMER0 || pHandle->timer_cfg->timer_peri == TIMER7)
                {
                    timer_primary_output_config(pHandle->timer_cfg->timer_peri, ENABLE);
                }
            }
        }
    }
    if(pHandle->timer_cfg->quadrature_decoder_mode_enable == 1)
    {
        timer_quadrature_decoder_mode_config(pHandle->timer_cfg->timer_peri , TIMER_ENCODER_MODE2, TIMER_IC_POLARITY_RISING, TIMER_IC_POLARITY_RISING);
    }
    else
    {
        timer_auto_reload_shadow_enable(pHandle->timer_cfg->timer_peri);
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


void TimerPWMDutySet(Timer_HandleType *pHandle)/*duty：0~100*/
{
    uint32_t period = pHandle->timer_cfg->period;
    uint32_t pulse_value;
    uint32_t duty = pHandle->oc_pwm_duty;

    if(duty > 100)
    {
        return;
    }
    pulse_value = period * duty / 100;

     timer_channel_output_pulse_value_config(pHandle->timer_cfg->timer_peri, period = pHandle->current_chl, pulse_value);
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














