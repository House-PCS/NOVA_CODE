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
    .gpiox                      = GPIOB,
    .gpio_pinx                  = GPIO_PIN_5,
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
//    LedPcsHandle.gpio_cfg = &_LedPcsGpioConfig;
//    LedConnHandle.gpio_cfg = &_LedConnGpioConfig;
    
    GpioInit(&LedSysHandle);
//    GpioInit(&LedPcsHandle);
//    GpioInit(&LedConnHandle);
}




/*******UARTºÍÖ÷¿Ø½»»¥******/           //  485
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
}

static Gpio_Config _DO_Cfg [DO_NUM]=
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
//		{
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
}

bool gDO_State[DO_NUM]={false};
bool gDO_ForceFlg = false;

void DO_Main()
{
//    static bool do_state[DO_NUM];
    for(uint8_t i = 0; i < DO_NUM; i++)
    {
        if(GpioOutputBitGet((&DO_Handle[i])) != gDO_State[i])
        {
            GpioBitWrite((&DO_Handle[i]),gDO_State[i]);

        }
    }
}
//////////////IO_OUT END/////////////////




