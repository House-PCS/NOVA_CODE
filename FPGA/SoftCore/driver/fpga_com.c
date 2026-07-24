/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-07-20
  * @brief       N32G4FR
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * <tr><td>2023-06-15    <td>V1.0.1    <td>           <td>增加同一串口可使用多种中断并存的机制
  * </table>
  ******************************************************************************
  */
#include "fpga_com.h"
#include "systick.h"
#include "queue.h"
#include "private_analysis.h"
uint16_t fpga_count = 0;
uint8_t comm_overtime_flag = 1;
uint8_t comm_fail_flag = 0;
uint8_t comm_overtime_count = 0;
static uint32_t comm_timeout = 0;
uint8_t shutdown[5] = {0x08};//0x0000 1000
uint16_t count = 0;
void INT02_Handler()
{
    static uint16_t index = 0;
    uint16_t reg_num = 0;
    EthIrqClr = 1;//清除中断
    comm_overtime_flag = 0;
    count++;
    if((CACHE_UDPDATA_RX->device_type ==  0x0001) || (CACHE_UDPDATA_RX->device_type ==  0x0002) || (CACHE_UDPDATA_RX->device_type ==  0x0003) || (CACHE_UDPDATA_RX->device_type ==  0x0004) || (CACHE_UDPDATA_RX->device_type ==  0x0005))
    {
        if(CACHE_UDPDATA_RX->reserved == 0)
        {
            reg_num = 1;
            fpga_count++;
        }
        else if(CACHE_UDPDATA_RX->reserved == 1)
        {
            reg_num = 2;
        }
        else if(CACHE_UDPDATA_RX->reserved == 2)
        {
            reg_num = 3;
        }
        else if(CACHE_UDPDATA_RX->reserved == 3)
        {
            reg_num = 4;
        }
        gUdpPara[index].data_size = (CACHE_UDPDATA_RX->len * reg_num) + 10; //取出udp协议的长度  新版协议数据长度 = 寄存器个数不包含udp数据头，因此字节长度 = 寄存器个数*2 +数据包头的长度
        if(gUdpPara[index].data_size > COM_RX_SIZE)
        {
            return;
        }
        memcpy((uint8_t*)gUdpPara[index].pData, CACHE_UDPDATA_RX, gUdpPara[index].data_size );
        if(QueuePut(&gUdpSlaveCommSendQueue, &gUdpPara[index]) == false)
        {
            return;
        }
        index++;
        index %= SCI_REV_QUEUE_DEPTH;
    }
}

void FpgaComInit(void)//串口初始
{
    READY = 0;
    EthIrqClr = 0x01;
    NVIC_EnableIRQ(INT02_IRQn);
}

void Commovertime(void)
{
    if(comm_overtime_flag == 1)
    {
        comm_overtime_count ++;
    }
    else
    {
		comm_fail_flag = 0;
        comm_overtime_count = 0;
        comm_timeout = TimGetSystime();
    }
    comm_overtime_flag = 1;
    if((comm_overtime_count > 10) && (TimDifftimeFromNow(comm_timeout) > 100000))
    {
		comm_fail_flag = 1;
    }
    if((comm_overtime_count > 10) && (TimDifftimeFromNow(comm_timeout) > 120000))
    {
		comm_fail_flag = 1;
        BMSWriteDataSeed(0x300100EC, 2, &shutdown[0]);
    }	
}
