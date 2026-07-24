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

#include "ddr_process.h"
#define BITCHECK(data,number)           ((data >> number) & (0x01))
uint8_t gBMSFirmwareType[4] = {0x0C, 0x0C, 0x0C, 0x03};
uint8_t gFPGAFirmwareType[4] = {0x0C, 0x0C, 0x0C, 0x04};
void DDRFirmwareAnalysis()
{
    if((gDDRFirmwareInfo.ddr_bms_check_sum_com == gDDRFirmwareInfo.ddr_bms_check_sum_rece) && (gDDRFirmwareInfo.ddr_bms_check_sum_com != 0) && (gDDRFirmwareInfo.ddr_bms_check_sum_rece != 0) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 12) == 0))
    {
        if(memcmp(bms_ddr_temp, gBMSFirmwareType, 4) != 0)
        {
            gBMSFirmwareUpgrateInfo.upgrate_fail_flag = 1;//ddr校验失败
            return;
        }
        else
        {
            gDDRFirmwareInfo.ddr_bms_firmware_success = 1;
        }
    }
    else if((gDDRFirmwareInfo.ddr_bms_check_sum_com != gDDRFirmwareInfo.ddr_bms_check_sum_rece) && ((BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 12) == 1)))
    {
        gDDRFirmwareInfo.ddr_bms_firmware_success = 0;
        gBMSFirmwareUpgrateInfo.upgrate_fail_flag = 1;//ddr校验失败
    }
    
    if((gDDRFirmwareInfo.ddr_fpga_check_sum_com == gDDRFirmwareInfo.ddr_fpga_check_sum_rece) && (gDDRFirmwareInfo.ddr_fpga_check_sum_com != 0) && (gDDRFirmwareInfo.ddr_fpga_check_sum_rece != 0) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 12) == 0))
    {
        if(memcmp(fpga_ddr_temp, gFPGAFirmwareType, 4) != 0)
        {
            gFPGAFirmwareUpgrateInfo.upgrate_fail_flag = 1;//ddr校验失败
            return;
        }
        else
        {
            gDDRFirmwareInfo.ddr_fpga_firmware_success = 1;
        }
    }
    else if((gDDRFirmwareInfo.ddr_fpga_check_sum_com != gDDRFirmwareInfo.ddr_fpga_check_sum_rece) && ((BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 12) == 1)))
    {
        gDDRFirmwareInfo.ddr_fpga_firmware_success = 0;
        gFPGAFirmwareUpgrateInfo.upgrate_fail_flag = 1;//ddr校验失败
    }    
}








