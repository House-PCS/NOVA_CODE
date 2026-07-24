/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
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
#include "fwdgt.h"
/////////////////////SPI/////////////////////////////////////

///////////////////////////////////////////////////
static Spi_Config FlashSpi_Cfg =
{
    .sck_gpiox          = GPIOE,
    .sck_gpio_pinx      = GPIO_PIN_12,
    .sck_alt_num        = GPIO_AF_5,
    
    .mosi_gpiox         = GPIOE,
    .mosi_gpio_pinx     = GPIO_PIN_14,
    .mosi_alt_num       = GPIO_AF_5,
    
    .miso_gpiox         = GPIOE,
    .miso_gpio_pinx     = GPIO_PIN_13,
    .miso_alt_num       = GPIO_AF_5,
    
//    .nss_gpiox          = GPIOE,
//    .nss_gpio_pinx      = GPIO_PIN_11,
//    .nss_alt_num        = GPIO_AF_5,
    
    .rcu_spi        = RCU_SPI3,
    .spi_peri       = SPI3,
    
    .device_mode            = SPI_MASTER,
    .trans_mode             = SPI_TRANSMODE_FULLDUPLEX,
    .frame_size             = SPI_FRAMESIZE_8BIT,
    .nss                    = SPI_NSS_SOFT,
    .endian                 = SPI_ENDIAN_MSB,
    .clock_polarity_phase   = SPI_CK_PL_HIGH_PH_2EDGE,
    .prescale               = SPI_PSC_16,
    
    .en_irq             = false,
        
    .en_dma_tx          = false,
    .en_dma_rx          = false,
};
static Gpio_Config SpiFlashCsCfg = 
{
        .gpiox                  = GPIOE,
        .gpio_pinx              = GPIO_PIN_11,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
};
Spi_HandleType gSpiFlashHandle;
Gpio_HandleType gSpiFlashCsHandle;
/** 
* @brief  外部Flash BSP层初始化
* @param  none
* @return  none
* @note:
*/
void SpiFlashBspInit()
{
    gSpiFlashHandle.spix = SPI3_PORT;
    gSpiFlashHandle.default_width = 8;
    gSpiFlashHandle.spi_cfg = &FlashSpi_Cfg;
    gSpiFlashCsHandle.gpio_cfg = &SpiFlashCsCfg;
//    FlashSpiHandle.gpio_cfg = &_SpiFlashWpCfg;
    
    SpiInit(&gSpiFlashHandle);
    GpioInit(&gSpiFlashCsHandle);
//    GpioInit(&WpSpiFlashHandle);
}
/////////////////////SPI_END/////////////////////////////////////

/////////////////////HT7017_UART/////////////////////////////////////
static Uart_Config _UartHt7017Cfg =
{
    .tx_gpiox       = GPIOF,
    .tx_gpio_pinx   = GPIO_PIN_7,
    .tx_alt_num     = GPIO_AF_8,
    .rx_gpiox       = GPIOF,
    .rx_gpio_pinx   = GPIO_PIN_6,
    .rx_alt_num     = GPIO_AF_8,

    .rcu_uart       = RCU_UART6,
    .uart_peri      = UART6,
    
    .baud_rate      = 4800,
    .wlen           = USART_WL_9BIT,
    .stblen         = USART_STB_1BIT,
    .parity_cfg     = USART_PM_EVEN,
    
    .en_irq         = true,
    .nvic_irq       = UART6_IRQn,
    .irq_prio       = 1,
    .irq_sub_prio   = 1,
    .irq_type       = EN_UART_INT_TYPE_RBNE,          //| EN_UART_INT_TYPE_TC     
    
    .en_dma_tx      = false,
    .en_dma_rx      = false,
};

Uart_HandleType gUartHT7017Handle;


/** 
* @brief  HT7017串口BSP初始化
* @param  none
* @return  none
* @note:
*
*/
void HT7017BspInit()
{
    gUartHT7017Handle.uartx = UART6_PORT;
    gUartHT7017Handle.uart_cfg = &_UartHt7017Cfg;
    
    UartInit(&gUartHT7017Handle);
}


///////////////////HT7017_UART_END///////////////
//////////////////////////////////////////////
/////////////////////COMM_UART/////////////////////////////////////
static Uart_Config _UartCommCfg[3] = 
{
    {
        .tx_gpiox               = GPIOD,
        .tx_gpio_pinx           = GPIO_PIN_5,
        .tx_alt_num             = GPIO_AF_7,
        .rx_gpiox               = GPIOD,
        .rx_gpio_pinx           = GPIO_PIN_6,
        .rx_alt_num             = GPIO_AF_7,
                                
        .rcu_uart               = RCU_USART1,
        .uart_peri              = USART1,
                                
        .baud_rate              = 9600,
        .wlen                   = USART_WL_8BIT,
        .stblen                 = USART_STB_1BIT,
        .parity_cfg             = USART_PM_NONE,
                                
        .en_irq                 = true,
        .nvic_irq               = USART1_IRQn,
        .irq_prio               = 1,
        .irq_sub_prio           = 1,
        .irq_type               = EN_UART_INT_TYPE_IDLE ,          // 
                                
        .en_dma_tx              = true,
        .en_dma_rx              = true,
                                
        .en_dma_tx_nowait       = true,
        .dma_tx_peri            = DMA0,
        .dma_tx_channel         = DMA_CH6,
        .dma_tx_subperi         = DMA_SUBPERI4,
        .en_dma_tx_irq          = true,
        .nvic_dma_tx_irq        = DMA0_Channel6_IRQn,
        .irq_dma_tx_prio        = 1,
        .irq_dma_tx_sub_prio    = 1,
            
            
        .dma_rx_peri            = DMA0,
        .dma_rx_channel         = DMA_CH5,
        .dma_rx_subperi         = DMA_SUBPERI4,
        .en_dma_rx_irq          = false,
    },
    {
        .tx_gpiox               = GPIOD,
        .tx_gpio_pinx           = GPIO_PIN_8,
        .tx_alt_num             = GPIO_AF_7,
        .rx_gpiox               = GPIOD,
        .rx_gpio_pinx           = GPIO_PIN_9,
        .rx_alt_num             = GPIO_AF_7,
                                  
        .rcu_uart               = RCU_USART2,
        .uart_peri              = USART2,
                                  
        .baud_rate              = 9600,
        .wlen                   = USART_WL_8BIT,
        .stblen                 = USART_STB_1BIT,
        .parity_cfg             = USART_PM_NONE,
                                  
        .en_irq                 = true,
        .nvic_irq               = USART2_IRQn,
        .irq_prio               = 1,
        .irq_sub_prio           = 1,
        .irq_type               = EN_UART_INT_TYPE_IDLE,          // 
        
        .en_dma_tx              = true,
        .en_dma_rx              = true, 
        .en_dma_tx_nowait       = true,
        .dma_tx_peri            = DMA0,
        .dma_tx_channel         = DMA_CH3,
        .dma_tx_subperi         = DMA_SUBPERI4,
        .en_dma_tx_irq          = true,
        .nvic_dma_tx_irq        = DMA0_Channel3_IRQn,
        .irq_dma_tx_prio        = 1,
        .irq_dma_tx_sub_prio    = 1,
                                  
                                  
        .dma_rx_peri            = DMA0,
        .dma_rx_channel         = DMA_CH1,
        .dma_rx_subperi         = DMA_SUBPERI4,
        .en_dma_rx_irq          = false,
    },
    {
        .tx_gpiox               = GPIOA,
        .tx_gpio_pinx           = GPIO_PIN_9,
        .tx_alt_num             = GPIO_AF_7,
        .rx_gpiox               = GPIOA,
        .rx_gpio_pinx           = GPIO_PIN_10,
        .rx_alt_num             = GPIO_AF_7,
                                  
        .rcu_uart               = RCU_USART0,
        .uart_peri              = USART0,
                                  
        .baud_rate              = 9600,
        .wlen                   = USART_WL_8BIT,
        .stblen                 = USART_STB_1BIT,
        .parity_cfg             = USART_PM_NONE,
                                  
        .en_irq                 = true,
        .nvic_irq               = USART0_IRQn,
        .irq_prio               = 1,
        .irq_sub_prio           = 1,
        .irq_type               = EN_UART_INT_TYPE_IDLE,          // 
        
        .en_dma_tx              = true,
        .en_dma_rx              = true, 
        .en_dma_tx_nowait       = true,
        .dma_tx_peri            = DMA1,
        .dma_tx_channel         = DMA_CH7,
        .dma_tx_subperi         = DMA_SUBPERI4,
        .en_dma_tx_irq          = false,              
                                  
        .dma_rx_peri            = DMA1,
        .dma_rx_channel         = DMA_CH5,
        .dma_rx_subperi         = DMA_SUBPERI4,
        .en_dma_rx_irq          = false,
    }
};

static Gpio_Config _RS485DECfg[2] =
{
    {
        .gpiox                  = GPIOD,
        .gpio_pinx              = GPIO_PIN_4,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOD,
        .gpio_pinx              = GPIO_PIN_10,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
};

Uart_HandleType gUart1ModbusHandle;
Gpio_HandleType gGpioDE1ModbusHandle;
Uart_HandleType gUart2ModbusHandle;
Gpio_HandleType gGpioDE2ModbusHandle;
Uart_HandleType gUart0TTLHandle;



void UartCommBspInit()
{
    gUart1ModbusHandle.uartx = UART1_PORT;
    gUart1ModbusHandle.uart_cfg = &_UartCommCfg[0];
    UartInit(&gUart1ModbusHandle);
    
    gGpioDE1ModbusHandle.gpio_cfg = &_RS485DECfg[0];
    GpioInit(&gGpioDE1ModbusHandle);
    
    gUart2ModbusHandle.uartx = UART2_PORT;
    gUart2ModbusHandle.uart_cfg = &_UartCommCfg[1];
    UartInit(&gUart2ModbusHandle);
    
    gGpioDE2ModbusHandle.gpio_cfg = &_RS485DECfg[1];
    GpioInit(&gGpioDE2ModbusHandle);
    
    gUart0TTLHandle.uartx = UART0_PORT;
    gUart0TTLHandle.uart_cfg = &_UartCommCfg[2];
    UartInit(&gUart0TTLHandle);
}


///////////////////COMM_UART_END///////////////
//////////////////////////////////////////
/////////////////////CAN/////////////////
static Can_Config _Can_Config[CAN_PORT_NUM] = 
{

    [0].tx_gpiox               = GPIOD,
    [0].tx_gpio_pinx           = GPIO_PIN_1,
    [0].tx_alt_num             = GPIO_AF_9,
    [0].rx_gpiox               = GPIOD,
    [0].rx_gpio_pinx           = GPIO_PIN_0,
    [0].rx_alt_num             = GPIO_AF_9,
    
    [0].rcu_can                = RCU_CAN0,
    [0].can_peri               = CAN0,
    [0].en_irq_rx0             = true,
    [0].nvic_irq_rx0           = CAN0_RX0_IRQn,        
    [0].irq_type_rx0           = CAN_INT_RFNE0,
    [0].prescaler              = CAN_BPS_250K,     
    [0].irq_prio_rx0           = 1,        
    [0].irq_sub_prio_rx0	   = 1,      
   
    
    [1].tx_gpiox               = GPIOB,
    [1].tx_gpio_pinx           = GPIO_PIN_6,
    [1].tx_alt_num             = GPIO_AF_9,
    [1].rx_gpiox               = GPIOB,
    [1].rx_gpio_pinx           = GPIO_PIN_5,
    [1].rx_alt_num             = GPIO_AF_9,
    
    [1].rcu_can                = RCU_CAN1,
    [1].can_peri               = CAN1,
    [1].en_irq_rx0             = true,
    [1].nvic_irq_rx0           = CAN1_RX0_IRQn,        
    [1].irq_type_rx0           = CAN_INT_RFNE0,
    [1].prescaler              = CAN_BPS_250K,     
    [1].irq_prio_rx0           = 1,        
    [1].irq_sub_prio_rx0	   = 2, 
};

Can_HandleType gCanHandle[CAN_PORT_NUM];
void CanBspInit(void)
{
    for(uint8_t i = 0; i < CAN_PORT_NUM; i++)     //  CAN_PORT_NUM
    {
        gCanHandle[i].canx = i;
        gCanHandle[i].can_cfg = &_Can_Config[i];
        CanInit(&gCanHandle[i]);
    }
}
//////////////CAN END/////////////////
///////////////////////////////////////////////
/////////////////////EEPROM_IIC///////////////
I2C_Config EEPROMI2cCfg =
{
    .scl_gpiox          = GPIOB,
    .scl_gpio_pinx      = GPIO_PIN_10,
    .scl_alt_num        = GPIO_AF_4,
    .sda_gpiox          = GPIOB,
    .sda_gpio_pinx      = GPIO_PIN_11,
    .sda_alt_num        = GPIO_AF_4,
    
    .rcu_i2c            = RCU_I2C1,
    .i2c_peri           = I2C1,
    .speed              = 400000,
    
    .en_irq             = false,
    
};
I2C_HandleType gEepromI2CHandle;
static SimI2C_Config _E2PROMI2C_Config =
{
    .scl_gpiox          = GPIOB,
    .scl_gpio_pinx      = GPIO_PIN_10,
    .sda_gpiox          = GPIOB,
    .sda_gpio_pinx      = GPIO_PIN_11,
};

SimI2C_HandleType gEepromSimI2CHandle;


/** 
* @brief  EEPROM BSP层初始化
* @param  none
* @return  none
* @note:
*
*/
void E2promBspInit()
{
    gEepromI2CHandle.i2c_cfg = &EEPROMI2cCfg;

    I2CInit(&gEepromI2CHandle);
}
//////////////////////RTC///
I2C_Config RTCI2cCfg =
{
    .scl_gpiox          = GPIOB,
    .scl_gpio_pinx      = GPIO_PIN_8,
    .scl_alt_num        = GPIO_AF_4,
    .sda_gpiox          = GPIOB,
    .sda_gpio_pinx      = GPIO_PIN_9,
    .sda_alt_num        = GPIO_AF_4,
    
    .rcu_i2c            = RCU_I2C0,
    .i2c_peri           = I2C0,
    .speed              = 100000,
    
    .en_irq             = false,
    
};
I2C_HandleType gRtcI2CHandle;
static SimI2C_Config _RtcCfg =
{
    .scl_gpiox              = GPIOB,
    .scl_gpio_pinx          = GPIO_PIN_8,
    
    .sda_gpiox              = GPIOB,
    .sda_gpio_pinx          = GPIO_PIN_9,
};
SimI2C_HandleType gSimRtcI2CHandle;

static Gpio_Config _RtcIntCfg =
{
    .gpiox                  = GPIOB,
    .gpio_pinx              = GPIO_PIN_7,
    .gpio_mode              = GPIO_MODE_INPUT,
    .gpio_pupd              = GPIO_PUPD_PULLDOWN,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = true,
        
    .gpio_exti_port         = EXTI_SOURCE_GPIOB,
    .gpio_exti_pin          = EXTI_SOURCE_PIN7,
    .gpio_exti_mode         = EXTI_INTERRUPT,
    .gpio_exti_triger       = EXTI_TRIG_RISING,
    
    .nvic_irq               = EXTI5_9_IRQn,
    .irq_prio               = 1,
    .irq_sub_prio           = 1,
};  
Gpio_HandleType gRtcIntHandle;

/** 
* @brief  RTC BSP层初始化
* @param  none
* @return  none
* @note:
*
*/
void RtcBspInit()
{
    gRtcI2CHandle.i2c_cfg = &RTCI2cCfg;
    I2CInit(&gRtcI2CHandle);
}



//////////////////////SGM58031///////////////////////////////////////////

SimI2C_Config SGM_I2cCfg =
{
    .scl_gpiox          = GPIOF,
    .scl_gpio_pinx      = GPIO_PIN_1,
    .sda_gpiox          = GPIOF,
    .sda_gpio_pinx      = GPIO_PIN_0,
};
SimI2C_HandleType gSGMI2CHandle;
/** 
* @brief  SGM58031 BSP层初始化
* @param  none
* @return  none
* @note:
*/
void SGMI2CBspInit()
{
    gSGMI2CHandle.simi2c_cfg = &SGM_I2cCfg;
    SimI2CInit(&gSGMI2CHandle);
}


//////////////IIC_END/////////////////
///////////////////////////////////////////////
/////////////////////ADC//////////////
static Adc_Config _AdcCfg = 
{
    .channel_config[0].enable               = true,
    .channel_config[0].chl_id               = ADC_CHANNEL_10,
    .channel_config[0].adc_gpiox            = GPIOC,
    .channel_config[0].adc_gpio_pinx        = GPIO_PIN_0,

    .channel_config[1].enable               = true,
    .channel_config[1].chl_id               = ADC_CHANNEL_12,
    .channel_config[1].adc_gpiox            = GPIOC,
    .channel_config[1].adc_gpio_pinx        = GPIO_PIN_2,
    
    .channel_config[2].enable               = true,
    .channel_config[2].chl_id               = ADC_CHANNEL_14,
    .channel_config[2].adc_gpiox            = GPIOF,
    .channel_config[2].adc_gpio_pinx        = GPIO_PIN_4,
    
    .channel_config[3].enable               = true,
    .channel_config[3].chl_id               = ADC_CHANNEL_15,
    .channel_config[3].adc_gpiox            = GPIOF,
    .channel_config[3].adc_gpio_pinx        = GPIO_PIN_5,
    
    .rcu_adc                                = RCU_ADC2,
    .adc_peri                               = ADC2,
#if ADC_USE_CONT_MODE
    .continuous_mode                        = true,
    .scan_mode                              = true,
    .en_dma                                 = true,
    .dma_peri                               = DMA1,
    .dma_channel                            = DMA_CH0,
    .dma_subperi                            = DMA_SUBPERI2,
    
    .en_dma_irq                             = true,
    .nvic_dma_irq                           = DMA1_Channel0_IRQn,
    .irq_dma_prio                           = 1,
    .irq_dma_sub_prio                       = 1,
#else
    .continuous_mode                        = false,
    .scan_mode                              = false,
    .en_dma                                 = false,
    .en_irq                                 = false,
#endif    
    
};

Adc_HandleType gAdc2Handle;


/** 
* @brief  ADC采样 BSP层初始化
* @param  none
* @return  none
* @note:
*
*/
void Adc2BspInit()
{
    gAdc2Handle.adcx = ADC2_PORT;
    gAdc2Handle.adc_cfg = &_AdcCfg;
    AdcInit(&gAdc2Handle);
}




////////////////ADC_END///////////////
//////////////////////////////////////
//////////////IO/////////////////
//双向输出
static Gpio_Config _ioctl_two_way_output_cfg[IO_2WAY_NUM] = 
{
    {
        .gpiox                  = GPIOF,
        .gpio_pinx              = GPIO_PIN_14,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOD,
        .gpio_pinx              = GPIO_PIN_2,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOG,
        .gpio_pinx              = GPIO_PIN_10,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOG,
        .gpio_pinx              = GPIO_PIN_12,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    }
};

//双向输入
static Gpio_Config _ioctl_two_way_input_cfg[IO_2WAY_NUM] = 
{
    {
        .gpiox                  = GPIOF,
        .gpio_pinx              = GPIO_PIN_15,
        .gpio_mode              = GPIO_MODE_INPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOD,
        .gpio_pinx              = GPIO_PIN_3,
        .gpio_mode              = GPIO_MODE_INPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOG,
        .gpio_pinx              = GPIO_PIN_11,
        .gpio_mode              = GPIO_MODE_INPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOG,
        .gpio_pinx              = GPIO_PIN_13,
        .gpio_mode              = GPIO_MODE_INPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .en_exti                = false,
    }
};

//低有效输入
static Gpio_Config _ioctl_act_low_input_cfg[IO_ACT_LOW_INPUT_NUM] = 
{
    {
        .gpiox                  = GPIOG,
        .gpio_pinx              = GPIO_PIN_6,
        .gpio_mode              = GPIO_MODE_INPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOG,
        .gpio_pinx              = GPIO_PIN_5,
        .gpio_mode              = GPIO_MODE_INPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOG,
        .gpio_pinx              = GPIO_PIN_4,
        .gpio_mode              = GPIO_MODE_INPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .en_exti                = false,
    },
};

//高有效输入
static Gpio_Config _ioctl_act_high_input_cfg[IO_ACT_HIGH_INPUT_NUM] = 
{
    {
        .gpiox                  = GPIOD,
        .gpio_pinx              = GPIO_PIN_13,
        .gpio_mode              = GPIO_MODE_INPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOG,
        .gpio_pinx              = GPIO_PIN_8,
        .gpio_mode              = GPIO_MODE_INPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOG,
        .gpio_pinx              = GPIO_PIN_7,
        .gpio_mode              = GPIO_MODE_INPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOG,
        .gpio_pinx              = GPIO_PIN_9,
        .gpio_mode              = GPIO_MODE_INPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .en_exti                = false,
    },
};

//隔离输入
static Gpio_Config _ioctl_iso_input_cfg[IO_ISO_INPUT_NUM] = 
{
    {
        .gpiox                  = GPIOD,
        .gpio_pinx              = GPIO_PIN_15,
        .gpio_mode              = GPIO_MODE_INPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .en_exti                = false,
    }
};

//低边输出
static Gpio_Config _ioctl_low_side_output_cfg[IO_LOW_SIDE_OUTPUT_NUM] = 
{
    {
        .gpiox                  = GPIOG,
        .gpio_pinx              = GPIO_PIN_0,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOG,
        .gpio_pinx              = GPIO_PIN_1,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOE,
        .gpio_pinx              = GPIO_PIN_7,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOD,
        .gpio_pinx              = GPIO_PIN_12,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    }
};

//高边输出
static Gpio_Config _ioctl_high_side_output_cfg[IO_HIGH_SIDE_OUTPUT_NUM] = 
{
    {
        .gpiox                  = GPIOE,
        .gpio_pinx              = GPIO_PIN_10,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOE,
        .gpio_pinx              = GPIO_PIN_9,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOE,
        .gpio_pinx              = GPIO_PIN_8,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOD,
        .gpio_pinx              = GPIO_PIN_11,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    }
};

//隔离输出
static Gpio_Config _ioctl_iso_output_cfg[IO_ISO_OUTPUT_NUM] = 
{
    {
        .gpiox                  = GPIOF,
        .gpio_pinx              = GPIO_PIN_8,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOF,
        .gpio_pinx              = GPIO_PIN_9,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOF,
        .gpio_pinx              = GPIO_PIN_11,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    }
};

//干接点输出
static Gpio_Config _ioctl_dry_output_cfg[IO_DRY_OUTPUT_NUM] = 
{
    {
        .gpiox                  = GPIOF,
        .gpio_pinx              = GPIO_PIN_13,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    },
    {
        .gpiox                  = GPIOF,
        .gpio_pinx              = GPIO_PIN_12,
        .gpio_mode              = GPIO_MODE_OUTPUT,
        .gpio_pupd              = GPIO_PUPD_NONE,
        .gpio_otype             = GPIO_OTYPE_PP,
        .en_exti                = false,
    }
};

Gpio_HandleType gTwoWayOutputHandle[IO_2WAY_NUM];
Gpio_HandleType gTwoWayInputHandle[IO_2WAY_NUM];
Gpio_HandleType gActLowInputHandle[IO_ACT_LOW_INPUT_NUM];
Gpio_HandleType gActHighInputHandle[IO_ACT_HIGH_INPUT_NUM];
Gpio_HandleType gIsoInputHandle[IO_ISO_INPUT_NUM];
Gpio_HandleType gLowSideOutputHandle[IO_LOW_SIDE_OUTPUT_NUM];
Gpio_HandleType gHighSideOutputHandle[IO_HIGH_SIDE_OUTPUT_NUM];
Gpio_HandleType gIsoOutputHandle[IO_ISO_OUTPUT_NUM];
Gpio_HandleType gDryOutputHandle[IO_DRY_OUTPUT_NUM];


/** 
* @brief  IO控制bsp初始化
* @param  none
* @return  none
* @note:
*
*/
void IOCtlBspInit()
{
    //双向IO
    for(uint8_t i = 0; i < IO_2WAY_NUM; i++)
    {
        gTwoWayOutputHandle[i].gpio_cfg = &_ioctl_two_way_output_cfg[i];
        gTwoWayInputHandle[i].gpio_cfg = &_ioctl_two_way_input_cfg[i];
        GpioInit(&gTwoWayOutputHandle[i]);
        GpioInit(&gTwoWayInputHandle[i]);
    }
    //低有效输入
    for(uint8_t i = 0; i < IO_ACT_LOW_INPUT_NUM; i++)
    {
        gActLowInputHandle[i].gpio_cfg = &_ioctl_act_low_input_cfg[i];
        GpioInit(&gActLowInputHandle[i]);
    }
    //高有效输入
    for(uint8_t i = 0; i < IO_ACT_HIGH_INPUT_NUM; i++)
    {
        gActHighInputHandle[i].gpio_cfg = &_ioctl_act_high_input_cfg[i];
        GpioInit(&gActHighInputHandle[i]);
    }
    //隔离输入
    for(uint8_t i = 0; i < IO_ISO_INPUT_NUM; i++)
    {
        gIsoInputHandle[i].gpio_cfg = &_ioctl_iso_input_cfg[i];
        GpioInit(&gIsoInputHandle[i]);
    }
    //低边输出
    for(uint8_t i = 0; i < IO_LOW_SIDE_OUTPUT_NUM; i++)
    {
        gLowSideOutputHandle[i].gpio_cfg = &_ioctl_low_side_output_cfg[i];
        GpioInit(&gLowSideOutputHandle[i]);
    }
    //高边输出
    for(uint8_t i = 0; i < IO_HIGH_SIDE_OUTPUT_NUM; i++)
    {
        gHighSideOutputHandle[i].gpio_cfg = &_ioctl_high_side_output_cfg[i];
        GpioInit(&gHighSideOutputHandle[i]);
    }
    //隔离输出
    for(uint8_t i = 0; i < IO_ISO_OUTPUT_NUM; i++)
    {
        gIsoOutputHandle[i].gpio_cfg = &_ioctl_iso_output_cfg[i];
        GpioInit(&gIsoOutputHandle[i]);
    }
    //干接点输出
    for(uint8_t i = 0; i < IO_DRY_OUTPUT_NUM; i++)
    {
        gDryOutputHandle[i].gpio_cfg = &_ioctl_dry_output_cfg[i];
        GpioInit(&gDryOutputHandle[i]);
    }
}

//////////////IO END/////////////////



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
    .nvic_irq               = TIMER5_DAC_IRQn,
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
    .gpiox                  = GPIOG,
    .gpio_pinx              = GPIO_PIN_15,
    .gpio_mode              = GPIO_MODE_OUTPUT,
    .gpio_pupd              = GPIO_PUPD_NONE,
    .gpio_otype             = GPIO_OTYPE_PP,
    .en_exti                = false,
};
Gpio_HandleType gGpioLedHandle;
void GpioLedBspInit()
{
    gGpioLedHandle.gpio_cfg = &_GpioLedCfg;
    GpioInit(&gGpioLedHandle);
}

//////////////LED指示灯 End/////////////////







//uint8_t buff[]={1,2,3,4,5};
//uint8_t red_buff[64];
/** 
* @brief  MCU BSP层初始化
* @param  none
* @return  none
* @note:
*
*/
void McuBspInit()
{
    DMAClkInit();
    GpioClkInit();
    TimerClkInit();
//    FWDGT_Init();
    
}



