/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_sd2506.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-06-19
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-02    <td>V1.0.0    <td>           <td>InitialVersion 
  * </table>
  ******************************************************************************
  */
#include "dev_sd2506.h"
#include "systick.h"
#include <time.h>

#define  USE_SIM_IIC 0

#define SD25XX_DEV_ADDR     (0x64)

DevRtc_t gDevRtc;

static bool _IrqRtcIntProcess(void);

#define SD25XX_SEC_REG_ADDR        (0x00U)
#define SD25XX_MIN_REG_ADDR        (0x01U)
#define SD25XX_HOUR_REG_ADDR       (0x02U)
#define SD25XX_WEEK_REG_ADDR       (0X03U)
#define SD25XX_DAY_REG_ADDR        (0x04U)
#define SD25XX_MONTH_REG_ADDR      (0x05U)
#define SD25XX_YEAR_REG_ADDR       (0x06U)
#define SD25XX_CTR1_ADDR           (0x0FU)
#define SD25XX_CTR2_ADDR           (0x10U)
#define SD25XX_CTR3_ADDR           (0x11U)
#define SD25XX_REG_CHARGE          (0x18U)

//以1970年1月1日为基准
//1970~2099年为合法年份
//月份数据表			
static uint8_t const _table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
//平年的月份日期表
static const uint8_t _mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

/**
  * @brief   RTC寄存器写操作
  * @param   reg_addr：寄存器地址
  * @param   value：寄存器要写入的值
  * @return  None
  */
static void _i2c_reg_write(uint8_t reg_addr, uint8_t *p_value, uint8_t size)
{
#if  (USE_SIM_IIC == 0)
    /*标准I2C*/
    gRtcI2CHandle.pTxBuf = p_value;
    gRtcI2CHandle.tx_size = size;
    gRtcI2CHandle.reg_addr = reg_addr;

    I2CRegWrite(&gRtcI2CHandle);
    
#else
	/*模拟I2C*/
    gSimRtcI2CHandle.pTxBuf = p_value;
	gSimRtcI2CHandle.tx_size = size;
	gSimRtcI2CHandle.reg_addr = reg_addr;
	
	SimI2CRegWrite(&gSimRtcI2CHandle);
	
#endif
    
}

/**
  * @brief   RTC寄存器读操作
  * @param   reg_addr：寄存器地址
  * @return  要读出的值
  */
static void _i2c_reg_read(uint8_t reg_addr, uint8_t *p_value, uint8_t size)
{
#if  (USE_SIMI2C == 0)
    /*标准I2C*/
    gRtcI2CHandle.pRxBuf = p_value;
    gRtcI2CHandle.rx_size = size;
    gRtcI2CHandle.reg_addr = reg_addr;

    I2CRegRead(&gRtcI2CHandle);
#else
	/*模拟I2C*/
    gSimRtcI2CHandle.pRxBuf = p_value;
	gSimRtcI2CHandle.rx_size = size;
	gSimRtcI2CHandle.reg_addr = reg_addr;
	
	SimI2CRegRead(&gSimRtcI2CHandle);
#endif
}

/**
  * @brief   闰年判断
  * @param   year：年份
  * @return  1表示闰年，0表示平年
  */
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


/**
  * @brief   星期计算
  * @param   year：年份 month：月份  day：日
  * @return  星期，1~7
  */
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

/**
  * @brief   时间戳转换为详细时间
  * @param   pDate：时间总结构体
  * @return  NULL
  */
void RtcSecConvert2Date(DevRtc_t *pDate)
{
    time_t timestamp = pDate->time_stamp; // 获取当前时间戳
    struct tm *tm_local = localtime(&timestamp); // 转换为本地时间
    uint32_t sec_temp;
    
    sec_temp = pDate->time_stamp % 86400;
    
    pDate->year = tm_local->tm_year + 1900;
    pDate->month = tm_local->tm_mon + 1;
    pDate->day = tm_local->tm_mday;
    pDate->hour = tm_local->tm_hour;
    pDate->minute = tm_local->tm_min;
    pDate->second = tm_local->tm_sec;
    pDate->week = _GetWeek(pDate->year, pDate->month, pDate->day);
}

/**
  * @brief   详细时间转换为时间戳
  * @param   pDate：时间总结构体
  * @return  NULL
  */
void RtcDat2Covert2Sec(DevRtc_t *pDate)
{
    struct tm date = {0};
    time_t stamp;
    
    date.tm_year = pDate->year - 1900; 
    date.tm_mon = pDate->month - 1;  
    date.tm_mday = pDate->day; 
    
    stamp = mktime(&date);
    stamp += (pDate->hour)*3600;
    stamp += (pDate->minute)*60;
    stamp += pDate->second;
    pDate->time_stamp = stamp;
}


/**
  * @brief   读取RTC芯片的时间
  * @param   pDate：时间总结构体
  * @return  None
  */
void DevRtcReadTime(DevRtc_t *pDate)
{
    uint8_t buf[7];
    
    _i2c_reg_read(SD25XX_SEC_REG_ADDR, buf, 7);
    pDate->year = 2000 + (buf[6] >> 4) * 10 + (buf[6] & 0x0f);
    pDate->month = (buf[5] >> 4) * 10 + (buf[5] & 0x0f);
    pDate->day = (buf[4] >> 4) * 10 + (buf[4] & 0x0f);
    pDate->hour = ((buf[2] >> 4) & 0x07) * 10 + (buf[2] & 0x0f);
    pDate->minute = (buf[1] >> 4) * 10 + (buf[1] & 0x0f);
    pDate->second = (buf[0] >> 4) * 10 + (buf[0] & 0x0f);
    pDate->week = _GetWeek(pDate->year, pDate->month, pDate->day);
}

/**
  * @brief   写入RTC芯片的时间
  * @param   pDate：时间总结构体
  * @return  None
  */
void DevRtcWriteTime(DevRtc_t *pDate)
{
    uint8_t buf[7];
    
    buf[0] = ((pDate->second / 10) << 4) | (pDate->second % 10);
    buf[1] = ((pDate->minute / 10) << 4) | (pDate->minute % 10);
    buf[2] = ((pDate->hour / 10) << 4) | (pDate->hour % 10) | (1 << 7);
    buf[3] = pDate->week % 7;
    buf[4] = ((pDate->day / 10) << 4) | (pDate->day % 10);
    buf[5] = ((pDate->month / 10) << 4) | (pDate->month % 10);
    buf[6] = ((pDate->year / 10) << 4) | (pDate->year % 10);
    _i2c_reg_write(SD25XX_SEC_REG_ADDR, buf, 7);
}


/**
  * @brief   获取时间戳信息
  * @param   None
  * @return  时间戳
  */
uint32_t GetRtcTimeStamp(void)
{
    return gDevRtc.time_stamp;
}



/**
  * @brief   RTC秒级中断处理
  * @param   None
  * @return  None
  */
static bool _IrqRtcIntProcess()
{
    gDevRtc.time_stamp++;
}


/**
  * @brief   RTC写允许操作
  * @param   None
  * @return  None
  */
static void _RtcWriteEnable()
{
    uint8_t data = 0;
    
    data = 0xB1;
    _i2c_reg_write(SD25XX_CTR2_ADDR, &data, 1);
    data = 0xFF;
    _i2c_reg_write(SD25XX_CTR1_ADDR, &data, 1);
    data = 0x2B;
    _i2c_reg_write(SD25XX_CTR3_ADDR, &data, 1);
}


/**
  * @brief   RTC写禁止操作
  * @param   None
  * @return  None
  */
static void _RtcWriteDisable()
{
    uint8_t data = 0;
    
    data = 0x7B;
    _i2c_reg_write(SD25XX_CTR1_ADDR,&data,1);
    data = 0x21;
    _i2c_reg_write(SD25XX_CTR2_ADDR,&data,1);
    data = 0x2B;
    _i2c_reg_write(SD25XX_CTR3_ADDR, &data, 1);
}




/**
  * @brief   初始化I2C和sd2506
  * @param   None
  * @return  None
  */
void DevRtcInit()
{
    uint8_t buf[8];
#if (USE_SIM_IIC == 1)
    gSimRtcI2CHandle.reg_addr_width = EN_SIMI2C_REG_ADDR_WIDTH_8;
    gSimRtcI2CHandle.slave_addr0 = SD25XX_DEV_ADDR;
    
    
#else
    gRtcI2CHandle.reg_addr_width = EN_I2C_REG_ADDR_WIDTH_8;
    gRtcI2CHandle.slave_addr0 = SD25XX_DEV_ADDR;
#endif   
    gRtcIntHandle.irq_process = _IrqRtcIntProcess;
    
    RtcBspInit();
    
    _RtcWriteEnable();
    
    buf[0] = 0x82;
    _i2c_reg_write(SD25XX_REG_CHARGE, buf, 1);
      
    DevRtcReadTime(&gDevRtc);//上电获取一次时间
    RtcDat2Covert2Sec(&gDevRtc);//转换时间戳
    RtcSecConvert2Date(&gDevRtc);//转换时间戳
}


/**
  * @brief   RTC回读大轮询
  * @param   None
  * @return  如使用外部中断，则可不需要轮询
  */
void DevRtcProcess()
{
    static uint32_t timecount = 0;
    if((uint32_t)(tim_get_systime() - timecount) > 500)
    {
        timecount = tim_get_systime();
        DevRtcReadTime(&gDevRtc);//获取一次时间
        RtcDat2Covert2Sec(&gDevRtc);//转换时间戳
    }
}






