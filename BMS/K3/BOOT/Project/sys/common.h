
#ifndef __COMMON_H_
#define __COMMON_H_


#include "stdint.h"
#include <string.h>

#pragma pack(1)
typedef struct _device_info_stru_t
{
    uint16_t            model_id;
    char                sn[21];
    char                sw_vesion[20];
}DeviceInfo_t;
#pragma pack()

#pragma pack(1)
typedef struct _device_para_stru_t
{
    uint16_t            current_capacity;
    uint16_t            current_temperature;
    uint32_t            expected_filling_duration;
    uint32_t            available_duration;
    uint8_t             add_package_num;
    uint32_t            ac_input_power;
    uint32_t            pv_input_power;
    uint32_t            cc_input_power;
    uint32_t            total_output_power;
    uint32_t            total_exectricity_consumption;
    uint8_t             charging_type;
    uint8_t             fun_status;
    uint8_t             whether_parallel_operation;
}DevicePara_t;
#pragma pack()

#define INTERFACE_OUTPUT_TYPE_NUM       6
#define INTERFACE_OUTPUT_TYPE_USBA      1
#define INTERFACE_OUTPUT_TYPE_TYPEC     2
#define INTERFACE_OUTPUT_TYPE_AC        3
#define INTERFACE_OUTPUT_TYPE_ADS       4
#define INTERFACE_OUTPUT_TYPE_CIGAR     5
#define INTERFACE_OUTPUT_TYPE_5521      6

#define INTERFACE_USBA_NUM              4
#define INTERFACE_TYPEC_NUM             2
#define INTERFACE_AC_NUM                4
#define INTERFACE_ADS_NUM               1
#define INTERFACE_CIGAR_NUM             1
#define INTERFACE_5521_NUM              1


typedef struct _device_interface_stru_t
{
    uint32_t            power_usba[INTERFACE_USBA_NUM];
    uint32_t            power_typec[INTERFACE_TYPEC_NUM];
    uint32_t            power_ac[INTERFACE_AC_NUM];
    uint32_t            power_ads[INTERFACE_ADS_NUM];
    uint32_t            power_cigar[INTERFACE_CIGAR_NUM];
    uint32_t            power_5521[INTERFACE_5521_NUM];
}DeviceInterface_t;

#pragma pack(1)
typedef struct _device_calibration_stru_t
{
    uint8_t             time_zone;
    uint32_t            time_stamp;
}DeviceCalibration_t;

#pragma pack()


#define DEVICE_MAX_ADD_PACKAGE_NUM 2
#define DEVICE_CELL_NUM     16

#pragma pack(1)
typedef struct _device_cell_group_status_stru_t
{
    uint16_t            soc;
    uint16_t            sop;
    uint16_t            soh;
    uint16_t            current_max_capacity;
    uint16_t            cell_group_loop_num;
    uint16_t            cell_group_temperature;
    uint32_t            total_voltage;
    uint8_t             cell_num;
    uint16_t            max_cell_voltage;
    uint16_t            min_cell_voltage;
    uint16_t            average_cell_voltage;
    uint16_t            cell_voltage[DEVICE_CELL_NUM];
}DeviceCellGroupStatus_t;

#pragma pack()

typedef struct _device_ctl_stru_t
{
    uint8_t             input_power_percent;
    uint32_t            device_screen_rest_time;
    uint8_t             interface_switch;
    uint8_t             charge_mode;
    uint8_t             charge_max_capacity;
}DeviceCtl_t;


typedef struct _device_base_info_stru_t
{
    char                name[32];
}DeviceBaseInfo_t;




extern DeviceInterface_t           gDeviceInterface;
extern DeviceInfo_t                gDeviceInfo;
extern DevicePara_t                gDevicePara;
extern DeviceCalibration_t         gDeviceCalibration;
extern DeviceCellGroupStatus_t     gDeviceCellGroupStatus[DEVICE_MAX_ADD_PACKAGE_NUM + 1];
extern DeviceCtl_t                 gDeviceCtl;
extern DeviceBaseInfo_t            gDeviceBaseInfo;


#endif

















