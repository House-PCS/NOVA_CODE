

/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        rtc.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-11-10    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "rtc.h"
#define LOG_TAG    "rtc"


//以1970年1月1日为基准
//1970~2099年为合法年份
//月份数据表			
static uint8_t const _table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
//平年的月份日期表
static const uint8_t _mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};


static void RtcConfiguration()
{
    /* allow access to BKP domain */
    pmu_backup_write_enable();
    /* reset backup domain */
    rcu_bkp_reset_enable();
    
#if 1    /*暂时屏蔽掉，调试板上没有32.768K晶体*/
    /* enable LXTAL */
    rcu_osci_on(RCU_LXTAL);
    /* wait till LXTAL is ready */
    rcu_osci_stab_wait(RCU_LXTAL);
    /* select RCU_LXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
#else
    rcu_osci_on(RCU_IRC40K);
    rcu_osci_stab_wait(RCU_IRC40K);
    rcu_rtc_clock_config(RCU_RTCSRC_IRC40K);
#endif 
    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);
    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();
    /* wait until last write operation on RTC registers has finished */
//    rtc_lwoff_wait();
    /* enable the RTC second interrupt*/
    rtc_interrupt_enable(RTC_INT_TIMESTAMP);
    /* wait until last write operation on RTC registers has finished */
//    rtc_lwoff_wait();
    /* set RTC prescaler: set RTC period to 1s */
//    rtc_prescaler_set(32767);
    /* wait until last write operation on RTC registers has finished */
//    rtc_lwoff_wait();
}

void TimeCalibrationByDateTest()
{
    RtcDateInfo_t date_info;
    date_info.year = 2022;
    date_info.month = 12;
    date_info.day = 30;
    date_info.hour = 16;
    date_info.minute = 19;
    date_info.second = 0;
    
    RtcSetTime(RtcDat2Covert2Sec(&date_info));
}

static bool _RtcNormalFlag = false;

void RtcInit()
{
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPSRAM);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();
    
    /* disable the tamper pin */
//    bkp_tamper_detection_disable();
//    if(bkp_read_data(BKP_DATA_0) != 0xA5A5)
//    {
//        /* backup data register value is not correct or not yet programmed
//        (when the first time the program is executed) */
////        LOG_INFO(LOG_TAG,"\r\n RTC not yet configured....");
//        /* RTC configuration */
//        RtcConfiguration();
////        LOG_INFO(LOG_TAG,"\r\n RTC configured....");
//        _RtcNormalFlag = false;
//        TimeCalibrationByDateTest();
//        bkp_write_data(BKP_DATA_0, 0xA5A5);
//    }
    if(0)
    {}
    else
    {
//        rcu_osci_on(RCU_IRC40K);/*正式板子上不需要*/
//        rcu_osci_stab_wait(RCU_IRC40K);
       
        /* check if the power on reset flag is set */
        if(RESET != rcu_flag_get(RCU_FLAG_PORRST))
        {
//            LOG_INFO(LOG_TAG,"\r\n\n Power On Reset occurred....");
        }
        /* check if the pin reset flag is set */
        else if(RESET != rcu_flag_get(RCU_FLAG_SWRST))
        {
//            LOG_INFO(LOG_TAG,"\r\n\n External Reset occurred....");
        }
        pmu_backup_write_enable();
//        LOG_INFO(LOG_TAG,"\r\n No need to configure RTC....");
        /* wait for RTC registers synchronization */
        rtc_register_sync_wait();
        /* enable the RTC second */
        rtc_interrupt_enable(RTC_INT_TIMESTAMP);
        /* wait until last write operation on RTC registers has finished */
//        rtc_lwoff_wait();
        _RtcNormalFlag = true;
    }
    /* clear reset flags */
    rcu_all_reset_flag_clear();
    
    nvic_irq_enable(RTC_WKUP_IRQn, 0, 0);
}

/*上电前RTC是否在正常运行*/
bool RtcNormalJudge()
{
    return _RtcNormalFlag;
}


void RtcSetTime(uint32_t sec)
{
    /* wait until last write operation on RTC registers has finished */
//    rtc_lwoff_wait();
    /* change the current time */
//    rtc_counter_set(sec);
    /* wait until last write operation on RTC registers has finished */
//    rtc_lwoff_wait();
    
}

uint32_t RtcGetTime()
{
    return rtc_subsecond_get();
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
    return (temp % 7);
}

void RtcSecCovert2Date(RtcDateInfo_t *pDate)
{
    static uint16_t daycnt = 0;
//    uint32_t sec = rtc_counter_get();
    uint32_t sec = 0;
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


uint32_t RtcDat2Covert2Sec(RtcDateInfo_t *pDate)
{
    uint16_t t;
	uint32_t sec_count = 0;
    if(pDate->year < 1970 || pDate->year > 2099)
    {
        return 1;	
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
    
    
    return sec_count;
}




void RtcIrqProcess(RtcDateInfo_t *pDataInfo)
{
    if(RESET != rtc_flag_get(RTC_FLAG_TS))
    {
        rtc_flag_clear(RTC_FLAG_TS);
        RtcSecCovert2Date(pDataInfo);
        pDataInfo->time_stamp = RtcGetTime();

#if  DEBUG_EN
        LOG_INFO(LOG_TAG,"rtc_intrupt"); 
#endif




    }
}
















