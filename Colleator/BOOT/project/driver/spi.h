
#ifndef __SPI_H_
#define __SPI_H_


#include "stdbool.h"
#include "drv_opt.h"
#include "dma.h"

typedef enum
{
#if USE_SPI1
    SPI1_PORT,
#endif
#if USE_SPI2
    SPI2_PORT,
#endif
#if USE_SPI3  
    SPI3_PORT,
#endif
#if USE_SPI4
    SPI4_PORT,
#endif
#if USE_SPI5
    SPI5_PORT,
#endif
    SPI_NUM
}EN_SPIx_PORT;


typedef struct _spi_config_stru_t
{
    GPIO_Module*            sck_gpiox;
    uint32_t                sck_gpio_pinx;
    
    GPIO_Module*            mosi_gpiox;
    uint32_t                mosi_gpio_pinx;
    
    GPIO_Module*            miso_gpiox;
    uint32_t                miso_gpio_pinx;
    
    GPIO_Module*            nss_gpiox;
    uint32_t                nss_gpio_pinx;

    uint8_t                 alt_rmp_mode;/*1:Alternate1, 2:Alternate2, 3:Remap*/
    uint32_t                alt_rmp_function;/*alt_rmp_mode=1时表示afio_function， alt_rmp_mode=3时表示remap function, alt_rmp_mode为其他值，该字段无意义*/
    
    uint32_t                rcc_spi;
    SPI_Module*             spi_peri;
    
    
    uint16_t                spi_mode;
    uint16_t                data_direction;
    uint16_t                data_len;
    uint16_t                clock_polarity;
    uint32_t                clock_phase;
    uint16_t                nss; /*从机模式只支持硬nss*/
    uint32_t                prescale;  
    uint16_t                first_bit;
    

    bool                    en_irq;
    uint32_t                nvic_irq;
    uint8_t                 irq_prio;
    uint8_t                 irq_sub_prio;
    uint32_t                irq_type;
    
    /*DMA相关参数*/
    bool                    en_dma_tx;
    bool                    en_dma_rx;
    
    DMA_ChannelType*        dma_tx_peri_channel;//bsp无需配置，驱动自行计算
    DMA_Module*             dma_tx_peri;
    dma_channel_enum        dma_tx_channel;
    
    DMA_ChannelType*        dma_rx_peri_channel;//bsp无需配置，驱动自行计算
    DMA_Module*             dma_rx_peri;
    dma_channel_enum        dma_rx_channel;
    
    /*后续增加dma中断相关参数*/
    
}Spi_Config;



typedef struct _spi_handle_stru_t
{
    EN_SPIx_PORT            spix;
    Spi_Config              *spi_cfg;
    
    
    void                    *pTxBuf;/*可定义成void, 兼容8位宽和16位宽传输*/
    uint32_t                tx_size;/*数据数，不是字节数*/
    void                    *pRxBuf;/*可定义成void, 兼容8位宽和16位宽传输*/
    uint32_t                rx_size;/*数据数，不是字节数*/
    uint8_t                 default_width;/*8或16*/
    void                    (*irq_rx_process)();
}Spi_HandleType;



void SpiInit(Spi_HandleType *pHandle);

void SpiNormalWrite(Spi_HandleType *pHandle);
void SpiNormalWrite8(Spi_HandleType *pHandle);
void SpiNormalWrite16(Spi_HandleType *pHandle);
void SpiNormalRead(Spi_HandleType *pHandle);
void SpiNormalRead8(Spi_HandleType *pHandle);
void SpiNormalRead16(Spi_HandleType *pHandle);

void SpiDMAWrite(Spi_HandleType *pHandle);
void SpiDMAWrite8(Spi_HandleType *pHandle);
void SpiDMAWrite16(Spi_HandleType *pHandle);
void SpiDMARead(Spi_HandleType *pHandle);
void SpiDMARead8(Spi_HandleType *pHandle);
void SpiDMARead16(Spi_HandleType *pHandle);


void SpiWrite(Spi_HandleType *pHandle);
void SpiWrite8(Spi_HandleType *pHandle);
void SpiWrite16(Spi_HandleType *pHandle);
void SpiRead(Spi_HandleType *pHandle);
void SpiRead8(Spi_HandleType *pHandle);
void SpiRead16(Spi_HandleType *pHandle);


FlagStatus  SPI_TxStreamIsReady(Spi_HandleType *pHandle);
FlagStatus  SPI_RxStreamIsReady(Spi_HandleType *pHandle);


#endif










