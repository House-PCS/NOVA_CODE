/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.c
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



#include "mcu_bsp.h"
//#include "protocol_can.h"
#include "systick.h"



#define LOG_TAG     "bsp"
/******************AFE SPI1*********************/
//#define AFE_SPI1      //从左到右
#ifdef AFE_SPI1
static Spi_Config _AFESpiCfg =
{
    .sck_gpiox              = GPIOA,
    .sck_gpio_pinx          = GPIO_PIN_5,

    .mosi_gpiox             = GPIOA,
    .mosi_gpio_pinx         = GPIO_PIN_7,

    .miso_gpiox             = GPIOA,
    .miso_gpio_pinx         = GPIO_PIN_6,

    .nss_gpiox              = GPIOA,
    .nss_gpio_pinx          = GPIO_PIN_4,

    .rcu_spi                = RCU_SPI0,
    .spi_peri               = SPI0,

    .device_mode            = SPI_MASTER,
    .trans_mode             = SPI_TRANSMODE_FULLDUPLEX,
    .frame_size             = SPI_FRAMESIZE_8BIT,
    .nss                    = SPI_NSS_SOFT,
    .endian                 = SPI_ENDIAN_MSB,
    .clock_polarity_phase   = SPI_CK_PL_LOW_PH_2EDGE,
    .prescale               = SPI_PSC_256,

    .en_irq                 = false,

    .en_dma_tx              = false,
    .en_dma_rx              = false,

    .dma_tx_peri            = DMA0,
    .dma_tx_channel         = DMA_CH4,

    .dma_rx_peri            = DMA0,
    .dma_rx_channel         = DMA_CH3,
};
static Gpio_Config _AFESpiCsCfg =
{
    .gpiox                  = GPIOA,
    .gpio_pinx              = GPIO_PIN_4,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};
/////6822主从模式选择，需要上拉////////////
static Gpio_Config _6822_Mstr =
{
    .gpiox                  = GPIOA,
    .gpio_pinx              = GPIO_PIN_1,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};
#else
static Spi_Config _AFESpiCfg =
{
    .sck_gpiox              = GPIOB,
    .sck_gpio_pinx          = GPIO_PIN_13,

    .mosi_gpiox             = GPIOB,
    .mosi_gpio_pinx         = GPIO_PIN_15,

    .miso_gpiox             = GPIOB,
    .miso_gpio_pinx         = GPIO_PIN_14,

    .nss_gpiox              = GPIOB,
    .nss_gpio_pinx          = GPIO_PIN_12,

    .rcu_spi                = RCU_SPI1,
    .spi_peri               = SPI1,

    .device_mode            = SPI_MASTER,
    .trans_mode             = SPI_TRANSMODE_FULLDUPLEX,
    .frame_size             = SPI_FRAMESIZE_8BIT,
    .nss                    = SPI_NSS_SOFT,
    .endian                 = SPI_ENDIAN_MSB,
    .clock_polarity_phase   = SPI_CK_PL_LOW_PH_2EDGE,
    .prescale               = SPI_PSC_256,

    .en_irq                 = false,

    .en_dma_tx              = false,
    .en_dma_rx              = false,

    .dma_tx_peri            = DMA0,
    .dma_tx_channel         = DMA_CH4,

    .dma_rx_peri            = DMA0,
    .dma_rx_channel         = DMA_CH3,
};
static Gpio_Config _AFESpiCsCfg =
{
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_12,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};
/////6822主从模式选择，需要上拉////////////
static Gpio_Config _6822_Mstr =
{
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_0,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

#endif



Spi_HandleType gSpiAdbms6830Handle;
Gpio_HandleType gCsAdbms6830Handle;
Gpio_HandleType Adbms6830Mstr_Handle;
void SpiAdbms6830BspInit()
{

    gSpiAdbms6830Handle.spix = SPI0_PORT;
    gSpiAdbms6830Handle.default_width = 8;
    gSpiAdbms6830Handle.spi_cfg = &_AFESpiCfg;
    gCsAdbms6830Handle.gpio_cfg = &_AFESpiCsCfg;

    SpiInit(&gSpiAdbms6830Handle);
    GpioInit(&gCsAdbms6830Handle);
    
    Adbms6830Mstr_Handle.gpio_cfg = &_6822_Mstr;
    GpioInit(&Adbms6830Mstr_Handle);
    GpioBitWrite((&Adbms6830Mstr_Handle),SET);
}


static Spi_Config FlashSpi_Cfg =
{
    .sck_gpiox              = GPIOB,
    .sck_gpio_pinx          = GPIO_PIN_3,

    .mosi_gpiox             = GPIOB,
    .mosi_gpio_pinx         = GPIO_PIN_5,

    .miso_gpiox             = GPIOB,
    .miso_gpio_pinx         = GPIO_PIN_4,

//    .nss_gpiox              = GPIOB,
//    .nss_gpio_pinx          = GPIO_PIN_12,

    .rcu_spi                = RCU_SPI2,
    .spi_peri               = SPI2,

    .device_mode            = SPI_MASTER,
    .trans_mode             = SPI_TRANSMODE_FULLDUPLEX,
    .frame_size             = SPI_FRAMESIZE_8BIT,
    .nss                    = SPI_NSS_SOFT,
    .endian                 = SPI_ENDIAN_MSB,
    .clock_polarity_phase   = SPI_CK_PL_HIGH_PH_2EDGE,
    .prescale               = SPI_PSC_8,

    .en_irq                 = false,

    .en_dma_tx              = false,
    .en_dma_rx              = false,

};
static Gpio_Config SpiFlashCsCfg = 
{
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_1,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};
Spi_HandleType SpiFlashHandle;
Gpio_HandleType SpiFlashCsHandle;
void SpiFlashBspInit()
{
    SpiFlashHandle.spix = SPI2_PORT;
    SpiFlashHandle.default_width = 8;
    SpiFlashHandle.spi_cfg = &FlashSpi_Cfg;
    SpiFlashCsHandle.gpio_cfg = &SpiFlashCsCfg;
//    FlashSpiHandle.gpio_cfg = &_SpiFlashWpCfg;
    
    SpiInit(&SpiFlashHandle);
    GpioInit(&SpiFlashCsHandle);
//    GpioInit(&WpSpiFlashHandle);
}

/******************TIMER(fan_out_pwm)*********************/
static Timer_Config _FanPwmCfg =
{
    .channel_config[0].enable               = true,
    .channel_config[0].chl_num              = TIMER_CH_0,
    .channel_config[0].ic_or_oc             = 1,
    .channel_config[0].timer_ch_gpiox       = GPIOC,
    .channel_config[0].timer_ch_gpio_pinx   = GPIO_PIN_6, 
    .channel_config[0].alt_rmp_mode         = 3,
    .channel_config[0].alt_rmp_function     = GPIO_TIMER2_FULL_REMAP,
    
    .channel_config[0].outputstate          = TIMER_CCX_ENABLE,
    .channel_config[0].outputnstate         = TIMER_CCXN_DISABLE,
    .channel_config[0].ocpolarity           = TIMER_OC_POLARITY_HIGH,
    .channel_config[0].ocnpolarity          = TIMER_OCN_POLARITY_HIGH,
    .channel_config[0].ocidlestate          = TIMER_OC_IDLE_STATE_LOW,
    .channel_config[0].ocnidlestate         = TIMER_OCN_IDLE_STATE_LOW,
    .channel_config[0].oc_mode              = TIMER_OC_MODE_PWM0,
    .channel_config[0].oc_duty              = 0,    //高电平时间
    
    .rcu_timer                              = RCU_TIMER2,
    .timer_peri                             = TIMER2,       /* 120M */
    
    .prescaler                              = 119,   //1MHz,1us
    .alignedmode                            = TIMER_COUNTER_EDGE,
    .counterdirection                       = TIMER_COUNTER_UP,
    .clockdivision                          = TIMER_CKDIV_DIV1,
    .period                                 = 40,   //40us,25KHz
    .repetitioncounter                      = 0,
    
    .quadrature_decoder_mode_enable         = false,
    .en_irq                                 = false,
};

Timer_HandleType gTimer2PwmFanHandle;
void PWMFanOutputBspInit()
{
    gTimer2PwmFanHandle.timerx        = TIMER2_PERI;
    gTimer2PwmFanHandle.timer_cfg     = &_FanPwmCfg;
    gTimer2PwmFanHandle.current_chl   = TIMER_CH_0;
    gTimer2PwmFanHandle.oc_pwm_duty   = 0;
    TimerInit(&gTimer2PwmFanHandle);
    
    TimerPWMDutySet(&gTimer2PwmFanHandle,100);
}

/******************TIMER(fan_rd_pwm_in)*********************/
static Timer_Config _FanRdPwmCfg =
{
    .channel_config[0].enable               = true,
    .channel_config[0].chl_num              = TIMER_CH_0,
    .channel_config[0].ic_or_oc             = 0,
    .channel_config[0].timer_ch_gpiox       = GPIOA,
    .channel_config[0].timer_ch_gpio_pinx   = GPIO_PIN_8, 
    .channel_config[0].alt_rmp_mode         = 2,
//    .channel_config[0].alt_rmp_function     = GPIO_TIMER0_PARTIAL_REMAP,
    .channel_config[0].icpolarity          = TIMER_IC_POLARITY_RISING,
    .channel_config[0].icselection         = TIMER_IC_SELECTION_DIRECTTI,
    .channel_config[0].icprescaler         = TIMER_IC_PSC_DIV1,                 //TIMER_CHCTL0_CH0CAPPSC
    .channel_config[0].icfilter            = 0,
    
    .rcu_timer                              = RCU_TIMER0,
    .timer_peri                             = TIMER0,       /* 120M */
    
    .prescaler                              = 119,//根据AHB时钟变化来变化，确保分配出来是1M
    .alignedmode                            = TIMER_COUNTER_EDGE,
    .counterdirection                       = TIMER_COUNTER_UP,
    .clockdivision                          = TIMER_CKDIV_DIV1,
    .period                                 = 65535,
    .repetitioncounter                      = 0,
    
    .dual_pwm_cap_mode                     = true,//采用双通道采样方式
    .en_irq                                = true,
    .nvic_irq                              = TIMER0_Channel_IRQn,
    .irq_prio                              = 1,
    .irq_sub_prio                          = 1,
    .irq_type                              = TIMER_INT_CH0,
};
Timer_HandleType gTimer0PwmFanRdHandle;
void PWMFanRdInputBspInit()
{
    gTimer0PwmFanRdHandle.timerx        = TIMER0_PERI;
    gTimer0PwmFanRdHandle.timer_cfg     = &_FanRdPwmCfg;
    TimerInit(&gTimer0PwmFanRdHandle);
}





/******************CAN(loopback)*********************/
static Can_Config _Can_Config[EN_DEV_CAN_NUM] = 
{

    [0].tx_gpiox               = GPIOA,
    [0].tx_gpio_pinx           = GPIO_PIN_12,
    [0].rcu_tx_gpio            = RCU_GPIOA,
    [0].rx_gpiox               = GPIOA,
    [0].rx_gpio_pinx           = GPIO_PIN_11,
    [0].rcu_rx_gpio            = RCU_GPIOA,
    [0].alt_rmp_mode           = false,
    [0].alt_rmp_function       = 1,
    
    [0].rcu_can                = RCU_CAN0,
    [0].can_peri               = CAN0,
    [0].en_irq_rx0             = true,
    [0].nvic_irq_rx0           = USBD_LP_CAN0_RX0_IRQn,        
    [0].irq_prio_rx0           = 0,
    [0].irq_sub_prio_rx0       = 1,
    [0].irq_type_rx0           = CAN_INT_RFNE0,
    
    [0].prescaler              = 24,              
    [0].time_segment_1         = CAN_BT_BS1_7TQ,        
    [0].time_segment_2         = CAN_BT_BS2_2TQ, 
    [0].resync_jump_width      = CAN_BT_SJW_1TQ,   
   
};

Can_HandleType gCanHandle[EN_DEV_CAN_NUM];
void CanBspInit(void)
{
    for(uint8_t i = 0; i < EN_DEV_CAN_NUM; i++)     //  CAN_PORT_NUM
    {
        gCanHandle[i].canx = i;
        gCanHandle[i].can_cfg = &_Can_Config[i];
        CanInit(&gCanHandle[i]);
    }
}

//////////////CAN END/////////////////


/*********JACK IO配置**********/
static Gpio_Config _JackIOCfg[4] =
{
    {
        .gpiox                  = GPIOB,
        .gpio_pinx              = GPIO_PIN_8,
        .gpio_mode              = GPIO_MODE_OUT_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOB,
        .gpio_pinx              = GPIO_PIN_9,
        .gpio_mode              = GPIO_MODE_IN_FLOATING,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOC,
        .gpio_pinx              = GPIO_PIN_2,
        .gpio_mode              = GPIO_MODE_OUT_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOC,
        .gpio_pinx              = GPIO_PIN_3,
        .gpio_mode              = GPIO_MODE_IN_FLOATING,
        .en_exti                = false,
    }
};
///***********LED************/
//static Gpio_Config _McuLedCfg =
//{
//    .gpiox                  = GPIOA,
//    .gpio_pinx              = GPIO_PIN_0,
//    .gpio_mode              = GPIO_MODE_OUT_PP,
//    .en_exti                = false,
//};

Gpio_HandleType gJackIOHandle[4];
Gpio_HandleType gMcuLedHandle;
/** 
* @brief  JackIO(用于地址分配) bsp初始化
* @param  none
* @return  none
* @note:
*
*/
void JackIOBspInit(void)
{
    for(uint8_t i = 0; i < 4; i++)
    {
        gJackIOHandle[i].gpio_cfg = &_JackIOCfg[i];
        GpioInit(&gJackIOHandle[i]);
    }
    GpioBitReset(&gJackIOHandle[0]);
    GpioBitReset(&gJackIOHandle[2]);
}

//void McuLedBspInit(void)
//{
//    gMcuLedHandle.gpio_cfg = &_McuLedCfg;
//    GpioInit(&gMcuLedHandle);
//}


//////////////Timer定时器轮询/////////////////
static Timer_Config _TimerPollingCfg = 
{
    .rcu_timer              = RCU_TIMER5,
    .timer_peri             = TIMER5,
    .prescaler              = 240,//根据系统时钟频率变化,如果RCU_CFG1的TIMERSEL为0，APB1 = AHB/4，CK_TIMER = 2xAPB1,所以是系统时钟的一半，如果TIMERSEL为1，则CK_TIMER=AHB
    .alignedmode            = TIMER_COUNTER_EDGE,
    .counterdirection       = TIMER_COUNTER_UP,
    .clockdivision          = TIMER_CKDIV_DIV1,
    .period                 = 10000,//10ms为一周期
    .repetitioncounter      = 0,
    
    .en_irq                 = true,
    .nvic_irq               = TIMER5_IRQn,
    .irq_prio               = 0,
    .irq_sub_prio           = 2,
    .irq_type               = TIMER_INT_UP,
};
Timer_HandleType gTimerPollingHandle;
void TimerPollingBspInit()
{
    gTimerPollingHandle.timerx = TIMER5_PERI;
    gTimerPollingHandle.timer_cfg = &_TimerPollingCfg;
    
    TimerInit(&gTimerPollingHandle);
}

//////////////Timer定时器轮询 End/////////////////


//////////////LED指示灯/////////////////
static Gpio_Config _GpioLedCfg = 
{
    .gpiox                  = GPIOA,
    .gpio_pinx              = GPIO_PIN_0,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};
Gpio_HandleType gGpioLedHandle;
void GpioLedBspInit()
{
    gGpioLedHandle.gpio_cfg = &_GpioLedCfg;
    GpioInit(&gGpioLedHandle);
}

//////////////LED指示灯 End/////////////////

/////////////////////////////////////////////
void BaseBspInit()
{
    DMAClkInit();
    GpioClkInit();
    TimerClkInit();
}




