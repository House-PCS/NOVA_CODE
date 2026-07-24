/*
 * data_process.h
 *
 *  Created on: 2025年10月24日
 *      Author: Nova007457
 */
#ifndef DATA_PROCESS_H

#define DATA_PROCESS_H

#include "F28x_Project.h"


// 定义电量统计结构体，方便管理
typedef struct {
    float32 PV_ElectricGenTotal;           // PV总发电量
    float32 elec_purchase_consum_total;    // 买电总电量
    float32 elec_sale_consum_total;        // 卖电总电量
    float32 Bat_ElectricChgTotal;          // 电池总充电量
    float32 Bat_ElectricDsgTotal;          // 电池总放电量
    float32 backup_total_consum_power;     // 本地负载总用电量 (EPS_ElectricTotal)
    float32 load_total_consum_power;       // 远端负载总用电量 (KWH)
    float32 Gen_ElectricTotal;             // 油机总发电量
    float32 EV_ElectricChgTotal;           // 充电桩给设备总充电量
    float32 EV_ElectricDsgTotal;           // 充电桩给设备总放电量
    float32 PV1_ElectricGenTotal;          // PV1总发电量
    float32 PV2_ElectricGenTotal;          // PV2总发电量
    float32 PV3_ElectricGenTotal;          // PV3总发电量
    float32 PV4_ElectricGenTotal;          // PV4总发电量
}InverterEnergyStats;


void updateEnergyStatistics(InverterEnergyStats *stats, uint32_t timeIntervalSec,
                            float32 pvPower,          // PV总功率（W）
                            float32 purchasePower,    // 买电功率（W）
                            float32 salePower,        // 卖电功率（W）
                            float32 batChargePower,   // 电池充电功率（W，正为充电）
                            float32 batDischargePower,// 电池放电功率（W，正为放电）
                            float32 backupLoadPower,  // 本地负载功率（W）
                            float32 remoteLoadPower,  // 远端负载功率（W）
                            float32 genPower,         // 油机发电功率（W）
                            float32 evChargePower,    // 充电桩充电功率（W）
                            float32 evDischargePower, // 充电桩放电功率（W）
                            float32 pv1Power,         // PV1功率（W）
                            float32 pv2Power,         // PV2功率（W）
                            float32 pv3Power,         // PV3功率（W）
                            float32 pv4Power);
                        
void updateEnergyState(void);
uint16_t updateEnergyState_Send(uint16_t clear);
uint16_t SetElecClearState();

extern InverterEnergyStats gstInvEnergyStats;
extern InverterEnergyStats gstInvEnergyStats_Send; //实时电量统计发送


#endif




