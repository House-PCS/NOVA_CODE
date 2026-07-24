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

#include "firmware_upgrate.h"
#define UPGRATE_DATA_SIZE       1024//数据大小
#define DDR_UPGRATE_DATA_SIZE   1024//数据大小
#define GRATE_DATA_SIZE   1024//数据大小
#define UPGRATE_TIME_OUT        3000
#define BITCHECK(data,number)           ((data >> number) & (0x01))
#define     ADDRESS 0x00005380
uint8_t fpga_firmware_data_save[1024] __attribute__((at(ADDRESS)));
#define     ADDRESS1 0x00005780
uint8_t fpga_firmware_data_load[1024] __attribute__((at(ADDRESS1)));

#define     ADDRESS2 0x00006000
static uint8_t upgrate_data_copy[1024] __attribute__((at(ADDRESS2)));
uint8_t rest[5] = {1};
uint32_t check_sum_sum_sum = 0;
static uint8_t  upgrate_data[1050];
uint16_t pack_index_count = 0;
uint16_t pack_count = 0;
uint8_t space_num = 0;
uint16_t error = 0;
uint8_t softcorerestflag = 0;
FPGAFirmwareUpgrateInfo_t   gFPGAFirmwareUpgrateInfo;//固件升级结构体
BMSFirmwareUpgrateInfo_t    gBMSFirmwareUpgrateInfo;//固件升级结构体

static void SoftReboot()
{
    *((uint32_t *)0x40000030) = 1;
}

void BMSUPGradeSendStart()
{
    uint16_t check_sum;
    upgrate_data[0] = 0x5A;
    upgrate_data[1] = 0x00;
    upgrate_data[2] = 0x03;
    upgrate_data[3] = BMS_CMD_UPGRATE_Start & 0xFF;
    upgrate_data[4] = (BMS_CMD_UPGRATE_Start >> 8) & 0xFF;
    upgrate_data[5] = 0x05;//数据域长度
    upgrate_data[6] = 0x00;

    upgrate_data[7] = gBMSFirmwareUpgrateInfo.bms_len & 0xFF;//文件总长度
    upgrate_data[8] = (gBMSFirmwareUpgrateInfo.bms_len >> 8) & 0xFF;
    upgrate_data[9] = (gBMSFirmwareUpgrateInfo.bms_len >> 16) & 0xFF;
    upgrate_data[10] = (gBMSFirmwareUpgrateInfo.bms_len >> 24) & 0xFF;

    upgrate_data[11] = 0x02;//space num
    space_num = upgrate_data[11];
    check_sum = CheckSum(upgrate_data, 12);
    upgrate_data[12] = check_sum & 0xFF;
    upgrate_data[13] = (check_sum >> 8) & 0xFF;
    Uart0Send(upgrate_data, 14);
}

void BMSUPGradeSendBusy(uint16_t size, uint32_t offset, uint16_t index)
{
    uint32_t check_sum;
    upgrate_data[0] = 0x5A;
    upgrate_data[1] = 0x00;
    upgrate_data[2] = 0x03;
    upgrate_data[3] = BMS_CMD_UPGRATE_BUSY & 0xFF;
    upgrate_data[4] = (BMS_CMD_UPGRATE_BUSY >> 8) & 0xFF;
    upgrate_data[5] = (size + space_num) & 0xFF;//数据域长度
    upgrate_data[6] = ((size + space_num) >> 8) & 0xFF;
    upgrate_data[7] = index & 0xFF;//包的序号
    upgrate_data[8] = (index >> 8) & 0xFF;
    memcpy(((uint8_t *)&upgrate_data_copy[0]), (uint8_t *)BMS_DDR_FIRMWARE_ADDR + offset, 1024);//cpy取最大值，实际发出按真实计算值。
    memcpy(((uint8_t *)&upgrate_data[9]), (uint8_t *)upgrate_data_copy, size);
    check_sum = CheckSumFun(upgrate_data, size + 9, 1);
    if(index == 0)
    {
        check_sum_sum_sum += CheckSumFun(&upgrate_data[9 + 256], size - 256, 1);
    }
    if(index != 0)
    {
        check_sum_sum_sum += CheckSumFun(&upgrate_data[9], size, 1);
    }
    upgrate_data[9 + size] = check_sum & 0xFF;
    upgrate_data[10 + size] = (check_sum >> 8) & 0xFF;
    Uart0Send(upgrate_data, size + 11);
}

void BMSUPGradeSendEnd()
{
    uint16_t check_sum;
    upgrate_data[0] = 0x5A;
    upgrate_data[1] = 0x00;
    upgrate_data[2] = 0x03;
    upgrate_data[3] = BMS_CMD_UPGRATE_END & 0xFF;
    upgrate_data[4] = (BMS_CMD_UPGRATE_END >> 8) & 0xFF;
    upgrate_data[5] = 0x00;
    upgrate_data[6] = 0x00;
    check_sum = CheckSum(upgrate_data, 7);
    upgrate_data[7] = check_sum & 0xFF;
    upgrate_data[8] = (check_sum >> 8) & 0xFF;
    Uart0Send(upgrate_data, 9);
}

void UpdataStatusAnalysis()
{

    static uint32_t softcoreresttime;
    static uint8_t  softcorerestcount;    
    DDRFirmwareAnalysis();//DDR数据校验
    Commovertime(); //监控卡与软核通讯超时，需要对BMS进行关机操作，通讯超时2min，关机。
    if((gTempSlaveDCDCPara.dc_dc_hold_para.ip_address_low == 0x010B) && (gTempSlaveDCDCPara.dc_dc_hold_para.ip_address_high == 0xC0A8) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 4) == 1))
    {
        //复位 第一个地址包
        BMSWriteDataSeed(0x300100EA, 2, &rest[0]);
        softcorerestflag = 1;
    }
    else if(((gTempSlaveDCDCPara.dc_dc_hold_para.ip_address_low != 0x010B) && (gTempSlaveDCDCPara.dc_dc_hold_para.ip_address_high == 0xC0A8)) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 1) == 1))
    {
        //复位  非第一个地址包
        BMSWriteDataSeed(0x300100EA, 2, &rest[0]);
        softcorerestflag = 1;
    }
    if((softcorerestflag) && (TimDifftimeFromNow(softcoreresttime) > 1000))
    {
        softcoreresttime =  TimGetSystime();
        softcorerestcount++;        
    }
    if((softcorerestflag) && (softcorerestcount > 10))
    {      
		softcorerestflag = 0;
        SoftReboot();
    }    
    
    if((BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 0) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 10) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 12) == 1) && (gDDRFirmwareInfo.ddr_fpga_firmware_success == 1))
    {
        if(gFPGAFirmwareUpgrateInfo.upgrate_finsh_flag == 0)
        {
            FPGAFirmwareAnalysis();//含有FPGA/或fpga和bms固件包 开始FPGA升级
        }
    }
    if((BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 0) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 8) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 10) == 0) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 12) == 1) && (gDDRFirmwareInfo.ddr_bms_firmware_success == 1))
    {
        //只含有BMS固件包
        gBMSFirmwareUpgrateInfo.bms_upgrate_en = 1;
        if(gBMSFirmwareUpgrateInfo.upgrate_finsh_flag == 0)
        {
            BMSUpgrateFirmwareProcess();//开始BMS升级
        }
    }
    if((BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 0) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 8) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 10) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 12) == 1) && (gDDRFirmwareInfo.ddr_bms_firmware_success == 1))
    {
        if(gBMSFirmwareUpgrateInfo.upgrate_finsh_flag == 0)
        {
            BMSUpgrateFirmwareProcess();//开始BMS升级
        }
    }
    FirmwareUpgrate();
}


static EN_FIRMWARE_ANALYSIS_STATUS fpga_firmware_status = EN_FIRMWARE_ANALYSIS_STATUS_IDLE;//初始置为空闲
void FPGAFirmwareAnalysis()
{
    static uint16_t size_per_transfer;
    static uint32_t fpga_firmware_checksum;
    static uint32_t base_ddr_addr;
    static uint32_t base_firmware_addr ;
    static uint32_t fpga_status_timeout;
    switch(fpga_firmware_status)
    {
        case  EN_FIRMWARE_ANALYSIS_STATUS_IDLE:
        {
            //从DDR中读取数据
            pack_index_count = 0;
            gFPGAFirmwareUpgrateInfo.upgrate_finsh_flag = 0;
            error = 0;
            base_firmware_addr = FPGA_FIRMWARE_ADDR;
            base_ddr_addr = FPGA_DDR_FIRMWARE_ADDR + 256;
            fpga_firmware_checksum = 0;
            size_per_transfer = 0;
            memcpy(((uint8_t *) (fpga_firmware_data_save)), (uint8_t *)FPGA_DDR_FIRMWARE_ADDR, 256);
            gFPGAFirmwareUpgrateInfo.fpga_offset = 0;
            gFPGAFirmwareUpgrateInfo.upgrate_success_flag = 0;
            gFPGAFirmwareUpgrateInfo.upgrate_fail_flag = 0;
            gFPGAFirmwareUpgrateInfo.fpga_len = (fpga_firmware_data_save[7] | (fpga_firmware_data_save[8] << 8) | (fpga_firmware_data_save[9] << 16) | (fpga_firmware_data_save[10] << 24)) - 256;///* 固件长度 */
            gFPGAFirmwareUpgrateInfo.fpga_checksum = (fpga_firmware_data_save[252] | (fpga_firmware_data_save[253] << 8) | (fpga_firmware_data_save[254] << 16) | (fpga_firmware_data_save[255] << 24));//按位检查 /* 固件校验和 数据域校验和 */
            gFPGAFirmwareUpgrateInfo.fpga_seek = 0;
            for(uint8_t i = 0; i < (gFPGAFirmwareUpgrateInfo.fpga_len / (1024 * 64) + 1); i++)
            {
                SpiFlashEraseBlock64K(0x00261000 + i * 64 * 1024);
            }
            fpga_status_timeout = TimGetSystime();
            fpga_firmware_status = EN_FIRMWARE_ANALYSIS_STATUS_START;
            break;
        }

        case EN_FIRMWARE_ANALYSIS_STATUS_START:
        {
            if(TimDifftimeFromNow(fpga_status_timeout) < 5)
            {
                break;
            }
            memcpy((uint8_t *) fpga_firmware_data_save, (uint8_t *)base_ddr_addr, DDR_UPGRATE_DATA_SIZE); //从DDR中拷贝数据到临时数组中
            if(gFPGAFirmwareUpgrateInfo.fpga_len - gFPGAFirmwareUpgrateInfo.fpga_seek < DDR_UPGRATE_DATA_SIZE)
            {
                size_per_transfer = gFPGAFirmwareUpgrateInfo.fpga_len - gFPGAFirmwareUpgrateInfo.fpga_seek;
            }
            else
            {
                size_per_transfer = DDR_UPGRATE_DATA_SIZE;
            }
            fpga_status_timeout = TimGetSystime();
            fpga_firmware_status = EN_FIRMWARE_ANALYSIS_STATUS_BUSY;
            break;
        }

        case EN_FIRMWARE_ANALYSIS_STATUS_BUSY:
        {
            if(TimDifftimeFromNow(fpga_status_timeout) < 5)
            {
                break;
            }
            SpiFlashWriteBytesWithErase(base_firmware_addr, (uint8_t *)fpga_firmware_data_save, size_per_transfer);
//            SpiFlashReadAllData(base_firmware_addr, (uint8_t *)fpga_firmware_data_load, size_per_transfer);
//            if(memcmp(fpga_firmware_data_save, fpga_firmware_data_load, size_per_transfer) != 0)
//            {
//                error++;
//                SpiFlashWriteBytesWithErase(base_firmware_addr, (uint8_t *)fpga_firmware_data_save, size_per_transfer);
//                SpiFlashReadAllData(base_firmware_addr, (uint8_t *)fpga_firmware_data_load, size_per_transfer);
//                if(memcmp(fpga_firmware_data_save, fpga_firmware_data_load, size_per_transfer) != 0)
//                {
//                    SpiFlashWriteBytesWithErase(base_firmware_addr, (uint8_t *)fpga_firmware_data_save, size_per_transfer);
//                    SpiFlashReadAllData(base_firmware_addr, (uint8_t *)fpga_firmware_data_load, size_per_transfer);
//                    if(memcmp(fpga_firmware_data_save, fpga_firmware_data_load, size_per_transfer) != 0)
//                    {
//                        error++;
//                        fpga_firmware_status = EN_FIRMWARE_ANALYSIS_STATUS_IDLE;
//                    }
//                }
//            }
            gFPGAFirmwareUpgrateInfo.fpga_seek += size_per_transfer;
            if((base_firmware_addr == FPGA_FIRMWARE_ADDR) && (base_ddr_addr == FPGA_DDR_FIRMWARE_ADDR))
            {
                fpga_firmware_checksum += CheckSumFun(&fpga_firmware_data_save[256], size_per_transfer - 256, 1);//包含包头的首包checksum
            }
            else
            {
                fpga_firmware_checksum += CheckSumFun(&fpga_firmware_data_save[0], size_per_transfer, 1);
            }
            base_firmware_addr = base_firmware_addr + size_per_transfer;
            pack_index_count++;
            base_ddr_addr = base_ddr_addr + size_per_transfer;
            if(gFPGAFirmwareUpgrateInfo.fpga_seek >= gFPGAFirmwareUpgrateInfo.fpga_len)
            {
                fpga_firmware_status = EN_FIRMWARE_ANALYSIS_STATUS_END;
            }
            else
            {
                fpga_firmware_status = EN_FIRMWARE_ANALYSIS_STATUS_START;
            }
            fpga_status_timeout = TimGetSystime();
            break;
        }

        case EN_FIRMWARE_ANALYSIS_STATUS_END:
        {
            if(fpga_firmware_checksum == gFPGAFirmwareUpgrateInfo.fpga_checksum)
            {
                //升级准备  开始下发升级固件
                memcpy(((uint8_t *) (fpga_firmware_data_save)), (uint8_t *)FPGA_DDR_FIRMWARE_ADDR, 256);
                SpiFlashWriteBytesWithErase(FPGA_FIRMWARE_VERSION, (uint8_t *)fpga_firmware_data_save, 256);//保存扩展版本号
                SpiFlashReadAllData(FPGA_FIRMWARE_VERSION, (uint8_t *)fpga_firmware_data_load, 256);
                if(memcmp(fpga_firmware_data_save, fpga_firmware_data_load, 256) != 0)
                {
                    memcpy(((uint8_t *) (fpga_firmware_data_save)), (uint8_t *)FPGA_DDR_FIRMWARE_ADDR, 256);
                    SpiFlashWriteBytesWithErase(FPGA_FIRMWARE_VERSION, (uint8_t *)fpga_firmware_data_save, 256);
                }
                gFPGAFirmwareUpgrateInfo.upgrate_success_flag = 1;
                gBMSFirmwareUpgrateInfo.bms_upgrate_en = 1;
            }
            else
            {
                gFPGAFirmwareUpgrateInfo.upgrate_fail_flag = 1;
                gBMSFirmwareUpgrateInfo.bms_upgrate_en = 0;
            }
            fpga_firmware_status = EN_FIRMWARE_ANALYSIS_STATUS_IDLE;
            gFPGAFirmwareUpgrateInfo.upgrate_finsh_flag = 1;
            break;
        }
        default:
            break;
    }
}

void BMSUpgrateFirmwareProcess()//固件更新函数
{
    static  EN_BMS_FIRMWARE_UPGRATE_STATUS bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_IDLE;//初始置为空闲
    static  uint32_t    firm_ware_timeout;
    static  uint8_t     error_code;
    static  uint16_t    size_per_transfer;
    static  uint16_t    pack_num_index;
    switch(bms_up_status)
    {
        case EN_FIRMWARE_UPGRATE_STATUS_IDLE:
        {
            if(gBMSFirmwareUpgrateInfo.bms_upgrate_en == 1)//FPGA升级完成开启BMS升级使能
            {
                size_per_transfer = 0;
                error_code = 0;
                pack_num_index = 0;
                pack_count = 0;
                gBMSFirmwareUpgrateInfo.upgrate_finsh_flag = 0;
                gBMSFirmwareUpgrateInfo.bms_upgrate_en = 0;
                gBMSFirmwareUpgrateInfo.upgrate_success_flag = 0;
                gBMSFirmwareUpgrateInfo.upgrate_fail_flag = 0;
                gBMSFirmwareUpgrateInfo.bms_offset = 0;
                gBMSFirmwareUpgrateInfo.bms_len = (bms_ddr_temp[7] | (bms_ddr_temp[8] << 8) | (bms_ddr_temp[9] << 16) | (bms_ddr_temp[10] << 24));///* 固件长度 */
                bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE;//开始升级
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE:
        {
            RegisterBMSUpdataFunPoint(RevBMSUpdataAnalysis);
            gBMSFirmwareUpgrateInfo.bms_seek = 0;
            gBMSFirmwareUpgrateInfo.bms_end_ack = 0;
            gBMSFirmwareUpgrateInfo.bms_ack = 0;
            gBMSFirmwareUpgrateInfo.offset = gBMSFirmwareUpgrateInfo.bms_offset;
            BMSUPGradeSendStart();
            gBMSFirmwareUpgrateInfo.len = gBMSFirmwareUpgrateInfo.bms_len;
            bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE_ACK;
            firm_ware_timeout = TimGetSystime();
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE_ACK:
        {
            if(TimDifftimeFromNow(firm_ware_timeout) < 10)
            {
                break;
            }
            if(gBMSFirmwareUpgrateInfo.bms_ack != 0)
            {
                if(gBMSFirmwareUpgrateInfo.bms_ack == 1)
                {
                    gBMSFirmwareUpgrateInfo.bms_ack = 0;
                    bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY;//发送数据中
                    firm_ware_timeout = TimGetSystime();
                }
                else
                {
                    bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                }
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY:
        {
            if(TimDifftimeFromNow(firm_ware_timeout) < 5)
            {
                break;
            }
            if(gBMSFirmwareUpgrateInfo.len - gBMSFirmwareUpgrateInfo.bms_seek < UPGRATE_DATA_SIZE)
            {
                size_per_transfer = gBMSFirmwareUpgrateInfo.len - gBMSFirmwareUpgrateInfo.bms_seek;//最后一包长度
            }
            else if(gBMSFirmwareUpgrateInfo.bms_seek < UPGRATE_DATA_SIZE)
            {
                size_per_transfer = UPGRATE_DATA_SIZE;
            }
            BMSUPGradeSendBusy(size_per_transfer, gBMSFirmwareUpgrateInfo.offset + gBMSFirmwareUpgrateInfo.bms_seek, pack_num_index);//发送数据
            pack_count ++;
            firm_ware_timeout = TimGetSystime();
            bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_WAIT_DATA_BUSY_ACK;
            gBMSFirmwareUpgrateInfo.bms_ack = 0;
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_WAIT_DATA_BUSY_ACK:
        {
            if(TimDifftimeFromNow(firm_ware_timeout) < 5)
            {
                break;
            }
            if(gBMSFirmwareUpgrateInfo.bms_ack != 0)
            {
                if(gBMSFirmwareUpgrateInfo.bms_ack == 1)
                {
                    pack_num_index ++;
                    error_code = 0;
                    gBMSFirmwareUpgrateInfo.bms_seek += size_per_transfer;//加入偏移地址
                    if(gBMSFirmwareUpgrateInfo.bms_seek >= gBMSFirmwareUpgrateInfo.len)
                    {
                        bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_END;//结束
                        firm_ware_timeout = TimGetSystime();
                    }
                    else
                    {
                        firm_ware_timeout = TimGetSystime();
                        bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY;
                    }
                }
                else if(gBMSFirmwareUpgrateInfo.bms_ack == 2)
                {
                    error_code ++;
                    if(error_code <= 3)
                    {
                        firm_ware_timeout = TimGetSystime();
                        bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY;
                    }
                    else if(error_code > 3)
                    {
                        bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_ERROR;//状态直接为错误
                    }
                }
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_SEND_END:
        {
            if(TimDifftimeFromNow(firm_ware_timeout) < 5)
            {
                break;
            }
            BMSUPGradeSendEnd();
            firm_ware_timeout = TimGetSystime();
            bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK;//结束检查
            firm_ware_timeout = TimGetSystime();
            gBMSFirmwareUpgrateInfo.bms_ack = 0;
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK:
        {
            if(gBMSFirmwareUpgrateInfo.bms_end_ack != 0)
            {
                if(gBMSFirmwareUpgrateInfo.bms_end_ack == 1)
                {
                    bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_OK;
                }
                else
                {
                    bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                }
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_ERROR:
        {
            gBMSFirmwareUpgrateInfo.upgrate_fail_flag = 1;
            bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_IDLE;
            gBMSFirmwareUpgrateInfo.upgrate_finsh_flag = 1;
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_OK:
        {
            gBMSFirmwareUpgrateInfo.upgrate_success_flag = 1;
            bms_up_status = EN_FIRMWARE_UPGRATE_STATUS_IDLE;
            gBMSFirmwareUpgrateInfo.upgrate_finsh_flag = 1;
            break;
        }
        default:
            break;
    }
    if(bms_up_status == EN_FIRMWARE_UPGRATE_STATUS_IDLE)//空闲状态的置位
    {
        gBMSFirmwareUpgrateInfo.bms_upgrating = 0;
    }
    else
    {
        gBMSFirmwareUpgrateInfo.bms_upgrating = 1; //更新中
    }
}

void FirmwareUpgrate()
{
    if((BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 14) == 1) || ((BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 13) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 14) == 0)))
    {
        //升级成功或者失败
        return;
    }
    if((BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 0) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 8) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 10) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 12) == 1))
    {
        //含有FPGA 和 BMS固件升级包

        if((gBMSFirmwareUpgrateInfo.upgrate_success_flag == 1) && (gFPGAFirmwareUpgrateInfo.upgrate_success_flag == 1))
        {
            gstDCDCReg->dc_dc_hold_para.updata_status  |= 0x2000;
            gDDRFirmwareInfo.ddr_bms_firmware_success = 0;
            gDDRFirmwareInfo.ddr_fpga_firmware_success = 0;
        }
        else if((gBMSFirmwareUpgrateInfo.upgrate_fail_flag == 1) || (gFPGAFirmwareUpgrateInfo.upgrate_fail_flag == 1))
        {
            gstDCDCReg->dc_dc_hold_para.updata_status |= 0x4000;
            gDDRFirmwareInfo.ddr_bms_firmware_success = 0;
            gDDRFirmwareInfo.ddr_fpga_firmware_success = 0;
        }
    }
    else if((BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 0) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 8) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 10) == 0) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 12) == 1))
    {
        //只含有BMS固件升级包
        if(gBMSFirmwareUpgrateInfo.upgrate_success_flag == 1)
        {
            gstDCDCReg->dc_dc_hold_para.updata_status  |= 0x2000;
            gDDRFirmwareInfo.ddr_bms_firmware_success = 0;

        }
        else if(gBMSFirmwareUpgrateInfo.upgrate_fail_flag == 1)
        {
            gstDCDCReg->dc_dc_hold_para.updata_status |= 0x4000;
            gDDRFirmwareInfo.ddr_bms_firmware_success = 0;

        }
    }
    else if((BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 0) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 8) == 0) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 10) == 1) && (BITCHECK(gTempSlaveDCDCPara.dc_dc_hold_para.updata_status, 12) == 1))
    {
        //只含有FPGA固件升级包
        if(gFPGAFirmwareUpgrateInfo.upgrate_success_flag == 1)
        {
            gstDCDCReg->dc_dc_hold_para.updata_status  |= 0x2000;
            gDDRFirmwareInfo.ddr_fpga_firmware_success = 0;

        }
        else if(gFPGAFirmwareUpgrateInfo.upgrate_fail_flag == 1)
        {
            gstDCDCReg->dc_dc_hold_para.updata_status |= 0x4000;
            gDDRFirmwareInfo.ddr_fpga_firmware_success = 0;

        }
    }
}
