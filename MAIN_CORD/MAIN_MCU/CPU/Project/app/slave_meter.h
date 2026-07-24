#ifndef __SLAVE_METER_H_
#define __SLAVE_METER_H_

#include "dev_modbus.h"
#include "bsp.h"
#include <stdio.h>

#define SLAVE_METER_ADDR                                (0x01)

#define METER_READ_INPUT_TYPE                                 (0x03)


//电表参数寄存器地址
#define METER_A_PHASE_VOLT                              (0x2006)
#define METER_B_PHASE_VOLT                              (0x2008)
#define METER_C_PHASE_VOLT                              (0x200A)
#define METER_A_PHASE_CURR                              (0x200C)
#define METER_B_PHASE_CURR                              (0x200E)
#define METER_C_PHASE_CURR                              (0x2010)
#define METER_T_ACTIVE_POWER                            (0x2012)
#define METER_A_ACTIVE_POWER                            (0x2014)
#define METER_B_ACTIVE_POWER                            (0x2016)
#define METER_C_ACTIVE_POWER                            (0x2018)
#define METER_T_REACTIVE_POWER                          (0x201A)
#define METER_A_REACTIVE_POWER                          (0x201C)
#define METER_B_REACTIVE_POWER                          (0x201E)
#define METER_C_REACTIVE_POWER                          (0x2020)
#define METER_FERQUENCY                                 (0x2044)
#define METER_T_ELEC_ENERGY                             (0x101E)


typedef enum
{
    EN_READ_METER_VOLT_CURR_POWER_EVENT,
    EN_READ_METER_FERQUENCY_EVENT,
    EN_READ_METER_T_ELEC_ENERGY_EVENT,
    EN_READ_METER_EVENT_NUM,
} EN_METER_PARA_READ_EVENT;



typedef enum
{
    EN_METER_POLLING_STATUS_IDLE = 0,
    EN_METER_POLLING_STATUS_WAITING_REV,
    EN_METER_POLLING_STATUS_REV_OVERTIME,
    EN_METER_POLLING_STATUS_REV_INTERVAL_WAIT,
} EN_METER_POLLING_STATUS;

#pragma pack(2)
typedef struct _meter_volt_curr_power_stru_t
{
    uint16_t    Ua_volt[2];        //A相电压
    uint16_t    Ub_volt[2];        //B相电压
    uint16_t    Uc_volt[2];        //C相电压
    uint16_t    Ia_curr[2];        //A相电流
    uint16_t    Ib_curr[2];        //B相电流
    uint16_t    Ic_curr[2];        //C相电流
    uint16_t    Pt_power[2];       //合相有功功率
    uint16_t    Pa_power[2];       //A相有功功率
    uint16_t    Pb_power[2];       //B相有功功率
    uint16_t    Pc_power[2];       //C相有功功率
    uint16_t    Qt_power[2];       //合相无功功率
    uint16_t    Qa_power[2];       //A相无功功率
    uint16_t    Qb_power[2];       //B相无功功率
    uint16_t    Qc_power[2];       //C相无功功率
} MeterVoltCurrPowerPara_t;

typedef struct _meter_para_stru_t
{
    MeterVoltCurrPowerPara_t volt_curr_power;
    uint16_t    freq[2];           //频率
    uint16_t    t_elec_enegy[2];    //总电能(用电量)
    uint8_t     meter_comm_err_cnt;
    uint8_t     meter_comm_time_out;
} ModbusReadMeterPara_t;
#pragma pack()

typedef struct _meter_real_value_stru_t
{
    float meter_a_volt;
    float meter_b_volt;
    float meter_c_volt;

    float meter_a_curr;
    float meter_b_curr;
    float meter_c_curr;

    float meter_a_active_power;
    float meter_b_active_power;
    float meter_c_active_power;
    float meter_t_active_power;

    float meter_a_reactive_power;
    float meter_b_reactive_power;
    float meter_c_reactive_power;
    float meter_t_reactive_power;

    float meter_frequency;
    float meter_t_elec_enegy;
} MeterRealValue_t;


extern ModbusReadMeterPara_t         gstMeterPara;
void ReadVoltValue(void);
void ReadCurrValue(void);
void SlaveMeterModbusCommProcess(void);
void RawDataToRealData(void);
MeterRealValue_t GetMeterAllValue(void);

int16_t GetMeterCurrValue(void);
void CheckMeterConnectStaus(void);
uint16_t GetMeterConnectSta(void);
#endif