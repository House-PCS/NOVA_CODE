/**
  ****************** (C) COPYRIGHT 2017 Nova  *******************
  * @file     firmware_iap.c
  * @auther   Liudongdong
  * @version  V1.0
  * @date     9/14/2017
  * @brief    This file provides the functions of firmware IAP.
  ***************************************************************
  * @attention
  *
  * History<br>
  * version   date            auther         description<br>
  * V1.0.0    2017.9.14       Liudongdong     Initial Version<br>
  ***************************************************************
  */

#include "firmware_iap.h"
#include "dev_ext_wdg.h"
#define LOG_TAG "firmware_iap"
/* Private define ------------------------------------------------------------*/
#define  FLASH_PAGE_SIZE            (256)

#define     ONCHIP_TO_OFFCHIP       0x00
#define     OFFCHIP_TO_ONCHIP       0x01
#define     EN_DOC_MCU              0x01

static _upgrade_flag_info_t  _sys_upgrade_flag = {0};
//#if BOOT_MODE

//#if USE_UPGRADE
/* Private types  ------------------------------------------------------------*/
typedef enum
{
    RUNMODE_DET = 0,
    POWER_KEY_DET,
    EXT_CTRL_DET,
    ONCHIP_VERIFY,
    OFFCHIP_VERIFY,
    VERIFY_RESULT,
    ONCHIP2OFFCHIP_COPY,
    OFFCHIP2ONCHIP_COPY,
    FIRMWARE_UPGRADE,
    JUMP2APP
} _IAP_fsm_t;

typedef enum
{
    NORMAL_POWER_ON = 0,    /* 正常上电运行 */
    UPGRADE_POWER_ON,       /* 固件更新上电运行 */
    ABNORMAL_POWER_ON,      /* 初次上电或异常断上电运行 */
} _sys_runmode_t;
typedef enum
{
    VERIFY_OK = 0,
    VERIFY_ERROR
} _verify_result_t;

typedef struct      /* IAP流程中的控制信息结构体 */
{
    _sys_runmode_t    sys_runmode;        /* 系统上电运行模式 */
    _verify_result_t  app_verify_onchip;  /* 片上APP程序校验结果 */
    _verify_result_t  app_verify_offchip;    /* 片外APP程序校验结果 */
    uint32_t  app_onchip_size;     /* 片内APP程序大小，字节为单位 */
    uint32_t  app_offchip_size;    /* 片外APP程序大小，字节为单位 */
} _IAP_state_t;

typedef  void (*pFunction)(void);

/* Private variables ---------------------------------------------------------*/
uint32_t  JumpAddress = 0;

_IAP_fsm_t    _sys_IAP_fsm = RUNMODE_DET;//RUNMODE_DET;

_IAP_state_t  _sys_IAP_state = {    \
                                    .sys_runmode = NORMAL_POWER_ON,
                                    .app_verify_onchip = VERIFY_OK,
                                    .app_verify_offchip = VERIFY_OK,
                                    .app_onchip_size = 0,
                                    .app_offchip_size = 0,
                               };

/* Private constants ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
pFunction Jump_To_Application;
static void  DisableAllInterrupt(void);

void FlashWrite(uint16_t len, uint8_t  *data, uint32_t addr_start)
{
    uint16_t i;
    uint32_t temp;

    fmc_state_enum  fmc_state = FMC_READY;

    fmc_unlock();

    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    for(i = 0; i < len / 4; i++)
    {   
        DevFeedExtWdg();
        fmc_state = fmc_word_program(addr_start, *(uint32_t*)data);

        if(fmc_state != FMC_READY)
        {
            return;
        }

        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

        data += 4;
        addr_start += 4;
    }
    if(len % 4)
    {
        temp = *(uint32_t*)data | 0xff000000;
        fmc_state = fmc_word_program(addr_start, temp);
    }

    fmc_lock();
}

void FlashRead(uint16_t len, uint8_t *outdata, uint32_t addr_start)
{
    uint32_t addr;
    uint16_t i;

    addr = addr_start;

    for(i = 0; i < len; i++)
    {
        *outdata = *(uint8_t*) addr;
        addr = addr + 1;
        outdata++;
    }
}

void FlashErase(uint32_t start, uint32_t end)
{
    uint32_t EraseCounter;
    uint32_t PageNum;

    PageNum = (end - start) / 0x800;
    /* unlock the flash program/erase controller */
    fmc_unlock();

    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    /* erase the flash pages */
    for(EraseCounter = 0; EraseCounter < PageNum; EraseCounter++)
    {
        DevFeedExtWdg();
        fmc_page_erase(start + (0x800 * EraseCounter));
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    }

    /* lock the main FMC after the erase operation */
    fmc_lock();
//  uint32_t EraseCounter;
//  fmc_state_enum fmc_state=FMC_READY;

//  /* unlock the flash program/erase controller */
//    fmc_unlock();
//    /* clear all pending flags */
//    fmc_flag_clear(FMC_FLAG_BANK0_END);
//    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
//    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
//
//    /* erase the flash pages */
//    while(start < end)
//  {
//      EraseCounter = start/FLASH_PAGE_SIZE;
//        fmc_state=fmc_page_erase(EraseCounter*FLASH_PAGE_SIZE);
//      if(fmc_state!=FMC_READY)
//      {
//          return;
//      }
//        fmc_flag_clear(FMC_FLAG_BANK0_END);
//        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
//        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
//
//      start += FLASH_PAGE_SIZE;
//    }
//    /* lock the main FMC after the erase operation */
//    fmc_lock();
}

//uint16_t i,j = 0;
//uint8_t wdata[256];
//uint8_t rdata[256];
//void flash_if_test(void)
//{
//    for(i = 0; i < 256; i ++)
//    {
//        wdata[i] = i;
//    }
//    *wdata = 0x0A;
//    *(wdata + 1) = 0x0A;
//    *(wdata + 2) = 0x0A;
//    *(wdata + 3) = 0x03;
//
//    FlashErase(0x08003800, 0x08020000);
////    FlashWrite(256, wdata, 0x08007800);
//    for(i = 0; i < 456; i ++)
//    {
//        FlashRead(256, rdata, 0x08007800 + i * 256);
//        for(j = 0; j < 256; j ++)
//        {
//            if(rdata[j] != 0xFF)
//            {
//                __NOP();
//            }
//        }
//    }
//
//}
//MSH_CMD_EXPORT(flash_if_test, ---);

void JumpFlagWrite(void)
{
    uint8_t firmware_upgrade[256] = {0};

    memset(firmware_upgrade, 0xA0, 256);
//    spi_flash_write_bytes_with_erase(UPGRADE_FLAG_ADDR, firmware_upgrade, 256);
    *firmware_upgrade = 0x0A;
    *(firmware_upgrade + 1) = 0x0A;
    *(firmware_upgrade + 2) = 0x0A;
    *(firmware_upgrade + 3) = 0x03;
    FlashErase(0x08007800, 0x08008000);
    FlashWrite(4, firmware_upgrade, 0x08007F00);

    jumptoappprocess();
}
MSH_CMD_EXPORT(JumpFlagWrite, boot->App);
/**
  * @brief  系统运行模式判断
  * @param  none
  * @return 系统运行模式
  */
void  DisableAllInterrupt(void)
{
    /* 跳转前关闭系统中断，待补充 */
    uint16_t  i;
//    SysTick_Disable();
    __set_PRIMASK(1);   // 关闭总中断
    //__set_FAULTMASK(1);    // 关闭总中断
//    for(i = 0; i < 3000; i++)
//    {
//        ;
//    }
}

/**
  * @brief  系统运行模式判断
  * @param  none
  * @return 系统运行模式
  */
static void sys_runmode_detect(void)
{
    uint8_t firmware_upgrade[256] = {0};
    uint16_t i = 0;
    spi_flash_read_bytes(UPGRADE_FLAG_ADDR, firmware_upgrade, 256);

    for(i = 0; i < 256; i ++)
    {
        DevFeedExtWdg();
        if(firmware_upgrade[i] != 0xA0)
        {
            break;
        }
    }
    if(i == 256)
    {
        _sys_IAP_state.sys_runmode = UPGRADE_POWER_ON;
    }
    else if((rcu_flag_get(RCU_FLAG_PORRST) == SET))
    {
        _sys_IAP_state.sys_runmode = ABNORMAL_POWER_ON;
    }
    else
    {
        _sys_IAP_state.sys_runmode = NORMAL_POWER_ON;
    }

    rcu_all_reset_flag_clear();
}

/**
  * @brief  片上APP程序校验
  * @param  none
  * @return none
  */
static void onchip_app_verify(void)
{
    uint32_t tmp_byte_cnt = 0;
    uint32_t tmp_file_len = 0;
    uint32_t tmp_app_sum = 0;
    uint32_t tmp_sum = 0;
    uint16_t mode_Id = 0;
    uint8_t * tmp_onchip_ptr = (uint8_t *)ONCHIP_APP_BASE_ADDR;

    if(memcmp(tmp_onchip_ptr, gBMSFirmwareType, 4) != 0)
    {
        _sys_IAP_state.app_verify_onchip = VERIFY_ERROR;
//        return;
    }
    else
    {
        _sys_IAP_state.app_verify_onchip = VERIFY_OK;
        return;
    }

    /* 获取文件长度和校验和 */
//    tmp_file_len = (uint32_t)(*(tmp_onchip_ptr + 7)) + (((uint32_t)(*(tmp_onchip_ptr + 8))) << 8) + \
//                   (((uint32_t)(*(tmp_onchip_ptr + 9))) << 16) + (((uint32_t)(*(tmp_onchip_ptr + 10))) << 24);
    tmp_file_len = *((uint32_t*)(tmp_onchip_ptr + 7));
    tmp_app_sum = *((uint32_t*)(tmp_onchip_ptr + 252));

    if((!tmp_file_len) || (tmp_file_len  > (128 * 1024) - FILE_HEADER_SIZE)) /*文件大小是否合理 */
    {
        _sys_IAP_state.app_verify_onchip = VERIFY_ERROR;
        return;
    }

    _sys_IAP_state.app_onchip_size = tmp_file_len;  /* 记录文件大小 */

    /* 全文件校验和计算 */
    tmp_onchip_ptr = (uint8_t *)(ONCHIP_APP_BASE_ADDR + FILE_HEADER_SIZE);
    tmp_sum = 0;
    for(tmp_byte_cnt = 0; tmp_byte_cnt < tmp_file_len - FILE_HEADER_SIZE; tmp_byte_cnt++)
    {
        tmp_sum += *(tmp_onchip_ptr + tmp_byte_cnt);
        if(tmp_sum == 0x98216b || tmp_sum == 0x982450)
        {
            __NOP();
        }
    }
//    tmp_sum += VERIFY_CONST_VAL;    /* 叠加最后的常量 */
    if(tmp_sum == tmp_app_sum)
    {
        _sys_IAP_state.app_verify_onchip = VERIFY_OK;
#if  IAP_DEBUG_MODE
        uart_sendstring(EN_UART3_BUS, "BOOT: ON VERIFY_OK;\r\n");
#endif
    }
    else
    {
        _sys_IAP_state.app_verify_onchip = VERIFY_ERROR;
#if  IAP_DEBUG_MODE
        uart_sendstring(EN_UART3_BUS, "BOOT: ON VERIFY_ERROR;\r\n");
#endif
    }
}

/**
  * @brief  片外APP程序校验
  * @param  none
  * @return none
  */
static void offchip_app_verify(void)
{
    uint32_t tmp_block_cnt = 0;
    uint32_t tmp_byte_cnt = 0;
    uint32_t tmp_file_len = 0;
    uint32_t tmp_file_remainder = 0;     /* 不足一个block的字节数 */
    uint32_t tmp_block_num = 0;
    uint32_t tmp_app_sum = 0;
    uint32_t tmp_sum = 0;
    uint8_t  tmp_buf[FLASH_BLOCK_SIZE] = {0};

    /* 获取文件长度和校验和 */
    spi_flash_read_bytes(BMS_FIRMWARE_ADDR, tmp_buf, 0x100);
    tmp_file_len = *((uint32_t*)&tmp_buf[7]);

    tmp_app_sum = *((uint32_t*)&tmp_buf[252]);

    if(!tmp_file_len || tmp_file_len > (128 * 1024)) /*文件大小是否合理 */
    {
        _sys_IAP_state.app_verify_offchip = VERIFY_ERROR;
        return;
    }

    if(memcmp(tmp_buf, gBMSFirmwareType, 4) != 0)
    {
        _sys_IAP_state.app_verify_offchip = VERIFY_ERROR;
        return;
    }

    _sys_IAP_state.app_offchip_size = tmp_file_len;  /* 记录文件大小 */

    /** 全文件校验和计算
     * 先计算整数block的校验和
     * 再计算不足一个block的校验和，最后加常量求和
     */
    tmp_sum = 0;
    tmp_block_num = tmp_file_len / FLASH_BLOCK_SIZE;
    for(tmp_block_cnt = 0; tmp_block_cnt < tmp_block_num; tmp_block_cnt++)
    {
        spi_flash_read_bytes(BMS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, tmp_buf, FLASH_BLOCK_SIZE);
        if(!tmp_block_cnt)
        {
            tmp_byte_cnt = FILE_HEADER_SIZE;    /* 计算校验和，应去掉前256包头 */
        }
        else
        {
            tmp_byte_cnt = 0;
        }

        for(; tmp_byte_cnt < FLASH_BLOCK_SIZE; tmp_byte_cnt++)
        {
            tmp_sum += tmp_buf[tmp_byte_cnt];
        }
    }

    tmp_file_remainder = tmp_file_len % FLASH_BLOCK_SIZE;
    if(tmp_file_remainder)
    {
        spi_flash_read_bytes(BMS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, tmp_buf, FLASH_BLOCK_SIZE);
//        filenbytesread(EN_DOC_MCU, tmp_buf, tmp_block_cnt * FLASH_BLOCK_SIZE, FLASH_BLOCK_SIZE);
        for(tmp_byte_cnt = 0; tmp_byte_cnt < tmp_file_remainder; tmp_byte_cnt++)
        {
            tmp_sum += tmp_buf[tmp_byte_cnt];
        }
    }

//    tmp_sum += VERIFY_CONST_VAL;    /* 叠加最后的常量 */

    if(tmp_sum == tmp_app_sum)
    {
        _sys_IAP_state.app_verify_offchip = VERIFY_OK;
#if  IAP_DEBUG_MODE
        uart_sendstring(EN_UART3_BUS, "BOOT: OFF VERIFY_OK;\r\n");
#endif
    }
    else
    {
        _sys_IAP_state.app_verify_offchip = VERIFY_ERROR;
#if  IAP_DEBUG_MODE
        uart_sendstring(EN_UART3_BUS, "BOOT: OFF VERIFY_ERROR;\r\n");
#endif
    }
}

/**
  * @brief  片内外APP程序互拷处理
  * @param  direction: 0x00,片内拷贝到片外; 0x01,片外拷贝到片内;
  * @return none
  */
static void app_filecopy_process(uint8_t direction)
{
    uint8_t * tmp_onchip_ptr = (uint8_t *)ONCHIP_APP_BASE_ADDR;
    uint32_t  tmp_onchip_addr = ONCHIP_APP_BASE_ADDR;
    uint32_t* tmp_onchip_uint32_tptr = &tmp_onchip_addr;
    uint32_t  tmp_block_cnt = 0;
    uint32_t  tmp_byte_cnt = 0;
    uint32_t  tmp_block_num = 0;
    uint32_t  tmp_byte_remainder = 0;
    uint32_t  tmp_file_len = 0;
    uint8_t   tmp_buf[512];
    DevFeedExtWdg();
    switch(direction)
    {
        case 0x00:  /* 片内拷贝到片外 */
        {
            /* 获取文件长度和校验和 */
            _sys_IAP_state.app_onchip_size = *((uint32_t*)(tmp_onchip_ptr + 7));
            tmp_file_len = _sys_IAP_state.app_onchip_size;
            if(!(tmp_file_len - FILE_HEADER_SIZE) || tmp_file_len > (128 * 1024)) /*文件大小是否合理 */
            {
                _sys_IAP_state.app_verify_onchip = VERIFY_ERROR;
                return;
            }

            tmp_block_num = (tmp_file_len / FLASH_BLOCK_SIZE);
            if(tmp_file_len % FLASH_BLOCK_SIZE)    /* 此处需要优化，防止block写时访问溢出 */
            {
                tmp_block_num++;
            }
            for(tmp_block_cnt = 0; tmp_block_cnt < tmp_block_num; tmp_block_cnt++)
            {
                DevFeedExtWdg();
                spi_flash_write_bytes_with_erase(BMS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, tmp_onchip_ptr, FLASH_BLOCK_SIZE);
//                filenbyteswrite(EN_DOC_MCU, tmp_onchip_ptr, tmp_block_cnt * FLASH_BLOCK_SIZE, FLASH_BLOCK_SIZE);
                tmp_onchip_ptr += FLASH_BLOCK_SIZE;
            }
        }
        break;

        case 0x01:  /* 片外拷贝到片内 */
        {
//            /* erase user flash area */
//            FlashErase(ONCHIP_APP_BASE_ADDR, 0x8020000);
//            FLASH_If_Erase(ONCHIP_APP_BASE_ADDR);

            /* 初始化片上用户区程序入口地址 */
//            tmp_onchip_addr = ONCHIP_APP_BASE_ADDR;
//            tmp_onchip_uint32_tptr = &tmp_onchip_addr;

            GpioBitSet((&LED_Handle));
            /* 获取文件长度和校验和 */
            spi_flash_read_bytes(BMS_FIRMWARE_ADDR, tmp_buf, FLASH_BLOCK_SIZE);

            _sys_IAP_state.app_offchip_size = *((uint32_t*)&tmp_buf[7]);
            tmp_file_len = _sys_IAP_state.app_offchip_size;
            if(!(tmp_file_len - FILE_HEADER_SIZE) || tmp_file_len > (256 * 1024)) /*文件大小是否合理 */
            {
                _sys_IAP_state.app_verify_offchip = VERIFY_ERROR;
                return;
            }

            /* erase user flash area */
//            FlashErase(ONCHIP_APP_BASE_ADDR, tmp_file_len + ONCHIP_APP_BASE_ADDR);
            FlashErase(0x8007800, 0x8040000);
            tmp_block_num =  (tmp_file_len + 3) / FLASH_BLOCK_SIZE;
            for(tmp_block_cnt = 0; tmp_block_cnt < tmp_block_num; tmp_block_cnt++)
            {
//                GpioBitToogle((&LED_Handle));
                DevFeedExtWdg();
                spi_flash_read_bytes(BMS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, tmp_buf, FLASH_BLOCK_SIZE);
                FlashWrite(FLASH_BLOCK_SIZE, tmp_buf, ONCHIP_APP_BASE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE);
            }

            tmp_byte_remainder = (tmp_file_len + 3) % FLASH_BLOCK_SIZE;
            spi_flash_read_bytes(BMS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, tmp_buf, FLASH_BLOCK_SIZE);
            FlashWrite(tmp_byte_remainder, tmp_buf, ONCHIP_APP_BASE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE);
//            filenbytesread(EN_DOC_MCU, tmp_buf, tmp_block_cnt * FLASH_BLOCK_SIZE, FLASH_BLOCK_SIZE);
//            for(tmp_byte_cnt = 0; tmp_byte_cnt < tmp_byte_remainder; tmp_byte_cnt += 4)
//            {
//                FlashWrite(FLASH_BLOCK_SIZE, tmp_buf, BMS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE);
//                FLASH_If_Write(tmp_onchip_uint32_tptr, (uint32_t *)&tmp_buf[tmp_byte_cnt], 1);
//            }
//            soft_delayms(100);
            GpioBitReset((&LED_Handle));
        }
        break;

        default :
            break;
    }
}

#define FMC_START_ADDR          ((uint32_t)0x08000000U)
#define APP_FIRMWARE_INTERNAL_ADDR           0x8000
typedef void (*iapfun)();

iapfun jump2app;
void JumpToApp()
{
//    LOG_INFO(LOG_TAG, "jumpStart");
//    UartDeInit(&Uart1_ModS_Handle);
    DevFeedExtWdg();
//    TurnOff();
    soft_delayms(10);
    SysTick->CTRL &= (~SysTick_CTRL_ENABLE_Msk);
//    INTX_DISABLE();
    __set_PRIMASK(1);   // 关闭总中断
    jump2app = (iapfun) * (uint32_t*)(USER_FLASH_START_ADDRESS + 4);
    __set_PSP(*(__IO uint32_t *)USER_FLASH_START_ADDRESS);
    __set_CONTROL(0);
    __set_MSP(*(__IO uint32_t *)USER_FLASH_START_ADDRESS);
//    jump2app = (iapfun)*(uint32_t*)(FMC_START_ADDR + APP_FIRMWARE_INTERNAL_ADDR + 4);
//    __set_PSP(*(__IO uint32_t *)FMC_START_ADDR + APP_FIRMWARE_INTERNAL_ADDR);
//    __set_CONTROL(0);
//    __set_MSP(*(__IO uint32_t *)FMC_START_ADDR + APP_FIRMWARE_INTERNAL_ADDR);
    jump2app();
}

void  jumptoappprocess(void)
{
    spi_flash_auto_erase_sector(UPGRADE_FLAG_ADDR, 256);
    uint8_t TimeOutCnt = 10;
//    TurnOff();
    soft_delayms(10);
    /* Check if valid stack address (RAM address) then jump to user application */
    while(TimeOutCnt)
    {
        if(((*(__IO uint32_t*)USER_FLASH_START_ADDRESS) & 0x2FFC0000) == 0x20000000)  /*检查栈顶地址是否合法*/
        {
            DisableAllInterrupt();
            /* Jump to user application */
            JumpAddress = *(__IO uint32_t*) (USER_FLASH_START_ADDRESS + 4);
            Jump_To_Application = (pFunction) JumpAddress;
            /* Initialize user application's Stack Pointer */
            __set_PSP(*(__IO uint32_t*) USER_FLASH_START_ADDRESS);
            __set_CONTROL(0);
            __set_MSP(*(__IO uint32_t*) USER_FLASH_START_ADDRESS);
            Jump_To_Application();
            while(1);
        }
        else
        {
            TimeOutCnt--;
        }
    }

    NVIC_SystemReset();
    while(1);
}

static void CheckBootKey(void)
{
#if BOOTKEY_TCA9555_UESED
    uint16_t keytmp = 0xFFFF;
#if USE_TCA9555
    /* 9555 */
    tca9555_setsinglepinmode(BOOTKEY_TCA9555, BOOTKEY_SEL_TCA9555_CH, TCA9555_IN_MODE);  /** ENTER **/
    tca9555_setsinglepinmode(BOOTKEY_TCA9555, BOOTKEY_ESC_TCA9555_CH, TCA9555_IN_MODE);  /** ESC **/

    tca9555_getpinstatus(BOOTKEY_TCA9555, &keytmp);  /** Boot 按键按下  ENTER+ESC**/

    if(BITGET(keytmp, BOOTKEY_SEL_TCA9555_CH) == 0 && BITGET(keytmp, BOOTKEY_ESC_TCA9555_CH) == 0)
    {
        _sys_IAP_fsm = FIRMWARE_UPGRADE;
    }
#endif
#else
#if USE_GPIO
    if(BOOTKEY_SEL_READ() && BOOTKEY_ESC_READ())
    {
        systick_delayms(40);
        if(BOOTKEY_SEL_READ() && BOOTKEY_ESC_READ())
        {
            _sys_IAP_fsm = FIRMWARE_UPGRADE;
        }
    }
#endif
#endif
}

/**
  * @brief  判断系统是否要进入BOOT模式，等待程序更新
  * @param  none
  * @return none
  */
void  enterbootmodejudge(void)
{
    while(_sys_IAP_fsm != FIRMWARE_UPGRADE)
    {
        DevFeedExtWdg();
        sysfirmwareiap(&_sys_upgrade_flag);
    }
}

/**
  * @brief  系统固件更新状态控制处理
  * @param  info_hw ：待初始化的硬件配置信息结构体指针
  * @return none
  */
void  sysfirmwareiap(_upgrade_flag_info_t  * upgrade_flag)
{
    DevFeedExtWdg();
    switch(_sys_IAP_fsm)
    {

        case RUNMODE_DET :  /* 上电运行类型判断 */
        {
#if  IAP_DEBUG_MODE
            uart_sendstring(EN_UART3_BUS, "BOOT: RUNMODE_DET;\r\n");
#endif
            sys_runmode_detect();
            if(_sys_IAP_state.sys_runmode == UPGRADE_POWER_ON)
            {
                _sys_IAP_fsm = OFFCHIP2ONCHIP_COPY;
            }
            else if(_sys_IAP_state.sys_runmode == ABNORMAL_POWER_ON || _sys_IAP_state.sys_runmode == NORMAL_POWER_ON)
            {
                _sys_IAP_fsm = ONCHIP_VERIFY;
            }
        }
        break;

        case  ONCHIP_VERIFY:    /* 片内程序校验(全文件) */
        {
#if  IAP_DEBUG_MODE
            uart_sendstring(EN_UART3_BUS, "BOOT: ONCHIP_VERIFY;\r\n");
#endif
            onchip_app_verify();
            if(VERIFY_OK == _sys_IAP_state.app_verify_onchip)
            {
                _sys_IAP_fsm = JUMP2APP;
            }
            else
            {
                _sys_IAP_fsm = FIRMWARE_UPGRADE;
            }
        }
        break;

        case  OFFCHIP_VERIFY:   /* 片外程序校验(全文件) */
        {
#if  IAP_DEBUG_MODE
            uart_sendstring(EN_UART3_BUS, "BOOT: OFFCHIP_VERIFY;\r\n");
#endif
            offchip_app_verify();
            _sys_IAP_fsm = VERIFY_RESULT;
        }
        break;

        case  VERIFY_RESULT:    /* 片内外校验结果处理 */
        {
#if  IAP_DEBUG_MODE
            uart_sendstring(EN_UART3_BUS, "BOOT: VERIFY_RESULT;\r\n");
#endif
            if((VERIFY_OK == _sys_IAP_state.app_verify_onchip) && (VERIFY_OK == _sys_IAP_state.app_verify_offchip))
            {
                _sys_IAP_fsm = JUMP2APP;
            }
            else if((VERIFY_OK == _sys_IAP_state.app_verify_onchip) && (VERIFY_ERROR == _sys_IAP_state.app_verify_offchip))
            {
                _sys_IAP_fsm = ONCHIP2OFFCHIP_COPY;
            }
            else if((VERIFY_ERROR == _sys_IAP_state.app_verify_onchip) && (VERIFY_OK == _sys_IAP_state.app_verify_offchip))
            {
                _sys_IAP_fsm = OFFCHIP2ONCHIP_COPY;
            }
            else    /* 两份均校验失败 */
            {
                _sys_IAP_fsm = FIRMWARE_UPGRADE;
            }
        }
        break;

        case  ONCHIP2OFFCHIP_COPY:  /* 将程序文件从片内拷贝到片外 */
        {
#if  IAP_DEBUG_MODE
            uart_sendstring(EN_UART3_BUS, "BOOT: ONCHIP2OFFCHIP_COPY;\r\n");
#endif
            app_filecopy_process(ONCHIP_TO_OFFCHIP );
            if(_sys_IAP_state.app_verify_onchip == VERIFY_ERROR)
            {
                _sys_IAP_fsm = FIRMWARE_UPGRADE;
            }
            else
            {
                _sys_IAP_fsm = JUMP2APP;
            }

        }
        break;

        case  OFFCHIP2ONCHIP_COPY:  /* 将程序文件从片外拷贝到片内 */
        {
#if  IAP_DEBUG_MODE
            uart_sendstring(EN_UART3_BUS, "BOOT: OFFCHIP2ONCHIP_COPY;\r\n");
#endif
            app_filecopy_process(OFFCHIP_TO_ONCHIP);
            if(_sys_IAP_state.app_verify_offchip == VERIFY_ERROR)
            {
                _sys_IAP_fsm = FIRMWARE_UPGRADE;
            }
            else
            {
                /* 固件更新完成，擦除更新标志 */
                spi_flash_auto_erase_sector(UPGRADE_FLAG_ADDR, 256);
                gBMSFirmwareUpgradeEnd = 2;
                _sys_IAP_fsm = JUMP2APP;
//                LOG_INFO(LOG_TAG, "OFFCHIP2ONCHIP_COPY  VERIFY_Success");
            }
        }
        break;

        case  FIRMWARE_UPGRADE:     /* 固件升级处理 */
        {
            if((upgrade_flag->doc_upgrade_en == 0x01) && (upgrade_flag->doc_upgrade_ok == EN_DOC_MCU))
            {
#if  IAP_DEBUG_MODE
                uart_sendstring(EN_UART3_BUS, "BOOT: FIRMWARE_UPGRADE;\r\n");
#endif
                _sys_IAP_fsm = OFFCHIP2ONCHIP_COPY; //OFFCHIP2ONCHIP_COPY;
            }
        }
        break;

        case  JUMP2APP:     /* 程序跳转处理 */
        {
#if  IAP_DEBUG_MODE
            uart_sendstring(EN_UART3_BUS, "BOOT: JUMP2APP;\r\n");
#endif
            JumpToApp();
//            while(1);
        }
        break;

        default :
        {
            /* Nothing to do */
        }
        break;
    }
}
void FirmwareCopy(void)
{
    _sys_IAP_fsm = OFFCHIP2ONCHIP_COPY;
}
//#endif
//#endif

MSH_CMD_EXPORT(jumptoappprocess, boot jump to app);