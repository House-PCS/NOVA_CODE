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


/*comm with maincard*/
static Uart_Config _ModbusHostUartCfg = 
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

Uart_HandleType ModbusHostUartHandle;


static Gpio_Config  _ModbusHostCtlGpioConfig = 
{
    .gpiox                      = GPIOC,
    .gpio_pinx                  = GPIO_PIN_15,
    .gpio_mode                  = GPIO_Mode_IN_FLOATING,
    .en_exti                    = false,
};
Gpio_HandleType ModbusHostCtlIOHandle;

void ModbusHostBspInit(void)
{
    ModbusHostUartHandle.uartx = UART3_PORT;
    ModbusHostUartHandle.uart_cfg = &_ModbusHostUartCfg;
    
    UartInit(&ModbusHostUartHandle);
    
    ModbusHostCtlIOHandle.gpio_cfg = &_ModbusHostCtlGpioConfig;
    GpioInit(&ModbusHostCtlIOHandle);
}

/*DEBUG UART*/
static Uart_Config _DbgUartCfg = 
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
Uart_HandleType DbgUartHandle;
void DbgUartBspInit(void)
{
    DbgUartHandle.uartx = UART1_PORT;
    DbgUartHandle.uart_cfg = &_DbgUartCfg;
    
    UartInit(&DbgUartHandle);
}


/*ATMOS LED*/
//左侧灯板 
static Gpio_Config  _LedCtrlLeftGpioConfig = 
{
    .gpiox                      = GPIOA,
    .gpio_pinx                  = GPIO_PIN_6,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    .en_exti                    = false,
};

//右侧灯板
static Gpio_Config  _LedCtrlRightGpioConfig = 
{
    .gpiox                      = GPIOA,
    .gpio_pinx                  = GPIO_PIN_7,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    .en_exti                    = false,
};
//按键灯
static Gpio_Config  _LedKeyGpioConfig = 
{
    .gpiox                      = GPIOA,
    .gpio_pinx                  = GPIO_PIN_0,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    .en_exti                    = false,
};
Gpio_HandleType AtmosLedHandle[6];
Gpio_HandleType SideAtmosLedHandle[2];
void AtmosLedBspInit(void)
{
    AtmosLedHandle[3].gpio_cfg = &_LedKeyGpioConfig;
    AtmosLedHandle[4].gpio_cfg = &_LedCtrlLeftGpioConfig;
    AtmosLedHandle[5].gpio_cfg = &_LedCtrlRightGpioConfig;
    
    GpioInit(&AtmosLedHandle[3]);
    GpioInit(&AtmosLedHandle[4]);
    GpioInit(&AtmosLedHandle[5]);

    GpioBitReset(&AtmosLedHandle[3]);
    GpioBitReset(&AtmosLedHandle[4]);
    GpioBitReset(&AtmosLedHandle[5]);
}

void SideLedBspInit(void)
{
//    SideAtmosLedHandle[0].gpio_cfg = &_LedCtrlLeftGpioConfig ;
//    SideAtmosLedHandle[1].gpio_cfg = &_LedCtrlRightGpioConfig;
//    
//    GpioInit(&SideAtmosLedHandle[0]);
//    GpioInit(&SideAtmosLedHandle[1]);
//    
//    GpioBitReset(&SideAtmosLedHandle[0]);
//    GpioBitReset(&SideAtmosLedHandle[1]);
}

//test IO
static Gpio_Config  _TESTGpioConfig = 
{
    .gpiox                      = GPIOC,
    .gpio_pinx                  = GPIO_PIN_4,
    .gpio_mode                  = GPIO_Mode_Out_PP,
    .en_exti                    = false,
};
Gpio_HandleType TESTIOHandle;

void TestIOInit(void)
{
    TESTIOHandle.gpio_cfg = &_TESTGpioConfig;
    GpioInit(&TESTIOHandle);
}

void SetTestIo(void)
{
    GpioBitSet(&TESTIOHandle);
}
void ReSetTestIo(void)
{
    GpioBitReset(&TESTIOHandle);
}




/***用于轮询的定时器***/
static Timer_Config _TimerPollingCfg =
{
    .rcc_timer              = RCC_APB1_PERIPH_TIM5,
    .timer_peri             = TIM5,             //36M
    .prescaler              = 72,//100,    //根据系统时钟频率变化
//    .alignedmode            = TIMER_COUNTER_EDGE,
    .counterdirection       = TIM_CNT_MODE_UP,
    .clockdivision          = TIM_CLK_DIV1,
    .period                 = 2000,                 //5ms
    .repetitioncounter      = 0,

    .en_irq                 = true,
    .nvic_irq               = TIM5_IRQn,
    .irq_prio               = 0,
    .irq_sub_prio           = 1,
    .irq_type               = TIM_INT_UPDATE,
};
Timer_HandleType TimerPollingHandle;

void TimerPollingBspInit(void)
{
    TimerPollingHandle.timerx = TIMER5_PERI;
    TimerPollingHandle.timer_cfg = &_TimerPollingCfg;
    TimerInit(&TimerPollingHandle);
}

void BaseBspInit(void)
{
    DMAClkInit();
    GpioClkInit();
}


