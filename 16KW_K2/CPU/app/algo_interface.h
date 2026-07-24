#ifndef __ALGO_INTERFACE_H_
#define __ALGO_INTERFACE_H_
#include "F28x_Project.h"
#include "AlgoFw/component/Comp_NoiseVar.h"

typedef struct inv_interface_stru_t //逆变器控制以及交互接口
{
    uint16_t en_inv_cmd; //逆变器使能命令
    uint16_t self_test_finish; //逆变器使能命令
    uint16_t reset_inv_cmd; //逆变器复位命令
    uint16_t openloop_inv_cmd; //开环运行命令
    uint16_t closeloop_inv_cmd; //闭环运行命令
    uint16_t en_oning_grid_cmd; //使能并网中外部接口
    uint16_t en_on_grid_cmd; //使能并网命令外部接口
    uint16_t clear_fault_cmd; //清除故障标志位
    float32 active_pwr_ref;
    float32 reactive_pwr_ref;
    float32 reactive_pwr_compute;
    float32 inv_cur_amp_max;
} InvInterface_t;

extern InvInterface_t gstInvInterface; //逆变器接口
extern Comp_NoiseVar_t gstPvPwrNoiseVar[4]; //PV功率噪声方差计算组件
extern Comp_NoiseVar_t gstPvPwrTotalNoiseVar;
//extern PwrClaCompute_t gstBackupData;
//extern PwrClaCompute_t gstInvData;
void InterfaceCmd(uint16_t en_inv, uint16_t en_grid, uint16_t loop_type, uint16_t clear_fault, uint16_t self_test_finish);
void AlgoInterfaceInit(void);
void SetPwrRef(float32 active_pwr_ref, float32 reactive_pwr_ref);

#define RELAY_TOTAL_OPEN GpioDataRegs.GPCSET.bit.GPIO87 = 1  //打开总继电器
#define RELAY_TOTAL_CLOSE GpioDataRegs.GPCCLEAR.bit.GPIO87 = 1  //关闭总继电器
#define RELAY_BACKUP_OPEN GpioDataRegs.GPCSET.bit.GPIO88 = 1  //打开负载继电器
#define RELAY_BACKUP_CLOSE GpioDataRegs.GPCCLEAR.bit.GPIO88 = 1  //关闭负载继电器
#define RELAY_GRID_OPEN GpioDataRegs.GPCSET.bit.GPIO89 = 1  //打开电网继电器
#define RELAY_GRID_CLOSE GpioDataRegs.GPCCLEAR.bit.GPIO89 = 1  //关闭电网继电器
#define RELAY_GEN_OPEN GpioDataRegs.GPCSET.bit.GPIO90 = 1  //打开油机继电器
#define RELAY_GEN_CLOSE GpioDataRegs.GPCCLEAR.bit.GPIO90 = 1  //关闭油机继电器
#define RELAY_GEN_START_OPEN GpioDataRegs.GPASET.bit.GPIO30 = 1  //断开油机取电继电器
#define RELAY_GEN_START_CLOSE GpioDataRegs.GPACLEAR.bit.GPIO30 = 1  //闭合油机取电继电器
// #define RELAY_EPS_PE_OPEN GpioDataRegs.GPCSET.bit.GPIO90 = 1  //打开总继电器
// #define RELAY_EPS_PE_CLOSE GpioDataRegs.GPCCLEAR.bit.GPIO90 = 1  //打开总继电器

#define GSTATE_RELAY_TOTAL  GpioDataRegs.GPCDAT.bit.GPIO87
#define GSTATE_RELAY_BACKUP GpioDataRegs.GPCDAT.bit.GPIO88
#define GSTATE_RELAY_GRID   GpioDataRegs.GPCDAT.bit.GPIO89
#define GSTATE_RELAY_GEN    GpioDataRegs.GPCDAT.bit.GPIO90
#define GSTATE_RELAY_GEN_START    GpioDataRegs.GPADAT.bit.GPIO30
#endif
