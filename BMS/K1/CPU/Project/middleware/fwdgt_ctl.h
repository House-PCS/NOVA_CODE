/**
 ******************************************************************************
 * @file    shell_netif.h
 * @author  NovaStar R&D Team
 * @version V1.0.0
 * @date    2017.09.28
 * @brief
 ******************************************************************************
 * @attention
 *
 * History<br>
 * version     date        auther      description<br>
 * V1.0.0      2017.11.13  Huyuxin     Initial Version<br>
 *
 ******************************************************************************
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _FWDGT_CONTROL_H
#define _FWDGT_CONTROL_H



/***** 看门狗存储地址及标志 *****/
#define FWDGT_CNT_HEADER        (0X5A)
#define FWDGT_SPI_FLASH_ADDR    (0x00130000)

/* Inculdes ------------------------------------------------------------------*/

#include "bsp.h"

/* Exported typedef -----------------------------------------------------------*/
#pragma pack(1)
typedef struct
{
    uint8_t                 header;             //数据头
    uint8_t                 fwdgt_cnt;          //看门狗触发次数计数
    uint8_t                 sumcheck;           //和校验
} FwdgtInfo_t;
#pragma pack()


/* 清除看门狗错误计数 */
void ClearFwdgtErrCnt(void);
#endif /*__SENSOR_CONTROL_H */

