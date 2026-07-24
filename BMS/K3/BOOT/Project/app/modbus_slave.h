/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_slave.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-12-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-12-16    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __MODBUS_SLAVE_H_
#define __MODBUS_SLAVE_H_
#include "bsp.h"
#include "uart.h"
#include "queue.h"
#include "bms_protocol.h"
#include "afe_control.h"
//#include "sox.h"

#define SADDRMASTER	            3
#define _MODBUS_RX_SIZE		    256
#define MODBUS_REV_QUEUE_DEPTH	4

/* RTU 应答代码 */
#define RSP_OK				0		/* 成功 */
#define RSP_ERR_CMD			0x01	/* 不支持的功能码 */
#define RSP_ERR_REG_ADDR	0x02	/* 寄存器地址错误 */
#define RSP_ERR_VALUE		0x03	/* 数据值域错误 */
#define RSP_ERR_WRITE		0x04	/* 写入失败 */

/* MODBUS 寄存器地址 */
#define MODBUS_HARDWARE_VERSION                 2000
#define MODBUS_SOFTWARE_VERSION                 2001
#define MODBUS_MODEL_ID                         2002
#define MODBUS_SN_19_18                         2003
#define MODBUS_SN_17_16                         2004
#define MODBUS_SN_15_14                         2005
#define MODBUS_SN_13_12                         2006
#define MODBUS_SN_11_10                         2007
#define MODBUS_SN_09_08                         2008
#define MODBUS_SN_07_06                         2009
#define MODBUS_SN_05_04                         2010
#define MODBUS_SN_03_02                         2011
#define MODBUS_SN_01_00                         2012
        
#define MODBUS_CELL_00_VDC                      2020
#define MODBUS_CELL_01_VDC                      2021
#define MODBUS_CELL_02_VDC                      2022
#define MODBUS_CELL_03_VDC                      2023
#define MODBUS_CELL_04_VDC                      2024
#define MODBUS_CELL_05_VDC                      2025
#define MODBUS_CELL_06_VDC                      2026
#define MODBUS_CELL_07_VDC                      2027
#define MODBUS_CELL_08_VDC                      2028
#define MODBUS_CELL_09_VDC                      2029
#define MODBUS_CELL_10_VDC                      2030
#define MODBUS_CELL_11_VDC                      2031
#define MODBUS_CELL_12_VDC                      2032
#define MODBUS_CELL_13_VDC                      2033
#define MODBUS_CELL_14_VDC                      2034
#define MODBUS_CELL_15_VDC                      2035
#define MODBUS_CELL_16_VDC                      2036
#define MODBUS_CELL_17_VDC                      2037
#define MODBUS_CELL_18_VDC                      2038
#define MODBUS_CELL_19_VDC                      2039
#define MODBUS_CELL_MAX_VOLTAGE                 2040
#define MODBUS_CELL_MIN_VOLTAGE                 2041
#define MODBUS_CELL_AVERAGE_VOLTAGE             2042
#define MODBUS_CELL_ALL_VDC_HIGH                2043
#define MODBUS_CELL_ALL_VDC_LOW                 2044
#define MODBUS_CELL_CURRENT_HIGH                2045
#define MODBUS_CELL_CURRENT_LOW                 2046
#define MODBUS_CURRENT_POWER                    2047
        
#define MODBUS_SOC_REAL                         2060
#define MODBUS_SOC_ACT                          2061
#define MODBUS_SOC_RPT                          2062
#define MODBUS_CAPACITY_STD_HIGH                2063
#define MODBUS_CAPACITY_STD_LOW                 2064
#define MODBUS_CURRENT_MAX_CAPA_HIGH            2065
#define MODBUS_CURRENT_MAX_CAPA_LOW             2066
#define MODBUS_PACK_HEALTH                      2067
#define MODBUS_PACK_CYCLES                      2068
#define MODBUS_CHARGING_CAPACITY_HIGH           2069
#define MODBUS_CHARGING_CAPACITY_LOW            2070
#define MODBUS_DISCHARGE_CAPACITY_HIGH          2071
#define MODBUS_DISCHARGE_CAPACITY_LOW           2072
#define MODBUS_CURRENT_MAX_CHG_CURRENT_HIGH     2073
#define MODBUS_CURRENT_MAX_CHG_CURRENT_LOW      2074
#define MODBUS_CURRENT_MAX_DSG_CURRENT_10_HIGH  2075
#define MODBUS_CURRENT_MAX_DSG_CURRENT_10_LOW   2076
#define MODBUS_CURRENT_MAX_DSG_CURRENT_30_HIGH  2077
#define MODBUS_CURRENT_MAX_DSG_CURRENT_30_LOW   2078
#define MODBUS_CURRENT_MAX_DSG_CURRENT_60_HIGH  2079
#define MODBUS_CURRENT_MAX_DSG_CURRENT_60_LOW   2080
#define MODBUS_PACK_SLEEP_TIME_HIGH             2081
#define MODBUS_PACK_SLEEP_TIME_LOW              2082
#define MODBUS_CHARGING_CAPACITY2_HIGH          2083
#define MODBUS_CHARGING_CAPACITY2_LOW           2084
#define MODBUS_DISCHARGE_CAPACITY2_HIGH         2085
#define MODBUS_DISCHARGE_CAPACITY2_LOW          2086
#define MODBUS_PACK_STATUS                      2087
#define MODBUS_RECHARGE_TIME_HIGH               2088
#define MODBUS_RECHARGE_TIME_LOW                2089

#define	MODBUS_CELL_TEMP_00                     2100
#define	MODBUS_CELL_TEMP_01                     2101
#define	MODBUS_CELL_TEMP_02                     2102
#define	MODBUS_CELL_TEMP_03                     2103
#define	MODBUS_CELL_TEMP_04                     2104
#define	MODBUS_CELL_TEMP_05                     2105
#define	MODBUS_CELL_TEMP_06                     2106
#define	MODBUS_CELL_TEMP_07                     2107
#define	MODBUS_CELL_TEMP_08                     2108
#define MODBUS_CELL_TEMP_09                     2109
#define	MODBUS_CELL_TEMP_10                     2110
#define	MODBUS_CELL_TEMP_11                     2111
#define	MODBUS_CELL_TEMP_12                     2112
#define	MODBUS_CELL_TEMP_13                     2113
#define	MODBUS_CELL_TEMP_14                     2114
#define	MODBUS_CELL_TEMP_15                     2115
#define	MODBUS_CELL_TEMP_16                     2116
#define	MODBUS_CELL_TEMP_17                     2117
#define	MODBUS_CELL_TEMP_18                     2118
#define MODBUS_CELL_TEMP_19                     2119
#define MODBUS_CELL_MAX_TEMP                    2120
#define MODBUS_CELL_MIN_TEMP                    2121
#define MODBUS_CELL_AVE_TEMP                    2122
#define MODBUS_BALANCE_TEMP                     2123
#define MODBUS_DSG_MOS_TEMP                     2124
#define MODBUS_CHG_MOS_TEMP                     2125
#define MODBUS_ADD_MOS_TEMP1                    2126
#define MODBUS_ADD_MOS_TEMP2                    2127
#define MODBUS_BALANCE_STATUS                   2128

#define MODBUS_DSG_MOS_CTL                      2140
#define MODBUS_CHG_MOS_CTL                      2141
#define MODBUS_PRE_MOS_CTL                      2142
#define MODBUS_ADD_MOS_CTL1                     2143
#define MODBUS_ADD_MOS_CTL2                     2144
#define MODBUS_BMS_SHUTDOWN                     2145

#define MODBUS_CELL_UV_LVL1                     2150
#define MODBUS_CELL_UV_LVL2                     2151
#define MODBUS_CELL_UV_LVL3                     2152
#define MODBUS_CELL_OV_LVL1                     2153
#define MODBUS_CELL_OV_LVL2                     2154
#define MODBUS_CELL_OV_LVL3                     2155
#define MODBUS_PACK_OV_LVL_HIGH                 2156
#define MODBUS_PACK_OV_LVL_LOW                  2157
#define MODBUS_PACK_UV_LVL_HIGH                 2158
#define MODBUS_PACK_UV_LVL_LOW                  2159
#define MODBUS_PACK_OT_LVL1                     2160
#define MODBUS_PACK_OT_LVL2                     2161
#define MODBUS_PACK_OT_LVL3                     2162
#define MODBUS_PACK_UT_LVL                      2163

#define MODBUS_MOS_ERROR_CODE                   2180
#define MODBUS_CURR_ERROR_CODE                  2181
#define MODBUS_CELL1_ERROR_CODE                 2182
#define MODBUS_CELL2_ERROR_CODE                 2183
#define MODBUS_PREC_ERROR_CODE                  2184
#define MODBUS_CELLSAMPLE_ERROR_CODE            2185
#define MODBUS_OTHER1_ERROR_CODE                2186

typedef struct _modbus_reg_stru_t
{
    uint16_t    dsg_mos_ctl;          
    uint16_t	chg_mos_ctl; 	
    uint16_t    pre_mos_ctl;     
    uint16_t    add_mos_ctl1;    
    uint16_t    add_mos_ctl2;    
    uint16_t    bms_shutdown;    
    uint16_t    cell_uv_lvl1;    
    uint16_t    cell_uv_lvl2;    
    uint16_t    cell_uv_lvl3;    
    uint16_t    cell_ov_lvl1;    
    uint16_t    cell_ov_lvl2;    
    uint16_t    cell_ov_lvl3;    
    uint32_t    pack_ov_lvl;     
    uint32_t    pack_uv_lvl;     
    uint16_t    pack_ot_lvl1;    
    uint16_t    pack_ot_lvl2;    
    uint16_t    pack_ot_lvl3;    
    uint16_t    pack_ut_lvl;     
                   
}ModbusReg_t;

typedef struct _modbus_rev_data_stru_t
{
    uint16_t                     data_size;
	uint8_t				        *pData;
}ModbusRevData_t; 


typedef struct
{
	uint8_t RxBuf[_MODBUS_RX_SIZE];
	uint8_t RxCount;
	uint8_t RxStatus;
	uint8_t RxNewFlag;

	uint8_t RspCode;

	uint8_t TxBuf[_MODBUS_RX_SIZE];
	uint8_t TxCount;
}MODS_T;

extern ModbusReg_t gMODBUS_Reg;

bool ModSInit(void);
void ModSProcess(void);
#endif