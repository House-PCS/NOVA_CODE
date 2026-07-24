#include "sys_info_storage.h"
#include "slave_collect.h"
#include "dev_modbus_rtu.h"
#include "flash.h"
#include "storage_cap.h"

void  UpdataPasswordProc(void);
volatile const SoftwareVer_t gstSoftwareVer = {{5, 1, 0}}; //LOW(8BIT) MID(8BIT) HI(8BIT)   {{6, 0, 1}}; 1-0-6
volatile const uint8_t arrHwVer[2] = {2, 0}; //A1      //{0, 2}; C0
volatile const char arrEquipmentId[20] = "MA-P10";//"AMP-10";
const CollectStorageInfo_t  INIT_COLLECT_VALUE =
{
    1,                                                                                           //  user_type;             501A
    1,                                                                                           //  slave_comm_addr;       502B                   
    0,                                                                                           //  monitor_485_baud;      502C                  
    1,                                                                                           //  backlight_time;        5031                   
    100,                                                                                         //  backlight_brightness;  5032                
    0,                                                                                           //  languange_type;        5033                  
    0,                                                                                           //  led_type_front_led_switch;   5035                                 
    0,                                                                                           //  mood_led_r_value;                       
    0xE8,                                                                                         //  mood_led_g_value;                       
    0xCD,                                                                                         //  mood_led_b_value;                       
    3,                                                                                           //  mood_led_mode;                          
    3000,                                                                                        //  mood_led_period;                        
    {0, 0x5460, 0x7080, 0x7e90, 0x13560, 0x14370, 0xb6d0, 0x12750, 0, 0, 0, 0, 0, 0, 0, 0},      //  peak_load_shift_time;       
//    0,                                                                                           //  batt_wakeup_time;                       
    3,                                                                                           //  dsg_chg_stop_soc_enable
    10,                                                                                          //  on_grid_dsg_stop_soc;
    100,                                                                                         //  on_grid_chg_stop_soc;
    0,                                                                                           //  heat_ctrl;           
    1,                                                                                           //  force_wake_enable;
    0,                                                                                           //  set_batt_chg_volt;                      
    400,                                                                                         //  set_batt_chg_curr;  40A                    
    16000,                                                                                       //  set_batt_chg_power;                     
    0,                                                                                           //  set_batt_dsg_volt;                      
    400,                                                                                         //  set_batt_dsg_curr;                      
    16000,                                                                                       //  set_batt_dsg_power; 
    0,                                                                                           //  special_mode; 

};

#define  MODE_ID        0XC290;
//volatile const char ModelId[2] ="01"
static  uint32_t _gdezipFileSize = 0;

void StorgDezipFileSize(uint32_t size)
{
    _gdezipFileSize = size;
    DeviceCtlWrite();
}
uint32_t GetDezipFileSize(void)
{
    return _gdezipFileSize;
}

/*ModelID和Sn号写入FLASH*/
void DeviceBaseInfoWrite(void)
{
    uint8_t arr_buf[256];
    uint8_t arr_valid_flag[4];
    uint8_t i = 0;

    for(i = 0; i < 4; i++)
    {
        arr_valid_flag[i] = 0x0B;
    }

    for(i = 0; i < 2; i++)//第二次循环是数据备份
    {
        SpiFlashAutoEraseSectors(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i, 4096);
        memcpy(arr_buf, (uint8_t *)&gstCollectInfo.maincard_data.base_info.model_id, 2);
        SpiFlashWriteBytesWithoutErase(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE, arr_buf, 2);//MODELID

        memcpy(arr_buf, (uint8_t *)gstCollectInfo.maincard_data.base_info.sn, sizeof(gstCollectInfo.maincard_data.base_info.sn));
        SpiFlashWriteBytesWithoutErase(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE * 2, arr_buf, sizeof(gstCollectInfo.maincard_data.base_info.sn));//SN
        SpiFlashWriteBytesWithoutErase(MAIN_CTL_FACTORY_INFO_FLASH_ADDR, arr_valid_flag, 4);
        UpdataPasswordProc();
    }
}


/*ModelID和Sn号从FLASH读出*/
void DeviceBaseInfoRead()
{
    uint8_t arr_buf[256];
    uint8_t i = 0;

    for(i = 0; i < 2; i++)//第二次循环是数据备份
    {
        SpiFlashReadData(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i , arr_buf, 4);
        if(arr_buf[0] == 0x0B && arr_buf[1] == 0x0B && arr_buf[2] == 0x0B && arr_buf[3] == 0x0B)
        {
            break;
        }
    }
    if(i == 2)/*数据无效，使用默认值*/
    {
        gstCollectInfo.maincard_data.base_info.model_id = MODE_ID;
        memset((uint8_t *)gstCollectInfo.maincard_data.base_info.sn, 0x30, 20);
        sprintf((char *)gstCollectInfo.maincard_data.base_info.sn, "2480191234567890"); ///
        

    }
    else
    {
        SpiFlashReadData(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE, arr_buf, 256);//ModelID
        memcpy((uint8_t *)&gstCollectInfo.maincard_data.base_info.model_id, arr_buf, 2);
//        gstCollectParaData.collect_data.mode_id =  MODE_ID;
        SpiFlashReadData(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE * 2, arr_buf, 256);//SN
        memcpy((uint8_t *)gstCollectInfo.maincard_data.base_info.sn, arr_buf, sizeof(gstCollectInfo.maincard_data.base_info.sn));
    }
}




/*控制信息写入FLASH*/
void DeviceCtlWrite()
{
    uint8_t arr_buf[1024];
    uint8_t arr_valid_flag[4];
    uint8_t i = 0;
    uint16_t len = 0;
    uint8_t size = 0 ;
    for(i = 0; i < 4; i++)
    {
        arr_valid_flag[i] = 0x0B;
    }

    /*数据区域*/
    for(i = 0; i < 2; i++)//第二次循环是数据备份
    {

        SpiFlashAutoEraseSectors(MAIN_CTL_PARA_INFO_FLASH_ADDR + 0x1000 * i, 1024);

        len = 0;
        arr_buf[len++] = (uint8_t)gstCollectInfo.maincard_data.base_info.user_type;
        arr_buf[len++] = (uint8_t)(gstCollectInfo.maincard_data.base_info.user_type >> 8);

        arr_buf[len++] = (uint8_t)gstCollectInfo.maincard_data.base_info.sw_ver;         //????? update_code_ver更新版本号
        arr_buf[len++] = (uint8_t)(gstCollectInfo.maincard_data.base_info.sw_ver >> 8);

//预留
        memcpy((uint8_t *)&arr_buf[256], (uint8_t *)&gstCollectNormalStorageData, sizeof(gstCollectNormalStorageData));
//补全256
        memcpy((uint8_t *)&arr_buf[512], (uint8_t *)&gstCollectInstallerStorageData, sizeof(gstCollectInstallerStorageData));

//        memcpy(&arr_buf[len], (uint8_t *)&gstCollectParaData.collect_data.collect_cap_info, sizeof(gstCollectParaData.collect_data.collect_cap_info));
//        len += sizeof(gstCollectParaData.collect_data.collect_cap_info);


//        memcpy(&arr_buf[len], (uint8_t *)&_gdezipFileSize, sizeof(_gdezipFileSize));
//        len += sizeof(_gdezipFileSize);
        SpiFlashWriteBytesWithoutErase(MAIN_CTL_PARA_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE, arr_buf, 1024);
        SpiFlashWriteBytesWithoutErase(MAIN_CTL_PARA_INFO_FLASH_ADDR + 0x1000 * i, arr_valid_flag, 4);
        SetCommAddr((uint8_t)gstCollectInfo.maincard_data.base_info.modbus_addr);
    }
}

/**
  * @brief  阈值检测
  * @param  NULL
  * @retval NULL
  */
void  _ReadThresHoldCheck(CollectInfo_t *pread_data)
{
    if(pread_data == NULL)
    {
        return;
    }

    if(pread_data->maincard_data.base_info.user_type > 1)
    {
        pread_data->maincard_data.base_info.user_type = 1;
    }
    if(pread_data->maincard_data.base_info.modbus_addr > 0x10)
    {
        pread_data->maincard_data.base_info.modbus_addr = COLLECT_SLAVE_ADR;
    }
#define  BAUD_MAX   7
    if(pread_data->maincard_data.base_info.baudrate > BAUD_MAX)
    {
        pread_data->maincard_data.base_info.baudrate = 0;
    }
    if(pread_data->maincard_data.total_pack_info.on_grid_dsg_stop_soc > 100)
    {
        pread_data->maincard_data.total_pack_info.on_grid_dsg_stop_soc = 0;
    }
//    if((pread_data->set_batt_chg_volt < 500) || (pread_data->set_batt_chg_volt > 576))
//    {
//        pread_data->set_batt_chg_volt = 576;
//    }
    if((pread_data->maincard_data.sys_info.max_chg_curr_set < 10) || (pread_data->maincard_data.sys_info.max_chg_curr_set > 990))
    {
        pread_data->maincard_data.sys_info.max_chg_curr_set = 990;
    }
    if(pread_data->maincard_data.sys_info.max_chg_power_set > 16000)
    {
        pread_data->maincard_data.sys_info.max_chg_power_set = 16000;
    }
//    if((pread_data->set_batt_dsg_volt < 400) || (pread_data->set_batt_dsg_volt > 500))
//    {
//        pread_data->set_batt_dsg_volt = 400;
//    }
    if((pread_data->maincard_data.sys_info.max_dsg_curr_set < 10) || (pread_data->maincard_data.sys_info.max_dsg_curr_set > 990))
    {
        pread_data->maincard_data.sys_info.max_dsg_curr_set = 990;
    }
    if(pread_data->maincard_data.sys_info.max_dsg_power_set > 16000)
    {
        pread_data->maincard_data.sys_info.max_dsg_power_set = 16000;
    }


}


/*控制信息从FLASH读出*/
void DeviceCtlRead()
{
    uint8_t arr_buf[1024] = {0};
    uint8_t i = 0;
    uint16_t len = 0;
    for(i = 0; i < 2; i++)//第二次循环是数据备份
    {
        SpiFlashReadData(MAIN_CTL_PARA_INFO_FLASH_ADDR + 0x1000 * i , arr_buf, 4);
        if(arr_buf[0] == 0x0B && arr_buf[1] == 0x0B && arr_buf[2] == 0x0B && arr_buf[3] == 0x0B)
        {
            break;
        }
    }
    if(i == 2)/*数据无效，使用默认值*/
    {
//        memcpy((uint8_t *)&gstCollectInfo.maincard_data.base_info.sw_ver,(uint8_t *)&SoftwareVer,sizeof(tuSoftwareVer));
        gstCollectNormalStorageData = INIT_COLLECT_VALUE;
        gstCollectInstallerStorageData = INIT_COLLECT_VALUE;
        ReadDataToStorageData(&gstCollectInfo, &gstCollectNormalStorageData);
        gstCollectInfo.maincard_data.base_info.user_type = 1;
        memset((uint8_t *)gstCollectInfo.maincard_data.base_info.normal_password,    0, sizeof(gstCollectInfo.maincard_data.base_info.normal_password));
        memset((uint8_t *)gstCollectInfo.maincard_data.base_info.manager_password,   0, sizeof(gstCollectInfo.maincard_data.base_info.manager_password));
        memset((uint8_t *)gstCollectInfo.maincard_data.base_info.installer_password, 0, sizeof(gstCollectInfo.maincard_data.base_info.installer_password));
        memset((uint8_t *)gstCollectInfo.maincard_data.base_info.internal_password,  0, sizeof(gstCollectInfo.maincard_data.base_info.internal_password));
        DeviceCtlWrite();           //写入默认值
    }
    else
    {
        SpiFlashReadData(MAIN_CTL_PARA_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE, arr_buf, 1024);//name
        len = 0;
        gstCollectInfo.maincard_data.base_info.user_type = arr_buf[len] | (arr_buf[len + 1] << 8) ;      //user_tyte

        len = 2;
//        gstCollectInfo.maincard_data.base_info.sw_ver = arr_buf[len] | (arr_buf[len + 1] << 8) ;      //升级包版本号  //?????更新版本号

        memcpy((uint8_t *)&gstCollectNormalStorageData, (uint8_t *)&arr_buf[256], sizeof(gstCollectNormalStorageData));
        memcpy((uint8_t *)&gstCollectInstallerStorageData, (uint8_t *)&arr_buf[512], sizeof(gstCollectInstallerStorageData));
        ReadDataToStorageData(&gstCollectInfo, &gstCollectNormalStorageData);
        ReadDataToStorageData(&gstInstallerCollectInfo, &gstCollectInstallerStorageData);//?????

        _ReadThresHoldCheck(&gstInstallerCollectInfo);    //?????
        _ReadThresHoldCheck(&gstCollectInfo);
//        memcpy((uint8_t *)&gstCollectParaData.collect_data.collect_cap_info, &arr_buf[len], sizeof(gstCollectParaData.collect_data.collect_cap_info));
//        len += sizeof(gstCollectParaData.collect_data.collect_cap_info);

//        memcpy((uint8_t *)&_gdezipFileSize, &arr_buf[len], sizeof(_gdezipFileSize));
//        len += sizeof(_gdezipFileSize);
    }

    if((gstCollectInfo.maincard_data.base_info.sw_ver == 0xffff) || (gstCollectInfo.maincard_data.base_info.sw_ver == 0x0000))  //?????更新版本号
    {
        gstCollectInfo.maincard_data.base_info.sw_ver = 0x0040;   //1.0.0   //?????更新版本号
    }

    SetCommAddr((uint8_t)gstCollectInfo.maincard_data.base_info.modbus_addr);
}

/**
  * @brief  更新存储值
  * @param  NULL
  * @retval NULL
  */
void WriteInfoToStorgeData(CollectStorageInfo_t *pdst, CollectInfo_t *psrc)
{
    if((pdst == NULL) || (psrc == NULL))
    {
        return;
    }
    pdst->user_type = psrc->maincard_data.base_info.user_type;            
    pdst->slave_comm_addr = psrc->maincard_data.base_info.modbus_addr;
    pdst->monitor_485_baud = psrc->maincard_data.base_info.baudrate;
    pdst->backlight_time = psrc->maincard_data.device_info.lcd_bl_time;
    pdst->backlight_brightness = psrc->maincard_data.device_info.lcd_bl_brightness;
    pdst->languange_type = psrc->maincard_data.device_info.lcd_languege;
    pdst->led_front_switch = psrc->maincard_data.device_info.led_front_switch;
//  psrc->side_led_switch = pdst->maincard_data.device_info.side_led_switch;

    pdst->mood_led_r_value = psrc->maincard_data.device_info.led_aux_r;
    pdst->mood_led_g_value = psrc->maincard_data.device_info.led_aux_g;
    pdst->mood_led_b_value = psrc->maincard_data.device_info.led_aux_b;
    pdst->mood_led_mode = psrc->maincard_data.device_info.led_aux_mode;
    pdst->mood_led_period = psrc->maincard_data.device_info.led_aux_period;
                                                         
    memcpy((uint8_t *)&pdst->peak_load_shift_time, (uint8_t *)&(psrc->maincard_data.peak_time_info), sizeof(pdst->peak_load_shift_time));
    pdst->on_grid_dsg_stop_soc = psrc->maincard_data.total_pack_info.on_grid_dsg_stop_soc;
    pdst->dsg_chg_stop_soc_enable = psrc->maincard_data.total_pack_info.dsg_chg_stop_soc_enable;
    pdst->on_grid_chg_stop_soc = psrc->maincard_data.total_pack_info.on_grid_chg_stop_soc;
    pdst->heat_ctrl = psrc->maincard_data.total_pack_info.heat_ctrl;              
    pdst->force_wake_enable = psrc->maincard_data.total_pack_info.force_wake_enable;

//    pdst->set_batt_chg_volt = psrc->set_batt_chg_volt;          
    pdst->set_batt_chg_curr = psrc->maincard_data.sys_info.max_chg_curr_set;
    pdst->set_batt_chg_power = psrc->maincard_data.sys_info.max_chg_power_set;
//    pdst->set_batt_dsg_volt = psrc->set_batt_dsg_volt;       
    pdst->set_batt_dsg_curr = psrc->maincard_data.sys_info.max_dsg_curr_set;;
    pdst->set_batt_dsg_power = psrc->maincard_data.sys_info.max_dsg_power_set;
                                                             
    pdst->special_mode = psrc->maincard_data.sys_info.special_mode;
}

/**
  * @brief  读出存储值
  * @param  NULL
  * @retval NULL
  */
void ReadDataToStorageData(CollectInfo_t *pdst, CollectStorageInfo_t *psrc)
{
    if((pdst == NULL) || (psrc == NULL))
    {
        return;
    }
    pdst->maincard_data.base_info.user_type             = psrc->user_type;
    pdst->maincard_data.base_info.modbus_addr           = psrc->slave_comm_addr;
    pdst->maincard_data.base_info.baudrate              = psrc->monitor_485_baud;
    pdst->maincard_data.device_info.lcd_bl_time         = psrc->backlight_time;
    pdst->maincard_data.device_info.lcd_bl_brightness   = psrc->backlight_brightness;
    pdst->maincard_data.device_info.lcd_languege        = psrc->languange_type;
    pdst->maincard_data.device_info.led_front_switch    = psrc->led_front_switch;
//    pdst->maincard_data.device_info.side_led_switch = psrc->side_led_switch;

    pdst->maincard_data.device_info.led_aux_r           = psrc->mood_led_r_value;
    pdst->maincard_data.device_info.led_aux_g           = psrc->mood_led_g_value;
    pdst->maincard_data.device_info.led_aux_b           = psrc->mood_led_b_value;
    pdst->maincard_data.device_info.led_aux_mode        = psrc->mood_led_mode;
    pdst->maincard_data.device_info.led_aux_period      = psrc->mood_led_period;

    memcpy((uint8_t *) & (pdst->maincard_data.peak_time_info), (uint8_t *)&psrc->peak_load_shift_time, sizeof(psrc->peak_load_shift_time));
    pdst->maincard_data.total_pack_info.on_grid_dsg_stop_soc     = psrc->on_grid_dsg_stop_soc;
    pdst->maincard_data.total_pack_info.dsg_chg_stop_soc_enable  = psrc->dsg_chg_stop_soc_enable;
    pdst->maincard_data.total_pack_info.on_grid_chg_stop_soc     = psrc->on_grid_chg_stop_soc;
    pdst->maincard_data.total_pack_info.heat_ctrl                = psrc->heat_ctrl;              
    pdst->maincard_data.total_pack_info.force_wake_enable        = psrc->force_wake_enable;

//    pdst->set_batt_chg_volt = psrc->set_batt_chg_volt;
    pdst->maincard_data.sys_info.max_chg_curr_set  = psrc->set_batt_chg_curr;
    pdst->maincard_data.sys_info.max_chg_power_set = psrc->set_batt_chg_power;
//    pdst->set_batt_dsg_volt = psrc->set_batt_dsg_volt;
    pdst->maincard_data.sys_info.max_dsg_curr_set  = psrc->set_batt_dsg_curr;
    pdst->maincard_data.sys_info.max_dsg_power_set = psrc->set_batt_dsg_power;

    pdst->maincard_data.sys_info.special_mode                = psrc->special_mode;
}


void StorageDataRest(uint8_t user_type)
{
    uint8_t arr_buf[1024];
    uint8_t i = 0;
    
    memset(arr_buf, 0, 1024);

    if(user_type > 1 )
    {
        #if 0
        //安装模式下恢复出厂设置
        gstCollectNormalStorageData = INIT_COLLECT_VALUE;
        gstCollectInstallerStorageData = INIT_COLLECT_VALUE;
        ReadDataToStorageData(&gstCollectInfo, &gstCollectInstallerStorageData);
        ReadDataToStorageData(&gstInstallerCollectInfo, &gstCollectInstallerStorageData);//?????
        DeviceCtlWrite();
        #else 
        /*数据区域*/
        for(i = 0; i < 2; i++)//第二次循环是数据备份
        {
            SpiFlashAutoEraseSectors(MAIN_CTL_PARA_INFO_FLASH_ADDR + 0x1000 * i, 1024);
//预留
            memcpy((uint8_t *)&arr_buf[256], (uint8_t *)&gstCollectNormalStorageData, sizeof(gstCollectNormalStorageData));
//补全256
            memcpy((uint8_t *)&arr_buf[512], (uint8_t *)&gstCollectInstallerStorageData, sizeof(gstCollectInstallerStorageData));
            
            SpiFlashWriteBytesWithoutErase(MAIN_CTL_PARA_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE, arr_buf, 1024);
            SetCommAddr((uint8_t)gstCollectInfo.maincard_data.base_info.modbus_addr);
        }   
        #endif
//清空历史用电数据信息 --将存储位置flash全部清空。
        SpiFlashAutoEraseSectors(CAP_STORAGE_BASE_ADDR, CAP_TOTAL_SIZE);
    }
    else
    { 
        #if 0
        //普通用户恢复出厂设置   需要恢复为安装模式的数据
        gstCollectNormalStorageData = INIT_COLLECT_VALUE;
        gstCollectInstallerStorageData = INIT_COLLECT_VALUE;
        gstCollectNormalStorageData = gstCollectInstallerStorageData;
        ReadDataToStorageData(&gstCollectInfo, &gstCollectInstallerStorageData);
        DeviceCtlWrite();
        #else 
        //普通用户恢复出厂设置   需要恢复为安装模式的数据
        //需要下电后实现 先擦除头信息 上电重新赋初始值
        /*数据区域*/
        for(i = 0; i < 2; i++)//第二次循环是数据备份
        {
            SpiFlashAutoEraseSectors(MAIN_CTL_PARA_INFO_FLASH_ADDR + 0x1000 * i, 1024);
//预留
            memcpy((uint8_t *)&arr_buf[256], (uint8_t *)&gstCollectNormalStorageData, sizeof(gstCollectNormalStorageData));
//补全256
            memcpy((uint8_t *)&arr_buf[512], (uint8_t *)&gstCollectInstallerStorageData, sizeof(gstCollectInstallerStorageData));
            
            SpiFlashWriteBytesWithoutErase(MAIN_CTL_PARA_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE, arr_buf, 1024);
            SetCommAddr((uint8_t)gstCollectInfo.maincard_data.base_info.modbus_addr);
        } 
        #endif
    }
}


void  UpdataPasswordProc(void)
{

    uint8_t i = 0;
    char password[6];
    char sn[20];
    memset((uint8_t *)&password[0],0,sizeof(password));
    #if 0
    memset((uint8_t *)gstCollectInfo.maincard_data.base_info.sn, 0,sizeof(gstCollectInfo.maincard_data.base_info.sn));
    #endif
    memcpy((uint8_t *)sn, (uint8_t *)gstCollectInfo.maincard_data.base_info.sn, sizeof(sn));
    if( (sn[0] >= '2') &&  \
        (sn[1] <= '9') && \
        (sn[1] >= '0') && \
        (sn[1] <= '9') )
    {
        //必须是20-29

        if( ((sn[2] >= '0') && (sn[2] <= '9'))  ||
                ((sn[2] >= 'a') && (sn[2] <= 'c'))  ||
                ((sn[2] >= 'A') && (sn[2] <= 'C')))
        {


            if((sn[3] >= '0') && (sn[3] <= '3'))
            {

                if((sn[4] >= '0') && (sn[4] <= '9'))
                {
                    //合法的SN

                    password[0] = sn[0] - 48; //2
                    password[1] = sn[1] - 48; //0
                    if((sn[2] >= '0') && (sn[2] <= '9'))
                    {
                        i = sn[2] - 48;
//                            password[2] = sn[2] -48;

                    }
                    else if((sn[2] >= 'a') && (sn[2] <= 'c'))
                    {
                        i = sn[2] - 97;
                        i += 10;
//                            password[2] = sn[2] -97;
                    }
                    else
                    {
                        i = sn[2] - 65;
                        i += 10;
//                            password[2] = sn[2] -65;
                    }

                    if(i < 10)
                    {
                        password[2] = 0;
                        password[3] = i;
                    }
                    else
                    {
                        password[2] = 1;
                        password[3] = i-10;                           
                    }

                    password[4] = sn[3] - 48;
                    password[5] = sn[4] - 48;
                    
                    memcpy((uint8_t *)&gstCollectInfo.maincard_data.base_info.normal_password[0],(uint8_t *)password,sizeof(password));
                    memcpy((uint8_t *)&gstCollectInfo.maincard_data.base_info.manager_password[0],(uint8_t *)password,sizeof(password));
                    password[5] += 1;
                    password[5] %=10;
                    memcpy((uint8_t *)&gstCollectInfo.maincard_data.base_info.installer_password[0],(uint8_t *)password,sizeof(password));
                    password[5] = (sn[4] -48);
                    password[5] +=2;
                    password[5] %=10;
                    memcpy((uint8_t *)&gstCollectInfo.maincard_data.base_info.internal_password[0],(uint8_t *)password,sizeof(password));
                }
            }
        }
    }
    else
    {
        memset((uint8_t *)&gstCollectInfo.maincard_data.base_info.normal_password[0],   0,sizeof(gstCollectInfo.maincard_data.base_info.normal_password));
        memset((uint8_t *)&gstCollectInfo.maincard_data.base_info.manager_password[0],  0,sizeof(gstCollectInfo.maincard_data.base_info.manager_password));
        memset((uint8_t *)&gstCollectInfo.maincard_data.base_info.installer_password[0],0,sizeof(gstCollectInfo.maincard_data.base_info.installer_password));
        memset((uint8_t *)&gstCollectInfo.maincard_data.base_info.internal_password[0], 0,sizeof(gstCollectInfo.maincard_data.base_info.internal_password));

    }

}
/*初始化调用*/
void SysInfoInit(void)
{
//在此处赋值软件版本号。
    (void)gstSoftwareVer;
    (void)arrHwVer;
    (void)arrEquipmentId;

    SoftwareVer_t moniitor_soft_ver;
//    uint8_t ver_arr[4];
//    uint8_t ver_arr[256];

	uint8_t ver[20]={0,0,0};
    InternalFlashByteRead(APP_FIRMWARE_HEAD_INTERNAL_ADDR+4, ver, 20);		//读主控软件版本号
    moniitor_soft_ver.bits.ver_hi = ver[0];
    moniitor_soft_ver.bits.ver_mid = ver[1];
    moniitor_soft_ver.bits.ver_low = ver[2];

    memcpy((uint8_t *)&gstCollectInfo.maincard_data.base_info.sw_ver_ext,(uint8_t *)&ver[7],12);
    memcpy((uint8_t *)&gstCollectInfo.maincard_data.base_info.hw_ver, (uint8_t *)&arrHwVer, sizeof(arrHwVer));
//    memcpy((uint8_t *)gstCollectInfo.maincard_data.inside_info.amp, (uint8_t *)arrEquipmentId, sizeof(arrEquipmentId));     //?????
    memcpy((uint8_t *)&gstCollectInfo.maincard_data.inside_info.main_version, (uint8_t *)&gstSoftwareVer, sizeof(SoftwareVer_t));  //监控卡自身版本
    memcpy((uint8_t *)&gstCollectInfo.maincard_data.base_info.sw_ver, (uint8_t *)&moniitor_soft_ver, sizeof(SoftwareVer_t));   //APP前头的数据   
    memcpy((uint8_t *)&gstCollectInfo.maincard_data.base_info.sw_ver, (uint8_t *)&gstSoftwareVer, sizeof(SoftwareVer_t));    //2026年3月11日 重新修订后 选取肚子里的数据

//    InternalFlashByteRead(APP_FIRMWARE_HEAD_INTERNAL_ADDR+4,ver_arr, 3);		//读主控软件版本号
//    InternalFlashByteRead(APP_FIRMWARE_HEAD_INTERNAL_ADDR,ver_arr, 256);		//读主控软件版本号
    DeviceBaseInfoRead();
    DeviceCtlRead();
    UpdataPasswordProc();
}
//InternalFlashRead(APP_FIRMWARE_HEAD_INTERNAL_ADDR+4, ver, 3);		//读主控软件版本号


static uint8_t _GetVersionIO1Data(void)
{
    uint8_t McuID1;
    McuID1 = GpioBitGet((&McuVerID1Handle));
    return McuID1;
}
static uint8_t _GetVersionIO2Data(void)
{
    uint8_t McuID1;
    McuID1 = GpioBitGet((&McuVerID2Handle));
    return McuID1;
}
/*******初始化版本区分******/
void McuVersionTypeInit(void)
{
    uint8_t McuID1;
    uint8_t McuID2;
    uint16_t VersionFlag;
    uint16_t old_VersionFlag;
    uint8_t cnt = 10;
    uint32_t time_out = 0;
    McuVersionBspInit();
    while(1)
    {
        McuID1 = _GetVersionIO1Data();
        McuID2 = _GetVersionIO2Data();
        VersionFlag = ((McuID1 << 8) | McuID2);
        if(VersionFlag != old_VersionFlag)
        {
            old_VersionFlag = VersionFlag;
            cnt = 0;
        }
        else
        {
            if(cnt++ > 20)
            {
                if(VersionFlag)
                {
                    gDeviceVersionType = DEVICE_TYPE_LED;
                }
                else
                {
                    gDeviceVersionType = DEVICE_TYPE_LCD;
                }
                break;
            }

        }
    }
    gstCollectInfo.maincard_data.device_info.dev_type = gDeviceVersionType;
}

