

#include "boot.h"

#define APP_FIRMWARE_INTERNAL_ADDR              0x8000
#define APP_FIRMWARE_HEAD_INTERNAL_ADDR         0x7800 //7F00修改为7800 内部flash擦除的方式是按页擦除 每页2K
#define APP_FIRMWARE_REAL_HEAD_INTERNAL_ADDR    0x7F00

#define  FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR       0x040000
#define  FIRMWARE_EXTERNAL_FLASH_ADDR            0x041000


__ALIGNED(4) static uint8_t _data_temp[256];

static uint32_t _FWDataOffset;
static uint32_t _FWDataLen;

 
typedef void (*iapfun)();

iapfun jump2app;
void JumpToApp()
{
    SysTick->CTRL &= (~SysTick_CTRL_ENABLE_Msk);
//    INTX_DISABLE();
    __set_PRIMASK(1);   // 关闭总中断
//    GPIO_DeInit(GPIOA);
//    GPIO_DeInit(GPIOB);
//    GPIO_DeInit(GPIOC);
//    GPIO_DeInit(GPIOD);
//    GPIO_DeInit(GPIOE);
    jump2app = (iapfun)*(uint32_t*)(FMC_START_ADDR + APP_FIRMWARE_INTERNAL_ADDR + 4);
    __set_PSP(*(__IO uint32_t *)FMC_START_ADDR + APP_FIRMWARE_INTERNAL_ADDR);
    __set_CONTROL(0);
    __set_MSP(*(__IO uint32_t *)FMC_START_ADDR + APP_FIRMWARE_INTERNAL_ADDR);
    jump2app();
}




/*判内部FLASH app-b是否有采集器固件*/
static EN_FW_EXT_STATUS _JudgeNewFirmware()
{
    uint16_t i;
    uint32_t file_size;
    uint32_t offset = 0;
    uint32_t fw_offset;
    uint32_t sub_len;
    
    EN_FW_EXT_STATUS ret = EN_FW_EXT_STATUS_NON_EXIST;
    
    /*先判断内部FLASH  APP-B是否有更新的固件*/
    InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR, _data_temp, 256);
    
    for(i = 0; i < 256; i++) 
    {
        if((_data_temp[i] != 0x0A) && (_data_temp[i] != 0x00)) 
        {
            ret = EN_FW_EXT_STATUS_NON_EXIST;
            return ret;
        }
    }
    
    
    for(i = 0; i < 256; i++) 
    {
        if(_data_temp[i] == 0x00)
        {
            ret = EN_FW_EXT_STATUS_EXIST_OLD;
            return ret;
        }
    }
    

    /*判断新固件是否包含主控部分*/
    InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_ADDR, _data_temp, 256);
    file_size = (_data_temp[4] | (_data_temp[5] << 8) | (_data_temp[6] << 16) | (_data_temp[7] << 24));
    

    InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_ADDR + offset, _data_temp, 256);
    if(_data_temp[0] != 0x0C || _data_temp[1] != 0x0C || _data_temp[2] != 0x0C)
    {
        ret = EN_FW_EXT_STATUS_NON_EXIST;
        return ret;
    }
    if(_data_temp[3] == 0x80)/*协处理器*/
    {
        _FWDataOffset = offset;
        _FWDataLen = (_data_temp[7] | (_data_temp[8] << 8) | (_data_temp[9] << 16) | (_data_temp[10] << 24));
        ret = EN_FW_EXT_STATUS_EXIST_NEW;
    }
    return ret;
}


void JudgeIfBootMode()
{
#if 1
    uint8_t internal_fw_valid_flag = 0;

    if(_JudgeNewFirmware() == EN_FW_EXT_STATUS_EXIST_NEW)/*内部FLASH app-b有更新的主控固件*/
    {
        uint32_t seek = 0;
        uint32_t len;
        
        /*先擦除固件头所在页*/
        InternalFlashErasePage(APP_FIRMWARE_HEAD_INTERNAL_ADDR);
        
        /*再擦写固件数据*/
        seek = 256;
        len = _FWDataLen - 256;
        while(len > 256)
        {
            InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_ADDR + seek, _data_temp, 256);
            InternalFlashWrite(APP_FIRMWARE_REAL_HEAD_INTERNAL_ADDR + seek, (uint32_t *)_data_temp, 256);
            len -= 256;
            seek += 256;
        }
        InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_ADDR + seek, _data_temp, len);
        InternalFlashWrite(APP_FIRMWARE_REAL_HEAD_INTERNAL_ADDR + seek, (uint32_t *)_data_temp, len);
        seek += len;
        len = 0;
        /*再写入固件头*/
        InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_ADDR, _data_temp, 256);
        InternalFlashWriteWithoutErase(APP_FIRMWARE_REAL_HEAD_INTERNAL_ADDR, (uint32_t *)_data_temp, 256);

        /*先擦除升级标志所在页*/
        InternalFlashErasePage(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR);
        /*清外部FLASH固件有效标志，改为0x00,表示固件已使用*/
        memset(_data_temp, 0x00, 256);
        InternalFlashWriteWithoutErase(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR, (uint32_t*)_data_temp, 256);
        
        /*重启*/
        __set_FAULTMASK(1);
        NVIC_SystemReset();
        
        while(1);
    }
    else
    {
//        JumpToApp();
        /*外部没有固件，检查内部是否有固件*/
        InternalFlashReadWord(APP_FIRMWARE_REAL_HEAD_INTERNAL_ADDR, (uint32_t *)_data_temp, 256);
        if(_data_temp[0] != 0x0C || _data_temp[1] != 0x0C || _data_temp[2] != 0x0C || _data_temp[3] != 0x80)
        {
            internal_fw_valid_flag = 0;
        }
        else 
        {
            internal_fw_valid_flag = 1;
        }
        
        if(internal_fw_valid_flag == 1)
        {
            JumpToApp();
            while(1);
        }
    }
#endif
}



void ExternalFWForceTrans()
{
    #if 0
    if(_JudgeNewFirmware() == EN_FW_EXT_STATUS_EXIST_OLD)/*外部FLASH有已更新过的主控固件*/
    {
        uint32_t seek = 0;
        uint32_t len;
        
        /*先擦除固件头所在页*/
        InternalFlashErasePage(APP_FIRMWARE_HEAD_INTERNAL_ADDR);
        /*再擦写固件数据*/
        seek = 256;
        len = _FWDataLen - 256;
        while(len > 256)
        {
            SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + _FWDataOffset + seek, _data_temp, 256);
            InternalFlashWrite(APP_FIRMWARE_HEAD_INTERNAL_ADDR + seek, (uint32_t *)_data_temp, 256);
            len -= 256;
            seek += 256;
        }
        SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + _FWDataOffset + seek, _data_temp, len);
        InternalFlashWrite(APP_FIRMWARE_HEAD_INTERNAL_ADDR + seek, (uint32_t *)_data_temp, len);
        seek += len;
        len = 0;
        /*再写入固件头*/
        SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + _FWDataOffset, _data_temp, 256);
        InternalFlashWriteWithoutErase(APP_FIRMWARE_HEAD_INTERNAL_ADDR, (uint32_t *)_data_temp, 256);
        
        /*重启*/
        __set_FAULTMASK(1);
        NVIC_SystemReset();
    }
    #endif
}


