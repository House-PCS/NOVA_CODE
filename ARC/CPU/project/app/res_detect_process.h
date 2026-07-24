/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.h
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
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __RES_DETECT_PROCESS_H_
#define __RES_DETECT_PROCESS_H_


#include <stdint.h>
#include <stdbool.h>
#include "dev_chan_sw.h"

#define R1     3000.0       //kΩ
#define R2     3000.0       //kΩ

#define R3     2.2           //kΩ
#define R4     2.2           //kΩ

#define R5     300.0         //kΩ

#define R6     6000.0        //kΩ
#define R7     6000.0        //kΩ

#define R9      39.0         //kΩ
#define R10     100.0         //kΩ
/////待调试结束需要对公式常量进行化简

typedef enum
{
    RES_READY = 0,
    RES_START,
    RES_WAIT,
    RES_INIT,
    RES_IDLE,
    RES_KP_ON,
    RES_WAIT1,
    RES_SAMPLE_VP1,
    RES_SAMPLE_VN1,
    RES_KN_ON,
    RES_WAIT2,
    RES_SAMPLE_VP2,
    RES_SAMPLE_VN2,
	RES_CALIBRATION,
    RES_COMPUTE,
	RES_JUDGE,
    RES_SAMPLE_ERR,
}EN_SAMPLE_RES;

typedef struct _bms_sample_res_stru_t
{
    bool            sample_en;
    bool            sample_over;    //采样就绪
    double         res_main;

    int16_t        valid_cnt;      //采集有效次数
    int16_t        error_cnt;      //采集错误次数
    double       res_total_voltage;

    float         res_p1;      //kΩ
    float         res_n1;

	uint32_t       adc_kp1;
	uint32_t       adc_kn1;
	uint32_t       adc_kp2;
	uint32_t       adc_kn2;

    double         volt_kp1;   //采样口电压值,±900mv
    double         volt_kn1;
    double         volt_kp2;
    double         volt_kn2;
	

//    uint32_t        volt_ch1;
//    uint32_t        volt_ch2;
}BmsSampleResType;


extern const BmsSampleResType *const IR_BmsSampleRes;
extern BmsSampleResType gBmsSampleRes[4];
//static int SampleVoltage1(uint8_t reg,uint32_t *temp_u32 ,float *data);
static int SampleVoltage(uint8_t reg,uint32_t *temp_u32);

void SampleRes_Init(void);

void SampleResStart(EN_CHANNAL channal_switch);
void SampleResStop(EN_CHANNAL channal_switch);
void SampleRes_Main(EN_CHANNAL channal_switch);
void Res_Detect_PrepareRetry(void);  
static int SampleVoltage1(uint8_t reg,double *data);

#endif
