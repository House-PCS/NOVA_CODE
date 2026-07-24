/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dma.h
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


#ifndef __DMA_H_
#define __DMA_H_

#include "stdbool.h"
#include "stdio.h"
#include "drv_opt.h"

typedef enum 
{
    DMA_CH1 = 0,                /*!< DMA Channel1 */ 
    DMA_CH2,                    /*!< DMA Channel2 */ 
    DMA_CH3,                    /*!< DMA Channel3 */ 
    DMA_CH4,                    /*!< DMA Channel4 */ 
    DMA_CH5,                    /*!< DMA Channel5 */ 
    DMA_CH6,                    /*!< DMA Channel6 */
    DMA_CH7,                    /*!< DMA Channel6 */
    DMA_CH8                     /*!< DMA Channel6 */
} dma_channel_enum;


void DMAClkInit(void);
DMA_ChannelType* GetDMAPeriChannel(DMA_Module* peri, dma_channel_enum channel);

#endif















