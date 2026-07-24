#include <stdint.h>
#include <math.h>
#include <ElectricInfoGet.h>
#include <cla_cpu_share.h>
#include "ethernet_analysis.h"
#include "AlgoFw/package/cntl/pv/pv.h"


InverterEnergyStats gstInvEnergyStats;      //实时电量统计
InverterEnergyStats gstInvEnergyStats_Send; //实时电量统计发送
uint32_t update_timescale = 1;
uint16_t Clear_ok;

// 更新累计电量函数
// 参数说明：
// - stats: 电量统计结构体指针
// - timeIntervalSec: 时间间隔（秒）
// - 其他参数：各部分瞬时功率（W），正负表示方向（如放电为负）

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
                            float32 pv4Power)         // PV4功率（W）
{
    // 计算电量增量（Wh）：功率（W） * 时间（h） = 功率 * (秒数 / 3600)
    // 使用64位整数避免溢出，并四舍五入到最接近的整数
    float32 energyIncrement;

    // 1. 更新PV总发电量（只累加正功率）
    if (pvPower > 0) {
        energyIncrement = pvPower * timeIntervalSec / 3600;
        stats->PV_ElectricGenTotal += energyIncrement;
    }

    // 2. 更新买电总电量（只累加正功率）
    if (purchasePower > 0) {
        energyIncrement = (purchasePower) * timeIntervalSec / 3600;
        stats->elec_purchase_consum_total += energyIncrement;
    }

    // 3. 更新卖电总电量（只累加负功率）
    if (salePower < 0) {
        energyIncrement = (fabs(salePower)) * timeIntervalSec / 3600;
        stats->elec_sale_consum_total += energyIncrement;
    }

    // 4. 更新电池总充电量（只累加正功率）
    if (batChargePower > 0) {
        energyIncrement = (batChargePower) * timeIntervalSec / 3600;
        stats->Bat_ElectricChgTotal += energyIncrement;
    }

    // 5. 更新电池总放电量（只累加负功率）
    if (batDischargePower < 0) {
        energyIncrement = (fabs(batDischargePower)) * timeIntervalSec / 3600;
        stats->Bat_ElectricDsgTotal += energyIncrement;
    }

    // 6. 更新本地负载总用电量（只累加正功率）
    if (backupLoadPower > 0) {
        energyIncrement = (backupLoadPower) * timeIntervalSec / 3600;
        stats->backup_total_consum_power += energyIncrement;
    }

    // 7. 更新远端负载总用电量（只累加正功率）
    if (remoteLoadPower > 0) {
        energyIncrement = (remoteLoadPower) * timeIntervalSec / 3600;
        stats->load_total_consum_power += energyIncrement;
    }

    // 8. 更新油机总发电量（只累加正功率）
    if (genPower > 0) {
        energyIncrement = (genPower) * timeIntervalSec / 3600;
        stats->Gen_ElectricTotal += energyIncrement;
    }

    // 9. 更新充电桩总充电量（只累加正功率）
    if (evChargePower > 0) {
        energyIncrement = (evChargePower) * timeIntervalSec / 3600;
        stats->EV_ElectricChgTotal += energyIncrement;
    }

    // 10. 更新充电桩总放电量（只累加正功率）
    if (evDischargePower > 0) {
        energyIncrement = (evDischargePower) * timeIntervalSec / 3600;
        stats->EV_ElectricDsgTotal += energyIncrement;
    }

    // 11. 更新PV1总发电量（只累加正功率）
    if (pv1Power > 0) {
        energyIncrement = (pv1Power) * timeIntervalSec / 3600;
        stats->PV1_ElectricGenTotal += energyIncrement;
    }

    // 12. 更新PV2总发电量（只累加正功率）
    if (pv2Power > 0) {
        energyIncrement = (pv2Power) * timeIntervalSec / 3600;
        stats->PV2_ElectricGenTotal += energyIncrement;
    }

    // 13. 更新PV3总发电量（只累加正功率）
    if (pv3Power > 0) {
        energyIncrement = (pv3Power) * timeIntervalSec / 3600;
        stats->PV3_ElectricGenTotal += energyIncrement;
    }

    // 14. 更新PV4总发电量（只累加正功率）
    if (pv4Power > 0) {
        energyIncrement = (pv4Power) * timeIntervalSec / 3600;
        stats->PV4_ElectricGenTotal += energyIncrement;
    }
}

void updateEnergyState(void)
{
    uint16_t ClearEnable;

    updateEnergyStatistics(&gstInvEnergyStats, 
                           update_timescale, 
                           Pv_Get_Total_Pwr_Ave(), 
                           gstAlgoInterface.grid_active_power,
                           gstAlgoInterface.grid_active_power,
                           gstAlgoInterface.bus2batpower,
                           gstAlgoInterface.bus2batpower,
                           gstAlgoInterface.load_active_power,
                           gstAlgoInterface.normalload_active_power,
                           gstAlgoInterface.generator_active_power,
                           0,
                           0,           //  充电桩电量暂时不统计
                           Pv_Get_Channel_Pwr(0),
                           Pv_Get_Channel_Pwr(1),
                           Pv_Get_Channel_Pwr(2),
                           Pv_Get_Channel_Pwr(3)            //PV1-PV4功率，后续添加
                        );

    ClearEnable = GetElecClear();
    Clear_ok = updateEnergyState_Send(ClearEnable);

}

uint16_t updateEnergyState_Send(uint16_t clear)
{
    static InverterEnergyStats gstInvEnergyStats_start;
    uint16_t Clearok;

    gstInvEnergyStats_Send.PV_ElectricGenTotal = gstInvEnergyStats.PV_ElectricGenTotal - gstInvEnergyStats_start.PV_ElectricGenTotal;
    gstInvEnergyStats_Send.elec_purchase_consum_total = gstInvEnergyStats.elec_purchase_consum_total - gstInvEnergyStats_start.elec_purchase_consum_total;
    gstInvEnergyStats_Send.elec_sale_consum_total = gstInvEnergyStats.elec_sale_consum_total - gstInvEnergyStats_start.elec_sale_consum_total;
    gstInvEnergyStats_Send.Bat_ElectricChgTotal = gstInvEnergyStats.Bat_ElectricChgTotal - gstInvEnergyStats_start.Bat_ElectricChgTotal;
    gstInvEnergyStats_Send.Bat_ElectricDsgTotal = gstInvEnergyStats.Bat_ElectricDsgTotal - gstInvEnergyStats_start.Bat_ElectricDsgTotal;
    gstInvEnergyStats_Send.backup_total_consum_power = gstInvEnergyStats.backup_total_consum_power - gstInvEnergyStats_start.backup_total_consum_power;
    gstInvEnergyStats_Send.load_total_consum_power = gstInvEnergyStats.load_total_consum_power - gstInvEnergyStats_start.load_total_consum_power;
    gstInvEnergyStats_Send.Gen_ElectricTotal = gstInvEnergyStats.Gen_ElectricTotal - gstInvEnergyStats_start.Gen_ElectricTotal;
    gstInvEnergyStats_Send.EV_ElectricChgTotal = gstInvEnergyStats.EV_ElectricChgTotal - gstInvEnergyStats_start.EV_ElectricChgTotal;
    gstInvEnergyStats_Send.EV_ElectricDsgTotal = gstInvEnergyStats.EV_ElectricDsgTotal - gstInvEnergyStats_start.EV_ElectricDsgTotal;
    gstInvEnergyStats_Send.PV1_ElectricGenTotal = gstInvEnergyStats.PV1_ElectricGenTotal - gstInvEnergyStats_start.PV1_ElectricGenTotal;
    gstInvEnergyStats_Send.PV2_ElectricGenTotal = gstInvEnergyStats.PV2_ElectricGenTotal - gstInvEnergyStats_start.PV2_ElectricGenTotal;
    gstInvEnergyStats_Send.PV3_ElectricGenTotal = gstInvEnergyStats.PV3_ElectricGenTotal - gstInvEnergyStats_start.PV3_ElectricGenTotal;
    gstInvEnergyStats_Send.PV4_ElectricGenTotal = gstInvEnergyStats.PV4_ElectricGenTotal - gstInvEnergyStats_start.PV4_ElectricGenTotal;

    Clearok = 0;

    if (clear == 1 )
    {
        gstInvEnergyStats_start.PV_ElectricGenTotal = gstInvEnergyStats.PV_ElectricGenTotal;
        gstInvEnergyStats_start.elec_purchase_consum_total = gstInvEnergyStats.elec_purchase_consum_total;
        gstInvEnergyStats_start.elec_sale_consum_total = gstInvEnergyStats.elec_sale_consum_total;
        gstInvEnergyStats_start.Bat_ElectricChgTotal = gstInvEnergyStats.Bat_ElectricChgTotal;
        gstInvEnergyStats_start.Bat_ElectricDsgTotal = gstInvEnergyStats.Bat_ElectricDsgTotal;
        gstInvEnergyStats_start.backup_total_consum_power = gstInvEnergyStats.backup_total_consum_power;
        gstInvEnergyStats_start.load_total_consum_power = gstInvEnergyStats.load_total_consum_power;
        gstInvEnergyStats_start.Gen_ElectricTotal = gstInvEnergyStats.Gen_ElectricTotal;
        gstInvEnergyStats_start.EV_ElectricChgTotal = gstInvEnergyStats.EV_ElectricChgTotal;
        gstInvEnergyStats_start.EV_ElectricDsgTotal = gstInvEnergyStats.EV_ElectricDsgTotal;
        gstInvEnergyStats_start.PV1_ElectricGenTotal = gstInvEnergyStats.PV1_ElectricGenTotal;
        gstInvEnergyStats_start.PV2_ElectricGenTotal = gstInvEnergyStats.PV2_ElectricGenTotal;
        gstInvEnergyStats_start.PV3_ElectricGenTotal = gstInvEnergyStats.PV3_ElectricGenTotal;
        gstInvEnergyStats_start.PV4_ElectricGenTotal = gstInvEnergyStats.PV4_ElectricGenTotal;

        Clearok = 1;
        
    }
    return Clearok;

}

uint16_t SetElecClearState()
{
    return Clear_ok;
}
