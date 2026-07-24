/**
******************************************************************************
* @copyright (C) COPYRIGHT Nova
* @file sys_info.c
* @author Xi'an NovaStar Tech Co.,LTD.
* @version V1.0.0
* @date 2023-8-28
* @brief
******************************************************************************
* @attention
* @relation
* @par :(修改日志)
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description
* </table>
******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "arc_detect_info.h"
#include "modbus_analysis.h"
#include "arc_detect_info.h"
#include "alarm.h"
#include "flash.h"
#include "arc_detect_threshold.h"
#include "arc_detect_process.h"
#include "res_detect_process.h"
#include "dev_chan_sw.h"
#include "main.h"
//#include "debug.h"
/* Global Variable -------------------------------------------------------------*/
SysInfo_t gSysInfo;
sys_thershold_info_t thershold_info;
static uint32_t s_restart_count = 0;

/* Private typedef -----------------------------------------------------------*/

/* Private function  -----------------------------------------------*/
//static void _ReadBasicInformation();
//static int _DecimalToBcdCode(int decimal);

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief   Convert decimal to BCD
  * @param   system Version Number
  * @return  None
  * @note    None
  *
*/
static int _DecimalToBcdCode(int decimal)
{
    int sum = 0, i;                 /*i计数变量，sum返回的BCD码*/
    for ( i = 0; decimal > 0; i++)
    {
        sum |= ((decimal % 10 ) << ( 4 * i));
        decimal /= 10;
    }
    return sum;
}

/**
  * @brief   Read id number,SN number,hardware version number,software version number,DC number
  * @param   None
  * @return  None
  * @note    None
  *
*/
/*** modeid\sn\软件版本\硬件版本 等信息赋值 ***/
static void _ReadBasicInformation()
{
    gSysInfo.model_id = 0X02;
    memset(gSysInfo.sn, 0x40, 20);
    gSysInfo.hw_version = 0x01;
    gSysInfo.sw_version = 0x02;
}

/*** ModeId和SN号写入内部FLASH中 ***/
void DeviceBaseInfoWrite()
{
    _ReadBasicInformation();
//    InternalFlashWrite(ARC_SYSINFO_SAVE_ADDR,(uint32_t *)&gSysInfo.model_id,sizeof(SysInfo_t));
}

/*** ModeId和SN号读取内部FLASH ***/
void DeviceBaseInfoRead()
{
    uint8_t buf[256];
    InternalFlashRead(ARC_SYSINFO_SAVE_ADDR, buf, sizeof(SysInfo_t));
    memcpy(&gSysInfo.model_id, buf, sizeof(SysInfo_t));
}

/*** 阈值信息写入FLASH ***/
void ThresholdInfoWriteToFlash()
{
    uint8_t save_flg[4] = {0};
    memset((uint8_t *)save_flg, 0x5A, 4);
    InternalFlashWrite(ARC_RES_THRESHOLD_ADDR, (uint32_t *)&thershold_info.arc_threshold1, sizeof(sys_thershold_info_t));
    InternalFlashWrite(ARC_RES_THRESHOLD_ADDR + 0x1000, (uint32_t*)&save_flg, 4);
}

/*** 阈值信息读取内部FLASH ***/
static uint8_t _RestartCountIsFlagEmpty(const uint8_t *flag)
{
    uint8_t i;

    for(i = 0; i < RESTART_COUNT_FLAG_SIZE; i++)
    {
        if(flag[i] != 0xFFU)
        {
            return 0;
        }
    }
    return 1;
}

static uint8_t _RestartCountIsFlagValid(const uint8_t *flag)
{
    uint8_t i;

    for(i = 0; i < RESTART_COUNT_FLAG_SIZE; i++)
    {
        if(flag[i] != RESTART_COUNT_FLAG_VALUE)
        {
            return 0;
        }
    }
    return 1;
}

static void _RestartCountWrite(uint32_t count)
{
    uint8_t save_flag[RESTART_COUNT_FLAG_SIZE];

    memset(save_flag, RESTART_COUNT_FLAG_VALUE, RESTART_COUNT_FLAG_SIZE);
    InternalFlashErasePage(RESTART_COUNT_RECORD_ADDR);
    InternalFlashWriteWithoutErase(RESTART_COUNT_RECORD_ADDR + RESTART_COUNT_DATA_OFFSET,
                                   (uint32_t *)&count, 4);
    InternalFlashWriteWithoutErase(RESTART_COUNT_RECORD_ADDR, (uint32_t *)save_flag, RESTART_COUNT_FLAG_SIZE);
}

void RestartCount_Init(void)
{
    uint8_t record_buf[RESTART_COUNT_FLAG_SIZE + 4];
    uint32_t restart_count = 0;

    InternalFlashRead(RESTART_COUNT_RECORD_ADDR, record_buf, sizeof(record_buf));
    if(_RestartCountIsFlagEmpty(record_buf))
    {
        restart_count = 1;
    }
    else if(_RestartCountIsFlagValid(record_buf))
    {
        memcpy(&restart_count, &record_buf[RESTART_COUNT_DATA_OFFSET], 4);
        restart_count++;
    }
    else
    {
        restart_count = 1;
    }

    _RestartCountWrite(restart_count);
    s_restart_count = restart_count;
}

uint32_t RestartCount_Get(void)
{
    return s_restart_count;
}

void ThresholdInfoReadFromFlash()
{

    uint8_t buf[256];
    uint8_t save_flag[4] = {0, 0, 0, 0};
    InternalFlashRead(ARC_RES_THRESHOLD_ADDR + 0x1000, (uint8_t*)save_flag, 4);
    if(save_flag[0] != 0x5A || save_flag[1] != 0x5A || save_flag[2] != 0x5A || save_flag[3] != 0x5A)
    {
        thershold_info.arc_threshold1 = 0x5A;//拉弧阈值个数
        thershold_info.arc_threshold2 = 0x5A;
        thershold_info.arc_threshold3 = 0x12C;
        thershold_info.arc_threshold4 = 0x12C;
        thershold_info.arc_threshold5 = 0x12C;
        thershold_info.arc_threshold6 = 0x12C;
        thershold_info.arc_threshold7 = 0x12C;
        thershold_info.arc_threshold8 = 0x12C;
        thershold_info.impedance_thersholdrx1 = 0xAA;//默认170K，即RX判定阈值 = 170 * 0.1 = 17K ；
        thershold_info.impedance_thersholdrx2 = 0xAA;
        thershold_info.impedance_thersholdrx3 = 0;
        thershold_info.impedance_thersholdrx4 = 0;
        thershold_info.impedance_thersholdrx5 = 0;
        thershold_info.impedance_thersholdrx6 = 0;
        thershold_info.impedance_thersholdrx7 = 0;
        thershold_info.impedance_thersholdrx8 = 0;

        thershold_info.impedance_thersholdry1 = 0xAA;//默认170K，即RY判定阈值 = 170 * 0.1 = 17K ；
        thershold_info.impedance_thersholdry2 = 0xAA;
        thershold_info.impedance_thersholdry3 = 0;
        thershold_info.impedance_thersholdry4 = 0;
        thershold_info.impedance_thersholdry5 = 0;
        thershold_info.impedance_thersholdry6 = 0;
        thershold_info.impedance_thersholdry7 = 0;
        thershold_info.impedance_thersholdry8 = 0;
    }
    else
    {

        InternalFlashRead(ARC_RES_THRESHOLD_ADDR, buf, sizeof(sys_thershold_info_t));
        memcpy(&Modbus_writeReg.arcalarmthreshold1, buf, sizeof(ModbusReg_t) - 6);
        memcpy(&thershold_info.arc_threshold1, buf, sizeof(sys_thershold_info_t));
    }
}

void SysInfoRead()
{
//    DeviceBaseInfoRead();
    memset(&Modbus_writeReg.reserve21_22, 0, sizeof(ModbusReg_t)); //初始化，全部赋0
    ThresholdInfoReadFromFlash();
    Modbus_writeReg.sysupgradestatus.value = 0;
}

/*----------------------------user code end-----------------------------------*/

/**
  * @brief   Information initialization
  * @param   None
  * @return  None
  * @note    None
  *
*/
void SysInfoInit()
{
    _ReadBasicInformation();
}


float sample_current_value(uint16_t value_voltage)
{
    float current_value = 0;
    current_value = (((float)value_voltage * 3.3) / 4096) * 100;
    return current_value;
}

uint16_t gArcState()
{
    //  uint16_t value = 0x00 ;
    static uint8_t error_flag = 0;
    static uint16_t real_state = 0x00;
//	static uint8_t error_count[2] = {0};
    uint8_t coe = 10;//默认低灵敏度
    if(error_flag == 0)
    {
        if(Modbus_writeReg.arcdetectlevel == 1)//高灵敏度
        {
//            Modbus_ReadReg.realtimearcalarminformation = 0;
//            Modbus_ReadReg.arcalarminformation = 0;
            alarm_flag = 0;
            coe = 2;
            if(Arcjudgment(DetectProcessHandle.FFT1_OutputMag_f32, DetectThreshold.Threshold1, thershold_info.arc_threshold1 * coe) != 0)
            {
//				error_count[0]++;
//				if(error_count[0] == 3)
//				{
					real_state = (real_state | (0x01 << 0));
					error_flag = 1;
//					error_count[0] = 0;
//				}
            }
//			else
//			{
//				error_count[0] = 0;
//			}
            if(Arcjudgment(DetectProcessHandle.FFT2_OutputMag_f32, DetectThreshold.Threshold2, thershold_info.arc_threshold2 * coe) != 0)
            {
//                error_count[1]++;
//				if(error_count[1] == 3)
//				{
					real_state = (real_state | (0x01 << 1));
					error_flag = 1;
//					error_count[1] = 0;
//				}
            }
//			else
//			{
//				error_count[1] = 0;
//			}
        }

        //  if(Arcjudgment(DetectProcessHandle.FFT3_OutputMag_f32,DetectThreshold.Threshold3,thershold_info.arc_threshold3) !=0)
        //  {
        //    real_state = (real_state|(0x01<<2));
        //  }
        //  if(Arcjudgment(DetectProcessHandle.FFT4_OutputMag_f32,DetectThreshold.Threshold4,thershold_info.arc_threshold4) !=0)
        //  {
        //    real_state = (real_state|(0x01<<3));
    }
//  }
    return real_state;




//  uint16_t value = 0x00 ;

}

uint16_t gArcRealState()
{
    uint16_t state = 0x00 ;
    if(Arcjudgment(DetectProcessHandle.FFT1_OutputMag_f32, DetectThreshold.Threshold1, thershold_info.arc_threshold1 * 2) != 0)
    {
        state = (state | (0x01 << 0));
    }
    if( Arcjudgment(DetectProcessHandle.FFT2_OutputMag_f32, DetectThreshold.Threshold2, thershold_info.arc_threshold2 * 2) != 0)
    {
        state = (state | (0x01 << 1));
    }
//  if( Arcjudgment(DetectProcessHandle.FFT3_OutputMag_f32,DetectThreshold.Threshold3,thershold_info.arc_threshold3) !=0)
//  {
//    state = (state|(0x01<<2));
//  }
//  if( Arcjudgment(DetectProcessHandle.FFT4_OutputMag_f32,DetectThreshold.Threshold4,thershold_info.arc_threshold4) !=0)
//  {
//    state = (state|(0x01<<3));
//  }
    return state;

}

int Max_valuehistory(uint16_t value, uint16_t* addr)
{
    static int max[4] = {0};
    uint32_t num = ((uint32_t)addr - (uint32_t)(&Modbus_ReadReg.channel1arcstrength)) >> 1; //地址作差判断通道
    switch(num)
    {
        case 0:
            if(value > max[0])
            {
                max[0] = value;
            }
            return max[0];
            break;
        case 1:
            if(value > max[1])
            {
                max[1] = value;
            }
            return max[1];
            break;
        case 2:
            if(value > max[2])
            {
                max[2] = value;
            }
            return max[2];
            break;
        case 3:
            if(value > max[3])
            {
                max[3] = value;
            }
            return max[3];
            break;
        default:
            return max[3];
            break;
    }
}

uint16_t res_judge(float Rx, float Ry, float thresholdRx, float thresholdRy)
{
    if((Rx < thresholdRx) || (Ry < thresholdRy))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint16_t gResRiskState(float COE)
{
    static uint16_t risk_state = 0x00;
//    static uint16_t error_flag = 0;
    float res_detect_flag = COE;
    if(res_detect_ok == 1)
    {
        if(res_detect_flag == 0.1f)
        {
//            if(error_flag == 0)
//            {
			uint16_t res = (res_judge(gBmsSampleRes[0].res_p1, gBmsSampleRes[0].res_n1, (float)thershold_info.impedance_thersholdrx1 * 0.1f +  Flash_DateReg.res_hysteresis_increment, (float)thershold_info.impedance_thersholdry1 * 0.1f) != 0);
            
//				risk_state = (risk_state | res);
			
				if(res)
                {
                    risk_state = (risk_state | (0x01 << 0));
//                    error_flag = 1;
                }
				else 
				{
					risk_state = (risk_state & (~(0x01 << 0)));
				}
				
                if((res_judge(gBmsSampleRes[1].res_p1, gBmsSampleRes[1].res_n1, (float)thershold_info.impedance_thersholdrx2 * 0.1f + Flash_DateReg.res_hysteresis_increment, (float)thershold_info.impedance_thersholdry2 * 0.1f) != 0))
                {
					
                    risk_state = (risk_state | (0x01 << 1));
//                    error_flag = 1;
                }
				else
				{
					risk_state = (risk_state & (~(0x01 << 1)));
				}
//            }
            return risk_state;
        }
        else if(res_detect_flag == 1)
        {
            uint16_t alam_state = 0x00;
            if((res_judge(gBmsSampleRes[0].res_p1, gBmsSampleRes[0].res_n1, (float)thershold_info.impedance_thersholdrx1 * COE +  Flash_DateReg.res_hysteresis_increment , (float)thershold_info.impedance_thersholdry1 * COE) != 0))
            {
                alam_state = (alam_state | (0x01 << 0));
            }
            if((res_judge(gBmsSampleRes[1].res_p1, gBmsSampleRes[1].res_n1, (float)thershold_info.impedance_thersholdrx2 * COE +  Flash_DateReg.res_hysteresis_increment, (float)thershold_info.impedance_thersholdry2 * COE) != 0))
            {
                alam_state = (alam_state | (0x01 << 1));
            }
            return alam_state;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}



//void ARC_RES_Info()
//{
//    uint8_t Sw_Flag = 0;
//  Modbus_ReadReg.arcselftestresult =0;
//    Modbus_ReadReg.channel4currentvalue = (int16_t)sample_current_value(gDevCurDetect.arc_cur_data_4[1]);//暂选1024中第一个值
//    Modbus_ReadReg.channel4arcstrength = Arc_strength(DetectProcessHandle.FFT4_OutputMag_f32);
//    Modbus_ReadReg.channel4arcIntensityhistoryvalue = Max_valuehistory(Modbus_ReadReg.channel4arcstrength,&Modbus_ReadReg.channel4arcstrength);
//    Modbus_ReadReg.channel4inputvoltage = gBmsSampleRes[3].res_total_voltage;
//    Modbus_ReadReg.channel4rx = gBmsSampleRes[3].res_p1;
//    Modbus_ReadReg.channel4ry = gBmsSampleRes[3].res_n1;

//    Modbus_ReadReg.channel3currentvalue = (int16_t)sample_current_value(gDevCurDetect.arc_cur_data_3[1]);//暂选1024中第一个值
//    Modbus_ReadReg.channel3arcstrength = Arc_strength(DetectProcessHandle.FFT3_OutputMag_f32);
//    Modbus_ReadReg.channel3arcIntensityhistoryvalue = Max_valuehistory(Modbus_ReadReg.channel3arcstrength,&Modbus_ReadReg.channel3arcstrength);
//    Modbus_ReadReg.channel3inputvoltage = gBmsSampleRes[2].res_total_voltage;
//    Modbus_ReadReg.channel3rx = gBmsSampleRes[2].res_p1;
//    Modbus_ReadReg.channel3ry = gBmsSampleRes[2].res_n1;


//    Modbus_ReadReg.channel2currentvalue = (int16_t)sample_current_value(gDevCurDetect.arc_cur_data_2[1]);//暂选1024中第一个值
//    Modbus_ReadReg.channel2arcstrength = Arc_strength(DetectProcessHandle.FFT2_OutputMag_f32);
//    Modbus_ReadReg.channel2arcIntensityhistoryvalue = Max_valuehistory(Modbus_ReadReg.channel2arcstrength,&Modbus_ReadReg.channel2arcstrength);
//    Modbus_ReadReg.channel2inputvoltage = gBmsSampleRes[1].res_total_voltage;
//    Modbus_ReadReg.channel2rx = gBmsSampleRes[1].res_p1;
//    Modbus_ReadReg.channel2ry = gBmsSampleRes[1].res_n1;


//    Modbus_ReadReg.channel1currentvalue = (int16_t)sample_current_value(gDevCurDetect.arc_cur_data_1[1]);//暂选1024中第一个值
//    Modbus_ReadReg.channel1arcstrength = Arc_strength(DetectProcessHandle.FFT1_OutputMag_f32);
//    Modbus_ReadReg.channel1arcIntensityhistoryvalue = Max_valuehistory(Modbus_ReadReg.channel1arcstrength,&Modbus_ReadReg.channel1arcstrength);
//    Modbus_ReadReg.channel1inputvoltage = gBmsSampleRes[0].res_total_voltage;
//    Modbus_ReadReg.channel1rx = gBmsSampleRes[0].res_p1;
//    Modbus_ReadReg.channel1ry = gBmsSampleRes[0].res_n1;
//    //Modbus_ReadReg.ChannelOfNum = ;

//
//}







/************************ (C) COPYRIGHT Nova *************END OF FILE**********/







