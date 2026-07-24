/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        spi.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-07-20
  * @brief       N32G4FR
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-07-20    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "spi.h"


static SPI_InitType _SPI_InitStructure[SPI_NUM];
static DMA_InitType  _DMA_SPI_Tx_InitStructure[SPI_NUM];
static DMA_InitType  _DMA_SPI_Rx_InitStructure[SPI_NUM];


/**
  * @brief  SPI GPIO init.
  * @param  Uart_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
static void _SpiGpioInit(Spi_HandleType *pHandle)
{
    GPIO_InitType  GPIO_InitStructure;
    
    if(pHandle->spi_cfg->alt_rmp_mode == 3)
    {
        GPIO_ConfigPinRemap(pHandle->spi_cfg->alt_rmp_function, ENABLE);     
    }
    GPIO_InitStruct(&GPIO_InitStructure);
    
    
    /*SCK*/
    GPIO_InitStructure.Pin        = pHandle->spi_cfg->sck_gpio_pinx;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitPeripheral(pHandle->spi_cfg->sck_gpiox, &GPIO_InitStructure);
    /*MOSI*/
    GPIO_InitStructure.Pin        = pHandle->spi_cfg->mosi_gpio_pinx;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitPeripheral(pHandle->spi_cfg->mosi_gpiox, &GPIO_InitStructure);
    /*MISO*/
    GPIO_InitStructure.Pin        = pHandle->spi_cfg->miso_gpio_pinx;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitPeripheral(pHandle->spi_cfg->miso_gpiox, &GPIO_InitStructure);
    
    if(pHandle->spi_cfg->nss == SPI_NSS_HARD)
    {
        GPIO_InitStructure.Pin        = pHandle->spi_cfg->nss_gpio_pinx;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_InitPeripheral(pHandle->spi_cfg->nss_gpiox, &GPIO_InitStructure);
    }    
}





static void _SpiDmaTxCfg(Spi_HandleType *pHandle)
{
    DMA_DeInit(pHandle->spi_cfg->dma_tx_peri_channel);
    SPI_I2S_EnableDma(pHandle->spi_cfg->spi_peri, SPI_I2S_DMA_TX, ENABLE);
    
    _DMA_SPI_Tx_InitStructure[pHandle->spix].BufSize                  = pHandle->tx_size;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].MemAddr                  = (uint32_t)pHandle->pTxBuf;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].DMA_MemoryInc            = DMA_MEM_INC_ENABLE;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].PeriphAddr               = (uint32_t)&(pHandle->spi_cfg->spi_peri->DAT);
    _DMA_SPI_Tx_InitStructure[pHandle->spix].PeriphInc                = DMA_PERIPH_INC_DISABLE;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].Priority                 = DMA_PRIORITY_HIGH;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].Direction                = DMA_DIR_PERIPH_DST;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].CircularMode             = DMA_MODE_NORMAL;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].Mem2Mem                  = DMA_M2M_DISABLE;
    
    if(_SPI_InitStructure[pHandle->spix].DataLen == SPI_DATA_SIZE_16BITS)
    {
        _DMA_SPI_Tx_InitStructure[pHandle->spix].MemDataSize            = DMA_MemoryDataSize_HalfWord;
        _DMA_SPI_Tx_InitStructure[pHandle->spix].PeriphDataSize         = DMA_PERIPH_DATA_SIZE_HALFWORD;
    }
    else
    {
        _DMA_SPI_Tx_InitStructure[pHandle->spix].MemDataSize            = DMA_MemoryDataSize_Byte;
        _DMA_SPI_Tx_InitStructure[pHandle->spix].PeriphDataSize         = DMA_PERIPH_DATA_SIZE_BYTE;
    }
    DMA_Init(pHandle->spi_cfg->dma_tx_peri_channel, &_DMA_SPI_Tx_InitStructure[pHandle->spix]);      
    DMA_EnableChannel(pHandle->spi_cfg->dma_tx_peri_channel, DISABLE);
}


static void _SpiDmaRxCfg(Spi_HandleType *pHandle)
{
    DMA_DeInit(pHandle->spi_cfg->dma_rx_peri_channel);
    SPI_I2S_EnableDma(pHandle->spi_cfg->spi_peri, SPI_I2S_DMA_RX, ENABLE);
    
    _DMA_SPI_Rx_InitStructure[pHandle->spix].BufSize                  = pHandle->rx_size;/*如果是空闲中断，通常配置一个比较大的值，确保实际所需接收数据小于该值*/
    _DMA_SPI_Rx_InitStructure[pHandle->spix].MemAddr                  = (uint32_t)pHandle->pRxBuf;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].DMA_MemoryInc            = DMA_MEM_INC_ENABLE;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].PeriphAddr               = (uint32_t)&(pHandle->spi_cfg->spi_peri->DAT);
    _DMA_SPI_Rx_InitStructure[pHandle->spix].PeriphInc                = DMA_PERIPH_INC_DISABLE;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].Priority                 = DMA_PRIORITY_HIGH;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].Direction                = DMA_DIR_PERIPH_SRC;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].CircularMode             = DMA_MODE_NORMAL;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].Mem2Mem                  = DMA_M2M_DISABLE;
    
    if(_SPI_InitStructure[pHandle->spix].DataLen == SPI_DATA_SIZE_16BITS)
    {
        _DMA_SPI_Rx_InitStructure[pHandle->spix].MemDataSize            = DMA_MemoryDataSize_HalfWord;
        _DMA_SPI_Rx_InitStructure[pHandle->spix].PeriphDataSize         = DMA_PERIPH_DATA_SIZE_HALFWORD;
    }
    else
    {
        _DMA_SPI_Rx_InitStructure[pHandle->spix].MemDataSize            = DMA_MemoryDataSize_Byte;
        _DMA_SPI_Rx_InitStructure[pHandle->spix].PeriphDataSize         = DMA_PERIPH_DATA_SIZE_BYTE;
    }
    DMA_Init(pHandle->spi_cfg->dma_rx_peri_channel, &_DMA_SPI_Rx_InitStructure[pHandle->spix]);      
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, DISABLE);
    
}




void SpiInit(Spi_HandleType *pHandle)
{   
    if(pHandle->spi_cfg->spi_peri == SPI1)
    {
        RCC_EnableAPB2PeriphClk(pHandle->spi_cfg->rcc_spi, ENABLE);
    }
    else
    {
        RCC_EnableAPB1PeriphClk(pHandle->spi_cfg->rcc_spi, ENABLE);
    }
    
    _SpiGpioInit(pHandle);
    
    if(pHandle->default_width == 16)
    {
        pHandle->spi_cfg->data_len = SPI_DATA_SIZE_16BITS;
    }
    else
    {
        pHandle->spi_cfg->data_len = SPI_DATA_SIZE_8BITS;
    }
    _SPI_InitStructure[pHandle->spix].DataDirection             = pHandle->spi_cfg->data_direction;
    _SPI_InitStructure[pHandle->spix].SpiMode                   = pHandle->spi_cfg->spi_mode;
    _SPI_InitStructure[pHandle->spix].DataLen                   = pHandle->spi_cfg->data_len;
    _SPI_InitStructure[pHandle->spix].CLKPOL                    = pHandle->spi_cfg->clock_polarity;
    _SPI_InitStructure[pHandle->spix].CLKPHA                    = pHandle->spi_cfg->clock_phase;
    _SPI_InitStructure[pHandle->spix].NSS                       = pHandle->spi_cfg->nss;
    _SPI_InitStructure[pHandle->spix].BaudRatePres              = pHandle->spi_cfg->prescale;
    _SPI_InitStructure[pHandle->spix].FirstBit                  = pHandle->spi_cfg->first_bit;
    _SPI_InitStructure[pHandle->spix].CRCPoly                   = 7;
    
    SPI_Init(pHandle->spi_cfg->spi_peri, &_SPI_InitStructure[pHandle->spix]);
   
    
    SPI_Enable(pHandle->spi_cfg->spi_peri, ENABLE);
    
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
        bit = SPI_DATA_SIZE_16BITS;
    }
    else
    {
        bit = SPI_DATA_SIZE_8BITS;
    }
    SPI_Enable(pHandle->spi_cfg->spi_peri, DISABLE);
    _SPI_InitStructure[pHandle->spix].DataLen = bit;
    SPI_Init(pHandle->spi_cfg->spi_peri, &_SPI_InitStructure[pHandle->spix]);
    SPI_Enable(pHandle->spi_cfg->spi_peri, ENABLE);
}



//FlagStatus  SPI_TxStreamIsReady(Spi_HandleType *pHandle)
//{
//    if((DMA_CHCTL(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel) &  DMA_CHXCTL_CHEN) == SET)
//    {
//        DMA_CHCTL(pHandle->spi_cfg->dma_tx_peri, pHandle->spi_cfg->dma_tx_channel) &= ~(uint32_t)DMA_CHXCTL_CHEN;
//        return RESET;
//    }
//    return SET;
//}


//FlagStatus  SPI_RxStreamIsReady(Spi_HandleType *pHandle)
//{
//    if((DMA_CHCTL(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel) &  DMA_CHXCTL_CHEN) == SET)
//    {
//        DMA_CHCTL(pHandle->spi_cfg->dma_rx_peri, pHandle->spi_cfg->dma_rx_channel) &= ~(uint32_t)DMA_CHXCTL_CHEN;
//        return RESET;
//    }
//    return SET;
//}



static uint8_t _SpiWriteReadByte(SPI_Module* spix, uint8_t *pData)
{
    uint16_t time_out = 0xfff;
    uint16_t temp_data = 0;
    uint16_t data_w = (uint16_t)(*pData); 
    while(SPI_I2S_GetStatus(spix, SPI_I2S_TE_FLAG) == RESET)
    {
        time_out--;
        if(time_out == 0)
        {
            return  0;
        }
    }
    SPI_I2S_TransmitData(spix, data_w);
    time_out = 0xfff;
    while(SPI_I2S_GetStatus(spix, SPI_I2S_RNE_FLAG) == RESET)
    {
        time_out--;
        if(time_out == 0)
        {
            return  0;
        }
    }
    temp_data = (SPI_I2S_ReceiveData(spix));
    
    return  temp_data;
}


static uint16_t _SpiWriteReadHalfWord(SPI_Module* spix, uint16_t *pData)
{
    uint16_t time_out = 0xfff;
    uint16_t temp_data = 0;

    while(SPI_I2S_GetStatus(spix, SPI_I2S_TE_FLAG) == RESET)
    {
        time_out--;
        if(time_out == 0)
        {
            return  0;
        }
    }
    SPI_I2S_TransmitData(spix, *pData);
    time_out = 0xfff;
    while(SPI_I2S_GetStatus(spix, SPI_I2S_RNE_FLAG) == RESET)
    {
        time_out--;
        if(time_out == 0)
        {
            return  0;
        }
    }
    temp_data = (SPI_I2S_ReceiveData(spix));
    
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
    
    _DMA_SPI_Tx_InitStructure[pHandle->spix].BufSize                  = pHandle->tx_size;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].MemAddr                  = (uint32_t)pHandle->pTxBuf;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].MemDataSize              = DMA_MemoryDataSize_Byte;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].PeriphDataSize           = DMA_PERIPH_DATA_SIZE_BYTE;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].DMA_MemoryInc            = DMA_MEM_INC_ENABLE;
    
    _DMA_SPI_Rx_InitStructure[pHandle->spix].BufSize                  = pHandle->tx_size;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].MemAddr                  = (uint32_t)&_DummyByte;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].MemDataSize              = DMA_MemoryDataSize_Byte;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].PeriphDataSize           = DMA_PERIPH_DATA_SIZE_BYTE;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].DMA_MemoryInc            = DMA_MEM_INC_DISABLE;
    
    
    DMA_Init(pHandle->spi_cfg->dma_tx_peri_channel, &_DMA_SPI_Tx_InitStructure[pHandle->spix]);  
    DMA_Init(pHandle->spi_cfg->dma_rx_peri_channel, &_DMA_SPI_Rx_InitStructure[pHandle->spix]);  
    
    if(SPI_I2S_GetStatus(pHandle->spi_cfg->spi_peri, SPI_I2S_RNE_FLAG) == RESET)
    {
        _DummyByte = pHandle->spi_cfg->spi_peri->DAT;
    }
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, ENABLE);
    DMA_EnableChannel(pHandle->spi_cfg->dma_tx_peri_channel, ENABLE);
    
    while (DMA_GetFlagStatus((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_rx_channel * 4)), pHandle->spi_cfg->dma_rx_peri) == RESET)
    {
        time_out--;
    }
    time_out = 0xfffffff;
    while (DMA_GetFlagStatus((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_tx_channel * 4)), pHandle->spi_cfg->dma_tx_peri) == RESET)
    {
        time_out--;
    }
    DMA_ClearFlag((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_rx_channel * 4)), pHandle->spi_cfg->dma_rx_peri);
    DMA_ClearFlag((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_tx_channel * 4)), pHandle->spi_cfg->dma_tx_peri);
    
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, DISABLE);
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, DISABLE);
    
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
    _DMA_SPI_Tx_InitStructure[pHandle->spix].BufSize                  = pHandle->tx_size;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].MemAddr                  = (uint32_t)pHandle->pTxBuf;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].MemDataSize              = DMA_MemoryDataSize_HalfWord;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].PeriphDataSize           = DMA_PERIPH_DATA_SIZE_HALFWORD;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].DMA_MemoryInc            = DMA_MEM_INC_ENABLE;
    
    _DMA_SPI_Rx_InitStructure[pHandle->spix].BufSize                  = pHandle->tx_size;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].MemAddr                  = (uint32_t)&_DummyHalfWord;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].MemDataSize              = DMA_MemoryDataSize_HalfWord;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].PeriphDataSize           = DMA_PERIPH_DATA_SIZE_HALFWORD;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].DMA_MemoryInc            = DMA_MEM_INC_DISABLE;
    
    DMA_Init(pHandle->spi_cfg->dma_tx_peri_channel, &_DMA_SPI_Tx_InitStructure[pHandle->spix]);  
    DMA_Init(pHandle->spi_cfg->dma_rx_peri_channel, &_DMA_SPI_Rx_InitStructure[pHandle->spix]);  
    
    if(SPI_I2S_GetStatus(pHandle->spi_cfg->spi_peri, SPI_I2S_RNE_FLAG) == RESET)
    {
        _DummyHalfWord = pHandle->spi_cfg->spi_peri->DAT;
    }
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, ENABLE);
    DMA_EnableChannel(pHandle->spi_cfg->dma_tx_peri_channel, ENABLE);
    
    while (DMA_GetFlagStatus((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_rx_channel * 4)), pHandle->spi_cfg->dma_rx_peri) == RESET)
    {
        time_out--;
    }
    time_out = 0xfffffff;
    while (DMA_GetFlagStatus((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_tx_channel * 4)), pHandle->spi_cfg->dma_tx_peri) == RESET)
    {
        time_out--;
    }
    DMA_ClearFlag((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_rx_channel * 4)), pHandle->spi_cfg->dma_rx_peri);
    DMA_ClearFlag((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_tx_channel * 4)), pHandle->spi_cfg->dma_tx_peri);
    
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, DISABLE);
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, DISABLE);
    
    
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
    _DMA_SPI_Tx_InitStructure[pHandle->spix].BufSize                  = pHandle->rx_size;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].MemAddr                  = (uint32_t )&_DummyByte;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].MemDataSize              = DMA_MemoryDataSize_Byte;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].PeriphDataSize           = DMA_PERIPH_DATA_SIZE_BYTE;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].DMA_MemoryInc            = DMA_MEM_INC_DISABLE;
    
    _DMA_SPI_Rx_InitStructure[pHandle->spix].BufSize                  = pHandle->rx_size;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].MemAddr                  = (uint32_t)pHandle->pRxBuf;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].MemDataSize              = DMA_MemoryDataSize_Byte;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].PeriphDataSize           = DMA_PERIPH_DATA_SIZE_BYTE;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].DMA_MemoryInc            = DMA_MEM_INC_ENABLE;
    
    DMA_Init(pHandle->spi_cfg->dma_rx_peri_channel, &_DMA_SPI_Rx_InitStructure[pHandle->spix]);  
    DMA_Init(pHandle->spi_cfg->dma_tx_peri_channel, &_DMA_SPI_Tx_InitStructure[pHandle->spix]);
    
    if(SPI_I2S_GetStatus(pHandle->spi_cfg->spi_peri, SPI_I2S_RNE_FLAG) == RESET)
    {
        _DummyByte = pHandle->spi_cfg->spi_peri->DAT;
    }
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, ENABLE);
    DMA_EnableChannel(pHandle->spi_cfg->dma_tx_peri_channel, ENABLE);
    
    while (DMA_GetFlagStatus((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_rx_channel * 4)), pHandle->spi_cfg->dma_rx_peri) == RESET)
    {
        time_out--;
    }
    time_out = 0xfffffff;
    while (DMA_GetFlagStatus((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_tx_channel * 4)), pHandle->spi_cfg->dma_tx_peri) == RESET)
    {
        time_out--;
    }
    DMA_ClearFlag((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_rx_channel * 4)), pHandle->spi_cfg->dma_rx_peri);
    DMA_ClearFlag((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_tx_channel * 4)), pHandle->spi_cfg->dma_tx_peri);
    
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, DISABLE);
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, DISABLE);
    
    
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
    _DMA_SPI_Tx_InitStructure[pHandle->spix].BufSize                  = pHandle->rx_size;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].MemAddr                  = (uint32_t )&_DummyHalfWord;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].MemDataSize              = DMA_MemoryDataSize_HalfWord;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].PeriphDataSize           = DMA_PERIPH_DATA_SIZE_HALFWORD;
    _DMA_SPI_Tx_InitStructure[pHandle->spix].DMA_MemoryInc            = DMA_MEM_INC_DISABLE;
    
    _DMA_SPI_Rx_InitStructure[pHandle->spix].BufSize                  = pHandle->rx_size;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].MemAddr                  = (uint32_t)pHandle->pRxBuf;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].MemDataSize              = DMA_MemoryDataSize_HalfWord;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].PeriphDataSize           = DMA_PERIPH_DATA_SIZE_HALFWORD;
    _DMA_SPI_Rx_InitStructure[pHandle->spix].DMA_MemoryInc            = DMA_MEM_INC_ENABLE;
    
    DMA_Init(pHandle->spi_cfg->dma_rx_peri_channel, &_DMA_SPI_Rx_InitStructure[pHandle->spix]);  
    DMA_Init(pHandle->spi_cfg->dma_tx_peri_channel, &_DMA_SPI_Tx_InitStructure[pHandle->spix]);
    
    if(SPI_I2S_GetStatus(pHandle->spi_cfg->spi_peri, SPI_I2S_RNE_FLAG) == RESET)
    {
        _DummyHalfWord = pHandle->spi_cfg->spi_peri->DAT;
    }
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, ENABLE);
    DMA_EnableChannel(pHandle->spi_cfg->dma_tx_peri_channel, ENABLE);
    
    while (DMA_GetFlagStatus((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_rx_channel * 4)), pHandle->spi_cfg->dma_rx_peri) == RESET)
    {
        time_out--;
    }
    time_out = 0xfffffff;
    while (DMA_GetFlagStatus((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_tx_channel * 4)), pHandle->spi_cfg->dma_tx_peri) == RESET)
    {
        time_out--;
    }
    DMA_ClearFlag((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_rx_channel * 4)), pHandle->spi_cfg->dma_rx_peri);
    DMA_ClearFlag((DMA1_FLAG_TC1 << (pHandle->spi_cfg->dma_tx_channel * 4)), pHandle->spi_cfg->dma_tx_peri);
    
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, DISABLE);
    DMA_EnableChannel(pHandle->spi_cfg->dma_rx_peri_channel, DISABLE);
    
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













