#ifndef __TIMER_H_
#define __TIMER_H_



#include "gd32f30x_adc.h"
#include "stdbool.h"
#include "drv_opt.h"

typedef enum
{
#if USE_TIMER0
    TIMER0_PERI,
#endif
#if USE_TIMER1
    TIMER1_PERI,
#endif
#if USE_TIMER2
    TIMER2_PERI,
#endif
#if USE_TIMER3
    TIMER3_PERI,
#endif
#if USE_TIMER4
    TIMER4_PERI,
#endif
#if USE_TIMER5
    TIMER5_PERI,
#endif
#if USE_TIMER6
    TIMER6_PERI,
#endif
#if USE_TIMER7
    TIMER7_PERI,
#endif
#if USE_TIMER8
    TIMER8_PERI,
#endif
#if USE_TIMER9
    TIMER9_PERI,
#endif
#if USE_TIMER10
    TIMER10_PERI,
#endif
#if USE_TIMER11
    TIMER11_PERI,
#endif
#if USE_TIMER12
    TIMER12_PERI,
#endif
#if USE_TIMER13
    TIMER13_PERI,
#endif
    TIMER_NUM
} EN_TIMERx_PERI;

#define TIMER_CHANNEL_NUM   4

typedef struct _timer_config_channel_stru_t
{
    bool                    enable;
    uint8_t                 chl_num;
    uint8_t                 ic_or_oc;/*0表示IC，1表示OC*/
    uint32_t                timer_ch_gpiox;
    uint32_t                timer_ch_gpio_pinx;

    uint16_t                outputstate;
    uint16_t                outputnstate;
    uint16_t                ocpolarity;
    uint16_t                ocnpolarity;
    uint16_t                ocidlestate;
    uint16_t                ocnidlestate;
    uint16_t                oc_mode;
    uint16_t                default_duty;

    uint16_t                icpolarity;
    uint16_t                icselection;
    uint16_t                icprescaler;
    uint16_t                icfilter;
} Timer_Config_Channel;


typedef struct _timer_config_stru_t
{
    Timer_Config_Channel    channel_config[TIMER_CHANNEL_NUM];
    rcu_periph_enum         rcu_timer;
    uint32_t                timer_peri;

    uint16_t                prescaler;
    uint16_t                alignedmode;
    uint16_t                counterdirection;
    uint16_t                clockdivision;
    uint32_t                period;
    uint8_t                 repetitioncounter;

    bool                    quadrature_decoder_mode_enable;

    bool                    en_irq;
    uint32_t                nvic_irq;
    uint8_t                 irq_prio;
    uint8_t                 irq_sub_prio;
    uint8_t                 irq_type;

} Timer_Config;


typedef struct _timer_handle_stru_t
{
    EN_TIMERx_PERI          timerx;
    Timer_Config            *timer_cfg;

    uint8_t                 current_chl;
    uint32_t                current_cnt;/*读写均用*/
    uint32_t                oc_pwm_duty;

    void                    (*irq_process)();
} Timer_HandleType;

#define TIMER_INT_FLAG(int)     int


void TimerInit(Timer_HandleType *pHandle);
void TimerCntSet(Timer_HandleType *pHandle);
uint32_t TimPeriodDefaultGet(Timer_HandleType *pHandle);
void TimerPWMDutySet(Timer_HandleType *pHandle);
void TimerIrqProcess(Timer_HandleType *pHandle);

#endif
