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

#include "dev_ddr.h"
uint8_t arr_buff[1050];
#define     BMS_DDR_ADDRESS 0x00007070
uint8_t bms_ddr_temp[256] __attribute__((at(BMS_DDR_ADDRESS)));
#define     FPGA_DDR_ADDRESS 0x00007170
uint8_t fpga_ddr_temp[256] __attribute__((at(FPGA_DDR_ADDRESS)));

DDR_Firmware_Info_t gDDRFirmwareInfo;
uint32_t bms_start_addr = (BMS_DDR_FIRMWARE_ADDR - 0x20000000) / (sizeof(CMSDK_DDR_TypeDef));
uint32_t fpga_start_addr = (FPGA_DDR_FIRMWARE_ADDR - 0x20000000) / (sizeof(CMSDK_DDR_TypeDef));

void WriteDDRSend(uint32_t start_addr, uint8_t *p, uint32_t len)
{
    start_addr = (start_addr - 0x20000000) / (sizeof(CMSDK_DDR_TypeDef));
    if(len % 4 != 0)
    {
        memcpy((uint8_t *)arr_buff, (uint8_t *)p, ((len / 4 ) + 1) * 4);
    }
    else
    {
        memcpy((uint8_t *)arr_buff, (uint8_t *)p, len);
    }

    if(len % 128 != 0)
    {
        memcpy(((uint8_t *) & DDR[start_addr + 0]), &arr_buff[0], ((len / 128 ) + 1) * 128);
    }
    else
    {
        memcpy(((uint8_t *) & DDR[start_addr + 0]), &arr_buff[0], len);//放到对应的固件DDR地址
    }
    if((start_addr >= fpga_start_addr) && (start_addr < bms_start_addr))//FPGA固件包
    {
        if(start_addr != fpga_start_addr)//0x00440000  非FPGA固件的首包地址
        {
            gDDRFirmwareInfo.ddr_fpga_check_sum_com += CheckSumFun(arr_buff, len, 1);
        }
        if(start_addr == fpga_start_addr)//0x00440000 FPGA的固件包
        {
            gDDRFirmwareInfo.ddr_fpga_check_sum_com = 0;
			gDDRFirmwareInfo.ddr_fpga_check_sum_com += CheckSumFun(&arr_buff[256], len - 256, 1);
            gDDRFirmwareInfo.ddr_fpga_check_sum_rece = 0;
            gDDRFirmwareInfo.ddr_fpga_pack_count = 0;
            gDDRFirmwareInfo.ddr_fpga_len = 0;
            memcpy(((uint8_t *) & (fpga_ddr_temp)), (uint8_t *)FPGA_DDR_FIRMWARE_ADDR, 256);
            gDDRFirmwareInfo.ddr_fpga_check_sum_rece = (fpga_ddr_temp[252] | (fpga_ddr_temp[253] << 8) | (fpga_ddr_temp[254] << 16) | (fpga_ddr_temp[255] << 24));//按位检查 /* 固件校验和 数据域校验和 */        
            gDDRFirmwareInfo.ddr_fpga_len = (fpga_ddr_temp[7] | (fpga_ddr_temp[8] << 8) | (fpga_ddr_temp[9] << 16) | (fpga_ddr_temp[10] << 24));///* 固件长度 */
        }
        gDDRFirmwareInfo.ddr_fpga_pack_count ++;
    }
    else if((start_addr >= bms_start_addr) && (start_addr < 0x00450000))//BMS固件包
    {
        if(start_addr != bms_start_addr)//0x00440000  非BMS固件的首包地址
        {
            gDDRFirmwareInfo.ddr_bms_check_sum_com += CheckSumFun(arr_buff, len, 1);
        }
        if(start_addr == bms_start_addr)//0x00440000 BMS的固件包
        {
            gDDRFirmwareInfo.ddr_bms_check_sum_com = 0;
            gDDRFirmwareInfo.ddr_bms_check_sum_rece = 0;
            gDDRFirmwareInfo.ddr_bms_pack_count = 0;
            gDDRFirmwareInfo.ddr_bms_len = 0;
            memcpy(((uint8_t *) & (bms_ddr_temp)), (uint8_t *)BMS_DDR_FIRMWARE_ADDR, 256);
            gDDRFirmwareInfo.ddr_bms_check_sum_rece = (bms_ddr_temp[252] | (bms_ddr_temp[253] << 8) | (bms_ddr_temp[254] << 16) | (bms_ddr_temp[255] << 24));//按位检查 /* 固件校验和 数据域校验和 */            
            gDDRFirmwareInfo.ddr_bms_len = (bms_ddr_temp[7] | (bms_ddr_temp[8] << 8) | (bms_ddr_temp[9] << 16) | (bms_ddr_temp[10] << 24));///* 固件长度 */
        }
        gDDRFirmwareInfo.ddr_bms_pack_count ++;
    }
}




