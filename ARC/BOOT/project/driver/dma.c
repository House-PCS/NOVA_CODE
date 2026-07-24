/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dma.c
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



#include "dma.h"


void DMAClkInit(void)
{
#if USE_DMA1
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA1, ENABLE);
#endif
#if USE_DMA2
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA2, ENABLE);
#endif   
}






















