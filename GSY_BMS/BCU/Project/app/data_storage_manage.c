/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        data_storage_manage.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-18
  * @brief       该模块用于需要存储的数据上电获取，以及定时保存，或者上位机下发固化等
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-18     <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "data_storage_manage.h"


#define INTERNAL_FLASH_FW_INFO_ADDR                 (0x10000 - 256)

#define FLASH_DEV_BASE_INFO_VALID_FLAG_ADDR         (0xC0000)
#define FLASH_DEV_BASE_INFO_ADDR                    (FLASH_DEV_BASE_INFO_VALID_FLAG_ADDR + 0x100)

#define FLASH_BMU_SAMPLE_INFO_VALID_FLAG_ADDR       (0xD2000)
#define FLASH_BMU_SAMPLE_INFO_ADDR                  (FLASH_BMU_SAMPLE_INFO_VALID_FLAG_ADDR + 0x100)
#define FLASH_PACK_FACT_PARA_VALID_FLAG_ADDR        (0xD3000)
#define FLASH_PACK_FACT_PARA_ADDR                   (FLASH_PACK_FACT_PARA_VALID_FLAG_ADDR + 0x100)
#define FLASH_ALARM_THRESHOLD_VALID_FLAG_ADDR       (0xD4000)
#define FLASH_ALARM_THRESHOLD_ADDR                  (FLASH_ALARM_THRESHOLD_VALID_FLAG_ADDR + 0x100)
#define FLASH_BCU_FUNC_THRESHOLD_VALID_FLAG_ADDR    (0xD5000)
#define FLASH_BCU_FUNC_THRESHOLD_ADDR               (FLASH_BCU_FUNC_THRESHOLD_VALID_FLAG_ADDR + 0x100)
#define FLASH_BCU_CORR_COEF_VALID_FLAG_ADDR         (0xC1000)
#define FLASH_BCU_CORR_COEF_ADDR                    (FLASH_BCU_CORR_COEF_VALID_FLAG_ADDR + 0x100)


#define EEPROM_SOX_INFO_STORAGE_NUM                 10   
#define EEPROM_PER_SOX_INFO_STORAGE_SIZE            (32 * 2)
#define EEPROM_FIRST_SOX_INFO_VALID_FLAG_ADDR       (0x0)
#define EEPROM_SOX_INFO_STORAGE_INDEX_OFFSET        4                   
#define EEPROM_SOX_INFO_STORAGE_OFFSET              8


#define EEPROM_ELECT_STATIST_STORAGE_NUM            2
#define EEPROM_PER_ELECT_STATIST_STORAGE_SIZE       (32 * 4)
#define EEPROM_FIRST_ELECT_STATIST_VALID_FLAG_ADDR  (0x500)
#define EEPROM_ELECT_STATIST_STORAGE_INDEX_OFFSET   4     
#define EEPROM_ELECT_STATIST_STORAGE_OFFSET         8

uint8_t buf_Version[3];
static uint32_t _gSoxInfoIndex = 0;//记录写入次数
static uint8_t _gSoxInfoLocation = 0xff;//当前数据来自于eeprom soxinfo哪个区域，0xff表示未确认区域

static uint32_t _gElectStatistIndex = 0;//记录写入次数
static uint8_t _gElectStatistLocation = 0xff;//当前数据来自于eeprom soxinfo哪个区域，0xff表示未确认区域

//BMU默认采样掩码
static const BMUSampleInfo_t _gBMUDefaultSampleInfo = 
{
    1,  //pack_num
    32, //cell_volt_sample_num
    26, //pack_temp_sample_num，新艾实际使用20个，每个AFE通道挂10个
    4,  //balance_temp_sample_num
    {0xFF, 0x7F, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00}, //cell_volt_mask
    //01111111 10000011 11111100 00
//    {0xFE, 0xC1, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},//cell_temp_mask,不包含极柱和正负极
    //11111111 11000111 11111110 00
    {0xFF, 0xE3, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},//cell_temp_mask,包含极柱和正负极
    {0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},//pole_temp_mask
    {0x01, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},//other_temp_mask
    {0x0F},//balance_temp_mask
};

//电芯默认出厂信息
static const PackFactPara_t _gPackDefaultFactPara = 
{
    100.0f,//soc_max
    0.0f,//soc_min
    280000,//q_max
    3620,//cell_max_chg_voltage
    2800,//cell_min_dsg_voltage
    3500,//chg_reduce_cur_voltage
    3000,//dsg_reduce_cur_voltage
    5,//chg_max_rate
    5,//dsg_max_rate
    2,//chg_reduce_cur_rate
    2//chg_reduce_cur_rate
};

//故障默认阈值
static const AlarmThreshold_t _gAlarmDefaultThreshold = 
{
    //测量告警阈值
    {3650, 3700, 3750},//单体过压，单位mV
    {2700, 2650, 2600},//单体欠压，单位mV
    {45, 50, 55},//单体过温，单位℃
    {-5, -10, -20},//单体欠温，单位℃
    {0, 0, 0},//mos过温，单位℃，暂不涉及
    {0, 0, 0},//mos欠温，单位℃，暂不涉及
    {75, 80, 85},//箱体1过温(继电器)，单位℃
    {-20, -30, -40},//箱体1欠温(继电器)，单位℃
    {75, 80, 85},//箱体2过温(高压箱体)，单位℃
    {-20, -30, -40},//箱体2欠温(高压箱体)，单位℃
    {0, 0, 0},//箱体3过温，单位℃，暂不涉及
    {0, 0, 0},//箱体3欠温，单位℃，暂不涉及
    {0, 0, 0},//箱体4过温，单位℃，暂不涉及
    {0, 0, 0},//箱体4欠温，单位℃，暂不涉及
    {876, 888, 900},//B+过压，单位V
    {648, 636, 624},//B+欠压，单位V
    {876, 888, 900},//P+过压，单位V
    {648, 636, 624},//P+欠压，单位V
    {0, 0, 0},//总压压差过大，单位V，单簇不涉及
    {20, 10, 0},//SOC低(0~100) 
    {80, 75, 70},//SOH低(0~100)   
    {145, 155, 165},//充电电流大，单位A
    {145, 155, 165},//放电电流大，单位A
    {50, 30, 10},//绝缘阻抗低，单位KΩ
    {200, 400, 600},//电芯压差大，单位mV
    {10, 15, 20},//电芯温差大，单位℃
    
    //其他告警阈值
    {0, 0, 0},//正极柱过温，单位℃，暂不涉及
    {0, 0, 0},//正极柱欠温，单位℃，暂不涉及  
    {0, 0, 0},//负极柱过温，单位℃，暂不涉及   
    {0, 0, 0},//负极柱欠温，单位℃，暂不涉及
    {0, 0, 0},//铜排过温，单位℃，暂不涉及    
    {0, 0, 0},//铜排欠温，单位℃，暂不涉及    
    {0, 0, 0},//单体充电过温，单位℃，暂不涉及    
    {0, 0, 0},//单体放电过温，单位℃，暂不涉及    
    {0, 0, 0},//单体充电欠温，单位℃，暂不涉及    
    {0, 0, 0},//单体放电欠温，单位℃，暂不涉及    
    {0, 0, 0},//单体温度变化率大，暂不涉及    
    {55, 60, 65},//极柱过温，单位℃    
    {-20, -30, -40},//极柱欠温，单位℃，暂不涉及  
    {0, 0, 0},//单体电压变化率大，暂不涉及    
    {0, 0, 0},//SOE低,暂不涉及    
    {0, 0, 0},//充电MAP过流，单位A，暂不涉及   
    {0, 0, 0},//放电MAP过流，单位A，暂不涉及        
};


//bcu功能阈值
static BCUFuncThreshold_t _gBCUFuncDefaultThreshold = 
{
    100,  //balance_start_diff_volt
    3100, //balance_min_volt
    25,   //fan_start_temp
    35,   //fan_full_temp
    23,   //fan_stop_temp
    8,    //fan_start_duty
    5000, //pre_charge_diff_volt
    10000,//hv_bmu_total_diff_max_volt
    2000,//pre_charge_complete_max_current
    20000,//cut_off_relay_p_max_current
};



//sox_info默认值
static const SOXInfo_t _gSoxDefaultInfo = 
{
    30.0f,//soc_real
    30.0f,//soc_disp
    100.0f,//soh
    0,//chg_cycle
    0,//dsg_cycle
    0,//ocv_time
};

//elect_statist无需默认值，全0即可

/** 
 * @brief    判断FLASH中对应数据是否有效
 * @param    addr:FLASH对应扇区首地址
 * @return   是否有效，true代表有效，false代表无效
 * @note:
 */
static bool _FlashDataValidJudge(uint32_t addr)
{
    uint8_t temp[256];
    
    memset(temp, 0x00, 256);
    SpiFlashReadData(addr, temp, 256);
    for(uint16_t i = 0; i < 256; i++)
    {
        if(temp[i] != 0x0A)
        {
            return false;
        }
    }
    return true;
}


/** 
 * @brief    判断EEPROM中对应数据是否有效
 * @param    addr:EEPROM对应Page的首地址
 * @return   是否有效，true代表有效，false代表无效
 * @note:
 */
static bool _EEPROMDataValidJudge(uint32_t addr)
{
    uint8_t temp[4];
    
    memset(temp, 0x00, 4);
    EepromRead(temp, addr, 4);
    for(uint8_t i = 0; i < 4; i++)
    {
        if(temp[i] != 0x5A)
        {
            return false;
        }
    }
    return true;
}


/** 
 * @brief    获取最新sox_info位置信息和写入次数索引
 * @param    none
 * @return   数据是否获取到，true表示获取到，false表示EEPROM没有存储相关信息(新设备)
 * @note:
 */
static bool _EEPROMLatestSoxInfoGet()
{
    bool ret = false;
    uint32_t index = 0;
    uint32_t addr;
    for(uint8_t i = 0; i < EEPROM_SOX_INFO_STORAGE_NUM; i++)
    {
        addr = EEPROM_FIRST_SOX_INFO_VALID_FLAG_ADDR + EEPROM_PER_SOX_INFO_STORAGE_SIZE * i;
        if(_EEPROMDataValidJudge(addr) == true)//获取到信息
        {
            uint8_t temp[4];
            EepromRead(temp, addr + EEPROM_SOX_INFO_STORAGE_INDEX_OFFSET, 4);//获取写入次数索引
            index = temp[0] | (temp[1] << 8) | (temp[2] << 16) | (temp[3] << 24);
            if(index > _gSoxInfoIndex)
            {
                _gSoxInfoIndex = index;
                _gSoxInfoLocation = i;
                ret = true;
            }
        }
    }
    return ret;
}


/** 
 * @brief    获取最新elect_statist位置信息和写入次数索引
 * @param    none
 * @return   数据是否获取到，true表示获取到，false表示EEPROM没有存储相关信息(新设备)
 * @note:
 */
static bool _EEPROMLatestElectStatistGet()
{
    bool ret = false;
    uint32_t index = 0;
    uint32_t addr;
    for(uint8_t i = 0; i < EEPROM_ELECT_STATIST_STORAGE_NUM; i++)
    {
        addr = EEPROM_FIRST_ELECT_STATIST_VALID_FLAG_ADDR + EEPROM_PER_ELECT_STATIST_STORAGE_SIZE * i;
        if(_EEPROMDataValidJudge(addr) == true)//获取到信息
        {
            uint8_t temp[4];
            EepromRead(temp, addr + EEPROM_ELECT_STATIST_STORAGE_INDEX_OFFSET, 4);//获取写入次数索引
            index = temp[0] | (temp[1] << 8) | (temp[2] << 16) | (temp[3] << 24);
            if(index > _gElectStatistIndex)
            {
                _gElectStatistIndex = index;
                _gElectStatistLocation = i;
                ret = true;
            }
        }
    }
    return ret;
}




/** 
 * @brief    sox_info存储
 * @param    none
 * @return   none
 * @note:    定时存储，要考虑EEPROM的写入寿命
 */
static void _EEPROMSoxInfoStorage()
{
    uint32_t invalid_flag = 0xFFFFFFFF;
    uint32_t valid_flag = 0x5A5A5A5A;
    uint32_t addr;
    
    if(_gSoxInfoLocation == 0xFF)//之前没有存储过数据
    {
        _gSoxInfoLocation = 0;
    }
    else
    {
        _gSoxInfoLocation++;
        _gSoxInfoLocation %= EEPROM_SOX_INFO_STORAGE_NUM;
    }
    _gSoxInfoIndex++;
    
    addr = EEPROM_FIRST_SOX_INFO_VALID_FLAG_ADDR + EEPROM_PER_SOX_INFO_STORAGE_SIZE * _gSoxInfoLocation;
    //先清除原先数据有效标志
    EepromWrite((uint8_t *)&invalid_flag, addr, 4);
    //再写入数据
    EepromWrite((uint8_t *)&(gPackInfo.sox_info), addr + EEPROM_SOX_INFO_STORAGE_OFFSET, sizeof(SOXInfo_t));
    //然后写入写入次数索引
    EepromWrite((uint8_t *)&_gSoxInfoIndex, addr + EEPROM_SOX_INFO_STORAGE_INDEX_OFFSET, 4);
    //最后写入有效标志
    EepromWrite((uint8_t *)&valid_flag, addr, 4);
}



/** 
 * @brief    elect_statist存储
 * @param    none
 * @return   none
 * @note:    定时存储，要考虑EEPROM的写入寿命
 */
static void _EEPROMElectStatistStorage()
{
    uint32_t invalid_flag = 0xFFFFFFFF;
    uint32_t valid_flag = 0x5A5A5A5A;
    uint32_t addr;
    
    if(_gElectStatistLocation == 0xFF)//之前没有存储过数据
    {
        _gElectStatistLocation = 0;
    }
    else
    {
        _gElectStatistLocation++;
        _gElectStatistLocation %= EEPROM_ELECT_STATIST_STORAGE_NUM;
    }
    _gElectStatistIndex++;
    
    addr = EEPROM_FIRST_ELECT_STATIST_VALID_FLAG_ADDR + EEPROM_PER_ELECT_STATIST_STORAGE_SIZE * _gElectStatistLocation;
    //先清除原先数据有效标志
    EepromWrite((uint8_t *)&invalid_flag, addr, 4);
    //再写入数据
    EepromWrite((uint8_t *)&(gPackInfo.elect_statist), addr + EEPROM_ELECT_STATIST_STORAGE_OFFSET, sizeof(ElectStatist_t));
    //然后写入写入次数索引
    EepromWrite((uint8_t *)&_gElectStatistIndex, addr + EEPROM_ELECT_STATIST_STORAGE_INDEX_OFFSET, 4);
    //最后写入有效标志
    EepromWrite((uint8_t *)&valid_flag, addr, 4);
}




/** 
 * @brief    sox_info存储数据清除
 * @param    none
 * @return   none
 * @note:    none
 */
void SoxInfoStorageClear()
{
    uint32_t invalid_flag = 0xFFFFFFFF;
    uint32_t addr;
    
    for(uint8_t i = 0; i < EEPROM_SOX_INFO_STORAGE_NUM; i++)
    {
        addr = EEPROM_FIRST_SOX_INFO_VALID_FLAG_ADDR + EEPROM_PER_SOX_INFO_STORAGE_SIZE * i;
        EepromWrite((uint8_t *)&invalid_flag, addr, 4);
    }
}


/** 
 * @brief    elect_statist存储数据清除
 * @param    none
 * @return   none
 * @note:    none
 */
void ElectStatistStorageClear()
{
    uint32_t invalid_flag = 0xFFFFFFFF;
    uint32_t addr;
    
    for(uint8_t i = 0; i < EEPROM_ELECT_STATIST_STORAGE_NUM; i++)
    {
        addr = EEPROM_FIRST_ELECT_STATIST_VALID_FLAG_ADDR + EEPROM_PER_ELECT_STATIST_STORAGE_SIZE * i;
        EepromWrite((uint8_t *)&invalid_flag, addr, 4);
    }
}



/** 
 * @brief    基础信息烧写
 * @param    p_buf,MODELID+SN,其中modelid为两个字节裸数据，sn为字符串
 * @param    size：数据长度
 * @return   none
 * @note:    none
 */
void BaseInfoStorage(uint8_t *p_buf, uint8_t size)
{
    uint8_t buf[256];
    //擦除
    SpiFlashEraseSector(FLASH_DEV_BASE_INFO_VALID_FLAG_ADDR);
    //写入数据
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_DEV_BASE_INFO_ADDR, p_buf, size);
    //写入标记
    memset(buf, 0x0A, 256);
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_DEV_BASE_INFO_VALID_FLAG_ADDR, buf, 256);
}

/** 
 * @brief    BMU采样掩码信息烧写
 * @param    p_buf：数据内容
 * @param    size：数据长度
 * @return   none
 * @note:    none
 */
void BMUSampleInfoStorge(BMUSampleInfo_t *p_sample_info)
{
    uint8_t buf[256];
    //擦除
    SpiFlashEraseSector(FLASH_BMU_SAMPLE_INFO_VALID_FLAG_ADDR);
    //写入数据
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_BMU_SAMPLE_INFO_ADDR, (uint8_t *)p_sample_info, sizeof(BMUSampleInfo_t));
    //写入标记
    memset(buf, 0x0A, 256);
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_BMU_SAMPLE_INFO_VALID_FLAG_ADDR, buf, 256);
}


/** 
 * @brief    电芯出厂信息烧写
 * @param    p_buf：数据内容
 * @param    size：数据长度
 * @return   none
 * @note:    none
 */
void PackFactInfoStorge(PackFactPara_t *p_pack_fact_info)
{
    uint8_t buf[256];
    //擦除
    SpiFlashEraseSector(FLASH_PACK_FACT_PARA_VALID_FLAG_ADDR);
    //写入数据
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_PACK_FACT_PARA_ADDR, (uint8_t *)p_pack_fact_info, sizeof(PackFactPara_t));
    //写入标记
    memset(buf, 0x0A, 256);
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_PACK_FACT_PARA_VALID_FLAG_ADDR, buf, 256);
}


/** 
 * @brief    故障阈值信息烧写
 * @param    p_buf：数据内容
 * @param    size：数据长度
 * @return   none
 * @note:    none
 */
void AlarmThresholdInfoStorge(AlarmThreshold_t *p_alarm_threshold_info)
{
    uint8_t buf[256];
    //擦除
    SpiFlashEraseSector(FLASH_ALARM_THRESHOLD_VALID_FLAG_ADDR);
    //写入数据
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_ALARM_THRESHOLD_ADDR, (uint8_t *)p_alarm_threshold_info, sizeof(AlarmThreshold_t));
    //写入标记
    memset(buf, 0x0A, 256);
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_ALARM_THRESHOLD_VALID_FLAG_ADDR, buf, 256);
}



/** 
 * @brief    BCU其他信息烧写
 * @param    p_buf：数据内容
 * @param    size：数据长度
 * @return   none
 * @note:    none
 */
void BCUFuncThresholdInfoStorge(BCUFuncThreshold_t *p_func_threshold_info)
{
    uint8_t buf[256];
    //擦除
    SpiFlashEraseSector(FLASH_BCU_FUNC_THRESHOLD_VALID_FLAG_ADDR);
    //写入数据
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_BCU_FUNC_THRESHOLD_ADDR, (uint8_t *)p_func_threshold_info, sizeof(BCUFuncThreshold_t));
    //写入标记
    memset(buf, 0x0A, 256);
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_BCU_FUNC_THRESHOLD_VALID_FLAG_ADDR, buf, 256);
}



/** 
 * @brief    BCU电流校准系数存储
 * @param    p_buf：数据内容
 * @param    size：数据长度
 * @return   none
 * @note:    none
 */
void BCUCurrCorrCoefStorge()
{
    uint8_t buf[256];
    //擦除
    SpiFlashEraseSector(FLASH_BCU_CORR_COEF_VALID_FLAG_ADDR);
    //写入数据
    memcpy(&buf[0], (uint8_t *)&gCurrentInfo.big_corr_coef_k, 4);
    memcpy(&buf[4], (uint8_t *)&gCurrentInfo.big_corr_coef_b, 4);
    memcpy(&buf[8], (uint8_t *)&gCurrentInfo.small_corr_coef_k, 4);
    memcpy(&buf[12], (uint8_t *)&gCurrentInfo.small_corr_coef_b, 4);
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_BCU_CORR_COEF_ADDR, buf, 16);
    //写入标记
    memset(buf, 0x0A, 256);
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_BCU_CORR_COEF_VALID_FLAG_ADDR, buf, 256);
}


/** 
 * @brief    BMU采样掩码信息清除
 * @param    none
 * @return   none
 * @note:    none
 */
void BMUSampleInfoClear()
{
    SpiFlashEraseSector(FLASH_BMU_SAMPLE_INFO_VALID_FLAG_ADDR);
}


/** 
 * @brief    电芯出厂信息清除
 * @param    none
 * @return   none
 * @note:    none
 */
void PackFactInfoClear()
{
    SpiFlashEraseSector(FLASH_PACK_FACT_PARA_VALID_FLAG_ADDR);
}



/** 
 * @brief    故障阈值信息清除
 * @param    none
 * @return   none
 * @note:    none
 */
void AlarmThresholdInfoClear()
{
    SpiFlashEraseSector(FLASH_ALARM_THRESHOLD_VALID_FLAG_ADDR);
}



/** 
 * @brief    BCU其他信息清除
 * @param    none
 * @return   none
 * @note:    none
 */
void BCUFuncThresholdInfoClear()
{
    SpiFlashEraseSector(FLASH_BCU_FUNC_THRESHOLD_VALID_FLAG_ADDR);
}



/** 
 * @brief    BCU电流校准系数清除
 * @param    none
 * @return   none
 * @note:    none
 */
void BCUCurrCorrCoefClear()
{
    SpiFlashEraseSector(FLASH_BCU_CORR_COEF_VALID_FLAG_ADDR);
}



/** 
 * @brief    上电从存储器加载相关数据
 * @param    none
 * @return   none
 * @note:
 */
void DataStorageInfoInit()
{
    uint8_t buf[32];
    /*从内部FLASH加载*/
    //固件版本号和读取
    InternalFlashByteRead(INTERNAL_FLASH_FW_INFO_ADDR, buf, 32);
	memcpy(buf_Version, &buf[4], 3);
    sprintf(gBMSDataInfo.fw_version, "%d.%d.%d", buf[4], buf[5], buf[6]);
    
    /*从SPI FLASH加载*/
    //MODELID和SN号读取
    if(_FlashDataValidJudge(FLASH_DEV_BASE_INFO_VALID_FLAG_ADDR) == true)
    {
        memset(buf, 0x00, sizeof(buf));
        SpiFlashReadData(FLASH_DEV_BASE_INFO_ADDR, buf, 23);
        gBMSDataInfo.model_id[0] =  buf[0];
        gBMSDataInfo.model_id[1] =  buf[1];
        strcpy(gBMSDataInfo.sn, (char *)&buf[2]);
    }
    else
    {
        gBMSDataInfo.model_id[0] =  0xC3;
        gBMSDataInfo.model_id[1] =  0x02;
        strcpy(gBMSDataInfo.sn, "24A01A100999999");
    }
    //bmu采样掩码信息
    if(_FlashDataValidJudge(FLASH_BMU_SAMPLE_INFO_VALID_FLAG_ADDR) == true)
    {
        SpiFlashReadData(FLASH_BMU_SAMPLE_INFO_ADDR, (uint8_t *)&gBMSDataInfo.bmu_sample_info, sizeof(BMUSampleInfo_t));
    }
    else
    {
        memcpy((uint8_t *)&gBMSDataInfo.bmu_sample_info, &_gBMUDefaultSampleInfo, sizeof(BMUSampleInfo_t));
    }
    //电芯出厂信息
    if(_FlashDataValidJudge(FLASH_PACK_FACT_PARA_VALID_FLAG_ADDR) == true)
    {
        SpiFlashReadData(FLASH_PACK_FACT_PARA_ADDR, (uint8_t *)&(gPackInfo.pack_fact_para), sizeof(PackFactPara_t));
    }
    else
    {
        memcpy((uint8_t *)&(gPackInfo.pack_fact_para), &_gPackDefaultFactPara, sizeof(PackFactPara_t));
    }
    //故障阈值信息
    if(_FlashDataValidJudge(FLASH_ALARM_THRESHOLD_VALID_FLAG_ADDR) == true)
    {
        SpiFlashReadData(FLASH_ALARM_THRESHOLD_ADDR, (uint8_t *)&gAlarmThreshold, sizeof(AlarmThreshold_t));
    }
    else
    {
        memcpy((uint8_t *)&gAlarmThreshold, (uint8_t *)&_gAlarmDefaultThreshold, sizeof(AlarmThreshold_t));
    }
    //BCU其他阈值信息
    if(_FlashDataValidJudge(FLASH_BCU_FUNC_THRESHOLD_VALID_FLAG_ADDR) == true)
    {
        SpiFlashReadData(FLASH_BCU_FUNC_THRESHOLD_ADDR, (uint8_t *)&gBMSDataInfo.bcu_func_threshold, sizeof(BCUFuncThreshold_t));
    }
    else
    {
        memcpy((uint8_t *)&gBMSDataInfo.bcu_func_threshold, (uint8_t *)&_gBCUFuncDefaultThreshold, sizeof(BCUFuncThreshold_t));
    }
    //BCU电流校准系数信息
    if(_FlashDataValidJudge(FLASH_BCU_CORR_COEF_VALID_FLAG_ADDR) == true)
    {
        uint8_t buf[16];
        
        SpiFlashReadData(FLASH_BCU_CORR_COEF_ADDR, buf, 16);
        memcpy((uint8_t *)&gCurrentInfo.big_corr_coef_k, &buf[0], 4);
        memcpy((uint8_t *)&gCurrentInfo.big_corr_coef_b, &buf[4], 4);
        memcpy((uint8_t *)&gCurrentInfo.small_corr_coef_k, &buf[8], 4);
        memcpy((uint8_t *)&gCurrentInfo.small_corr_coef_b, &buf[12], 4);
    }
    else
    {
        gCurrentInfo.big_corr_coef_k = 1;
        gCurrentInfo.big_corr_coef_b = 0;
        gCurrentInfo.small_corr_coef_k = 1;
        gCurrentInfo.small_corr_coef_b = 0;
    }
    
    
    /*从EEPROM加载*/
    //sox_info信息
    if(_EEPROMLatestSoxInfoGet() == true)
    {
        uint32_t addr = EEPROM_FIRST_SOX_INFO_VALID_FLAG_ADDR + EEPROM_SOX_INFO_STORAGE_OFFSET + EEPROM_PER_SOX_INFO_STORAGE_SIZE * _gSoxInfoLocation;
        EepromRead((uint8_t *)&(gPackInfo.sox_info), addr, sizeof(SOXInfo_t));
    }
    else
    {
        memcpy((uint8_t *)&(gPackInfo.sox_info), (uint8_t *)&_gSoxDefaultInfo, sizeof(SOXInfo_t));
    }
    //elect_statist信息
    if(_EEPROMLatestElectStatistGet() == true)
    {
        uint32_t addr = EEPROM_FIRST_ELECT_STATIST_VALID_FLAG_ADDR + EEPROM_ELECT_STATIST_STORAGE_OFFSET + EEPROM_PER_ELECT_STATIST_STORAGE_SIZE * _gElectStatistLocation;
        EepromRead((uint8_t *)&(gPackInfo.elect_statist), addr, sizeof(ElectStatist_t));
    }
    else
    {
        memset((uint8_t *)&(gPackInfo.elect_statist), 0x00, sizeof(ElectStatist_t));
    }
}



/** 
 * @brief    定时保存数据处理
 * @param    none
 * @return   none
 * @note:    none
 */
void ScheStorageDataProcess()
{
    static uint32_t sox_info_time_cnt = 0;
    static uint32_t elect_statist_time_cnt = 0;
    
    //sox_info，一分钟保存一次
    if((uint32_t)(tim_get_systime() - sox_info_time_cnt) > 1000 * 60)
    {
        sox_info_time_cnt = tim_get_systime();
        _EEPROMSoxInfoStorage();
    }
    //elect_statist，十分钟保存一次
    if((uint32_t)(tim_get_systime() - elect_statist_time_cnt) > 1000 * 600)
    {
        elect_statist_time_cnt = tim_get_systime();
        _EEPROMElectStatistStorage();
    }
}










