
#include "boot.h"
#include "flash.h"
#include "firmware_upgrate.h"
#include "dev_key.h"
#include "timed_polling.h"


#define FW_HEAD_STAND_ALONE 1
#if FW_HEAD_STAND_ALONE 
#define APP_FIRMWARE_INTERNAL_ADDR           0x10000
#define APP_FIRMWARE_HEAD_INTERNAL_ADDR      0xFF00//0xDF00
#else
#define APP_FIRMWARE_INTERNAL_ADDR           0xC100
#define APP_FIRMWARE_HEAD_INTERNAL_ADDR      0xC000
#endif
static uint32_t _MainCtlDataOffset;
static uint32_t _MainCtlDataLen;
#define __ALIGNED(x)                           __attribute__((aligned(x)))
__ALIGNED(4) static uint8_t data_temp[256];

typedef void (*iapfun)();

iapfun jump2app;
void JumpToApp()
{
    SysTick->CTRL &= (~SysTick_CTRL_ENABLE_Msk);
//    INTX_DISABLE();
    __set_PRIMASK(1);   // 关闭总中断
    jump2app = (iapfun)*(uint32_t*)(FMC_START_ADDR + APP_FIRMWARE_INTERNAL_ADDR + 4);
    __set_PSP(*(__IO uint32_t *)FMC_START_ADDR + APP_FIRMWARE_INTERNAL_ADDR);
    __set_CONTROL(0);
    __set_MSP(*(__IO uint32_t *)FMC_START_ADDR + APP_FIRMWARE_INTERNAL_ADDR);
    jump2app();
}
/*判断外部FLASH是否有主控固件*/
static bool _JudgeNewMainCtlFirmware()
{
    uint16_t i;
    uint32_t file_size;
    uint32_t offset = 0;
    uint32_t main_ctl_offset;
    uint32_t sub_len;
    
    /*先判断外部FLASH是否有更新的固件*/
    SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR, data_temp, 256); 
    for(i = 0; i < 256; i++)
    {
        if(data_temp[i] != 0x0A)
        {
            break;
        }
    }
    if(i != 256)
    {
        return false;
    }
    
    /*判断新固件是否包含主控部分*/
    SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR, data_temp, 256);
    file_size = (data_temp[4] | (data_temp[5] << 8) | (data_temp[6] << 16) | (data_temp[7] << 24));
    
    offset += 256;
    while(offset < file_size - 4)//-4是去除总和校验
    {
        SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + offset, data_temp, 256);
        if(data_temp[0] != 0x0C || data_temp[1] != 0x0C || data_temp[2] != 0x0C)
        {
            return false;
        }
        if(data_temp[3] == 0x00)/*主控*/
        {
            _MainCtlDataOffset = offset;
            _MainCtlDataLen = (data_temp[7] | (data_temp[8] << 8) | (data_temp[9] << 16) | (data_temp[10] << 24));
            return true;
        }
        else
        {
            offset += (data_temp[7] | (data_temp[8] << 8) | (data_temp[9] << 16) | (data_temp[10] << 24));
        }            
    }
    
    return false;
}

void JudgeIfBootMode()
{
    uint8_t internal_fw_valid_flag = 0;
    uint16_t i = 0;
    static uint8_t  filter_cnt=0;
    static uint8_t  filter_cnt1=0;
    static uint32_t time_out = 0;
//此处延迟20ms 
//        JumpToApp();
//        while(1);
    while(1)
    {
        if((uint32_t)(tim_get_systime() - time_out) > 100)
        {
            time_out = tim_get_systime();
            if(GetBootKeyData())
            {
                filter_cnt1 = 0;
                filter_cnt++;
                if(filter_cnt >10)
                {
                    filter_cnt = 0;
                    return;  //停留在BOOT
                }
            }
            else
            {
                filter_cnt = 0;
                if(filter_cnt1++ >10)
                {
                    filter_cnt1 = 0;
                    break;
                }
                
            }
        }   
    }
    /*后续增加通过按键强制停留在boot*/
    /**/
    
    if(_JudgeNewMainCtlFirmware() == true)/*外部FLASH有更新的主控固件*/
    {
        uint32_t seek = 0;
        uint32_t len;
        
//        LOG_DEBUG("BOOT", "There is new fw in external flash\r\n");
        //1、搬运
        /*先搬运数据部分*/
        //因为顺序问题，先擦除第一个页
        InternalFlashErasePage(APP_FIRMWARE_HEAD_INTERNAL_ADDR);
        seek = 256;
        len = _MainCtlDataLen - 256;
        while(len > 256)
        {
            SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + _MainCtlDataOffset + seek, data_temp, 256);
            InternalFlashWordWrite(APP_FIRMWARE_HEAD_INTERNAL_ADDR + seek, (uint32_t *)data_temp, 256);
            len -= 256;
            seek += 256;
        }
        SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + _MainCtlDataOffset + seek, data_temp, len);
        InternalFlashWordWrite(APP_FIRMWARE_HEAD_INTERNAL_ADDR + seek, (uint32_t *)data_temp, len);
        seek += len;
        len = 0;
        
        /*再搬运固件头部分*/
        SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + _MainCtlDataOffset, data_temp, 256);
        InternalFlashWordWriteWithoutErase(APP_FIRMWARE_HEAD_INTERNAL_ADDR, (uint32_t *)data_temp, 256);
        
        //2、清外部标志
        memset(data_temp, 0x00, 256);
        SpiFlashWriteBytesWithoutErase(gFirmwareUpgrateInfo.write_addr, data_temp, 256);
        //3、重启
        
//        LOG_DEBUG("BOOT", "fw update success!\r\n");
        __set_FAULTMASK(1);
        NVIC_SystemReset();
        
        while(1);
    }
    
//    LOG_DEBUG("BOOT", "No fw in external flash\r\n");
//    JumpToApp();
    /*外部没有固件，检查内部是否有固件*/
    InternalFlashWordRead(APP_FIRMWARE_HEAD_INTERNAL_ADDR, (uint32_t *)data_temp, 256);
    if(data_temp[0] != 0x0C || data_temp[1] != 0x0C || data_temp[2] != 0x0C || data_temp[3] != 0x00)
    {
        internal_fw_valid_flag = 0;
    }
    else
    {
        internal_fw_valid_flag = 1;
    }
    
    
    if(internal_fw_valid_flag == 1)
    {

 
//        __set_PRIMASK(1);   // 关闭总中断
//将所有的中断DISABLE

//       TimePollingDeint();
        JumpToApp();
        while(1);
    }
//    LOG_DEBUG("BOOT", "No APP\r\n");
    
}






























