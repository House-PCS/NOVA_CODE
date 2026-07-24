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


/*WIFI/4G Module UART*/
static Uart_Config _WirelessUartCfg = 
{
    .tx_gpiox                   = GPIOB,
    .tx_gpio_pinx               = GPIO_PIN_10,
    
    .rx_gpiox                   = GPIOB,
    .rx_gpio_pinx               = GPIO_PIN_11,
    
    .rcc_uart                   = RCC_APB1_PERIPH_USART3,
    .uart_peri                  = USART3,
    .alt_rmp_mode               = 2,
    
    .baud_rate                  = 115200,
    .wlen                       = USART_WL_8B,  
    .stblen                     = USART_STPB_1, 
    .parity_cfg                 = USART_PE_NO,
    
    .en_irq                     = true,
    .nvic_irq                   = USART3_IRQn,
    .irq_prio                   = 1,
    .irq_sub_prio               = 1,
    .irq_type                   = EN_UART_INT_TYPE_IDLEF,
    
    .en_dma_tx                  = true,
    .en_dma_rx                  = true,
    .en_dma_tx_nowait           = false,
        
    .dma_tx_peri                = DMA1,
    .dma_tx_channel             = DMA_CH2,
    .en_dma_tx_irq              = false,
        
    .dma_rx_peri                = DMA1,
    .dma_rx_channel             = DMA_CH3,
    .en_dma_rx_irq              = false,
};

Uart_HandleType WirelessUartHandle;

/*WIFI/4G Module IO*/
static Gpio_Config  _WirelessResetGpioConfig = 
{
    .gpiox                      = GPIOB,
    .gpio_pinx                  = GPIO_PIN_3,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    
    .en_exti                    = false,
};
Gpio_HandleType WirelessResetIOHandle;

static Gpio_Config  _WirelessPowerEnGpioConfig = 
{
    .gpiox                      = GPIOA,
    .gpio_pinx                  = GPIO_PIN_0,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    
    .en_exti                    = false,
};
Gpio_HandleType WirelessPowerEnIOHandle;


void WirelessBspInit()
{
    WirelessUartHandle.uartx = UART3_PORT;
    WirelessUartHandle.uart_cfg = &_WirelessUartCfg;
    WirelessResetIOHandle.gpio_cfg = &_WirelessResetGpioConfig;
    WirelessPowerEnIOHandle.gpio_cfg = &_WirelessPowerEnGpioConfig;
    UartInit(&WirelessUartHandle);
    GpioInit(&WirelessResetIOHandle);
    GpioInit(&WirelessPowerEnIOHandle);
}


/*MODBUS PCS*/
static Uart_Config _ModbusHostUartCfg = 
{
    .tx_gpiox                   = GPIOA,
    .tx_gpio_pinx               = GPIO_PIN_9,
    
    .rx_gpiox                   = GPIOA,
    .rx_gpio_pinx               = GPIO_PIN_10,
    
    .rcc_uart                   = RCC_APB2_PERIPH_USART1,
    .uart_peri                  = USART1,
    .alt_rmp_mode               = 2,
    
    .baud_rate                  = 9600,
    .wlen                       = USART_WL_8B,  
    .stblen                     = USART_STPB_1, 
    .parity_cfg                 = USART_PE_NO,
    
    .en_irq                     = true,
    .nvic_irq                   = USART1_IRQn,
    .irq_prio                   = 1,
    .irq_sub_prio               = 1,
    .irq_type                   = EN_UART_INT_TYPE_IDLEF,
    
    .en_dma_tx                  = true,
    .en_dma_rx                  = true,
    .en_dma_tx_nowait           = false,
        
    .dma_tx_peri                = DMA1,
    .dma_tx_channel             = DMA_CH4,
    .en_dma_tx_irq              = false,
        
    .dma_rx_peri                = DMA1,
    .dma_rx_channel             = DMA_CH5,
    .en_dma_rx_irq              = false,
};

Uart_HandleType ModbusHostUartHandle;


static Gpio_Config  _ModbusHostCtlGpioConfig = 
{
    .gpiox                      = GPIOC,
    .gpio_pinx                  = GPIO_PIN_15,
    .gpio_mode                  = GPIO_Mode_IN_FLOATING,//GPIO_Mode_Out_PP,
    .en_exti                    = false,
};
Gpio_HandleType ModbusHostCtlIOHandle;

void ModbusHostBspInit(uint32_t baud_rate)
{
	_ModbusHostUartCfg.baud_rate = baud_rate;
    ModbusHostUartHandle.uartx = UART1_PORT;
    ModbusHostUartHandle.uart_cfg = &_ModbusHostUartCfg;
    
    UartInit(&ModbusHostUartHandle);
    
    ModbusHostCtlIOHandle.gpio_cfg = &_ModbusHostCtlGpioConfig;
    GpioInit(&ModbusHostCtlIOHandle);
}


/*DEBUG UART*/
static Uart_Config _DbgUartCfg = 
{
    .tx_gpiox                   = GPIOB,
    .tx_gpio_pinx               = GPIO_PIN_4,
    
    .rx_gpiox                   = GPIOB,
    .rx_gpio_pinx               = GPIO_PIN_5,
    
    .rcc_uart                   = RCC_APB1_PERIPH_USART2,
    .uart_peri                  = USART2,
    .alt_rmp_mode               = 3,
    .alt_rmp_function           = GPIO_RMP3_USART2,
    
    .baud_rate                  = 115200,
    .wlen                       = USART_WL_8B,  
    .stblen                     = USART_STPB_1, 
    .parity_cfg                 = USART_PE_NO,
    
    .en_irq                     = false,
    .nvic_irq                   = USART2_IRQn,
    .irq_prio                   = 1,
    .irq_sub_prio               = 1,
    .irq_type                   = EN_UART_INT_TYPE_IDLEF,
    
    .en_dma_tx                  = false,
    .en_dma_rx                  = false,
    .en_dma_tx_nowait           = false,
        
};
Uart_HandleType DbgUartHandle;
void DbgUartBspInit()
{
    DbgUartHandle.uartx = UART2_PORT;
    DbgUartHandle.uart_cfg = &_DbgUartCfg;
    
    UartInit(&DbgUartHandle);
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
}



/*LED*/
static Gpio_Config  _LedSysGpioConfig = 
{
    .gpiox                      = GPIOA,
    .gpio_pinx                  = GPIO_PIN_11,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    .en_exti                    = false,
};
Gpio_HandleType LedSysHandle;

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
    LedPcsHandle.gpio_cfg = &_LedPcsGpioConfig;
    LedConnHandle.gpio_cfg = &_LedConnGpioConfig;
    
    GpioInit(&LedSysHandle);
    GpioInit(&LedPcsHandle);
    GpioInit(&LedConnHandle);
}

/*KEY*/
static Gpio_Config  _KeyRstGpioConfig = 
{
    .gpiox                      = GPIOC,
    .gpio_pinx                  = GPIO_PIN_4,
    .gpio_mode                  = GPIO_Mode_IPU,
    .en_exti                    = false,
};
Gpio_HandleType KeyRstHandle;

void KeyBspInit()
{
    KeyRstHandle.gpio_cfg       = &_KeyRstGpioConfig;
    
    GpioInit(&KeyRstHandle);
}

/*Board id*/
static Gpio_Config  _BoardIDGpioConfig = 
{
    .gpiox                      = GPIOC,
    .gpio_pinx                  = GPIO_PIN_5,
    .gpio_mode                  = GPIO_Mode_IPU,
    .en_exti                    = false,
};
Gpio_HandleType BoardIDGpioHandle;
void BoardIDBspInit()
{
    BoardIDGpioHandle.gpio_cfg       = &_BoardIDGpioConfig;
    
    GpioInit(&BoardIDGpioHandle);
}


void BaseBspInit()
{
    DMAClkInit();
    GpioClkInit();
}


