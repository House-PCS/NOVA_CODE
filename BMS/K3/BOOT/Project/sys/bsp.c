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
    .irq_type           = USART_INT_IDLE,

    .en_dma_tx          = true,
    .en_dma_rx          = true,

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

    .dma_rx_peri        = DMA0,
    .dma_rx_channel     = DMA_CH5,
};


Uart_HandleType Uart1_ModS_Handle;


void Uart1ModsBspInit(void)
{
    Uart1_ModS_Handle.uartx = UART1_PORT;
    Uart1_ModS_Handle.uart_cfg = &_Uart1Cfg;

    UartInit(&Uart1_ModS_Handle);
}


/******************LED*********************/
static Gpio_Config _LEDCfg =
{
    .gpiox                  = GPIOD,
    .gpio_pinx              = GPIO_PIN_8,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

Gpio_HandleType LED_Handle;

void LEDBspInit(void)
{
    LED_Handle.gpio_cfg = &_LEDCfg;
    GpioInit(&LED_Handle);
}

/************************Power Hold********************/
static Gpio_Config _PowerHoldCfg =
{
    .gpiox                  = GPIOE,
    .gpio_pinx              = GPIO_PIN_6,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};
Gpio_HandleType PowerHoldHandle;

void PowerHoldBspInit(void)
{
    PowerHoldHandle.gpio_cfg    = &_PowerHoldCfg;
    GpioInit(&PowerHoldHandle);
}

/******************externa2 wdg in*********************/
static Gpio_Config _ExtWdgInCfg =
{
    .gpiox                  = GPIOA,
    .gpio_pinx              = GPIO_PIN_12,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};
Gpio_HandleType ExtWdgInHandle;

void ExtWdgOut2BspInit(void)
{
    ExtWdgInHandle.gpio_cfg   = &_ExtWdgInCfg;
    GpioInit(&ExtWdgInHandle);
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
    .prescale               = SPI_PSC_4,

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

static Gpio_Config _SpiFlashWpCfg =
{
    .gpiox                  = GPIOA,
    .gpio_pinx              = GPIO_PIN_8,
    .gpio_mode              = GPIO_MODE_OUT_PP,
    .en_exti                = false,
};

Spi_HandleType  SpiFlashHandle;
Gpio_HandleType CsSpiFlashHandle;
Gpio_HandleType WpSpiFlashHandle;

void SpiFlashBspInit(void)
{
    SpiFlashHandle.spix = SPI0_PORT;
    SpiFlashHandle.default_width = 8;
    SpiFlashHandle.spi_cfg = &_SpiFlashSpiCfg;
    CsSpiFlashHandle.gpio_cfg = &_SpiFlashCsCfg;
    WpSpiFlashHandle.gpio_cfg = &_SpiFlashWpCfg;

    SpiInit(&SpiFlashHandle);
    GpioInit(&CsSpiFlashHandle);
    GpioInit(&WpSpiFlashHandle);
}

static Gpio_Config _KeyCfg =
{
    .gpiox                  = GPIOC,
    .gpio_pinx              = GPIO_PIN_3,
    .gpio_mode              = GPIO_MODE_IN_FLOATING,
    .en_exti                = false,
};

Gpio_HandleType KeySwitchHandle;

void KeyBspInit(void)
{
    KeySwitchHandle.gpio_cfg   = &_KeyCfg;
    GpioInit(&KeySwitchHandle);
}

void BspInit(void)
{
    DMAClkInit();
    GpioClkInit();
    LEDBspInit();
    ExtWdgOut2BspInit();
    PowerHoldBspInit();
}















