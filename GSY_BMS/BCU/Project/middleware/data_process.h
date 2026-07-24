#ifndef __DATA_PROCESS_H_
#define __DATA_PROCESS_H_

#include "stdbool.h"
#include "stdint.h"
#include "string.h"

#define  ABS_DIFF(a,b)      (((a)>(b))?((a)-(b)):((b)-(a)))
#define  REF_MIN(a,b)       (((a)<(b))?(a):(b))
#define  REF_MAX(a,b)       (((a)>(b))?(a):(b))


////////////////////////RTC//////////////////////////
typedef struct _rtc_stru_t
{
    unsigned short      year;
    unsigned char       month;
    unsigned char       day;
    unsigned char       week;
    unsigned char       hour;
    unsigned char       minute;
    unsigned char       seconds;
}RTC_Type;

unsigned char GetWeek(unsigned short year, unsigned char month, unsigned char day);
uint32_t RtcDate2SecondsFun(RTC_Type *pDate);
void Seconds2RtcDateFun(RTC_Type *pDate,uint32_t  rtc_time);
////////////////////////RTC_END//////////////////////////

unsigned short Crc16_Modbus (unsigned char *puchMsg, unsigned short usDataLen);

unsigned char bcd_to_hex(unsigned char data);
unsigned char hex_to_bcd(unsigned char data);


////////////////////////KF//////////////////////////
typedef struct
{
    float  kalman; //上次卡尔曼值(估计出的最优值)
    float  p;
    float  q; //q:过程噪声
    float  r; //r:测量噪声
    float  kg; //kg:卡尔曼增益
}KalmanParaType;
int KalmanFilter(int inData,KalmanParaType* k_para);
////////////////////////KF_END//////////////////////////


#endif
