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


#include "private_analysis.h"
#define BITCHECK(data,number)           ((data >> number) & (0x01))
uint8_t garrBMSTxbuf[600];
uint8_t garrDCDCTxbuf[600];
SlaveBmsWrite_t     gBmsModbusSlaveWrite;//私有协议
PrivateBmsWrite_t   gBmsPrivateWrite;//私有协议
UdpRevData_t        gUdpPara[REV_QUEUE_DEPTH];
uint8_t             RevData[REV_QUEUE_DEPTH][COM_RX_SIZE];

Queue_t gBMSModebusRevQueue;
Queue_t gBmsSendQueue;
Queue_t gUdpSlaveCommSendQueue;
Queue_t gBmsModbusSlaveCommSendQueue;
static uint8_t _read_bms_data[20];
static uint8_t garrWriteTxBuf[100];

void SlaveCommInit()
{
    QueueInit(&gBMSModebusRevQueue, SLAVE_WRITE_QUEUE_DEPTH, sizeof(SlaveWrite_t));//标准协议

    QueueInit(&gBmsSendQueue, SLAVE_WRITE_QUEUE_DEPTH, sizeof(PrivateBmsWrite_t));//给BMS写数据队列——私有协议
    QueueInit(&gUdpSlaveCommSendQueue, REV_QUEUE_DEPTH, sizeof(UdpRevData_t));//UDP通信协议
    for(uint8_t i = 0; i < SLAVE_WRITE_QUEUE_DEPTH; i++)
    {
        gUdpPara[i].pData = RevData[i];
    }
}

uint16_t CheckSum(uint8_t *p, uint16_t size)
{
    uint16_t i;
    uint16_t ret = 0;
    for(i = 0; i < size; i++)
    {
        ret += p[i];
    }
    return ret;
}

uint32_t CheckSumFun(uint8_t *pdata, uint16_t len, uint8_t mode)
{
    uint16_t checksum16 = 0;
    uint32_t checksum32 = 0;
    uint16_t i = 0;

    if(mode)
    {
        for(i = 0; i < len; i ++)
        {
            checksum32 += pdata[i];
        }

        return checksum32;
    }
    else
    {
        for(i = 0; i < len; i ++)
        {
            checksum16 += pdata[i];
        }
        return checksum16;
    }
}
void ReadBMSReg()
{
    uint16_t check_sum;
    _read_bms_data[0] = 0x5A;
    _read_bms_data[1] = 0x00;
    _read_bms_data[2] = 0x03;
    _read_bms_data[3] = READ_BMS_INFO_CMD & 0xFF;
    _read_bms_data[4] = (READ_BMS_INFO_CMD >> 8) & 0xFF;
    _read_bms_data[5] = 0x00;
    _read_bms_data[6] = 0x00;
    check_sum = CheckSum(_read_bms_data, 7);
    _read_bms_data[7] = check_sum & 0xFF;
    _read_bms_data[8] = (check_sum >> 8) & 0xFF;
    Uart0Send(_read_bms_data, 9);
}

void ReadDCDCReg()
{
    uint16_t size = 0;
    if((sizeof(DCDCPara_t)) % 4 != 0)
    {
        size = ((sizeof(DCDCPara_t) / 4) + 1) * 4;
    }
    else
    {
        size = sizeof(DCDCPara_t);
    }
    memcpy((uint8_t*)&gTempSlaveDCDCPara, (uint8_t *)gstDCDCReg, size);
}

void WriteRestBmsData()
{
    uint16_t check_sum = 0;
    uint16_t data_field_len = 0;
    garrWriteTxBuf[0] = 0x5A;
    garrWriteTxBuf[1] = 0x00;
    garrWriteTxBuf[2] = 0x03;
    garrWriteTxBuf[3] = WRITE_BMS_INFO_CMD & 0xFF;
    garrWriteTxBuf[4] = (WRITE_BMS_INFO_CMD >> 8) & 0xFF;
    /* 数据域长度*/
    data_field_len = 5 + 2;
    garrWriteTxBuf[5] = data_field_len & 0xFF;
    garrWriteTxBuf[6] = (data_field_len >> 8) & 0xFF;
    /*数据域*/
    garrWriteTxBuf[7] = 0x00; //BMS序号
    garrWriteTxBuf[8] = 0x72; //BMS寄存器起始地址
    garrWriteTxBuf[9] = 0x01; //BMS寄存器起始地址

    garrWriteTxBuf[10] = 0x02 ; //数据长度
    garrWriteTxBuf[11] = 0x00; //数据长度

    garrWriteTxBuf[12] = 0x01; //数据
    garrWriteTxBuf[13] = 0x00; //数据
    check_sum = CheckSum(garrWriteTxBuf, 14);
    garrWriteTxBuf[14] = check_sum & 0xFF;
    garrWriteTxBuf[15] = (check_sum >> 8) & 0xFF;
    Uart0Send(garrWriteTxBuf, 16);
}

void WriteBmsData(uint32_t reg_start_addr, uint16_t data_len, uint8_t *pReg_value)
{
    uint16_t check_sum = 0;
    uint16_t i = 0;
    uint16_t data_field_len = 0;
    reg_start_addr = (reg_start_addr * 2) + 0x30010000;
    garrWriteTxBuf[0] = 0x5A;
    garrWriteTxBuf[1] = 0x00;
    garrWriteTxBuf[2] = 0x03;
    garrWriteTxBuf[3] = WRITE_BMS_INFO_CMD & 0xFF;
    garrWriteTxBuf[4] = (WRITE_BMS_INFO_CMD >> 8) & 0xFF;
    /* 数据域长度*/
    data_field_len = 5 + data_len;
    garrWriteTxBuf[5] = data_field_len & 0xFF;
    garrWriteTxBuf[6] = (data_field_len >> 8) & 0xFF;
    /*数据域*/
    garrWriteTxBuf[7] = 0x00; //BMS序号
    garrWriteTxBuf[8] = reg_start_addr & 0xFF; //BMS寄存器起始地址
    garrWriteTxBuf[9] = (reg_start_addr >> 8) & 0xFF; //BMS寄存器起始地址
    garrWriteTxBuf[10] = data_len & 0xFF; //数据长度
    garrWriteTxBuf[11] = (data_len >> 8) & 0xFF; //数据长度
    for(i = 0; i < (data_len >> 1); i++)
    {
        garrWriteTxBuf[2 * i + 12] = pReg_value[2 * i];
        garrWriteTxBuf[2 * i + 13] = pReg_value[2 * i + 1];
    }
    check_sum = CheckSum(garrWriteTxBuf, 12 + data_len);
    garrWriteTxBuf[2 * i + 12] = check_sum & 0xFF;
    garrWriteTxBuf[2 * i + 13] = (check_sum >> 8) & 0xFF;
    Uart0Send(garrWriteTxBuf, 12 + 2 + data_len);
}

void WriteDCDCData(uint32_t reg_start_addr, uint16_t data_len, uint8_t *pReg_value)
{
    uint8_t i = 0;
    i = (sizeof(DCDCHoldPara_t)) % 4;
    reg_start_addr = reg_start_addr + 0x30020000;
    memcpy((uint8_t *)((uint8_t *)(&(gTempSlaveDCDCPara.dc_dc_hold_para.ip_address_high)) + (reg_start_addr - 0x30020090) * 2), ((uint8_t *)(pReg_value)), data_len);
    if(i != 0)
    {
        memcpy(((uint8_t *) & (gstDCDCReg->dc_dc_hold_para)), ((uint8_t *) & (gTempSlaveDCDCPara.dc_dc_hold_para)), ((sizeof(DCDCHoldPara_t) / 4) + 1) * 4);
    }
    else
    {
        memcpy(((uint8_t *) & (gstDCDCReg->dc_dc_hold_para)), ((uint8_t *) & (gTempSlaveDCDCPara.dc_dc_hold_para)), sizeof(DCDCHoldPara_t));
    }
}

static bool _AckCmdAnalysis(uint8_t *pBuf, uint16_t len)
{
    uint16_t cmd = 0;
    uint8_t i = 0;
    cmd =  pBuf[0] | (pBuf[1] << 8);
    switch(cmd)
    {
        case READ_BMS_INFO_CMD:
        {
            if(pBuf[2] == 0)
            {
                memcpy((uint8_t*)&gSlaveBMSPara.bms_only_read_para, &pBuf[8], sizeof(BMSPara_t));
                i = (sizeof(BMSPara_t)) % 4;
                if(i != 0)
                {
                    memcpy(((uint8_t *) & (gstBmsReg->bms_only_read_para)), ((uint8_t *) & (gSlaveBMSPara.bms_only_read_para)), ((sizeof(BMSPara_t) / 4) + 1) * 4);
                }
                else
                {
                    memcpy(((uint8_t *) & (gstBmsReg->bms_only_read_para)), ((uint8_t *) & (gSlaveBMSPara.bms_only_read_para)), sizeof(BMSPara_t));
                }
            }
            break;
        }
        case WRITE_BMS_INFO_CMD:
        {
            break;
        }
        default:
            break;
    }
    return true;
}

bool ProtoUnpackJudge(ComRevData_t *p)
{
    uint16_t i = 0;
    uint16_t checksum = 0;
    uint8_t *pBuf = p->pData;
    uint16_t len = p->data_size;

    for(i = 0; i < len - 2; i++)
    {
        checksum += pBuf[i];
    }

    if(checksum != (pBuf[len - 2] | (pBuf[len - 1] << 8)))
    {
        return false;
    }
    //从机响应——开始解包数据
    _AckCmdAnalysis(&pBuf[3], len - 5);
    return true;
}

uint16_t  packcount1 = 0;
bool UdpProtopackJudge(FpgaUdpRevData_t *p)
{
    static      uint16_t faulrecord_index = 0;
    uint8_t     garrDCDCFaultTxbuf[160] = {0};
    uint16_t    device_type = 0;
    uint8_t     reserved = 0;
    uint8_t     cmd = 0;
    uint32_t    data_addr = 0;
    uint16_t    reg_data_addr = 0;
    uint16_t    data_len = 0;
    uint8_t     *pBuf = p->pData;
    uint16_t    len = p->data_size;
    uint16_t    *p_recv_data = NULL;
    uint8_t     *reg_start_add = 0;
    uint16_t    read_len = 0;
    uint8_t     reg_len = 0;

    if(p == NULL || p->pData == NULL)
    {
        return false;
    }
    if(len < 8)
    {
        return false;
    }
    device_type = *(uint16_t *)&pBuf[0];//device_type
    reserved = *(uint8_t *)&pBuf[2];
    if(reserved == 0)
    {
        reg_len = 1;
    }
    else if(reserved == 1)
    {
        reg_len = 2;
    }
    else if(reserved == 2)
    {
        reg_len = 3;
    }
    else if(reserved == 3)
    {
        reg_len = 4;
    }
    if(device_type != 0x0003)
    {
        return false;
    }
    cmd = pBuf[3]; //opCmd
    data_addr = *(uint32_t *)&pBuf[4];//
    reg_data_addr = *(uint16_t *)&pBuf[4];//寄存器起始地址 此地址监控卡下发不包含  偏移地址0x3001-0000
    if((cmd != 0x43) && (cmd != 0x45))
    {
        //0x45写操作、0x43读操作
        return false;
    }
//    if(data_addr  >= 0x30020109)
//    {
//        return false;
//    }

    data_len = (*((uint16_t *)&pBuf[8]) * reg_len) ; //数据总长度；取出udp协议的长度  新版协议数据长度 = 寄存器个数不包含udp数据头，因此字节长度 = 寄存器个数*2 +数据包头的长度
    if((cmd == 0x43) && (data_addr >= 0x30010000) && (data_addr <= 0x300100FF))//收到监控卡读BMS操作的数据回复
    {
        /* 小端转大端 */
        for(uint16_t i = 0; i < (sizeof(BMSPara_t) / 2); i ++)
        {
            TabLsbToMsb((uint8_t*)&gSlaveBMSPara + i * 2, 2);
        }
        read_len =  data_len;
        garrBMSTxbuf[0] = 0x03;    //2字节 device_type
        garrBMSTxbuf[1] = 0x00;
        garrBMSTxbuf[2] = 0x01;    //1字节 reserved
        garrBMSTxbuf[3] = 0x43;    //1字节 opCmd
        garrBMSTxbuf[4] = data_addr;    //4字节 Addr
        garrBMSTxbuf[5] = (uint8_t)(data_addr >> 8);
        garrBMSTxbuf[6] = (uint8_t)(data_addr >> 16);
        garrBMSTxbuf[7] = (uint8_t)(data_addr >> 24);
        garrBMSTxbuf[8] = (*((uint16_t *)&pBuf[8]));//数据总长度只包含 数据长度
        garrBMSTxbuf[9] = (uint8_t)((*((uint16_t *)&pBuf[8])) >> 8);
        reg_start_add = (uint8_t*)&gSlaveBMSPara.bms_only_read_para;//加入偏移地址
        memcpy((uint8_t*)&garrBMSTxbuf[10], (reg_start_add + reg_data_addr * 2), read_len);
        memcpy((uint8_t*)CACHE_UDPDATA_TX, garrBMSTxbuf, (((read_len + 10) / 4) + 1) * 4); //4字节对齐，多copy字节，发出按实际值
        LEN = read_len + 10;
        READY = 1;
        READY = 0;
        /* 大端转小端 */
        for(uint16_t i = 0; i < (sizeof(BMSPara_t) / 2); i ++)
        {
            TabMsbToLsb((uint8_t*)&gSlaveBMSPara + i * 2, 2);
        }
        return true;
    }
    else if((cmd == 0x43) && (data_addr >= 0x30020000) && (data_addr <= 0x300200FF))//收到监控卡读DCDC操作的数据回复
    {
        /* 小端转大端 */
        for(uint16_t i = 0; i < (sizeof(DCDCPara_t) / 2); i ++)
        {
            TabLsbToMsb((uint8_t*)&gTempSlaveDCDCPara + i * 2, 2);
        }
        read_len =  data_len;
        garrDCDCTxbuf[0] = 0x03;    //2字节 device_type
        garrDCDCTxbuf[1] = 0x00;
        garrDCDCTxbuf[2] = 0x01;    //1字节 reserved
        garrDCDCTxbuf[3] = 0x43;    //1字节 opCmd
        garrDCDCTxbuf[4] = data_addr;    //4字节 Addr
        garrDCDCTxbuf[5] = (uint8_t)(data_addr >> 8);
        garrDCDCTxbuf[6] = (uint8_t)(data_addr >> 16);
        garrDCDCTxbuf[7] = (uint8_t)(data_addr >> 24);
        garrDCDCTxbuf[8] = (*((uint16_t *)&pBuf[8]));//只包含 数据长度
        garrDCDCTxbuf[9] = (uint8_t)((*((uint16_t *)&pBuf[8])) >> 8);

        reg_start_add = (uint8_t*)&gTempSlaveDCDCPara.dc_dc_only_read_para;//加入偏移地址
        memcpy((uint8_t*)&garrDCDCTxbuf[10], (reg_start_add + reg_data_addr * 2), read_len);
        memcpy((uint8_t*)CACHE_UDPDATA_TX, garrDCDCTxbuf, (((read_len + 10)  / 4) + 1) * 4); //4字节对齐，多copy字节，发出按实际值
        LEN = read_len + 10;
        READY = 1;
        READY = 0;
        packcount1 ++;
        /* 大端转小端 */
        for(uint16_t i = 0; i < (sizeof(DCDCPara_t) / 2); i ++)
        {
            TabMsbToLsb((uint8_t*)&gTempSlaveDCDCPara + i * 2, 2);
        }
        return true;
    }
    else if((cmd == 0x43) && (data_addr >= 0x30030000) && (data_addr <= 0x300CFFFF))//收到读取录波故障的命令码
    {
        read_len =  data_len;
        if(read_len != 0x90)
        {
            return true;
        }
        garrDCDCFaultTxbuf[0] = 0x03;    //2字节 device_type
        garrDCDCFaultTxbuf[1] = 0x00;
        garrDCDCFaultTxbuf[2] = 0x01;    //1字节 reserved
        garrDCDCFaultTxbuf[3] = 0x43;    //1字节 opCmd
        garrDCDCFaultTxbuf[4] = data_addr;    //4字节 Addr
        garrDCDCFaultTxbuf[5] = (uint8_t)(data_addr >> 8);
        garrDCDCFaultTxbuf[6] = (uint8_t)(data_addr >> 16);
        garrDCDCFaultTxbuf[7] = (uint8_t)(data_addr >> 24);
        garrDCDCFaultTxbuf[8] = (*((uint16_t *)&pBuf[8]));//只包含数据长度
        garrDCDCFaultTxbuf[9] = (uint8_t)((*((uint16_t *)&pBuf[8])) >> 8);

        garrDCDCFaultTxbuf[10] = 0xA5;
        garrDCDCFaultTxbuf[11] = 0x5A;
        garrDCDCFaultTxbuf[12] = 0xFE;
        garrDCDCFaultTxbuf[13] = 0x01;

//        SpiFlashReadAllData(0x004C2000 + (data_addr - 0x30030000) + faulrecord_index * 0x90, (uint8_t *)&garrDCDCFaultTxbuf[14], read_len);
		SpiFlashReadAllData(0x004C2000 + (data_addr - 0x30030000), (uint8_t *)&garrDCDCFaultTxbuf[14], read_len);

        garrDCDCFaultTxbuf[14 + read_len] = 0x00;
        garrDCDCFaultTxbuf[15 + read_len] = 0x00;
        garrDCDCFaultTxbuf[16 + read_len] = 0x80;
        garrDCDCFaultTxbuf[17 + read_len] = 0x7F;

        memcpy((uint8_t*)CACHE_UDPDATA_TX, garrDCDCFaultTxbuf, (((read_len + 10 + 8)  / 4) + 1) * 4); //4字节对齐，多copy字节，发出按实际值
        faulrecord_index++;
		packcount1 ++;
        LEN = read_len + 10 + 8;
        READY = 1;
        READY = 0;
        return true;
    }
    else if((cmd == 0x45) && (data_addr >= 0x30010000) && (data_addr < 0x30020000))//收到监控卡写BMS操作
    {
        p_recv_data = (uint16_t *)&pBuf[10];
        /* 大端转小端 */
        for(uint16_t i = 0; i < data_len / 2; i ++)
        {
            TabMsbToLsb((uint8_t*)&pBuf[10] + i * 2, 2);
        }
        BMSWriteDataSeed(reg_data_addr, data_len, (uint8_t*)p_recv_data);
        return true;
    }
    else if((cmd == 0x45) && (data_addr >= 0x30020000) && (data_addr <= 0x300200FF))//收到监控卡写DCDC操作
    {
        p_recv_data = (uint16_t *)&pBuf[10];
        /* 大端转小端 */
        for(uint16_t i = 0; i < data_len / 2; i ++)
        {
            TabMsbToLsb((uint8_t*)&pBuf[10] + i * 2, 2);
        }
        WriteDCDCData(reg_data_addr, data_len, (uint8_t*)p_recv_data);
        return true;
    }
    else if((cmd == 0x45) && (data_addr >= 0x20000000) && (data_addr <= 0x246FFFFF))//收到监控卡 升级协议 给DDR写数据
    {
        p_recv_data = (uint16_t *)&pBuf[10];
        WriteDDRSend(data_addr, (uint8_t *)p_recv_data, data_len);
        return true;
    }
    return false;
}

bool BMSWriteDataSeed(uint32_t reg_start_addr, uint16_t len, uint8_t *data)
{
    gBmsPrivateWrite.reg_addr = reg_start_addr;
    gBmsPrivateWrite.reg_num = len;
    gBmsPrivateWrite.p = data;
    if(QueuePut(&gBmsSendQueue, &gBmsPrivateWrite) == false)
    {
        return false;
    }
    return true;
}

uint8_t RevBMSUpdataAnalysis(uint8_t *p, uint16_t size)
{
    uint8_t *pReceBuf = p;
    uint16_t error_code = pReceBuf[5];
    if(pReceBuf[4] == 0x80)
    {
        switch(pReceBuf[3])
        {
            case 0xF0://升级结束返回  SEND_CMD
            {
                if(error_code == 0)
                {
                    gBMSFirmwareUpgrateInfo.bms_ack = 1;
                }
                else
                {
                    gBMSFirmwareUpgrateInfo.bms_ack = 2;
                }
                break;
            }
            case 0xF1:    //发送数据返回。
            {
                if(error_code == 0)
                {
                    gBMSFirmwareUpgrateInfo.bms_ack = 1;
                }
                else
                {
                    gBMSFirmwareUpgrateInfo.bms_ack = 2;
                }
                break;
            }
            case 0xFF:    //发送数据返回。
            {
                if(error_code == 0)
                {
                    gBMSFirmwareUpgrateInfo.bms_end_ack = 1;
                }
                else
                {
                    gBMSFirmwareUpgrateInfo.bms_end_ack = 2;
                }
                break;
            }
            default:
                break;
        }
    }
    return 0;
}








