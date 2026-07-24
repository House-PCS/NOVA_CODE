
#ifndef __SYS_INFO_H_
#define __SYS_INFO_H_

#include "stdint.h"
#include <string.h>
#include "rtc.h"

typedef struct _product_info_stru_t
{
    uint8_t             model_id[2];
    char                sn[21];
    char                sw_version[10];
    char                sw_main_ctl_version[10];
    char                sw_bms_version[10];
    char                sw_pcs_version[10];
    char                sw_dcdc_version[10];
    char                hw_main_ctl_version[4];
    char                hw_bms_version[4];
    char                hw_pcs_version[4];
    char                hw_dcdc_version[4];
}PdtInfo_t;


typedef struct _chg_time_interval_stru_t
{
    uint8_t             enable;
    uint32_t            start_time;
    uint32_t            end_time;
}ChgTimeInterval_t;

typedef struct _led_set_stru_t
{
    uint8_t             status;
    uint8_t             bright_percent;
}LedSet_t;




#pragma pack(1)
typedef struct _sys_device_status_stru_t
{
    uint16_t            current_quantity;//0.1%
    uint16_t            device_temperature;//0.1℃
    uint32_t            chg_full_time;//预计充满时长
    uint32_t            dsg_empty_time;//剩余可用时长
    uint8_t             add_pack_num;//加电包数量
    uint8_t             parallel_status;//并机状态
    uint16_t            pcs_electric_voltage;//市电电压,0.1V
    uint16_t            pcs_pack_voltage;//当前电池pack端电压,0.1V
    int16_t             pcs_current;//市电充电或放电电流,0.1A
    uint32_t            pcs_pack_chg_power;//电池充电功率，0.1W
    uint8_t             reserved1[4]; //24
    uint16_t            dcdc_input_voltage;//光伏电压
    uint16_t            dcdc_input_current;//光伏充电电流
    uint32_t            dcdc_input_power;//光伏充电功率
    uint8_t             reserved2[4];//36
    uint16_t            ac_output_voltage;//交流输出电压
    uint16_t            ac_output_current;//交流输出电流
    uint32_t            ac_output_power;//交流输出power
    uint8_t             reserved3[4];//48
    uint16_t            dc_12v10a_output_voltage;//DC12V10A输出电压
    uint16_t            dc_12v10a_output_current;//DC12V10A输出电流
    uint32_t            dc_12v10a_output_power;//DC12V10A输出power
    uint8_t             reserved4[4];//60
    uint16_t            dc_12v30a_output_voltage;//DC12V30A输出电压
    uint16_t            dc_12v30a_output_current;//DC12V30A输出电流
    uint32_t            dc_12v30a_output_power;//DC12V30A输出power
    uint8_t             reserved5[4];//72
    uint16_t            dc_24v20a_output_voltage;//DC24V20A输出电压
    uint16_t            dc_24v20a_output_current;//DC24V20A交流输出电流
    uint32_t            dc_24v20a_output_power;//DC24V20A交流输出power
    uint8_t             reserved6[4];//84
    int16_t             pcs_contra_radiator_temperature;//逆变板散热器温度
    int16_t             pcs_boost_radiator_temperature;//升压散热器温度
    int16_t             pcs_transformor_temperature;//变压器温度
    uint8_t             pcs_fan_status;//风扇状态,0关1开
    uint8_t             reserved7[4];//95
    int16_t             dcdc_24v_radiator_temperature;//24v散热器温度
    int16_t             dcdc_12v_radiator_temperature;//12v散热器温度
    int16_t             dcdc_pv_radiator_temperature;//光伏板温度
    uint8_t             reserved8[4];//105
}StoreEnergyStatus_t;
#pragma pack()


#define CELL_STRING_NUM     16
#define CELL_TEMP_NUM       4
#define ADD_PACK_NUM        2

#pragma pack(1)
typedef struct _sys_cell_status_stru_t
{
    uint16_t            soc_real;//0.1%，真实SOC
    uint16_t            soc_act;//0.1%，计算使用时长的SOC
    uint16_t            soc_rpt;//0.1%，表显SOC
    uint32_t            rated_capa;//额定容量
    uint32_t            act_capa;//根据环境因素的实际容量
    uint16_t            soh;//电池健康度，0.1%
    uint16_t            cycle_num;//电池充放电循环次数(取充电放电较小的那个)
    uint32_t            total_chg_capa_c;//总充电量, mAh
    uint32_t            total_dsg_capa_c;//总放电量，mAh
    uint32_t            total_chg_capa_p;//总充电量, mWh
    uint32_t            total_dsg_capa_p;//总放电量，mWh
    uint32_t            max_chg_current;//最大充电电流
    uint32_t            max_dsg_current10;//10S内最大放电电流
    uint32_t            max_dsg_current30;//30S内最大放电电流
    uint32_t            max_dsg_current60;//稳态最大放电电流
    uint32_t            idle_time;//电芯处于静态状态时长
    uint32_t            total_voltage;//电池总压
    uint16_t            max_cell_voltage;//最大单体电压
    uint16_t            min_cell_voltage;//最小单体电压
    uint16_t            average_cell_voltage;//平均单体电压
    int32_t             current;//当前电流
    uint8_t             string_num;//电池串数
    uint16_t            cell_voltage[CELL_STRING_NUM];//单体电压
    int16_t             max_cell_temperature;//最大电池温度
    int16_t             min_cell_temperature;//最小电池温度
    int16_t             average_cell_temperature;//平均电池温度
    uint8_t             ntc_num;//温度传感器数
    int16_t             cell_temperature[CELL_TEMP_NUM];//单体温度
    uint8_t             balance_status;//均衡是否打开
    int16_t             balance_res_temperature;//均衡电阻温度
    int16_t             dsg_mos_temperature;//充电MOS温度
    int16_t             chg_mos_temperature;//放电MOS温度
    int16_t             pre_dsg_mos_temperature;//预放电MOS温度
    uint8_t             add_pack_num;//加电包接口数量
    int16_t             add_pack_temperature[ADD_PACK_NUM];//加电包MOS温度
}CellStatus_t;
#pragma pack()



typedef struct _sys_info_stru_t
{
    PdtInfo_t           pdt_info;
    
    RtcDateInfo_t       rtc_time;
    
    char                name[33];//最后一个字节恒为0，初始化就赋值0
    
    uint8_t             chg_max_power_percent;/*充电功率控制百分比*/
    uint8_t             screen_bright_time;/*无操作亮屏时间*/
    uint8_t             output_interface_switch;/*输出接口开关控制*/
    ChgTimeInterval_t   chg_time_interval_set;
    uint8_t             chg_mode;/*充电模式，快充还是慢充*/
    uint8_t             chg_max_capa_percent;/*最大充电容量设置*/
    LedSet_t            ambience_lamp;/*氛围灯*/
    LedSet_t            head_light;/*手电筒*/
    
    StoreEnergyStatus_t store_energy_status;
    CellStatus_t        cell_status[ADD_PACK_NUM + 1];
}SysInfo_t;



extern SysInfo_t gSysInfo;




void DeviceBaseInfoWrite();
void DeviceBaseInfoRead();

void DeviceCtlWrite();
void DeviceCtlRead();



#endif





























