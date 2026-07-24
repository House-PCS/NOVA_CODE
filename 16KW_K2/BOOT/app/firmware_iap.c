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
#include "firmware_upgrate.h"
#include "F28x_Project.h"
#include "gpio.h"

/* Private define ------------------------------------------------------------*/
#define BOOT_RS485_DE_GPIO      69u
#define BOOT_ISO485_DE_GPIO     44u
#define BOOT_EPWM_SOFTPRES_MASK 0x00000FFFu
#define BOOT_SCI_SOFTPRES_MASK  0x0000000Fu
#define BOOT_SPI_SOFTPRES_MASK  0x00000007u
#define BMS_FIRMWARE_ADDR   0x00001000
#define UPGRADE_FLAG_ADDR   0x00000000

#define     ONCHIP_TO_OFFCHIP       0x00
#define     OFFCHIP_TO_ONCHIP       0x01
#define     EN_DOC_MCU              0x01
//uint16_t gPCSFirmwareType[4] = {0x0A, 0x0A, 0x0A, 0x03};
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

void ArrU8ToU16(uint16_t* u16_arr, uint16_t* u8_arr, uint16_t u8_arr_len)
{
    uint16_t i = 0;
    uint16_t data_len = 0;

    data_len = (u8_arr_len >> 1) * 2;

    for(i = 0; i < data_len; i ++)
    {
        *(u16_arr + i) = *(u8_arr + 2 * i) + (*(u8_arr + 2 * i + 1) << 8);
    }
}

void JumpFlagWrite(void)
{
    uint16_t firmware_upgrade[256] = {0};

//    memset(firmware_upgrade, 0xA0, 256);
//    SpiFlashWriteBytesWithErase(UPGRADE_FLAG_ADDR, firmware_upgrade, 256);
    *firmware_upgrade = 0x0A;
    *(firmware_upgrade + 1) = 0x0A;
    *(firmware_upgrade + 2) = 0x0A;
    *(firmware_upgrade + 3) = 0x03;
    InFlashWrite(ONCHIP_APP_BASE_ADDR, firmware_upgrade, 8);

    jumptoappprocess();
}

/**
  * @brief  系统运行模式判断
  * @param  none
  * @return 系统运行模式
  */
static void sys_runmode_detect(void)
{
    uint16_t firmware_upgrade[256] = {0};
    uint16_t i = 0;

    SpiFlashReadBytes(UPGRADE_FLAG_ADDR, firmware_upgrade, 256);

    for(i = 0; i < 256; i ++)
    {
        if(firmware_upgrade[i] != 0xA0)
        {
            break;
        }
    }
    if(i == 256)
    {
//        printf("There has new firmware!!");
        _sys_IAP_state.sys_runmode = UPGRADE_POWER_ON;
    }
    else
    {
        _sys_IAP_state.sys_runmode = NORMAL_POWER_ON;
    }
}

/**
  * @brief  片上APP程序校验
  * @param  none
  * @return none
  */
static void onchip_app_verify(void)
{
    uint32_t tmp_byte_cnt __attribute((unused));
    uint32_t tmp_file_len __attribute((unused));
    uint32_t tmp_app_sum __attribute((unused));
    uint32_t tmp_sum __attribute((unused));
    uint16_t mode_Id __attribute((unused));
    uint16_t * tmp_onchip_ptr = (uint16_t *)ONCHIP_APP_BASE_ADDR;

    if(memcmp(tmp_onchip_ptr, garrDspFirmwareType, 4) != 0)
    {
        _sys_IAP_state.app_verify_onchip = VERIFY_ERROR;
        return;
    }
    else
    {
        _sys_IAP_state.app_verify_onchip = VERIFY_OK;
        return;
    }

    /* 获取文件长度和校验和 */
//    tmp_file_len = (uint32_t)(*(tmp_onchip_ptr + 7)) + (((uint32_t)(*(tmp_onchip_ptr + 8))) << 8) + \
//                   (((uint32_t)(*(tmp_onchip_ptr + 9))) << 16) + (((uint32_t)(*(tmp_onchip_ptr + 10))) << 24);
//
//    tmp_app_sum = (uint32_t)(*(tmp_onchip_ptr + 252)) + (((uint32_t)(*(tmp_onchip_ptr + 253))) << 8) + \
//                       (((uint32_t)(*(tmp_onchip_ptr + 254))) << 16) + (((uint32_t)(*(tmp_onchip_ptr + 255))) << 24);
//
//    if((tmp_file_len == 0) || (tmp_file_len  > FIRMWARE_SIZE_MAX)) /*文件大小是否合理 */
//    {
//        _sys_IAP_state.app_verify_onchip = VERIFY_ERROR;
//        return;
//    }
//
//    _sys_IAP_state.app_onchip_size = tmp_file_len;  /* 记录文件大小 */
//
//    /* 全文件校验和计算 */
//    tmp_onchip_ptr = (uint16_t *)(ONCHIP_APP_BASE_ADDR + FILE_HEADER_SIZE);
//    tmp_sum = 0;
//    for(tmp_byte_cnt = 0; tmp_byte_cnt < tmp_file_len - FILE_HEADER_SIZE; tmp_byte_cnt++)
//    {
//        tmp_sum += *(tmp_onchip_ptr + tmp_byte_cnt);
//        if(tmp_sum == 0x98216b || tmp_sum == 0x982450)
//        {
//
//        }
//    }
////    tmp_sum += VERIFY_CONST_VAL;    /* 叠加最后的常量 */
//    if(tmp_sum == tmp_app_sum)
//    {
//        _sys_IAP_state.app_verify_onchip = VERIFY_OK;
//    #if  IAP_DEBUG_MODE
//            uart_sendstring(EN_UART3_BUS, "BOOT: ON VERIFY_OK;\r\n");
//    #endif
//    }
//    else
//    {
//        _sys_IAP_state.app_verify_onchip = VERIFY_ERROR;
//    #if  IAP_DEBUG_MODE
//            uart_sendstring(EN_UART3_BUS, "BOOT: ON VERIFY_ERROR;\r\n");
//    #endif
//    }
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
    uint16_t  tmp_buf[FLASH_BLOCK_SIZE] = {0};

    /* 获取文件长度和校验和 */
    SpiFlashReadBytes(BMS_FIRMWARE_ADDR, tmp_buf, 0x100);
//    tmp_file_len = *((uint32_t*)&tmp_buf[7]);
    tmp_file_len = (uint32_t)(*(tmp_buf + 7)) + (((uint32_t)(*(tmp_buf + 8))) << 8) + \
                   (((uint32_t)(*(tmp_buf + 9))) << 16) + (((uint32_t)(*(tmp_buf + 10))) << 24);

//    tmp_app_sum = *((uint32_t*)&tmp_buf[252]);
    tmp_app_sum = (uint32_t)(*(tmp_buf + 252)) + (((uint32_t)(*(tmp_buf + 253))) << 8) + \
                  (((uint32_t)(*(tmp_buf + 254))) << 16) + (((uint32_t)(*(tmp_buf + 255))) << 24);

    if(!tmp_file_len || tmp_file_len > FIRMWARE_SIZE_MAX) /*文件大小是否合理 */
    {
        _sys_IAP_state.app_verify_offchip = VERIFY_ERROR;
        return;
    }

    if(memcmp(tmp_buf, garrDspFirmwareType, 4) != 0)
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
        SpiFlashReadBytes(BMS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, tmp_buf, FLASH_BLOCK_SIZE);
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
        SpiFlashReadBytes(BMS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, tmp_buf, FLASH_BLOCK_SIZE);
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
//    uint8_t * tmp_onchip_ptr = (uint8_t *)ONCHIP_APP_BASE_ADDR;
//    uint32_t  tmp_onchip_addr = ONCHIP_APP_BASE_ADDR;
//    uint32_t* tmp_onchip_uint32_tptr = &tmp_onchip_addr;
    uint32_t  tmp_block_cnt = 0;
    uint32_t  tmp_byte_cnt __attribute((unused));
    uint32_t  tmp_block_num = 0;
    uint32_t  tmp_byte_remainder = 0;
    uint32_t  tmp_file_len = 0;
    uint16_t  tmp_buf[512];
    uint16_t  tmp_buf_u16[256];

    switch(direction)
    {
        case 0x00:  /* 片内拷贝到片外 */
        {
            /* 获取文件长度和校验和 */
//            _sys_IAP_state.app_onchip_size = *((uint32_t*)(tmp_onchip_ptr + 7));
//            tmp_file_len = _sys_IAP_state.app_onchip_size;
//            if(!(tmp_file_len - FILE_HEADER_SIZE) || tmp_file_len > (128 *1024)) /*文件大小是否合理 */
//            {
//                _sys_IAP_state.app_verify_onchip = VERIFY_ERROR;
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
//                SpiFlashWriteBytesWithErase(BMS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, tmp_onchip_ptr, FLASH_BLOCK_SIZE);
//                tmp_onchip_ptr += FLASH_BLOCK_SIZE;
//            }
        }
        break;

        case 0x01:  /* 片外拷贝到片内 */
        {
//          SciaMsg("move\r\n");
            /* 获取文件长度和校验和 */
            SpiFlashReadBytes(BMS_FIRMWARE_ADDR, tmp_buf, FILE_HEADER_SIZE);

            _sys_IAP_state.app_offchip_size = (uint32_t)tmp_buf[7] + ((uint32_t)tmp_buf[8] << 8) + \
                                              ((uint32_t)tmp_buf[9] << 16) + ((uint32_t)tmp_buf[10] << 24);
            tmp_file_len = _sys_IAP_state.app_offchip_size - FILE_HEADER_SIZE;
            if(!(tmp_file_len - FILE_HEADER_SIZE) || tmp_file_len > FIRMWARE_SIZE_MAX) /*文件大小是否合理 */
            {
                _sys_IAP_state.app_verify_offchip = VERIFY_ERROR;
                return;
            }

            tmp_block_num =  tmp_file_len / FLASH_BLOCK_SIZE;
//            tmp_byte_remainder = (tmp_file_len + 15) % FLASH_BLOCK_SIZE;
            if(tmp_file_len % FLASH_BLOCK_SIZE)
            {
                tmp_block_num += 1;
            }

            /* erase user flash area */
            InFlashErase();
//            SpiFlashReadBytes(BMS_FIRMWARE_ADDR, (uint8_t*)tmp_buf, FILE_HEADER_SIZE);
            InFlashWrite(ONCHIP_APP_BASE_ADDR, tmp_buf, FILE_HEADER_SIZE);

            for(tmp_block_cnt = 0; tmp_block_cnt < tmp_block_num; tmp_block_cnt++)
            {
                SpiFlashReadBytes(FILE_HEADER_SIZE + BMS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, tmp_buf, FLASH_BLOCK_SIZE);
                ArrU8ToU16(tmp_buf_u16, tmp_buf, FLASH_BLOCK_SIZE);
                InFlashWrite(USER_FLASH_START_ADDRESS + tmp_block_cnt * (FLASH_BLOCK_SIZE >> 1), tmp_buf_u16, (FLASH_BLOCK_SIZE >> 1));
//                InFlashWrite(ONCHIP_APP_BASE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, tmp_buf, FLASH_BLOCK_SIZE);
            }
            DELAY_MS(10);
//            SpiFlashReadBytes(FILE_HEADER_SIZE + BMS_FIRMWARE_ADDR + tmp_block_cnt * FLASH_BLOCK_SIZE, tmp_buf, tmp_byte_remainder);
//            ArrU8ToU16(tmp_buf_u16, tmp_buf, tmp_byte_remainder);
//            InFlashWrite(USER_FLASH_START_ADDRESS + tmp_block_cnt * (FLASH_BLOCK_SIZE >> 1), tmp_buf_u16, (tmp_byte_remainder >> 1));
//            SciaMsg("New Firmware Move Success！\r\n");
        }
        break;

        default :
            break;
    }
}

/**
  * @brief  停止 SCI 并进入软件复位态
  */
static void Boot_SciShutdown(volatile struct SCI_REGS *sci)
{
    sci->SCICTL2.bit.TXINTENA = 0;
    sci->SCICTL2.bit.RXBKINTENA = 0;
    sci->SCIFFTX.bit.TXFIFORESET = 0;
    sci->SCIFFRX.bit.RXFIFORESET = 0;
    sci->SCICTL1.bit.SWRESET = 0;
}

/**
  * @brief  停止 SPI 并进入软件复位态
  */
static void Boot_SpiShutdown(volatile struct SPI_REGS *spi)
{
    spi->SPICTL.bit.SPIINTENA = 0;
    spi->SPICTL.bit.TALK = 0;
    spi->SPICCR.bit.SPISWRESET = 0;
}

/**
  * @brief  停止 ePWM 输出并冻结时基
  */
static void Boot_StopEpwm(void)
{
    Uint16 i;
    volatile struct EPWM_REGS * const epwm_list[] =
    {
        &EPwm1Regs,  &EPwm2Regs,  &EPwm3Regs,  &EPwm4Regs,
        &EPwm5Regs,  &EPwm6Regs,  &EPwm7Regs,  &EPwm8Regs,
        &EPwm9Regs,  &EPwm10Regs, &EPwm11Regs, &EPwm12Regs
    };

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    for(i = 0; i < 12u; i++)
    {
        epwm_list[i]->TZFRC.bit.OST = 1;
    }
    EDIS;
}

/**
  * @brief  对 Boot 使用的外设做软复位脉冲
  */
static void Boot_PeriphSoftReset(void)
{
    EALLOW;
    DevCfgRegs.SOFTPRES2.all = BOOT_EPWM_SOFTPRES_MASK;
    DevCfgRegs.SOFTPRES7.all = BOOT_SCI_SOFTPRES_MASK;
    DevCfgRegs.SOFTPRES8.all = BOOT_SPI_SOFTPRES_MASK;
    asm(" RPT #7 || NOP");
    DevCfgRegs.SOFTPRES2.all = 0;
    DevCfgRegs.SOFTPRES7.all = 0;
    DevCfgRegs.SOFTPRES8.all = 0;
    EDIS;
}

/**
  * @brief  跳转 APP 前清理中断、外设与时钟相关状态
  */
static void Boot_PreJumpCleanup(void)
{
    DINT;
    IER = 0;
    IFR = 0;
    InitPieCtrl();
    PieCtrlRegs.PIEACK.all = 0xFFFF;

    StopCpuTimer0();
    StopCpuTimer1();
    StopCpuTimer2();

    Boot_StopEpwm();

    Boot_SciShutdown(&ScibRegs);
    Boot_SciShutdown(&ScicRegs);
    Boot_SciShutdown(&ScidRegs);

    Boot_SpiShutdown(&SpiaRegs);
    Boot_SpiShutdown(&SpibRegs);

    GPIO_WritePin(BOOT_RS485_DE_GPIO, 0);
    GPIO_WritePin(BOOT_ISO485_DE_GPIO, 0);
    FlashCsGpioHigh();

    while(Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
    {
    }

    Boot_PeriphSoftReset();
    DisableDog();
}

void  jumptoappprocess(void)
{
    Boot_PreJumpCleanup();
    APP_JUMP;
}

/**
  * @brief  判断系统是否要进入BOOT模式，等待程序更新
  * @param  none
  * @return none
  */
void  enterbootmodejudge(void)
{
    //CheckBootKey();
    while(_sys_IAP_fsm != FIRMWARE_UPGRADE)
    {
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
//                SciaMsg("check\r\n");
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
//            _sys_IAP_fsm = OFFCHIP_VERIFY;
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
//            SciaMsg("copy\r\n");
//            DisableDog();
            InFlashInit();
            app_filecopy_process(OFFCHIP_TO_ONCHIP);
            if(_sys_IAP_state.app_verify_offchip == VERIFY_ERROR)
            {
                _sys_IAP_fsm = FIRMWARE_UPGRADE;
            }
            else
            {
                /* 固件更新完成，擦除更新标志 */
                SpiFlashAutoEraseSector(UPGRADE_FLAG_ADDR, 256);
                _sys_IAP_fsm = JUMP2APP;
//                LOG_INFO(LOG_TAG, "OFFCHIP2ONCHIP_COPY  VERIFY_Success");
            }
//            EnableDog();
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
//            SciaMsg("jump\r\n");
            jumptoappprocess();
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

