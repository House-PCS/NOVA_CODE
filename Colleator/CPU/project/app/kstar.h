#ifndef _KSTAR_H
#define _KSTAR_H

#include <stdint.h>

#pragma pack(1)

typedef struct _dev_kstar_register_info_t
{
	uint16_t reg_start;				// 寄存器起始地址
	uint16_t reg_size;				// 寄存器数量
	uint8_t data[0];				// 寄存器实际数据
} DevKstarRegisterInfo_t;

/**
 * @brief kstar 系统信息
 */
typedef struct _dev_kstar_sys_info_t
{
	char machine_id[16];		// 机器型号(字符串)
	char battery_name[16];		// 电池名称
	uint8_t arm_ver;			// ARM版本号
	uint8_t arm_test_ver;		// ARM测试版本号
	uint8_t dsp_ver;			// DSP版本号
	uint8_t dsp_test_ver;		// DSP测试版本号
	uint8_t dsp2_ver;			// DSP2版本号
	uint8_t dsp2_test_ver;		// DSP2测试版本号
} DevKstarSysInfo_t;

/**
 * @brief kstar 遥测操作
 */
typedef struct _dev_kstar_telemetry_t
{
	uint16_t reg_addr;		// 寄存器地址
	uint16_t reg_val;		// 寄存器值
} DevKstarTelemetry_t;

typedef struct _dev_kstar_set_sys_t
{
	uint16_t reg_addr;		// 寄存器地址(逆变器设置类型)
	uint16_t reg_size;		// 寄存器数量
	uint8_t data[0];		// 寄存器数据
} DevKstarSetSys_t;

typedef struct _dev_kstar_opt_reg_t
{
	uint16_t reg_addr;		// 寄存器地址
	uint16_t reg_val;		// 寄存器值
} DevKstarOptReg_t;

#pragma pack()

#endif /*  */
