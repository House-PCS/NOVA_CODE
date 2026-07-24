/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-08-07
  * @brief       N32G4FR
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-08-07    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "bsp.h"
/*LED*/
static Gpio_Config  _LedSysGpioConfig =
{
    .gpiox                      = GPIOC,
    .gpio_pinx                  = GPIO_PIN_15,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    .en_exti                    = false,
};
Gpio_HandleType LedSysHandle;

static Gpio_Config  _ToMintorGpioConfig =
{
    .gpiox                      = GPIOC,
    .gpio_pinx                  = GPIO_PIN_14,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    .en_exti                    = false,
};
Gpio_HandleType ToMintorHandle;


static Gpio_Config  _LedPcsGpioConfig =
{
    .gpiox                      = GPIOA,
    .gpio_pinx                  = GPIO_PIN_8,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    .en_exti                    = false,
};
Gpio_HandleType LedPcsHandle;

static Gpio_Config  _LedConnGpioConfig =
{
    .gpiox                      = GPIOA,
    .gpio_pinx                  = GPIO_PIN_12,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    .en_exti                    = false,
};
Gpio_HandleType LedConnHandle;



void LedBspInit()
{
    LedSysHandle.gpio_cfg = &_LedSysGpioConfig;
    ToMintorHandle.gpio_cfg = &_ToMintorGpioConfig;
//    LedPcsHandle.gpio_cfg = &_LedPcsGpioConfig;
//    LedConnHandle.gpio_cfg = &_LedConnGpioConfig;

    GpioInit(&LedSysHandle);
    GpioInit(&ToMintorHandle);
//    GpioInit(&LedPcsHandle);
//    GpioInit(&LedConnHandle);
}

/* 时钟输出*/
static Gpio_Config  _RccMcoGpioConfig =
{
    .gpiox                      = GPIOA,
    .gpio_pinx                  = GPIO_PIN_8,
    .gpio_mode                  = GPIO_Mode_AF_PP,
    .en_exti                    = false,
};
Gpio_HandleType RccMcoHandle;
void RccMcoBspInit()
{
    RccMcoHandle.gpio_cfg = &_RccMcoGpioConfig;
    GpioInit(&RccMcoHandle);

}
/* 光伏检测ID引脚配置 */
static Gpio_Config  _DetectIdGpioConfig1 =
{
    .gpiox                      = GPIOC,
    .gpio_pinx                  = GPIO_PIN_4,
    .gpio_mode                  = GPIO_Mode_IPU,
    .en_exti                    = false,
};
Gpio_HandleType DetectIdHandle1;
static Gpio_Config  _DetectIdGpioConfig2 =
{
    .gpiox                      = GPIOC,
    .gpio_pinx                  = GPIO_PIN_5,
    .gpio_mode                  = GPIO_Mode_IPU,
    .en_exti                    = false,
};
Gpio_HandleType DetectIdHandle2;


void DetectIdBspInit()
{
    DetectIdHandle1.gpio_cfg = &_DetectIdGpioConfig1;
    DetectIdHandle2.gpio_cfg = &_DetectIdGpioConfig2;
    GpioInit(&DetectIdHandle1);
    GpioInit(&DetectIdHandle2);

}

/* 切换光伏检测通道继电器io配置 */
static Gpio_Config  _DetectChannelSwitchGpioConfig1 =
{
    .gpiox                      = GPIOB,
    .gpio_pinx                  = GPIO_PIN_4,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    .en_exti                    = false,
};

Gpio_HandleType DetectChannelSwitchHandle1;

static Gpio_Config  _DetectChannelSwitchGpioConfig2 =
{
    .gpiox                      = GPIOB,
    .gpio_pinx                  = GPIO_PIN_5,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    .en_exti                    = false,
};

Gpio_HandleType DetectChannelSwitchHandle2;
void DetectChannelSwitchBspInit()
{
    DetectChannelSwitchHandle1.gpio_cfg = &_DetectChannelSwitchGpioConfig1;
    DetectChannelSwitchHandle2.gpio_cfg = &_DetectChannelSwitchGpioConfig2;
    GpioInit(&DetectChannelSwitchHandle1);
    GpioInit(&DetectChannelSwitchHandle2);
}


/*******ADC1电流采集*******/
static Adc_Config _ArcCurAdc1Cfg =
{
    .channel_config[0].enable                   = true,
    .channel_config[0].chl_id                   = ADC1_Channel_03_PA6,
    .channel_config[0].adc_gpiox                = GPIOA,
    .channel_config[0].adc_gpio_pinx            = GPIO_PIN_6,

    .rcc_adc                                    = RCC_AHB_PERIPH_ADC1,
    .adc_peri                                   = ADC1,

    .continuous_mode                            = true,
    .scan_mode                                  = true,
    .sample_period                              = ADC_SAMP_TIME_28CYCLES5,
    
    .en_dma                                     = true,
    .dma_peri                                   = DMA1,
    .dma_channel                                = DMA_CH1,

    .en_dma_irq                                 = true,
    .nvic_dma_irq                               = DMA1_Channel1_IRQn,
    .irq_dma_prio                               = 1,
    .irq_dma_sub_prio                           = 1,
};
Adc_HandleType Adc1ArcCurHandle;
void Adc1ArcCurBspInit()
{
    Adc1ArcCurHandle.adcx = ADC1_PORT;
    Adc1ArcCurHandle.adc_cfg = &_ArcCurAdc1Cfg;
    AdcInit(&Adc1ArcCurHandle);
}

/*******ADC2电流采集*******/
static Adc_Config _ArcCurAdc2Cfg =
{
    .channel_config[0].enable                   = true,
    .channel_config[0].chl_id                   = ADC2_Channel_01_PA4,
    .channel_config[0].adc_gpiox                = GPIOA,
    .channel_config[0].adc_gpio_pinx            = GPIO_PIN_4,

    .channel_config[1].enable                   = true,
    .channel_config[1].chl_id                   = ADC2_Channel_02_PA5,
    .channel_config[1].adc_gpiox                = GPIOA,
    .channel_config[1].adc_gpio_pinx            = GPIO_PIN_5,

    .channel_config[2].enable                   = true,
    .channel_config[2].chl_id                   = ADC2_Channel_04_PA7,
    .channel_config[2].adc_gpiox                = GPIOA,
    .channel_config[2].adc_gpio_pinx            = GPIO_PIN_7,

    .rcc_adc                                    = RCC_AHB_PERIPH_ADC2,
    .adc_peri                                   = ADC2,

    .continuous_mode                            = true,
    .scan_mode                                  = true,
    .sample_period                              = ADC_SAMP_TIME_7CYCLES5,
    
    .en_dma                                     = true,
    .dma_peri                                   = DMA1,
    .dma_channel                                = DMA_CH8,

    .en_dma_irq                                 = true,
    .nvic_dma_irq                               = DMA1_Channel8_IRQn,
    .irq_dma_prio                               = 1,
    .irq_dma_sub_prio                           = 1,
};
Adc_HandleType Adc2ArcCurHandle;
void Adc2ArcCurBspInit()
{
    Adc2ArcCurHandle.adcx = ADC2_PORT;
    Adc2ArcCurHandle.adc_cfg = &_ArcCurAdc2Cfg;
    AdcInit(&Adc2ArcCurHandle);
}

/*******UART和主控交互******/           //  485
static Uart_Config _ModbusSlaveUartCfg =
{
    .tx_gpiox                   = GPIOA,
    .tx_gpio_pinx               = GPIO_PIN_9,

    .rx_gpiox                   = GPIOA,
    .rx_gpio_pinx               = GPIO_PIN_10,

    .rcc_uart                   = RCC_APB2_PERIPH_USART1,
    .uart_peri                  = USART1,
    .alt_rmp_mode               = 2,

    .baud_rate                  = 115200,
    .wlen                       = USART_WL_8B,
    .stblen                     = USART_STPB_1,
    .parity_cfg                 = USART_PE_NO,

    .en_irq                     = true,
    .nvic_irq                   = USART1_IRQn,
    .irq_prio                   = 1,
    .irq_sub_prio               = 1,
    .irq_type                   = EN_UART_INT_TYPE_IDLEF ,//| EN_UART_INT_TYPE_RXDNE,

    .en_dma_tx                  = true,
    .en_dma_rx                  = true,
    .en_dma_tx_nowait           = false,

    .dma_tx_peri                = DMA1,
    .dma_tx_channel             = DMA_CH4,
    .en_dma_tx_irq              = true,

    .dma_rx_peri                = DMA1,
    .dma_rx_channel             = DMA_CH5,
    .en_dma_rx_irq              = true,
};
Uart_HandleType gUart1ModbusHandle;
void DevModbusSlaveBspInit()
{
    gUart1ModbusHandle.uartx = UART1_PORT;
    gUart1ModbusHandle.uart_cfg = &_ModbusSlaveUartCfg;

    UartInit(&gUart1ModbusHandle);
}
static Gpio_Config _Rs485EnCfg =
{
    .gpiox                  = GPIOA,
    .gpio_pinx              = GPIO_PIN_0,
    .gpio_mode              = GPIO_Mode_Out_PP,
    .en_exti                = false,
};
Gpio_HandleType gGpioDirModbusHandle;

void Rs485EnBspInit()
{
    gGpioDirModbusHandle.gpio_cfg = &_Rs485EnCfg;

    GpioInit(&gGpioDirModbusHandle);
}


void BaseBspInit()
{
    DMAClkInit();
    GpioClkInit();
    Rs485EnBspInit();
    GpioBitReset(&gGpioDirModbusHandle);
//    GpioBitSet(&gGpioDirModbusHandle);
}

/******* HT7017 config 配置 ********/
static Uart_Config UartHt7017Cfg =
{
    .tx_gpiox                   = GPIOB,
    .tx_gpio_pinx               = GPIO_PIN_10,

    .rx_gpiox                   = GPIOB,
    .rx_gpio_pinx               = GPIO_PIN_11,

    .rcc_uart                   = RCC_APB1_PERIPH_USART3,
    .uart_peri                  = USART3,
    .alt_rmp_mode               = 2,

    .baud_rate                  = 4800,
    .wlen                       = USART_WL_9B, //USART_WL_9B
    .stblen                     = USART_STPB_1,
    .parity_cfg                 = USART_PE_EVEN,//USART_PE_NO

    .en_irq                     = true,
    .nvic_irq                   = USART3_IRQn,
    .irq_prio                   = 1,
    .irq_sub_prio               = 1,
    .irq_type                   = EN_UART_INT_TYPE_RXDNE | EN_UART_INT_TYPE_IDLEF,

    .en_dma_tx                  = false,
    .en_dma_rx                  = false,
};

Uart_HandleType Ht7017UartHandle;
void Ht7017Uart_Init(void)
{
    Ht7017UartHandle.uartx = UART3_PORT;
    Ht7017UartHandle.uart_cfg = &UartHt7017Cfg;

    UartInit(&Ht7017UartHandle);
}

//Uart_HandleType Ht7017UartHandle =
//{
//    .uart_cfg = &UartHt7017Cfg,
//    .pRxBuf   = gHt7017UartRxBuff,
//
//};
UART_RXTX_STATE     Ht7017RxTxState;
//void Uart6IrqProcess()                     //需要修改中断函数名字
//{
//if(0)
//{
//  Ht7017RxTxState.ReceiveFlg=1;
//}
//if(0)   //接收缓冲区非空
//{
//  Ht7017RxTxState.ReceiveNum++;
////        Ht7017UartHandle.pRxBuf++;
//}

//}
void Ht7017UartSend(uint8_t *buff, uint16_t len)
{
////    if(Ht7017RxTxState.Sending == UART_IDLE)
//    {
    Ht7017UartHandle.pTxBuf = buff;
    Ht7017UartHandle.tx_size = len;
    Ht7017RxTxState.Sending = UART_SEND_ING;
    UartNormalSend(&Ht7017UartHandle);
//    }
}
/*SPI FLASH*/
static Spi_Config _SpiFlashSpiCfg =
{
    .sck_gpiox                  = GPIOA,
    .sck_gpio_pinx              = GPIO_PIN_5,

    .mosi_gpiox                 = GPIOA,
    .mosi_gpio_pinx             = GPIO_PIN_7,

    .miso_gpiox                 = GPIOA,
    .miso_gpio_pinx             = GPIO_PIN_6,

    .alt_rmp_mode               = 2,
    .rcc_spi                    = RCC_APB2_PERIPH_SPI1,
    .spi_peri                   = SPI1,

    .spi_mode                   = SPI_MODE_MASTER,
    .data_direction             = SPI_DIR_DOUBLELINE_FULLDUPLEX,
    .data_len                   = SPI_DATA_SIZE_8BITS,
    .clock_polarity             = SPI_CLKPOL_HIGH,
    .clock_phase                = SPI_CLKPHA_SECOND_EDGE,
    .nss                        = SPI_NSS_SOFT,
    .prescale                   = SPI_BR_PRESCALER_4,
    .first_bit                  = SPI_FB_MSB,

    .en_irq                     = false,
    .en_dma_tx                  = false,
    .en_dma_rx                  = false,

//    .dma_tx_peri                = DMA1,
//    .dma_tx_channel             = DMA_CH3,

//    .dma_rx_peri                = DMA1,
//    .dma_rx_channel             = DMA_CH2,
};
Spi_HandleType SpiFlashHandle;

/*SPIFLASH CS*/
static Gpio_Config  _SpiFlashCsGpioConfig =
{
    .gpiox                      = GPIOA,
    .gpio_pinx                  = GPIO_PIN_4,
    .gpio_mode                  = GPIO_Mode_Out_PP,

    .en_exti                    = false,
};
Gpio_HandleType SpiFlashCsHandle;


void SpiFlashBspInit()
{
    SpiFlashHandle.spix = SPI1_PORT;
    SpiFlashHandle.spi_cfg = &_SpiFlashSpiCfg;
    SpiFlashCsHandle.gpio_cfg = &_SpiFlashCsGpioConfig;
    SpiInit(&SpiFlashHandle);
    GpioInit(&SpiFlashCsHandle);
    GpioBitSet(&SpiFlashCsHandle);
}


///////////////////HT7017_UARTCONfig_END///////////////
//static Uart_Config Rs485Cfg[RS485_NUM]=
//{
//    {
//        .tx_gpiox           =GPIOA,
//        .tx_gpio_pinx       =GPIO_PIN_9,
//        .tx_alt_num         =GPIO_AF_7,
//        .rx_gpiox           =GPIOD,
//        .rx_gpio_pinx       =GPIO_PIN_6,
//        .rx_alt_num         =GPIO_AF_7,

//        .rcu_uart           =RCU_USART1,
//        .uart_peri          =USART1,
//
//        .baud_rate          =9600,
//        .wlen               =USART_WL_8BIT,
//        .stblen             =USART_STB_1BIT,
//        .parity_cfg         =USART_PM_NONE,
//
//        .en_irq             =true,
//        .nvic_irq           =USART1_IRQn,
//        .irq_prio           =1,
//        .irq_sub_prio       =1,
//        .irq_type           = EN_UART_INT_TYPE_IDLE ,          //
//
//        .en_dma_tx          =true,
//        .en_dma_rx          = true,
//        .rcu_dma            =RCU_DMA0,
//
//        .en_dma_tx_nowait   =true,
//        .dma_tx_peri        =DMA0,
//        .dma_tx_channel     =DMA_CH6,
//        .dma_tx_subperi     =DMA_SUBPERI4,
//        .en_dma_tx_irq      =true,
//        .nvic_dma_tx_irq    =DMA0_Channel6_IRQn,
//        .irq_dma_tx_prio    =1,
//        .irq_dma_tx_sub_prio=1,
//
//
//        .dma_rx_peri        =DMA0,
//        .dma_rx_channel     =DMA_CH5,
//        .dma_rx_subperi     =DMA_SUBPERI4,
//        .en_dma_rx_irq      =false,
//    },
//    {
//        .tx_gpiox       =GPIOD,
//        .tx_gpio_pinx   =GPIO_PIN_8,
//        .tx_alt_num     =GPIO_AF_7,
//        .rx_gpiox       =GPIOD,
//        .rx_gpio_pinx   =GPIO_PIN_9,
//        .rx_alt_num     =GPIO_AF_7,

//        .rcu_uart       =RCU_USART2,
//        .uart_peri      =USART2,
//
//        .baud_rate      =9600,
//        .wlen           =USART_WL_8BIT,
//        .stblen         =USART_STB_1BIT,
//        .parity_cfg     =USART_PM_NONE,
//
//        .en_irq         =true,
//        .nvic_irq       =USART2_IRQn,
//        .irq_prio       =1,
//        .irq_sub_prio   =1,
//        .irq_type       = EN_UART_INT_TYPE_RT,          //
//
//        .en_dma_tx          = true,
//        .en_dma_rx          = true,
//        .rcu_dma            =RCU_DMA0,
//
//        .en_dma_tx_nowait   =true,
//        .dma_tx_peri        =DMA0,
//        .dma_tx_channel     =DMA_CH3,
//        .dma_tx_subperi     =DMA_SUBPERI4,
//        .en_dma_tx_irq      =true,
//        .nvic_dma_tx_irq    =DMA0_Channel3_IRQn,
//        .irq_dma_tx_prio    =1,
//        .irq_dma_tx_sub_prio=1,
//
//
//        .dma_rx_peri        =DMA0,
//        .dma_rx_channel     =DMA_CH1,
//        .dma_rx_subperi     =DMA_SUBPERI4,
//        .en_dma_rx_irq      =false,
//    }
//};
static Gpio_Config _DO_Cfg [DO_NUM] =
{
    {
        .gpiox                  = GPIOA,
        .gpio_pinx              = GPIO_PIN_11,
        .gpio_mode              = GPIO_Mode_Out_PP,
//        .gpio_pupd              = GPIO_PUPD_NONE,
//        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOA,
        .gpio_pinx              = GPIO_PIN_15,
        .gpio_mode              = GPIO_Mode_Out_PP,
//        .gpio_pupd              = GPIO_PUPD_NONE,
//        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOA,
        .gpio_pinx              = GPIO_PIN_12,
        .gpio_mode              = GPIO_Mode_Out_PP,
//        .gpio_pupd              = GPIO_PUPD_NONE,
//        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOB,
        .gpio_pinx              = GPIO_PIN_4,
        .gpio_mode              = GPIO_Mode_Out_PP,
//        .gpio_pupd              = GPIO_PUPD_NONE,
//        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOB,
        .gpio_pinx              = GPIO_PIN_5,
        .gpio_mode              = GPIO_Mode_Out_PP,
//        .gpio_pupd              = GPIO_PUPD_NONE,
//        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
//      {
//        .gpiox                  = GPIOA,
//        .gpio_pinx              = GPIO_PIN_0,
//        .gpio_mode              = GPIO_Mode_Out_PP,
////        .gpio_pupd              = GPIO_PUPD_NONE,
////        .gpio_otype             = GPIO_OTYPE_PP,
//        .en_exti                = false,
//    },
    {
        .gpiox                  = GPIOB,
        .gpio_pinx              = GPIO_PIN_3,
        .gpio_mode              = GPIO_Mode_Out_PP,
//      .gpio_pupd              = GPIO_PUPD_NONE,
//      .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
};

Gpio_HandleType DO_Handle[DO_NUM];

void DO_BspInit(void)
{
    for(uint8_t i = 0; i < DO_NUM; i++)
    {
        DO_Handle[i].gpio_cfg   = &_DO_Cfg[i];
        GpioInit(&DO_Handle[i]);
    }
    GpioBitReset(&LedSysHandle);//初始化报警信号
    GpioBitReset(&ToMintorHandle);
}

bool gDO_State[DO_NUM] = {false};
bool gDO_ForceFlg = false;

void DO_Main()
{
//    static bool do_state[DO_NUM];
    for(uint8_t i = 0; i < DO_NUM; i++)
    {
        if(GpioOutputBitGet((&DO_Handle[i])) != gDO_State[i])
        {
            GpioBitWrite((&DO_Handle[i]), gDO_State[i]);

        }
    }
}
//////////////IO_OUT END/////////////////




