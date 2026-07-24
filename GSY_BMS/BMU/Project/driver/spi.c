/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        spi.c
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


#include "spi.h"

static spi_parameter_struct _SPI_InitStructure[SPI_NUM];
static dma_parameter_struct  _DMA_SPI_Tx_InitStructure[SPI_NUM];
static dma_parameter_struct  _DMA_SPI_Rx_InitStructure[SPI_NUM];




/**
  * @brief  UART GPIO init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
static void _SpiGpioInit(Spi_HandleType *pHandle)
{
    if(pHandle->spi_cfg->alt_rmp_mode == 3)
    {
        gpio_pin_remap_config(pHandle->spi_cfg->alt_rmp_function, ENABLE);     
    }
    
    gpio_init(pHandle->spi_cfg->sck_gpiox, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, pHandle->spi_cfg->sck_gpio_pinx);
    gpio_init(pHandle->spi_cfg->mosi_gpiox, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, pHandle->spi_cfg->mosi_gpio_pinx);
    gpio_init(pHandle->spi_cfg->miso_gpiox, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, pHandle->spi_cfg->miso_gpio_pinx);
    if(pHandle->spi_cfg->nss == SPI_NSS_HARD)
    {
        gpio_init(pHandle->spi_cfg->nss_gpiox, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, pHandle->spi_cfg->nss_gpio_pinx);
    }    
}





static void _SpiDmaTxCfg(Spi_HandleType *pHandle)
{
    dma_deinit(pHandle->spi_cfg->dma_tx_peri,pHandle->spi_cfg->dma_tx_channel);
    spi_dma_enable(pHandle->spi_cfg->spi_peri, SPI_DMA_TRANSMIT);
    
    _DMA_SPI_Tx_InitStructure[pHandle->spix].periph_addr                 = (uint32_t)&SPI_DATA(pHandle->spi_cfg->spi_peri);
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_addr                 = (uint32_t)pHandle->pTxBuf;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].direction                   = DMA_MEMORY_TO_PERIPHERAL;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].number                      = pHandle->tx_size;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].periph_inc                  = DMA_PERIPH_INCREASE_DISABLE;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_inc                  = DMA_MEMORY_INCREASE_ENABLE;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].priority                    = DMA_PRIORITY_ULTRA_HIGH;
    
    if(_SPI_InitStructure[pHandle->spix].frame_size == SPI_FRAMESIZE_16BIT)
    {
        _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_width            = DMA_MEMORY_WIDTH_16BIT;
        _DMA_SPI_Tx_InitStructure[pHandle->spix].periph_width            = DMA_PERIPHERAL_WIDTH_16BIT;
    }
    else
    {
        _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_width            = DMA_MEMORY_WIDTH_8BIT;
        _DMA_SPI_Tx_InitStructure[pHandle->spix].periph_width            = DMA_PERIPHERAL_WIDTH_8BIT;
    }
    dma_init(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, &_DMA_SPI_Tx_InitStructure[pHandle->spix]);
    dma_circulation_disable(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);    
    dma_memory_to_memory_disable(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);       
    dma_channel_disable(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);
}

static void _SpiDmaRxCfg(Spi_HandleType *pHandle)
{
    dma_deinit(pHandle->spi_cfg->dma_rx_peri,pHandle->spi_cfg->dma_rx_channel);
    spi_dma_enable(pHandle->spi_cfg->spi_peri, SPI_DMA_RECEIVE);
    
    _DMA_SPI_Rx_InitStructure[pHandle->spix].periph_addr                 = (uint32_t)&SPI_DATA(pHandle->spi_cfg->spi_peri);
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_addr                 = (uint32_t)pHandle->pRxBuf;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].direction                   = DMA_PERIPHERAL_TO_MEMORY;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].number                      = pHandle->rx_size;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].periph_inc                  = DMA_PERIPH_INCREASE_DISABLE;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_inc                  = DMA_MEMORY_INCREASE_ENABLE;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].priority                    = DMA_PRIORITY_HIGH;
    
    dma_init(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, &_DMA_SPI_Rx_InitStructure[pHandle->spix]);
    dma_circulation_disable(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    dma_memory_to_memory_disable(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    dma_channel_disable(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
}



void SpiInit(Spi_HandleType *pHandle)
{   
    rcu_periph_clock_enable(pHandle->spi_cfg->rcu_spi);
    _SpiGpioInit(pHandle);
    
    if(pHandle->default_width == 16)
    {
        pHandle->spi_cfg->frame_size = SPI_FRAMESIZE_16BIT;
    }
    else
    {
        pHandle->spi_cfg->frame_size = SPI_FRAMESIZE_8BIT;
    }
    _SPI_InitStructure[pHandle->spix].trans_mode             = pHandle->spi_cfg->trans_mode;
    _SPI_InitStructure[pHandle->spix].device_mode            = pHandle->spi_cfg->device_mode;
    _SPI_InitStructure[pHandle->spix].frame_size             = pHandle->spi_cfg->frame_size;
    _SPI_InitStructure[pHandle->spix].clock_polarity_phase   = pHandle->spi_cfg->clock_polarity_phase;
    _SPI_InitStructure[pHandle->spix].nss                    = pHandle->spi_cfg->nss;
    _SPI_InitStructure[pHandle->spix].prescale               = pHandle->spi_cfg->prescale;
    _SPI_InitStructure[pHandle->spix].endian                 = pHandle->spi_cfg->endian;
    
    spi_init(pHandle->spi_cfg->spi_peri, &_SPI_InitStructure[pHandle->spix]);
    
    /* set crc polynomial */
    spi_crc_polynomial_set(pHandle->spi_cfg->spi_peri, 7);
    
    spi_enable(pHandle->spi_cfg->spi_peri);
    
    if(pHandle->spi_cfg->en_dma_rx)
    {
        _SpiDmaRxCfg(pHandle);
    }
    if(pHandle->spi_cfg->en_dma_tx)
    {
        _SpiDmaTxCfg(pHandle);
    }
}



static void _SpiWidthCfg(Spi_HandleType *pHandle, uint8_t width)
{
    uint32_t bit;
    if(width == 16)
    {
        bit = SPI_FRAMESIZE_16BIT;
    }
    else
    {
        bit = SPI_FRAMESIZE_8BIT;
    }
    spi_disable(pHandle->spi_cfg->spi_peri);
    _SPI_InitStructure[pHandle->spix].frame_size = bit;
    spi_init(pHandle->spi_cfg->spi_peri, &_SPI_InitStructure[pHandle->spix]);
    spi_enable(pHandle->spi_cfg->spi_peri);	
}



FlagStatus  SPI_TxStreamIsReady(Spi_HandleType *pHandle)
{
    if((DMA_CHCTL(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel) &  DMA_CHXCTL_CHEN) == SET)
    {
        DMA_CHCTL(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel) &= ~(uint32_t)DMA_CHXCTL_CHEN;
        return RESET;
    }
    return SET;
}


FlagStatus  SPI_RxStreamIsReady(Spi_HandleType *pHandle)
{
    if((DMA_CHCTL(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel) &  DMA_CHXCTL_CHEN) == SET)
    {
        DMA_CHCTL(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel) &= ~(uint32_t)DMA_CHXCTL_CHEN;
        return RESET;
    }
    return SET;
}


static uint8_t _SpiWriteReadByte(uint32_t spix, uint8_t *pData)
{
    uint16_t time_out = 0xfff;
    uint16_t temp_data = 0;
    uint16_t data_w = (uint16_t)(*pData); 
    while(spi_i2s_flag_get(spix, SPI_FLAG_TBE) == RESET)
    {
        time_out--;
        if(time_out == 0)
        {
            return  0;
        }
    }
    spi_i2s_data_transmit(spix, data_w);
//    while(spi_i2s_flag_get(spix, SPI_FLAG_TRANS) == SET);
    time_out = 0xfff;
    while(spi_i2s_flag_get(spix, SPI_FLAG_RBNE) == RESET)
    {
        time_out--;
        if(time_out == 0)
        {
            return  0;
        }
    }
    temp_data = (spi_i2s_data_receive(spix));
    
    return  temp_data;
}


static uint16_t _SpiWriteReadHalfWord(uint32_t spix, uint16_t *pData)
{
    uint16_t time_out = 0xfff;
    uint16_t temp_data = 0;

    while(spi_i2s_flag_get(spix, SPI_FLAG_TBE) == RESET)
    {
        time_out--;
        if(time_out == 0)
        {
            return  0;
        }
    }
    spi_i2s_data_transmit(spix, *pData);
    while(spi_i2s_flag_get(spix, SPI_FLAG_TRANS) == SET);
    time_out = 0xfff;
    while(spi_i2s_flag_get(spix, SPI_FLAG_RBNE) == RESET)
    {
        time_out--;
        if(time_out == 0)
        {
            return  0;
        }
    }
    temp_data = (spi_i2s_data_receive(spix));
    
    return  temp_data;
}


static uint8_t  _DummyByte      = 0;
static uint16_t _DummyHalfWord  = 0;  

/***************非DMA操作*****************/

/**
  * @brief  spi send out without dma, 位宽强制为8.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiNormalWrite8(Spi_HandleType *pHandle)
{
    uint32_t i = 0;
    if(pHandle->default_width == 16)
    {
        _SpiWidthCfg(pHandle, 8);
    }
    for(i = 0; i < pHandle->tx_size; i++)
    {
        _SpiWriteReadByte(pHandle->spi_cfg->spi_peri, &((uint8_t *)pHandle->pTxBuf)[i]);
    }
    
    
    if(pHandle->default_width == 16)
    {
        _SpiWidthCfg(pHandle, 16);
    }
}

/**
  * @brief  spi send out without dma, 位宽强制为16.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiNormalWrite16(Spi_HandleType *pHandle)
{
    uint32_t i = 0;
    if(pHandle->default_width == 8)
    {
        _SpiWidthCfg(pHandle, 16);
    }
    for(i = 0; i < pHandle->tx_size; i++)
    {
        _SpiWriteReadHalfWord(pHandle->spi_cfg->spi_peri, &((uint16_t *)pHandle->pTxBuf)[i]);
    }
    
    
    if(pHandle->default_width == 8)
    {
        _SpiWidthCfg(pHandle, 8);
    }
}

/**
  * @brief  spi send out without dma, 位宽由用户默认配置的位宽确定.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiNormalWrite(Spi_HandleType *pHandle)
{
    if(pHandle->default_width == 16)
    {
        SpiNormalWrite16(pHandle);
    }
    else
    {
        SpiNormalWrite8(pHandle);
    }
}




/**
  * @brief  spi receive in without dma, 位宽由用户默认配置的位宽确定.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiNormalRead8(Spi_HandleType *pHandle)
{
    uint32_t i = 0;
    if(pHandle->default_width == 16)
    {
        _SpiWidthCfg(pHandle, 8);
    }
    for(i = 0; i < pHandle->rx_size; i++)
    {
        ((uint8_t *)pHandle->pRxBuf)[i] = _SpiWriteReadByte(pHandle->spi_cfg->spi_peri, &_DummyByte);
    }
    
    
    if(pHandle->default_width == 16)
    {
        _SpiWidthCfg(pHandle, 16);
    }
}

/**
  * @brief  spi receive in without dma, 位宽由用户默认配置的位宽确定.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiNormalRead16(Spi_HandleType *pHandle)
{
    uint32_t i = 0;
    if(pHandle->default_width == 8)
    {
        _SpiWidthCfg(pHandle, 16);
    }
    for(i = 0; i < pHandle->rx_size; i++)
    {
        ((uint16_t *)pHandle->pRxBuf)[i] = _SpiWriteReadHalfWord(pHandle->spi_cfg->spi_peri, &_DummyHalfWord);
    }
    
    
    if(pHandle->default_width == 8)
    {
        _SpiWidthCfg(pHandle, 8);
    }
}

/**
  * @brief  spi receive in without dma, 位宽由用户默认配置的位宽确定.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiNormalRead(Spi_HandleType *pHandle)
{
    if(pHandle->default_width == 16)
    {
        SpiNormalRead16(pHandle);
    }
    else
    {
        SpiNormalRead8(pHandle);
    }
}



/***************DMA操作*****************/

/**
  * @brief  spi send out with dma, 位宽强制为8.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiDMAWrite8(Spi_HandleType *pHandle)
{
    uint32_t time_out = 0xfffffff;
    
    if(pHandle->spi_cfg->en_dma_tx != true || pHandle->spi_cfg->en_dma_rx != true)
    {
        return;
    }
    if(pHandle->default_width == 16)
    {
        _SpiWidthCfg(pHandle, 8);
    }
    
//    dma_deinit(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);
//    dma_deinit(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_addr        = (uint32_t )pHandle->pTxBuf;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_width        = DMA_MEMORY_WIDTH_16BIT;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].periph_width        = DMA_PERIPHERAL_WIDTH_16BIT;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].number              = pHandle->tx_size;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_addr        = (uint32_t)&_DummyByte;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_width        = DMA_MEMORY_WIDTH_16BIT;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].periph_width        = DMA_PERIPHERAL_WIDTH_16BIT;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].number              = pHandle->tx_size;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_inc          = DMA_MEMORY_INCREASE_DISABLE;
    
    dma_init(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, &_DMA_SPI_Rx_InitStructure[pHandle->spix]);
    dma_init(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, &_DMA_SPI_Tx_InitStructure[pHandle->spix]);
    
    if(spi_i2s_flag_get(pHandle->spi_cfg->spi_peri, SPI_FLAG_RBNE) == RESET)
    {
        _DummyByte = SPI_DATA(pHandle->spi_cfg->spi_peri);
    }
    dma_channel_enable(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    dma_channel_enable(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);
    
    while (dma_flag_get(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, DMA_FLAG_FTF) == RESET)
    {
        time_out--;
    }
    time_out = 0xfffffff;
    while (dma_flag_get(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, DMA_FLAG_FTF)==RESET)
    {
        time_out--;
    }
    dma_flag_clear(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, DMA_FLAG_FTF);
    dma_flag_clear(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, DMA_FLAG_FTF);
    
    dma_channel_disable(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    dma_channel_disable(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);
    
    
    if(pHandle->default_width == 16)
    {
        _SpiWidthCfg(pHandle, 16);
    }
}

/**
  * @brief  spi send out with dma, 位宽强制为16.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiDMAWrite16(Spi_HandleType *pHandle)
{
    uint32_t time_out = 0xfffffff;
    
    if(pHandle->spi_cfg->en_dma_tx != true || pHandle->spi_cfg->en_dma_rx != true)
    {
        return;
    }
    if(pHandle->default_width == 8)
    {
        _SpiWidthCfg(pHandle, 16);
    }
    
//    dma_deinit(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);
//    dma_deinit(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_addr         = (uint32_t )pHandle->pTxBuf;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_width        = DMA_MEMORY_WIDTH_16BIT;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].periph_width        = DMA_PERIPHERAL_WIDTH_16BIT;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].number              = pHandle->tx_size;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_addr         = (uint32_t)&_DummyHalfWord;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_width        = DMA_MEMORY_WIDTH_16BIT;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].periph_width        = DMA_PERIPHERAL_WIDTH_16BIT;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].number              = pHandle->tx_size;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_inc          = DMA_MEMORY_INCREASE_DISABLE;
    
    dma_init(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, &_DMA_SPI_Rx_InitStructure[pHandle->spix]);
    dma_init(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, &_DMA_SPI_Tx_InitStructure[pHandle->spix]);
    
    if(spi_i2s_flag_get(pHandle->spi_cfg->spi_peri, SPI_FLAG_RBNE) == RESET)
    {
        _DummyByte = SPI_DATA(pHandle->spi_cfg->spi_peri);
    }
    dma_channel_enable(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    dma_channel_enable(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);
    
    while (dma_flag_get(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, DMA_FLAG_FTF) == RESET)
    {
        time_out--;
    }
    time_out = 0xfffffff;
    while (dma_flag_get(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, DMA_FLAG_FTF)==RESET)
    {
        time_out--;
    }
    dma_flag_clear(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, DMA_FLAG_FTF);
    dma_flag_clear(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, DMA_FLAG_FTF);
    
    dma_channel_disable(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    dma_channel_disable(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);
    
    
    if(pHandle->default_width == 8)
    {
        _SpiWidthCfg(pHandle, 8);
    }
}


/**
  * @brief  spi send out with dma, 位宽由用户默认配置的位宽确定.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiDMAWrite(Spi_HandleType *pHandle)
{
    if(pHandle->default_width == 16)
    {
        SpiDMAWrite16(pHandle);
    }
    else
    {
        SpiDMAWrite8(pHandle);
    }
}





/**
  * @brief  spi receive in with dma, 位宽由用户默认配置的位宽确定.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiDMARead8(Spi_HandleType *pHandle)
{
    uint32_t time_out = 0xfffffff;
    if(pHandle->spi_cfg->en_dma_tx != true || pHandle->spi_cfg->en_dma_rx != true)
    {
        return;
    }
    if(pHandle->default_width == 16)
    {
        _SpiWidthCfg(pHandle, 8);
    }
    
//    dma_deinit(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);
//    dma_deinit(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_addr        = (uint32_t )&_DummyByte;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_width        = DMA_MEMORY_WIDTH_16BIT;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].periph_width        = DMA_PERIPHERAL_WIDTH_16BIT;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].number              = pHandle->rx_size;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_inc          = DMA_MEMORY_INCREASE_DISABLE;
    
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_addr        = (uint32_t)pHandle->pRxBuf;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_width        = DMA_MEMORY_WIDTH_16BIT;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].periph_width        = DMA_PERIPHERAL_WIDTH_16BIT;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].number              = pHandle->rx_size;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    
    dma_init(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, &_DMA_SPI_Rx_InitStructure[pHandle->spix]);
    dma_init(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, &_DMA_SPI_Tx_InitStructure[pHandle->spix]);
    
    if(spi_i2s_flag_get(pHandle->spi_cfg->spi_peri, SPI_FLAG_RBNE) == RESET)
    {
        _DummyByte = SPI_DATA(pHandle->spi_cfg->spi_peri);
    }
    dma_channel_enable(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    dma_channel_enable(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);
    
    while (dma_flag_get(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, DMA_FLAG_FTF) == RESET)
    {
        time_out--;
    }
    time_out = 0xfffffff;
    while (dma_flag_get(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, DMA_FLAG_FTF)==RESET)
    {
        time_out--;
    }
    dma_flag_clear(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, DMA_FLAG_FTF);
    dma_flag_clear(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, DMA_FLAG_FTF);
    
    dma_channel_disable(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    dma_channel_disable(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);
    
    
    if(pHandle->default_width == 16)
    {
        _SpiWidthCfg(pHandle, 16);
    }
}

/**
  * @brief  spi receive in with dma, 位宽由用户默认配置的位宽确定.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiDMARead16(Spi_HandleType *pHandle)
{
    uint32_t time_out = 0xfffffff;
    if(pHandle->spi_cfg->en_dma_tx != true || pHandle->spi_cfg->en_dma_rx != true)
    {
        return;
    }
    if(pHandle->default_width == 8)
    {
        _SpiWidthCfg(pHandle, 16);
    }
    
//    dma_deinit(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);
//    dma_deinit(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_addr        = (uint32_t )&_DummyHalfWord;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_width        = DMA_MEMORY_WIDTH_16BIT;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].periph_width        = DMA_PERIPHERAL_WIDTH_16BIT;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].number              = pHandle->rx_size;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].memory_inc          = DMA_MEMORY_INCREASE_DISABLE;
    
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_addr        = (uint32_t)pHandle->pRxBuf;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_width        = DMA_MEMORY_WIDTH_16BIT;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].periph_width        = DMA_PERIPHERAL_WIDTH_16BIT;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].number              = pHandle->rx_size;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    
    dma_init(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, &_DMA_SPI_Rx_InitStructure[pHandle->spix]);
    dma_init(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, &_DMA_SPI_Tx_InitStructure[pHandle->spix]);
    
    if(spi_i2s_flag_get(pHandle->spi_cfg->spi_peri, SPI_FLAG_RBNE) == RESET)
    {
        _DummyByte = SPI_DATA(pHandle->spi_cfg->spi_peri);
    }
    dma_channel_enable(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    dma_channel_enable(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);
    
    while (dma_flag_get(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, DMA_FLAG_FTF) == RESET)
    {
        time_out--;
    }
    time_out = 0xfffffff;
    while (dma_flag_get(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, DMA_FLAG_FTF)==RESET)
    {
        time_out--;
    }
    dma_flag_clear(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel, DMA_FLAG_FTF);
    dma_flag_clear(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel, DMA_FLAG_FTF);
    
    dma_channel_disable(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel);
    dma_channel_disable(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel);  
    
    if(pHandle->default_width == 8)
    {
        _SpiWidthCfg(pHandle, 8);
    }
}


/**
  * @brief  spi receive in with dma, 位宽由用户默认配置的位宽确定.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiDMARead(Spi_HandleType *pHandle)
{
    if(pHandle->default_width == 16)
    {
        SpiDMARead16(pHandle);
    }
    else
    {
        SpiDMARead8(pHandle);
    }
}


/***************根据用户配置自动选择DMA或非DMA操作*****************/


/**
  * @brief  spi send out, 位宽强制为8.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiWrite8(Spi_HandleType *pHandle)
{
    if(pHandle->spi_cfg->en_dma_tx == true)
    {
        SpiDMAWrite8(pHandle);
    }
    else
    {
        SpiNormalWrite8(pHandle);
    }
}

/**
  * @brief  spi send out, 位宽强制为16.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiWrite16(Spi_HandleType *pHandle)
{
    if(pHandle->spi_cfg->en_dma_tx == true)
    {
        SpiDMAWrite16(pHandle);
    }
    else
    {
        SpiNormalWrite16(pHandle);
    }
}

/**
  * @brief  spi send out, 位宽由用户默认配置的位宽确定.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiWrite(Spi_HandleType *pHandle)
{
    if(pHandle->spi_cfg->en_dma_tx == true)
    {
        SpiDMAWrite(pHandle);
    }
    else
    {
        SpiNormalWrite(pHandle);
    }
}




/**
  * @brief  spi receive in, 位宽由用户默认配置的位宽确定.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiRead8(Spi_HandleType *pHandle)
{
    if(pHandle->spi_cfg->en_dma_rx == true)
    {
        SpiDMARead8(pHandle);
    }
    else
    {
        SpiNormalRead8(pHandle);
    }
}

/**
  * @brief  spi receive in, 位宽由用户默认配置的位宽确定.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiRead16(Spi_HandleType *pHandle)
{
    if(pHandle->spi_cfg->en_dma_rx == true)
    {
        SpiDMARead16(pHandle);
    }
    else
    {
        SpiNormalRead16(pHandle);
    }
}


/**
  * @brief  spi receive in, 位宽由用户默认配置的位宽确定.
  * @param  Spi_HandleType: SPI需要用户配置的参数
  * @return NULL
  * @note   none
  */
void SpiRead(Spi_HandleType *pHandle)
{
    if(pHandle->spi_cfg->en_dma_rx == true)
    {
        SpiDMARead(pHandle);
    }
    else
    {
        SpiNormalRead(pHandle);
    }
}









