/*
 * flash_default.c
 *
 *  Created on: 2026年1月30日
 *      Author: Nova007457
 */

#include "flash_default.h"
#include "ethernet_analysis.h"
#include "dev_flash.h"
#include "string.h"
#include <stddef.h>


const uint16_t Flash_DspHoldType[4] = {0x55,0xAA,0x55,0xAA};
const uint16_t Flash_DspHoldType_init[4] = {0x00,0x00,0x00,0x00};

uint8_t Restore_Factory = 0;
uint8_t date_buf[2*sizeof(UdpDspHoldPara_t)];

/* 恢复出厂设置守卫标志：置位后禁止自动保存把 Flash header 写回有效值，
 * 避免在等待重启期间出现 “恢复出厂 vs 自动保存” 的竞态。*/
 static volatile bool s_pending_factory_reset = false;

 /* 自动保存连续失败保护：达到阈值后停止重试，防止 Flash 异常时
  * 每帧都触发一次擦写造成寿命快速耗尽。*/
 #define HOLD_AUTOSAVE_FAIL_LIMIT   5u
 static uint16_t s_hold_autosave_fail_cnt = 0;
 static bool     s_hold_autosave_disabled = false;

UdpDspHoldPara_t UdpPCS_DspHoldPara_Init =
{
    .Second_ID = 0 ,                                  //0x6220 功率段设别ID 0：默认1：6kw功率段 2：8kw功率段 3：10kw功率段 4：12kw功率段 5：14kw功率段 6 ：16kw功率段
    .work_mode = 0 ,                                  //0x6221 设置工作模式    2:削峰填谷 1:电池优先 0:自发自用
    .charging_input_type = 0 ,                        //0x6222 设置强制发电机使能    1:强制为发电机 0:强制为市电 2: 有市电用市电无市电用发电机
    .set_batt_chg_curr = 0 ,                          //0x6223 设置电池充电最大电流    0.1A
    .set_batt_chg_power = {0,16000},                  //0x6224-0x6225 设置电池放电最大功率    1.0W
    .set_batt_dsg_curr = 0 ,                          //0x6226 设置电池放电最大电流    0.1A
    .set_batt_dsg_power = {0,16000},                  //0x6227-0x6228 设置电池放电最大功率    1.0W
    .InvPVMaxPower = {0,24000},                       //0x6229-0x622A 逆变器最大允许的光伏功率(默认值为设备根据自身型号预置的最大值)
    .InvOnGridIFInMaxPower = {0,16000},                   //0x622B~0x622C 并网口最大输入功率(默认值为设备根据自身型号预置最大值)
    .InvOnGridIFOutMaxPower = {0,16000},                  //0x622D~0x622E 并网口最大输出功率(默认值为设备根据自身型号预置最大值)
    .InvGenMaxPower = {0,4000},                          //0x622F~0x6230 发电机最大功率(默认为0，首次使用需要先设置)
    .InvEVMaxInPower = {0,4000},                         //0x6231~0x6232 设备吸收充电桩最大功率
    .InvEVMaxOutPower = {0,4000},                        //0x6233~0x6234 设备供给充电桩最大功率
    .NetworkPowerMaxPowerLimit = {0,20000},               //0x6235~0x6236 入户功率限制：根据用户实际电力布局设定，一般要大于我们设备+总负载的承载功率
    .on_grid_power = {0,0},                           //0x6237~0x6238 最大并网功率 不同产品默认值不同

    .pv_input_mode = 0,                              //0x6250 设置PV输入模式 00:输入独立模式 01:输入并联模式 02:输入恒压模式
    .anti_reverse_flow_enable = 0,                   //0x6251 设置防逆流使能 00:禁止PV给电网送电 01:允许PV给电网送电

    .reserve2 = {0,0},                                //0x6252~0x6253 预留

    .x_boost = 0,                                    //0x6254 设置X-BOOST功能使能   1:使能 0:禁止
    .ups_delay_0ms = 1,                              //0x6255 设置Ups的0毫秒切换功能使能    1:使能 0:禁止
    .backup_output_enable = 1,                       //0x6256 设置本地负载输出使能    1:使能 0:禁止
    .program_ctrl_enable = 0,                        //0x6257 设置程序控制使能 00:禁止 01:使能
    .over_volt_derating_enable = 0,                  //0x6258 设置过压降额使能 00:禁止 01:使能
    .over_freq_derating_enable = 0,                  //0x6259 设置过频降额使能 00:禁止 01:使能
    .under_volt_derating_enable = 0,                 //0x625A 设置欠压降额使能 00:禁止 01:使能
    .under_freq_derating_enable = 0,                 //0x625B 设置欠频降额使能 00:禁止 01:使能
    .island_enable = 1,                              //0x625C 设置孤岛使能    00:禁止 01:使能
    .ac_coupling_enable = 1,                         //0x625D 设置AC耦合使能  00:禁止 01:使能
    .high_low_crossover_enable = 0,                  //0x625E 设置高低穿使能  00:禁止 01:使能
    .drm_state = 0x0010,                             //0x625F 每个bit代表一路DRM 0-关闭，1-打开   默认为DRM4  BIT0:DRM0  BIT1:DRM1  BIT2:DRM2  BIT3:DRM3  BIT4:DRM4
    .LLCEnable = 0,                                  //0x6260 （Pack预充动作完成后开启）0-关闭，1-打开   默认关闭

    .cto_meter = 0,                                  //0x6266 设置CT或电表   00:CT 01:meter
    .ct_direction = 0,                               //0x6267 设置CT方向   00:正方向 01:负方向
    .meter_brand = 0,                                //0x6268 设置电表品牌   详细见协议 不常用变量 00:正泰（CHNT） 01：安科瑞（Acrel）
    .grid_standards = 2,                             //0x6269 设置市场协商电网标准 00:中国 01:德国 02:澳洲A 03:澳洲B 04:澳洲C 05:新西兰 06H：意大利
    .grad_standards_segment = 0,                     //0x626A 设置电网标准细分 详细见协议 不常用变量
    .BatteryChgMaxSOC = 100,                           //0x626B 电池充电截止SOC：0% ~ 100%   默认：100%
    .OnGridBatteryDsgMinSoc = 10,                     //0x626C 并网电池放电截止SOC：0% ~ 100%   默认：10%
    .OffGridBatteryDsgMinSoc = 10,                    //0x626D 离网电池放电截止SOC：0% ~ 100%   默认：10%

    .reserve4={0,0,0,0,0,0,0,0} ,                               //0x626E~0x6275 预留

    .reset_factory = 0,                              //0x6276 设置恢复出厂设置 写入 0X5AA5 则恢复出厂设置
    .on_grid_reconnect_time = 30,                     //0x6277 设置并网重连时间(S)  5 - 300
    .reactive_power_respones_time = 10,               //0x6278 设置无功响应时间(S)
    .reactiveSet = 0,                                //0x6279 无功功率设定，-100~100
    .reactive_power_ctrl_method = 1,                 //0x627A 设置无功控制方式 00:按照功率因数控制 01:按照无功功率控制 02:按照QU曲线控制 03:按照QP曲线控制
    .ActiveSet = 100,                                  //0x627B 有功设置值：0-100	限定最大功率百分比
    .FactorSet = 0,                                  //0x627C 工因设置值：//80-100	无功为负，超前功因 0.8-1 //-(80-100)	无功为正，滞后功因 0.8-1

    .reserve5 = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},                               //0x627D~0x628F 预留

    .remote_shutdown_status = 0,                     //0x6290 设置远程开关机 0x01待机，0x00为正常运行
    .PV_Switch = 1,                                  //0x6291 00H：Disable01H：Enable 默认：Enable
    .PF_Switch = 1,                                  //0x6292 00H：Disable01H：Enable 默认：Enable
    .dsp_wake_en = 3,                                //0X6293 PV WakeUp Status 设置激活使能 01:PV激活使能 02:PV激活完成 03:正常功能
    .pcs_dsg_ctrl = 1,                               //0x6294 设置放电控制 00:监控卡异常禁止放电 01:允许放电 02:禁止放电
    .pcs_chg_ctrl = 1,                               //0x6295 设置充电控制 00:监控卡异常禁止充电 01:允许充电 02:禁止充电
    .pcs_user_type = 0,                              //0x6296 设置用户类型 00:普通用户 01:管理员 02:安装调试 03:内部
    .ignor_ct_err = 0,                               //0x6297 设置消除CT故障 00:不消除CT告警 01:消除CT告警
    .sys_upgrade_status = 0,                         //0x6298 系统升级
    .ElecClear = 0,                                  //0x6299 清除累计电量，0：不清除，1：清除
    .ShutDown = 0,                                   //0x629A 关机，0：不关机，1：关机

    .reserve6 = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},                               //0x629A~0x62AF 预留

    .batt_dsg_volt_point = 3700,                        //0x62B0 设置电池放电截止电压   0.1V 范围 43.2 - 57.6V
    .batt_chg_volt_point = 5500,                        //0x62B1 设置电池充电电压保护电  0.1V 范围 43.2 - 57.6V
    .pv_uvp_point = 750,                               //0x62B2 设置PV输入欠压保护点   0.1V
    .pv_ovp_point = 5200,                               //0x62B3 设置PV输入过压保护点   0.1V
    .ev_uvp_point = 0,                               //0x62B4 设置EV输入欠压保护点   0.1V
    .ev_ovp_point = 0,                               //0x62B5 设置EV输入过压保护点   0.1V
    .grid_volt_low_limit = 1750,                        //0x62B6 设置电网电压下限  0.1V
    .grid_volt_upper_limit = 2750,                      //0x62B7 设置电网电压上限  0.1V
    .grid_freq_low_limit = 4500,                        //0x62B8 设置电网频率下限  0.01HZ
    .grid_freq_upper_limit = 5500,                      //0x62B8 设置电网频率上限  0.01HZ
    .bypass_volt_low_limit = 1750,                      //0x62BA 设置旁路电压下限  0.1V
    .bypass_volt_upper_limit = 2750,                    //0x62BB 设置旁路电压上限  0.1V
    .bypass_freq_low_limit = 4500,                      //0x62BC 设置旁路频率下限  0.01HZ
    .bypass_freq_upper_limit = 5500,                    //0x62BD 设置旁路频率上限  0.01HZ
    .over_volt_derating_point = 270,                    //0x62BE 设置过压降额点    1%
    .over_freq_derating_point = 5400,                    //0x62BF 设置过频降额点    1%
    .under_volt_derating_point = 1800,                   //0x62C0 设置欠压降额点(充电)  1%
    .under_freq_derating_point = 4600,                   //0x62C1 设置欠频降额点(充电)  1%

    .reserve7 = {0,0,0,0,0,0,0,0,0,0,0,0,0,0},                              //0x62C2~0x62CF 预留

    .CalibrationReset = 0,                           //0x62D0 校准复位，即恢复出厂设置
    .reserve8 = {0,0,0,0},                                //0x62D1~0x62D4 预留
    .r_phase_volt_calibration_k_parm = 4096,            //0x62D5 R(U/A)相电压校准系数K: 默认值 4096
    .r_phase_volt_calibration_b_parm = 4096,            //0x62D6 R(U/A)相电压校准偏置B: 默认值 4096
    .s_phase_volt_calibration_k_parm = 4096,            //0x62D7 S(V/B)相电压校准系数K: 默认值 4096
    .s_phase_volt_calibration_b_parm = 4096,            //0x62D8 S(V/B)相电压校准偏置B: 默认值 4096
    .t_phase_volt_calibration_k_parm = 4096,            //0x62D9 T(W/C)相电压校准系数K: 默认值 4096
    .t_phase_volt_calibration_b_parm = 4096,            //0x62DA T(W/C)相电压校准偏置B: 默认值 4096
    .r_phase_cur_calibration_k_parm = 4096,             //0x62DB R(U/A)相电流校准系数K: 默认值 4096
    .r_phase_cur_calibration_b_parm = 4096,             //0x62DC R(U/A)相电流校准偏置B: 默认值 4096
    .s_phase_cur_calibration_k_parm = 4096,             //0x62DD S(V/B)相电流校准系数K: 默认值 4096
    .s_phase_cur_calibration_b_parm = 4096,             //0x62DE S(V/B)相电流校准偏置B: 默认值 4096
    .t_phase_cur_calibration_k_parm = 4096,             //0x62DF T(W/C)相电流校准系数K: 默认值 4096
    .t_phase_cur_calibration_b_parm = 4096,             //0x62F0 T(W/C)相电流校准偏置B: 默认值 4096

    .Inv_Battery_Volt_Calibration_CoefK = 4096,         //0x62F1 逆变电池侧电压校准系数
    .Inv_Battery_Volt_Calibration_CoefB = 4096,         //0x62F2 逆变电池侧电压校准偏置
    .Inv_Battery_Cur_Calibration_CoefK = 4096,          //0x62F3 逆变电池侧电流校准系数
    .Inv_Battery_Cur_Calibration_CoefB = 4096,          //0x62F4 逆变电池侧电流校准偏置
    .Inv_EV_Volt_Calibration_CoefK = 4096,              //0x62F5 逆变EV侧电压校准系数
    .Inv_EV_Volt_Calibration_CoefB = 4096,              //0x62F6 逆变EV侧电压校准偏置
    .Inv_EV_Cur_Calibration_CoefK = 4096,               //0x62F7 逆变EV侧电流校准系数
    .Inv_EV_Cur_Calibration_CoefB = 4096,               //0x62F8 逆变EV侧电流校准偏置

    .pv_ch1_volt_k_calibrate = 4096,                    //0x62f9 PV1电压校准系数K: 默认值 4096
    .pv_ch1_volt_b_calibrate = 4096,                    //0x62fa PV1电压校准偏置B: 默认值 4096
    .pv_ch1_curr_k_calibrate = 4096,                    //0x62fb PV1电流校准系数K: 默认值 4096
    .pv_ch1_curr_b_calibrate = 4096,                    //0x62fc PV1电流校准偏置B: 默认值 4096

    .pv_ch2_volt_k_calibrate = 4096,                    //0x62fd PV2电压校准系数K: 默认值 4096
    .pv_ch2_volt_b_calibrate = 4096,                    //0x62fe PV2电压校准偏置B: 默认值 4096
    .pv_ch2_curr_k_calibrate = 4096,                    //0x62ff PV2电流校准系数K: 默认值 4096
    .pv_ch2_curr_b_calibrate = 4096,                    //0x6300 PV2电流校准偏置B: 默认值 4096

    .pv_ch3_volt_k_calibrate = 4096,                    //0x6301 PV3电压校准系数K: 默认值 4096
    .pv_ch3_volt_b_calibrate = 4096,                    //0x6302 PV3电压校准偏置B: 默认值 4096
    .pv_ch3_curr_k_calibrate = 4096,                    //0x6303 PV3电流校准系数K: 默认值 4096
    .pv_ch3_curr_b_calibrate = 4096,                    //0x6304 PV3电流校准偏置B: 默认值 4096

    .pv_ch4_volt_k_calibrate = 4096,                    //0x6305 PV4电压校准系数K: 默认值 4096
    .pv_ch4_volt_b_calibrate = 4096,                    //0x6306 PV4电压校准偏置B: 默认值 4096
    .pv_ch4_curr_k_calibrate = 4096,                    //0x6307 PV4电流校准系数K: 默认值 4096
    .pv_ch4_curr_b_calibrate = 4096,                    //0x6308 PV4电流校准偏置B: 默认值 4096

    .reserve9 = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

};


// 存储函数
void struct_to_bytes(UdpDspHoldPara_t* src, uint8_t* dest) {
    uint16_t temp[sizeof(UdpDspHoldPara_t)];
    memcpy(temp, src, sizeof(UdpDspHoldPara_t));
    for (size_t i = 0; i < sizeof(UdpDspHoldPara_t); i++)
    {
        dest[2*i] = (uint8_t)(temp[i] & 0x00FF);
        dest[2*i + 1] = (uint8_t)((temp[i] >> 8) & 0x00FF);
    }


}

// 读取函数
void bytes_to_struct(uint8_t* src, UdpDspHoldPara_t* dest) {
    uint16_t temp[sizeof(UdpDspHoldPara_t)];
    for (size_t i = 0; i < sizeof(UdpDspHoldPara_t); i++)
    {
        temp[i] = (src[i*2] & 0x00FF) | ((src[i*2+1] & 0x00FF) << 8);
    }
    memcpy(dest, temp, sizeof(UdpDspHoldPara_t));
}

/* 自动保存镜像：与当前 RAM 比较用；忽略字段变化不触发写 Flash */
static UdpDspHoldPara_t s_hold_saved_mirror;

/* 自动保存比较缓存：用于在 hold_equal_for_autosave() 中存放被掩码后的副本。
 * 出于栈空间考虑（UdpDspHoldPara_t 体积较大，C28x 任务栈有限），
 * 改为文件作用域的全局缓存以避免栈溢出。仅在该函数内部使用，调用不可重入。*/
static UdpDspHoldPara_t s_hold_cmp_a;
static UdpDspHoldPara_t s_hold_cmp_b;

static void hold_mask_excluded_for_compare(UdpDspHoldPara_t *p)
{
    p->ShutDown = 0;
    p->LLCEnable = 0;
    p->remote_shutdown_status = 0;
    p->charging_input_type = 0;
    p->set_batt_chg_power[0] = 0;
    p->set_batt_chg_power[1] = 0;
    p->set_batt_dsg_power[0] = 0;
    p->set_batt_dsg_power[1] = 0;
    p->reset_factory = 0;
}

static bool hold_equal_for_autosave(const UdpDspHoldPara_t *a, const UdpDspHoldPara_t *b)
{
    memcpy(&s_hold_cmp_a, a, sizeof(s_hold_cmp_a));
    memcpy(&s_hold_cmp_b, b, sizeof(s_hold_cmp_b));
    hold_mask_excluded_for_compare(&s_hold_cmp_a);
    hold_mask_excluded_for_compare(&s_hold_cmp_b);
    return memcmp(&s_hold_cmp_a, &s_hold_cmp_b, sizeof(s_hold_cmp_a)) == 0;
}

static void hold_snapshot_sync_from(const UdpDspHoldPara_t *src)
{
    memcpy(&s_hold_saved_mirror, src, sizeof(s_hold_saved_mirror));
}

void UdpDspHoldPara_AutoSaveIfNeeded(void)
{
    /* 守卫 1：恢复出厂设置已下发，正在等待重启 —— 禁止任何自动写回，
     * 否则会把 Flash header 重新写成有效值，导致恢复出厂失败。*/
    if (s_pending_factory_reset)
    {
        return;
    }

    /* 守卫 2：自动保存连续失败超过阈值后停用，保护 Flash 寿命。
     * 一旦 Flash 异常恢复需要重启或显式调用 Save_System_Parameter
     * 成功后再行重置（见下方成功分支）。*/
    if (s_hold_autosave_disabled)
    {
        return;
    }

    if (hold_equal_for_autosave(&gstUdpPcsPara.udp_pcs_hold, &s_hold_saved_mirror))
    {
        return;
    }

    if (Save_System_Parameter(&gstUdpPcsPara.udp_pcs_hold))
    {
        hold_snapshot_sync_from(&gstUdpPcsPara.udp_pcs_hold);
        s_hold_autosave_fail_cnt = 0;
    }
    else
    {
        if (s_hold_autosave_fail_cnt < 0xFFFFu)
        {
            s_hold_autosave_fail_cnt++;
        }
        if (s_hold_autosave_fail_cnt >= HOLD_AUTOSAVE_FAIL_LIMIT)
        {
            /* Flash 持续异常：停止自动重写，等待人工/重启复位。*/
            s_hold_autosave_disabled = true;
        }
    }
}


void Init_System_Parameter(void)
{
    UdpDspHoldPara_t flash_storage;
    uint16_t temp[10];
    static bool flash_update_read;

    /* 读取flash存储的参数 */

    SpiFlashReadBytes(FLASH_DspHoldFRAME_START_ADDR , (uint16_t*)&temp, sizeof(temp));
    if (temp[0] != Flash_DspHoldType[0] || temp[1] != Flash_DspHoldType[1] || temp[2] != Flash_DspHoldType[2] || temp[3] != Flash_DspHoldType[3])
    {
        flash_update_read = false; //flash数据无效
    }
    else
    {
        flash_update_read = true;

    }


    if (flash_update_read == false) //flash数据无效
    {
        /* code */
        struct_to_bytes(&UdpPCS_DspHoldPara_Init, date_buf);
        SpiFlashWriteBytesWithErase(FLASH_DspHoldFRAME_START_ADDR , (uint16_t*)&Flash_DspHoldType, 4);
        SpiFlashWriteBytesWithErase(FLASH_DspHoldPara_START_ADDR , (uint16_t*)date_buf, sizeof(UdpDspHoldPara_t));
        memcpy((uint16_t*)&gstUdpPcsPara.udp_pcs_hold, (uint16_t*)&UdpPCS_DspHoldPara_Init, sizeof(UdpDspHoldPara_t));
    }

    if (flash_update_read == true)
    {
        SpiFlashReadBytes(FLASH_DspHoldPara_START_ADDR , (uint16_t*)&date_buf, sizeof(UdpDspHoldPara_t));
        bytes_to_struct(date_buf, &flash_storage);
        memcpy((uint16_t*)&gstUdpPcsPara.udp_pcs_hold, (uint16_t*)&flash_storage, sizeof(UdpDspHoldPara_t));
    }

    hold_snapshot_sync_from(&gstUdpPcsPara.udp_pcs_hold);
}



bool Save_System_Parameter(UdpDspHoldPara_t* para)
{
    UdpDspHoldPara_t flash_storage;
    uint16_t temp[10];
    static bool flash_save_status;

    /* 写入前对瞬态字段做掩码：将 hold_mask_excluded_for_compare 中排除的
     * 字段（ShutDown、LLCEnable、remote_shutdown_status 等）清零后再落盘，
     * 避免瞬态值在重启后被误读为有效配置。*/
    memcpy((uint16_t*)&s_hold_cmp_a, (uint16_t*)para, sizeof(UdpDspHoldPara_t));
    hold_mask_excluded_for_compare(&s_hold_cmp_a);

    SpiFlashWriteBytesWithErase(FLASH_DspHoldFRAME_START_ADDR , (uint16_t*)&Flash_DspHoldType_init, 4);
    struct_to_bytes(&s_hold_cmp_a, date_buf);
    SpiFlashWriteBytesWithErase(FLASH_DspHoldPara_START_ADDR , (uint16_t*)date_buf, sizeof(UdpDspHoldPara_t));
    SpiFlashWriteBytesWithErase(FLASH_DspHoldFRAME_START_ADDR , (uint16_t*)&Flash_DspHoldType, 4);

    SpiFlashReadBytes(FLASH_DspHoldFRAME_START_ADDR , (uint16_t*)&temp, sizeof(temp));


    if (temp[0] == Flash_DspHoldType[0] && temp[1] == Flash_DspHoldType[1] && temp[2] == Flash_DspHoldType[2] && temp[3] == Flash_DspHoldType[3])
    {
        flash_save_status = true;
    }
    else
    {
        flash_save_status = false;
        return false;
    }
    if (flash_save_status == true)
    {

        SpiFlashReadBytes(FLASH_DspHoldPara_START_ADDR , (uint16_t*)&date_buf, sizeof(UdpDspHoldPara_t));
        bytes_to_struct(date_buf, &flash_storage);
        if (memcmp((uint16_t*)&s_hold_cmp_a, (uint16_t*)&flash_storage, sizeof(UdpDspHoldPara_t)) == 0)
        {
            hold_snapshot_sync_from(para);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}




void ShutDown_SaveFlash(void)
{
    static uint8_t shutdown_state = 0;
    static uint32_t shutdown_count = 0;

    switch (shutdown_state)
    {
        case 0:
        {
            if(gstUdpPcsPara.udp_pcs_hold.ShutDown == 1)
            {
                shutdown_state = 1;
            }
            break;
        }
        case 1:
        {
            gstUdpPcsPara.udp_pcs_hold.ShutDown = 0;    //清除关机标志位
            gstUdpPcsPara.udp_pcs_hold.LLCEnable = 0;    //关机时关闭LLC
            gstUdpPcsPara.udp_pcs_hold.remote_shutdown_status = 0;    //关机时清除远程开关机状态
            gstUdpPcsPara.udp_pcs_hold.charging_input_type = 0 ; //关机时强制发电机模式
            if(Restore_Factory? (Save_System_Parameter(&UdpPCS_DspHoldPara_Init) == true) : (Save_System_Parameter(&gstUdpPcsPara.udp_pcs_hold) == true))  //保存参数到flash
            {
                shutdown_state = 0;
            }
            else
            {
                shutdown_count++;
                if (shutdown_count < 3)
                {
                    shutdown_state = 1;

                }
                else
                {
                    shutdown_state = 0;
                    shutdown_count = 0;
                    gstUdpPcsPara.udp_pcs_hold.ShutDown = 0;
                }

            }
            break;
        }

        default:
            shutdown_state = 0;
            break;
    }
}


void Restore_Factory_Settings(void)
{
    uint16_t temp[10];
    static uint8_t restore_state = 0;
//    static uint32_t restore_count = 0;

    switch (restore_state)
    {

        /*修改恢复出厂设置业务逻辑，恢复出厂设置指令下发后，不立即执行，重启生效*/
        case 0:
        {
            if(gstUdpPcsPara.udp_pcs_hold.reset_factory == 0x5AA5)
            {
                /* 先抢占式置守卫标志：防止本函数把 header 写无效之后、
                 * 在主循环下一拍 UdpDspHoldPara_AutoSaveIfNeeded() 又把
                 * header 写回有效值，造成恢复出厂被回滚（race fix）。*/
                s_pending_factory_reset = true;
                restore_state = 1;
                SpiFlashWriteBytesWithErase(FLASH_DspHoldFRAME_START_ADDR , (uint16_t*)&Flash_DspHoldType_init, 4); //将flash中存储flag置为无效
            }
            break;
        }
        case 1:
        {

            SpiFlashReadBytes(FLASH_DspHoldFRAME_START_ADDR , (uint16_t*)&temp, sizeof(temp));

            if (temp[0] == Flash_DspHoldType_init[0] && temp[1] == Flash_DspHoldType_init[1] && temp[2] == Flash_DspHoldType_init[2] && temp[3] == Flash_DspHoldType_init[3])
            {
                restore_state = 1;
                gstUdpPcsPara.udp_pcs_hold.reset_factory = 0;
            }
            else
            {
                /* 读回 header 不是无效图样，说明置无效未成功：
                 * 清守卫并重试，避免永久屏蔽自动保存。*/
                s_pending_factory_reset = false;
                restore_state = 0;
                gstUdpPcsPara.udp_pcs_hold.reset_factory = 0x5AA5;
            }
            // Restore_Factory = 1;    //设置恢复出厂标志位，重启后执行恢复出厂设置

            // memcpy((uint16_t*)&gstUdpPcsPara.udp_pcs_hold, (uint16_t*)&UdpPCS_DspHoldPara_Init, sizeof(UdpDspHoldPara_t));
            // if(memcmp((uint16_t*)&gstUdpPcsPara.udp_pcs_hold, (uint16_t*)&UdpPCS_DspHoldPara_Init, sizeof(UdpDspHoldPara_t)) == 0)
            // {
            //     restore_state = 0;
            // }
            // else
            // {
            //     restore_count++;
            //     if (restore_count < 3)
            //     {
            //         restore_state = 1;

            //     }
            //     else
            //     {
            //         restore_state = 0;
            //         restore_count = 0;
            //         gstUdpPcsPara.udp_pcs_hold.reset_factory = 0;
            //     }

            // }
            break;
        }

        default:
            restore_state = 0;
            break;
    }
}

