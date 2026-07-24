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



#include "bsp.h"

#define LOG_TAG     "bsp"

EN_BMS_TYPE geBMSType = EN_BMS_NUM;
/******************AFE SPI1*********************/
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
    .clock_polarity_phase   = SPI_CK_PL_HIGH_PH_2EDGE,
    .prescale               = SPI_PSC_128,

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

Spi_HandleType gSpiAdbms6830Handle;
Gpio_HandleType gCsAdbms6830Handle;

void SpiAdbms6830BspInit(void)
{

    gSpiAdbms6830Handle.spix = SPI0_PORT;
    gSpiAdbms6830Handle.default_width = 8;
    gSpiAdbms6830Handle.spi_cfg = &_AFESpiCfg;
    gCsAdbms6830Handle.gpio_cfg = &_AFESpiCsCfg;

    SpiInit(&gSpiAdbms6830Handle);
    GpioInit(&gCsAdbms6830Handle);
}

/******************UART0 Debug*********************/
static Uart_Config _UartCfg =
{
    .tx_gpiox           = GPIOA,
    .tx_gpio_pinx       = GPIO_PIN_9,
    .rx_gpiox           = GPIOA,
    .rx_gpio_pinx       = GPIO_PIN_10,

    .rcu_uart           = RCU_USART0,
    .uart_peri          = USART0,
    .nvic_irq           = USART0_IRQn,
    .baud_rate          = 115200,
    .wlen               = USART_WL_8BIT,
    .stblen             = USART_STB_1BIT,
    .parity_cfg         = USART_PM_NONE,

    .en_irq             = true,
    .irq_prio           = 1,
    .irq_sub_prio       = 1,
    .irq_type           = USART_INT_RBNE,

    .en_dma_tx          = false,
    .en_dma_rx          = false,

    .dma_tx_peri        = DMA0,
    .dma_tx_channel     = DMA_CH3,

    .dma_rx_peri        = DMA0,
    .dma_rx_channel     = DMA_CH4,
};


Uart_HandleType Uart0_Debug_Handle;

void Uart0DebugBspInit(void)
{
    Uart0_Debug_Handle.uartx = UART0_PORT;
    Uart0_Debug_Handle.uart_cfg = &_UartCfg;

    UartInit(&Uart0_Debug_Handle);
}

/******************UART1 ModS*********************/
static Uart_Config _Uart1Cfg =
{
    .tx_gpiox           = GPIOA,
    .tx_gpio_pinx       = GPIO_PIN_2,
    .rx_gpiox           = GPIOA,
    .rx_gpio_pinx       = GPIO_PIN_3,

    .rcu_uart           = RCU_USART1,
    .uart_peri          = USART1,
    .nvic_irq           = USART1_IRQn,
    .baud_rate          = 115200,
    .wlen               = USART_WL_8BIT,
    .stblen             = USART_STB_1BIT,
    .parity_cfg         = USART_PM_NONE,

    .en_irq             = true,
    .irq_prio           = 1,
    .irq_sub_prio       = 1,
    .irq_type           = USART_INT_IDLE,

    .en_dma_tx          = true,
    .en_dma_rx          = true,
    .en_dma_tx_nowait   = true,

    .dma_tx_peri        = DMA0,
    .dma_tx_channel     = DMA_CH6,
    
    .en_dma_tx_irq      = true,
    .nvic_dma_tx_irq    = DMA0_Channel6_IRQn,    
    .irq_dma_tx_prio    = 1,
    .irq_dma_tx_sub_prio= 1,

    .dma_rx_peri        = DMA0,
    .dma_rx_channel     = DMA_CH5,
};

static Gpio_Config _Modubus485_EN1 = 
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_14,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

Uart_HandleType Uart1_ModS_Handle;
Gpio_HandleType Modbus485En1;

void Uart1ModsBspInit()
{
    Uart1_ModS_Handle.uartx = UART1_PORT;
    Uart1_ModS_Handle.uart_cfg = &_Uart1Cfg;

    UartInit(&Uart1_ModS_Handle);
    
    Modbus485En1.gpio_cfg = &_Modubus485_EN1;
    GpioInit(&Modbus485En1);
    GpioBitReset((&Modbus485En1));//低电平接收 高电平发送
}

void Modbus485RxEn1(void)
{
    GpioBitReset((&Modbus485En1));  
}
void Modbus485TxEn1(void)
{
    GpioBitSet((&Modbus485En1));     
}

/******************UART2 ModS*********************/
static Uart_Config _Uart2Cfg =
{
    .tx_gpiox           = GPIOB,
    .tx_gpio_pinx       = GPIO_PIN_10,
    .rx_gpiox           = GPIOB,
    .rx_gpio_pinx       = GPIO_PIN_11,

    .rcu_uart           = RCU_USART2,
    .uart_peri          = USART2,
    .nvic_irq           = USART2_IRQn,
    .baud_rate          = 115200,
    .wlen               = USART_WL_8BIT,
    .stblen             = USART_STB_1BIT,
    .parity_cfg         = USART_PM_NONE,

    .en_irq             = true,
    .irq_prio           = 1,
    .irq_sub_prio       = 1,
    .irq_type           = USART_INT_IDLE,

    .en_dma_tx          = true,
    .en_dma_rx          = true,
    .en_dma_tx_nowait   = true,

    .dma_tx_peri        = DMA0,
    .dma_tx_channel     = DMA_CH1,
    .en_dma_tx_irq      = true,
    .nvic_dma_tx_irq    = DMA0_Channel1_IRQn,    
    .irq_dma_tx_prio    = 1,
    .irq_dma_tx_sub_prio= 1,

    .dma_rx_peri        = DMA0,
    .dma_rx_channel     = DMA_CH2,
};

static Gpio_Config _Modubus485_EN2 = 
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_15,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

Uart_HandleType Uart2_ModS_Handle;
Gpio_HandleType Modbus485En2;

void Uart2ModsBspInit()
{
    Uart2_ModS_Handle.uartx = UART2_PORT;
    Uart2_ModS_Handle.uart_cfg = &_Uart2Cfg;

    UartInit(&Uart2_ModS_Handle);
    
    Modbus485En2.gpio_cfg = &_Modubus485_EN2;
    GpioInit(&Modbus485En2);
    GpioBitReset((&Modbus485En2));//低电平接收 高电平发送
}

void Modbus485RxEn2(void)
{
    GpioBitReset((&Modbus485En2));  
}
void Modbus485TxEn2(void)
{
    GpioBitSet((&Modbus485En2));     
}

/******************SPI FLASH*********************/
static Spi_Config _SpiFlashSpiCfg =
{
    .sck_gpiox              = GPIOA,
    .sck_gpio_pinx          = GPIO_PIN_5,

    .mosi_gpiox             = GPIOA,
    .mosi_gpio_pinx         = GPIO_PIN_7,

    .miso_gpiox             = GPIOA,
    .miso_gpio_pinx         = GPIO_PIN_6,

    .nss_gpiox              = GPIOA,
    .nss_gpio_pinx          = GPIO_PIN_4,

    .alt_rmp_mode           = 2,

    .rcu_spi                = RCU_SPI0,
    .spi_peri               = SPI0,

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

    .dma_tx_peri            = DMA0,
    .dma_tx_channel         = DMA_CH2,

    .dma_rx_peri            = DMA0,
    .dma_rx_channel         = DMA_CH1,
};

static Gpio_Config _SpiFlashCsCfg =
{
    .gpiox                  = GPIOA,
    .gpio_pinx              = GPIO_PIN_4,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

Spi_HandleType  SpiFlashHandle;
Gpio_HandleType CsSpiFlashHandle;

void SpiFlashBspInit(void)
{
    SpiFlashHandle.spix = SPI0_PORT;
    SpiFlashHandle.default_width = 8;
    SpiFlashHandle.spi_cfg = &_SpiFlashSpiCfg;
    CsSpiFlashHandle.gpio_cfg = &_SpiFlashCsCfg;

    SpiInit(&SpiFlashHandle);
    GpioInit(&CsSpiFlashHandle);
}

/******************SIM_IIC(EEPROM)*********************/
static SimI2C_Config _E2PROMI2C_Config =
{
    .scl_gpiox          = GPIOE,
    .scl_gpio_pinx      = GPIO_PIN_8,
    .sda_gpiox          = GPIOE,
    .sda_gpio_pinx      = GPIO_PIN_7,
};

static Gpio_Config _E2PROMWpCfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_9,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

SimI2C_HandleType   gSimI2CEepromHandleType;
Gpio_HandleType     E2PROMWpHandle;

void SimI2CEepromBspInit(void)
{
    gSimI2CEepromHandleType.simi2c_cfg = &_E2PROMI2C_Config;   
    gSimI2CEepromHandleType.slave_addr0 = 0xA0;
    SimI2CInit(&gSimI2CEepromHandleType);
    
    E2PROMWpHandle.gpio_cfg = &_E2PROMWpCfg;
    GpioInit(&E2PROMWpHandle);  //E2PROM 默认处于解保护状态
}


/******************SIM_IIC(SQ52201)*********************/
static SimI2C_Config _SQ52201I2C_Config =
{
    .scl_gpiox          = GPIOB,
    .scl_gpio_pinx      = GPIO_PIN_6,
    .sda_gpiox          = GPIOB,
    .sda_gpio_pinx      = GPIO_PIN_7,
};

static Gpio_Config _SQ52201AlertCfg =
{
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_8,
    .gpio_mode              = GPIO_MODE_IN_FLOATING,
    .en_exti                = false,
};

static Gpio_Config _SQ52201PowerEnCfg =
{
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_9,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

static Timer_Config _SQ52201SampleTime =
{
    .rcu_timer                              = RCU_TIMER2,
    .timer_peri                             = TIMER2,       /* 60M */

    .prescaler                              = 1199,
    .alignedmode                            = TIMER_COUNTER_EDGE,
    .counterdirection                       = TIMER_COUNTER_UP,
    .clockdivision                          = TIMER_CKDIV_DIV1,
    .period                                 = 19999,          /* 200ms */
    .repetitioncounter                      = 0,

    .quadrature_decoder_mode_enable         = false,
    .en_irq                                 = true,
    .nvic_irq                               = TIMER2_IRQn,
    .irq_prio                               = 2,
    .irq_sub_prio                           = 1,
    .irq_type                               = TIMER_INT_UP,
};
Timer_HandleType SQ52201SampleTimeHandle;

SimI2C_HandleType   SQ52201Handle;
Gpio_HandleType     SQ52201AlertHandle;
Gpio_HandleType     SQ52201PowerEnHandle;

void SQ52201BspInit(void)
{
    SQ52201Handle.simi2c_cfg = &_SQ52201I2C_Config;
    SQ52201Handle.slave_addr0 = 0x80;  //iic地址需查阅手册
    SQ52201Handle.reg_addr_width = EN_SIMI2C_REG_ADDR_WIDTH_8;
    SimI2CInit(&SQ52201Handle);
    
    SQ52201AlertHandle.gpio_cfg = &_SQ52201AlertCfg;
    SQ52201PowerEnHandle.gpio_cfg = &_SQ52201PowerEnCfg;

    GpioInit(&SQ52201AlertHandle);
    GpioInit(&SQ52201PowerEnHandle);
    GpioBitSet((&SQ52201PowerEnHandle));     //芯片默认供电
    
    SQ52201SampleTimeHandle.timerx = TIMER2_PERI;
    SQ52201SampleTimeHandle.timer_cfg = &_SQ52201SampleTime;
    TimerInit(&SQ52201SampleTimeHandle);
    

}

/******************ADC(board id)*********************/
static Adc_Config _BoardIdCfg =
{
    .channel_config[0].enable         = true,
    .channel_config[0].chl_num        = ADC_CHANNEL_15,         //Board id
    .channel_config[0].adc_gpiox      = GPIOC,
    .channel_config[0].adc_gpio_pinx  = GPIO_PIN_5,
    .rcu_adc                          = RCU_ADC1,
    .adc_peri                         = ADC1,
    .en_irq                           = false,
    .nvic_irq                         = ADC0_1_IRQn,
    .irq_prio                         = 1,
    .irq_sub_prio                     = 1,
};

static Gpio_Config _IDPowerEn1Cfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_10,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};
Adc_HandleType  BoardIDAdcHandle;
Gpio_HandleType IDPowerEn1Handle;

void BoardIdBspInit()
{
    uint8_t c_current_chl = ADC_CHANNEL_15;
    float RValtage = 0;
    
    IDPowerEn1Handle.gpio_cfg = &_IDPowerEn1Cfg;

    GpioInit(&IDPowerEn1Handle);
    GpioBitSet((&IDPowerEn1Handle));     //打开 ID EN1
    
    
    BoardIDAdcHandle.adc_cfg = &_BoardIdCfg;
    BoardIDAdcHandle.current_chl = c_current_chl;
    BoardIDAdcHandle.pRValtage = &RValtage;
    AdcInit(&BoardIDAdcHandle);
    AdcVoltageGet(&BoardIDAdcHandle);

    if(RValtage < 1.0f)
    {
        geBMSType = EN_BMS_A0;
    }
    else
    {
        geBMSType = EN_BMS_A1;
    }

    LOG_INFO(LOG_TAG, "Board id: %7.3fV", RValtage);
    
    GpioBitReset((&IDPowerEn1Handle));     //关闭 ID EN1
}
/******************ADC*********************/
static Adc_Config _AdcCfg =
{
    .channel_config[0].enable         = true,
    .channel_config[0].chl_num        = ADC_CHANNEL_15,         //Board id
    .channel_config[0].adc_gpiox      = GPIOC,
    .channel_config[0].adc_gpio_pinx  = GPIO_PIN_5,
    .channel_config[1].enable         = true,
    .channel_config[1].chl_num        = ADC_CHANNEL_14,         //Mos ntc
    .channel_config[1].adc_gpiox      = GPIOC,
    .channel_config[1].adc_gpio_pinx  = GPIO_PIN_4,
    .channel_config[2].enable         = true,
    .channel_config[2].chl_num        = ADC_CHANNEL_13,         //环境 ntc
    .channel_config[2].adc_gpiox      = GPIOC,
    .channel_config[2].adc_gpio_pinx  = GPIO_PIN_3,
    .channel_config[3].enable         = true,
    .channel_config[3].chl_num        = ADC_CHANNEL_12,         //AFE ntc
    .channel_config[3].adc_gpiox      = GPIOC,
    .channel_config[3].adc_gpio_pinx  = GPIO_PIN_2,
    .channel_config[4].enable         = true,
    .channel_config[4].chl_num        = ADC_CHANNEL_11,         //Vbat 采样
    .channel_config[4].adc_gpiox      = GPIOC,
    .channel_config[4].adc_gpio_pinx  = GPIO_PIN_1,
    .channel_config[5].enable         = true,
    .channel_config[5].chl_num        = ADC_CHANNEL_10,         //OVC 采样
    .channel_config[5].adc_gpiox      = GPIOC,
    .channel_config[5].adc_gpio_pinx  = GPIO_PIN_0,
    .channel_config[6].enable         = true,
    .channel_config[6].chl_num        = ADC_CHANNEL_0,         //Cell NTC1 采样
    .channel_config[6].adc_gpiox      = GPIOA,
    .channel_config[6].adc_gpio_pinx  = GPIO_PIN_0,
    .channel_config[7].enable         = true,
    .channel_config[7].chl_num        = ADC_CHANNEL_1,         //Cell NTC2 采样
    .channel_config[7].adc_gpiox      = GPIOA,
    .channel_config[7].adc_gpio_pinx  = GPIO_PIN_1,
    .channel_config[8].enable         = true,
    .channel_config[8].chl_num        = ADC_CHANNEL_8,         //Cell NTC3 采样
    .channel_config[8].adc_gpiox      = GPIOB,
    .channel_config[8].adc_gpio_pinx  = GPIO_PIN_0,
    .channel_config[9].enable         = true,
    .channel_config[9].chl_num        = ADC_CHANNEL_9,         //Cell NTC4 采样
    .channel_config[9].adc_gpiox      = GPIOB,
    .channel_config[9].adc_gpio_pinx  = GPIO_PIN_1,
    
    .rcu_adc                          = RCU_ADC1,
    .adc_peri                         = ADC1,
    .en_irq                           = false,
    .nvic_irq                         = ADC0_1_IRQn,
    .irq_prio                         = 1,
    .irq_sub_prio                     = 1,
};

Adc_HandleType  PackAdcHandle;

void ADCBspInit()
{
    uint8_t c_current_chl = ADC_CHANNEL_15;
    float RValtage = 0;
    PackAdcHandle.adc_cfg = &_AdcCfg;
    PackAdcHandle.current_chl = c_current_chl;
    PackAdcHandle.pRValtage = &RValtage;
    AdcInit(&PackAdcHandle);
}


/****************delay timer****************/
static Timer_Config _DelayTimer_cfg =
{
    .rcu_timer                              = RCU_TIMER6,
    .timer_peri                             = TIMER6,       /* 60M */

    .prescaler                              = 119,
    .alignedmode                            = TIMER_COUNTER_EDGE,
    .counterdirection                       = TIMER_COUNTER_UP,
    .clockdivision                          = TIMER_CKDIV_DIV1,
    .period                                 = 49999,          
    .repetitioncounter                      = 0,

    .quadrature_decoder_mode_enable         = false,
    .en_irq                                 = false,
    .nvic_irq                               = TIMER6_IRQn,
    .irq_prio                               = 2,
    .irq_sub_prio                           = 2,
    .irq_type                               = TIMER_INT_UP,
};
Timer_HandleType gDelayTimerHandle;

void DelayTimerInit(void)
{
    gDelayTimerHandle.timerx = TIMER6_PERI;
    gDelayTimerHandle.timer_cfg = &_DelayTimer_cfg;
    TimerInit(&gDelayTimerHandle);
}


/******************Heat*********************/

static Gpio_Config _HeatEnCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_10,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

static Gpio_Config _HeatCheckCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_11,
    .gpio_mode              = GPIO_MODE_IN_FLOATING,
    .en_exti                = false,
};
Gpio_HandleType HeatEnHandle;
Gpio_HandleType HeatCheckHandle;

void HeatInit(void)
{    
    HeatEnHandle.gpio_cfg = &_HeatEnCfg;
    HeatCheckHandle.gpio_cfg = &_HeatCheckCfg;

    GpioInit(&HeatEnHandle);
    GpioInit(&HeatCheckHandle);
}

/******************OV Check*********************/

static Gpio_Config _OVCheckCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_9,
    .gpio_mode              = GPIO_MODE_IN_FLOATING,
    .en_exti                = false,
};
Gpio_HandleType OVCheckHandle;

void OVCheckInit(void)
{    
    OVCheckHandle.gpio_cfg = &_OVCheckCfg;
    GpioInit(&OVCheckHandle);
}





/******************Dsg_OC_Check*********************/
static Gpio_Config _Dsg_OC_CheckCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_8,
    .gpio_mode              = GPIO_MODE_IN_FLOATING,
    .en_exti                = false,
};


Gpio_HandleType Dsg_OC_Check_Handle;

void Dsg_OC_CheckGpioBspInit()
{
    
    Dsg_OC_Check_Handle.gpio_cfg    = &_Dsg_OC_CheckCfg;     
    GpioInit(&Dsg_OC_Check_Handle);
}

/******************Chg&Dsg_OC_Clear*********************/
static Gpio_Config _Dsg_ClearCfg =
{
    .gpiox                  = GPIOD,
    .gpio_pinx              = GPIO_PIN_2,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};
Gpio_HandleType Dsg_OC_Clear_Handle;

void Dsg_OC_ClearGpioBspInit()
{
    Dsg_OC_Clear_Handle.gpio_cfg    = &_Dsg_ClearCfg;   
    GpioInit(&Dsg_OC_Clear_Handle);
}

/******************Mos*********************/
static Gpio_Config _PMosEnCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_6,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

static Gpio_Config _DMosEnCfg =
{
    .gpiox                  = GPIOD,
    .gpio_pinx              = GPIO_PIN_3,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

static Gpio_Config _CMosEnCfg = 
{
    .gpiox                  = GPIOD,
    .gpio_pinx              = GPIO_PIN_4,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};
Gpio_HandleType PMosEnHandle;
Gpio_HandleType DMosEnHandle;
Gpio_HandleType CMosEnHandle;

void MosBspInit(void)
{    
    PMosEnHandle.gpio_cfg = &_PMosEnCfg;
    DMosEnHandle.gpio_cfg = &_DMosEnCfg;
    CMosEnHandle.gpio_cfg = &_CMosEnCfg;

    GpioInit(&PMosEnHandle);
    GpioInit(&DMosEnHandle);
    GpioInit(&CMosEnHandle);
    
    GpioBitReset((&PMosEnHandle));
    GpioBitReset((&DMosEnHandle));
    GpioBitReset((&CMosEnHandle));
}

/******************Mcu_Led*********************/
static Gpio_Config _McuLedCfg =
{
    .gpiox                  = GPIOD,
    .gpio_pinx              = GPIO_PIN_8,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

Gpio_HandleType McuLedHandle;

void McuLedBspInit(void)
{    
    McuLedHandle.gpio_cfg = &_McuLedCfg;
    GpioInit(&McuLedHandle);

}


/*********************OVC**********************/
static Gpio_Config _OVCEnCfg =
{
    .gpiox                  = GPIOD,
    .gpio_pinx              = GPIO_PIN_9,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};


Gpio_HandleType OvcEnHandle;


void OVCBspInit(void)
{    
    OvcEnHandle.gpio_cfg = &_OVCEnCfg;
    GpioInit(&OvcEnHandle);
    GpioBitReset((&OvcEnHandle));

}

/******************Run_Led_En*********************/
static Gpio_Config _RunLedEnCfg =
{
    .gpiox                  = GPIOD,
    .gpio_pinx              = GPIO_PIN_1,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

Gpio_HandleType RunLedEnHandle;

void RunLedEnInit(void)
{    
    RunLedEnHandle.gpio_cfg = &_RunLedEnCfg;
    GpioInit(&RunLedEnHandle);
}

/******************充电器检测*********************/
static Gpio_Config _ChargerCheckCfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_12,
    .gpio_mode              = GPIO_MODE_IN_FLOATING,
    .en_exti                = false,
};

Gpio_HandleType ChargerCheckHandle;

void ChargerCheckBspInit(void)
{    
    ChargerCheckHandle.gpio_cfg = &_ChargerCheckCfg;
    GpioInit(&ChargerCheckHandle);
}

/******************CC_En（限流模块开启）*********************/
static Gpio_Config _CCEnEnCfg =
{
    .gpiox                  = GPIOD,
    .gpio_pinx              = GPIO_PIN_5,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};


/******************CC_PowerEn（限流模块电源）*********************/
static Gpio_Config _CCPowerEnCfg =
{
    .gpiox                  = GPIOD,
    .gpio_pinx              = GPIO_PIN_6,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

Gpio_HandleType CCEnHandle;
Gpio_HandleType CCPowerEnHandle;

void CCEnInit(void)
{    
    CCEnHandle.gpio_cfg = &_CCEnEnCfg;
    GpioInit(&CCEnHandle);
    //关闭限流
    GpioBitReset((&CCEnHandle));
    
    CCPowerEnHandle.gpio_cfg = &_CCPowerEnCfg;
    GpioInit(&CCPowerEnHandle);

    //关闭限流电源使能
    GpioBitReset((&CCPowerEnHandle));
}

/*****************泄放电路*********************/
static Gpio_Config _ReleaseCfg =
{
    .gpiox                  = GPIOD,
    .gpio_pinx              = GPIO_PIN_14,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

Gpio_HandleType ReleaseHandle;
void ReleaseInit(void)
{
    ReleaseHandle.gpio_cfg = &_ReleaseCfg;
    GpioInit(&ReleaseHandle);    
    
    GpioBitReset((&ReleaseHandle));
}

/******************VbatAdcCtrl（Vbat采样使能）*********************/
static Gpio_Config _VbatAdcCtrlCfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_11,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

Gpio_HandleType VbatAdcCtrlHandle;

void VbatAdcCtrlInit(void)
{    
    VbatAdcCtrlHandle.gpio_cfg = &_VbatAdcCtrlCfg;
    GpioInit(&VbatAdcCtrlHandle);
}

/******************McuPowerEn*********************/
static Gpio_Config _McuPowerEnCfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_5,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

Gpio_HandleType McuPowerEnHandle;

void McuPowerEnInit(void)
{    
    McuPowerEnHandle.gpio_cfg = &_McuPowerEnCfg;
    GpioInit(&McuPowerEnHandle);
}

/*****************PackKey*********************/
static Gpio_Config _AllPackPowerKeyCfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_6,
    .gpio_mode              = GPIO_MODE_IN_FLOATING,
    .en_exti                = false,
};

static Gpio_Config _SignalPackPowerKeyCfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_4,
    .gpio_mode              = GPIO_MODE_IN_FLOATING,
    .en_exti                = false,
};

Gpio_HandleType AllPackPowerKeyHandle;
Gpio_HandleType SignalPackPowerKeyHandle;

void PowerKeyInit(void)
{    
    AllPackPowerKeyHandle.gpio_cfg = &_AllPackPowerKeyCfg;
    GpioInit(&AllPackPowerKeyHandle);
    GpioBitSet((&AllPackPowerKeyHandle));
    
    SignalPackPowerKeyHandle.gpio_cfg = &_SignalPackPowerKeyCfg;
    GpioInit(&SignalPackPowerKeyHandle);
}

/******************CAN*********************/
static Can_Config _Can_Config[EN_DEV_CAN_NUM] = 
{

    [0].tx_gpiox               = GPIOA,
    [0].tx_gpio_pinx           = GPIO_PIN_12,
    [0].rcu_tx_gpio            = RCU_GPIOA,
    [0].rx_gpiox               = GPIOA,
    [0].rx_gpio_pinx           = GPIO_PIN_11,
    [0].rcu_rx_gpio            = RCU_GPIOA,
    [0].alt_rmp_mode           = 2,
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



void BspInit(void)
{
    /* enable clock */
    rcu_periph_clock_enable(RCU_PMU);
    DMAClkInit();
    GpioClkInit();
    ExtiClkInit();
    AdcClkInit();
    DelayTimerInit();
    Dsg_OC_CheckGpioBspInit();

    OVCheckInit();
    HeatInit();
    CCEnInit();
    ChargerCheckBspInit();

    McuPowerEnInit();
    ReleaseInit();
    OVCBspInit();
}