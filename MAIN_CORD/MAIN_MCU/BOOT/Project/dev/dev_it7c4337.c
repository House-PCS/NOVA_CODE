/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_it7c4337.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-11-22
  * @brief       GD32F450
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-11-22    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_it7c4337.h"

#define USE_SIMI2C      1

#define IT7C4337_ADDR     (0xD0U)

DevRtc_t gDevRtc;


//以1970年1月1日为基准
//1970~2099年为合法年份
//月份数据表			
static uint8_t const _table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
//平年的月份日期表
static const uint8_t _mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

static void _i2c_reg_write(uint8_t reg_addr, uint8_t value)
{
#if  (USE_SIMI2C == 0)
    /*标准I2C*/
    RtcHandle.pTxBuf = &value;
    RtcHandle.tx_size = 1;
    RtcHandle.reg_addr = reg_addr;

    I2CRegWrite(&RtcHandle);
    
#else
	/*模拟I2C*/
    RtcHandle.pTxBuf = &value;
	RtcHandle.tx_size = 1;
	RtcHandle.reg_addr = reg_addr;
	
	SimI2CRegWrite(&RtcHandle);
	
#endif
    
}

static uint8_t _i2c_reg_read(uint8_t reg_addr)
{
    uint8_t ret = 0;
#if  (USE_SIMI2C == 0)
    /*标准I2C*/
    RtcHandle.pRxBuf = &ret;
    RtcHandle.rx_size = 1;
    RtcHandle.reg_addr = reg_addr;

    I2CRegRead(&RtcHandle);
#else
	/*模拟I2C*/
    RtcHandle.pRxBuf = &ret;
	RtcHandle.rx_size = 1;
	RtcHandle.reg_addr = reg_addr;
	
	SimI2CRegRead(&RtcHandle);
#endif
    return ret;
}


static uint8_t _IsLeapYear(uint16_t year)
{
    if(year % 4 == 0) //必须能被4整除
    {
        if(year % 100 == 0) 
        {
            if(year % 400 == 0)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

/*0~6*/
static uint8_t _GetWeek(uint16_t year, uint8_t month, uint8_t day)
{
    uint16_t temp;
    uint8_t year_h, year_l;
    year_h = year / 100;
    year_l = year % 100;
    
    if(year_h > 19)
    {
        year_l += 100;
    }
    temp = year_l + year_l / 4;
    temp %= 7;
    temp = temp + day + _table_week[month - 1];
    if(year_l % 4 == 0 && month < 3)
    {
        temp--;
    }
    
    temp %= 7;
    if(temp == 0)
    {
        temp = 7;//星期天用7表示
    }
    return temp;
}

void RtcSecConvert2Date(DevRtc_t *pDate)
{
    static uint16_t daycnt = 0;
    uint32_t sec = pDate->time_stamp;
    uint32_t temp = 0;
    uint16_t base_year = 0, base_month = 0;
    uint32_t sec_temp;
    temp = sec / 86400;

    base_year = 1970;
    while(temp >= 365)
    {
        if(_IsLeapYear(base_year))//闰年
        {
            if(temp >= 366)
            {
                temp -= 366;
            }
            else
            {
//                    base_year++;
                break;
            }
        }
        else
        {
            temp -= 365;
        }
        base_year++;
    }
    pDate->year = base_year;
    
    base_month = 0;
    while(temp >= 28)
    {
        if(_IsLeapYear(base_year) && base_month == 1)
        {
            if(temp >= 29)
            {
                temp -= 29;
            }
            else
            {
                break;
            }
        }
        else
        {
            if(temp >= _mon_table[base_month])
            {
                temp -= _mon_table[base_month];
            }
            else
            {
                break;
            }
        }
        base_month++;
    }
    pDate->month = base_month + 1;
    pDate->day = temp + 1;
        
    sec_temp = sec % 86400;
    pDate->hour = sec_temp / 3600;
    pDate->minute = (sec_temp % 3600) / 60;
    pDate->second = (sec_temp % 3600) % 60;
    pDate->week = _GetWeek(pDate->year, pDate->month, pDate->day);
}

void RtcDat2Covert2Sec(DevRtc_t *pDate)
{
    uint16_t t;
	uint32_t sec_count = 0;
    if(pDate->year < 1970 || pDate->year > 2099)
    {
        return;	
    }   
    for(t = 1970;t < pDate->year;t++)	//把所有年份的秒钟相加
	{
		if(_IsLeapYear(t))
        {
            sec_count += 31622400;//闰年的秒钟数
        }
		else 
        {
            sec_count += 31536000;//平年的秒钟数
        }
	}
    for(t = 0;t < pDate->month - 1;t++)	   //把前面月份的秒钟数相加
    {
        sec_count += (uint32_t)_mon_table[t] * 86400;//月份秒钟数相加
        if(_IsLeapYear(pDate->year) && t == 1)
        {
            sec_count += 86400;//闰年2月份增加一天的秒钟数	   
        }
    }
    sec_count += (uint32_t)((pDate->day)-1)*86400;//把前面日期的秒钟数相加 
	sec_count += (uint32_t)(pDate->hour)*3600;//小时秒钟数
	sec_count += (uint32_t)(pDate->minute)*60;	 //分钟秒钟数
	sec_count += pDate->second ;//最后的秒钟加上去
    
    pDate->time_stamp = sec_count;
}




/**
  * @brief   读取RTC芯片的时间
  * @param   None
  * @return  None
  */
void DevRtcReadTime(DevRtc_t *pDate)
{
    uint8_t ret;
    
    ret = _i2c_reg_read(IT7C4337_YEAR_REG_ADDR);
    pDate->year = 2000 + (ret >> 4) * 10 + (ret & 0x0f);
    
    ret = _i2c_reg_read(IT7C4337_MONTH_REG_ADDR) & 0x1f;
    pDate->month = (ret >> 4) * 10 + (ret & 0x0f);
    
    ret = _i2c_reg_read(IT7C4337_DAY_REG_ADDR) & 0x3f;
    pDate->day = (ret >> 4) * 10 + (ret & 0x0f);
    
    pDate->week = _GetWeek(pDate->year, pDate->month, pDate->day);
    
    ret = _i2c_reg_read(IT7C4337_HOUR_REG_ADDR) & 0x1f;
    pDate->hour = (ret >> 4) * 10 + (ret & 0x0f);
    
    ret = _i2c_reg_read(IT7C4337_MIN_REG_ADDR) & 0x7f;
    pDate->minute = (ret >> 4) * 10 + (ret & 0x0f);
    
    ret = _i2c_reg_read(IT7C4337_SEC_REG_ADDR) & 0x7f;
    pDate->second = (ret >> 4) * 10 + (ret & 0x0f);
}

/**
  * @brief   写入RTC芯片的时间
  * @param   None
  * @return  None
  */
void DevRtcWriteTime(DevRtc_t *pDate)
{
    uint8_t data;
    //秒设置
    data = 0x00;
    data |= (pDate->second / 10) << 4;
    data |= (pDate->second % 10);
    _i2c_reg_write(0x00, data);
    //分钟设置
    data = 0x00;
    data |= (pDate->minute / 10) << 4;
    data |= (pDate->minute % 10);
    _i2c_reg_write(0x01, data);
    //小时设置
    data = 0x00;
    data |= (pDate->hour / 10) << 4;
    data |= (pDate->hour % 10);
    _i2c_reg_write(0x02, data);
    //星期设置
//    data = 0x00;
//    data = pDate->week;
//    data |= 0x18;
//    _i2c_reg_write(0x03, data);
    //日期设置
    data = 0x00;
    data |= (pDate->day / 10) << 4;
    data |= (pDate->day % 10);
    _i2c_reg_write(0x04, data);
    //月设置
    data = 0x00;
    data |= (pDate->month / 10) << 4;
    data |= (pDate->month % 10);
    _i2c_reg_write(0x05, data);
    //年设置
    data = 0x00;
    data |= ((pDate->year-2000) / 10) << 4;
    data |= ((pDate->year-2000) % 10);
    _i2c_reg_write(0x06, data);
}






/**
  * @brief   RTC秒级中断处理
  * @param   None
  * @return  None
  */
static bool _IrqRtcIntProcess()
{
    gDevRtc.time_stamp++;
//    if(RtcIntHandle.gpio_cfg->gpio_exti_triger == EXTI_TRIG_RISING)
//    {
//        if(GpioBitGet(&RtcIntHandle) == 1)
//        {
//            return true;
//        }
//        else
//        {
//            return false;
//        }
//    }
//    else if(RtcIntHandle.gpio_cfg->gpio_exti_triger == EXTI_TRIG_FALLING)
//    {
//        if(GpioBitGet(&RtcIntHandle) == 0)
//        {
//            return true;
//        }
//        else
//        {
//            return false;
//        }
//    }
//    else
//    {
//        return true;
//    }
    
    
    
    return true;
}



uint32_t GetRtcTimeStamp(void)
{
    return gDevRtc.time_stamp;
}


/**
  * @brief   初始化I2C和4337
  * @param   None
  * @return  None
  */
void DevRtcInit()
{
    RtcHandle.reg_addr_width = EN_SIMI2C_REG_ADDR_WIDTH_8;
    RtcHandle.slave_addr0 = IT7C4337_ADDR;
    
    RtcIntHandle.irq_process = _IrqRtcIntProcess;
    
    RtcBspInit();
    
    _i2c_reg_write(0x0E, 0x00);
    DevRtcReadTime(&gDevRtc);//上电获取一次时间
    RtcDat2Covert2Sec(&gDevRtc);//转换时间戳
    
}

void    CalibrationRtcTime(uint32_t stamp)
{
    gDevRtc.time_stamp = stamp;
    RtcSecConvert2Date(&gDevRtc);
    DevRtcWriteTime(&gDevRtc);
}



