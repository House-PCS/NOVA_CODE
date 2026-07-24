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
#include "dev_spiflash.h"
#include "mid_shell.h"
#include "dev_communication.h"
//#include "firmware_upgrade.h"
//#include "flash_if.h"
//#include "mdk_lib.h"
/* Exported macros --------------------------------------------------------*/
#define  IAP_DEBUG_MODE                     0           /* 是否处于DEBUG模式 */     
     
#define  FILE_HEADER_SIZE                   0x100       /* 程序文件头大小 */

#define  ONCHIP_APP_BASE_ADDR               0x08007F00  /* 用户程序的真正入口地址，应从该基地址向后偏移0x100 */
#define USER_FLASH_START_ADDRESS            0x08008000  /* Only as example see comment */
/* 后续要重新分配，考虑预留全部种类子卡的文件空间，避免频繁变动 */
#define   FLASH_BLOCK_SIZE                512             /* 单位：字节 */

#ifdef APOLLO_OUTPUT_APPLIB
#define BOOTKEY_TCA9555_UESED            1
#if BOOTKEY_TCA9555_UESED
    #define BOOTKEY_TCA9555             EN_TCA9555_DEV_A

    #define BOOTKEY_SEL_TCA9555_CH      EN_TCA9555_CH_00
    #define BOOTKEY_ESC_TCA9555_CH      EN_TCA9555_CH_17
#else  
    /** 如果按下为低电平需要取反 **/
    #define BOOTKEY_SEL_READ()		    (!GPIOB_GETVALUE(EN_GPIO_PIN_06))
    #define BOOTKEY_ESC_READ()		    (!GPIOB_GETVALUE(EN_GPIO_PIN_06))
#endif
#elif defined (OUTPUT_KING_LIB)
#define BOOTKEY_TCA9555_UESED            1
#if BOOTKEY_TCA9555_UESED
    #define BOOTKEY_TCA9555             EN_TCA9555_DEV_A

    #define BOOTKEY_SEL_TCA9555_CH      EN_TCA9555_CH_00
    #define BOOTKEY_ESC_TCA9555_CH      EN_TCA9555_CH_17
#else  
    /** 如果按下为低电平需要取反 **/
    #define BOOTKEY_SEL_READ()		    (!GPIOB_GETVALUE(EN_GPIO_PIN_06))
    #define BOOTKEY_ESC_READ()		    (!GPIOB_GETVALUE(EN_GPIO_PIN_06))
#endif
#else
#define BOOTKEY_TCA9555_UESED           1
#if BOOTKEY_TCA9555_UESED
    #define BOOTKEY_TCA9555             EN_TCA9555_DEV_NUM

    #define BOOTKEY_SEL_TCA9555_CH      EN_TCA9555_CH_00
    #define BOOTKEY_ESC_TCA9555_CH      EN_TCA9555_CH_00
#else  
    /** 如果按下为低电平需要取反 **/
    #define BOOTKEY_SEL_READ()		    
    #define BOOTKEY_ESC_READ()		    
#endif
#endif
/* Exported types ---------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
typedef struct{
    bool        doc_upgrade_ing;    /* 更新中*/
    uint8_t     doc_upgrade_en;     /* 使能文件更新完成后,立即加载标志 */
    uint8_t     doc_upgrade_ok;  /* 文件更新完成后标志*/
    uint32_t    doc_upgrade_stamp;  /* 更新的时间戳记录，约定在5s内，如果没有下一包数据到达子卡，退出更新状态 */
}_upgrade_flag_info_t;
extern void  jumptoappprocess(void);

void  enterbootmodejudge(void);
void  sysfirmwareiap(_upgrade_flag_info_t  * upgrade_flag);
void FirmwareCopy(void);
void flash_if_test(void);



#ifdef __cplusplus
}
#endif  /* __cplusplus */


#endif  /* _FIRMWARE_IAP_H_ */

