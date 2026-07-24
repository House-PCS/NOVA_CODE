/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-11-22
  * @brief       GD32F450
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-11-22    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "bsp.h"


/*******EXMC SDRAM AND LCD*******/
static ExmcConfig_t _ExmcConfig = 
{
    .gpio_c_pinx            = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3,
    .gpio_d_pinx            = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | 
                              GPIO_PIN_10 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15,
    .gpio_e_pinx            = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | 
                              GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15,
    .gpio_f_pinx            = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_11 | 
                              GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15,
    .gpio_g_pinx            = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15,
    
    .sdram_en               = true,
    .sdram_device_id        = EXMC_SDRAM_DEVICE0,
    
    .sram_en                = true,
    .sram_region_id         = EXMC_BANK0_NORSRAM_REGION0,
};
Exmc_HandleType ExmcHandle;


void SdramBspInit()
{
    ExmcHandle.exmc_config = &_ExmcConfig;
    ExmcInit(&ExmcHandle, EN_EXMC_MODE_MASK_SDRAM);
}

void SramBspInit()
{
    ExmcHandle.exmc_config = &_ExmcConfig;
    
    if(ExmcHandle.exmc_config->sram_region_id == EXMC_BANK0_NORSRAM_REGION0)
    {
        ExmcHandle.sram_start_addr = 0x60000000;
    }
    else if(ExmcHandle.exmc_config->sram_region_id == EXMC_BANK0_NORSRAM_REGION1)
    {
        ExmcHandle.sram_start_addr = 0x64000000;
    }
    else if(ExmcHandle.exmc_config->sram_region_id == EXMC_BANK0_NORSRAM_REGION2)
    {
        ExmcHandle.sram_start_addr = 0x68000000;
    }
    else if(ExmcHandle.exmc_config->sram_region_id == EXMC_BANK0_NORSRAM_REGION3)
    {
        ExmcHandle.sram_start_addr = 0x6C000000;
    }
    ExmcInit(&ExmcHandle, EN_EXMC_MODE_MASK_SRAM);
}

void ExmcBspInit()
{
    SdramBspInit();
    SramBspInit();
}

void SdramPreInit()//用于main函数之前的初始化
{
    ExmcConfig_t exmc_cfg;
    Exmc_HandleType handle;
   
    exmc_cfg.gpio_c_pinx = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3;
    exmc_cfg.gpio_d_pinx = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | 
                           GPIO_PIN_10 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    exmc_cfg.gpio_e_pinx = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | 
                           GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    exmc_cfg.gpio_f_pinx = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_11 | 
                           GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    exmc_cfg.gpio_g_pinx = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
    exmc_cfg.sdram_en = true;
    exmc_cfg.sdram_device_id = EXMC_SDRAM_DEVICE0;
    
    handle.exmc_config = &exmc_cfg;
    
    ExmcSdramInit(&handle);
}




/*******LCD Rst AND BL*******/
static Gpio_Config _LcdRstnCfg =
{
    .gpiox                  = GPIOG,
    .gpio_pinx              = GPIO_PIN_7,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLUP,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};

static Gpio_Config _LcdBLCfg =
{
    .gpiox                  = GPIOF,
    .gpio_pinx              = GPIO_PIN_8,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLUP,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};

Gpio_HandleType LcdRstnHandle;
Gpio_HandleType LcdBLHandle;

void LcdCtlBspInit()
{
    LcdRstnHandle.gpio_cfg = &_LcdRstnCfg;
    LcdBLHandle.gpio_cfg = &_LcdBLCfg;
    
    GpioInit(&LcdRstnHandle);
    GpioInit(&LcdBLHandle);
}



/*******SPI FLASH*******/
static Spi_Config _SpiFlashSpiCfg = 
{
    .sck_gpiox              = GPIOA,
    .sck_gpio_pinx          = GPIO_PIN_5,
    .sck_alt_num            = GPIO_AF_5,

    .mosi_gpiox             = GPIOB,
    .mosi_gpio_pinx         = GPIO_PIN_5,
    .mosi_alt_num           = GPIO_AF_5,

    .miso_gpiox             = GPIOA,
    .miso_gpio_pinx         = GPIO_PIN_6,
    .miso_alt_num           = GPIO_AF_5,

    .nss_gpiox              = GPIOA,
    .nss_gpio_pinx          = GPIO_PIN_4,
    .nss_alt_num            = GPIO_AF_5,
    
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

    .dma_tx_peri            = DMA1,
    .dma_tx_channel         = DMA_CH3,
    .dma_tx_subperi         = DMA_SUBPERI3,

    .dma_rx_peri            = DMA1,
    .dma_rx_channel         = DMA_CH0,
    .dma_rx_subperi         = DMA_SUBPERI3,
};
Spi_HandleType SpiFlashHandle;

static Gpio_Config _SpiFlashCsCfg =
{
    .gpiox                  = GPIOA,
    .gpio_pinx              = GPIO_PIN_4,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLUP,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};

static Gpio_Config _SpiFlashWpCfg =
{
    .gpiox                  = GPIOA,
    .gpio_pinx              = GPIO_PIN_0,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLUP,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};

Gpio_HandleType SpiFlashCsHandle;
Gpio_HandleType SpiFlashWpHandle;


void SpiFlashBspInit()
{
    SpiFlashHandle.spix = SPI0_PORT;
    SpiFlashHandle.spi_cfg = &_SpiFlashSpiCfg;
    SpiFlashCsHandle.gpio_cfg = &_SpiFlashCsCfg;
    SpiFlashWpHandle.gpio_cfg = &_SpiFlashWpCfg;
    
    SpiInit(&SpiFlashHandle);
    GpioInit(&SpiFlashCsHandle);
    GpioInit(&SpiFlashWpHandle);
}


/*******Comm with BMS*******/
static Uart_Config _BMSCommUartCfg = 
{   
    .tx_gpiox               = GPIOC,
    .tx_gpio_pinx           = GPIO_PIN_12,
    .tx_alt_num             = GPIO_AF_8,
    .rx_gpiox               = GPIOD,
    .rx_gpio_pinx           = GPIO_PIN_2,
    .rx_alt_num             = GPIO_AF_8,
    
    .rcu_uart               = RCU_UART4,
    .uart_peri              = UART4,
    .baud_rate              = 115200,
    .wlen                   = USART_WL_8BIT,
    .stblen                 = USART_STB_1BIT,
    .parity_cfg             = USART_PM_NONE,
    
    .en_irq                 = true,
    .nvic_irq               = UART4_IRQn,
    .irq_prio               = 1,
    .irq_sub_prio           = 1,
    .irq_type               = EN_UART_INT_TYPE_IDLE,
    
    .en_dma_tx              = true,
    .en_dma_rx              = true,
    .en_dma_tx_irq          = true,    
    .dma_tx_peri            = DMA0,
    .dma_tx_channel         = DMA_CH7,
    .dma_tx_subperi         = DMA_SUBPERI4,
    

    .en_dma_tx_nowait       =true,
    .nvic_dma_tx_irq        =DMA0_Channel7_IRQn,
    .irq_dma_tx_prio        =1,
    .irq_dma_tx_sub_prio    =1,


    .dma_rx_peri            = DMA0,
    .dma_rx_channel         = DMA_CH0,
    .dma_rx_subperi         = DMA_SUBPERI4,
};

/*****需要改动*****/
static Gpio_Config _BMSCommDECfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_3,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};

Uart_HandleType ModbusHostBMSUartHandle;
Gpio_HandleType ModbusHostBMSDEHandle;

void ModbusHostBMSBspInit()
{
    ModbusHostBMSUartHandle.uartx = UART4_PORT;
    ModbusHostBMSUartHandle.uart_cfg = &_BMSCommUartCfg;
    
    UartInit(&ModbusHostBMSUartHandle);
    
    ModbusHostBMSDEHandle.gpio_cfg = &_BMSCommDECfg;
    GpioInit(&ModbusHostBMSDEHandle);
}

/*******Comm with PCS*******/
static Uart_Config _PCSCommUartCfg = 
{   
    .tx_gpiox               = GPIOC,
    .tx_gpio_pinx           = GPIO_PIN_10,
    .tx_alt_num             = GPIO_AF_7,
    .rx_gpiox               = GPIOC,
    .rx_gpio_pinx           = GPIO_PIN_11,
    .rx_alt_num             = GPIO_AF_7,
    
    .rcu_uart               = RCU_USART2,
    .uart_peri              = USART2,
    .baud_rate              = 115200,
    .wlen                   = USART_WL_8BIT,
    .stblen                 = USART_STB_1BIT,
    .parity_cfg             = USART_PM_NONE,
    
    .en_irq                 = true,
    .nvic_irq               = USART2_IRQn,
    .irq_prio               = 1,
    .irq_sub_prio           = 1,
    .irq_type               = EN_UART_INT_TYPE_IDLE,
    




    .en_dma_tx              = true,
    .en_dma_rx              = true,
    .dma_tx_peri            = DMA0,
    .dma_tx_channel         = DMA_CH3,
    .dma_tx_subperi         = DMA_SUBPERI4,

    .en_dma_tx_irq          = true,  
    .en_dma_tx_nowait       = true,
    .nvic_dma_tx_irq        = DMA0_Channel3_IRQn,
    .irq_dma_tx_prio        = 1,
    .irq_dma_tx_sub_prio    = 1,

    .dma_rx_peri            = DMA0,
    .dma_rx_channel         = DMA_CH1,
    .dma_rx_subperi         = DMA_SUBPERI4,
};

static Gpio_Config _PCSCommDECfg =
{
    .gpiox                  = GPIOG,
    .gpio_pinx              = GPIO_PIN_11,//9
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};

Uart_HandleType ModbusHostPCSUartHandle;
Gpio_HandleType ModbusHostPCSDEHandle;

void ModbusHostPCSBspInit()
{
    ModbusHostPCSUartHandle.uartx = UART2_PORT;
    ModbusHostPCSUartHandle.uart_cfg = &_PCSCommUartCfg;
    
    UartInit(&ModbusHostPCSUartHandle);
    
    ModbusHostPCSDEHandle.gpio_cfg = &_PCSCommDECfg;
    GpioInit(&ModbusHostPCSDEHandle);
}

/*******Comm with Collector*******/
static Uart_Config _CollectorCommUartCfg = 
{   
    .tx_gpiox               = GPIOA,
    .tx_gpio_pinx           = GPIO_PIN_11,
    .tx_alt_num             = GPIO_AF_8,
    .rx_gpiox               = GPIOA,
    .rx_gpio_pinx           = GPIO_PIN_12,
    .rx_alt_num             = GPIO_AF_8,
    
    .rcu_uart               = RCU_USART5,
    .uart_peri              = USART5,
    .baud_rate              = 9600,
    .wlen                   = USART_WL_8BIT,
    .stblen                 = USART_STB_1BIT,
    .parity_cfg             = USART_PM_NONE,
    
    .en_irq                 = true,
    .nvic_irq               = USART5_IRQn,
    .irq_prio               = 1,
    .irq_sub_prio           = 1,
    .irq_type               = EN_UART_INT_TYPE_IDLE,
    

    .en_dma_tx              = true,
    .en_dma_rx              = true,
    .dma_tx_peri            = DMA1,
    .dma_tx_channel         = DMA_CH7,
    .dma_tx_subperi         = DMA_SUBPERI5,

    .en_dma_tx_irq          = true,  
    .en_dma_tx_nowait       = true,
    .nvic_dma_tx_irq        = DMA1_Channel7_IRQn,
    .irq_dma_tx_prio        = 1,
    .irq_dma_tx_sub_prio    = 1,


    .dma_rx_peri            = DMA1,
    .dma_rx_channel         = DMA_CH1,
    .dma_rx_subperi         = DMA_SUBPERI5,
};

static Gpio_Config _CollectorCommDECfg =
{
    .gpiox                  = GPIOG,
    .gpio_pinx              = GPIO_PIN_11,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};

Uart_HandleType ModbusSlaveCollectorUartHandle;
Gpio_HandleType ModbusSlaveCollectorDEHandle;

void ModbusHostCollectorBspInit()
{
    ModbusSlaveCollectorUartHandle.uartx = UART5_PORT;
    ModbusSlaveCollectorUartHandle.uart_cfg = &_CollectorCommUartCfg;
    
    UartInit(&ModbusSlaveCollectorUartHandle);
    
    ModbusSlaveCollectorDEHandle.gpio_cfg = &_CollectorCommDECfg;
    GpioInit(&ModbusSlaveCollectorDEHandle);
}


/*******Key*******/
static Gpio_Config _KeyLeftLed =
{
    .gpiox                  = GPIOD,
    .gpio_pinx              = GPIO_PIN_11,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
static Gpio_Config _KeyLeftCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_9,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
static Gpio_Config _KeyRightCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_8,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
static Gpio_Config _KeyConfirmCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_6,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
static Gpio_Config _KeyEscCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_7,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
Gpio_HandleType KeyLeftLedHandle;
Gpio_HandleType KeyLeftHandle;
Gpio_HandleType KeyRightHandle;
Gpio_HandleType KeyConfirmHandle;
Gpio_HandleType KeyEscHandle;

void KeyBspInit()
{
	KeyLeftLedHandle.gpio_cfg = &_KeyLeftLed;
    KeyLeftHandle.gpio_cfg = &_KeyLeftCfg;
    KeyRightHandle.gpio_cfg = &_KeyRightCfg;
    KeyConfirmHandle.gpio_cfg = &_KeyConfirmCfg;
    KeyEscHandle.gpio_cfg = &_KeyEscCfg;
    
    GpioInit(&KeyLeftLedHandle);
    GpioInit(&KeyLeftHandle);
    GpioInit(&KeyRightHandle);
    GpioInit(&KeyConfirmHandle);
    GpioInit(&KeyEscHandle);
}



/*******LED*******/


/******氛围灯&按键背光灯******/
static Gpio_Config _AtmosCh1Cfg =
{
    .gpiox                  = GPIOD,
    .gpio_pinx              = GPIO_PIN_12,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
static Gpio_Config _AtmosCh2Cfg =
{
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_14,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
static Gpio_Config _AtmosCh3Cfg =
{
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_15,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
static Gpio_Config _AtmosCh4Cfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_2,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};

Gpio_HandleType AtmosChxHandle[4];
void AtmosBspInit(void)
{
	AtmosChxHandle[0].gpio_cfg = &_AtmosCh1Cfg;
    AtmosChxHandle[1].gpio_cfg = &_AtmosCh2Cfg;
    AtmosChxHandle[2].gpio_cfg = &_AtmosCh3Cfg;
	AtmosChxHandle[3].gpio_cfg = &_AtmosCh4Cfg;

    GpioInit(&AtmosChxHandle[0]);
    GpioInit(&AtmosChxHandle[1]);
    GpioInit(&AtmosChxHandle[2]);
	GpioInit(&AtmosChxHandle[3]);
	
	GpioBitReset(&AtmosChxHandle[0]);
	GpioBitReset(&AtmosChxHandle[1]);
	GpioBitReset(&AtmosChxHandle[2]);
	GpioBitReset(&AtmosChxHandle[3]);
}


/********数码管驱动IC   MBI5024*******/
static Gpio_Config _MB15024SPWM_DIN =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_6,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,//GPIO_OTYPE_OD,
    .en_exti                = false,
    
};

static Gpio_Config _MBI502xLedClkCfg =
{
    .gpiox                  = GPIOF,
    .gpio_pinx              = GPIO_PIN_6,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
static Gpio_Config _MBI502xLedDataCfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_6,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
static Gpio_Config _MBI502xLedLatchCfg =
{
    .gpiox                  = GPIOG,
    .gpio_pinx              = GPIO_PIN_9,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
static Gpio_Config _MBI502xLedOECfg =
{
    .gpiox                  = GPIOG,
    .gpio_pinx              = GPIO_PIN_10,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLUP,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
Gpio_HandleType MBI502xLedClkHandle;
Gpio_HandleType MBI502xLedDataHandle;
Gpio_HandleType MBI502xLedLatchHandle;
Gpio_HandleType MBI502xLedOEHandle;


void MBI502xLedBspInit(void)
{
	MBI502xLedClkHandle.gpio_cfg = &_MBI502xLedClkCfg;
    MBI502xLedDataHandle.gpio_cfg = &_MBI502xLedDataCfg;
    MBI502xLedLatchHandle.gpio_cfg = &_MBI502xLedLatchCfg;
    MBI502xLedOEHandle.gpio_cfg = &_MBI502xLedOECfg;

    GpioInit(&MBI502xLedClkHandle);
    GpioInit(&MBI502xLedDataHandle);
    GpioInit(&MBI502xLedLatchHandle);
    GpioInit(&MBI502xLedOEHandle);
	
	GpioBitReset(&MBI502xLedClkHandle);
	GpioBitReset(&MBI502xLedDataHandle);
	GpioBitReset(&MBI502xLedLatchHandle);
    GpioBitSet(&MBI502xLedOEHandle);
}



/***用于轮询的定时器***/
static Timer_Config _TimerPollingCfg = 
{
    .rcu_timer              = RCU_TIMER5,
    .timer_peri             = TIMER5,
    .prescaler              = 100,//根据系统时钟频率变化,RCU_CFG1的TIMERSEL为0，APB1 = AHB/4，CK_TIMER = 2xAPB1,所以是系统时钟的一半
    .alignedmode            = TIMER_COUNTER_EDGE,
    .counterdirection       = TIMER_COUNTER_UP,
    .clockdivision          = TIMER_CKDIV_DIV1,
    .period                 = 5000,
    .repetitioncounter      = 0,
    
    .en_irq                 = true,
    .nvic_irq               = TIMER5_DAC_IRQn,
    .irq_prio               = 0,
    .irq_sub_prio           = 2,
    .irq_type               = TIMER_INT_UP,
};
Timer_HandleType TimerPollingHandle;

void TimerPollingBspInit()
{
    TimerPollingHandle.timerx = TIMER5_PERI;
    TimerPollingHandle.timer_cfg = &_TimerPollingCfg;
    
    TimerInit(&TimerPollingHandle);
}
void TimerPollingBspDeInit(void)
{
    TimerDeInit(&TimerPollingHandle);    
}



/*******RTC*******/
#if 0
static I2C_Config _RtcCfg =
{
    .scl_gpiox              = GPIOB,
    .scl_gpio_pinx          = GPIO_PIN_8,
    .scl_alt_num            = GPIO_AF_4,
    
    .sda_gpiox              = GPIOB,
    .sda_gpio_pinx          = GPIO_PIN_9,
	.sda_alt_num            = GPIO_AF_4,
    
    .rcu_i2c                = RCU_I2C0,
    .i2c_peri               = I2C0,
    .speed                  = 100000,
};
I2C_HandleType RtcHandle;
#endif

static SimI2C_Config _RtcCfg =
{
    .scl_gpiox              = GPIOB,
    .scl_gpio_pinx          = GPIO_PIN_3,
    
    .sda_gpiox              = GPIOB,
    .sda_gpio_pinx          = GPIO_PIN_4,

};
SimI2C_HandleType RtcHandle;

static Gpio_Config _RtcIntCfg =
{
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_6,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = true,
        
    .gpio_exti_port         = EXTI_SOURCE_GPIOB,
    .gpio_exti_pin          = EXTI_SOURCE_PIN6,
    .gpio_exti_mode         = EXTI_INTERRUPT,
    .gpio_exti_triger       = EXTI_TRIG_RISING,
    
    .nvic_irq               = EXTI5_9_IRQn,
    .irq_prio               = 1,
    .irq_sub_prio           = 1,
};  
Gpio_HandleType RtcIntHandle;

void RtcBspInit(void)
{
#if 0
    RtcHandle.i2cx = I2C0_PORT;
    RtcHandle.i2c_cfg = &_RtcCfg;
    I2CInit(&RtcHandle);
#endif
	RtcHandle.simi2c_cfg = &_RtcCfg;
	SimI2CInit(&RtcHandle);
    RtcIntHandle.gpio_cfg = &_RtcIntCfg;
    GpioInit(&RtcIntHandle);
}


/********TCA 9555 START*********/
static SimI2C_Config _TCA9555Cfg =
{
    .scl_gpiox              = GPIOF,
    .scl_gpio_pinx          = GPIO_PIN_10,
    
    .sda_gpiox              = GPIOF,
    .sda_gpio_pinx          = GPIO_PIN_9,

};
SimI2C_HandleType TCA9555I2CHandle;

static Gpio_Config _TCA9555IntCfg =
{
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_10,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = true,
        
    .gpio_exti_port         = EXTI_SOURCE_GPIOB,
    .gpio_exti_pin          = EXTI_SOURCE_PIN10,
    .gpio_exti_mode         = EXTI_INTERRUPT,
    .gpio_exti_triger       = EXTI_TRIG_RISING,
    
    .nvic_irq               = EXTI10_15_IRQn,
    .irq_prio               = 1,
    .irq_sub_prio           = 1,
};  
Gpio_HandleType TCA9555IntHandle;


static Gpio_Config _SwitchLedCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_15,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
Gpio_HandleType SwitchLedHandle;



void TCA9555BspInit(void)
{
	TCA9555I2CHandle.simi2c_cfg = &_TCA9555Cfg;
	SimI2CInit(&TCA9555I2CHandle);
	
    TCA9555IntHandle.gpio_cfg = &_TCA9555IntCfg;
	
    GpioInit(&TCA9555IntHandle);


}


/********TCA9555 END*********/
void BaseBspInit()
{
    ExtiClkInit();
    DMAClkInit();
    GpioClkInit();

    SwitchLedHandle.gpio_cfg = &_SwitchLedCfg;
    GpioBitReset(&SwitchLedHandle);
    GpioInit(&SwitchLedHandle);
#if(SWITCH_LED_HIGH_ENABLE)


    GpioBitReset(&SwitchLedHandle);

#else
    GpioBitSet(&SwitchLedHandle);   

#endif

}





