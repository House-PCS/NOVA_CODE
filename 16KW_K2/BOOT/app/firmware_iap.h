/**
  ****************** (C) COPYRIGHT 2017 Nova  *******************
  * @file     firmware_iap.c
  * @auther   Miles Wang
  * @version  V1.0
  * @date     9/14/2017
  * @brief    This file provides the functions of firmware IAP.
  ***************************************************************
  * @attention
  *
  * History<br>
  * version   date            auther         description<br>
  * V1.0.0    2017.9.14       Miles Wang     Initial Version<br>
  ***************************************************************
  */
#ifndef _FIRMWARE_IAP_H_
#define _FIRMWARE_IAP_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include "flash.h"
#include "dev_flash.h"

#define APP_JUMP    asm("   LB  0x98000")
/* Exported macros --------------------------------------------------------*/
#define  IAP_DEBUG_MODE                     0           /* 是否处于DEBUG模式 */

#define  FILE_HEADER_SIZE                   0x100       /* 程序文件头大小 */
#define  FIRMWARE_SIZE_MAX                  (0x20000 - 0x100)

#define  ONCHIP_APP_BASE_ADDR               0x90000  /* 用户程序的真正入口地址，应从该基地址向后偏移0x100 */
#define USER_FLASH_START_ADDRESS            0x98000  /* Only as example see comment */
/* 后续要重新分配，考虑预留全部种类子卡的文件空间，避免频繁变动 */
#define   FLASH_BLOCK_SIZE                512             /* 单位：字节 */

/* Exported types ---------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
typedef struct
{
    bool        doc_upgrade_ing;    /* 更新中*/
    uint8_t     doc_upgrade_en;     /* 使能文件更新完成后,立即加载标志 */
    uint8_t     doc_upgrade_ok;  /* 文件更新完成后标志*/
    uint32_t    doc_upgrade_stamp;  /* 更新的时间戳记录，约定在5s内，如果没有下一包数据到达子卡，退出更新状态 */
} _upgrade_flag_info_t;


void  enterbootmodejudge(void);
void  sysfirmwareiap(_upgrade_flag_info_t  * upgrade_flag);
void FirmwareCopy(void);
void flash_if_test(void);
void  jumptoappprocess(void);



#ifdef __cplusplus
}
#endif  /* __cplusplus */


#endif  /* _FIRMWARE_IAP_H_ */

