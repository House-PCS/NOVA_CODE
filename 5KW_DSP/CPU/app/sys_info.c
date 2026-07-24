/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sys_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-7-16
  * @brief
  ******************************************************************************
  * @attention=
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-7-16    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "sys_info.h"
#include "cpu_timers.h"
#include "modbus_analysis.h"
#include "mppt_process.h"
#include "inverter_loop_cntl.h"
#include "algo_err_check.h"
#include "fault_diagnosis.h"
#include "dev_sensor.h"
#include "mppt_process.h"
#include "llc_dcdc_cntl.h"
#include "energy_management.h"
#include "algo_info.h"
#include "algorithm_interface.h"
#include "activation.h"
#include "generator.h"


#define ADMIN_INFO_ADDR     (0x280000)
#define ADMIN_INFO_ADDR_FLAG     (0x290000)

#define COEFF_AVE 0.9f

static InverterState_t* _gInvSta;
static uint32_t _gModHoldRegFlashPageIndex = 0;
static uint32_t _gElectricInfoFlashPageIndex = 0;
static uint16_t _garrWriteData[0x100];
static uint16_t _garrReadData[0x100];
static uint16_t garrValidFlag[4] = {0x5A, 0x5A, 0x5A, 0x5A};
static uint8_t admin_flag = 0;
static uint8_t user_flag = 0;
static uint8_t cnt_flag = 0;

static uint32_t pv1_power_genration_total = 0;
static uint32_t pv2_power_genration_total = 0;
static uint32_t purchase_power_genration_total = 0;
static uint32_t sale_power_genration_total = 0;
static uint32_t load_power_genration_total = 0;
static uint32_t norload_power_genration_total = 0;
static uint32_t bat_chg_power_genration_total = 0;
static uint32_t bat_dsg_power_genration_total = 0;

float gAntiReFlowValue = 0;
extern uint16_t Enoil;
int ct_dir_fix = 1;
//static const AlgoInfo* _gAlgoInfo;
SysInfoSet_t gSysSetInfo;
SysInfoSet_t gAdminSetInfo;
SysInfoSet_t gDefaultSetInfo;

SysInfoGet_t gSysInfoGet;
extern float flag_CT_exist;
extern float InvState;
extern ParallelComm_t gParallelComm;
extern EN_PARALLEL_OFF_NETWORK_STA geInvsta;

extern int16_t flag_CertificationTest;
extern int16_t flag_NetDisableTest_Enable;

/**
* @brief: ModHoldRegSave
* @param: void                        
* @return: true or false
* @note: 
*
*/
bool ModHoldRegSave(void)
{
    uint16_t i = 0;
    uint16_t j = 0;
    uint32_t write_addr = 0;
    uint16_t data_len = 0;

    write_addr = MOD_HOLD_REG_FLASH_ADDR + (_gModHoldRegFlashPageIndex % BACK_UP_PAGE_NUM) * FLASH_ERASE_SECTOR_SIZE;
    data_len = sizeof(ModbusReg_t) << 1;
    /* u16转为u8 */
    MemcpyU16toU8((uint8_t*)_garrWriteData, (uint16_t*)&ModbusReg, data_len );
//    memcpy((uint8_t*)_garrWriteData, (uint8_t*)&ModbusReg, data_len);

    for(i = 0; i < 3; i++)
    {
        SpiFlashWriteBytesWithErase(write_addr, _garrWriteData, data_len);
        SpiFlashReadBytes(write_addr, _garrReadData, data_len);
        if(memcmp(_garrWriteData, _garrReadData, data_len) == 0)
        {
            _garrWriteData[0] = garrValidFlag[0];
            _garrWriteData[1] = garrValidFlag[1];
            _garrWriteData[2] = garrValidFlag[2];
            _garrWriteData[3] = garrValidFlag[3];
            _garrWriteData[4] = _gModHoldRegFlashPageIndex & 0xFF;
            _garrWriteData[5] = (_gModHoldRegFlashPageIndex >> 8) & 0xFF;
            _garrWriteData[6] = (_gModHoldRegFlashPageIndex >> 16) & 0xFF;
            _garrWriteData[7] = (_gModHoldRegFlashPageIndex >> 24) & 0xFF;

            data_len = 8;
//            write_addr = write_addr + FLASH_ERASE_SECTOR_SIZE - data_len;
            write_addr = MOD_HOLD_REG_VALID_FLASH_ADDR + (_gModHoldRegFlashPageIndex % BACK_UP_PAGE_NUM) * FLASH_ERASE_SECTOR_SIZE;

            for(j = 0; j < 3; j++)
            {
                SpiFflashWriteBytesWithoutErase(write_addr, _garrWriteData, data_len);
                SpiFlashReadBytes(write_addr, _garrReadData, data_len);
                if(memcmp((uint8_t*)_garrWriteData, (uint8_t*)_garrReadData, data_len) == 0)
                {
                    _gModHoldRegFlashPageIndex ++;
                    cnt_flag = 1;
                    return true;
                }
            }
        }
    }

    return false;
}

/**
* @brief: ModHoldRegRead
* @param: void                        
* @return: true or false
* @note: 
*
*/
bool ModHoldRegRead(void)
{
    uint16_t i = 0;
    uint32_t arr_addr_index[BACK_UP_PAGE_NUM];

    memset(arr_addr_index, 0, BACK_UP_PAGE_NUM);

    for(i = 0; i < BACK_UP_PAGE_NUM; i++)
    {
        SpiFlashReadBytes((MOD_HOLD_REG_VALID_FLASH_ADDR + (uint32_t)i * FLASH_ERASE_SECTOR_SIZE), _garrReadData, 8);
        if(memcmp(_garrReadData, garrValidFlag, 4) == 0)
        {
            arr_addr_index[i] = ((uint32_t)_garrReadData[4] & 0xFF) | (((uint32_t)_garrReadData[5] << 8) & 0xFF00) | \
                                (((uint32_t)_garrReadData[6] << 16) & 0xFF0000) | (((uint32_t)_garrReadData[7] << 24) & 0xFF000000);

            if(arr_addr_index[i] == 0xFFFFFFFF)
            {
                arr_addr_index[i] = 0;
            }
        }
        else
        {
            arr_addr_index[i] = 0;
        }
    }

    for(i = 0; i < BACK_UP_PAGE_NUM; i++)
    {
        if (arr_addr_index[i] > _gModHoldRegFlashPageIndex)
        {
            _gModHoldRegFlashPageIndex = arr_addr_index[i];
        }
    }
    if(_gModHoldRegFlashPageIndex == 0)
    {
        uint16_t len = sizeof(ModbusReg_t);
        for(i = 0; i < len; i ++)
        {
            ((uint16_t*)&ModbusReg)[i] = 0;
        }
        ModbusReg.reactive_power_control_method = 0;
        ModbusReg.power_factor_setting_value = 100;
        ModbusReg.pcs_chg_ctrl = 0;
        ModbusReg.pcs_dsg_ctrl = 0;
        ModbusReg.ongrid_reconnect_time = 30;
        ModbusReg.charging_input_typesetting = 0;
        ModbusReg.ac_output_voltage_range = 4;
        ModbusReg.ac_output_frequency_range = 2;
        ModbusReg.grid_standards = 2;
        ModbusReg.grids_tandard_segmentation = 0;
        ModbusReg.ongrid_power = 0;
        ModbusReg.meter_brand = 0;//20260409，增加电表型号默认值设定
        ModbusReg.ct_or_meter = 0;
        ModbusReg.antireverseflow_enable = 1;

        ModbusReg.grid_frequency_lower_limit = 45;
        ModbusReg.grid_frequency_upper_limit = 55;
        ModbusReg.grid_voltage_lower_limit = 170;
        ModbusReg.grid_voltage_upper_limit = 265;
        ModbusReg.network_power_limit_enable = 0;
        ModbusReg.network_power_limit = 15000;
        ModbusReg.residual_current_protection_threshold = 150; //20251124，提高默认漏电流阈值到150mA
        ModbusReg.residual_current_change_response_time = 300;

        ModbusReg.off_grid_sensitivity = 3;  // 20260415，新增0ms脱网响应时间等级，默认3
        ModbusReg.ups_delay0ms = 1; //20251124，增加UPS的0ms切换，20260122，增加0ms功能默认关闭，20260127，增加0ms功能默认打开

        ModbusReg.backup_output_enable = 1;//20260121,按照客户要求设置默认backup打开
    }
    else
    {
        SpiFlashReadBytes((MOD_HOLD_REG_FLASH_ADDR + (_gModHoldRegFlashPageIndex % BACK_UP_PAGE_NUM) * FLASH_ERASE_SECTOR_SIZE), \
                      _garrReadData, sizeof(ModbusReg_t) << 1);
        MemcpyU8toU16((uint16_t*)&ModbusReg, _garrReadData, sizeof(ModbusReg_t));
    }

    _gModHoldRegFlashPageIndex ++;
    cnt_flag = 1;

    return true;
}

/**
* @brief: ElectricInfoSave
* @param: void                        
* @return: true or false
* @note: 
*
*/
bool ElectricInfoSave(void)
{
    uint16_t i = 0;
    uint16_t j = 0;
    uint32_t write_addr = 0;
    uint16_t data_len = 0;
    uint16_t *p = NULL;

    p = _garrWriteData;
    write_addr = ELECTRIC_INFO_FLASH_ADDR + (_gElectricInfoFlashPageIndex % BACK_UP_PAGE_NUM) * FLASH_ERASE_SECTOR_SIZE;
    data_len = 18;

    /* 监控卡会定时读Modbus寄存器，这里不做寄存器更新 */
    /* 负载总用电量 */


    *(p++) = gSysInfoGet.load_electric_cost_total_low & 0xFF;
    *(p++) = (gSysInfoGet.load_electric_cost_total_low >> 8) & 0xFF;
    *(p++) = gSysInfoGet.load_electric_cost_total_high & 0xFF;
    *(p++) = (gSysInfoGet.load_electric_cost_total_high >> 8) & 0xFF;

    /* 并网次数 */
    *(p++) = gSysInfoGet.grid_connect_count_low & 0xFF;
    *(p++) = (gSysInfoGet.grid_connect_count_low >> 8) & 0xFF;
    *(p++) = gSysInfoGet.grid_connect_count_high & 0xFF;
    *(p++) = (gSysInfoGet.grid_connect_count_high >> 8) & 0xFF;

    /* 买电总量 */
    *(p++) = gSysInfoGet.electric_purchase_total_low & 0xFF;
    *(p++) = (gSysInfoGet.electric_purchase_total_low >> 8) & 0xFF;
    *(p++) = gSysInfoGet.electric_purchase_total_high & 0xFF;
    *(p++) = (gSysInfoGet.electric_purchase_total_high >> 8) & 0xFF;

    /* 卖电总量 */
    *(p++) = gSysInfoGet.electric_sale_total_low & 0xFF;
    *(p++) = (gSysInfoGet.electric_sale_total_low >> 8) & 0xFF;
    *(p++) = gSysInfoGet.electric_sale_total_high & 0xFF;
    *(p++) = (gSysInfoGet.electric_sale_total_high >> 8) & 0xFF;

    /*  PV1总发电量 */
    *(p++) = gSysInfoGet.pv1_power_generation_total_low & 0xFF;
    *(p++) = (gSysInfoGet.pv1_power_generation_total_low >> 8) & 0xFF;
    *(p++) = gSysInfoGet.pv1_power_generation_total_high & 0xFF;
    *(p++) = (gSysInfoGet.pv1_power_generation_total_high >> 8) & 0xFF;

    /*  PV2总发电量 */
    *(p++) = gSysInfoGet.pv2_power_generation_total_low & 0xFF;
    *(p++) = (gSysInfoGet.pv2_power_generation_total_low >> 8) & 0xFF;
    *(p++) = gSysInfoGet.pv2_power_generation_total_high & 0xFF;
    *(p++) = (gSysInfoGet.pv2_power_generation_total_high >> 8) & 0xFF;


    /*  PV3总发电量 */
    *(p++) = gSysInfoGet.pv3_power_generation_total_low & 0xFF;
    *(p++) = (gSysInfoGet.pv3_power_generation_total_low >> 8) & 0xFF;
    *(p++) = gSysInfoGet.pv3_power_generation_total_high & 0xFF;
    *(p++) = (gSysInfoGet.pv3_power_generation_total_high >> 8) & 0xFF;


    /*  PV4总发电量 */
    *(p++) = gSysInfoGet.pv4_power_generation_total_low & 0xFF;
    *(p++) = (gSysInfoGet.pv4_power_generation_total_low >> 8) & 0xFF;
    *(p++) = gSysInfoGet.pv4_power_generation_total_high & 0xFF;
    *(p++) = (gSysInfoGet.pv4_power_generation_total_high >> 8) & 0xFF;

    for(i = 0; i < 3; i++)
    {
        SpiFlashWriteBytesWithErase(write_addr, _garrWriteData, 32);
        SpiFlashReadBytes(write_addr, _garrReadData, 32);
        if(memcmp(_garrWriteData, _garrReadData, 32) == 0)
        {
            _garrWriteData[0] = garrValidFlag[0];
            _garrWriteData[1] = garrValidFlag[1];
            _garrWriteData[2] = garrValidFlag[2];
            _garrWriteData[3] = garrValidFlag[3];
            _garrWriteData[4] = _gElectricInfoFlashPageIndex & 0xFF;
            _garrWriteData[5] = (_gElectricInfoFlashPageIndex >> 8) & 0xFF;
            _garrWriteData[6] = (_gElectricInfoFlashPageIndex >> 16) & 0xFF;
            _garrWriteData[7] = (_gElectricInfoFlashPageIndex >> 24) & 0xFF;

            data_len = 8;
//            write_addr = write_addr + FLASH_ERASE_SECTOR_SIZE - data_len;
            write_addr = ELECTRIC_INFO_VALID_FLASH_ADDR + (_gElectricInfoFlashPageIndex % BACK_UP_PAGE_NUM) * FLASH_ERASE_SECTOR_SIZE;

            for(j = 0; j < 3; j++)
            {
                SpiFflashWriteBytesWithoutErase(write_addr, _garrWriteData, data_len);
                SpiFlashReadBytes(write_addr, _garrReadData, data_len);
                if(memcmp(_garrWriteData, _garrReadData, data_len) == 0)
                {
                    _gElectricInfoFlashPageIndex++;
                    return true;
                }
            }
        }
    }

    return false;
}

/**
* @brief: ElectricInfoRead
* @param: void                        
* @return: true or false
* @note: 
*
*/
bool ElectricInfoRead(void)
{
    uint16_t i = 0;
    uint32_t arr_addr_index[BACK_UP_PAGE_NUM];

    memset(arr_addr_index, 0, BACK_UP_PAGE_NUM);

    for(i = 0; i < BACK_UP_PAGE_NUM; i++)
    {
        SpiFlashReadBytes((ELECTRIC_INFO_VALID_FLASH_ADDR + i * FLASH_ERASE_SECTOR_SIZE), _garrReadData, 8);
        if(memcmp(_garrReadData, garrValidFlag, 4) == 0)
        {
            arr_addr_index[i] = ((uint32_t)_garrReadData[4] & 0xFF) | (((uint32_t)_garrReadData[5] << 8) & 0xFF00) | \
                                (((uint32_t)_garrReadData[6] << 16) & 0xFF0000) | (((uint32_t)_garrReadData[7] << 24) & 0xFF000000);

            if(arr_addr_index[i] == 0xFFFFFFFF)
            {
                arr_addr_index[i] = 0;
            }
        }
        else
        {
            arr_addr_index[i] = 0;
        }
    }

    for(i = 1; i < BACK_UP_PAGE_NUM; i++)
    {
        if (arr_addr_index[i] > _gElectricInfoFlashPageIndex)
        {
            _gElectricInfoFlashPageIndex = arr_addr_index[i];
        }
    }
    if(_gModHoldRegFlashPageIndex == 0)
    {
        gSysInfoGet.load_electric_cost_total_low = 0;
        gSysInfoGet.load_electric_cost_total_high = 0;
        gSysInfoGet.grid_connect_count_low = 0;
        gSysInfoGet.grid_connect_count_high = 0;
        gSysInfoGet.electric_purchase_total_low = 0;
        gSysInfoGet.electric_purchase_total_high = 0;
        gSysInfoGet.electric_sale_total_low = 0;
        gSysInfoGet.electric_sale_total_high = 0;
        gSysInfoGet.pv1_power_generation_total_low = 0;
        gSysInfoGet.pv1_power_generation_total_high = 0;
        gSysInfoGet.pv2_power_generation_total_low = 0;
        gSysInfoGet.pv2_power_generation_total_high = 0;
        gSysInfoGet.pv3_power_generation_total_low = 0;
        gSysInfoGet.pv3_power_generation_total_high = 0;
        gSysInfoGet.pv4_power_generation_total_low = 0;
        gSysInfoGet.pv4_power_generation_total_high = 0;
    }
    else
    {
        SpiFlashReadBytes((ELECTRIC_INFO_FLASH_ADDR + (_gElectricInfoFlashPageIndex % BACK_UP_PAGE_NUM) * FLASH_ERASE_SECTOR_SIZE), \
                          _garrReadData, 32);
        gSysInfoGet.load_electric_cost_total_low = (_garrReadData[0] & 0xFF) | ((_garrReadData[1] << 8) & 0xFF00);
        gSysInfoGet.load_electric_cost_total_high = (_garrReadData[2] & 0xFF) | ((_garrReadData[3] << 8) & 0xFF00);
        gSysInfoGet.grid_connect_count_low = (_garrReadData[4] & 0xFF) | ((_garrReadData[5] << 8) & 0xFF00);
        gSysInfoGet.grid_connect_count_high = (_garrReadData[6] & 0xFF) | ((_garrReadData[7] << 8) & 0xFF00);
        gSysInfoGet.electric_purchase_total_low = (_garrReadData[8] & 0xFF) | ((_garrReadData[9] << 8) & 0xFF00);
        gSysInfoGet.electric_purchase_total_high = (_garrReadData[10] & 0xFF) | ((_garrReadData[11] << 8) & 0xFF00);
        gSysInfoGet.electric_sale_total_low = (_garrReadData[12] & 0xFF) | ((_garrReadData[13] << 8) & 0xFF00);
        gSysInfoGet.electric_sale_total_high = (_garrReadData[14] & 0xFF) | ((_garrReadData[15] << 8) & 0xFF00);
        gSysInfoGet.pv1_power_generation_total_low = (_garrReadData[16] & 0xFF) | ((_garrReadData[17] << 8) & 0xFF00);
        gSysInfoGet.pv1_power_generation_total_high = (_garrReadData[18] & 0xFF) | ((_garrReadData[19] << 8) & 0xFF00);
        gSysInfoGet.pv2_power_generation_total_low = (_garrReadData[20] & 0xFF) | ((_garrReadData[21] << 8) & 0xFF00);
        gSysInfoGet.pv2_power_generation_total_high = (_garrReadData[22] & 0xFF) | ((_garrReadData[23] << 8) & 0xFF00);
        gSysInfoGet.pv3_power_generation_total_low = (_garrReadData[24] & 0xFF) | ((_garrReadData[25] << 8) & 0xFF00);
        gSysInfoGet.pv3_power_generation_total_high = (_garrReadData[26] & 0xFF) | ((_garrReadData[27] << 8) & 0xFF00);
        gSysInfoGet.pv4_power_generation_total_low = (_garrReadData[28] & 0xFF) | ((_garrReadData[29] << 8) & 0xFF00);
        gSysInfoGet.pv4_power_generation_total_high = (_garrReadData[30] & 0xFF) | ((_garrReadData[31] << 8) & 0xFF00);
    }

    pv1_power_genration_total = (uint32_t)(gSysInfoGet.pv1_power_generation_total_high) << 16 | gSysInfoGet.pv1_power_generation_total_low;
    pv2_power_genration_total = (uint32_t)(gSysInfoGet.pv2_power_generation_total_high) << 16 | gSysInfoGet.pv2_power_generation_total_low;
    purchase_power_genration_total = (uint32_t)(gSysInfoGet.electric_purchase_total_high) << 16 | gSysInfoGet.electric_purchase_total_low;
    sale_power_genration_total =(uint32_t)(gSysInfoGet.electric_sale_total_high) << 16 | gSysInfoGet.electric_sale_total_low;
    load_power_genration_total =(uint32_t)(gSysInfoGet.load_electric_cost_total_high) << 16 | gSysInfoGet.load_electric_cost_total_low;


    _gElectricInfoFlashPageIndex ++;

    return true;
}


/**
* @brief: TimingStorage
* @param: void
* @return:
* @note:
*
*/
void TimingStorage(void)
{
    if((CpuTimer2.InterruptCount % 7200000) == 0)
    {
        ElectricInfoSave();
    }
}

/**
* @brief: DefaultInfoGet
* @param:
* @return:
* @note:
*
*/
void DefaultInfoGet()
{
    memset(&gDefaultSetInfo, 0, sizeof(SysInfoSet_t));
    gDefaultSetInfo.battery_charging_voltage = 57.6;
    gDefaultSetInfo.battery_charging_current = 50;
    gDefaultSetInfo.battery_charging_power = 2500;
    gDefaultSetInfo.battery_discharging_voltage = 43.2;
    gDefaultSetInfo.battery_discharging_current = 50;
    gDefaultSetInfo.battery_discharging_power = 2500;
    gDefaultSetInfo.charging_input_typesetting = 0;
    gDefaultSetInfo.ac_output_voltage_range = 4;
    gDefaultSetInfo.ac_output_frequency_range = 2;
    gDefaultSetInfo.grid_standards = 2;
    gDefaultSetInfo.backup_output_enable = 1;////20251204，可能改成0,20260121,按照客户要求设置默认backup打开
    gDefaultSetInfo.ac_coupling_enable = 1;
    gDefaultSetInfo.network_power_limit_enable = 0;
    gDefaultSetInfo.network_power_limit = 15000;
    gDefaultSetInfo.residual_current_protection_threshold = 150; //20251124，提高默认漏电流阈值到150mA
    gDefaultSetInfo.residual_current_change_response_time = 500; //20251124，漏电流响应时间延长到500ms
    gDefaultSetInfo.off_grid_sensitivity = 3;  // 20260415，新增0ms脱网响应时间等级，默认3
    gDefaultSetInfo.ups_delay0ms = 1; //20251124，增加UPS的0ms切换，20260122，增加0ms功能默认关闭，20260127，增加0ms功能默认打开
    gDefaultSetInfo.antireverseflow_enable = 1;//20251204,按照市场前端要求的默认值改
    gDefaultSetInfo.ongrid_power = 0;//20251204,按照市场前端要求的默认值改
    gDefaultSetInfo.meter_brand = 0;//20251210,增加电表型号默认值设定

    gDefaultSetInfo.pv_ov_protection_point = 550;
    gDefaultSetInfo.pv_uv_protection_point = 100;
    gDefaultSetInfo.grid_voltage_upper_limit = 265;
    gDefaultSetInfo.grid_voltage_lower_limit = 170;
    gDefaultSetInfo.grid_frequency_upper_limit = 55;
    gDefaultSetInfo.grid_frequency_lower_limit = 45;
    gDefaultSetInfo.bypass_voltage_upper_limit = 265;
    gDefaultSetInfo.bypass_voltage_lower_limit = 170;
    gDefaultSetInfo.bypass_frequency_upper_limit = 55;
    gDefaultSetInfo.bypass_frequency_lower_limit = 45;
    gDefaultSetInfo.ov_derating_point = 0;
    gDefaultSetInfo.of_derating_point = 0;
    gDefaultSetInfo.uv_derating_point_charge = 0;
    gDefaultSetInfo.uf_derating_point_charge = 0;
    gDefaultSetInfo.grid_ov_protect_Point = 0;
    gDefaultSetInfo.inverter_uv_protect_Point = 0;
    gDefaultSetInfo.low_frequency_protect_Point = 0;
    gDefaultSetInfo.over_frequency_protect_Point = 0;
    gDefaultSetInfo.reactive_power_response_time = 0;
}

/**
* @brief: UsageInfoGet
* @param: void
* @return:
* @note:
*
*/
void UsageInfoGet()
{
    ModHoldRegRead();
//    ElectricInfoRead();
}

bool FlashIsAllowSave(void)
{
    if(gSysSetInfo.x_boost != ModbusReg.x_boost)
    {
        return true;
    }

    if(gSysSetInfo.ups_delay0ms != ModbusReg.ups_delay0ms)
    {
        return true;
    }

    if(gSysSetInfo.network_power_limit_enable != ModbusReg.network_power_limit_enable)
    {
        return true;
    }

    if(gSysSetInfo.network_power_limit != ModbusReg.network_power_limit)
    {
        return true;
    }

    if(gSysSetInfo.parallel_mode_enable != ModbusReg.parallel_mode_enable)
    {
        return true;
    }

    if(gSysSetInfo.charging_input_typesetting != ModbusReg.charging_input_typesetting)
    {
        return true;
    }

    if(gSysSetInfo.ac_output_voltage_range != ModbusReg.ac_output_voltage_range)
    {
        return true;
    }

    if(gSysSetInfo.ac_output_frequency_range != ModbusReg.ac_output_frequency_range)
    {
        return true;
    }

    if(gSysSetInfo.grid_standards != ModbusReg.grid_standards)
    {
        return true;
    }

    if(gSysSetInfo.grids_tandard_segmentation != ModbusReg.grids_tandard_segmentation)
    {
        return true;
    }

    if(gSysSetInfo.backup_output_enable != ModbusReg.backup_output_enable)
    {
        return true;
    }

    if(gSysSetInfo.antireverseflow_enable != ModbusReg.antireverseflow_enable)
    {
        return true;
    }

    if(gSysSetInfo.program_control_enable != ModbusReg.program_control_enable)
    {
        return true;
    }

    if(gSysSetInfo.island_enable != ModbusReg.island_enable)
    {
        return true;
    }

    if(gSysSetInfo.user_type != ModbusReg.user_type)
    {
        return true;
    }

    if(gSysSetInfo.ct_direction != ModbusReg.ct_direction)
    {
        return true;
    }

    if(gSysSetInfo.ongrid_reconnect_time != ModbusReg.ongrid_reconnect_time)
    {
        return true;
    }

    if(gSysSetInfo.ongrid_power != ModbusReg.ongrid_power)
    {
        return true;
    }

    if(gSysSetInfo.meter_brand != ModbusReg.meter_brand) // 20260302,新增DSP保存电表品牌设置，关机保持上一次的设置，不会刷新为默认值
    {
        return true;
    }

    if(gSysSetInfo.work_mode != ModbusReg.work_mode)
    {
        return true;
    }

    if(gSysSetInfo.pv_max_chg_power != ModbusReg.pv_max_chg_power)
    {
        return true;
    }

    if(gSysSetInfo.max_inv_power != ModbusReg.max_inv_power)
    {
        return true;
    }

    if(gSysSetInfo.hard_limit_threshold != ModbusReg.hard_limit_threshold)
    {
        return true;
    }

    if(gSysSetInfo.residual_current_protection_threshold != ModbusReg.residual_current_protection_threshold)
    {
        return true;
    }

    if(gSysSetInfo.residual_current_change_response_time != ModbusReg.residual_current_change_response_time)
    {
        return true;
    }

    if(gSysSetInfo.off_grid_sensitivity != ModbusReg.off_grid_sensitivity)
    {
        return true;
    }

    return false;
}
/**
* @brief: UsageInfoGet
* @param: void
* @return:
* @note:
*
*/
void AdminInfoGet()
{
    static uint16_t temp_data[0x100];
    static uint16_t save_flag[4] = {0};
    memset((uint16_t*)&gAdminSetInfo, 0, sizeof(SysInfoSet_t));
    SpiFlashReadBytes(ADMIN_INFO_ADDR_FLAG,(uint16_t *)&save_flag, 4);
    if(memcmp((uint16_t *)save_flag, (uint16_t *)garrValidFlag, 4) == 0)
    {
        SpiFlashReadBytes(ADMIN_INFO_ADDR,(uint16_t *)&temp_data, sizeof(SysInfoSet_t) << 1);
        MemcpyU8toU16((uint16_t*)&gAdminSetInfo, temp_data, sizeof(ModbusReg_t));
    }
}
/**
* @brief: AdminInfoSave
* @param: void
* @return:
* @note:
*
*/
void AdminInfoSave()
{
    uint16_t temp_data[0x100];
    MemcpyU16toU8((uint8_t*)temp_data, (uint16_t*)&gAdminSetInfo, sizeof(SysInfoSet_t) << 1);
    SpiFlashWriteBytesWithErase(ADMIN_INFO_ADDR, temp_data,  sizeof(SysInfoSet_t) << 1);
    SpiFlashWriteBytesWithErase(ADMIN_INFO_ADDR_FLAG, garrValidFlag, 4);
}

/**
* @brief: UsageInfoSave
* @param: void
* @return:
* @note:
*
*/
uint32_t gStorageCount1, gStorageCount2;
void UsageInfoSave()
{
    if(ModbusReg.user_type == 2 |ModbusReg.user_type == 3)//厂家（一级用户）
    {
        if(admin_flag == 0)
        {
            AdminInfoGet();
            memcpy((uint16_t*)&ModbusReg, (uint16_t *)&gAdminSetInfo, sizeof(SysInfoSet_t));
            admin_flag = 1;
        }
        else
        {
            if(memcmp((uint16_t *)&gSysSetInfo,(uint16_t *)&ModbusReg,sizeof(ModbusReg_t)) != 0)
            {
//                gStorageCount1++;
                if(FlashIsAllowSave() == true)
                {
//                    gStorageCount2++;
                    ModHoldRegSave();
                    memcpy((uint16_t*)&gAdminSetInfo, (uint16_t *)&ModbusReg, sizeof(ModbusReg_t));
                    AdminInfoSave();
                }
            }
        }
    }
    else if(ModbusReg.user_type == 0 |ModbusReg.user_type == 1)//二级用户
    {
        if(ModbusReg.user_type == 0)
        {
            user_flag = 0;
        }
        if(ModbusReg.user_type == 1)
        {
            user_flag = 1;
        }
        if(admin_flag == 1)
        {
            if(cnt_flag == 1)
            {
                _gModHoldRegFlashPageIndex = _gModHoldRegFlashPageIndex - 1;
                cnt_flag = 0;
            }
            ModHoldRegRead();
            if(user_flag == 0)
            {
                ModbusReg.user_type = 0;
            }
            if(user_flag == 1)
            {
                ModbusReg.user_type = 1;
            }
        }
        else
        {
            if(memcmp((uint16_t *)&gSysSetInfo,(uint16_t *)&ModbusReg,sizeof(ModbusReg_t)) != 0)
            {
//                gStorageCount1++;
                if(FlashIsAllowSave() == true)
                {
//                    gStorageCount2++;
                    ModHoldRegSave();
                    memcpy((uint16_t*)&gAdminSetInfo, (uint16_t *)&ModbusReg, sizeof(ModbusReg_t));
                    //AdminInfoSave();
                }
            }
        }
        admin_flag = 0;
    }
    memcpy((uint16_t*)&gSysSetInfo, (uint16_t*)&ModbusReg, sizeof(ModbusReg_t));
}
/**
* @brief: ResetFactory
* @param: void
* @return:
* @note:
*
*/
void ResetFactory()
{
    if(gSysSetInfo.reset_factory == 0x5AA5)
    {
        if(gSysSetInfo.user_type == 2 |gSysSetInfo.user_type == 3)//厂家（一级用户）
        {
            memcpy((uint16_t*)&gAdminSetInfo, (uint16_t*)&gDefaultSetInfo, sizeof(SysInfoSet_t));
            memcpy((uint16_t*)&ModbusReg, (uint16_t*)&gDefaultSetInfo, sizeof(SysInfoSet_t));
            if(gSysSetInfo.user_type == 2)
            {
                gAdminSetInfo.user_type = 2;
                ModbusReg.user_type = 2;
            }
            if(gSysSetInfo.user_type == 3)
            {
                gAdminSetInfo.user_type = 3;
                ModbusReg.user_type = 3;
            }
            AdminInfoSave();      
        }
        if(gSysSetInfo.user_type == 0 |gSysSetInfo.user_type == 1)//二级用户
        {
            memcpy((uint16_t*)&ModbusReg, (uint16_t*)&gAdminSetInfo, sizeof(SysInfoSet_t));
            if(gSysSetInfo.user_type == 0)
            {
                ModbusReg.user_type = 0;
            }
            if(gSysSetInfo.user_type == 1)
            {
                ModbusReg.user_type = 1;
            }        
        }
        ModbusReg.reset_factory = 0;
        ModbusReg.network_power_limit_enable = 0;
        ModbusReg.network_power_limit = 15000;
        ModbusReg.residual_current_protection_threshold = 150; //20251124，提高默认漏电流阈值到150mA
        ModbusReg.residual_current_change_response_time = 500;
        ModbusReg.grid_standards = 2; //并网标准默认澳大利亚A
        ModbusReg.power_factor_setting_value = 100; //功率因数默认100% = 1.00

        ModbusReg.antireverseflow_enable = 1;
        ModbusReg.ongrid_power = 0;
        ModbusReg.meter_brand = 0;//20251210，增加电表型号默认值设定
        ModbusReg.backup_output_enable = 1;//20260121,按照客户要求设置默认backup打开
        ModbusReg.off_grid_sensitivity = 3;  // 20260415，新增0ms脱网响应时间等级，默认3
        ModbusReg.ups_delay0ms = 1; //20251124，增加UPS的0ms切换，20260122，增加0ms功能默认关闭，20260127，增加0ms功能默认打开

        memcpy((uint16_t*)&gSysSetInfo, (uint16_t*)&ModbusReg, sizeof(ModbusReg_t));
        ModHoldRegSave();    
    }

    if(gSysSetInfo.firmware_erasure == 0xAE86)
    {
        gSysSetInfo.firmware_erasure = 0;
        SpiFlashUnlock();
        SpiFlashAutoEraseSector(PCS_FIRMWARE_ADDR, 0x1000);
        SpiFlashLock();
    }
}
/**
* @brief: ElectricInfoGet
* @param: void
* @return:
* @note:
*
*/

void ElectricInfoGet(void)
{
    if(gPvPowerHoursComputedFlag == 1)//1min上报更新一次数据
    {
        static uint32_t pv1_power_hours_total = 0;
        static uint32_t pv2_power_hours_total = 0;
        static uint32_t purchase_power_hours_total = 0;
        static uint32_t sale_power_hours_total = 0;
        static uint32_t load_power_hours_total = 0;
        static uint32_t Norload_power_hours_total = 0;
        static uint32_t bat_chg_power_hours_total = 0;
        static uint32_t bat_dsg_power_hours_total = 0;


        static uint32_t pv1_power_hours_mod = 0;
        static uint32_t pv2_power_hours_mod = 0;
        static uint32_t purchase_power_hours_mod = 0;
        static uint32_t sale_power_hours_mod = 0;
        static uint32_t load_power_hours_mod = 0;
        static uint32_t norload_power_hours_mod = 0;
        static uint32_t bat_chg_power_hours_mod = 0;
        static uint32_t bat_dsg_power_hours_mod = 0;



         /*1h电量+上一小时不满1h电量*/
        pv1_power_hours_total = gPv1PowerTotalHours / 36000 + pv1_power_hours_mod;
        pv2_power_hours_total = gPv2PowerTotalHours / 36000 + pv2_power_hours_mod;
        purchase_power_hours_total = gPurchasePowerTotalHours / 36000 + purchase_power_hours_mod;
        sale_power_hours_total = gSalePowerTotalHours /36000 + sale_power_hours_mod;
        load_power_hours_total = gLoadPowerTotalHours / 36000 + load_power_hours_mod;
        Norload_power_hours_total = gNorLoadPowerTotalHours / 36000 + norload_power_hours_mod;

        bat_chg_power_hours_total = gBatChgPowerTotalHours / 36000 + bat_chg_power_hours_mod;
        bat_dsg_power_hours_total = gBatDsgPowerTotalHours / 36000 + bat_dsg_power_hours_mod;
        /*取整*/
        pv1_power_genration_total +=  pv1_power_hours_total / 100;//0.1KWH
        pv2_power_genration_total += pv2_power_hours_total / 100;
        purchase_power_genration_total += purchase_power_hours_total / 100;
        sale_power_genration_total += sale_power_hours_total / 100;
        load_power_genration_total += load_power_hours_total / 100;
        norload_power_genration_total += Norload_power_hours_total / 100;

        bat_chg_power_genration_total += bat_chg_power_hours_total / 100;
        bat_dsg_power_genration_total += bat_dsg_power_hours_total / 100;
        /*取模*/
        pv1_power_hours_mod = pv1_power_hours_total % 100;
        pv2_power_hours_mod = pv2_power_hours_total % 100;
        purchase_power_hours_mod = purchase_power_hours_total % 100;
        sale_power_hours_mod = sale_power_hours_total % 100;
        load_power_hours_mod = load_power_hours_total % 100;
        norload_power_hours_mod = Norload_power_hours_total % 100;
        bat_chg_power_hours_mod = bat_chg_power_hours_total % 100;
        bat_dsg_power_hours_mod = bat_dsg_power_hours_total % 100;

        gPvPowerHoursComputedFlag = 0; //外部读取后清0，等待下一次置位累加
    }
    if((gSysSetInfo.clean_current_cap == 1) || (gParallelComm.mode == 2))   //监控下发清除或设备为从机模式不进行电量统计
    {
        pv1_power_genration_total = 0;
        pv2_power_genration_total = 0;
        purchase_power_genration_total = 0;
        sale_power_genration_total = 0;
        load_power_genration_total = 0;
        norload_power_genration_total = 0;
        bat_chg_power_genration_total = 0;
        bat_dsg_power_genration_total = 0;
        gSysSetInfo.clean_current_cap = 0;
        ModbusReg.clean_current_cap = 0;
    }
    gSysInfoGet.pv1_power_generation_total_high = pv1_power_genration_total >> 16;
    gSysInfoGet.pv1_power_generation_total_low = pv1_power_genration_total;
    gSysInfoGet.pv2_power_generation_total_high = pv2_power_genration_total >> 16;
    gSysInfoGet.pv2_power_generation_total_low = pv2_power_genration_total;
    gSysInfoGet.electric_purchase_total_high = purchase_power_genration_total >> 16;
    gSysInfoGet.electric_purchase_total_low = purchase_power_genration_total;
    gSysInfoGet.electric_sale_total_high = sale_power_genration_total >> 16;
    gSysInfoGet.electric_sale_total_low = sale_power_genration_total;
    gSysInfoGet.load_electric_cost_total_high = load_power_genration_total >> 16;
    gSysInfoGet.load_electric_cost_total_low = load_power_genration_total;
    gSysInfoGet.remote_load_power_total_high = norload_power_genration_total >> 16;
    gSysInfoGet.remote_load_power_total_low =  norload_power_genration_total;
    gSysInfoGet.battery_charge_total_high = bat_chg_power_genration_total >> 16;
    gSysInfoGet.battery_charge_total_low =  bat_chg_power_genration_total;
    gSysInfoGet.battery_discharge_total_high = bat_dsg_power_genration_total >> 16;
    gSysInfoGet.battery_discharge_total_low = bat_dsg_power_genration_total;
}
//123
static void NormalLoadcal()
{
    if(gSysSetInfo.ct_or_meter == 1)
    {
        gSysInfoGet.r_phase_load_activepower   = (int16_t)(gSysSetInfo.meter_active_power - _gState_algo.grid_active_power) ;
        gSysInfoGet.r_phase_load_reactivepower = (int16_t)(gSysSetInfo.meter_reactive_power - _gState_algo.grid_reactive_power) ;
        gSysInfoGet.r_phase_load_apparentpower = (int16_t)(gSysSetInfo.meter_apparent_power - _gState_algo.grid_pwr) ;
    }
    else
    {
//        gSysInfoGet.r_phase_load_activepower   = (int16_t)( ct_dir_fix * _gState_algo.CT_active_power - _gState_algo.grid_active_power) ;
//        gSysInfoGet.r_phase_load_reactivepower = (int16_t)( _gState_algo.CT_reactive_power - _gState_algo.grid_reactive_power) ;
//        gSysInfoGet.r_phase_load_apparentpower = (int16_t)( ct_dir_fix * _gState_algo.CT_pwr - _gState_algo.grid_pwr) ;

        if(InvState == 2) //EN_GRID_CONNECT_MODE
        {
            if(flag_CT_exist == 1)
            {
                gSysInfoGet.r_phase_load_activepower   = (int16_t)( ct_dir_fix * _gState_algo.CT_active_power - _gState_algo.grid_active_power) ;
                gSysInfoGet.r_phase_load_reactivepower = (int16_t)( _gState_algo.CT_reactive_power - _gState_algo.grid_reactive_power) ;
                gSysInfoGet.r_phase_load_apparentpower = (int16_t)( ct_dir_fix * _gState_algo.CT_pwr - _gState_algo.grid_pwr) ;
            }
            else
            {
                gSysInfoGet.r_phase_load_activepower   = 0;
                gSysInfoGet.r_phase_load_reactivepower = 0;
                gSysInfoGet.r_phase_load_apparentpower = 0;
            }
            if(flag_NetDisableTest_Enable == 1)
            {
                gSysInfoGet.r_phase_load_activepower   = 0;
                gSysInfoGet.r_phase_load_reactivepower = 0;
                gSysInfoGet.r_phase_load_apparentpower = 0;
            }
//            if(flag_CertificationTest == 1)
//            {
//                gSysInfoGet.r_phase_load_activepower   = 0;
//                gSysInfoGet.r_phase_load_reactivepower = 0;
//                gSysInfoGet.r_phase_load_apparentpower = 0;
//            }
        }
        else
        {
            gSysInfoGet.r_phase_load_activepower   = 0;
            gSysInfoGet.r_phase_load_reactivepower = 0;
            gSysInfoGet.r_phase_load_apparentpower = 0;
        }

//        if(flag_CT_exist == 1)
//        {
//            gSysInfoGet.r_phase_load_activepower   = (int16_t)( ct_dir_fix * _gState_algo.CT_active_power - _gState_algo.grid_active_power) ;
//            gSysInfoGet.r_phase_load_reactivepower = (int16_t)( _gState_algo.CT_reactive_power - _gState_algo.grid_reactive_power) ;
//            gSysInfoGet.r_phase_load_apparentpower = (int16_t)( ct_dir_fix * _gState_algo.CT_pwr - _gState_algo.grid_pwr) ;
//        }
//        else
//        {
//            gSysInfoGet.r_phase_load_activepower   = 0;
//            gSysInfoGet.r_phase_load_reactivepower = 0;
//            gSysInfoGet.r_phase_load_apparentpower = 0;
////            gSysInfoGet.r_phase_load_activepower   = (int16_t)( ct_dir_fix * (-1 * _gState_algo.inv_active_power) - _gState_algo.grid_active_power) ;
////            gSysInfoGet.r_phase_load_reactivepower = (int16_t)( _gState_algo.inv_reactive_power - _gState_algo.grid_reactive_power) ;
////            gSysInfoGet.r_phase_load_apparentpower = (int16_t)( ct_dir_fix * _gState_algo.inv_pwr - _gState_algo.grid_pwr) ;
//        }
    }

    if(Enoil)
    {
        gSysInfoGet.r_phase_load_activepower   = 0;
        gSysInfoGet.r_phase_load_reactivepower = 0;
        gSysInfoGet.r_phase_load_apparentpower = 0;
    }

    gAntiReFlowValue = -1 * (float)(gSysInfoGet.r_phase_load_activepower); // 20260519，补充备注：设备实时远端功率（设备AC口与CT口之间的所有功率之和，包含远端负载和第三方光伏逆变器等），正数为产生多余功率，负数为消耗设备功率，例：-4000表示远端总消耗的功率为4000W

}


/**
* @brief: SysInfoGet
* @param: void
* @return:
* @note:
*
*/
int16_t gird_activepower = 0;
void SysInfoGet(void)
{
    static uint16_t grid_power_filtering_positive = 0;
    static uint16_t grid_power_filtering_negative = 0;
    static float ct_cur_with_dir_ave = 0;
    static float grid_cur_with_dir_ave = 0;
    static float backup_active_pwr_ave = 0;
    static float backup_apparent_pwr_ave = 0;
    static float backup_reactive_pwr_ave = 0;
    int16_t grid_dir = GridDirGet();
    int16_t ct_dir = CtDirGet();
    if(gSysSetInfo.ct_direction == 0)
    {
        ct_dir_fix = 1;
    }
    else if(gSysSetInfo.ct_direction == 1)
    {
        ct_dir_fix = -1;
    }

//    uint16_t grid_apparentpower = 0;
//    int16_t gird_activepower = 0;20251229
//    uint16_t gird_reactivepower = 0;
    uint16_t warning_code = 0;
    ElectricInfoGet();
    NormalLoadcal(); // 远端负载的计算
//    _gAlgoInfo = AlgostateGet();
    _gInvSta = InverterGetState();
    gSysInfoGet.inverter_model = 0x01;//5Kw
    gSysInfoGet.reserved2[0] = garrTemperInitialData[7];//功率底板ID号，设定为0.20625V，对应4096范围上位机读取到230~280之间为可接受数值
    /*系统输入状态*/
    if(_gInvSta->sta_grid_connect == 1)
    {
        gSysInfoGet.sys_input_stat |= (1 << 0);
    }
    else
    {
        gSysInfoGet.sys_input_stat &= ~(1 << 0);
    }
    if(gstPv1Data.mppt_state == 1)
    {
        gSysInfoGet.sys_input_stat |= (1 << 2);
        gSysInfoGet.mppt_status = EN_MPPT_RUN;
    }
    else
    {
        gSysInfoGet.sys_input_stat &= ~(1 << 2);
    }
    if(gstPv2Data.mppt_state == 1)
    {
        gSysInfoGet.sys_input_stat |= (1 << 3);
        gSysInfoGet.mppt_status = EN_MPPT_RUN;
    }
    else
    {
        gSysInfoGet.sys_input_stat &= ~(1 << 3);
    }
    if(gstPv1Data.mppt_state == 0 && gstPv2Data.mppt_state == 0)
    {
        gSysInfoGet.mppt_status = EN_MPPT_STANDBY;
    }
    else
    {
        gSysInfoGet.mppt_status = EN_MPPT_RUN;
    }
    /*系统状态*/

    /*并离网状态*/
    if(GetInvStandbyState() == true)
    {
        gSysInfoGet.inv_status = EN_INV_STANDBY;
    }
    else if(_gInvSta->sta_grid_connect == 0)
    {
        gSysInfoGet.inv_status = EN_INPUT_OFF_GRID;
        gSysInfoGet.system_status = EN_INPUT_OFF_BRID;
    }
    else if(_gInvSta->sta_grid_connect == 1)
    {
        gSysInfoGet.inv_status = EN_INPUT_OFF_TO_ON_GRID;
    }
    else if(_gInvSta->sta_grid_connect == 2)
    {
        gSysInfoGet.inv_status = EN_INPUT_ON_GRID;
        if(gstPv1Data.mppt_state == 1|gstPv2Data.mppt_state == 1)
        {
            gSysInfoGet.system_status = EN_INPUT_MIX_BRID;
        }
    }
    /*DCDC状态*/
    if(gIsLlcStartupOk == 0)
    {
        gSysInfoGet.dcdc_status = EN_DCDC_STANDBY;
    }
    else
    {
//        if(gBat_Current_Ave < -4)
//        {
//            gSysInfoGet.dcdc_status = EN_DCDC_CHG;
//        }
//        else if(gBat_Current_Ave > 4)
//        {
//            gSysInfoGet.dcdc_status = EN_DCDC_DSG;
//        }
//        else
//        {
//            gSysInfoGet.dcdc_status = EN_DCDC_STANDBY;
//        }
        if(gBat_Current_Ave < 0)
        {
            gSysInfoGet.dcdc_status = EN_DCDC_CHG;
        }
        else if(gBat_Current_Ave > 0)
        {
            gSysInfoGet.dcdc_status = EN_DCDC_DSG;
        }
        else
        {
            gSysInfoGet.dcdc_status = EN_DCDC_STANDBY;
        }
    }
    /*MPPT状态*/
    gSysInfoGet.dsp_wake_up_over = ActivationRunStateRead();
    /*DSP alarm*/
//    warning_code = (((gAlgoWarningDef.byte[1]<<8)|gAlgoWarningDef.byte[0])& 0x7f)|(gstFaultInfo.worning_code.value);
    gSysInfoGet.dsp_alarm_code_high = 0;
    gSysInfoGet.dsp_alarm_code_low = (gAlgoWarningDef.byte[0] & 0x7f)|gstFaultInfo.worning_code.value;
    /*DSP error*/
    gSysInfoGet.dsp_error_code_high = gAlgoErrDef.byte[1]|gComFail;
    gSysInfoGet.dsp_error_code_low = gAlgoErrDef.byte[0];

    gSysInfoGet.bus_voltage = gBus_Voltage_Ave * 10;
    /*散热器温度*/
    gSysInfoGet.inverter_radiator_temperature = gstSensorData.temp[4];//除腔体温度外其余各部件中最大温度
    /*机箱温度*/
    gSysInfoGet.chassis_internal_temperature = gstSensorData.temp[5];

    gSysInfoGet.r_phase_backup_voltage = _gInvSta->sta_load_vol_rms * 10;
    gSysInfoGet.r_phase_backup_current = (int)_gInvSta->sta_load_cur_rms * 10;
    backup_apparent_pwr_ave =COEFF_AVE * backup_apparent_pwr_ave + (1 - COEFF_AVE) * _gState_algo.load_pwr;
    backup_active_pwr_ave =COEFF_AVE * backup_active_pwr_ave - (1 - COEFF_AVE) * _gState_algo.load_active_power;
    backup_reactive_pwr_ave =COEFF_AVE * backup_reactive_pwr_ave - (1 - COEFF_AVE) * _gState_algo.load_reactive_power;

    gSysInfoGet.r_phase_backup_apparentpower = (int16_t)backup_apparent_pwr_ave;//backup和远端负载的区别？
    gSysInfoGet.r_phase_backup_activepower = (int16_t)backup_active_pwr_ave;
    gSysInfoGet.r_phase_backup_reactivepower = (int16_t)backup_reactive_pwr_ave;

    gSysInfoGet.battery_voltage = gBat_Voltage_Ave * 10;
    gSysInfoGet.battery_current = (int16_t)(gBat_Current_Ave * 10);
    gSysInfoGet.battery_power = (int16_t)(gBat_Voltage_Ave * gBat_Current_Ave);

    gSysInfoGet.r_phase_grid_voltage = _gState_algo.grid_vol_rms * 10;
    gSysInfoGet.r_phase_grid_frequency = _gState_algo.grid_freq *100;
    // ct_cur_with_dir_ave = COEFF_AVE * ct_cur_with_dir_ave + (1 - COEFF_AVE) * ct_dir * _gState_algo.ct_cur_rms_ave;//r相电表电流==相电网电流？;
    ct_cur_with_dir_ave = COEFF_AVE * ct_cur_with_dir_ave + (1 - COEFF_AVE) * _gState_algo.CT_active_power / (_gState_algo.grid_vol_rms + 0.001f);//r相电表电流==相电网电流？;
    gSysInfoGet.r_phase_meter_current = (int16_t)(ct_dir_fix * ct_cur_with_dir_ave * 10.0f);

    if(InvState == 2) //EN_GRID_CONNECT_MODE
    {
        if(flag_CT_exist == 1)
        {
            gird_activepower = (int16_t)_gState_algo.CT_active_power * ct_dir_fix;
        }
        else
        {
            gird_activepower = (int16_t)(-1 * _gState_algo.inv_active_power) * ct_dir_fix + gSysInfoGet.r_phase_backup_activepower;
            if(gird_activepower >= 0)
            {
                gSysInfoGet.r_phase_meter_current = 1;
            }
            else
            {
                gSysInfoGet.r_phase_meter_current = -1;
            }
        }
    }
    else
    {
        gird_activepower = 0; // 20260317，逆变器离网状态AC耦合有bug，待修复
    }

//    gird_activepower = (int16_t)_gState_algo.CT_active_power * ct_dir_fix;

    if(gird_activepower >= 0)
    {
        grid_power_filtering_positive ++;
        grid_power_filtering_negative = 0;
    }
    else
    {
        grid_power_filtering_negative ++;
        grid_power_filtering_positive = 0;
    }
    if((grid_power_filtering_positive > 10)||(grid_power_filtering_negative > 10))
    {
        gSysInfoGet.r_phase_meter_activepower = gird_activepower;
        if((grid_power_filtering_positive > 60000)||(grid_power_filtering_negative > 60000))
        {
            grid_power_filtering_positive = 10;
//            grid_power_filtering_positive = 10;
            grid_power_filtering_negative = 10;
        }
    }
    else
    {
        gSysInfoGet.r_phase_meter_activepower = 0;
    }

    if(flag_CT_exist == 1)//CT存在且并网
    {
        gSysInfoGet.r_phase_meter_apparentpower = _gState_algo.CT_pwr;//瞬时值滤波
        gSysInfoGet.r_phase_meter_reactivepower = (int16_t)_gState_algo.CT_reactive_power;
    }
    else if(flag_CT_exist == 0 && geInvsta == EN_GRID_CONNECT_MODE)//20260129修改，CT不存在且并网
    {
        gSysInfoGet.r_phase_meter_apparentpower = _gState_algo.inv_pwr;//瞬时值滤波
        gSysInfoGet.r_phase_meter_reactivepower = (int16_t)_gState_algo.inv_reactive_power;
    }
    else//20260129修改，离网
    {
        gSysInfoGet.r_phase_meter_apparentpower = 0;
        gSysInfoGet.r_phase_meter_reactivepower = 0;
    }

//    gSysInfoGet.r_phase_meter_apparentpower = _gState_algo.CT_pwr;//瞬时值滤波
////    gSysInfoGet.r_phase_meter_activepower = (int16_t)_gState_algo.grid_active_power;
//    gSysInfoGet.r_phase_meter_reactivepower = (int16_t)_gState_algo.CT_reactive_power;

//    gSysInfoGet.phase_sequence =

    if(GetGenToGridFinish() == 1)//已切换至市电模式(市电模式)油机模式,油机的显示值均归零
    {
        gSysInfoGet.oil_current = 0;
        gSysInfoGet.oil_voltage = 0;
        gSysInfoGet.oil_power = 0;
        gSysInfoGet.oil_frequency = 0;
    }
    else
    {
        gSysInfoGet.dsp_error_code_high &= 0xFF7F;  //屏蔽bit23，发电机模式下不上报逆变过压
        gSysInfoGet.dsp_error_code_low &= 0xBFFF;   //屏蔽bit14，发电机模式下不上报硬件过流
        gSysInfoGet.oil_current = _gState_algo.grid_cur_rms * 10;
        gSysInfoGet.oil_voltage = _gState_algo.grid_vol_rms * 10;
        gSysInfoGet.oil_power = _gState_algo.grid_active_power;//油机显示有功功率
        gSysInfoGet.oil_frequency = _gState_algo.grid_freq *100;
        gSysInfoGet.r_phase_grid_voltage = 0;
        gSysInfoGet.r_phase_grid_frequency = 0;
        gSysInfoGet.r_phase_meter_current = 0;
        gSysInfoGet.r_phase_meter_apparentpower = 0;
        gSysInfoGet.r_phase_meter_reactivepower = 0;
        gSysInfoGet.r_phase_meter_activepower = 0;
    }

    // grid_cur_with_dir_ave = COEFF_AVE * grid_cur_with_dir_ave + (1 - COEFF_AVE) * grid_dir * _gState_algo.grid_cur_rms_ave;//r相电表电流==相电网电流？;
    grid_cur_with_dir_ave = COEFF_AVE * grid_cur_with_dir_ave + (1 - COEFF_AVE) * _gState_algo.grid_cur_rms;    //ac口电流
    gSysInfoGet.r_phase_inverter_input_current = (int16_t)(grid_dir * grid_cur_with_dir_ave * 10.0f);
    gSysInfoGet.r_phase_ac_apparentpower = (int16_t)_gState_algo.grid_pwr;
    gSysInfoGet.r_phase_ac_activepower = (int16_t)_gState_algo.grid_active_power;
    gSysInfoGet.r_phase_ac_reactivepower = (int16_t)_gState_algo.grid_reactive_power;

    gSysInfoGet.r_phase_inverter_voltage = _gInvSta->sta_inv_vol_rms * 10;//r相逆变电压（单相）
    gSysInfoGet.r_phase_inverter_current = (int16_t)(_gInvSta->sta_inv_cur_rms * 10);//r相逆变电流（单相）
    gSysInfoGet.r_phase_inverter_frequency = _gInvSta->sta_inv_freq * 100;
    gSysInfoGet.r_phase_inverter_apparentpower = (int16_t)(_gInvSta->sta_inv_vol_rms * _gInvSta->sta_inv_cur_rms);
    gSysInfoGet.r_phase_inverter_activepower = (int16_t)_gState_algo.inv_active_power;
    gSysInfoGet.r_phase_inverter_reactivepower = (int16_t)_gState_algo.inv_reactive_power;


//    gSysInfoGet.r_phase_load_apparentpower = (int16_t)_gState_algo.norload_pwr;
//    gSysInfoGet.r_phase_load_activepower = (int16_t)_gState_algo.norload_active_power;
//    gSysInfoGet.r_phase_load_reactivepower = (int16_t)_gState_algo.norload_reactive_power;

    float32 pv1_pwr = gPv1_Voltage_Ave * gPv1_Current_Ave;
    float32 pv2_pwr = gPv2_Voltage_Ave * gPv2_Current_Ave;
//    if(pv1_pwr < 50)
//    {
//        pv1_pwr = 0;
//    }
//    if(pv2_pwr < 50)
//    {
//        pv2_pwr = 0;
//    }


    gSysInfoGet.pv1_input_voltage = gPv1_Voltage_Ave * 10;
    gSysInfoGet.pv1_input_current = gPv1_Current_Ave * 10;
    gSysInfoGet.pv1_input_power   = pv1_pwr;
    gSysInfoGet.pv2_input_voltage = gPv2_Voltage_Ave * 10;
    gSysInfoGet.pv2_input_current = gPv2_Current_Ave * 10;
    gSysInfoGet.pv2_input_power   = pv2_pwr;

//    if(gParallelComm.mode == 1 && geInvsta == EN_GRID_CONNECT_MODE)     //主机模式电量统计
//    {
//        gSysInfoGet.r_phase_backup_apparentpower += gParallelComm.slave_info.slave_backup_power;//backup和远端负载的区别？
//        gSysInfoGet.r_phase_backup_activepower += gParallelComm.slave_info.slave_backup_power;
//        gSysInfoGet.battery_power += gParallelComm.slave_info.slave_bat_power;
//        gSysInfoGet.pv1_input_power += gParallelComm.slave_info.slave_pv_power;
//    }

    gSysInfoGet.grid_connect_count_high =  gOnGridCnt >> 16;
    gSysInfoGet.grid_connect_count_low = gOnGridCnt;
    gSysInfoGet.on_grid_time = gOnGridTime;
//    if(geInvsta == EN_OFF_GRID_MODE)//解决离网模式下，无PV情况下电池功率与负载功率不同问题。
//    {
//        if(gSysInfoGet.pv1_input_power == 0 && gSysInfoGet.pv2_input_power == 0 && gSysInfoGet.r_phase_meter_activepower == 0)
//        {
//            gSysInfoGet.battery_power = gSysInfoGet.r_phase_backup_activepower;
//        }
//    }
//    if(gSysInfoGet.dcdc_status == EN_DCDC_STANDBY)//与监控卡电池状态（不充不放）状态对应。
//    {
//        gSysInfoGet.battery_power = 0;
//    }
//    if(fabs(gSysInfoGet.r_phase_meter_current) < 10)//与监控卡电网状态 静止状态对应
//    {
//        gSysInfoGet.r_phase_meter_activepower = 0;
//    }


#if TEST_MODE
    /*DSP alarm*/
    gSysInfoGet.dsp_alarm_code_high = 0;
    gSysInfoGet.dsp_alarm_code_low = 1;
    /*DSP error*/
    gSysInfoGet.dsp_error_code_high = 2;
    gSysInfoGet.dsp_error_code_low = 3;
    gSysInfoGet.bus_voltage = 4;
    gSysInfoGet.inverter_radiator_temperature = 5;
    /*机箱温度*/
    gSysInfoGet.chassis_internal_temperature = 6;
    gSysInfoGet.battery_voltage = 7;
    gSysInfoGet.battery_current = 8;
    gSysInfoGet.battery_power = 9;
//    gSysInfoGet.battery_active_power = ;
//    gSysInfoGet.battery_reactive_power = ;
    gSysInfoGet.r_phase_grid_voltage = 10;
//    gSysInfoGet.r_phase_grid_frequency = ;
    gSysInfoGet.r_phase_meter_current = 11;//r相电表电流==相电网电流？
    gSysInfoGet.r_phase_meter_apparentpower = 12;//瞬时值滤波
//    gSysInfoGet.r_phase_meter_activepower =
//    gSysInfoGet.r_phase_meter_reactivepower =
//    gSysInfoGet.phase_sequence =

    gSysInfoGet.oil_current = 13;//需要滤波？
    gSysInfoGet.oil_voltage = 14;//需滤波？
    gSysInfoGet.oil_power = 15;
//    gSysInfoGet.oil_frequency =
    gSysInfoGet.r_phase_inverter_voltage = 16;//r相逆变电压（单相）
    gSysInfoGet.r_phase_inverter_current = 17;//r相逆变电流（单相）
    gSysInfoGet.r_phase_inverter_frequency = 18;
    gSysInfoGet.r_phase_inverter_apparentpower = 19;
    gSysInfoGet.r_phase_inverter_activepower = 20;
    gSysInfoGet.r_phase_inverter_reactivepower = 21;
    gSysInfoGet.r_phase_backup_voltage = 22;
    gSysInfoGet.r_phase_backup_current = 23;
    gSysInfoGet.r_phase_backup_apparentpower = 24;//backup和远端负载的区别？
//    gSysInfoGet.r_phase_backup_activepower =
//    gSysInfoGet.r_phase_backup_reactivepower =
//    gSysInfoGet.r_phase_load_apparentpower =
//    gSysInfoGet.r_phase_load_activepower =
//    gSysInfoGet.r_phase_load_reactivepower =
    gSysInfoGet.pv1_input_voltage = 25;
    gSysInfoGet.pv1_input_current = 26;
    gSysInfoGet.pv1_input_power = 27;
    gSysInfoGet.pv2_input_voltage = 28;
    gSysInfoGet.pv2_input_current = 29;
    gSysInfoGet.pv2_input_power = 30;
//
////    gSysInfoGet.load_electric_cost_total_high =
////    gSysInfoGet.load_electric_cost_total_low =
    gSysInfoGet.grid_connect_count_high =  31;
    gSysInfoGet.grid_connect_count_low = 32;
//    gSysInfoGet.electric_purchase_total_high =
//    gSysInfoGet.electric_purchase_total_low =
//    gSysInfoGet.electric_sale_total_high =
//    gSysInfoGet.electric_sale_total_low =
//    gSysInfoGet.pv1_power_generation_total_high =
//    gSysInfoGet.pv1_power_generation_total_low =
//    gSysInfoGet.pv2_power_generation_total_high =
//    gSysInfoGet.pv2_power_generation_total_low =
#endif
}


