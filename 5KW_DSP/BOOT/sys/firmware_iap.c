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
#include "modbus_analysis.h"

/* Private define ------------------------------------------------------------*/
#define PCS_FIRMWARE_ADDR   0x00001000
#define UPGRADE_FLAG_ADDR   0x00000000

#define     ONCHIP_TO_OFFCHIP       0x00
#define     OFFCHIP_TO_ONCHIP       0x01

static uint16_t _garrTemp512[512];
static uint16_t _garrTemp256[256];
//#if BOOT_MODE

//#if USE_UPGRADE
/* Private types  ------------------------------------------------------------*/
typedef enum
{
    EN_RUNMODE_DET = 0,
    EN_POWER_KEY_DET,
    EN_EXT_CTRL_DET,
    EN_ONCHIP_VERIFY,
    EN_OFFCHIP_VERIFY,
    EN_VERIFY_RESULT,
    EN_ONCHIP2OFFCHIP_COPY,
    EN_OFFCHIP2ONCHIP_COPY,
    EN_FIRMWARE_UPGRADE,
    EN_JUMP2APP
} EN_IAP_FSM;

typedef enum
{
    EN_NORMAL_POWER_ON = 0,    /* 正常上电运行 */
    EN_UPGRADE_POWER_ON,       /* 固件更新上电运行 */
    EN_ABEN_NORMAL_POWER_ON,      /* 初次上电或异常断上电运行 */
} EN_SYS_RUNMODE;

typedef enum
{
    EN_VERIFY_OK = 0,
    EN_VERIFY_ERROR
} EN_VERIFY_STATUS;

typedef struct _iap_state_stru_t     /* IAP流程中的控制信息结构体 */
{
    EN_SYS_RUNMODE    sys_runmode;        /* 系统上电运行模式 */
    EN_VERIFY_STATUS  app_verify_onchip;  /* 片上APP程序校验结果 */
    EN_VERIFY_STATUS  app_verify_offchip;    /* 片外APP程序校验结果 */
    uint32_t  app_onchip_size;     /* 片内APP程序大小，字节为单位 */
    uint32_t  app_offchip_size;    /* 片外APP程序大小，字节为单位 */
} IapState_t;

/* Private variables ---------------------------------------------------------*/
uint32_t  JumpAddress = 0;

static EN_IAP_FSM  _geSysIapFsm = EN_RUNMODE_DET;//EN_RUNMODE_DET;

static IapState_t _gstSysIapState = {    \
                                         .sys_runmode = EN_NORMAL_POWER_ON,
                                         .app_verify_onchip = EN_VERIFY_ERROR,
                                         .app_verify_offchip = EN_VERIFY_ERROR,
                                         .app_onchip_size = 0,
                                         .app_offchip_size = 0,
                                    };

/* Private constants ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void ArrU8ToU16(uint16_t* u16_arr, uint16_t* u8_arr, uint16_t u8_arr_len)
{
    uint16_t i = 0;
    uint16_t data_len = 0;

    if(u16_arr == NULL || u8_arr == NULL || (u8_arr_len % 2 != 0)) // 20260429，优化代码
    {
        return;
    }

    data_len = (u8_arr_len >> 1) * 2;

    for(i = 0; i < data_len; i ++)
    {
        *(u16_arr + i) = *(u8_arr + 2 * i) + (*(u8_arr + 2 * i + 1) << 8);
    }
}

void JumpFlagWrite(void)
{
//    memset(arr_firmware_info, 0xA0, 256);
//    SpiFlashWriteBytesWithErase(UPGRADE_FLAG_ADDR, arr_firmware_info, 256);
    *_garrTemp256 = 0x0A;
    *(_garrTemp256 + 1) = 0x0A;
    *(_garrTemp256 + 2) = 0x0A;
    *(_garrTemp256 + 3) = 0x03;
    InFlashWrite(ONCHIP_APP_BASE_ADDR, _garrTemp256, 8);

    JumpToAppProcess();
}

/**
  * @brief  系统运行模式判断
  * @param  none
  * @return 系统运行模式
  */
static void _SysRunmodeDetect(void)
{
    uint16_t i = 0;

    SpiFlashReadBytes(UPGRADE_FLAG_ADDR, _garrTemp256, 256);
    for(i = 0; i < 256; i ++)
    {
        if(_garrTemp256[i] != 0xA0)
        {
            break;
        }
    }

    if(i == 256)
    {
        _gstSysIapState.sys_runmode = EN_UPGRADE_POWER_ON;
    }
    else
    {
        _gstSysIapState.sys_runmode = EN_NORMAL_POWER_ON;
    }
}

/**
  * @brief  片上APP程序校验
  * @param  none
  * @return none
  */
static void _OnchipAppVerify(void)
{
    uint32_t tmp_byte_cnt __attribute((unused));
    uint32_t tmp_file_len __attribute((unused));
    uint32_t tmp_app_sum __attribute((unused));
    uint32_t tmp_sum __attribute((unused));
    uint16_t mode_Id __attribute((unused));
    uint16_t * tmp_onchip_ptr = (uint16_t *)ONCHIP_APP_BASE_ADDR;
    uint16_t tmp_data = 0;
    if(memcmp(tmp_onchip_ptr, gPCSFirmwareType, 4) != 0)
    {
        _gstSysIapState.app_verify_onchip = EN_VERIFY_ERROR;
        return;
    }

    /* 获取文件长度和校验和 */
    tmp_file_len = (uint32_t)(*(tmp_onchip_ptr + 7)) + (((uint32_t)(*(tmp_onchip_ptr + 8))) << 8) + \
                   (((uint32_t)(*(tmp_onchip_ptr + 9))) << 16) + (((uint32_t)(*(tmp_onchip_ptr + 10))) << 24);

    tmp_app_sum = (uint32_t)(*(tmp_onchip_ptr + 252)) + (((uint32_t)(*(tmp_onchip_ptr + 253))) << 8) + \
                  (((uint32_t)(*(tmp_onchip_ptr + 254))) << 16) + (((uint32_t)(*(tmp_onchip_ptr + 255))) << 24);

//    if((tmp_file_len == 0) || (tmp_file_len  > FIRMWARE_SIZE_MAX)) /*文件大小是否合理 */
//    {
//        _gstSysIapState.app_verify_onchip = EN_VERIFY_ERROR;
//        return;
//    }

    if(tmp_file_len <= FILE_HEADER_SIZE || tmp_file_len > FIRMWARE_SIZE_MAX)  // 20260429，优化代码
    {
        _gstSysIapState.app_verify_onchip = EN_VERIFY_ERROR;
        return;
    }

    _gstSysIapState.app_onchip_size = ((tmp_file_len - FILE_HEADER_SIZE) >> 1);  /* 记录文件大小 */

    /* 全文件校验和计算 */
    /*0x90000(256字节包头)*/
    /*0x98000(程序入口)*/
    tmp_onchip_ptr = (uint16_t *)(USER_FLASH_START_ADDRESS);
    tmp_sum = 0;
    for(tmp_byte_cnt = 0; tmp_byte_cnt < _gstSysIapState.app_onchip_size; tmp_byte_cnt++)
    {
        /*dsp最小存储单元16bit，原程序4A 00 9A 29 -> 片上存储为004A 299A*/
        tmp_data = *(tmp_onchip_ptr + tmp_byte_cnt);
        tmp_sum += (tmp_data & 0x00FF);
        tmp_sum += (tmp_data >> 8);
    }

    if((tmp_file_len - FILE_HEADER_SIZE) % 2 != 0)
    {
        _gstSysIapState.app_onchip_size++;  /*还剩余一个字节*/
        tmp_data = *(tmp_onchip_ptr + tmp_byte_cnt);
        tmp_sum += (tmp_data & 0x00FF);
    }
        
    if(tmp_sum == tmp_app_sum)
    {
        _gstSysIapState.app_verify_onchip = EN_VERIFY_OK;
    }
    else
    {
        _gstSysIapState.app_verify_onchip = EN_VERIFY_ERROR;
    }
}

/**
  * @brief  片外APP程序校验
  * @param  none
  * @return none
  */
static void _OffchipAppVerify(void)
{
    uint32_t tmp_block_cnt = 0;
    uint32_t tmp_byte_cnt = 0;
    uint32_t tmp_file_len = 0;
    uint32_t tmp_file_remainder = 0;     /* 不足一个block的字节数 */
    uint32_t tmp_block_num = 0;
    uint32_t tmp_app_sum = 0;
    uint32_t tmp_sum = 0;

    /* 获取文件长度和校验和 */
    SpiFlashReadBytes(PCS_FIRMWARE_ADDR, _garrTemp512, 0x100);
//    tmp_file_len = *((uint32_t*)&_garrTemp512[7]);
    tmp_file_len = (uint32_t)(*(_garrTemp512 + 7)) + (((uint32_t)(*(_garrTemp512 + 8))) << 8) + \
                   (((uint32_t)(*(_garrTemp512 + 9))) << 16) + (((uint32_t)(*(_garrTemp512 + 10))) << 24);

//    tmp_app_sum = *((uint32_t*)&_garrTemp512[252]);
    tmp_app_sum = (uint32_t)(*(_garrTemp512 + 252)) + (((uint32_t)(*(_garrTemp512 + 253))) << 8) + \
                  (((uint32_t)(*(_garrTemp512 + 254))) << 16) + (((uint32_t)(*(_garrTemp512 + 255))) << 24);

    if((tmp_file_len == 0) || (tmp_file_len > FIRMWARE_SIZE_MAX)) /*文件大小是否合理 */
    {
        _gstSysIapState.app_verify_offchip = EN_VERIFY_ERROR;
        return;
    }

    if(memcmp(_garrTemp512, gPCSFirmwareType, 4) != 0)
    {
        _gstSysIapState.app_verify_offchip = EN_VERIFY_ERROR;
        return;
    }

    _gstSysIapState.app_offchip_size = tmp_file_len;  /* 记录文件大小 */

    /** 全文件校验和计算
     * 先计算整数block的校验和
     * 再计算不足一个block的校验和，最后加常量求和
     */
    tmp_sum = 0;
    tmp_block_num = tmp_file_len / FLASH_BLOCK_SIZE;
    for(tmp_block_cnt = 0; tmp_block_cnt < tmp_block_num; tmp_block_cnt++)
    {
        SpiFlashReadBytes(PCS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, _garrTemp512, FLASH_BLOCK_SIZE);
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
            tmp_sum += _garrTemp512[tmp_byte_cnt];
        }
    }

    tmp_file_remainder = tmp_file_len % FLASH_BLOCK_SIZE;
    if(tmp_file_remainder)
    {
        SpiFlashReadBytes(PCS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, _garrTemp512, FLASH_BLOCK_SIZE);
        for(tmp_byte_cnt = 0; tmp_byte_cnt < tmp_file_remainder; tmp_byte_cnt++)
        {
            tmp_sum += _garrTemp512[tmp_byte_cnt];
        }
    }


    if(tmp_sum == tmp_app_sum)
    {
        _gstSysIapState.app_verify_offchip = EN_VERIFY_OK;
    }
    else
    {
        _gstSysIapState.app_verify_offchip = EN_VERIFY_ERROR;
    }
}

/**
  * @brief  片内外APP程序互拷处理
  * @param  direction: 0x00,片内拷贝到片外; 0x01,片外拷贝到片内;
  * @return none
  */
static void _AppFileCopyProcess(uint8_t direction)
{
//    uint8_t * tmp_onchip_ptr = (uint8_t *)ONCHIP_APP_BASE_ADDR;
//    uint32_t  tmp_onchip_addr = ONCHIP_APP_BASE_ADDR;
//    uint32_t* tmp_onchip_uint32_tptr = &tmp_onchip_addr;
    uint32_t  tmp_block_cnt = 0;
    uint32_t  tmp_byte_cnt __attribute((unused));
    uint32_t  tmp_block_num = 0;
    uint32_t  tmp_file_len = 0;

    switch(direction)
    {
        case 0x00:  /* 片内拷贝到片外 */
        {
            /* 获取文件长度和校验和 */
//            _gstSysIapState.app_onchip_size = *((uint32_t*)(tmp_onchip_ptr + 7));
//            tmp_file_len = _gstSysIapState.app_onchip_size;
//            if(!(tmp_file_len - FILE_HEADER_SIZE) || tmp_file_len > (128 *1024)) /*文件大小是否合理 */
//            {
//                _gstSysIapState.app_verify_onchip = EN_VERIFY_ERROR;
//                return;
//            }
//
//            tmp_block_num = (tmp_file_len / FLASH_BLOCK_SIZE);
//            if(tmp_file_len % FLASH_BLOCK_SIZE)    /* 此处需要优化，防止block写时访问溢出 */
//            {
//                tmp_block_num++;
//            }
//            for(tmp_block_cnt = 0; tmp_block_cnt < tmp_block_num; tmp_block_cnt++)
//            {
//                SpiFlashWriteBytesWithErase(PCS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, tmp_onchip_ptr, FLASH_BLOCK_SIZE);
//                tmp_onchip_ptr += FLASH_BLOCK_SIZE;
//            }
            break;
        }
        case 0x01:  /* 片外拷贝到片内 */
        {
            /* 获取文件长度和校验和 */
            SpiFlashReadBytes(PCS_FIRMWARE_ADDR, _garrTemp512, FILE_HEADER_SIZE);

            _gstSysIapState.app_offchip_size = (uint32_t)_garrTemp512[7] + ((uint32_t)_garrTemp512[8] << 8) + \
                                               ((uint32_t)_garrTemp512[9] << 16) + ((uint32_t)_garrTemp512[10] << 24);
            tmp_file_len = _gstSysIapState.app_offchip_size - FILE_HEADER_SIZE;
            if((tmp_file_len == 0) || tmp_file_len > FIRMWARE_SIZE_MAX) /*文件大小是否合理 */
            {
                _gstSysIapState.app_verify_offchip = EN_VERIFY_ERROR;
                return;
            }

            tmp_block_num =  tmp_file_len / FLASH_BLOCK_SIZE;
            if(tmp_file_len % FLASH_BLOCK_SIZE)
            {
                tmp_block_num += 1;
            }

            /* erase user flash area */
            InFlashErase();
            InFlashWrite(ONCHIP_APP_BASE_ADDR, _garrTemp512, FILE_HEADER_SIZE);

            for(tmp_block_cnt = 0; tmp_block_cnt < tmp_block_num; tmp_block_cnt++)
            {
                SpiFlashReadBytes(FILE_HEADER_SIZE + PCS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, _garrTemp512, FLASH_BLOCK_SIZE);
                ArrU8ToU16(_garrTemp256, _garrTemp512, FLASH_BLOCK_SIZE);
                InFlashWrite(USER_FLASH_START_ADDRESS + tmp_block_cnt * (FLASH_BLOCK_SIZE >> 1), _garrTemp256, (FLASH_BLOCK_SIZE >> 1));
            }
            DELAY_MS(10);
            break;
        }
        default :
            break;
    }
}

void JumpToAppProcess(void)
{
//  DisableDog();
//    ClearScia();
    APP_JUMP;
}

/**
  * @brief  判断系统是否要进入BOOT模式，等待程序更新
  * @param  none
  * @return none
  */
void EnterBootModeJudge(void)
{
    while(_geSysIapFsm != EN_FIRMWARE_UPGRADE)
    {
        SysFirmwareIap();
    }
}

/**
* @brief: SysFirmwareIap
* @param: upgrade_flag 待初始化的硬件配置信息结构体指针
* @note:
* 策略：有新固件 - 校验外部flash成功 - 搬运到内部 - 校验成功 - 跳转     √
*       有新固件 - 校验外部flash成功 - 搬运到内部 - 校验失败 - 停留boot  √
*       有新固件 - 校验外部flash失败 -  校验内部 - 擦除固件更新标志 - 跳转  √
*       有新固件 - 校验外部flash失败 - 校验内部 - 校验失败  - 停留boot  √
*       无新固件 - 校验内部flash成功 - 跳转 √
*       无新固件 - 校验内部flash失败 - 校验外部flash成功 -  搬运到内部 - 校验成功 - 跳转   √
*       无新固件 - 校验内部flash失败 - 校验外部flash失败 -  停留boot  √
*       无新固件 - 校验内部flash失败 - 校验外部flash成功 -  搬运到内部 - 校验失败 -  停留boot  √
*/
void SysFirmwareIap(void)
{
    static uint16_t onchip_verify_fault_count = 0;  /*片内校验失败次数*/

    switch(_geSysIapFsm)
    {
        case EN_RUNMODE_DET :  /* 上电运行类型判断 */
        {
            onchip_verify_fault_count = 0;
            _SysRunmodeDetect();
            if(_gstSysIapState.sys_runmode == EN_UPGRADE_POWER_ON)
            {
                _geSysIapFsm = EN_OFFCHIP_VERIFY;   /*有固件更新，先校验片外*/
            }
            else if(_gstSysIapState.sys_runmode == EN_ABEN_NORMAL_POWER_ON || _gstSysIapState.sys_runmode == EN_NORMAL_POWER_ON)
            {
                _geSysIapFsm = EN_ONCHIP_VERIFY;    /*无固件更新*/
            }
            break;
        }
        case EN_ONCHIP_VERIFY:    /* 片内程序校验(全文件) */
        {

            _OnchipAppVerify();
            if(_gstSysIapState.app_verify_onchip == EN_VERIFY_OK)
            {
                _geSysIapFsm = EN_JUMP2APP;     /*片内校验成功，直接跳转App*/
            }
            else
            {
                onchip_verify_fault_count++;
                if(onchip_verify_fault_count > 2)
                {
                    _geSysIapFsm = EN_FIRMWARE_UPGRADE;   /*2次校验失败，停留boot*/
                }
                else
                {
                    _geSysIapFsm = EN_OFFCHIP_VERIFY;     /*片内校验失败，校验片外*/
                }
            }
            break;
        }
        case EN_OFFCHIP_VERIFY:   /* 片外程序校验(全文件) */
        {
            _OffchipAppVerify();
            if(_gstSysIapState.app_verify_offchip == EN_VERIFY_ERROR)   /*如果片外校验失败了*/
            {
                _geSysIapFsm = EN_ONCHIP_VERIFY;    /*校验片内*/
            }
            else
            {
                _geSysIapFsm = EN_OFFCHIP2ONCHIP_COPY;
            }
            break;
        }
        case EN_VERIFY_RESULT:    /* 片内外校验结果处理 */
        {
            if((_gstSysIapState.app_verify_onchip == EN_VERIFY_OK) && (_gstSysIapState.app_verify_offchip == EN_VERIFY_OK))     /*片内外都校验成功*/
            {
                _geSysIapFsm = EN_JUMP2APP;
            }
            else if((_gstSysIapState.app_verify_onchip == EN_VERIFY_OK) && (_gstSysIapState.app_verify_offchip == EN_VERIFY_ERROR))     /*片内校验成功，片外校验失败*/
            {
                _geSysIapFsm = EN_JUMP2APP;
            }
            else if((_gstSysIapState.app_verify_onchip == EN_VERIFY_ERROR) && (_gstSysIapState.app_verify_offchip == EN_VERIFY_OK))     /*片内失败，片外成功*/
            {
                _geSysIapFsm = EN_OFFCHIP2ONCHIP_COPY;
            }
            else    /* 两份均校验失败 */
            {
                _geSysIapFsm = EN_FIRMWARE_UPGRADE;
            }
            break;
        }
        case EN_ONCHIP2OFFCHIP_COPY:  /* 将程序文件从片内拷贝到片外 */
        {
            _AppFileCopyProcess(ONCHIP_TO_OFFCHIP );
            if(_gstSysIapState.app_verify_onchip == EN_VERIFY_ERROR)
            {
                _geSysIapFsm = EN_FIRMWARE_UPGRADE;
            }
            else
            {
                _geSysIapFsm = EN_JUMP2APP;
            }
            break;
        }
        case EN_OFFCHIP2ONCHIP_COPY:  /* 将程序文件从片外拷贝到片内 */
        {
            InFlashInit();  /*dsp特殊处理，初始化片内flash 否则写不成功*/
            _AppFileCopyProcess(OFFCHIP_TO_ONCHIP);
            if(_gstSysIapState.app_verify_offchip == EN_VERIFY_ERROR)
            {
                _geSysIapFsm = EN_FIRMWARE_UPGRADE;
            }
            else
            {
                _geSysIapFsm = EN_ONCHIP_VERIFY;
            }
            break;
        }
        case EN_FIRMWARE_UPGRADE:     /* 固件升级处理 */
        {
            ModbusCmdProcess();
            UpgradeProcesshandle();
            break;
        }
        case EN_JUMP2APP:     /* 程序跳转处理 */
        {
            /* 固件更新完成，擦除更新标志 */
            if(_gstSysIapState.sys_runmode == EN_UPGRADE_POWER_ON)
            {
                memset(_garrTemp256, 0, 256);
                SpiFlashWriteBytesWithoutEraseNoAligned(UPGRADE_FLAG_ADDR, _garrTemp256, 256);
            }
            JumpToAppProcess();
            break;
        }
        default:
            break;
    }
}
void FirmwareCopy(void)
{
    _geSysIapFsm = EN_OFFCHIP2ONCHIP_COPY;
}

