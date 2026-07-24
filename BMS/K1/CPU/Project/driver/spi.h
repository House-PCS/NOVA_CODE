/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        spi.h
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

#ifndef __SPI_H_
#define __SPI_H_


#include "gd32f30x_spi.h"
#include "stdbool.h"
#include "drv_opt.h"



typedef enum
{
#if USE_SPI0
    SPI0_PORT,
#endif
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
    uint32_t                sck_gpiox;
    uint32_t                sck_gpio_pinx;
    
    uint32_t                mosi_gpiox;
    uint32_t                mosi_gpio_pinx;
    
    uint32_t                miso_gpiox;
    uint32_t                miso_gpio_pinx;
    
    uint32_t                nss_gpiox;
    uint32_t                nss_gpio_pinx;

    uint8_t                 alt_rmp_mode;/*1:Alternate1, 2:Alternate2, 3:Remap*/
    uint32_t                alt_rmp_function;/*alt_rmp_mode=1时表示afio_function， alt_rmp_mode=3时表示remap function, alt_rmp_mode为其他值，该字段无意义*/
    
    rcu_periph_enum         rcu_spi;
    uint32_t                spi_peri;
    
    uint32_t                device_mode;
    uint32_t                trans_mode;
    uint32_t                frame_size;
    uint32_t                nss; /*从机模式只支持硬nss*/
    uint32_t                endian;
    uint32_t                clock_polarity_phase;
    uint32_t                prescale;  
    
    bool                    en_irq;
    uint32_t                nvic_irq;
    uint8_t                 irq_prio;
    uint8_t                 irq_sub_prio;
    usart_interrupt_enum    irq_type;
    
    /*DMA相关参数*/
    bool                    en_dma_tx;
    bool                    en_dma_rx;
    
    uint32_t                dma_tx_peri;
    dma_channel_enum        dma_tx_channel;
    
    uint32_t                dma_rx_peri;
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







