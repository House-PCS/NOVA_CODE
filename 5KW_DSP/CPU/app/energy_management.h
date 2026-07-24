/*
 * energy_management.h
 *
 *  Created on: 2024脛锚4脭脗24脠脮
 *      Author: Nova006352
 */

#ifndef APP_ENERGY_MANAGEMENT_H_
#define APP_ENERGY_MANAGEMENT_H_

#include "F28x_Project.h"

/*逆变、电池工作模式*/
typedef enum
{
    EN_INV_BAT_IDLE,
    EN_INV_BAT_SPONTANEOUS_USE,
    EN_INV_BAT_PEAK_VALLEY,
    EN_INV_BAT_BATTERY_PRIORITY,
    EN_INV_BAT_MALFUNCTION,
} EN_INV_BAT_WORK_MODE;


/*PV工作模式*/
typedef enum
{
    EN_PV_IDLE,
    EN_PV_SOFT_START,
    EN_PV_RUNNING,
    EN_PV_MALFUNCTION,
} EN_PV_WORK_MODE;


/*并离网状态*/
typedef enum
{
    EN_OFF_GRID_MODE,//0
    EN_OFF_GRID_TO_GRID_CONNECT_MODE,//1
    EN_GRID_CONNECT_MODE,//2
    EN_GRID_CONNECT_TO_OFF_GRID_MODE,//3
} EN_PARALLEL_OFF_NETWORK_STA;

/*充电模式*/
typedef enum
{
    EN_POLLING_SERVICE_PV,
    EN_POLLING_SERVICE_INV,
    EN_POLLING_SERVICE_BAT,
} EN_POLLING_SERVICE_TYPE;

/*工作模式*/
typedef enum
{
    EN_SPONTANEOUS_USE_MODE,
    EN_BAT_PRIORITY_MODE,
    EN_PEAK_VALLEY_MODE,
    EN_STAND_BY_MODE,
} EN_WORK_MODE;
/*运行状态*/
typedef enum
{
    EN_LOAD_WITH_GRID_ONLY,  //并网，从电网下拉功率，PV不存在
    EN_LOAD_WITH_GRID_AND_PVSOFT,  //并网，从电网下拉功率，PV存在,PV软启动过程
    EN_LOAD_WITH_GRID_AND_PV,  //并网，从电网下拉功率，PV存在,PV启动完成
    EN_GENRATION_WITH_GRID_ONLY, //并网，向电网上推功率，PV不存在
    EN_GENRATION_WITH_GRID_AND_PVSOFT, //并网，向电网上推功率，PV存在,PV软启动过程
    EN_GENRATION_WITH_GRID_AND_PV,  //并网，从电网下拉功率，PV存在,PV启动完成
    EN_STOPGENRATION_WITH_GRID_ONLY,  //并网，禁止发电(包含并网功率为0和防逆流),PV不存在
    EN_STOPGENRATION_WITH_GRID_AND_PVSOFT,  //并网，禁止发电(包含并网功率为0和防逆流),PV软起动过程
    EN_STOPGENRATION_WITH_GRID_AND_PV,  //并网，防逆流开启，PV存在，PV启动完成
    EN_GRID_NORMAL,  //并网, 正常状态
    EN_OFFGRID_ONLY,  //离网, 无PV
    EN_OFFGRID_AND_PV,  //离网，有PV
} EN_POWER_WORK_MODE_STA;

/*逆变等级*/
typedef enum
{
    EN_INV_PWR_LEVEL1,
    EN_INV_PWR_LEVEL2,
    EN_INV_PWR_LEVEL3,
} EN_INV_PWR_LEVEL;



//并机相关
typedef struct _parallel_slave_info_stru_t
{
    float slave_grid_port_allow_max_power;
    float slave_grid_port_allow_min_power;
    float slave_grid_port_power;
    float slave_backup_power;
    float slave_pv_power;
    float slave_bat_power;
    float slave_bat_allow_max_power;
    float slave_bat_allow_min_power;
} ParallelSlaveInfo_t;



typedef struct _parallel_slave_set_stru_t
{
    float slave_grid_port_power_set;
} ParallelSlaveSet_t;

typedef struct _parallel_comm_stru_t
{
    uint32_t mode;//0-正常模式，1-主机模式，2-从机模式
    uint16_t parallel_master_forbidden_flag;//并机禁止标志，从机不满足并机条件时触发
    ParallelSlaveInfo_t slave_info;
    ParallelSlaveSet_t slave_set;
} ParallelComm_t;

//CSIP
typedef struct _csip_info_stru_t
{
    uint32_t count_csip                         ;   
    float32 num_ct_power_hold                   ;   //获取实时ct功率

    int32_t power_ac_in_ct_import               ;   //换算ac口最大输入功率限制
    int32_t power_ac_out_ct_export              ;   //换算ac口最大输出功率限制

    uint32_t num_import_max_power               ;  //读取CT口最大入户功率
    uint32_t num_export_max_power               ;  //读取CT口最大卖电功率
    uint32_t num_ac_in_max_power                ;  //读取AC口最大输入功率
    uint32_t num_ac_out_max_power               ;  //读取AC口最大输出功率
    uint32_t num_import_max_power_last          ;  //上一次读取CT口最大入户功率
    uint32_t num_export_max_power_last          ;  //上一次读取CT口最大卖电功率
    uint32_t num_ac_in_max_power_last           ;  //上一次读取AC口最大输入功率
    uint32_t num_ac_out_max_power_last          ;  //上一次读取AC口最大输出功率

    uint16_t flag_csip_enable                   ;  //csip状态使能
    uint16_t flag_import_ct_source              ;  //读取CT输入数据来源
    uint16_t flag_import_ac_source              ;  //读取AC输入数据来源
    uint16_t flag_export_ct_source              ;  //读取CT输出数据来源
    uint16_t flag_export_ac_source              ;  //读取AC输出数据来源

    uint16_t flag_import_power_ramp_up_type     ;  //读取CT口输入功率爬坡类型    
    uint16_t flag_export_power_ramp_up_type     ;  //读取CT口输出功率爬坡类型      
    uint16_t flag_ac_in_power_ramp_up_type      ;  //读取AC口输入功率爬坡类型
    uint16_t flag_ac_out_power_ramp_up_type     ;  //读取AC口输出功率爬坡类型
    uint16_t flag_import_power_ramp_up_type_last;  //读取CT口输入功率爬坡类型    
    uint16_t flag_export_power_ramp_up_type_last;  //读取CT口输出功率爬坡类型      
    uint16_t flag_ac_in_power_ramp_up_type_last;  //读取AC口输入功率爬坡类型
    uint16_t flag_ac_out_power_ramp_up_type_last;  //读取AC口输出功率爬坡类型

    uint16_t num_import_power_ramp_up_value     ;  //读取CT口输入功率爬坡参数
    uint16_t num_export_power_ramp_up_value     ;  //读取CT口输出功率爬坡参数    
    uint16_t num_ac_in_power_ramp_up_value      ;  //读取AC口输入功率爬坡参数    
    uint16_t num_ac_out_power_ramp_up_value     ;  //读取AC口输出功率爬坡参数
    uint16_t num_import_power_ramp_up_value_last;   //读取CT口输入功率爬坡参数
    uint16_t num_export_power_ramp_up_value_last;   //读取CT口输出功率爬坡参数
    uint16_t num_ac_in_power_ramp_up_value_last ;   //读取AC口输入功率爬坡参数
    uint16_t num_ac_out_power_ramp_up_value_last;   //读取AC口输出功率爬坡参数


    uint16_t num_import_power_ramp_up_time      ;  //计算CT口输入功率爬坡时间
    uint16_t num_export_power_ramp_up_time      ;  //计算CT口输出功率爬坡时间  
    uint16_t num_ac_in_power_ramp_up_time       ;  //计算AC口输入功率爬坡时间 
    uint16_t num_ac_out_power_ramp_up_time      ;  //计算AC口输出功率爬坡时间    

    uint16_t num_import_power_ramp_up_rate      ;  //计算CT口输入功率爬坡斜率
    uint16_t num_export_power_ramp_up_rate      ;  //计算CT口输出功率爬坡斜率
    uint16_t num_ac_in_power_ramp_up_rate       ;  //计算AC口输入功率爬坡斜率
    uint16_t num_ac_out_power_ramp_up_rate      ;  //计算AC口输出功率爬坡斜率

    uint16_t num_import_power_ramp_up_step      ; //计算CT口输入功率爬坡步长
    uint16_t num_export_power_ramp_up_step      ; //计算CT口输出功率爬坡步长
    uint16_t num_ac_in_power_ramp_up_step       ; //计算AC口输入功率爬坡步长
    uint16_t num_ac_out_power_ramp_up_step      ; //计算AC口输出功率爬坡步长


    int32_t final_safe_import_power_limit       ;   //最终安全输入功率限制
    int32_t final_safe_export_power_limit       ;   //最终安全输出功率限制
    uint16_t num_import_power_step              ;   //最终计算输入功率步长 w/100ms
    uint16_t num_export_power_step              ;   //最终计算输出功率步长 w/100ms
    uint16_t num_import_power_step_reduce       ;   //最终计算输入功率步长 w/100ms（减小）
    uint16_t num_export_power_step_reduce       ;   //最终计算输出功率步长 w/100ms（减小）    
    int32_t target_import_power                 ;   //最终计算输入功率
    int32_t target_export_power                 ;   //最终计算输出功率

    uint16_t flag_import_power_status          ; // 0ct主导 1ac主导 2默认
    uint16_t flag_export_power_status          ; // 0ct主导 1ac主导 2默认

    int16_t cease_power                     ; // 20260518，新增csip并网认证变量，逆变器与电网停止功率交互
    int16_t disconnect_grid                 ; // 20260518，新增csip并网认证变量，逆变器主动脱网
} CSIP_t;

typedef struct _grid_ctrl_stru_t
{
    int32_t import_power;   //输入功率
    int32_t export_power;   //输出功率
} GridCtrl_t;


extern ParallelComm_t gParallelComm;

extern float  chargepower ;
extern float  dischargepower ;
extern float  pushpower ;
extern uint16_t gRelayCloseFlag;

extern uint16_t gRunstate;
extern uint16_t  PvSetRunstate;
//extern uint16_t dsg_ctrl ;
//extern uint16_t chg_ctrl ;

extern float  gChgPower;
extern float  gDsgPower;
extern int32_t  gFeedPower;

//float _Bat_Ref = 0;
extern float gPvPowerSet;
extern float gGridPowerSet;
extern float gGridReactivePowerSet;
extern float gBatRealPower;
extern float gBatSetPower;
extern float gOilPowerSet;
extern uint32_t gOnGridCnt;
extern uint16_t gOnGridTime;
extern uint16_t gEnChgClose;
extern uint16_t gEnDsgClose;
extern EN_INV_PWR_LEVEL geInvPwrLevel;
extern uint16_t gMaxPvPwr;
extern float32 ct_change_trend;
extern float32 inv_change_trend;
extern float32 grid_change_trend;
extern float32 grid_dir;
extern float32 ct_dir;
extern float gGridPwr;
extern float gCtPwr;

extern float32 inv_output_act_pwr ;
extern float32 inv_output_reactive_pwr ;


//float BatChargeGpGvCntl(float basics,float ref,float bat_pwr,float bat_vol);
//float BatChargeGpGvCntlGrid(float basics,float ref,float bat_pwr,float bat_vol);

void MainPollingProcess();
void SpontaneousUse();
void PeakValley();
void BatPriority();
void CtReverseCheck();
void PvInvAllowMaxPower(void);
void UserBackupSwitchSet(void);
uint16_t DeviceSwitchCtrl();
uint16_t CheckCtExistsErr();
void LLC_Set_EN();
void CsipInit(void);
void CSIPOnGridCertification(void);
bool GetInvStandbyState(void);

#define VALUE_LIMIT(x,max,min)        ((x>max)?max:((x<min)?min:x))
#define POWER_VALUE_LIMIT(x,max)      ((x>max)?max:x)

#endif /* APP_ENERGY_MANAGEMENT_H_ */




