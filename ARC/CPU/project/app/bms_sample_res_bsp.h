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
#ifndef __BMS_SAMPLE_RES_BSP_H_
#define __BMS_SAMPLE_RES_BSP_H_


#include <stdint.h>
#include <stdbool.h>

#define R1     6000.0       //kΩ
#define R2     6000.0       //kΩ

#define R3     3.0           //kΩ
#define R4     3.0           //kΩ

#define R5     300.0         //kΩ

#define R6     5700.0        //kΩ
#define R7     5700.0        //kΩ

#define R9      57.0         //kΩ
#define R10     57.0         //kΩ
/////待调试结束需要对公式常量进行化简

typedef enum
{
    RES_INIT=0,
    RES_IDLE,
    RES_KP_ON,
    RES_WAIT1,
    RES_SAMPLE_VP1,
    RES_SAMPLE_VN1,
    RES_KN_ON,
    RES_WAIT2,
    RES_SAMPLE_VP2,
    RES_SAMPLE_VN2,
    RES_COMPUTE,
    RES_SAMPLE_ERR,
}EN_SAMPLE_RES;

typedef struct _bms_sample_res_stru_t
{
    bool            sample_en;
    bool            sample_over;    //采样就绪
    uint16_t         res_main;
    uint16_t        valid_cnt;      //采集有效次数
    uint16_t        error_cnt;      //采集错误次数
    uint16_t        res_total_voltage;
    uint16_t        res_p;      //kΩ
    uint16_t        res_n;
    int16_t        volt_kp1;   //采样口电压值,±900mv
    int16_t        volt_kn1;
    int16_t        volt_kp2;
    int16_t        volt_kn2;
//    double        volt_kp1;   //采样口电压值,±900mv
//    double        volt_kn1;
//    double        volt_kp2;
//    double        volt_kn2;

//    uint32_t        volt_ch1;
//    uint32_t        volt_ch2;
}BmsSampleResType;


extern const BmsSampleResType *const IR_BmsSampleRes;


void SampleResStart(void);
void SampleResStop(void);
void SampleRes_Init(void);
void SampleRes_Main(void);

#endif
