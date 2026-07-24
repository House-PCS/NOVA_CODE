/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        timer.c
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
#include "timer.h"
#include "systick.h"

#define NULL    (0)
static uint32_t _gHCLK;

/**
  * @brief  打开RCU_AF.
  * @param  NULL
  * @return NULL
  * @note   none
  */
void TimerClkInit()
{
    rcu_periph_clock_enable(RCU_AF);
}


/**
  * @brief  Timer GPIO init.
  * @param  pHandle: Timer需要用户配置的参数
  * @return NULL
  * @note   none
  */
static void _TimerGpioInit(Timer_HandleType *pHandle)
{
    uint8_t i;
    for(i = 0; i < TIMER_CHANNEL_NUM; i++)
    {
        if(pHandle->timer_cfg->channel_config[i].enable == true)
        {
            if(pHandle->timer_cfg->channel_config[i].alt_rmp_mode == 3)
            {
                gpio_pin_remap_config(pHandle->timer_cfg->channel_config[i].alt_rmp_function, ENABLE);
            }
            gpio_init(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx);
            
            if(pHandle->timer_cfg->timer_peri == TIMER0 || pHandle->timer_cfg->timer_peri == TIMER7) 
            {
                if(pHandle->timer_cfg->channel_config[i].complementary_enable == true)
                {
                    if(pHandle->timer_cfg->channel_config[i].comp_alt_rmp_mode == 3)
                    {
                        gpio_pin_remap_config(pHandle->timer_cfg->channel_config[i].comp_alt_rmp_function, ENABLE);
                    }
                    gpio_init(pHandle->timer_cfg->channel_config[i].timer_ch_on_gpiox, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, pHandle->timer_cfg->channel_config[i].timer_ch_on_gpio_pinx);
                }
            }
        }
    }
}


/**
  * @brief  Timer驱动初始化.
  * @param  pHandle: Timer需要用户配置的参数
  * @return NULL
  * @note   none
  */
void TimerInit(Timer_HandleType *pHandle)
{
    timer_parameter_struct      timer_initpara;
    timer_ic_parameter_struct   timer_icinitpara;
    timer_oc_parameter_struct   timer_ocinitpara;
    timer_break_parameter_struct timer_breakpara;
    
    uint8_t i;
    uint8_t oc_existed = 0;

    rcu_periph_clock_enable(pHandle->timer_cfg->rcu_timer);
    _gHCLK = rcu_clock_freq_get(CK_AHB);
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
                else if(pHandle->timer_cfg->dual_pwm_cap_mode == true)//通道0和通道1只能选一个，通道2和通道3不能用该模式
                {
                    timer_input_pwm_capture_config(pHandle->timer_cfg->timer_peri,pHandle->timer_cfg->channel_config[i].chl_num,&timer_icinitpara);
                    /*使能主从管理复位模式*/
                    if(pHandle->timer_cfg->channel_config[i].chl_num == TIMER_CH_0)
                    {
                        timer_input_trigger_source_select(pHandle->timer_cfg->timer_peri,TIMER_SMCFG_TRGSEL_CI0FE0);
                    }
                    else
                    {
                        timer_input_trigger_source_select(pHandle->timer_cfg->timer_peri,TIMER_SMCFG_TRGSEL_CI1FE1);
                    }
                    timer_slave_mode_select(pHandle->timer_cfg->timer_peri,TIMER_SLAVE_MODE_RESTART);
                    timer_master_slave_mode_config(pHandle->timer_cfg->timer_peri,TIMER_MASTER_SLAVE_MODE_ENABLE);
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

                timer_channel_output_mode_config(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->channel_config[i].chl_num, pHandle->timer_cfg->channel_config[i].oc_mode);
                timer_channel_output_shadow_config(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->channel_config[i].chl_num, TIMER_OC_SHADOW_ENABLE);
                timer_channel_output_pulse_value_config(pHandle->timer_cfg->timer_peri, pHandle->timer_cfg->channel_config[i].chl_num, pHandle->timer_cfg->channel_config[i].oc_duty);
                
                oc_existed = 1;
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


/**
  * @brief  Timer计数器配置.
  * @param  pHandle: Timer需要用户配置的参数
  * @return NULL
  * @note   none
  */
void TimerCntSet(Timer_HandleType *pHandle)
{
    timer_counter_value_config(pHandle->timer_cfg->timer_peri, pHandle->current_cnt);
}

/**
  * @brief  Timer计数器获取.
  * @param  pHandle: Timer需要用户配置的参数
  * @return NULL
  * @note   none
  */
void TimerCntGet(Timer_HandleType *pHandle)
{
    pHandle->current_cnt = timer_counter_read(pHandle->timer_cfg->timer_peri);
}


/**
  * @brief  Timer最大计数获取
  * @param  pHandle: Timer需要用户配置的参数
  * @return NULL
  * @note   none
  */
uint32_t TimPeriodDefaultGet(Timer_HandleType *pHandle)
{
    return pHandle->timer_cfg->period;
}


/**
  * @brief  Timer PWM占空比设置
  * @param  pHandle: Timer需要用户配置的参数
  * @param  duty_particle:占空比颗粒度
  * @return NULL
  * @note   none
  */
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


static uint32_t _NoCapEventTimeCount[TIMER_NUM][TIMER_CHANNEL_NUM];//用于输入捕获未发生时间计数，标记某个定时器某个通道未发生捕获事件的情况，主要用户判断PWM输入检测出现0%和100%占空比的情况



/**
  * @brief  针对输入捕获，计算是否出现占空比出现0或100的情况，并更新频率和占空比的值.
  * @param  pHandle: 需要用户配置的参数
  * @return NULL
  * @note   none
  */
void TimerNoCapEventDetect(Timer_HandleType *pHandle)
{
    uint8_t i = 0;
    for(i = 0; i < TIMER_CHANNEL_NUM; i++)
    {
        if(pHandle->timer_cfg->channel_config[i].chl_num == pHandle->current_chl)
        {
            break;
        }
    }
    if(i >= TIMER_CHANNEL_NUM)
    {
        return;
    }
    
    if((uint32_t)(tim_get_systime() - _NoCapEventTimeCount[pHandle->timerx][pHandle->current_chl]) > 1000)//暂定超时1S
    {
        if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0)
        {
            pHandle->pwm_capture_duty[pHandle->current_chl] = 0;
            pHandle->pwm_capture_freq[pHandle->current_chl] = 0;
        }
        else
        {
            pHandle->pwm_capture_duty[pHandle->current_chl] = 100;
            pHandle->pwm_capture_freq[pHandle->current_chl] = 0;
        }
    }
    else
    {
        return;
    }
}




static uint32_t _ic1value[TIMER_NUM], _ic2value[TIMER_NUM];//用于双通道采样PWM的机制
static uint8_t _timeUpFlag[TIMER_NUM][TIMER_CHANNEL_NUM], _timeDownFlag[TIMER_NUM][TIMER_CHANNEL_NUM];//用于单通道采样PWM的机制
static uint32_t _timeUpNum[TIMER_NUM][TIMER_CHANNEL_NUM], _timeDownNum[TIMER_NUM][TIMER_CHANNEL_NUM];//用于单通道采样PWM的机制


  /**
  * @brief  Timer中断处理函数.
  * @param  pHandle: Timer需要用户配置的参数
  * @return NULL
  * @note   none
  */
void TimerIrqProcess(Timer_HandleType *pHandle)
{
    uint8_t clk_div = 1;//clockdivision对应的值
    
    if(pHandle->timer_cfg->clockdivision == TIMER_CKDIV_DIV1)
    {
        clk_div = 1;
    }
    else if(pHandle->timer_cfg->clockdivision == TIMER_CKDIV_DIV2)
    {
        clk_div = 2;
    }
    else
    {
        clk_div = 4;
    }
    
    if(timer_interrupt_flag_get(pHandle->timer_cfg->timer_peri, TIMER_INT_FLAG_UP) == SET)
    {
        timer_interrupt_flag_clear(pHandle->timer_cfg->timer_peri, TIMER_INT_FLAG_UP);

        //
        if(pHandle->irq_process != NULL)
        {
            pHandle->irq_process();
        }
    }
    else if(timer_interrupt_flag_get(pHandle->timer_cfg->timer_peri,TIMER_INT_FLAG_CH0))
    {
        uint16_t default_icpolarity;
        uint8_t i = 0;
        
        timer_interrupt_flag_clear(pHandle->timer_cfg->timer_peri, TIMER_INT_FLAG_CH0);
        for(i = 0; i < TIMER_CHANNEL_NUM; i++)
        {
            if(pHandle->timer_cfg->channel_config[i].chl_num == TIMER_CH_0 && pHandle->timer_cfg->channel_config[i].ic_or_oc == 0)
            {
                break;
            }
        }
        if(i >= TIMER_CHANNEL_NUM)
        {
            return;
        }
        default_icpolarity = pHandle->timer_cfg->channel_config[i].icpolarity;
        if(pHandle->timer_cfg->dual_pwm_cap_mode == true)//采用两个内部通道的方式
        {
            /*刷新捕获触发超时标志*/
            _NoCapEventTimeCount[pHandle->timerx][0] = tim_get_systime();
            _NoCapEventTimeCount[pHandle->timerx][1] = tim_get_systime();
            /*读取通道0触发(边沿)时的计数器值，即整个周期的计数*/
            _ic1value[pHandle->timerx] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri, TIMER_CH_0) + 1;
            if(_ic1value[pHandle->timerx] != 0)
            {
                /*读取通道1触发(边沿)时的计数器值，如果默认是TIMER_IC_POLARITY_RISING，即高电平持续段的计数，如果默认是TIMER_IC_POLARITY_FALLING，即低电平持续段的计数*/
                _ic2value[pHandle->timerx] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri, TIMER_CH_1) + 1;
                if(default_icpolarity == TIMER_IC_POLARITY_RISING)
                {
                    pHandle->pwm_capture_duty[0] = ((float)_ic2value[pHandle->timerx] * 100) / _ic1value[pHandle->timerx];
                    pHandle->pwm_capture_freq[0] = (_gHCLK / (pHandle->timer_cfg->prescaler + 1) / clk_div) / _ic1value[pHandle->timerx];
                }
                else
                {
                    pHandle->pwm_capture_duty[0] = ((float)(_ic1value[pHandle->timerx] - _ic2value[pHandle->timerx]) * 100) / _ic1value[pHandle->timerx];
                    pHandle->pwm_capture_freq[0] = (_gHCLK / (pHandle->timer_cfg->prescaler + 1) / clk_div) / _ic1value[pHandle->timerx];
                }
            }
            else
            {
                pHandle->pwm_capture_duty[0] = 0;
                pHandle->pwm_capture_freq[0] = 0;
            }
        }
        else if(pHandle->timer_cfg->quadrature_decoder_mode_enable == false)//普通的单通道输入捕获
        {
            timer_ic_parameter_struct   timer_icinitpara;
            
            timer_icinitpara.icpolarity         = pHandle->timer_cfg->channel_config[i].icpolarity;
            timer_icinitpara.icselection        = pHandle->timer_cfg->channel_config[i].icselection;
            timer_icinitpara.icprescaler        = pHandle->timer_cfg->channel_config[i].icprescaler;
            timer_icinitpara.icfilter           = pHandle->timer_cfg->channel_config[i].icfilter;
            
            /*刷新捕获触发超时标志*/
            _NoCapEventTimeCount[pHandle->timerx][0] = tim_get_systime();
            
            if(default_icpolarity == TIMER_IC_POLARITY_RISING)
            {
                if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) && 
                    _timeUpFlag[pHandle->timerx][0] == 0) //第一次上升
                {
                    _timeUpFlag[pHandle->timerx][0] = 1;//只做第一次检测用
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_FALLING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_0,&timer_icinitpara); 
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0 && 
                        _timeDownFlag[pHandle->timerx][0] == 0) //下降
                {
                    _timeDownNum[pHandle->timerx][0] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri,TIMER_CH_0) + 1;//读取捕获计数，这个时间即为高电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_0,&timer_icinitpara); 
                    _timeDownFlag[pHandle->timerx][0] = 1;
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) && 
                        _timeDownFlag[pHandle->timerx][0] == 1)//第二次之后上升,完整的一个周期完成
                {
                    _timeUpNum[pHandle->timerx][0] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri, TIMER_CH_0) + 1;//读取捕获计数，这个时间即为高电平+低电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_FALLING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_0,&timer_icinitpara); 
                    _timeDownFlag[pHandle->timerx][0] = 0;
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                    
                    pHandle->pwm_capture_duty[0] = ((float)_timeDownNum[pHandle->timerx][0] * 100) / _timeUpNum[pHandle->timerx][0]; //高电平持续时间 * 100 / 整个周期持续时间
                    pHandle->pwm_capture_freq[0] = (_gHCLK / (pHandle->timer_cfg->prescaler + 1) / clk_div) / _timeUpNum[pHandle->timerx][0];
                }
            }
            else if(default_icpolarity == TIMER_IC_POLARITY_FALLING)
            {
                if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0 && 
                    _timeDownFlag[pHandle->timerx][0] == 0) //第一次下降
                {
                    _timeDownFlag[pHandle->timerx][0] = 1;//只做第一次检测用
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_0,&timer_icinitpara); 
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) && 
                    _timeUpFlag[pHandle->timerx][0] == 0)//上升
                {
                    _timeUpNum[pHandle->timerx][0] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri,TIMER_CH_0) + 1;//读取捕获计数，这个时间即为低电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_FALLING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_0,&timer_icinitpara); 
                    _timeUpFlag[pHandle->timerx][0] = 1;
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0 && 
                        _timeUpFlag[pHandle->timerx][0] == 1)//第二次之后下降,完整的一个周期完成
                {
                    _timeDownNum[pHandle->timerx][0] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri, TIMER_CH_0) + 1;//读取捕获计数，这个时间即为低电平+高电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_0,&timer_icinitpara); 
                    _timeUpFlag[pHandle->timerx][0] = 0;
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                    
                    pHandle->pwm_capture_duty[0] = ((float)(_timeDownNum[pHandle->timerx][0] - _timeUpNum[pHandle->timerx][0]) * 100) / _timeDownNum[pHandle->timerx][0]; //高电平持续时间 * 100 / 整个周期持续时间
                    pHandle->pwm_capture_freq[0] = (_gHCLK / (pHandle->timer_cfg->prescaler + 1) / clk_div) / _timeDownNum[pHandle->timerx][0];
                }
            }
        }
    }
    else if(timer_interrupt_flag_get(pHandle->timer_cfg->timer_peri,TIMER_INT_FLAG_CH1))
    {
        uint16_t default_icpolarity;
        uint8_t i = 0;
        
        timer_interrupt_flag_clear(pHandle->timer_cfg->timer_peri, TIMER_INT_FLAG_CH1);
        for(i = 0; i < TIMER_CHANNEL_NUM; i++)
        {
            if(pHandle->timer_cfg->channel_config[i].chl_num == TIMER_CH_1 && pHandle->timer_cfg->channel_config[i].ic_or_oc == 0)
            {
                break;
            }
        }
        if(i >= TIMER_CHANNEL_NUM)
        {
            return;
        }
        default_icpolarity = pHandle->timer_cfg->channel_config[i].icpolarity;
        if(pHandle->timer_cfg->dual_pwm_cap_mode == true)//采用两个内部通道的方式
        {
            /*刷新捕获触发超时标志*/
            _NoCapEventTimeCount[pHandle->timerx][0] = tim_get_systime();
            _NoCapEventTimeCount[pHandle->timerx][1] = tim_get_systime();
            /*读取通道1触发(边沿)时的计数器值，即整个周期的计数*/
            _ic1value[pHandle->timerx] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri, TIMER_CH_1) + 1;
            if(_ic1value[pHandle->timerx] != 0)
            {
                /*读取通道0触发(边沿)时的计数器值，如果默认是TIMER_IC_POLARITY_RISING，即高电平持续段的计数，如果默认是TIMER_IC_POLARITY_FALLING，即低电平持续段的计数*/
                _ic2value[pHandle->timerx] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri, TIMER_CH_0) + 1;
                if(default_icpolarity == TIMER_IC_POLARITY_RISING)
                {
                    pHandle->pwm_capture_duty[1] = ((float)_ic2value[pHandle->timerx] * 100) / _ic1value[pHandle->timerx];
                    pHandle->pwm_capture_freq[1] = (_gHCLK / (pHandle->timer_cfg->prescaler + 1) / clk_div) / _ic1value[pHandle->timerx];
                }
                else
                {
                    pHandle->pwm_capture_duty[1] = ((float)(_ic1value[pHandle->timerx] - _ic2value[pHandle->timerx]) * 100) / _ic1value[pHandle->timerx];
                    pHandle->pwm_capture_freq[1] = (_gHCLK / (pHandle->timer_cfg->prescaler + 1) / clk_div) / _ic1value[pHandle->timerx];
                }
            }
            else
            {
                pHandle->pwm_capture_duty[1] = 0;
                pHandle->pwm_capture_freq[1] = 0;
            }
        }
        else if(pHandle->timer_cfg->quadrature_decoder_mode_enable == false)//普通的单通道输入捕获
        {
            timer_ic_parameter_struct   timer_icinitpara;
            
            timer_icinitpara.icpolarity         = pHandle->timer_cfg->channel_config[i].icpolarity;
            timer_icinitpara.icselection        = pHandle->timer_cfg->channel_config[i].icselection;
            timer_icinitpara.icprescaler        = pHandle->timer_cfg->channel_config[i].icprescaler;
            timer_icinitpara.icfilter           = pHandle->timer_cfg->channel_config[i].icfilter;
            
            /*刷新捕获触发超时标志*/
            _NoCapEventTimeCount[pHandle->timerx][1] = tim_get_systime();
            
            if(default_icpolarity == TIMER_IC_POLARITY_RISING)
            {
                if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) && 
                    _timeUpFlag[pHandle->timerx][1] == 0) //第一次上升
                {
                    _timeUpFlag[pHandle->timerx][1] = 1;//只做第一次检测用
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_FALLING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_1,&timer_icinitpara); 
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0 && 
                        _timeDownFlag[pHandle->timerx][1] == 0) //下降
                {
                    _timeDownNum[pHandle->timerx][1] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri,TIMER_CH_1) + 1;//读取捕获计数，这个时间即为高电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_1,&timer_icinitpara); 
                    _timeDownFlag[pHandle->timerx][1] = 1;
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) && 
                        _timeDownFlag[pHandle->timerx][1] == 1)//第二次之后上升,完整的一个周期完成
                {
                    _timeUpNum[pHandle->timerx][1] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri, TIMER_CH_1) + 1;//读取捕获计数，这个时间即为高电平+低电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_FALLING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_1,&timer_icinitpara); 
                    _timeDownFlag[pHandle->timerx][1] = 0;
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                    
                    pHandle->pwm_capture_duty[1] = ((float)_timeDownNum[pHandle->timerx][1] * 100) / _timeUpNum[pHandle->timerx][1]; //高电平持续时间 * 100 / 整个周期持续时间
                    pHandle->pwm_capture_freq[1] = (_gHCLK / (pHandle->timer_cfg->prescaler + 1) / clk_div) / _timeUpNum[pHandle->timerx][1];
                }
            }
            else if(default_icpolarity == TIMER_IC_POLARITY_FALLING)
            {
                if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0 && 
                    _timeDownFlag[pHandle->timerx][1] == 0) //第一次下降
                {
                    _timeDownFlag[pHandle->timerx][1] = 1;//只做第一次检测用
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_1,&timer_icinitpara); 
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) && 
                    _timeUpFlag[pHandle->timerx][1] == 0)//上升
                {
                    _timeUpNum[pHandle->timerx][1] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri,TIMER_CH_1) + 1;//读取捕获计数，这个时间即为低电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_FALLING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_1,&timer_icinitpara); 
                    _timeUpFlag[pHandle->timerx][1] = 1;
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0 && 
                        _timeUpFlag[pHandle->timerx][1] == 1)//第二次之后下降,完整的一个周期完成
                {
                    _timeDownNum[pHandle->timerx][1] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri, TIMER_CH_1) + 1;//读取捕获计数，这个时间即为低电平+高电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_1,&timer_icinitpara); 
                    _timeUpFlag[pHandle->timerx][1] = 0;
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                    
                    pHandle->pwm_capture_duty[1] = ((float)(_timeDownNum[pHandle->timerx][1] - _timeUpNum[pHandle->timerx][1]) * 100) / _timeDownNum[pHandle->timerx][1]; //高电平持续时间 * 100 / 整个周期持续时间
                    pHandle->pwm_capture_freq[1] = (_gHCLK / (pHandle->timer_cfg->prescaler + 1) / clk_div) / _timeDownNum[pHandle->timerx][1];
                }
            }
        }
    }
    else if(timer_interrupt_flag_get(pHandle->timer_cfg->timer_peri,TIMER_INT_FLAG_CH2))
    {
        uint16_t default_icpolarity;
        uint8_t i = 0;
        
        timer_interrupt_flag_clear(pHandle->timer_cfg->timer_peri, TIMER_INT_FLAG_CH2);
        for(i = 0; i < TIMER_CHANNEL_NUM; i++)
        {
            if(pHandle->timer_cfg->channel_config[i].chl_num == TIMER_CH_2 && pHandle->timer_cfg->channel_config[i].ic_or_oc == 0)
            {
                break;
            }
        }
        if(i >= TIMER_CHANNEL_NUM)
        {
            return;
        }
        default_icpolarity = pHandle->timer_cfg->channel_config[i].icpolarity;
        if(pHandle->timer_cfg->dual_pwm_cap_mode == true)//采用两个内部通道的方式
        {
            /*后两个通道不支持双通道检测模式*/
			return;
        }
        else if(pHandle->timer_cfg->quadrature_decoder_mode_enable == false)//普通的单通道输入捕获
        {
            timer_ic_parameter_struct   timer_icinitpara;
            
            timer_icinitpara.icpolarity         = pHandle->timer_cfg->channel_config[i].icpolarity;
            timer_icinitpara.icselection        = pHandle->timer_cfg->channel_config[i].icselection;
            timer_icinitpara.icprescaler        = pHandle->timer_cfg->channel_config[i].icprescaler;
            timer_icinitpara.icfilter           = pHandle->timer_cfg->channel_config[i].icfilter;
            
            /*刷新捕获触发超时标志*/
            _NoCapEventTimeCount[pHandle->timerx][2] = tim_get_systime();
            
            if(default_icpolarity == TIMER_IC_POLARITY_RISING)
            {
                if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) && 
                    _timeUpFlag[pHandle->timerx][2] == 0) //第一次上升
                {
                    _timeUpFlag[pHandle->timerx][2] = 1;//只做第一次检测用
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_FALLING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_2,&timer_icinitpara); 
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0 && 
                        _timeDownFlag[pHandle->timerx][2] == 0) //下降
                {
                    _timeDownNum[pHandle->timerx][2] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri,TIMER_CH_2) + 1;//读取捕获计数，这个时间即为高电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_2,&timer_icinitpara); 
                    _timeDownFlag[pHandle->timerx][2] = 1;
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) && 
                        _timeDownFlag[pHandle->timerx][2] == 1)//第二次之后上升,完整的一个周期完成
                {
                    _timeUpNum[pHandle->timerx][2] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri, TIMER_CH_2) + 1;//读取捕获计数，这个时间即为高电平+低电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_FALLING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_2,&timer_icinitpara); 
                    _timeDownFlag[pHandle->timerx][2] = 0;
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                    
                    pHandle->pwm_capture_duty[2] = ((float)_timeDownNum[pHandle->timerx][2] * 100) / _timeUpNum[pHandle->timerx][2]; //高电平持续时间 * 100 / 整个周期持续时间
                    pHandle->pwm_capture_freq[2] = (_gHCLK / (pHandle->timer_cfg->prescaler + 1) / clk_div) / _timeUpNum[pHandle->timerx][2];
                }
            }
            else if(default_icpolarity == TIMER_IC_POLARITY_FALLING)
            {
                if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0 && 
                    _timeDownFlag[pHandle->timerx][2] == 0) //第一次下降
                {
                    _timeDownFlag[pHandle->timerx][2] = 1;//只做第一次检测用
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_2,&timer_icinitpara); 
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) && 
                    _timeUpFlag[pHandle->timerx][2] == 0)//上升
                {
                    _timeUpNum[pHandle->timerx][2] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri,TIMER_CH_2) + 1;//读取捕获计数，这个时间即为低电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_FALLING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_2,&timer_icinitpara); 
                    _timeUpFlag[pHandle->timerx][2] = 1;
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0 && 
                        _timeUpFlag[pHandle->timerx][2] == 1)//第二次之后下降,完整的一个周期完成
                {
                    _timeDownNum[pHandle->timerx][2] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri, TIMER_CH_2) + 1;//读取捕获计数，这个时间即为低电平+高电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_2,&timer_icinitpara); 
                    _timeUpFlag[pHandle->timerx][2] = 0;
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                    
                    pHandle->pwm_capture_duty[2] = ((float)(_timeDownNum[pHandle->timerx][2] - _timeUpNum[pHandle->timerx][2]) * 100) / _timeDownNum[pHandle->timerx][2]; //高电平持续时间 * 100 / 整个周期持续时间
                    pHandle->pwm_capture_freq[2] = (_gHCLK / (pHandle->timer_cfg->prescaler + 1) / clk_div) / _timeDownNum[pHandle->timerx][2];
                }
            }
        }
    }
    else if(timer_interrupt_flag_get(pHandle->timer_cfg->timer_peri,TIMER_INT_FLAG_CH3))
    {
        uint16_t default_icpolarity;
        uint8_t i = 0;
        
        timer_interrupt_flag_clear(pHandle->timer_cfg->timer_peri, TIMER_INT_FLAG_CH3);
        for(i = 0; i < TIMER_CHANNEL_NUM; i++)
        {
            if(pHandle->timer_cfg->channel_config[i].chl_num == TIMER_CH_3 && pHandle->timer_cfg->channel_config[i].ic_or_oc == 0)
            {
                break;
            }
        }
        if(i >= TIMER_CHANNEL_NUM)
        {
            return;
        }
        default_icpolarity = pHandle->timer_cfg->channel_config[i].icpolarity;
        if(pHandle->timer_cfg->dual_pwm_cap_mode == true)//采用两个内部通道的方式
        {
            /*后两个通道不支持双通道检测模式*/
			return;
        }
        else if(pHandle->timer_cfg->quadrature_decoder_mode_enable == false)//普通的单通道输入捕获
        {
            timer_ic_parameter_struct   timer_icinitpara;
            
            timer_icinitpara.icpolarity         = pHandle->timer_cfg->channel_config[i].icpolarity;
            timer_icinitpara.icselection        = pHandle->timer_cfg->channel_config[i].icselection;
            timer_icinitpara.icprescaler        = pHandle->timer_cfg->channel_config[i].icprescaler;
            timer_icinitpara.icfilter           = pHandle->timer_cfg->channel_config[i].icfilter;
            
            /*刷新捕获触发超时标志*/
            _NoCapEventTimeCount[pHandle->timerx][3] = tim_get_systime();
            
            if(default_icpolarity == TIMER_IC_POLARITY_RISING)
            {
                if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) && 
                    _timeUpFlag[pHandle->timerx][3] == 0) //第一次上升
                {
                    _timeUpFlag[pHandle->timerx][3] = 1;//只做第一次检测用
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_FALLING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_3,&timer_icinitpara); 
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0 && 
                        _timeDownFlag[pHandle->timerx][3] == 0) //下降
                {
                    _timeDownNum[pHandle->timerx][3] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri,TIMER_CH_3) + 1;//读取捕获计数，这个时间即为高电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_3,&timer_icinitpara); 
                    _timeDownFlag[pHandle->timerx][3] = 1;
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) && 
                        _timeDownFlag[pHandle->timerx][3] == 1)//第二次之后上升,完整的一个周期完成
                {
                    _timeUpNum[pHandle->timerx][3] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri, TIMER_CH_3) + 1;//读取捕获计数，这个时间即为高电平+低电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_FALLING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_3,&timer_icinitpara); 
                    _timeDownFlag[pHandle->timerx][3] = 0;
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                    
                    pHandle->pwm_capture_duty[3] = ((float)_timeDownNum[pHandle->timerx][3] * 100) / _timeUpNum[pHandle->timerx][3]; //高电平持续时间 * 100 / 整个周期持续时间
                    pHandle->pwm_capture_freq[3] = (_gHCLK / (pHandle->timer_cfg->prescaler + 1) / clk_div) / _timeUpNum[pHandle->timerx][3];
                }
            }
            else if(default_icpolarity == TIMER_IC_POLARITY_FALLING)
            {
                if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0 && 
                    _timeDownFlag[pHandle->timerx][3] == 0) //第一次下降
                {
                    _timeDownFlag[pHandle->timerx][3] = 1;//只做第一次检测用
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_3,&timer_icinitpara); 
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) && 
                    _timeUpFlag[pHandle->timerx][3] == 0)//上升
                {
                    _timeUpNum[pHandle->timerx][3] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri,TIMER_CH_3) + 1;//读取捕获计数，这个时间即为低电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_FALLING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_3,&timer_icinitpara); 
                    _timeUpFlag[pHandle->timerx][3] = 1;
                }
                else if(gpio_input_bit_get(pHandle->timer_cfg->channel_config[i].timer_ch_gpiox, pHandle->timer_cfg->channel_config[i].timer_ch_gpio_pinx) == 0 && 
                        _timeUpFlag[pHandle->timerx][3] == 1)//第二次之后下降,完整的一个周期完成
                {
                    _timeDownNum[pHandle->timerx][3] = timer_channel_capture_value_register_read(pHandle->timer_cfg->timer_peri, TIMER_CH_3) + 1;//读取捕获计数，这个时间即为低电平+高电平持续的时间
                    timer_icinitpara.icpolarity = TIMER_IC_POLARITY_RISING;//改变触发边沿
                    timer_input_capture_config(pHandle->timer_cfg->timer_peri,TIMER_CH_3,&timer_icinitpara); 
                    _timeUpFlag[pHandle->timerx][3] = 0;
                    timer_counter_value_config(pHandle->timer_cfg->timer_peri , 0); // 计数清零，从头开始计
                    
                    pHandle->pwm_capture_duty[3] = ((float)(_timeDownNum[pHandle->timerx][3] - _timeUpNum[pHandle->timerx][3]) * 100) / _timeDownNum[pHandle->timerx][3]; //高电平持续时间 * 100 / 整个周期持续时间
                    pHandle->pwm_capture_freq[3] = (_gHCLK / (pHandle->timer_cfg->prescaler + 1) / clk_div) / _timeDownNum[pHandle->timerx][3];
                }
            }
        }
    }
}













