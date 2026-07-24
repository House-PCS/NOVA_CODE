
#include "boot.h"

#define APP_FIRMWARE_INTERNAL_ADDR           0x7F00
#define APP_FIRMWARE_INTERNAL_ADDR1           0x8000
#define APP_FIRMWARE_HEAD_INTERNAL_ADDR       0x7F00

#define  FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR       0x00040000
#define  FIRMWARE_EXTERNAL_FLASH_ADDR            0x00041000


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
    jump2app = (iapfun)*(uint32_t*)(FMC_START_ADDR + APP_FIRMWARE_INTERNAL_ADDR1 + 4);
    __set_PSP(*(__IO uint32_t *)FMC_START_ADDR + APP_FIRMWARE_INTERNAL_ADDR1);
    __set_CONTROL(0);
    __set_MSP(*(__IO uint32_t *)FMC_START_ADDR + APP_FIRMWARE_INTERNAL_ADDR1);
    jump2app();
}




/*判断外部FLASH是否有采集器固件*/
static EN_FW_EXT_STATUS _JudgeNewFirmware()
{
    uint16_t i;
    uint32_t file_size;
    uint32_t offset = 0;
    uint32_t fw_offset;
    uint32_t sub_len;
    
    EN_FW_EXT_STATUS ret = EN_FW_EXT_STATUS_NON_EXIST;
    
    /*先判断内部FLASH -app-b是否有更新的固件*/
	InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR, _data_temp, 256);   
    for(i = 0; i < 256; i++) 
    {
        if((_data_temp[i] != 0xA0) && (_data_temp[i] != 0x00)) 
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
	
    /*判断新固件是否包含拉弧检测部分*/
	InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_ADDR, _data_temp, 256);
	
//    file_size = (_data_temp[7] | (_data_temp[8] << 8) | (_data_temp[9] << 16) | (_data_temp[10] << 24));
//    
//    offset += 256;
//    while(offset < file_size - 4)//-4是去除总和校验
//    {
//		InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR + offset, _data_temp, 256);
//        SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + offset, _data_temp, 256);
        if(_data_temp[0] != 0x0C || _data_temp[1] != 0x0C || _data_temp[2] != 0x0C)
        {
            ret = EN_FW_EXT_STATUS_NON_EXIST;
            return ret;
        }
        if(_data_temp[3] == 0x02) /*拉弧检测*/
        {
            _FWDataOffset = offset;
            _FWDataLen = (_data_temp[7] | (_data_temp[8] << 8) | (_data_temp[9] << 16) | (_data_temp[10] << 24));
            ret = EN_FW_EXT_STATUS_EXIST_NEW;
        }
//        else
//        {
//            offset += (_data_temp[7] | (_data_temp[8] << 8) | (_data_temp[9] << 16) | (_data_temp[10] << 24));   //重复判断包里是否含有自己的包
//        }
			return ret;		
}
//		/*判断新固件是否包含主控部分*/
//		InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_ADDR, _data_temp, 256);
//		file_size = (_data_temp[4] | (_data_temp[5] << 8) | (_data_temp[6] << 16) | (_data_temp[7] << 24));
//		offset += 256;
//		while(offset < file_size - 4)//-4是去除总和校验
//		{
//			
//			InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_ADDR + offset, _data_temp, 256);
//			if(_data_temp[0] != 0x0B || _data_temp[1] != 0x0B || _data_temp[2] != 0x0B)
//			{
//				ret = EN_FW_EXT_STATUS_NON_EXIST;
//				return ret;
//			}
//			if(_data_temp[3] == 0x05) /*拉弧检测*/
//			{
//				_FWDataOffset = offset;
//				_FWDataLen = (_data_temp[7] | (_data_temp[8] << 8) | (_data_temp[9] << 16) | (_data_temp[10] << 24));
//				ret = EN_FW_EXT_STATUS_EXIST_NEW;
//				break;
//			}
//        else
//        {
//            offset += (_data_temp[7] | (_data_temp[8] << 8) | (_data_temp[9] << 16) | (_data_temp[10] << 24));
//        }            
//    }
//    
//    
//    return ret;
//}  
    
//    return ret;
//}


void JudgeIfBootMode()
{
    uint8_t internal_fw_valid_flag = 0;
    
    /*检测按键是否按下，按下则停留在boot*/
//    if(GpioBitGet(&KeyRstHandle) == 0)
//    {
//        return;
//    }
    
    if(_JudgeNewFirmware() == EN_FW_EXT_STATUS_EXIST_NEW)/*内部FLASH有更新的固件*/
    {
        uint32_t seek = 0;
        uint32_t len;		
		/*先擦除固件头所在页*/
        InternalFlashErasePage(APP_FIRMWARE_HEAD_INTERNAL_ADDR);
        /*再擦写固件数据*/
        seek = 256;
        len = _FWDataLen ;
        while(len > 256)
        {
			InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_ADDR + seek, _data_temp, 256);            
            InternalFlashWrite(APP_FIRMWARE_INTERNAL_ADDR+ seek, (uint32_t *)_data_temp, 256);  
            len -= 256;
            seek += 256;
        }
		InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_ADDR + seek , _data_temp, len);                
        InternalFlashWrite(APP_FIRMWARE_INTERNAL_ADDR + seek, (uint32_t *)_data_temp, len);      
        seek += len;
        len = 0;
        /*再写入固件头*/
		InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_ADDR, _data_temp, 256);
        InternalFlashWriteWithoutErase(APP_FIRMWARE_HEAD_INTERNAL_ADDR, (uint32_t *)_data_temp, 256);
        
        /*清外部FLASH固件有效标志，改为0x00,表示固件已使用*/
        memset(_data_temp, 0x00, 256);
		InternalFlashWrite(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR, (uint32_t *)_data_temp, 256);

        
        /*重启*/
        __set_FAULTMASK(1);
        NVIC_SystemReset();
        
        while(1);
    }
    else
    {
//        JumpToApp();
        /*外部没有新固件，检查内部是否有固件*/
        InternalFlashReadWord(APP_FIRMWARE_HEAD_INTERNAL_ADDR, (uint32_t *)_data_temp, 256);
        if(_data_temp[0] != 0x0C || _data_temp[1] != 0x0C || _data_temp[2] != 0x0C || _data_temp[3] != 0x02)
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
}



//void ExternalFWForceTrans()
//{
//    if(_JudgeNewFirmware() == EN_FW_EXT_STATUS_EXIST_OLD)/*外部FLASH有已更新过的主控固件*/
//    {
//        uint32_t seek = 0;
//        uint32_t len;
//        
//        /*先擦除固件头所在页*/
//        InternalFlashErasePage(APP_FIRMWARE_HEAD_INTERNAL_ADDR);
//        /*再擦写固件数据*/
//        seek = 256;
//        len = _FWDataLen - 256;
//        while(len > 256)
//        {
//            SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + _FWDataOffset + seek, _data_temp, 256);
//            InternalFlashWrite(APP_FIRMWARE_HEAD_INTERNAL_ADDR + seek, (uint32_t *)_data_temp, 256);
//            len -= 256;
//            seek += 256;
//        }
//        SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + _FWDataOffset + seek, _data_temp, len);
//        InternalFlashWrite(APP_FIRMWARE_HEAD_INTERNAL_ADDR + seek, (uint32_t *)_data_temp, len);
//        seek += len;
//        len = 0;
//        /*再写入固件头*/
//        SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + _FWDataOffset, _data_temp, 256);
//        InternalFlashWriteWithoutErase(APP_FIRMWARE_HEAD_INTERNAL_ADDR, (uint32_t *)_data_temp, 256);
//        
//        /*重启*/
//        __set_FAULTMASK(1);
//        NVIC_SystemReset();
//    }
//}
















