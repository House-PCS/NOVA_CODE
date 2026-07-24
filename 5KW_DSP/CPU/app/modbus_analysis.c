/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * <tr><td>2024-2-4    <td>V1.0.1    <td>xingxiao    <td>modbus_reg LSB Save
  * </table>
  ******************************************************************************
  */

#include "modbus_analysis.h"
#include "dev_sensor.h"
#include "inverter_loop_cntl.h"
#include "mppt_process.h"
#include "dev_flash.h"
#include "fault_diagnosis.h"
#include "algo_err_check.h"
#include "sys_info.h"
#include "algo_info.h"
#include "authentication_func.h"
#include "energy_management.h"

#define MOD_FUN_ERROR       (1)
#define MOD_REG_ADDR_ERROR  (2)
#define MOD_REG_DATA_ERROR  (3)
#define MOD_COMM_ERROR      (4)

#define GPIO0_OUT           (30)
#define GPIO1_OUT           (28)
#define GPIO2_OUT           (29)
#define GPIO3_OUT           (31)
#define GPIO4_OUT           (32)
#define GPIO5_OUT           (33)
#define DSP_HI_Sync_OUT     (0)
#define DSP_LOW_Sync_OUT    (1)

#define DSP_HI_Sync_IN      (62)
#define DSP_LOW_Sync_IN     (63)
#define TEST_MODE (0)


//#define MODBUSREG_INFO_SAVE_ADD  0x200000

const uint16_t ModelID = 0xC281;
const uint16_t Hardware_Ver[2] = {0, 1};
const uint16_t Software_Ver[3] = {9, 1, 43}; // 20260228,41固件,20260410,临时42固件,20260415,43固件,20260429,新bug修改，44固件,20260507,45固件,20260514,46固件,20260518,46固件T2,20260519,47固件CSIP

//static ModbusReadRemainAndInput_t _gModbusRead;
static uint8_t _ModbusAckData[300];
static uint16_t _ModbusAckSize;
uint16_t gUpgraderRestartFlag;
uint16_t gSysStartFlag = 0;
ModbusReadReg_t      ModbusReadReg;
ModbusReg_t          ModbusReg;
ModbusReadRegTest_t  ModbusReadRegTest;
ModbusRegTest_t      ModbusRegTest;
ModbusRegCSIP_t      gstModbusRegCSIP;

static InverterState_t* _gInvSta;
const AlgoInfo_t* _gInfoState;
uint8_t gOpenInv = 0;
uint8_t dsg_ctrl = 0;
uint8_t chg_ctrl = 0;
uint8_t remove_open_ctrl = 2;
uint8_t Antireverseflow_flag = 0;
uint16_t gUpgraderRestartFlag = 0;
static void _TestModeEnterJudge(uint16_t value);
static void _TestModePwmRelayCtrl(uint16_t value);
static void _TestModeIOCtrl(uint16_t value);
void ModbusCtrl(); // 20260514,新增寄存器控制函数

/**
* @brief: ModbusRegInit
* @param: void
* @note:
*
*/
void ModbusRegInit(void)
{
    uint16_t i = 0;
    uint16_t len = 0;
    uint16_t sw_version = 0;
    uint16_t hw_version = 0;

    hw_version = (Hardware_Ver[0] << 8) | Hardware_Ver[1];
    sw_version = (Software_Ver[0] << 12) | (Software_Ver[1] << 6) | Software_Ver[2];

    /* 测试数据 */
    len = sizeof(ModbusReadReg_t);
    for(i = 0; i < len; i ++)
    {
        ((uint16_t*)&ModbusReadReg)[i] = 0;
    }

    memset((uint16_t*)&ModbusReg, 0, sizeof(ModbusReg_t));
    memset((uint16_t*)&gSysSetInfo, 0, sizeof(SysInfoSet_t));

    ModbusReadReg.model_id = ModelID;
    ModbusReadReg.hardware_version = hw_version;
    ModbusReadReg.software_version = sw_version;
    memset(ModbusReadReg.sn, 0x10, 10);
    memset((uint16_t*)&ModbusReadRegTest, 0, sizeof(ModbusReadRegTest_t));
    memset((uint16_t*)&ModbusRegTest, 0, sizeof(ModbusRegTest_t));
    memset((uint16_t*)&gSysInfoGet, 0, sizeof(SysInfoGet_t));
    memset((float*)&gApprovalStdInfo, 0, sizeof(ApprovalStdInfo_t));
    memset(&gstModbusRegCSIP, 0, sizeof(ModbusRegCSIP_t));

    ModbusReg.parallel_mode_enable = 0; //20260226，修改并机使能位置，每次关机后重新开机会保持关机前的并机使能状态
    gSysSetInfo.parallel_mode_enable = 0; //20260226，修改并机使能位置，每次关机后重新开机会保持关机前的并机使能状态

    UsageInfoGet();
    AdminInfoGet();
    DefaultInfoGet();
    ModbusReg.ongrid_reconnect_time = 30;
    ModbusReg.remote_standby_cmd = 0;
    ModbusReg.pcs_chg_ctrl = 0;
    ModbusReg.pcs_dsg_ctrl = 0;
    ModbusReg.ct_direction = 0;
    ModbusReg.on_grid_power_hard_limit = 1;
    ModbusReg.on_grid_power_soft_limit = 1;

    ModbusReg.ct_or_meter = 0;
    ModbusReg.dsp_wake_en = 0;
    ModbusReg.batt_volt = 0;
    ModbusReg.ignore_ct_err = 0; //ignore_ct_err
    ModbusReg.sysupgradestatus.value = 0;
    ModbusReg.pv_max_chg_power = 5000;
    ModbusReg.max_inv_power = 2500;
    ModbusReg.dc_source = 0;
    ModbusReg.remote_standby_cmd = 1;   //2026年6月12日幸逍：上电默认将待机标志置位，等待监控发起机
    memcpy((uint16_t*)&gSysSetInfo, (uint16_t *)&ModbusReg, sizeof(ModbusReg_t));
    _gInvSta = InverterGetState();
    _gInfoState = AlgoStateGet();

}

static void ModBusReadRegUpdate()
{
#if TEST_MODE
    ModbusReadReg.sys_input_stat = 0;
    ModbusReadReg.system_status = 2;
    ModbusReadReg.inv_status = 2;
    ModbusReadReg.dcdc_status = 2;
    ModbusReadReg.dsp_alarm_code_high = 0;
    ModbusReadReg.dsp_alarm_code_low = 2;
    ModbusReadReg.dsp_error_code_high = 0;
    ModbusReadReg.dsp_error_code_low = 0;
    ModbusReadReg.bus_voltage = 200*10;//母线电压
    ModbusReadReg.inverter_radiator_temperature = 200;
    ModbusReadReg.chassis_internal_temperature = 250;
    ModbusReadReg.battery_voltage = 60*10;//电池测电压
    ModbusReadReg.battery_current = 5*10;//电池测电流
    ModbusReadReg.battery_power = 300;//电池测功率
    ModbusReadReg.battery_reactive_power = 100;
    ModbusReadReg.battery_active_power = 200;

    ModbusReadReg.r_phase_grid_voltage = 500*10;
    ModbusReadReg.r_phase_meter_current = 10*10;
    ModbusReadReg.r_phase_grid_frequency = 4999;
    ModbusReadReg.r_phase_meter_apparentpower = 5000;
    ModbusReadReg.r_phase_meter_activepower = 4000;
    ModbusReadReg.r_phase_meter_reactivepower = 1000;

    ModbusReadReg.oil_current = 200*10;//油机电压  是微网吗？
    ModbusReadReg.oil_voltage = 10*10;//油机电流
    ModbusReadReg.oil_frequency = 4988;
    ModbusReadReg.oil_power = 2000;//油机功率
    ModbusReadReg.r_phase_inverter_voltage = 200*10;//r相逆变电压（单相）
    ModbusReadReg.r_phase_inverter_current = 10*10;
    ModbusReadReg.r_phase_inverter_frequency = 5000;
    ModbusReadReg.r_phase_inverter_apparentpower = 2000;//r相视在功率
    ModbusReadReg.r_phase_inverter_activepower = 1500;
    ModbusReadReg.r_phase_inverter_reactivepower = 500;

    ModbusReadReg.r_phase_backup_voltage = 1000;
    ModbusReadReg.r_phase_backup_current = 100;
    ModbusReadReg.r_phase_backup_apparentpower = 1000;
    ModbusReadReg.r_phase_backup_activepower=  800;
    ModbusReadReg.r_phase_backup_reactivepower=200;
    ModbusReadReg.r_phase_load_activepower = 1500;
    ModbusReadReg.r_phase_load_reactivepower=500;
    ModbusReadReg.r_phase_load_apparentpower = 2000;

    ModbusReadReg.load_electric_cost_total_high = 0;
    ModbusReadReg.load_electric_cost_total_low = 3000;
    ModbusReadReg.electric_purchase_total_high = 0;
    ModbusReadReg.electric_purchase_total_low = 1000;

    ModbusReadReg.pv1_input_current = 30;
    ModbusReadReg.pv1_input_voltage = 450;
    ModbusReadReg.pv1_input_power = 135;
    ModbusReadReg.pv2_input_current =40 ;
    ModbusReadReg.pv2_input_voltage = 500;
    ModbusReadReg.pv2_input_power = 200;

    ModbusReadReg.pv_power_generation_day=300;
    ModbusReadReg.pv_power_generation_month_high=0;
    ModbusReadReg.pv_power_generation_month_low= 5000;
    ModbusReadReg.pv_power_generation_year_low = 30000;
    ModbusReadReg.pv_power_generation_total_low = 60000;
    ModbusReadReg.pv1_power_generation_day = 100 ;
    ModbusReadReg.pv1_power_generation_month_low = 1000;
    ModbusReadReg.pv1_power_generation_year_low =  8000;
    ModbusReadReg.pv1_power_generation_total_low = 20000;

    ModbusReadReg.pv2_power_generation_day = 200 ;
    ModbusReadReg.pv2_power_generation_month_low = 1100;
    ModbusReadReg.pv2_power_generation_year_low =  8800;
    ModbusReadReg.pv2_power_generation_total_low = 22020;

    ModbusReadReg.electric_sale_total_low = 60000;
    ModbusReadReg.pv_temperature = 450;
    ModbusReadReg.on_grid_time = 123;

#else
    memcpy((uint16_t*)&ModbusReadReg.inverter_model, (uint16_t*)&gSysInfoGet, sizeof(SysInfoGet_t));
    if(gParallelComm.mode == 1)
    {
        ModbusReadReg.pv1_input_power += gParallelComm.slave_info.slave_pv_power;
        ModbusReadReg.battery_power += gParallelComm.slave_info.slave_bat_power;
        //电池电流暂时无法增加从机，因为主机暂时无法获取从机电池电流
        ModbusReadReg.r_phase_backup_activepower += gParallelComm.slave_info.slave_backup_power;
        ModbusReadReg.r_phase_backup_apparentpower += gParallelComm.slave_info.slave_backup_power;
        ModbusReadReg.r_phase_load_activepower = (float)(gSysInfoGet.r_phase_load_activepower) - gParallelComm.slave_info.slave_grid_port_power;
        ModbusReadReg.r_phase_load_apparentpower = (float)(gSysInfoGet.r_phase_load_activepower) - gParallelComm.slave_info.slave_grid_port_power;
    }



//    MemcpyU16toU8((uint8_t*)&ModbusReadReg.inverter_model, (uint16_t*)&gSysInfoGet, sizeof(SysInfoGet_t));

//    ModbusReadReg.bus_voltage = gstSensorData.bus_voltage*10;//母线电压
//    //逆变器散热器温度
//    //机箱内部温度
//    ModbusReadReg.battery_voltage = bat_voltage_ave*10;//电池测电压
//    ModbusReadReg.battery_current = bat_current_ave*10;//电池测电流
//    ModbusReadReg.battery_power = bat_current_ave * bat_voltage_ave;//电池测功率
//    //电池有功功率；
//    ModbusReadReg.r_phase_grid_voltage = (_gInvSta->sta_grid_vol_rms)*10;  //（R单相）三相电流、电压、频率、功率、有功功率、无功功率
//    ModbusReadReg.r_phase_meter_current = (_gInvSta->sta_grid_cur_rms)*10; //r相电表电流==相电网电流？
////    ModbusReadReg.r_phase_meter_apparentpower = _gInvSta->sta_grid_vol_rms * _gInvSta->sta_grid_cur_rms;
//
//    ModbusReadReg.r_phase_meter_apparentpower = Grid_power_ave;//瞬时值滤波
//
//
//    ModbusReadReg.oil_current = gstSensorData.micro_grid_voltage*10;//油机电压  是微网吗？
//    ModbusReadReg.oil_voltage = gstSensorData.micro_grid_current*10;//油机电流
//    ModbusReadReg.oil_power = gstSensorData.micro_grid_voltage * gstSensorData.micro_grid_current;//油机功率                                                               //油机频率
//    ModbusReadReg.r_phase_inverter_voltage = _gInvSta->sta_inv_vol_rms*10;//r相逆变电压（单相）
//    ModbusReadReg.r_phase_inverter_current = _gInvSta->sta_inv_cur_rms*10; //r相逆变电流（单相）
//    ModbusReadReg.r_phase_inverter_apparentpower = (_gInvSta->sta_inv_vol_rms) * (_gInvSta->sta_inv_cur_rms);//r相视在功率
//    ModbusReadReg.r_phase_inverter_activepower = _gInvSta->sta_inv_active_pwr;
//    ModbusReadReg.r_phase_inverter_reactivepower = _gInvSta->sta_inv_reactive_pwr;
//    ModbusReadReg.r_phase_inverter_frequency=_gInvSta->sta_inv_freq*100;
//    ModbusReadReg.r_phase_backup_current = _gInvSta->sta_load_cur_rms;
//    ModbusReadReg.r_phase_backup_voltage = (_gInvSta->sta_load_vol_rms)*10;
//    ModbusReadReg.r_phase_backup_apparentpower = (_gInvSta->sta_load_vol_rms) * (_gInvSta->sta_load_cur_rms);
////    ModbusReadReg.r_phase_load_apparentpower = (_gInvSta->sta_load_vol_rms) * (_gInvSta->sta_load_cur_rms); //单相（r）负载有功功率
//    ModbusReadReg.pv1_input_current = pv1_current_ave*10;
//    ModbusReadReg.pv1_input_voltage = pv1_voltage_ave*10;
//    ModbusReadReg.pv1_input_power = pv1_voltage_ave*pv1_current_ave;
//   ModbusReadReg.pv2_input_current = pv2_current_ave*10;
//   ModbusReadReg.pv2_input_voltage = pv2_voltage_ave*10;
//   ModbusReadReg.pv2_input_power = pv2_voltage_ave*pv2_current_ave;
////    ModbusReadReg.mppt_status = 0; //暂定待机；
////    ModbusReadReg.system_status = 0;//暂定市电
////    ModbusReadReg.inverter_model = 1;//逆变：5K
//    ModbusReadReg.dsp_alarm_code_low = gstFaultInfo.worning_code.value;
//    ModbusReadReg.dsp_alarm_code_high = 0;
//    ModbusReadReg.dsp_error_code_high = gAlgoErrDef.byte[1];
//    ModbusReadReg.dsp_error_code_low = gAlgoErrDef.byte[0];
////    ModbusReadReg.dsp_error_code_high = 1;
////    ModbusReadReg.dsp_error_code_low = 1;
//
//
//      ModbusReadReg.load_electric_cost_total_high = 0;
//      ModbusReadReg.load_electric_cost_total_low = 0 ;
//      ModbusReadReg.grid_connect_count_high= 0;
//      ModbusReadReg.grid_connect_count_low = 0;
//      ModbusReadReg.pv1_power_generation_total_high = 0;
//      ModbusReadReg.pv1_power_generation_total_low = 0;
//      ModbusReadReg.pv2_power_generation_total_high = 0;
//      ModbusReadReg.pv2_power_generation_total_low = 0;
//
//    ModbusReadReg.electric_purchase_total_high = electricity_purchase >> 16;
//    ModbusReadReg.electric_purchase_total_low = electricity_purchase;
//    ModbusReadReg.electric_sale_total_high = electricity_sales >> 16;
//    ModbusReadReg.electric_sale_total_low = electricity_sales;
//
//    ModbusReadReg.inverter_radiator_temperature = gstSensorData.temp[4];
//    //    ModbusReadReg.inv_status = _gStaGridConnect;
//    //    ModbusReadReg.inverter_model =
//    //    ModbusReadReg.r_phase_grid_frequency                                //r相逆变频率（单相）



#endif

    if(ModbusRegTest.enter_test_mode)
    {
        ModbusReadRegTest.io_input_status = (GPIO_ReadPin(DSP_HI_Sync_IN) & 0x0001) | (((GPIO_ReadPin(DSP_LOW_Sync_IN) & 0x0001) << 1));
        ModbusReadRegTest.adc_bus_voltage           = ADC_BUS_VOLTAGE;
        ModbusReadRegTest.adc_bat_voltage           = ADC_BAT_VOLTAGE;
        ModbusReadRegTest.adc_load_voltage          = ADC_LOAD_VOLTAGE;
        ModbusReadRegTest.adc_microgrid_voltage     = ADC_MICRO_GRID_VOLTAGE;
        ModbusReadRegTest.adc_powergrid_voltage     = ADC_POWER_GRID_VOLTAGE;
        ModbusReadRegTest.adc_invert_voltage        = ADC_INVERT_VOLTAGE;
        ModbusReadRegTest.adc_pv2_voltage           = ADC_PV2_VOLTAGE;
        ModbusReadRegTest.adc_pv1_voltage           = ADC_PV1_VOLTAGE;
        ModbusReadRegTest.adc_pv2_current           = ADC_PV2_CURRENT;
        ModbusReadRegTest.adc_pv1_current           = ADC_PV1_CURRENT;
        ModbusReadRegTest.adc_bat_current           = ADC_BAT_CURRENT;
        ModbusReadRegTest.adc_load_current          = ADC_LOAD_CURRENT;
        ModbusReadRegTest.adc_microgrid_current     = ADC_MICRO_GRID_CURRENT;
        ModbusReadRegTest.adc_powergrid_current     = ADC_POWER_GRID_CURRENT;
        ModbusReadRegTest.adc_leak_current          = ADC_LEAK_CURRENT;
        ModbusReadRegTest.adc_inductor_current      = ADC_INDUCTOR_CURRENT;
        ModbusReadRegTest.adc_transformer_current   = ADC_TRANSFORMER_CURRENT;
        ModbusReadRegTest.adc_dc_component          = ADC_DC_COMPONENT;
        ModbusReadRegTest.adc_ct_current            = ADC_CT_CURRENT;
        ModbusReadRegTest.ref_1_5v                  = gstSensorData.temp[0];
        ModbusReadRegTest.temp_a1                   = garrTemperInitialData[1];
        ModbusReadRegTest.temp_a2                   = garrTemperInitialData[2];
        ModbusReadRegTest.temp_a3                   = garrTemperInitialData[3];
        ModbusReadRegTest.temp_a4                   = garrTemperInitialData[4];
        ModbusReadRegTest.temp_a5                   = garrTemperInitialData[5];
        ModbusReadRegTest.temp_a6                   = garrTemperInitialData[6];
        ModbusReadRegTest.temp_a7                   = garrTemperInitialData[7];
    }
}

/**
* @brief: ModbusRTUSend
* @param: p
* @param: size
* @note:
*
*/
void ModbusRTUSend(uint8_t *p, uint16_t size)
{
    uint16_t crc;

    crc = CrcModbus(p, size);
    _ModbusAckData[size] = crc & 0xFF;
    _ModbusAckData[size + 1] = (crc >> 8) & 0xFF;

//    _gModbusSendMaxTime = 1000 * (8 + 4) * Uart1_ModS_Handle.tx_size / Uart1_ModS_Handle.uart_cfg->baud_rate + 2;//单位ms,8表示数据位时长，4表示起始+停止+GAP时长, +2表示增加一些冗余
    SciaSend(_ModbusAckData, _ModbusAckSize + 2);
//    UartSend(&Uart1_ModS_Handle);
//    _gModbusSendTimeout = tim_get_systime();
}

static void _ModbusReadInput(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num, uint8_t slave_addr)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint16_t valid_reg_start_addr = 0;
    uint16_t* p_modbus = NULL;

    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _ModbusAckData[offset] = slave_addr;

    /*做数据正确性判断*/
    if((reg_start_addr >= MODBUS_READ_REG_START) && (reg_start_addr < MODBUS_REG_START))
    {
        valid_reg_start_addr = MODBUS_READ_REG_START;
        p_modbus = (uint16_t*)&ModbusReadReg;
    }
    else if((reg_start_addr >= MODBUS_READ_REG_TEST_START) && (reg_start_addr <= MODBUS_READ_REG_TEST_END))
    {
        valid_reg_start_addr = MODBUS_READ_REG_TEST_START;
        p_modbus = (uint16_t*)&ModbusReadRegTest;
    }
   
    if(reg_num > 125) // 20260429，优化代码
    {
        error = MOD_REG_ADDR_ERROR;
    }
    /*
     *  error判断，根据业务添加代码
     */
    ModBusReadRegUpdate();
    if(error == 0)
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x04;
        /*字节数*/
        _ModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体
        MemcpyU16toU8SwapByte(&_ModbusAckData[offset + 3], p_modbus + (reg_start_addr - valid_reg_start_addr), reg_num * 2);
        _ModbusAckSize = reg_num * 2 + 3 + offset;
    }
    else
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x84;
        /*错误码*/
        _ModbusAckData[offset + 2] = error;
        _ModbusAckSize = 3 + offset;
    }

}

/**
* @brief: _ModbusReadRemain
* @param: modbus_type
* @param: reg_start_addr
* @param: reg_num
* @param: slave_addr
* @param: error_code
* @return:
* @note:
*
*/
static void _ModbusReadRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num, uint8_t slave_addr, uint8_t error_code)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint16_t valid_reg_start_addr = 0;
    uint16_t* p_modbus = NULL;

    error = error_code;
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _ModbusAckData[offset] = slave_addr;


    /*做数据正确性判断*/
    if((reg_start_addr >= MODBUS_REG_START) && (reg_start_addr <= MODBUS_REG_END))
    {
        valid_reg_start_addr = MODBUS_REG_START;
        p_modbus = (uint16_t*)&ModbusReg;
    }
    else if((reg_start_addr >= MODBUS_REG_TEST_START) && (reg_start_addr <= MODBUS_REG_TEST_END))
    {
        valid_reg_start_addr = MODBUS_REG_TEST_START;
        p_modbus = (uint16_t*)&ModbusRegTest;
    }
    else if((reg_start_addr >= MODBUS_REG_CSIP_START) && (reg_start_addr <= MODBUS_REG_CSIP_END))
    {
        valid_reg_start_addr = MODBUS_REG_CSIP_START;
        p_modbus = (uint16_t*)&gstModbusRegCSIP;
    }
    else
    {
        error = MOD_REG_ADDR_ERROR;
    }

    /*
     *  error判断，根据业务添加代码
     */

//    ModBusReadRegUpdate();
    if(error == 0)
    {

        /*功能码*/
        _ModbusAckData[offset + 1] = 0x03;
        /*字节数*/
        _ModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体
        MemcpyU16toU8SwapByte(&_ModbusAckData[offset + 3], p_modbus + (reg_start_addr - valid_reg_start_addr), reg_num * 2);

        _ModbusAckSize = reg_num * 2 + 3 + offset;
    }
    else
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x83;
        /*错误码*/
        _ModbusAckData[offset + 2] = error;
        _ModbusAckSize = 3 + offset;
    }

}

/**
* @brief: _ModbusWriteSingleRemain
* @param: modbus_type
* @param: reg_start_addr
* @param: reg_data
* @param: slave_addr
* @return:
* @note:
*
*/
static void _ModbusWriteSingleRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_data, uint8_t slave_addr)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint16_t valid_reg_start_addr = 0;
    uint16_t* p_modbus = NULL;

    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }

    /*地址*/
    _ModbusAckData[offset] = slave_addr;
    /*做数据正确性判断*/
    if((reg_start_addr >= MODBUS_REG_START) && (reg_start_addr <= MODBUS_REG_END))
    {
        valid_reg_start_addr = MODBUS_REG_START;
        p_modbus = (uint16_t*)&ModbusReg;

        //判断是否接收到监控的待机控制信号
        if((reg_start_addr - MODBUS_REG_START) == (&ModbusReg.remote_standby_cmd - &ModbusReg))
        {
            gSysStartFlag = 1;
        }
    }
    else if((reg_start_addr >= MODBUS_READ_REG_TEST_START) && (reg_start_addr <= MODBUS_READ_REG_TEST_END))
    {
        valid_reg_start_addr = MODBUS_READ_REG_TEST_START;
        p_modbus = (uint16_t*)&ModbusRegTest;
    }
    else if((reg_start_addr >= MODBUS_REG_CSIP_START) && (reg_start_addr <= MODBUS_REG_CSIP_END))
    {
        valid_reg_start_addr = MODBUS_REG_CSIP_START;
        p_modbus = (uint16_t*)&gstModbusRegCSIP;
    }
    else
    {
        error = MOD_REG_ADDR_ERROR;
    }
    /*
     *  error判断，根据业务添加代码
     */
    /*如Error为0执行写操作的任务*/
    if(error == 0)
    {   
        *(p_modbus + (reg_start_addr - valid_reg_start_addr)) = reg_data;

        /*功能码*/
        _ModbusAckData[offset + 1] = 0x06;
        /*寄存器地址*/
        _ModbusAckData[offset + 2] = (reg_start_addr >> 8) & 0xFF;
        _ModbusAckData[offset + 3] = reg_start_addr & 0xFF;
        _ModbusAckData[offset + 4] = (reg_data >> 8) & 0xFF;
        _ModbusAckData[offset + 5] = reg_data & 0xFF;
        _ModbusAckSize = 4 + 2 + offset;
    }
    else
    {
        /*功能码*/
        _ModbusAckData[1] = 0x86;
        /*错误码*/
        _ModbusAckData[2] = error;
        _ModbusAckSize = 3 + offset;
    }
    UsageInfoSave();
    ResetFactory();
}

/**
* @brief: _ModbusWriteMultiRemain
* @param: modbus_type
* @param: reg_start_addr
* @param: p_reg_data
* @param: reg_num
* @param: slave_addr
* @param: error_code
* @return:
* @note:
*
*/
static void _ModbusWriteMultiRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint8_t *p_reg_data, uint16_t reg_num, uint8_t slave_addr, uint8_t error_code)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint8_t i = 0;
    uint16_t value = 0;
    uint16_t valid_reg_start_addr = 0;
    uint16_t* p_modbus = NULL;

    error = error_code;
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _ModbusAckData[offset] = slave_addr;

    /*做数据正确性判断*/
    if((reg_start_addr >= MODBUS_REG_START) && (reg_start_addr <= MODBUS_REG_END))
    {
        valid_reg_start_addr = MODBUS_REG_START;
        p_modbus = (uint16_t*)&ModbusReg;

        //判断是否接收到监控的待机控制信号
        if(((reg_start_addr - MODBUS_REG_START) <= (&ModbusReg.remote_standby_cmd - &ModbusReg)) && \
            ((reg_start_addr - MODBUS_REG_START + reg_num) > (&ModbusReg.remote_standby_cmd - &ModbusReg)))
        {
            gSysStartFlag = 1;
        }
    }
    else if((reg_start_addr >= MODBUS_READ_REG_TEST_START) && (reg_start_addr <= MODBUS_READ_REG_TEST_END))
    {
        valid_reg_start_addr = MODBUS_READ_REG_TEST_START;
        p_modbus = (uint16_t*)&ModbusRegTest;
    }
    else if((reg_start_addr >= MODBUS_REG_CSIP_START) && (reg_start_addr <= MODBUS_REG_CSIP_END))
    {
        valid_reg_start_addr = MODBUS_REG_CSIP_START;
        p_modbus = (uint16_t*)&gstModbusRegCSIP;
    }

    else
    {
        error = MOD_REG_ADDR_ERROR;
    }

    /*
     *  error判断，根据业务添加代码
     */
    /*如Error为0执行写操作的任务*/

    if(error == 0)
    {
        /* 写寄存器操作 */
        for (i = 0; i < reg_num; i++)
        {
            value = p_reg_data[i * 2 + 1] + (p_reg_data[i * 2] << 8);

            *(p_modbus + (reg_start_addr + i - valid_reg_start_addr)) = value;

            if(reg_start_addr + i == 0x622f)
            {
                if(ModbusReg.sysupgradestatus.b.restate_state == 1)
                {
                    gUpgraderRestartFlag = 1;
                }
                if(gUpgraderRestartFlag == 1)
                {
                    ModbusReg.sysupgradestatus.b.mintor_test_restate = 0;
                }
            }
//            /*放电使能*/
//            if(reg_start_addr + i == 0x620D)
//            {
//                dsg_ctrl = value;
//            }
//            /*充电使能*/
//            if(reg_start_addr + i == 0x620E)
//            {
//                chg_ctrl = value;
//            }
//            /*远程开关机*/
//            if(reg_start_addr + i == 0x6215)
//            {
//                if(value == 2)
//                {
//                    remove_open_ctrl = 2;
//                }
//                else if(value == 1)
//                {
//                    remove_open_ctrl = 1;
//                }
//                else if(value == 0)
//                {
//                    remove_open_ctrl = 0;
//                }
//            }
//            /*防逆流开关*/
//            if(reg_start_addr + i == 0x6204)
//            {
//                if(value == 1)
//                {
//                    gSysSetInfo.antireverseflow_enable = 1;//允许馈电
//                }
//                else
//                {
//                    Antireverseflow_flag = 0;//不允许馈电
//                }
//            }

            /* 测试模式进入判断 */
            if((reg_start_addr + i) == (MODBUS_REG_TEST_START + (&ModbusRegTest.enter_test_mode - &ModbusRegTest.enter_test_mode)))
            {
                _TestModeEnterJudge(value);
            }

            /* IO寄存区控制 */
            if((reg_start_addr + i) == (MODBUS_REG_TEST_START + (&ModbusRegTest.pwm_relay_out_ctrl - &ModbusRegTest.enter_test_mode)))
            {
                _TestModePwmRelayCtrl(value);
            }

            if((reg_start_addr + i) == (MODBUS_REG_TEST_START + (&ModbusRegTest.io_out_ctrl  - &ModbusRegTest.enter_test_mode)))
            {
                _TestModeIOCtrl(value);
            }
        }

        /*功能码*/
        _ModbusAckData[offset + 1] = 0x10;
        /*寄存器地址*/
        _ModbusAckData[offset + 2] = (reg_start_addr >> 8) & 0xFF;
        _ModbusAckData[offset + 3] = reg_start_addr & 0xFF;
        _ModbusAckData[offset + 4] = (reg_num >> 8) & 0xFF;
        _ModbusAckData[offset + 5] = reg_num & 0xFF;
        _ModbusAckSize = 4 + 2 + offset;

    }
    else
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x90;
        /*错误码*/
        _ModbusAckData[offset + 2] = error;
        _ModbusAckSize = 3 + offset;
    }
    UsageInfoSave();
    ResetFactory();
//    ModbusCtrl(); // 20260514,新增寄存器控制函数

//    MemcpyU16toU8((uint8_t*)&gSysSetInfo, (uint16_t*)&ModbusReg, sizeof(ModbusReg_t));
//    ModHoldRegSave();


}

static EN_MODBUS_REV_STATUS sta = EN_MODBUS_REV_STATUS_IDLE;


/**
* @brief: ModbusCmdProcess
* @param:
* @note: 轮询执行
*
*/
void ModbusCmdProcess()
{
    static SciRevData_t *p_rtu_cmd;

    static uint8_t func_code;
    static EN_MODBUS_TYPE   modbus_type = EN_MODBUS_RTU;
    static uint8_t   slave_addr = 0;
    uint8_t error_code = 0;
    SciRevData_t com_revdata ;
    static uint32_t com_data_send_wait_time = 0;
    static uint32_t mod_data_send_wait_time = 0;

    memset((uint16_t *)&com_revdata,0,sizeof(SciRevData_t));
    MidComserviceHandle();

    switch(sta)
    {
        case EN_MODBUS_REV_STATUS_IDLE:
        {
            if(QueueGet(&gstSciRevQueue, (void *)&p_rtu_cmd) == true)
            {
                if(p_rtu_cmd->data_type == TYPE_MODBUS_DATA)
                {
                    func_code = p_rtu_cmd->pData[1];
                    modbus_type = EN_MODBUS_RTU;
                    slave_addr = p_rtu_cmd->slave_addr;
                    sta = EN_MODBUS_REV_STATUS_ANALYSIS;
                }
                else if(p_rtu_cmd->data_type == TYPE_COM_DATA)
                {
                    com_revdata.data_size = p_rtu_cmd->data_size;
                    com_revdata.pData = p_rtu_cmd->pData;
                    //TODO 升级协议解析
                    ProtoUnpackJudge(&com_revdata);
                    com_data_send_wait_time = TimGetSystime();
                    sta = EN_COM_REV_STATUS_ACK_WAIT;
                }
            }

            break;
        }
        case EN_MODBUS_REV_STATUS_ANALYSIS:
        {
            switch(func_code)
            {
                case EN_MODBUS_FUNC_CODE_READ_INPUT:
                {
                    uint16_t reg_start_addr;
                    uint16_t reg_num;
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_num = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusReadInput(modbus_type, reg_start_addr, reg_num, slave_addr);
                    break;
                }
                case EN_MODBUS_FUNC_CODE_READ_REMAIN:
                {
                    uint16_t reg_start_addr;
                    uint16_t reg_num;
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_num = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];

                        if((p_rtu_cmd->data_size != 8) || (reg_num > 128))
                        {
                            error_code = EN_MODBUS_ERROR_CODE_INVALID_CMD;
                        }
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusReadRemain(modbus_type, reg_start_addr, reg_num, slave_addr, error_code);
                    break;
                }
                case EN_MODBUS_FUNC_CODE_WRITE_SINGLE:
                {
                    uint16_t reg_start_addr;
                    uint16_t reg_data;
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_data = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusWriteSingleRemain(modbus_type, reg_start_addr, reg_data, slave_addr);
                    break;
                }
                case EN_MODBUS_FUNC_CODE_WRITE_MULTI:
                {
                    uint16_t reg_start_addr;
                    uint16_t reg_num;
                    uint8_t *p_reg_data = NULL;
                    uint8_t byte_num;

                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_num = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];
                        p_reg_data = &p_rtu_cmd->pData[7];
                        byte_num = p_rtu_cmd->pData[6];
                        if((p_rtu_cmd->data_size < 11) || (byte_num != (2 * reg_num)))
                        {
                            error_code = EN_MODBUS_ERROR_CODE_INVALID_CMD;
                        }
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusWriteMultiRemain(modbus_type, reg_start_addr, p_reg_data, reg_num, slave_addr, error_code);
                    sta = EN_MODBUS_REV_STATUS_ACK;
                    break;
                }
                default:
                {
                    uint8_t offset;
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        offset = 0;
                    }
                    else
                    {
                        offset = 6;
                    }
                    _ModbusAckData[offset] = slave_addr;
                    _ModbusAckData[offset + 1] = func_code | 0x80;
                    _ModbusAckData[offset + 2] = EN_MODBUS_ERROR_CODE_INVALID_CMD;
                    _ModbusAckSize = 3 + offset;
                    break;
                }
            }
            mod_data_send_wait_time = TimGetSystime();
            sta = EN_MODBUS_REV_STATUS_ACK_SEND_WAIT;
//            sta = EN_MODBUS_REV_STATUS_ACK;
            break;
        }
        case EN_MODBUS_REV_STATUS_ACK_SEND_WAIT:
        {
            if(TimDifftimeFromNow(mod_data_send_wait_time) > 100)
            {
                sta = EN_MODBUS_REV_STATUS_ACK;
            }
            break;
        }
        case EN_MODBUS_REV_STATUS_ACK:
        {
            if(modbus_type == EN_MODBUS_RTU)
            {
                ModbusRTUSend(_ModbusAckData, _ModbusAckSize);
                sta = EN_MODBUS_REV_STATUS_ACK_WAIT;
            }
            else
            {
                /*后期增加MODBUS_TCP*/
            }
            break;
        }
        case EN_MODBUS_REV_STATUS_ACK_WAIT:
        {
            if(modbus_type == EN_MODBUS_RTU)
            {
//                if(DevModbusSendComplete() == true)
//                {
                sta = EN_MODBUS_REV_STATUS_IDLE;
//                }
            }
            else
            {
                /*后期增加MODBUS_TCP,可不用等待直接跳入IDLE*/
            }
            break;
        }
        case EN_COM_REV_STATUS_ACK_WAIT:
        {
            if(TimDifftimeFromNow(com_data_send_wait_time) > 100)
            {
                sta = EN_COM_SEND;
            }
            break;
        }
        case EN_COM_SEND:
        {
            DataSendPolling(gReadResponsed);
            sta = EN_MODBUS_REV_STATUS_IDLE;
            break;
        }
        default:
            break;
    }

}

static void _TestModeEnterJudge(uint16_t value)
{
    uint16_t i = 0;
    if(value == 1)
    {

        if(FlashTest() == true)
        {
            ModbusReadRegTest.flash_test_result = 1;
        }

//        InitGpio();
        /* GPIO0 */
        GPIO_SetupPinMux(GPIO0_OUT, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(GPIO0_OUT, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(GPIO0_OUT, 0);

        /* GPIO1 */
        GPIO_SetupPinMux(GPIO1_OUT, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(GPIO1_OUT, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(GPIO1_OUT, 0);

        /* GPIO2 */
        GPIO_SetupPinMux(GPIO2_OUT, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(GPIO2_OUT, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(GPIO2_OUT, 0);

        /* GPIO3 */
        GPIO_SetupPinMux(GPIO3_OUT, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(GPIO3_OUT, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(GPIO3_OUT, 0);

        /* GPIO4 */
        GPIO_SetupPinMux(GPIO4_OUT, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(GPIO4_OUT, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(GPIO4_OUT, 0);

        /* GPIO5 */
        GPIO_SetupPinMux(GPIO5_OUT, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(GPIO5_OUT, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(GPIO5_OUT, 0);

        /* DSP_HI_Sync_OUT */
        GPIO_SetupPinMux(DSP_HI_Sync_OUT, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(DSP_HI_Sync_OUT, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(DSP_HI_Sync_OUT, 0);

        /* DSP_LOW_Sync_OUT */
        GPIO_SetupPinMux(DSP_LOW_Sync_OUT, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(DSP_LOW_Sync_OUT, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(DSP_LOW_Sync_OUT, 0);

        /* DSP_HI_Sync_IN */
        GPIO_SetupPinMux(DSP_HI_Sync_IN, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(DSP_HI_Sync_IN, GPIO_OUTPUT, GPIO_INPUT);
        GPIO_WritePin(DSP_HI_Sync_IN, 0);

        /* DSP_LOW_Sync_IN */
        GPIO_SetupPinMux(DSP_LOW_Sync_IN, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(DSP_LOW_Sync_IN, GPIO_OUTPUT, GPIO_INPUT);
        GPIO_WritePin(DSP_LOW_Sync_IN, 0);

        /* pwm & relay */

        for(i = 2; i <= 23; i++)
        {
            GPIO_SetupPinMux(i, GPIO_MUX_CPU1, 0);
            GPIO_SetupPinOptions(i, GPIO_OUTPUT, GPIO_PUSHPULL);
            GPIO_WritePin(i, 0);
        }

        /***********************************relay***************************************/
        /* GPIO87 */
        GPIO_SetupPinMux(87, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(87, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(87, 0);

        /* GPIO88 */
        GPIO_SetupPinMux(88, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(88, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(88, 0);

        /* GPIO89 */
        GPIO_SetupPinMux(89, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(89, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(89, 0);

        /* GPIO90 */
        GPIO_SetupPinMux(90, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(90, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(90, 0);

        /* GPIO91 */
        GPIO_SetupPinMux(91, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(91, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(91, 0);

        /* GPIO92 */
        GPIO_SetupPinMux(92, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(92, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(92, 0);

        /* GPIO93 */
        GPIO_SetupPinMux(93, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(93, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(93, 0);

        /* GPIO94 */
        GPIO_SetupPinMux(94, GPIO_MUX_CPU1, 0);
        GPIO_SetupPinOptions(94, GPIO_OUTPUT, GPIO_PUSHPULL);
        GPIO_WritePin(94, 0);


    }

}

void _TestModePwmRelayCtrl(uint16_t value)
{
    uint16_t i = 0;

    if(value)
    {
        for(i = 2; i <= 23; i++)
        {
            GPIO_WritePin(i, 1);
        }

        GPIO_WritePin(87, 1);
        GPIO_WritePin(88, 1);
        GPIO_WritePin(89, 1);
        GPIO_WritePin(90, 1);
        GPIO_WritePin(91, 1);
        GPIO_WritePin(92, 1);
        GPIO_WritePin(93, 1);
        GPIO_WritePin(94, 1);

    }
    else
    {
        for(i = 2; i <= 23; i++)
        {
            GPIO_WritePin(i, 0);
        }

        GPIO_WritePin(87, 0);
        GPIO_WritePin(88, 0);
        GPIO_WritePin(89, 0);
        GPIO_WritePin(90, 0);
        GPIO_WritePin(91, 0);
        GPIO_WritePin(92, 0);
        GPIO_WritePin(93, 0);
        GPIO_WritePin(94, 0);
    }
}

void _TestModeIOCtrl(uint16_t value)
{
    GPIO_WritePin(GPIO0_OUT, (value & 0x0001));
    GPIO_WritePin(GPIO1_OUT, ((value >> 1) & 0x0001));
    GPIO_WritePin(GPIO2_OUT, ((value >> 2) & 0x0001));
    GPIO_WritePin(GPIO3_OUT, ((value >> 3) & 0x0001));
    GPIO_WritePin(GPIO4_OUT, ((value >> 4) & 0x0001));
    GPIO_WritePin(GPIO5_OUT, ((value >> 5) & 0x0001));
    GPIO_WritePin(DSP_HI_Sync_OUT, ((value >> 6) & 0x0001));
    GPIO_WritePin(DSP_LOW_Sync_OUT, ((value >> 7) & 0x0001));
}

void ModbusCtrl() // 20260514,新增寄存器控制函数
{
    if(gSysSetInfo.remote_standby_cmd == 1) // 20260514,新增当待机寄存器为1时，控制按键LED亮灭
    {
        GPIO_WritePin(KEY_GPIO, 0); //按键LED灭
    }
    else
    {
        GPIO_WritePin(KEY_GPIO, 1); //按键LED亮
    }

}

//
//void ModbusRegSave()
//{
//    SpiFlashWriteBytesWithErase(MODBUSREG_INFO_SAVE_ADD, (uint16_t *)&ModbusReg.battery_charging_voltage, sizeof(ModbusReg_t));
//}
//
//void ModbusRegRead()
//{
//    SpiFlashReadBytes(MODBUSREG_INFO_SAVE_ADD, (uint16_t *)&ModbusReg.battery_charging_voltage,  sizeof(ModbusReg_t));
//}
//
////void CtrlInfoGet()
////{
////
////
////}
