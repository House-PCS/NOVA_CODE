/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_upstream_analysis.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-18
  * @brief    
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
#include "modbus_upstream_analysis.h"

ModUpstreamReadInfo_t gModUpstreamReadInfo;


/** 
* @brief  数据大小端转换
* @param  p_dst:转换后拷贝的存储指针
* @param  p_src:要转换的数据指针
* @param  len:转换的16bit数据数量
* @return none
* @note:
*/
static void _Little2Big16(uint16_t *p_dst, uint16_t *p_src, uint16_t len)
{
    uint8_t *p_dst_temp_8 = (uint8_t *)p_dst;
    uint8_t *p_src_temp_8 = (uint8_t *)p_src;
    
    for(uint16_t i = 0; i < len; i++)
    {
        p_dst[i] = __REV16(p_src[i]);
//        p_dst_temp_8[i * 2] = p_src_temp_8[i * 2 + 1];
//        p_dst_temp_8[i * 2 + 1] = p_src_temp_8[i * 2];
    }
}



/** 
* @brief  从控编址信息回读响应
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusBMUAddressingInfoReadAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint16_t *p_buf = (uint16_t *)&gModUpstreamReadInfo.bmu_addressing_info;
    
    p_buf += addr_offset;
    /*数据大小端转换并拷贝至待回复buf*/
    _Little2Big16(p_reg_value, p_buf, reg_len);
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}


/** 
* @brief  从控IO输出回读响应
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusBMUIOOutputInfoReadAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint16_t *p_buf = (uint16_t *)&gModUpstreamReadInfo.bmu_io_output_stat[0];
    
    p_buf += addr_offset;
    /*数据大小端转换并拷贝至待回复buf*/
    _Little2Big16(p_reg_value, p_buf, reg_len);
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}


/** 
* @brief  从控IO输入回读响应
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusBMUIOInputInfoReadAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint16_t *p_buf = (uint16_t *)&gModUpstreamReadInfo.bmu_io_input_stat[0];
    
    p_buf += addr_offset;
    /*数据大小端转换并拷贝至待回复buf*/
    _Little2Big16(p_reg_value, p_buf, reg_len);
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}


/** 
* @brief  电池统计回读响应
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusPackStatisReadAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint16_t *p_buf = (uint16_t *)&gModUpstreamReadInfo.pack_statis_info;
    
    if(gBMSDataInfo.bmu_calc_info.valid_flag != true)//数据尚未计算完
    {
        return EN_MODBUS_UPSTREAM_OP_STAT_BUSY;
    }
    
    p_buf += addr_offset;
    /*数据大小端转换并拷贝至待回复buf*/
    _Little2Big16(p_reg_value, p_buf, reg_len);
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}


/** 
* @brief  高压箱信号回读响应
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusBoxInfoReadAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint16_t *p_buf = (uint16_t *)&gModUpstreamReadInfo.box_info;
    
    p_buf += addr_offset;
    /*数据大小端转换并拷贝至待回复buf*/
    _Little2Big16(p_reg_value, p_buf, reg_len);
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}

/** 
* @brief  时间信息回读响应
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusRTCReadAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint16_t buf_temp[MODBUS_UPSTREAM_READ_RTC_MAX_LEN];
    
    buf_temp[0] = (gDevRtc.time_stamp >> 16);
    buf_temp[1] = gDevRtc.time_stamp;
    buf_temp[2] = gDevRtc.year;
    buf_temp[3] = gDevRtc.month;
    buf_temp[4] = gDevRtc.day;
    buf_temp[5] = gDevRtc.hour;
    buf_temp[6] = gDevRtc.minute;
    buf_temp[7] = gDevRtc.second;
    
    /*数据大小端转换并拷贝至待回复buf*/
    _Little2Big16(p_reg_value, buf_temp + addr_offset, reg_len);
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}

/** 
* @brief  BCU状态/控制信息回读响应
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusBCUStatCtlReadAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint16_t *p_buf = (uint16_t *)&gModUpstreamReadInfo.bcu_sys_info;
    
    p_buf += addr_offset;
    /*数据大小端转换并拷贝至待回复buf*/
    _Little2Big16(p_reg_value, p_buf, reg_len);
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}



/** 
* @brief  设备故障信息回读响应
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusAlarmReadAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint16_t buf_temp[MODBUS_UPSTREAM_READ_ALARM_MAX_LEN];
    
    buf_temp[0] = (gAlarmCode.measure_alarm_code[0] >> 16);
    buf_temp[1] = gAlarmCode.measure_alarm_code[0];
    buf_temp[2] = (gAlarmCode.measure_alarm_code[1] >> 16);
    buf_temp[3] = gAlarmCode.measure_alarm_code[1];
    buf_temp[4] = (gAlarmCode.measure_alarm_code[2] >> 16);
    buf_temp[5] = gAlarmCode.measure_alarm_code[2];
    buf_temp[6] = (gAlarmCode.dev_alarm_code[0] >> 16);
    buf_temp[7] = gAlarmCode.dev_alarm_code[0];
    buf_temp[8] = (gAlarmCode.dev_alarm_code[1] >> 16);
    buf_temp[9] = gAlarmCode.dev_alarm_code[1];
    buf_temp[10] = (gAlarmCode.dev_alarm_code[2] >> 16);
    buf_temp[11] = gAlarmCode.dev_alarm_code[2];
    buf_temp[12] = (gAlarmCode.other_alarm_code[0] >> 16);
    buf_temp[13] = gAlarmCode.other_alarm_code[0];
    buf_temp[14] = (gAlarmCode.other_alarm_code[1] >> 16);
    buf_temp[15] = gAlarmCode.other_alarm_code[1];
    buf_temp[16] = (gAlarmCode.other_alarm_code[2] >> 16);
    buf_temp[17] = gAlarmCode.other_alarm_code[2];
    
    /*数据大小端转换并拷贝至待回复buf*/
    _Little2Big16(p_reg_value, buf_temp + addr_offset, reg_len);
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}




/** 
* @brief  详细电压信息回读响应
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusDetailVoltReadAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint16_t buf_temp[MODBUS_UPSTREAM_READ_DETAIL_VOLT_MAX_LEN];
    
    if(gBMSDataInfo.bmu_calc_info.valid_flag != true)//数据尚未计算完
    {
        return EN_MODBUS_UPSTREAM_OP_STAT_BUSY;
    }
    for(uint16_t i = 0; i < gBMSDataInfo.per_bmu_cell_num * gBMSDataInfo.bmu_num; i++)
    {
        buf_temp[i] = gBMSDataInfo.bmu_detail_volt_info[i / gBMSDataInfo.per_bmu_cell_num].cell_volt[i % gBMSDataInfo.per_bmu_cell_num];
    }
    
    /*数据大小端转换并拷贝至待回复buf*/
    _Little2Big16(p_reg_value, buf_temp + addr_offset, reg_len);
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}



/** 
* @brief  详细温度信息回读响应
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusDetailTempReadAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint16_t buf_temp[MODBUS_UPSTREAM_READ_DETAIL_TEMP_MAX_LEN];
    
    if(gBMSDataInfo.bmu_calc_info.valid_flag != true)//数据尚未计算完
    {
        return EN_MODBUS_UPSTREAM_OP_STAT_BUSY;
    }
    for(uint16_t i = 0; i < gBMSDataInfo.per_bmu_cell_temp_num * gBMSDataInfo.bmu_num; i++)
    {
        buf_temp[i] = gBMSDataInfo.bmu_detail_temp_info[i / gBMSDataInfo.per_bmu_cell_num].cell_temp[i % gBMSDataInfo.per_bmu_cell_temp_num];
    }
    
    /*数据大小端转换并拷贝至待回复buf*/
    _Little2Big16(p_reg_value, buf_temp + addr_offset, reg_len);
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}




/** 
* @brief  电流校准信息回读响应
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusCorrCoefReadAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint16_t buf_temp[MODBUS_UPSTREAM_READ_CORR_COEF_MAX_LEN];
    uint32_t temp;
    
    memcpy((uint8_t *)temp, (uint8_t *)&gCurrentInfo.big_corr_coef_k, 4);
    buf_temp[0] = (temp >> 16) & 0xFFFF;
    buf_temp[1] = (temp) & 0xFFFF;
    memcpy((uint8_t *)temp, (uint8_t *)&gCurrentInfo.big_corr_coef_b, 4);
    buf_temp[2] = (temp >> 16) & 0xFFFF;
    buf_temp[3] = (temp) & 0xFFFF;
    memcpy((uint8_t *)temp, (uint8_t *)&gCurrentInfo.small_corr_coef_k, 4);
    buf_temp[4] = (temp >> 16) & 0xFFFF;
    buf_temp[5] = (temp) & 0xFFFF;
    memcpy((uint8_t *)temp, (uint8_t *)&gCurrentInfo.small_corr_coef_b, 4);
    buf_temp[6] = (temp >> 16) & 0xFFFF;
    buf_temp[7] = (temp) & 0xFFFF;
    
    /*数据大小端转换并拷贝至待回复buf*/
    _Little2Big16(p_reg_value, buf_temp + addr_offset, reg_len);
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}




static const ModUpstreamOpList_t _gModUpstreamReadList[] = 
{
    {MODBUS_UPSTREAM_READ_BMU_ADDRESSING_INFO_START_ADDR, 1, MODBUS_UPSTREAM_READ_BMU_ADDRESSING_INFO_MAX_LEN, _ModbusBMUAddressingInfoReadAnalysis},
    {MODBUS_UPSTREAM_READ_BMU_IOOUT_INFO_START_ADDR,      1, MODBUS_UPSTREAM_READ_BMU_IOOUT_INFO_MAX_LEN,      _ModbusBMUIOOutputInfoReadAnalysis},
    {MODBUS_UPSTREAM_READ_BMU_IOIN_INFO_START_ADDR,       1, MODBUS_UPSTREAM_READ_BMU_IOIN_INFO_MAX_LEN,       _ModbusBMUIOInputInfoReadAnalysis},
    {MODBUS_UPSTREAM_READ_PACK_STATIS_START_ADDR,         1, MODBUS_UPSTREAM_READ_PACK_STATIS_MAX_LEN,         _ModbusPackStatisReadAnalysis},
    {MODBUS_UPSTREAM_READ_BOX_INFO_START_ADDR,            1, MODBUS_UPSTREAM_READ_BOX_INFO_MAX_LEN,            _ModbusBoxInfoReadAnalysis},
    {MODBUS_UPSTREAM_READ_RTC_START_ADDR,                 1, MODBUS_UPSTREAM_READ_RTC_MAX_LEN,                 _ModbusRTCReadAnalysis},
    {MODBUS_UPSTREAM_READ_BCU_STAT_CTL_START_ADDR,        1, MODBUS_UPSTREAM_READ_BCU_STAT_CTL_MAX_LEN,        _ModbusBCUStatCtlReadAnalysis},
    {MODBUS_UPSTREAM_READ_ALARM_START_ADDR,               1, MODBUS_UPSTREAM_READ_ALARM_MAX_LEN,               _ModbusAlarmReadAnalysis},
    {MODBUS_UPSTREAM_READ_DETAIL_VOLT_START_ADDR,         1, MODBUS_UPSTREAM_READ_DETAIL_VOLT_MAX_LEN,         _ModbusDetailVoltReadAnalysis},
    {MODBUS_UPSTREAM_READ_DETAIL_TEMP_START_ADDR,         1, MODBUS_UPSTREAM_READ_DETAIL_TEMP_MAX_LEN,         _ModbusDetailTempReadAnalysis},
    {MODBUS_UPSTREAM_READ_CORR_COEF_ADDR,                 1, MODBUS_UPSTREAM_READ_CORR_COEF_MAX_LEN,           _ModbusCorrCoefReadAnalysis},
};



/** 
* @brief  MODBUS读数据有效性判断,如有效，则根据地址和数据长度进行数据回复处理
* @param  reg_addr:读寄存器起始地址
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
EN_MODBUS_UPSTREAM_OP_STAT ModbusUpstreamReadJudgeAndAnalysis(uint16_t reg_addr, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint8_t i = 0;

    /*读取的区域判断*/
    for(i = 0; i < sizeof(_gModUpstreamReadList)/sizeof(ModUpstreamOpList_t); i++)
    {
        if(reg_addr >= _gModUpstreamReadList[i].region_reg_start_addr && 
           reg_addr < _gModUpstreamReadList[i].region_reg_start_addr + _gModUpstreamReadList[i].region_reg_max_len)
        {
            if(reg_addr + reg_len > _gModUpstreamReadList[i].region_reg_start_addr + _gModUpstreamReadList[i].region_reg_max_len)//长度越界
            {
                return EN_MODBUS_UPSTREAM_OP_STAT_LEN_ERROR;
            }
            else if(reg_len < _gModUpstreamReadList[i].region_reg_min_len)
            {
                return EN_MODBUS_UPSTREAM_OP_STAT_LEN_ERROR;
            }
            else
            {
                return _gModUpstreamReadList[i].analysis_fun(reg_addr - _gModUpstreamReadList[i].region_reg_start_addr, reg_len, p_reg_value);
            }
        }
    }
    if(i >= sizeof(_gModUpstreamReadList)/sizeof(ModUpstreamOpList_t))
    {
        return EN_MODBUS_UPSTREAM_OP_STAT_ADDR_ERROR;
    }
}









/** 
* @brief  高压箱信号写操作处理
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusBoxWriteAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    while(reg_len > 0)
    {
        if(addr_offset == 0)//操作总正继电器
        {
            if(__REV16(p_reg_value[0]) == 0)//断开继电器
            {
                gBMSDataInfo.relay_p_stat = false;
                RelayPCtl(gBMSDataInfo.relay_p_stat);
            }
            else if(__REV16(p_reg_value[0]) == 1)//闭合继电器
            {
                gBMSDataInfo.relay_p_stat = true;
                RelayPCtl(gBMSDataInfo.relay_p_stat);
            }
            else//数据非法
            {
                return EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA;
            }
        }
        else if(addr_offset == 1)//操作总负继电器
        {
            //本设备没有总负继电器
        }
        else if(addr_offset == 2)//操作预充继电器
        {
            if(__REV16(p_reg_value[0]) == 0)//断开继电器
            {
                gBMSDataInfo.relay_pre_stat = false;
                RelayPreCtl(gBMSDataInfo.relay_pre_stat);
            }
            else if(__REV16(p_reg_value[0]) == 1)//闭合继电器
            {
                gBMSDataInfo.relay_pre_stat = true;
                RelayPreCtl(gBMSDataInfo.relay_pre_stat);
            }
            else//数据非法
            {
                return EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA;
            }
        }
        else if(addr_offset == 3)//操作断路器
        {
            if(__REV16(p_reg_value[0]) == 0)//释放脱扣
            {
                gBMSDataInfo.breaker_stat = false;
                BreakerCtl(gBMSDataInfo.breaker_stat);
            }
            else if(__REV16(p_reg_value[0]) == 1)//触发脱扣
            {
                gBMSDataInfo.breaker_stat = true;
                BreakerCtl(gBMSDataInfo.breaker_stat);
            }
            else//数据非法
            {
                return EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA;
            }
        }
        else if(addr_offset == 4)//操作风扇继电器
        {
            if(__REV16(p_reg_value[0]) == 0)//断开继电器
            {
                gBMSDataInfo.relay_fan_stat = false;
                RelayFanCtl(gBMSDataInfo.relay_fan_stat);
            }
            else if(__REV16(p_reg_value[0]) == 1)//闭合继电器
            {
                gBMSDataInfo.relay_fan_stat = true;
                RelayFanCtl(gBMSDataInfo.relay_fan_stat);
            }
            else//数据非法
            {
                return EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA;
            }
        }
        p_reg_value++;
        addr_offset++;
        reg_len--;
    }
    
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}


/** 
* @brief  时间戳校时写操作处理
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusTimeStampWriteAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint8_t *p_temp_8 = (uint8_t *)p_reg_value;
    
    gDevRtc.time_stamp = (p_temp_8[0] << 24) | (p_temp_8[1] << 16) | (p_temp_8[2] << 8) | p_temp_8[3];
    RtcSecConvert2Date(&gDevRtc);
    DevRtcWriteTime(&gDevRtc);
    
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}


/** 
* @brief  绝对时间校时写操作处理
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusTimeWriteAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    gDevRtc.year = __REV16(p_reg_value[0]);
    gDevRtc.month = __REV16(p_reg_value[1]);
    gDevRtc.day = __REV16(p_reg_value[2]);
    gDevRtc.hour = __REV16(p_reg_value[3]);
    gDevRtc.minute = __REV16(p_reg_value[4]);
    gDevRtc.second = __REV16(p_reg_value[5]);
    RtcDat2Covert2Sec(&gDevRtc);
    DevRtcWriteTime(&gDevRtc);
    
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}




/** 
* @brief  相对时间校时写操作处理
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusRelaveTimeWriteAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint8_t *p_temp_8 = (uint8_t *)p_reg_value;
    int32_t temp = (int32_t)((p_temp_8[0] << 24) | (p_temp_8[1] << 16) | (p_temp_8[2] << 8) | p_temp_8[3]);
    
    gDevRtc.time_stamp += temp;
    RtcSecConvert2Date(&gDevRtc);
    DevRtcWriteTime(&gDevRtc);
    
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}



/** 
* @brief  并簇退簇写操作处理
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusClusterWriteAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    if(__REV16(p_reg_value[0]) == 1)//并簇
    {
        gSysRunInfo.cluster_cmd = 1;
    }
    if(__REV16(p_reg_value[0]) == 2)//退簇
    {
        gSysRunInfo.cluster_cmd = 2;
    }
    else
    {
        return EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA;
    }
    
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}




/** 
* @brief  系统复位写操作处理
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusSysResetWriteAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    if(__REV16(p_reg_value[0]) == 1)
    {
        gBMSDataInfo.bcu_reset_flag = true;
    }
    else
    {
        return EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA;
    }
    
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}



/** 
* @brief  绝缘检测命令写操作处理
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusInsResWriteAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    if(__REV16(p_reg_value[0]) == 1)//关闭绝缘检测
    {
        gBMSDataInfo.running_ins_res_cmd = false;
    }
    if(__REV16(p_reg_value[0]) == 2)//开启绝缘检测
    { 
        gBMSDataInfo.running_ins_res_cmd = true;
    }
    else
    {
        return EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA;
    }
    
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}


/** 
* @brief  清除存储数据命令写操作处理
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusClearStorageWriteAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint16_t temp = __REV16(p_reg_value[0]);
    
    if(temp == 0x0100)//清除FLASH下述全部信息
    {
        BMUSampleInfoClear();
        PackFactInfoClear();
        AlarmThresholdInfoClear();
        BCUFuncThresholdInfoClear();
    }
    else if(temp == 0x0101)//清除FLASH采样掩码信息
    {
        BMUSampleInfoClear();
    }
    else if(temp == 0x0102)//清除FLASH电芯出厂参数
    {
        PackFactInfoClear();
    }
    else if(temp == 0x0103)//清除FLASH故障阈值信息
    {
        AlarmThresholdInfoClear();
    }
    else if(temp == 0x0104)//清除FLASH BCU其他阈值信息
    {
        BCUFuncThresholdInfoClear();
    }
    else if(temp == 0x0200)//清除EEPROM下述全部信息
    {
        SoxInfoStorageClear();
        ElectStatistStorageClear();
    }
    else if(temp == 0x0201)//清除EEPROM SOX_INFO信息
    {
        SoxInfoStorageClear();
    }
    else if(temp == 0x0202)//清除EEPROM elect_statist信息
    {
        ElectStatistStorageClear();
    }
    else if(temp == 0x0300)//清除FLASH和EEPROM上述全部信息
    {
        BMUSampleInfoClear();
        PackFactInfoClear();
        AlarmThresholdInfoClear();
        BCUFuncThresholdInfoClear();
        SoxInfoStorageClear();
        ElectStatistStorageClear();
    }
    else
    {
        return EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA;
    }
    
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}



/** 
* @brief  电流校准系数锁存命令写操作处理
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusCorrCoefLatchWriteAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    if(gCurrentInfo.big_corr_coef_new_flag == true || gCurrentInfo.small_corr_coef_new_flag == true)
    {
        BCUCurrCorrCoefStorge();
    }
    
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}


/** 
* @brief  电流校准系数清除命令写操作处理
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusCorrCoefClearWriteAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    BCUCurrCorrCoefClear();
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}



/** 
* @brief  大量程电流校准系数数据写操作处理
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusHallBigCorrCoefWriteAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    float k, b;
    uint8_t temp[4];
    
    temp[0] = (p_reg_value[1] >> 8) & 0xFF;
    temp[1] = p_reg_value[1] & 0xFF;
    temp[2] = (p_reg_value[0] >> 8) & 0xFF;
    temp[3] = p_reg_value[0] & 0xFF;
    memcpy((uint8_t *)&k, temp, 4);
    temp[0] = (p_reg_value[3] >> 8) & 0xFF;
    temp[1] = p_reg_value[3] & 0xFF;
    temp[2] = (p_reg_value[2] >> 8) & 0xFF;
    temp[3] = p_reg_value[2] & 0xFF;
    memcpy((uint8_t *)&b, temp, 4);
    if(k >= 0.5 && k <= 2 && b <= 10 && b >= -10)//系数进行限制
    {
        gCurrentInfo.big_corr_coef_k = k;
        gCurrentInfo.big_corr_coef_b = b;
        gCurrentInfo.big_corr_coef_new_flag = true;
    }
    else
    {
        return EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA;
    }
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}


/** 
* @brief  小量程电流校准系数数据写操作处理
* @param  addr_offset:对应区域的寄存器地址偏移，从0开始
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
static EN_MODBUS_UPSTREAM_OP_STAT _ModbusHallSmallCorrCoefWriteAnalysis(uint16_t addr_offset, uint16_t reg_len, uint16_t *p_reg_value)
{
    float k, b;
    uint8_t temp[4];
    
    temp[0] = (p_reg_value[1] >> 8) & 0xFF;
    temp[1] = p_reg_value[1] & 0xFF;
    temp[2] = (p_reg_value[0] >> 8) & 0xFF;
    temp[3] = p_reg_value[0] & 0xFF;
    memcpy((uint8_t *)&k, temp, 4);
    temp[0] = (p_reg_value[3] >> 8) & 0xFF;
    temp[1] = p_reg_value[3] & 0xFF;
    temp[2] = (p_reg_value[2] >> 8) & 0xFF;
    temp[3] = p_reg_value[2] & 0xFF;
    memcpy((uint8_t *)&b, temp, 4);
    if(k >= 0.5 && k <= 2 && b <= 10 && b >= -10)//系数进行限制
    {
        gCurrentInfo.small_corr_coef_k = k;
        gCurrentInfo.small_corr_coef_b = b;
        gCurrentInfo.small_corr_coef_new_flag = true;
    }
    else
    {
        return EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA;
    }
    return EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS;
}




static const ModUpstreamOpList_t _gModUpstreamWriteList[] = 
{
    {MODBUS_UPSTREAM_WRITE_BOX_ADDR,                    MODBUS_UPSTREAM_WRITE_BOX_MIN_LEN,               MODBUS_UPSTREAM_WRITE_BOX_MAX_LEN,               _ModbusBoxWriteAnalysis},
    {MODBUS_UPSTREAM_WRITE_TIMESTAMP_ADDR,              MODBUS_UPSTREAM_WRITE_TIMESTAMP_LEN,             MODBUS_UPSTREAM_WRITE_TIMESTAMP_LEN,             _ModbusTimeStampWriteAnalysis},
    {MODBUS_UPSTREAM_WRITE_TIME_ADDR,                   MODBUS_UPSTREAM_WRITE_TIME_LEN,                  MODBUS_UPSTREAM_WRITE_TIME_LEN,                  _ModbusTimeWriteAnalysis},
    {MODBUS_UPSTREAM_WRITE_RELATIVE_TIMESTAMP_ADDR,     MODBUS_UPSTREAM_WRITE_RELATIVE_TIMESTAMP_LEN,    MODBUS_UPSTREAM_WRITE_RELATIVE_TIMESTAMP_LEN,    _ModbusRelaveTimeWriteAnalysis},
    {MODBUS_UPSTREAM_WRITE_CLUSTER_ADDR,                MODBUS_UPSTREAM_WRITE_CLUSTER_LEN,               MODBUS_UPSTREAM_WRITE_CLUSTER_LEN,               _ModbusClusterWriteAnalysis},
    {MODBUS_UPSTREAM_WRITE_RESET_ADDR,                  MODBUS_UPSTREAM_WRITE_RESET_LEN,                 MODBUS_UPSTREAM_WRITE_RESET_LEN,                 _ModbusSysResetWriteAnalysis},
    {MODBUS_UPSTREAM_WRITE_INS_RES_ADDR,                MODBUS_UPSTREAM_WRITE_INS_RES_LEN,               MODBUS_UPSTREAM_WRITE_INS_RES_LEN,               _ModbusInsResWriteAnalysis},
    {MODBUS_UPSTREAM_WRITE_CLEAR_STORAGE_ADDR,          MODBUS_UPSTREAM_WRITE_CLEAR_STORAGE_LEN,         MODBUS_UPSTREAM_WRITE_CLEAR_STORAGE_LEN,         _ModbusClearStorageWriteAnalysis},
    {MODBUS_UPSTREAM_WRITE_LATCH_CORR_COEF_ADDR,        MODBUS_UPSTREAM_WRITE_LATCH_CORR_COEF_LEN,       MODBUS_UPSTREAM_WRITE_LATCH_CORR_COEF_LEN,       _ModbusCorrCoefLatchWriteAnalysis},
    {MODBUS_UPSTREAM_WRITE_CLEAR_CORR_COEF_ADDR,        MODBUS_UPSTREAM_WRITE_CLEAR_CORR_COEF_LEN,       MODBUS_UPSTREAM_WRITE_CLEAR_CORR_COEF_LEN,       _ModbusCorrCoefClearWriteAnalysis},
    {MODBUS_UPSTREAM_WRITE_HALL_BIG_CORR_COEF_ADDR,     MODBUS_UPSTREAM_WRITE_HALL_BIG_CORR_COEF_LEN,    MODBUS_UPSTREAM_WRITE_HALL_BIG_CORR_COEF_LEN,    _ModbusHallBigCorrCoefWriteAnalysis},
    {MODBUS_UPSTREAM_WRITE_HALL_SMALL_CORR_COEF_ADDR,   MODBUS_UPSTREAM_WRITE_HALL_SMALL_CORR_COEF_LEN,  MODBUS_UPSTREAM_WRITE_HALL_SMALL_CORR_COEF_LEN,  _ModbusHallSmallCorrCoefWriteAnalysis},
};





/** 
* @brief  MODBUS写数据有效性判断,如有效，则执行写操作
* @param  reg_addr:读寄存器起始地址
* @param  reg_len:要读的寄存器长度
* @param  p_reg_value:读数据指针
* @return 操作结果
* @note:
*/
EN_MODBUS_UPSTREAM_OP_STAT ModbusUpstreamWriteJudgeAndAnalysis(uint16_t reg_addr, uint16_t reg_len, uint16_t *p_reg_value)
{
    uint8_t i = 0;
    
    //写入地址判断
    for(i = 0; i < sizeof(_gModUpstreamWriteList)/sizeof(ModUpstreamOpList_t); i++)
    {
        if(reg_addr >= _gModUpstreamWriteList[i].region_reg_start_addr && 
           reg_addr < _gModUpstreamWriteList[i].region_reg_start_addr + _gModUpstreamWriteList[i].region_reg_max_len)
        {
            if(reg_len < _gModUpstreamWriteList[i].region_reg_min_len)
            {
                return EN_MODBUS_UPSTREAM_OP_STAT_LEN_ERROR;
            }
            else if(reg_addr + reg_len > _gModUpstreamWriteList[i].region_reg_start_addr + _gModUpstreamWriteList[i].region_reg_max_len)
            {
                return EN_MODBUS_UPSTREAM_OP_STAT_LEN_ERROR;
            }
            else
            {
                return _gModUpstreamWriteList[i].analysis_fun(reg_addr - _gModUpstreamWriteList[i].region_reg_start_addr, reg_len, p_reg_value);
            }
        }
    }
    if(i >= sizeof(_gModUpstreamWriteList)/sizeof(ModUpstreamOpList_t))
    {
        return EN_MODBUS_UPSTREAM_OP_STAT_ADDR_ERROR;
    }
}



/** 
* @brief  MODBUS读操作数据整合大轮询处理
* @param  none
* @return none
* @note:
*/
void ModbusUpstreamReadInfoProcess()
{
    /*从控编址信息*/
    gModUpstreamReadInfo.bmu_addressing_info.addressing_res = gBMSDataInfo.bmu_adressing_res;
    gModUpstreamReadInfo.bmu_addressing_info.bmu_num = gBMSDataInfo.bmu_num;
    gModUpstreamReadInfo.bmu_addressing_info.per_bmu_cell_num = gBMSDataInfo.per_bmu_cell_num;
    gModUpstreamReadInfo.bmu_addressing_info.per_bmu_temp_num = gBMSDataInfo.per_bmu_cell_temp_num;
    gModUpstreamReadInfo.bmu_addressing_info.per_bmu_pole_temp_num = gBMSDataInfo.per_bmu_pole_temp_num;
    gModUpstreamReadInfo.bmu_addressing_info.per_bmu_other_temp_num = gBMSDataInfo.per_bmu_other_temp_num;
    
    /*从控IO输出信息和从控IO输入信息*/
    for(uint8_t i = 0; i < gModUpstreamReadInfo.bmu_addressing_info.bmu_num; i++)
    {
        gModUpstreamReadInfo.bmu_io_output_stat[i].io_output_stat = gBMSDataInfo.bmu_io_info[i].io_output_stat;
        gModUpstreamReadInfo.bmu_io_output_stat[i].pwm1_output_duty = gBMSDataInfo.bmu_io_info[i].pwm_1_output_duty;
        gModUpstreamReadInfo.bmu_io_output_stat[i].pwm2_output_duty = gBMSDataInfo.bmu_io_info[i].pwm_2_output_duty;
        
        gModUpstreamReadInfo.bmu_io_input_stat[i].io_input_stat = gBMSDataInfo.bmu_io_info[i].io_input_stat;
        gModUpstreamReadInfo.bmu_io_input_stat[i].pwm1_input_duty = gBMSDataInfo.bmu_io_info[i].pwm_1_input_duty;
        gModUpstreamReadInfo.bmu_io_input_stat[i].pwm2_input_duty = gBMSDataInfo.bmu_io_info[i].pwm_2_input_duty;
    }
    
    /*电池统计信息*/
    gModUpstreamReadInfo.pack_statis_info.total_volt = gBMSDataInfo.bmu_calc_info.cell_total_volt / 100;
    gModUpstreamReadInfo.pack_statis_info.current = gPackInfo.current / 100;
    gModUpstreamReadInfo.pack_statis_info.mesure_current_big = gCurrentInfo.filter_big_range_current / 100;
    gModUpstreamReadInfo.pack_statis_info.mesure_current_small = gCurrentInfo.filter_small_range_current / 100;
    gModUpstreamReadInfo.pack_statis_info.soc_real = gPackInfo.sox_info.soc_real * 10;
    gModUpstreamReadInfo.pack_statis_info.soc_disp = gPackInfo.sox_info.soc_disp * 10;
    gModUpstreamReadInfo.pack_statis_info.soh = gPackInfo.sox_info.soh * 10;
    gModUpstreamReadInfo.pack_statis_info.chg_cycle = gPackInfo.sox_info.chg_cycle;
    gModUpstreamReadInfo.pack_statis_info.dsg_cycle = gPackInfo.sox_info.dsg_cycle;
    gModUpstreamReadInfo.pack_statis_info.ins_res_p = gInsResInfo.bp_res;
    gModUpstreamReadInfo.pack_statis_info.ins_res_n = gInsResInfo.bn_res;
    gModUpstreamReadInfo.pack_statis_info.cell_max_volt_num = gBMSDataInfo.bmu_calc_info.cell_max_volt_num;
    gModUpstreamReadInfo.pack_statis_info.cell_max_volt = gBMSDataInfo.bmu_calc_info.cell_max_volt;
    gModUpstreamReadInfo.pack_statis_info.cell_min_volt_num = gBMSDataInfo.bmu_calc_info.cell_min_volt_num;
    gModUpstreamReadInfo.pack_statis_info.cell_min_volt = gBMSDataInfo.bmu_calc_info.cell_min_volt;
    gModUpstreamReadInfo.pack_statis_info.cell_max_temp_num = gBMSDataInfo.bmu_calc_info.cell_max_temp_num;
    gModUpstreamReadInfo.pack_statis_info.cell_max_temp = gBMSDataInfo.bmu_calc_info.cell_max_temp;
    gModUpstreamReadInfo.pack_statis_info.cell_min_temp_num = gBMSDataInfo.bmu_calc_info.cell_min_temp_num;
    gModUpstreamReadInfo.pack_statis_info.cell_min_temp = gBMSDataInfo.bmu_calc_info.cell_min_temp;
    gModUpstreamReadInfo.pack_statis_info.cell_max_volt_diff = gBMSDataInfo.bmu_calc_info.cell_max_volt - gBMSDataInfo.bmu_calc_info.cell_min_volt;
    gModUpstreamReadInfo.pack_statis_info.cell_max_temp_diff = gBMSDataInfo.bmu_calc_info.cell_max_temp - gBMSDataInfo.bmu_calc_info.cell_min_temp;
    gModUpstreamReadInfo.pack_statis_info.cell_max_pole_temp_num = gBMSDataInfo.bmu_calc_info.pole_max_temp_num;
    gModUpstreamReadInfo.pack_statis_info.cell_max_pole_temp = gBMSDataInfo.bmu_calc_info.pole_max_temp;
    gModUpstreamReadInfo.pack_statis_info.cell_min_pole_temp_num = gBMSDataInfo.bmu_calc_info.pole_min_temp_num;
    gModUpstreamReadInfo.pack_statis_info.cell_min_pole_temp = gBMSDataInfo.bmu_calc_info.pole_min_temp;
    gModUpstreamReadInfo.pack_statis_info.cell_max_other_temp_num = 0;//暂不支持
    gModUpstreamReadInfo.pack_statis_info.cell_max_other_temp = 0;//暂不支持
    gModUpstreamReadInfo.pack_statis_info.cell_min_other_temp_num = 0;//暂不支持
    gModUpstreamReadInfo.pack_statis_info.cell_min_other_temp = 0;//暂不支持
    gModUpstreamReadInfo.pack_statis_info.cell_ave_volt = gBMSDataInfo.bmu_calc_info.cell_ave_volt;
    gModUpstreamReadInfo.pack_statis_info.cell_ave_temp = gBMSDataInfo.bmu_calc_info.cell_ave_temp;
    gModUpstreamReadInfo.pack_statis_info.chg_cur_day_accum_e_high    = ((uint32_t)(gPackInfo.elect_statist.chg_cur_day_accum_e / 100) >> 16);
    gModUpstreamReadInfo.pack_statis_info.chg_cur_day_accum_e_low     = ((uint32_t)(gPackInfo.elect_statist.chg_cur_day_accum_e / 100));
    gModUpstreamReadInfo.pack_statis_info.dsg_cur_day_accum_e_high    = ((uint32_t)(gPackInfo.elect_statist.dsg_cur_day_accum_e / 100) >> 16);
    gModUpstreamReadInfo.pack_statis_info.dsg_cur_day_accum_e_low     = ((uint32_t)(gPackInfo.elect_statist.dsg_cur_day_accum_e / 100));
    gModUpstreamReadInfo.pack_statis_info.chg_cur_month_accum_e_high  = ((uint32_t)(gPackInfo.elect_statist.chg_cur_month_accum_e / 100) >> 16);
    gModUpstreamReadInfo.pack_statis_info.chg_cur_month_accum_e_low   = ((uint32_t)(gPackInfo.elect_statist.chg_cur_month_accum_e / 100));
    gModUpstreamReadInfo.pack_statis_info.dsg_cur_month_accum_e_high  = ((uint32_t)(gPackInfo.elect_statist.dsg_cur_month_accum_e / 100) >> 16);
    gModUpstreamReadInfo.pack_statis_info.dsg_cur_month_accum_e_low   = ((uint32_t)(gPackInfo.elect_statist.dsg_cur_month_accum_e / 100));
    gModUpstreamReadInfo.pack_statis_info.chg_cur_year_accum_e_high   = ((uint32_t)(gPackInfo.elect_statist.chg_cur_year_accum_e / 100) >> 16);
    gModUpstreamReadInfo.pack_statis_info.chg_cur_year_accum_e_low    = ((uint32_t)(gPackInfo.elect_statist.chg_cur_year_accum_e / 100));
    gModUpstreamReadInfo.pack_statis_info.dsg_cur_year_accum_e_high   = ((uint32_t)(gPackInfo.elect_statist.dsg_cur_year_accum_e / 100) >> 16);
    gModUpstreamReadInfo.pack_statis_info.dsg_cur_year_accum_e_low    = ((uint32_t)(gPackInfo.elect_statist.dsg_cur_year_accum_e / 100));
    gModUpstreamReadInfo.pack_statis_info.chg_total_accum_e_high      = ((uint32_t)(gPackInfo.elect_statist.chg_total_accum_e / 100) >> 16);
    gModUpstreamReadInfo.pack_statis_info.chg_total_accum_e_low       = ((uint32_t)(gPackInfo.elect_statist.chg_total_accum_e / 100));
    gModUpstreamReadInfo.pack_statis_info.dsg_total_accum_e_high      = ((uint32_t)(gPackInfo.elect_statist.dsg_total_accum_e / 100) >> 16);
    gModUpstreamReadInfo.pack_statis_info.dsg_total_accum_e_low       = ((uint32_t)(gPackInfo.elect_statist.dsg_total_accum_e / 100));
    gModUpstreamReadInfo.pack_statis_info.chg_total_accum_q_high      = ((uint32_t)(gPackInfo.elect_statist.chg_total_accum_q / 100) >> 16);
    gModUpstreamReadInfo.pack_statis_info.chg_total_accum_q_low       = ((uint32_t)(gPackInfo.elect_statist.chg_total_accum_q / 100));
    gModUpstreamReadInfo.pack_statis_info.dsg_total_accum_q_high      = ((uint32_t)(gPackInfo.elect_statist.dsg_total_accum_q / 100) >> 16);
    gModUpstreamReadInfo.pack_statis_info.dsg_total_accum_q_low       = ((uint32_t)(gPackInfo.elect_statist.dsg_total_accum_q / 100));
    
    /*高压箱控制信息*/
    gModUpstreamReadInfo.box_info.relay_p_stat = RelayPFeedback();
    gModUpstreamReadInfo.box_info.relay_n_stat = true;//无总负继电器，默认认为闭合
    gModUpstreamReadInfo.box_info.relay_pre_stat = RelayPreFeedback();
    gModUpstreamReadInfo.box_info.qf_stat = BreakerFeedback();
    gModUpstreamReadInfo.box_info.relay_fan_stat = true;//无反馈，默认认为闭合
    
    /*BCU系统信息*/
    if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_IDLE || gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_WAIT)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_run_stat = 0;
    }
    else if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_BMU_ADDRESSING)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_run_stat = 1;
    }
    else if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_WAIT_FULL_REPORT)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_run_stat = 2;
    }
    else if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_HV_EXAM || gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_INS_RES_DETECT)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_run_stat = 3;
    }
    else if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_WAIT_CLUSTER_CMD)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_run_stat = 4;
    }
    else if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_RELAY_PRE_ON)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_run_stat = 5;
    }
    else if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_RELAY_P_ON || gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_NORMAL_RUN)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_run_stat = 6;
    }
    else if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_QUIT_CLUSTER_CMD || gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_QUIT_CLUSTER_CUT_OFF_BREAKER ||
            gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_QUIT_CLUSTER_CUT_OFF_RELAY)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_run_stat = 7;
    }
    else if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_FAIL)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_run_stat = 255;
    }
    gModUpstreamReadInfo.bcu_sys_info.sys_mark = 0;
    //系统正常运行
    if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_NORMAL_RUN)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_mark |= (1 << 0);
    }
    else
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_mark &= ~(1 << 0);
    }
    //充电完成
    if(gPackInfo.full_flag == true)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_mark |= (1 << 1);
    }
    else
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_mark &= ~(1 << 1);
    }
    //放电完成
    if(gPackInfo.empty_flag == true)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_mark |= (1 << 2);
    }
    else
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_mark &= ~(1 << 2);
    }
    if(gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] != 0 || 
       gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] != 0 ||
       gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_1] != 0)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_mark |= (1 << 3);
    }
    else
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_mark &= ~(1 << 3);
    }
    //二级报警
    if(gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] != 0 || 
       gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_2] != 0 ||
       gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_2] != 0)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_mark |= (1 << 4);
    }
    else
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_mark &= ~(1 << 4);
    }
    //三级报警
    if(gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] != 0 || 
       gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_3] != 0 ||
       gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_3] != 0)
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_mark |= (1 << 5);
    }
    else
    {
        gModUpstreamReadInfo.bcu_sys_info.sys_mark &= ~(1 << 5);
    }
}





























