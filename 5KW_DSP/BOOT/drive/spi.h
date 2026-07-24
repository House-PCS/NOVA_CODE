/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        spi.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __SPI_H__
#define __SPI_H__

#include "F28x_Project.h"
#ifdef __cplusplus
extern "C" {
#endif
void SpiFifoInit(void);
void SpiWriteData(uint16_t len, uint16_t* data);
void SpiReadData(uint16_t len, uint16_t* data);

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================
