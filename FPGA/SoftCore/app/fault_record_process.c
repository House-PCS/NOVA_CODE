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
  * <tr><td>2024-2-4    <td>V1.0.0    <td>lixudong    <td>InitialVersion
  * <tr><td>2024-2-4    <td>V1.0.1    <td>lixudong    <td>modbus_reg LSB Save
  * </table>
  ******************************************************************************
  */

#include "fault_record_process.h"
#define FAULT_RECORD_DATA_SIZE   1024//数据大小
#define FPGA_FAULTRECORD_ADDR    0x004C2000

#define DDR_RING_START    0x21000000    // DDR起始
#define DDR_RING_END      0x2108FFFF    // DDR结束
#define DDR_RING_SIZE     (DDR_RING_END - DDR_RING_START + 1)  // 总长度

uint8_t fpga_faultrecord_data_save[1024] = {0};

static EN_FAULT_ANALYSIS_STATUS fpga_fault_status = EN_FAULT_ANALYSIS_STATUS_IDLE;//初始置为空闲
uint16_t fault_record_index = 0;
FPGAFaultInfo_t   gFPGAFaultInfo;//固件升级结构体
void FPGARecordPorcess()
{
	if(gTempSlaveDCDCPara.dc_dc_only_read_para.recorden == 0)
	{
		gstDCDCReg->dc_dc_hold_para.recorden_status = 0;
	}
		
    if((gTempSlaveDCDCPara.dc_dc_only_read_para.recorden == 1) && (gstDCDCReg->dc_dc_hold_para.recorden_status == 0))
    {
        FPGAFaultAnalysis();
    }
}

void FPGAFaultAnalysis()
{
    static uint16_t size_per;
    static uint32_t base_faultrecord_addr ;
    static uint32_t fpga_faultstatus_timeout;
    static uint32_t faultrecord_flash_addr ;
    static uint32_t read_start_addr;  // 真正的读取起点（边界保护后）
    static uint32_t read_stop_addr;   // 停止地址 = 起点前一个地址
    switch(fpga_fault_status)
    {
        case  EN_FAULT_ANALYSIS_STATUS_IDLE:
        {
            //从DDR中读取数据
            fault_record_index = 0;
            gFPGAFaultInfo.fault_record_finsh_flag = 0;
            faultrecord_flash_addr = FPGA_FAULTRECORD_ADDR;
            base_faultrecord_addr = 0x20000000 + gTempSlaveDCDCPara.dc_dc_only_read_para.record_addr_low + (gTempSlaveDCDCPara.dc_dc_only_read_para.record_addr_high << 16);
            // ===================== 【边界保护：确保在环形区内】 =====================
            if (base_faultrecord_addr < DDR_RING_START || base_faultrecord_addr > DDR_RING_END)
            {
                base_faultrecord_addr = DDR_RING_START;
            }
            // 保存起点 + 计算停止地址（卷回终点）
            read_start_addr = base_faultrecord_addr;
            read_stop_addr  = read_start_addr - 1;
            // 处理 0x1000000 前一个地址变成 0x108FFFF
            if (read_stop_addr < DDR_RING_START)
            {
                read_stop_addr = DDR_RING_END;
            }
            size_per = 0;
            gFPGAFaultInfo.fault_record_fail_flag = 0;
            gFPGAFaultInfo.fpga_len = (144 * 4096);///* 固件长度 */
            gFPGAFaultInfo.fpga_seek = 0;
            for(uint8_t i = 0; i < (gFPGAFaultInfo.fpga_len / (1024 * 64) + 1); i++)
            {
                SpiFlashEraseBlock64K(FPGA_FAULTRECORD_ADDR + i * 64 * 1024);
            }
            fpga_faultstatus_timeout = TimGetSystime();
            fpga_fault_status = EN_FAULT_ANALYSIS_STATUS_START;
            break;
        }

        case EN_FAULT_ANALYSIS_STATUS_START:
        {
            if(TimDifftimeFromNow(fpga_faultstatus_timeout) < 5)
            {
                break;
            }
            uint32_t bytes_to_end;
            // 计算当前位置到结束点的剩余长度
            if(base_faultrecord_addr <= read_stop_addr)
            {
                bytes_to_end = read_stop_addr - base_faultrecord_addr + 1;
            }
            else
            {
                bytes_to_end = DDR_RING_END - base_faultrecord_addr + 1;

            }
            // ===================== 关键：每次最多读1024 =====================
            if(bytes_to_end >= 1024)
            {
                size_per = 1024;
            }
            else
            {
                size_per = bytes_to_end;

            }
            if(size_per == 0)
            {
                fpga_fault_status = EN_FAULT_ANALYSIS_STATUS_END;
                break;
            }
            // 从DDR读取数据（最多1024字节）
            memcpy(fpga_faultrecord_data_save, (uint8_t*)base_faultrecord_addr, size_per);

            fpga_faultstatus_timeout = TimGetSystime();
            fpga_fault_status = EN_FAULT_ANALYSIS_STATUS_BUSY;
            break;
        }

        case EN_FAULT_ANALYSIS_STATUS_BUSY:
        {
            if(TimDifftimeFromNow(fpga_faultstatus_timeout) < 5)
            {
                break;
            }
            // 写入FLASH
            SpiFlashWriteBytesWithErase(faultrecord_flash_addr, (uint8_t *)fpga_faultrecord_data_save, size_per);
            // ===================== 【地址步进 + 自动环形卷回】 =====================
            base_faultrecord_addr += size_per;
            faultrecord_flash_addr += size_per;
            gFPGAFaultInfo.fpga_seek += size_per;
            fault_record_index++;

            // 超出DDR_END → 卷回DDR_START
            if (base_faultrecord_addr > DDR_RING_END)
            {
                base_faultrecord_addr = DDR_RING_START;
            }
            // ===================== 【读完停止条件】 =====================
            if (base_faultrecord_addr == read_start_addr)
            {
                fpga_fault_status = EN_FAULT_ANALYSIS_STATUS_END;
            }
            else
            {
                fpga_fault_status = EN_FAULT_ANALYSIS_STATUS_START;
                fpga_faultstatus_timeout = TimGetSystime();
            }
            break;
        }
        case EN_FAULT_ANALYSIS_STATUS_END:
        {
            fpga_fault_status = EN_FAULT_ANALYSIS_STATUS_IDLE;
            gFPGAFaultInfo.fault_record_finsh_flag = 1;
			gstDCDCReg->dc_dc_hold_para.recorden_status = 1;
            break;
        }
        default:
            break;
    }
}

