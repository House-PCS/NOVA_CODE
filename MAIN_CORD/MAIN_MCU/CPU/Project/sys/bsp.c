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

int32_t  gDeviceVersionType = 0;    //  0--LCD    1--LED


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
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_8,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};

Uart_HandleType ModbusSlaveCollectorUartHandle;
Gpio_HandleType ModbusSlaveCollectorDEHandle;

void ModbusHostCollectorBspInit(void)
{
    ModbusSlaveCollectorUartHandle.uartx = UART5_PORT;
    ModbusSlaveCollectorUartHandle.uart_cfg = &_CollectorCommUartCfg;

    UartInit(&ModbusSlaveCollectorUartHandle);

    ModbusSlaveCollectorDEHandle.gpio_cfg = &_CollectorCommDECfg;
    GpioInit(&ModbusSlaveCollectorDEHandle);
}

/*******Comm with Voltmeter/ARC*******/
static Uart_Config _VoltmeterArcCommUartCfg =
{
    .tx_gpiox               = GPIOF,
    .tx_gpio_pinx           = GPIO_PIN_7,
    .tx_alt_num             = GPIO_AF_8,
    .rx_gpiox               = GPIOF,
    .rx_gpio_pinx           = GPIO_PIN_6,
    .rx_alt_num             = GPIO_AF_8,

    .rcu_uart               = RCU_UART6,
    .uart_peri              = UART6,
    .baud_rate              = 9600,
    .wlen                   = USART_WL_8BIT,
    .stblen                 = USART_STB_1BIT,
    .parity_cfg             = USART_PM_NONE,

    .en_irq                 = true,
    .nvic_irq               = UART6_IRQn,
    .irq_prio               = 1,
    .irq_sub_prio           = 1,
    .irq_type               = EN_UART_INT_TYPE_IDLE, // EN_UART_INT_TYPE_RBNE,


    .en_dma_tx              = true, //false,
    .en_dma_rx              = true, //false,
    .dma_tx_peri            = DMA0,
    .dma_tx_channel         = DMA_CH1,
    .dma_tx_subperi         = DMA_SUBPERI5,

    .en_dma_tx_irq          = true, //false,
    .en_dma_tx_nowait       = true,
    .nvic_dma_tx_irq        = DMA0_Channel1_IRQn,
    .irq_dma_tx_prio        = 1,
    .irq_dma_tx_sub_prio    = 1,


    .dma_rx_peri            = DMA0,
    .dma_rx_channel         = DMA_CH3,
    .dma_rx_subperi         = DMA_SUBPERI5,
};

static Gpio_Config _VoltMeterArcCommDECfg =
{
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_7,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};

Gpio_HandleType ModbusSlaveVoltmeterArcDEHandle;
Uart_HandleType ModbusSlaveVoltmeterArcUartHandle;

void ModbusHostVoltmeterArcBspInit(void)
{
    ModbusSlaveVoltmeterArcUartHandle.uartx = UART6_PORT;
    ModbusSlaveVoltmeterArcUartHandle.uart_cfg = &_VoltmeterArcCommUartCfg;
    UartInit(&ModbusSlaveVoltmeterArcUartHandle);
    
    ModbusSlaveVoltmeterArcDEHandle.gpio_cfg = &_VoltMeterArcCommDECfg;
    GpioInit(&ModbusSlaveVoltmeterArcDEHandle);
}


/*******Comm with PCS (预留的 暂未使用 工装测试使用)*******/
static Uart_Config _PCSCommUartCfg =
{
        
    .tx_gpiox               = GPIOC,
    .tx_gpio_pinx           = GPIO_PIN_10,
    .tx_alt_num             = GPIO_AF_8,
    .rx_gpiox               = GPIOC,
    .rx_gpio_pinx           = GPIO_PIN_11,
    .rx_alt_num             = GPIO_AF_8,

    .rcu_uart               = RCU_UART3,
    .uart_peri              = UART3,
    .baud_rate              = 9600,
    .wlen                   = USART_WL_8BIT,
    .stblen                 = USART_STB_1BIT,
    .parity_cfg             = USART_PM_NONE,

    .en_irq                 = true,
    .nvic_irq               = UART3_IRQn,
    .irq_prio               = 1,
    .irq_sub_prio           = 1,
    .irq_type               = EN_UART_INT_TYPE_IDLE,


    .en_dma_tx              = true, //true,
    .en_dma_rx              = true, //true,
    .dma_tx_peri            = DMA0,
    .dma_tx_channel         = DMA_CH4,
    .dma_tx_subperi         = DMA_SUBPERI4,

    .en_dma_tx_irq          = true, //true,
    .en_dma_tx_nowait       = true, //true,
    .nvic_dma_tx_irq        = DMA0_Channel4_IRQn,
    .irq_dma_tx_prio        = 1,
    .irq_dma_tx_sub_prio    = 1,

    .dma_rx_peri            = DMA0,
    .dma_rx_channel         = DMA_CH2,
    .dma_rx_subperi         = DMA_SUBPERI4,
};

static Gpio_Config _PCSCommDECfg =
{
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_9,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};

Uart_HandleType ModbusHostPCSUartHandle;
Gpio_HandleType ModbusHostPCSDEHandle;

void ModbusHostPCSBspInit()
{
    ModbusHostPCSUartHandle.uartx = UART3_PORT;
    ModbusHostPCSUartHandle.uart_cfg = &_PCSCommUartCfg;

    UartInit(&ModbusHostPCSUartHandle);

    ModbusHostPCSDEHandle.gpio_cfg = &_PCSCommDECfg;
    GpioInit(&ModbusHostPCSDEHandle);
}


/*******Comm with 协处理器 串口通信*******/
static Uart_Config _CoprocessorCommUartCfg =
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


    .en_dma_tx_nowait       = true,
    .nvic_dma_tx_irq        = DMA0_Channel7_IRQn,
    .irq_dma_tx_prio        = 1,
    .irq_dma_tx_sub_prio    = 1,


    .dma_rx_peri            = DMA0,
    .dma_rx_channel         = DMA_CH0,
    .dma_rx_subperi         = DMA_SUBPERI4,
};



Uart_HandleType ModbusHostCoprocessorUartHandle;
Gpio_HandleType ModbusHostCoprocessorDeHandle;
void ModbusHostCoprocessorBspInit()
{
    ModbusHostCoprocessorUartHandle.uartx = UART4_PORT;
    ModbusHostCoprocessorUartHandle.uart_cfg = &_CoprocessorCommUartCfg;

    UartInit(&ModbusHostCoprocessorUartHandle);
}

/*******uart  debug*******/
static Uart_Config _DebugUart0Cfg =
{
    .tx_gpiox               = GPIOA,
    .tx_gpio_pinx           = GPIO_PIN_9,
    .tx_alt_num             = GPIO_AF_7,
    .rx_gpiox               = GPIOA,
    .rx_gpio_pinx           = GPIO_PIN_10,
    .rx_alt_num             = GPIO_AF_7,

    .rcu_uart               = RCU_USART0,
    .uart_peri              = USART0,
    .baud_rate              = 115200,
    .wlen                   = USART_WL_8BIT,
    .stblen                 = USART_STB_1BIT,
    .parity_cfg             = USART_PM_NONE,

    .en_irq                 = true,
    .nvic_irq               = USART0_IRQn,
    .irq_prio               = 1,
    .irq_sub_prio           = 1,
    .irq_type               = EN_UART_INT_TYPE_RBNE,

    .en_dma_tx              = false,
    .en_dma_rx              = false,
    .dma_tx_peri            = DMA1,
    .dma_tx_channel         = DMA_CH7,
    .dma_tx_subperi         = DMA_SUBPERI4,

    .en_dma_tx_irq          = false,
    .en_dma_tx_nowait       = false,
    .nvic_dma_tx_irq        = DMA1_Channel7_IRQn,
    .irq_dma_tx_prio        = 1,
    .irq_dma_tx_sub_prio    = 1,


    .dma_rx_peri            = DMA1,
    .dma_rx_channel         = DMA_CH5,
    .dma_rx_subperi         = DMA_SUBPERI4,
};

Uart_HandleType DebugUart0Handle;
void DebugUart0BspInit(void)
{
    DebugUart0Handle.uartx = UART0_PORT;
    DebugUart0Handle.uart_cfg = &_DebugUart0Cfg;

    UartInit(&DebugUart0Handle);
}
/********版本区分IO*********/
static Gpio_Config _McuVerID1Cfg =
{
    .gpiox                  = GPIOA,
    .gpio_pinx              = GPIO_PIN_8,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_NONE,
    .en_exti                = false,
};
static Gpio_Config _McuVerID2Cfg =
{
    .gpiox                  = GPIOA,
    .gpio_pinx              = GPIO_PIN_15,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_NONE,
    .en_exti                = false,
};

Gpio_HandleType McuVerID1Handle;
Gpio_HandleType McuVerID2Handle;

void McuVersionBspInit(void)
{
    McuVerID1Handle.gpio_cfg = &_McuVerID1Cfg;
    McuVerID2Handle.gpio_cfg = &_McuVerID2Cfg;

    GpioInit(&McuVerID1Handle);
    GpioInit(&McuVerID2Handle);
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

void KeyBspInit(void)
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

static Gpio_Config _SwitchLedCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_15,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLUP,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};


Gpio_HandleType SwitchLedHandle;
Gpio_HandleType AtmosChxHandle[6];
void BspSwitchLedInit(void)
{
    SwitchLedHandle.gpio_cfg = &_SwitchLedCfg;
    GpioInit(&SwitchLedHandle);
#if(SWITCH_LED_HIGH_ENABLE)
    GpioBitReset(&SwitchLedHandle);
#else
    GpioBitSet(&SwitchLedHandle);   
#endif       
}

/********数码管驱动IC   MBI5024*******/
static Gpio_Config _MBI502xLedClkCfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_2,
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

//DRM0功能所用的IO口（干接点 做输入检测用）
static Gpio_Config _DRM0ConnectIOCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_14,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_NONE,
    .en_exti                = false,
};
static Gpio_Config _DRM1ConnectIOCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_15,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_NONE,
    .en_exti                = false,
};
static Gpio_Config _DRM2ConnectIOCfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_5,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_NONE,
    .en_exti                = false,
};
static Gpio_Config _DRM3ConnectIOCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_13,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_NONE,
    .en_exti                = false,
};

static Gpio_Config _DRM4ConnectIOCfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_3,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_NONE,
    .en_exti                = false,
};


Gpio_HandleType DRM0ConnectIOHandle;
Gpio_HandleType DRM1ConnectIOHandle;
Gpio_HandleType DRM2ConnectIOHandle;
Gpio_HandleType DRM3ConnectIOHandle;
Gpio_HandleType DRM4ConnectIOHandle;

void DRMConnectIOInit(void)
{
    DRM0ConnectIOHandle.gpio_cfg = &_DRM0ConnectIOCfg;
    GpioInit(&DRM0ConnectIOHandle);
    DRM1ConnectIOHandle.gpio_cfg = &_DRM1ConnectIOCfg;
    GpioInit(&DRM1ConnectIOHandle);
    DRM2ConnectIOHandle.gpio_cfg = &_DRM2ConnectIOCfg;
    GpioInit(&DRM2ConnectIOHandle);
    DRM3ConnectIOHandle.gpio_cfg = &_DRM3ConnectIOCfg;
    GpioInit(&DRM3ConnectIOHandle);
    DRM4ConnectIOHandle.gpio_cfg = &_DRM4ConnectIOCfg;
    GpioInit(&DRM4ConnectIOHandle);
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
    .irq_sub_prio           = 1,
    .irq_type               = TIMER_INT_UP,
};
Timer_HandleType TimerPollingHandle;

void TimerPollingBspInit()
{
    TimerPollingHandle.timerx = TIMER5_PERI;
    TimerPollingHandle.timer_cfg = &_TimerPollingCfg;
    TimerInit(&TimerPollingHandle);
}




/***用于定时通讯***/
static Timer_Config _CommTimerPollingCfg =
{
    .rcu_timer              = RCU_TIMER6,
    .timer_peri             = TIMER6,
    .prescaler              = 100,//根据系统时钟频率变化,RCU_CFG1的TIMERSEL为0，APB1 = AHB/4，CK_TIMER = 2xAPB1,所以是系统时钟的一半
    .alignedmode            = TIMER_COUNTER_EDGE,
    .counterdirection       = TIMER_COUNTER_UP,
    .clockdivision          = TIMER_CKDIV_DIV1,
    .period                 = 50000,
    .repetitioncounter      = 0,

    .en_irq                 = true,
    .nvic_irq               = TIMER6_IRQn,
    .irq_prio               = 2,
    .irq_sub_prio           = 1,
    .irq_type               = TIMER_INT_UP,
};
Timer_HandleType CommTimerPollingHandle;

void CommTimerPollingBspInit(void)
{
    CommTimerPollingHandle.timerx = TIMER6_PERI;
    CommTimerPollingHandle.timer_cfg = &_CommTimerPollingCfg;
    TimerInit(&CommTimerPollingHandle);
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

/*****************MCU LED***********************/
static Gpio_Config _McuWorkLedCfg =
{
    .gpiox                  = GPIOF,
    .gpio_pinx              = GPIO_PIN_9,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_PULLUP,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};

Gpio_HandleType McuWorkLedHandle;


void McuWorkLedBspInit(void)
{
    McuWorkLedHandle.gpio_cfg = &_McuWorkLedCfg;
    GpioInit(&McuWorkLedHandle);
}


//    GpioBitSet(&MBI502xLedOEHandle);
/*************工装输入IO测试配置****************/
static Gpio_Config _ConnectGPIOCfg[CONNECT_GPIO_NUM] =
{
    [0].gpiox                  = GPIOE,                 //GPIO1
    [0].gpio_pinx              = GPIO_PIN_3,
    [0].gpio_mode              = GPIO_MODE_INPUT,
    [0].gpio_pupd              = GPIO_PUPD_PULLDOWN,
    [0].en_exti                = false,

    [1].gpiox                  = GPIOE,                 //GPIO2
    [1].gpio_pinx              = GPIO_PIN_4,
    [1].gpio_mode              = GPIO_MODE_INPUT,
    [1].gpio_pupd              = GPIO_PUPD_PULLDOWN,
    [1].en_exti                = false,

    [2].gpiox                  = GPIOE,                 //GPIO3
    [2].gpio_pinx              = GPIO_PIN_5,
    [2].gpio_mode              = GPIO_MODE_INPUT,
    [2].gpio_pupd              = GPIO_PUPD_PULLDOWN,
    [2].en_exti                = false,

    [3].gpiox                  = GPIOC,                 //GPIO4
    [3].gpio_pinx              = GPIO_PIN_13,
    [3].gpio_mode              = GPIO_MODE_INPUT,
    [3].gpio_pupd              = GPIO_PUPD_PULLDOWN,
    [3].en_exti                = false,

    [4].gpiox                  = GPIOC,                 //GPIO5
    [4].gpio_pinx              = GPIO_PIN_14,
    [4].gpio_mode              = GPIO_MODE_INPUT,
    [4].gpio_pupd              = GPIO_PUPD_PULLDOWN,
    [4].en_exti                = false,

    [5].gpiox                  = GPIOC,                 //GPIO6
    [5].gpio_pinx              = GPIO_PIN_15,
    [5].gpio_mode              = GPIO_MODE_INPUT,
    [5].gpio_pupd              = GPIO_PUPD_PULLDOWN,
    [5].en_exti                = false,

    [6].gpiox                  = GPIOD,                 //Node0
    [6].gpio_pinx              = GPIO_PIN_3,
    [6].gpio_mode              = GPIO_MODE_INPUT,
    [6].gpio_pupd              = GPIO_PUPD_PULLDOWN,
    [6].en_exti                = false,

    [7].gpiox                  = GPIOD,                 //Node1
    [7].gpio_pinx              = GPIO_PIN_6,
    [7].gpio_mode              = GPIO_MODE_INPUT,
    [7].gpio_pupd              = GPIO_PUPD_PULLDOWN,
    [7].en_exti                = false,
};

Gpio_HandleType ConnectGPIOHandle[CONNECT_GPIO_NUM];

void ConnectGPIOBspInit(void)
{
    uint8_t i = 0;
    for(i = 0; i < CONNECT_GPIO_NUM; ++i)
    {
        ConnectGPIOHandle[i].gpio_cfg = &_ConnectGPIOCfg[i];
        GpioInit(&ConnectGPIOHandle[i]);
    }
}


/*************工装输出IO测试配置****************/
static Gpio_Config _OutputGPIOCfg[2] =
{
    [0].gpiox                  = GPIOB,
    [0].gpio_pinx              = GPIO_PIN_14,
    [0].gpio_mode              = GPIO_MODE_OUTPUT,
    [0].gpio_pupd              = GPIO_PUPD_PULLUP,
    [0].gpio_otype             = GPIO_OTYPE_PP,
    [0].en_exti                = false,
        
    [1].gpiox                  = GPIOB,
    [1].gpio_pinx              = GPIO_PIN_15,
    [1].gpio_mode              = GPIO_MODE_OUTPUT,
    [1].gpio_pupd              = GPIO_PUPD_PULLUP,
    [1].gpio_otype             = GPIO_OTYPE_PP,
    [1].en_exti                = false,
};

Gpio_HandleType OutputGPIOHandle[2];
void OutputGPIOBspInit(void)
{
    uint8_t i = 0;
    for(i = 0; i < 2; ++i)
    {
        OutputGPIOHandle[i].gpio_cfg = &_OutputGPIOCfg[i];
        GpioInit(&OutputGPIOHandle[i]);
    }
}




/********TCA9555 END*********/

void BaseBspInit()
{
    ExtiClkInit();
    DMAClkInit();
    GpioClkInit();
}





