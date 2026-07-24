/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.h
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
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * <tr><td>2023-06-15    <td>V1.0.1    <td>           <td>增加同一串口可使用多种中断并存的机制
  * </table>
  ******************************************************************************
  */
#ifndef __FPGA_COM_H_
#define __FPGA_COM_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "CMSDK_CM0.h"
#include "core_cm0.h"

#pragma pack(1)
typedef struct
{
    __IO    uint16_t device_type;
    __IO    uint8_t  reserved;
    __IO    uint8_t  op_cmd;
    __IO    uint32_t addr;
    __IO    uint16_t len;
} CACHE_UDPDATA_TypeDef;
#pragma pack()

#define CACHE_UDPDATA_RX        ((CACHE_UDPDATA_TypeDef*) 0x30000000)
#define CACHE_UDPDATA_TX        (0x30001000)
#define EthIrqClr               (*(volatile int *)0x40000018)
#define LEN                     (*(volatile int *)0x40000010)
#define READY                   (*(volatile int *)0x40000014)
#define WEWE                    (0x30020100)    
extern uint8_t comm_fail_flag;
void FpgaComInit(void);
void Commovertime(void);
#endif


