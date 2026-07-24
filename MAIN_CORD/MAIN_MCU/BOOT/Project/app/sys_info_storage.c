#include "sys_info_storage.h"
#include "slave_collect.h"

volatile const tuSoftwareVer SoftwareVer = {{1,3,4}}; 
volatile const char HwVer[2]= {'0','0'};
volatile const char hwver[2] ="11";
volatile const char EquipmentId[20] = "MA-P10";//"AMP-10";

/*ModelID和Sn号写入FLASH*/
void DeviceBaseInfoWrite(void)
{
    uint8_t buf[256];
    uint8_t valid_flag[4];
    uint8_t i = 0;

    for(i = 0; i < 4; i++)
    {
        valid_flag[i] = 0x0A;
    }

    for(i = 0; i < 2; i++)//第二次循环是数据备份
    {
        SpiFlashAutoEraseSectors(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i, 4096);
        memcpy(buf, (uint8_t *)&gCollectParaData.collect_data.mode_id, 2);
        SpiFlashWriteBytesWithoutErase(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE, buf, 2);//MODELID

        memcpy(buf, (uint8_t *)gCollectParaData.collect_data.sn, sizeof(gCollectParaData.collect_data.sn));
        SpiFlashWriteBytesWithoutErase(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE * 2, buf, sizeof(gCollectParaData.collect_data.sn));//SN
        SpiFlashWriteBytesWithoutErase(MAIN_CTL_FACTORY_INFO_FLASH_ADDR, valid_flag, 4);
    }
}


/*ModelID和Sn号从FLASH读出*/
void DeviceBaseInfoRead()
{
    uint8_t buf[256];
    uint8_t i = 0;

    for(i = 0; i < 2; i++)//第二次循环是数据备份
    {
        SpiFlashReadData(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i , buf, 4);
        if(buf[0] == 0x0A && buf[1] == 0x0A && buf[2] == 0x0A && buf[3] == 0x0A)
        {
            break;
        }
    }
    if(i == 2)/*数据无效，使用默认值*/
    {
        gCollectParaData.collect_data.mode_id = 0xC280;
        memset((uint8_t *)gCollectParaData.collect_data.sn, 0x30, 20);
        sprintf((char *)gCollectParaData.collect_data.sn, "BZSA17466W0X20000001"); ///
    }
    else
    {
        SpiFlashReadData(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE, buf, 256);//ModelID
        memcpy((uint8_t *)&gCollectParaData.collect_data.mode_id, buf, 2);
        gCollectParaData.collect_data.mode_id = 0xC280;
        SpiFlashReadData(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE * 2, buf, 256);//SN
        memcpy((uint8_t *)gCollectParaData.collect_data.sn, buf, sizeof(gCollectParaData.collect_data.sn));
    }
}




/*控制信息写入FLASH*/
void DeviceCtlWrite()
{
    uint8_t buf[256];
    uint8_t valid_flag[4];
    uint8_t i = 0;
	uint8_t len=0;

    for(i = 0; i < 4; i++)
    {
        valid_flag[i] = 0x0A;
    }

    /*数据区域*/
    for(i = 0; i < 2; i++)//第二次循环是数据备份
    {
    }
}

/*控制信息从FLASH读出*/
void DeviceCtlRead()
{
    uint8_t buf[256] = {0};
    uint8_t i = 0;
    uint8_t len = 0;
    for(i = 0; i < 2; i++)//第二次循环是数据备份
    {
        SpiFlashReadData(MAIN_CTL_PARA_INFO_FLASH_ADDR + 0x1000 * i , buf, 4);
        if(buf[0] == 0x0A && buf[1] == 0x0A && buf[2] == 0x0A && buf[3] == 0x0A)
        {
            break;
        }
    }
    if(i == 2)/*数据无效，使用默认值*/
    {

    }
    else
    {
    }
}


/*初始化调用*/
void SysInfoInit(void)
{
//在此处赋值软件版本号。
    (void)SoftwareVer;
    (void)HwVer;
    

    memcpy((uint8_t *)&gCollectParaData.collect_data.hw_ver,(uint8_t *)&HwVer,sizeof(HwVer));
    memcpy((uint8_t *)&gCollectParaData.collect_data.equipement_id[0],(uint8_t *)EquipmentId,sizeof(EquipmentId));
    DeviceBaseInfoRead();
    DeviceCtlRead();
    gCollectParaData.collect_data.slave_comm_addr = 0X01;
}