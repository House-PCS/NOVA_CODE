/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief       板级支持包(BSP)配置文件，包含ADC、UART、SPI、I2C、GPIO、TIMER等外设的
  *              硬件配置和初始化函数，为上层应用提供统一的硬件访问接口
  ******************************************************************************
  * @attention
  * 1. 本文件针对特定MCU平台设计，所有外设引脚、时钟、中断优先级等配置需与硬件原理图一致
  * 2. 初始化函数需在系统启动时按依赖顺序调用，核心初始化入口为BspInit()
  * 3. 外设配置结构体采用静态定义，避免全局变量污染，通过句柄暴露给上层
  * @relation
  * @par :
  * <table>
  * <tr><<th>Date          <<th>Version   <<th>Author      <<th>Description
  * <tr><td>2025-6-1       <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "bsp.h"
#define LOG_TAG     "bsp"  // 日志标签，用于调试输出

/**********************ADC模块配置与初始化*****************************/
/**
 * @brief ADC外设配置结构体，包含9个通道的配置信息（对应不同的检测参数）
 * @note 所有ADC通道均启用，使用ADC1外设，不使能中断模式
 */
static Adc_Config _AdcCfg =
{
    // 通道0：R- 检测（电阻负极）- PC5引脚
    .channel_config[0].enable         = true,
    .channel_config[0].chl_num        = ADC_CHANNEL_15,
    .channel_config[0].adc_gpiox      = GPIOC,
    .channel_config[0].adc_gpio_pinx  = GPIO_PIN_5,
    
    // 通道1：R+ 检测（电阻正极）- PC4引脚
    .channel_config[1].enable         = true,
    .channel_config[1].chl_num        = ADC_CHANNEL_14,
    .channel_config[1].adc_gpiox      = GPIOC,
    .channel_config[1].adc_gpio_pinx  = GPIO_PIN_4,
    
    // 通道2：版本ID检测 - PC3引脚
    .channel_config[2].enable         = true,
    .channel_config[2].chl_num        = ADC_CHANNEL_13,
    .channel_config[2].adc_gpiox      = GPIOC,
    .channel_config[2].adc_gpio_pinx  = GPIO_PIN_3,
    
    // 通道3：电芯串数ID检测 - PC2引脚
    .channel_config[3].enable         = true,
    .channel_config[3].chl_num        = ADC_CHANNEL_12,
    .channel_config[3].adc_gpiox      = GPIOC,
    .channel_config[3].adc_gpio_pinx  = GPIO_PIN_2,
    
    // 通道4：NTC2温度传感器检测 - PC1引脚
    .channel_config[4].enable         = true,
    .channel_config[4].chl_num        = ADC_CHANNEL_11,
    .channel_config[4].adc_gpiox      = GPIOC,
    .channel_config[4].adc_gpio_pinx  = GPIO_PIN_1,
    
    // 通道5：NTC1温度传感器检测 - PC0引脚
    .channel_config[5].enable         = true,
    .channel_config[5].chl_num        = ADC_CHANNEL_10,
    .channel_config[5].adc_gpiox      = GPIOC,
    .channel_config[5].adc_gpio_pinx  = GPIO_PIN_0,
    
    // 通道6：BMS总电压检测 - PB1引脚
    .channel_config[6].enable         = true,
    .channel_config[6].chl_num        = ADC_CHANNEL_9,
    .channel_config[6].adc_gpiox      = GPIOB,
    .channel_config[6].adc_gpio_pinx  = GPIO_PIN_1,
    
    // 通道7：BMS电流检测1 - PA1引脚
    .channel_config[7].enable         = true,
    .channel_config[7].chl_num        = ADC_CHANNEL_1,
    .channel_config[7].adc_gpiox      = GPIOA,
    .channel_config[7].adc_gpio_pinx  = GPIO_PIN_1,
    
    // 通道8：BMS电流检测2 - PA0引脚
    .channel_config[8].enable         = true,
    .channel_config[8].chl_num        = ADC_CHANNEL_0,
    .channel_config[8].adc_gpiox      = GPIOA,
    .channel_config[8].adc_gpio_pinx  = GPIO_PIN_0,
    
    .rcu_adc                          = RCU_ADC1,        // ADC1时钟使能配置
    .adc_peri                         = ADC1,            // 所用ADC外设为ADC1
    .en_irq                           = false,           // 不使能ADC中断
    .nvic_irq                         = ADC0_1_IRQn,     // ADC中断向量号（备用）
    .irq_prio                         = 1,               // 中断主优先级（备用）
    .irq_sub_prio                     = 1,               // 中断子优先级（备用）
};

Adc_HandleType Adc_Handle;  // ADC外设句柄，供上层调用

/**
 * @brief ADC模块板级初始化函数
 * @param  无
 * @retval 无
 * @note   初始化ADC句柄参数，绑定配置结构体，调用底层ADC初始化接口
 */
void ADCBspInit()
{
    uint8_t c_current_chl = ADC_CHANNEL_15;  // 初始通道设为通道15（R-检测）
    float RValtage = 0;                      // 电阻电压存储变量
    
    Adc_Handle.adc_cfg = &_AdcCfg;           // 绑定ADC配置结构体
    Adc_Handle.current_chl = c_current_chl;  // 设置初始通道
    Adc_Handle.pRValtage = &RValtage;        // 绑定电压存储变量指针
    AdcInit(&Adc_Handle);                    // 调用底层ADC初始化函数
}

/******************UART1（与FPGA软核通信）配置与初始化*********************/
/**
 * @brief UART1配置结构体，用于与FPGA软核进行串口通信
 * @note 波特率115200，8位数据位，1位停止位，无校验，使能DMA收发
 */
static Uart_Config _Uart1Cfg =
{
    .tx_gpiox           = GPIOA,            // TX引脚端口：GPIOA
    .tx_gpio_pinx       = GPIO_PIN_2,       // TX引脚：PA2
    .rx_gpiox           = GPIOA,            // RX引脚端口：GPIOA
    .rx_gpio_pinx       = GPIO_PIN_3,       // RX引脚：PA3
    .rcu_uart           = RCU_USART1,       // USART1时钟使能配置
    .uart_peri          = USART1,           // 所用UART外设为USART1
    .nvic_irq           = USART1_IRQn,      // USART1中断向量号
    .baud_rate          = 115200,           // 波特率：115200
    .wlen               = USART_WL_8BIT,    // 数据位：8位
    .stblen             = USART_STB_1BIT,   // 停止位：1位
    .parity_cfg         = USART_PM_NONE,    // 校验位：无
    .en_irq             = true,             // 使能UART中断
    .irq_prio           = 1,                // 中断主优先级：1
    .irq_sub_prio       = 1,                // 中断子优先级：1
    .irq_type           = USART_INT_IDLE,   // 中断类型：空闲中断
    .en_dma_tx          = true,             // 使能DMA发送
    .en_dma_rx          = true,             // 使能DMA接收
    .en_dma_tx_nowait   = true,             // DMA发送非阻塞模式
    .dma_tx_peri        = DMA0,             // 发送DMA外设：DMA0
    .dma_tx_channel     = DMA_CH6,          // 发送DMA通道：通道6
    .en_dma_tx_irq      = false,            // 不使能DMA发送中断
    .nvic_dma_tx_irq    = DMA0_Channel6_IRQn,// DMA发送中断向量号（备用）
    .irq_dma_tx_prio    = 1,                // DMA发送中断主优先级（备用）
    .irq_dma_tx_sub_prio = 1,               // DMA发送中断子优先级（备用）
    .dma_rx_peri        = DMA0,             // 接收DMA外设：DMA0
    .dma_rx_channel     = DMA_CH5,          // 接收DMA通道：通道5
};

Uart_HandleType Uart1_Fpga_Sofcore_Handle;  // UART1（FPGA通信）句柄

/**
 * @brief UART1（FPGA软核通信）板级初始化函数
 * @param  无
 * @retval 无
 * @note   初始化UART1句柄，绑定配置结构体，调用底层UART初始化接口
 */
void Uart1FpgaSofcoreBspInit()
{
    Uart1_Fpga_Sofcore_Handle.uartx = UART1_PORT;  // 绑定UART1端口
    Uart1_Fpga_Sofcore_Handle.uart_cfg = &_Uart1Cfg; // 绑定UART1配置结构体
    UartInit(&Uart1_Fpga_Sofcore_Handle);          // 调用底层UART初始化函数
}

/******************SPI FLASH配置与初始化*********************/
/**
 * @brief SPI FLASH的SPI接口配置结构体
 * @note 主模式，全双工通信，8位数据帧，软件NSS，时钟极性高/相位2边沿
 */
static Spi_Config _SpiFlashSpiCfg =
{
    .sck_gpiox              = GPIOA,                // SCK引脚端口：GPIOA
    .sck_gpio_pinx          = GPIO_PIN_5,           // SCK引脚：PA5
    .mosi_gpiox             = GPIOA,                // MOSI引脚端口：GPIOA
    .mosi_gpio_pinx         = GPIO_PIN_7,           // MOSI引脚：PA7
    .miso_gpiox             = GPIOA,                // MISO引脚端口：GPIOA
    .miso_gpio_pinx         = GPIO_PIN_6,           // MISO引脚：PA6
    .nss_gpiox              = GPIOA,                // NSS引脚端口：GPIOA
    .nss_gpio_pinx          = GPIO_PIN_4,           // NSS引脚：PA4
    .alt_rmp_mode           = 2,                    // 引脚复用模式：2
    .rcu_spi                = RCU_SPI0,             // SPI0时钟使能配置
    .spi_peri               = SPI0,                 // 所用SPI外设为SPI0
    .device_mode            = SPI_MASTER,           // 设备模式：主模式
    .trans_mode             = SPI_TRANSMODE_FULLDUPLEX, // 传输模式：全双工
    .frame_size             = SPI_FRAMESIZE_8BIT,   // 数据帧大小：8位
    .nss                    = SPI_NSS_SOFT,         // NSS模式：软件控制
    .endian                 = SPI_ENDIAN_MSB,       // 字节序：大端
    .clock_polarity_phase   = SPI_CK_PL_HIGH_PH_2EDGE, // 时钟极性/相位：高电平/第2边沿
    .prescale               = SPI_PSC_8,            // 时钟分频系数：8
    .en_irq                 = false,                // 不使能SPI中断
    .en_dma_tx              = false,                // 不使能DMA发送
    .en_dma_rx              = false,                // 不使能DMA接收
    .dma_tx_peri            = DMA0,                 // 发送DMA外设（备用）
    .dma_tx_channel         = DMA_CH2,              // 发送DMA通道（备用）
    .dma_rx_peri            = DMA0,                 // 接收DMA外设（备用）
    .dma_rx_channel         = DMA_CH1,              // 接收DMA通道（备用）
};

/**
 * @brief SPI FLASH的片选（CS）引脚配置结构体
 * @note 推挽输出模式，用于控制SPI FLASH的片选信号
 */
static Gpio_Config _SpiFlashCsCfg =
{
    .gpiox                  = GPIOA,            // 端口：GPIOA
    .gpio_pinx              = GPIO_PIN_4,       // 引脚：PA4
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Spi_HandleType  SpiFlashHandle;    // SPI FLASH的SPI接口句柄
Gpio_HandleType CsSpiFlashHandle;  // SPI FLASH的片选引脚句柄

/**
 * @brief SPI FLASH板级初始化函数
 * @param  无
 * @retval 无
 * @note   初始化SPI和片选引脚句柄，分别调用SPI和GPIO底层初始化接口
 */
void SpiFlashBspInit()
{
    SpiFlashHandle.spix = SPI0_PORT;              // 绑定SPI0端口
    SpiFlashHandle.default_width = 8;             // 默认数据宽度：8位
    SpiFlashHandle.spi_cfg = &_SpiFlashSpiCfg;    // 绑定SPI配置结构体
    CsSpiFlashHandle.gpio_cfg = &_SpiFlashCsCfg;  // 绑定片选引脚配置结构体
    
    SpiInit(&SpiFlashHandle);                     // 调用底层SPI初始化函数
    GpioInit(&CsSpiFlashHandle);                  // 调用底层GPIO初始化函数
}

/******************UART0（通用串口）配置与初始化*********************/
/**
 * @brief UART0配置结构体，通用串口通信接口
 * @note 波特率115200，8位数据位，1位停止位，无校验，使能接收中断
 */
static Uart_Config _Uart0Cfg =
{
    .tx_gpiox           = GPIOA,            // TX引脚端口：GPIOA
    .tx_gpio_pinx       = GPIO_PIN_9,       // TX引脚：PA9
    .rx_gpiox           = GPIOA,            // RX引脚端口：GPIOA
    .rx_gpio_pinx       = GPIO_PIN_10,      // RX引脚：PA10
    .rcu_uart           = RCU_USART0,       // USART0时钟使能配置
    .uart_peri          = USART0,           // 所用UART外设为USART0
    .nvic_irq           = USART0_IRQn,      // USART0中断向量号
    .baud_rate          = 115200,           // 波特率：115200
    .wlen               = USART_WL_8BIT,    // 数据位：8位
    .stblen             = USART_STB_1BIT,   // 停止位：1位
    .parity_cfg         = USART_PM_NONE,    // 校验位：无
    .en_irq             = true,             // 使能UART中断
    .irq_prio           = 1,                // 中断主优先级：1
    .irq_sub_prio       = 1,                // 中断子优先级：1
    .irq_type           = USART_INT_RBNE,   // 中断类型：接收缓冲区非空
    .en_dma_tx          = false,            // 不使能DMA发送
    .en_dma_rx          = false,            // 不使能DMA接收
    .dma_tx_peri        = DMA0,             // 发送DMA外设（备用）
    .dma_tx_channel     = DMA_CH3,          // 发送DMA通道（备用）
    .dma_rx_peri        = DMA0,             // 接收DMA外设（备用）
    .dma_rx_channel     = DMA_CH4,          // 接收DMA通道（备用）
};

Uart_HandleType Uart0_Handle;  // UART0句柄

/**
 * @brief UART0板级初始化函数
 * @param  无
 * @retval 无
 * @note   初始化UART0句柄，绑定配置结构体，调用底层UART初始化接口
 */
void Uart0BspInit()
{
    Uart0_Handle.uartx = UART0_PORT;  // 绑定UART0端口
    Uart0_Handle.uart_cfg = &_Uart0Cfg; // 绑定UART0配置结构体
    UartInit(&Uart0_Handle);          // 调用底层UART初始化函数
}

/******************外部看门狗输出引脚（ExtWdgOut）配置与初始化*********************/
/**
 * @brief 外部看门狗输出引脚配置结构体
 * @note 浮空输入模式，用于接收外部看门狗的输出信号
 */
static Gpio_Config _ExtWdgOutCfg =
{
    .gpiox                  = GPIOA,                // 端口：GPIOA
    .gpio_pinx              = GPIO_PIN_11,           // 引脚：PA11
    .gpio_mode              = GPIO_MODE_IN_FLOATING, // 模式：浮空输入
    .en_exti                = false,                // 不使能外部中断
};

Gpio_HandleType ExtWdgOutHandle;  // 外部看门狗输出引脚句柄

/**
 * @brief 外部看门狗输出引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void ExtWdgOut1BspInit(void)
{
    ExtWdgOutHandle.gpio_cfg   = &_ExtWdgOutCfg;  // 绑定配置结构体
    GpioInit(&ExtWdgOutHandle);                   // 调用底层GPIO初始化函数
}

/******************外部看门狗输入引脚（ExtWdgIn）配置与初始化*********************/
/**
 * @brief 外部看门狗输入引脚配置结构体
 * @note 推挽输出模式，用于向外部看门狗发送喂狗信号
 */
static Gpio_Config _ExtWdgInCfg =
{
    .gpiox                  = GPIOA,            // 端口：GPIOA
    .gpio_pinx              = GPIO_PIN_12,       // 引脚：PA12
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType ExtWdgInHandle;  // 外部看门狗输入引脚句柄

/**
 * @brief 外部看门狗输入引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void ExtWdgOut2BspInit(void)
{
    ExtWdgInHandle.gpio_cfg   = &_ExtWdgInCfg;  // 绑定配置结构体
    GpioInit(&ExtWdgInHandle);                  // 调用底层GPIO初始化函数
}

/******************VB-使能引脚（VBEN）配置与初始化*********************/
/**
 * @brief VB-使能引脚配置结构体
 * @note 推挽输出模式，用于控制VB-电源的使能
 */
static Gpio_Config _VBENCfg =
{
    .gpiox                  = GPIOB,            // 端口：GPIOB
    .gpio_pinx              = GPIO_PIN_0,       // 引脚：PB0
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType VBENHandle;  // VB-使能引脚句柄

/**
 * @brief VB-使能引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void VBENBspInit(void)
{
    VBENHandle.gpio_cfg   = &_VBENCfg;  // 绑定配置结构体
    GpioInit(&VBENHandle);              // 调用底层GPIO初始化函数
}

/******************IIC0（与BQ76952通信）配置与初始化*********************/
#if BQHWI2C  // 条件编译：使用硬件I2C
/**
 * @brief 硬件I2C0配置结构体，用于与BQ76952电池管理芯片通信
 * @note 通信速率300KHz（接近400KHz高速模式）
 */
static I2C_Config _I2C0Cfg =
{
    .scl_gpiox            = GPIOB,    // SCL引脚端口：GPIOB
    .scl_gpio_pinx        = GPIO_PIN_6, // SCL引脚：PB6
    .sda_gpiox            = GPIOB,    // SDA引脚端口：GPIOB
    .sda_gpio_pinx        = GPIO_PIN_7, // SDA引脚：PB7
    .rcu_i2c              = RCU_I2C0, // I2C0时钟使能配置
    .i2c_peri             = I2C0,     // 所用I2C外设为I2C0
    .speed                = 300000,   // 通信速率：300KHz（400KHz高速模式兼容）
};
#else  // 条件编译：使用软件模拟I2C
/**
 * @brief 软件模拟I2C0配置结构体，用于与BQ76952电池管理芯片通信
 */
static SimI2C_Config _I2C0Cfg =
{
    .scl_gpiox = GPIOB,    // SCL引脚端口：GPIOB
    .scl_gpio_pinx = GPIO_PIN_6, // SCL引脚：PB6
    .sda_gpiox = GPIOB,    // SDA引脚端口：GPIOB
    .sda_gpio_pinx = GPIO_PIN_7, // SDA引脚：PB7
};
#endif

#if BQHWI2C
I2C_HandleType gI2CBq76952HandleType;  // 硬件I2C句柄（BQ76952）
#else
SimI2C_HandleType gI2CBq76952HandleType; // 软件I2C句柄（BQ76952）
#endif

/**
 * @brief BQ76952写保护引脚（WP）配置结构体
 * @note 推挽输出模式，用于控制BQ76952的写保护功能
 */
static Gpio_Config _Bq76952WpCfg =
{
    .gpiox                  = GPIOE,            // 端口：GPIOE
    .gpio_pinx              = GPIO_PIN_2,       // 引脚：PE2
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType Bq76952WpHandle;  // BQ76952写保护引脚句柄

/**
 * @brief IIC0（BQ76952通信）板级初始化函数
 * @param  无
 * @retval 无
 * @note   根据编译选项初始化硬件/软件I2C，同时初始化BQ76952写保护引脚
 */
void I2CBq76952BspInit(void)
{
#if BQHWI2C
    gI2CBq76952HandleType.i2cx = I2C0_PORT;      // 绑定I2C0端口
    gI2CBq76952HandleType.i2c_cfg = &_I2C0Cfg;   // 绑定硬件I2C配置结构体
    I2CInit(&gI2CBq76952HandleType);             // 调用底层硬件I2C初始化函数
#else
    gI2CBq76952HandleType.simi2c_cfg = &_I2C0Cfg; // 绑定软件I2C配置结构体
    SimI2CInit(&gI2CBq76952HandleType);          // 调用底层软件I2C初始化函数
#endif
    // 初始化BQ76952写保护引脚（用于唤醒AFE芯片）
    Bq76952WpHandle.gpio_cfg = &_Bq76952WpCfg;  // 绑定写保护引脚配置结构体
    GpioInit(&Bq76952WpHandle);                  // 调用底层GPIO初始化函数
}

/******************IIC1（与EEPROM通信）配置与初始化*********************/
#if USE_SIM_IIC == 0  // 条件编译：使用硬件I2C
/**
 * @brief 硬件I2C1配置结构体，用于与EEPROM通信
 * @note 通信速率100KHz（标准模式）
 */
static I2C_Config _I2C1Cfg =
{
    .scl_gpiox            = GPIOB,    // SCL引脚端口：GPIOB
    .scl_gpio_pinx        = GPIO_PIN_10, // SCL引脚：PB10
    .sda_gpiox            = GPIOB,    // SDA引脚端口：GPIOB
    .sda_gpio_pinx        = GPIO_PIN_11, // SDA引脚：PB11
    .rcu_i2c              = RCU_I2C1, // I2C1时钟使能配置
    .i2c_peri             = I2C1,     // 所用I2C外设为I2C1
    .speed                = 100000,   // 通信速率：100KHz（标准模式）
};
I2C_HandleType I2CEepromHandle;  // 硬件I2C句柄（EEPROM）
#else  // 条件编译：使用软件模拟I2C
/**
 * @brief 软件模拟I2C配置结构体，用于与EEPROM通信
 */
static SimI2C_Config _E2PROMI2CCfg =
{
    .scl_gpiox          = GPIOB,    // SCL引脚端口：GPIOB
    .scl_gpio_pinx      = GPIO_PIN_10, // SCL引脚：PB10
    .sda_gpiox          = GPIOB,    // SDA引脚端口：GPIOB
    .sda_gpio_pinx      = GPIO_PIN_11, // SDA引脚：PB11
};
SimI2C_HandleType   SimI2CEepromHandle;  // 软件I2C句柄（EEPROM）
#endif

/**
 * @brief EEPROM写保护引脚（WP）配置结构体
 * @note 推挽输出模式，用于控制EEPROM的写保护功能
 */
static Gpio_Config _E2promWpCfg =
{
    .gpiox                  = GPIOA,            // 端口：GPIOA
    .gpio_pinx              = GPIO_PIN_8,       // 引脚：PA8
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType E2promWpHandle;  // EEPROM写保护引脚句柄

/**
 * @brief IIC1（EEPROM通信）板级初始化函数
 * @param  无
 * @retval 无
 * @note   根据编译选项初始化硬件/软件I2C，初始化EEPROM写保护引脚并置低（关闭写保护）
 */
void I2CEepromBspInit(void)
{
#if USE_SIM_IIC == 0
    I2CEepromHandle.i2cx = I2C1_PORT;          // 绑定I2C1端口
    I2CEepromHandle.i2c_cfg = &_I2C1Cfg;       // 绑定硬件I2C配置结构体
    I2CInit(&I2CEepromHandle);                 // 调用底层硬件I2C初始化函数
#else
    SimI2CEepromHandle.simi2c_cfg = &_E2PROMI2CCfg; // 绑定软件I2C配置结构体
    SimI2CEepromHandle.slave_addr0 = 0xA0;     // EEPROM从机地址：0xA0
    SimI2CInit(&SimI2CEepromHandle);           // 调用底层软件I2C初始化函数
#endif
    // 初始化EEPROM写保护引脚
    E2promWpHandle.gpio_cfg = &_E2promWpCfg;  // 绑定写保护引脚配置结构体
    GpioInit(&E2promWpHandle);                 // 调用底层GPIO初始化函数
    GpioBitReset((&E2promWpHandle));           // 写保护引脚置低（关闭写保护，允许写入）
}

/******************MCU指示灯（MCU LED）配置与初始化*********************/
/**
 * @brief MCU指示灯引脚配置结构体
 * @note 推挽输出模式，用于MCU运行状态指示
 */
static Gpio_Config _MCULEDCfg =
{
    .gpiox                  = GPIOD,            // 端口：GPIOD
    .gpio_pinx              = GPIO_PIN_8,       // 引脚：PD8
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType MCU_LED_Handle;  // MCU指示灯引脚句柄

/**
 * @brief MCU指示灯初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void MCULEDBspInit()
{
    MCU_LED_Handle.gpio_cfg = &_MCULEDCfg;  // 绑定配置结构体
    GpioInit(&MCU_LED_Handle);              // 调用底层GPIO初始化函数
}

/******************温度传感器S2（TempS2）配置与初始化*********************/
/**
 * @brief 温度传感器S2控制引脚配置结构体
 * @note 推挽输出模式，用于控制温度传感器S2的使能/选通
 */
static Gpio_Config _TempS2Cfg =
{
    .gpiox                  = GPIOD,            // 端口：GPIOD
    .gpio_pinx              = GPIO_PIN_13,      // 引脚：PD13
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType TempS2Handle;  // 温度传感器S2控制引脚句柄

/**
 * @brief 温度传感器S2初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void TempS2BspInit(void)
{
    TempS2Handle.gpio_cfg    = &_TempS2Cfg;  // 绑定配置结构体
    GpioInit(&TempS2Handle);                 // 调用底层GPIO初始化函数
}

/******************温度传感器S1（TempS1）配置与初始化*********************/
/**
 * @brief 温度传感器S1控制引脚配置结构体
 * @note 推挽输出模式，用于控制温度传感器S1的使能/选通
 */
static Gpio_Config _TempS1Cfg =
{
    .gpiox                  = GPIOD,            // 端口：GPIOD
    .gpio_pinx              = GPIO_PIN_14,      // 引脚：PD14
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType TempS1Handle;  // 温度传感器S1控制引脚句柄

/**
 * @brief 温度传感器S1初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void TempS1BspInit(void)
{
    TempS1Handle.gpio_cfg    = &_TempS1Cfg;  // 绑定配置结构体
    GpioInit(&TempS1Handle);                 // 调用底层GPIO初始化函数
}

/******************温度传感器S0（TempS0）配置与初始化*********************/
/**
 * @brief 温度传感器S0控制引脚配置结构体
 * @note 推挽输出模式，用于控制温度传感器S0的使能/选通
 */
static Gpio_Config _TempS0Cfg =
{
    .gpiox                  = GPIOD,            // 端口：GPIOD
    .gpio_pinx              = GPIO_PIN_15,      // 引脚：PD15
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType TempS0Handle;  // 温度传感器S0控制引脚句柄

/**
 * @brief 温度传感器S0初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void TempS0BspInit(void)
{
    TempS0Handle.gpio_cfg    = &_TempS0Cfg;  // 绑定配置结构体
    GpioInit(&TempS0Handle);                 // 调用底层GPIO初始化函数
}

/******************电源按键检测（Power_Key_Check）配置与初始化*********************/
/**
 * @brief 电源按键检测引脚配置结构体
 * @note 浮空输入模式，用于检测电源按键的按下状态
 */
static Gpio_Config _KeyCheckCfg =
{
    .gpiox                  = GPIOE,                // 端口：GPIOE
    .gpio_pinx              = GPIO_PIN_0,           // 引脚：PE0
    .gpio_mode              = GPIO_MODE_IN_FLOATING, // 模式：浮空输入
    .en_exti                = false,                // 不使能外部中断
};

Gpio_HandleType KeyCheckHandle;  // 电源按键检测引脚句柄

/**
 * @brief 电源按键检测引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void KeyCheckBspInit(void)
{
    KeyCheckHandle.gpio_cfg    = &_KeyCheckCfg;  // 绑定配置结构体
    GpioInit(&KeyCheckHandle);                   // 调用底层GPIO初始化函数
}

/******************AFE温度传感器TS2输出（AfeTs2）配置与初始化*********************/
/**
 * @brief AFE温度传感器TS2输出控制引脚配置结构体
 * @note 推挽输出模式，用于控制AFE芯片的TS2温度传感器输出
 */
static Gpio_Config _AfeTs2Cfg =
{
    .gpiox                  = GPIOE,            // 端口：GPIOE
    .gpio_pinx              = GPIO_PIN_3,       // 引脚：PE3
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType AfeTs2Handle;  // AFE温度传感器TS2输出控制引脚句柄

/**
 * @brief AFE温度传感器TS2输出控制引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void AfeTsBspInit(void)
{
    AfeTs2Handle.gpio_cfg    = &_AfeTs2Cfg;  // 绑定配置结构体
    GpioInit(&AfeTs2Handle);                 // 调用底层GPIO初始化函数
}

/******************电源保持（Power Hold）配置与初始化*********************/
/**
 * @brief 电源保持引脚配置结构体
 * @note 推挽输出模式，用于维持系统电源稳定（防止意外掉电）
 */
static Gpio_Config _PowerHoldCfg =
{
    .gpiox                  = GPIOE,            // 端口：GPIOE
    .gpio_pinx              = GPIO_PIN_6,       // 引脚：PE6
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType PowerHoldHandle;  // 电源保持引脚句柄

/**
 * @brief 电源保持引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void PowerHoldBspInit(void)
{
    PowerHoldHandle.gpio_cfg    = &_PowerHoldCfg;  // 绑定配置结构体
    GpioInit(&PowerHoldHandle);                    // 调用底层GPIO初始化函数
}

/******************按键指示灯使能（Key LED EN）配置与初始化*********************/
/**
 * @brief 按键指示灯使能引脚配置结构体
 * @note 推挽输出模式，用于控制按键指示灯的使能
 */
static Gpio_Config _KeyLedENCfg =
{
    .gpiox                  = GPIOE,            // 端口：GPIOE
    .gpio_pinx              = GPIO_PIN_7,       // 引脚：PE7
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType KEY_LED_EN_Handle;  // 按键指示灯使能引脚句柄

/**
 * @brief 按键指示灯使能引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void KeyLEDBspInit()
{
    KEY_LED_EN_Handle.gpio_cfg = &_KeyLedENCfg;  // 绑定配置结构体
    GpioInit(&KEY_LED_EN_Handle);                // 调用底层GPIO初始化函数
}

/******************加热检测（HeatCk）配置与初始化*********************/
/**
 * @brief 加热检测引脚配置结构体
 * @note 浮空输入模式，用于检测加热模块的工作状态
 */
static Gpio_Config _HeatCkCfg =
{
    .gpiox                  = GPIOE,                // 端口：GPIOE
    .gpio_pinx              = GPIO_PIN_8,           // 引脚：PE8
    .gpio_mode              = GPIO_MODE_IN_FLOATING, // 模式：浮空输入
    .en_exti                = false,                // 不使能外部中断
};

Gpio_HandleType HeatCkHandle;  // 加热检测引脚句柄

/**
 * @brief 加热检测引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void HeatCKBspInit(void)
{
    HeatCkHandle.gpio_cfg    = &_HeatCkCfg;  // 绑定配置结构体
    GpioInit(&HeatCkHandle);                 // 调用底层GPIO初始化函数
}

/******************加热使能（HeatEn）配置与初始化*********************/
/**
 * @brief 加热使能引脚配置结构体
 * @note 推挽输出模式，用于控制加热模块的开启/关闭
 */
static Gpio_Config _HeatEnCfg =
{
    .gpiox                  = GPIOE,            // 端口：GPIOE
    .gpio_pinx              = GPIO_PIN_9,       // 引脚：PE9
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType HeatEnHandle;  // 加热使能引脚句柄

/**
 * @brief 加热使能引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void HeatEnBspInit(void)
{
    HeatEnHandle.gpio_cfg    = &_HeatEnCfg;  // 绑定配置结构体
    GpioInit(&HeatEnHandle);                 // 调用底层GPIO初始化函数
}

/******************ID ADC使能（IdAdcEn）配置与初始化*********************/
/**
 * @brief ID ADC使能引脚配置结构体
 * @note 推挽输出模式，用于使能ID检测的ADC通道
 */
static Gpio_Config _IDADCEnCfg =
{
    .gpiox                  = GPIOE,            // 端口：GPIOE
    .gpio_pinx              = GPIO_PIN_11,      // 引脚：PE11
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType IDADCEnHandle;  // ID ADC使能引脚句柄

/**
 * @brief ID ADC使能引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void IDAdcEnBspInit(void)
{
    IDADCEnHandle.gpio_cfg   = &_IDADCEnCfg;  // 绑定配置结构体
    GpioInit(&IDADCEnHandle);                 // 调用底层GPIO初始化函数
}

/******************错误检测（ErrorCk）配置与初始化*********************/
/**
 * @brief 错误检测引脚配置结构体
 * @note 浮空输入模式，用于检测系统故障/错误状态
 */
static Gpio_Config _ErrorCkCfg =
{
    .gpiox                  = GPIOE,                // 端口：GPIOE
    .gpio_pinx              = GPIO_PIN_12,          // 引脚：PE12
    .gpio_mode              = GPIO_MODE_IN_FLOATING, // 模式：浮空输入
    .en_exti                = false,                // 不使能外部中断
};

Gpio_HandleType ErrorCkHandle;  // 错误检测引脚句柄

/**
 * @brief 错误检测引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void ErrorCkBspInit(void)
{
    ErrorCkHandle.gpio_cfg   = &_ErrorCkCfg;  // 绑定配置结构体
    GpioInit(&ErrorCkHandle);                 // 调用底层GPIO初始化函数
}

/******************PMOS使能（PmosEn）配置与初始化*********************/
/**
 * @brief PMOS管使能引脚配置结构体
 * @note 推挽输出模式，用于控制PMOS功率管的导通/关断
 */
static Gpio_Config _PmosEnCfg =
{
    .gpiox                  = GPIOE,            // 端口：GPIOE
    .gpio_pinx              = GPIO_PIN_13,      // 引脚：PE13
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType PmosEnHandle;  // PMOS使能引脚句柄

/**
 * @brief PMOS使能引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void PmosEnBspInit(void)
{
    PmosEnHandle.gpio_cfg   = &_PmosEnCfg;  // 绑定配置结构体
    GpioInit(&PmosEnHandle);                 // 调用底层GPIO初始化函数
}

/******************CMOS使能（CmosEn）配置与初始化*********************/
/**
 * @brief CMOS管使能引脚配置结构体
 * @note 推挽输出模式，用于控制CMOS功率管的导通/关断
 */
static Gpio_Config _CmosEnCfg =
{
    .gpiox                  = GPIOE,            // 端口：GPIOE
    .gpio_pinx              = GPIO_PIN_14,      // 引脚：PE14
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType CmosEnHandle;  // CMOS使能引脚句柄

/**
 * @brief CMOS使能引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void CmosEnBspInit(void)
{
    CmosEnHandle.gpio_cfg   = &_CmosEnCfg;  // 绑定配置结构体
    GpioInit(&CmosEnHandle);                 // 调用底层GPIO初始化函数
}

/******************DMOS使能（DmosEn）配置与初始化*********************/
/**
 * @brief DMOS管使能引脚配置结构体
 * @note 推挽输出模式，用于控制DMOS功率管的导通/关断
 */
static Gpio_Config _DmosEnCfg =
{
    .gpiox                  = GPIOE,            // 端口：GPIOE
    .gpio_pinx              = GPIO_PIN_15,      // 引脚：PE15
    .gpio_mode              = GPIO_MODE_OUT_PP, // 模式：推挽输出
    .en_exti                = false,            // 不使能外部中断
};

Gpio_HandleType DmosEnHandle;  // DMOS使能引脚句柄

/**
 * @brief DMOS使能引脚初始化函数
 * @param  无
 * @retval 无
 * @note   绑定配置结构体，调用底层GPIO初始化接口
 */
void DmosEnBspInit(void)
{
    DmosEnHandle.gpio_cfg   = &_DmosEnCfg;  // 绑定配置结构体
    GpioInit(&DmosEnHandle);                 // 调用底层GPIO初始化函数
}

/******************定时器1（AFE采样定时）配置与初始化*********************/
/**
 * @brief 定时器1配置结构体，用于AFE芯片的采样定时触发
 * @note 定时器时钟60MHz，分频后1MHz，周期5ms（定时中断间隔5ms）
 */
static Timer_Config _AFESampleTime =
{
    .rcu_timer                              = RCU_TIMER1,       // TIMER1时钟使能配置
    .timer_peri                             = TIMER1,           // 所用定时器外设：TIMER1
    .prescaler                              = 119,              // 预分频系数：119
    .alignedmode                            = TIMER_COUNTER_EDGE, // 计数对齐模式：边沿对齐
    .counterdirection                       = TIMER_COUNTER_UP, // 计数方向：向上计数
    .clockdivision                          = TIMER_CKDIV_DIV1, // 时钟分频：不分频
    .period                                 = 4999,             // 自动重载值：4999（5ms中断一次）
    .repetitioncounter                      = 0,                // 重复计数器：0
    .quadrature_decoder_mode_enable         = false,            // 不使能正交解码器模式
    .en_irq                                 = true,             // 使能定时器中断
    .nvic_irq                               = TIMER1_IRQn,      // TIMER1中断向量号
    .irq_prio                               = 2,                // 中断主优先级：2
    .irq_sub_prio                           = 2,                // 中断子优先级：2
    .irq_type                               = TIMER_INT_UP,     // 中断类型：更新中断（溢出中断）
};

Timer_HandleType AFESampleTimeHandle;  // AFE采样定时器句柄

/**
 * @brief AFE采样定时器初始化函数
 * @param  无
 * @retval 无
 * @note   初始化定时器句柄，绑定配置结构体，调用底层定时器初始化接口
 */
void AFETimerInit(void)
{
    AFESampleTimeHandle.timerx = TIMER1_PERI;        // 绑定TIMER1外设
    AFESampleTimeHandle.timer_cfg = &_AFESampleTime; // 绑定定时器配置结构体
    TimerInit(&AFESampleTimeHandle);                 // 调用底层定时器初始化函数
}

/**
 * @brief 基础外设时钟初始化函数
 * @param  无
 * @retval 无
 * @note   初始化DMA、GPIO、EXTI、ADC的时钟，需在其他外设初始化前调用
 */
void BspBaseInit()
{
    DMAClkInit();   // DMA控制器时钟初始化
    GpioClkInit();  // GPIO外设时钟初始化
    ExtiClkInit();  // EXTI外部中断时钟初始化
    AdcClkInit();   // ADC外设时钟初始化
}

/**
 * @brief 板级支持包（BSP）总初始化函数
 * @param  无
 * @retval 无
 * @note   统一初始化所有板级外设，按依赖顺序调用各模块初始化函数
 *         需在系统启动后、应用程序运行前调用
 */
void BspInit()
{
    ExtWdgOut2BspInit();  // 外部看门狗输入引脚初始化
    VBENBspInit();        // VB-使能引脚初始化
    TempS2BspInit();      // 温度传感器S2初始化
    TempS1BspInit();      // 温度传感器S1初始化
    TempS0BspInit();      // 温度传感器S0初始化
    PowerHoldBspInit();   // 电源保持引脚初始化
    HeatCKBspInit();      // 加热检测引脚初始化
    HeatEnBspInit();      // 加热使能引脚初始化
    IDAdcEnBspInit();     // ID ADC使能引脚初始化
    ErrorCkBspInit();     // 错误检测引脚初始化
    PmosEnBspInit();      // PMOS使能引脚初始化
    CmosEnBspInit();      // CMOS使能引脚初始化
    DmosEnBspInit();      // DMOS使能引脚初始化
    ADCBspInit();         // ADC模块初始化
}