/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_mbi502x_led.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-3-18
  * @brief       
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-3-18     <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "bsp.h"
#include "systick.h"




void DevMBI502xLedInit(void);
void DevMBI502xProcess(void);

void DevLightningLedCtl(uint8_t level);
void DevPercentLedCtl(uint8_t level);
void DevDightLedCtl(uint8_t value);
void DevFaultLedCtl(uint8_t value);









