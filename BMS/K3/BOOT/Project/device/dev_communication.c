#include "dev_communication.h"
#include "math.h"
#include "mid_shell.h"
#include "firmware_iap.h"
#define LOG_TAG     "dev_communication"

#define UartComHandle  Uart1_ModS_Handle

#define REQUST_PACKET_HEAD      0x5A
#define RESPONSE_PACKET_HEAD    0xA5
#define SEND_ADDR               0x00 /* 主控 */
#define RECV_ADDR               0x03 /* BMS */
#define  ACK_OK                             0x00             /* ack success */
#define  ACK_ERROR                          0x01             /* ack error */
#define  ACK_INVALID                        0xFF             /* ack invalid */

static uint8_t _gComRxBuf[_COM_RX_SIZE];
static ComRevData_t _gComRevData[COM_REV_QUEUE_DEPTH];
static uint8_t _RevData[COM_REV_QUEUE_DEPTH][_COM_RX_SIZE];
static uint8_t _RevDataBufIndex = 0;
static uint16_t _DataRevSize = 0;
static ProtocalRequestPacket_t          _gstRequestPacketData       = {0};
static ProtocalResponsePacket_t         _gstResponsePacketData = {0};


static uint8_t  _ProtocolBuf[_COM_RX_SIZE] = {0};
uint8_t gBMSFirmwareUpgradeEnd = 0;
uint8_t gBMSHardVersion[3] = {0};
uint8_t gBMSSpaceSize = 0;
uint16_t gBMSVerExt[6] = {0};
uint32_t gBMSHardwareLen;
uint32_t gBMSbmsFirmwareChecksum;
uint8_t gBMSFirmwareType[4] = {0x0C, 0x0C, 0x0C, 0x03};
Queue_t gComRevQueue;


uint8_t pack_count = 0;
uint16_t updata_pack_size = 0;
uint8_t space_num = 0;
static void IrqRxProcess(void);

uint32_t CheckSum(uint8_t *pdata, uint16_t len, uint8_t mode)
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

static void _RevDataAnalysis()
{
    uint8_t *p = UartComHandle.pRxBuf;
    if((*UartComHandle.pRxBuf == REQUST_PACKET_HEAD) && (*(UartComHandle.pRxBuf + 1) == SEND_ADDR) && \
            (*(UartComHandle.pRxBuf + 2) == RECV_ADDR))
    {
        if(QueueIsFull() == true)
        {
            return;//扔掉此包数据
        }

        _gComRevData[_RevDataBufIndex].data_type = COM_TYPE_UART_DATA;
        _gComRevData[_RevDataBufIndex].data_size = _DataRevSize;
        memcpy(_gComRevData[_RevDataBufIndex].pData, p, _DataRevSize);
        if(QueuePut(&gComRevQueue, &_gComRevData[_RevDataBufIndex]) == false)
        {
            return;
        }
        _RevDataBufIndex++;
        _RevDataBufIndex %= COM_REV_QUEUE_DEPTH;
    }
}

static void IrqRxProcess(void)
{
    /*DMA方式，空闲中断机制*/
    UartDMARxDisable(&UartComHandle);
    _DataRevSize = UartDMARxDataLength(&UartComHandle);
    _RevDataAnalysis();
    UartDMARxEnable(&UartComHandle);
}

bool CommunicationInit(void)
{
    bool ret;
    uint8_t times = 0;
    uint8_t i = 0;

    QueueInit(&gComRevQueue, COM_REV_QUEUE_DEPTH, sizeof(ComRevData_t));
    for(i = 0; i < COM_REV_QUEUE_DEPTH; i++)
    {
        _gComRevData[i].pData = _RevData[i];
    }

    UartComHandle.pRxBuf = _gComRxBuf;
    UartComHandle.rx_size = _COM_RX_SIZE;
    UartComHandle.irq_rx_process = IrqRxProcess;
    Uart1ModsBspInit();

    if(UartComHandle.uart_cfg->en_dma_rx == true)
    {
        /* 重新设置DMA传输(初始化DMA还未映射rx_buf,上面映射完后重新初始化一下DMA)*/
        UartDMARxDisable(&UartComHandle);
        UartDMARxEnable(&UartComHandle);
    }
    return true;
}

void DataSendPolling()
{
    /* 组包 */
    _gstResponsePacketData.packet_head = RESPONSE_PACKET_HEAD;
    _gstResponsePacketData.send_addr   = _gstRequestPacketData.recv_addr;
    _gstResponsePacketData.recv_addr   = _gstRequestPacketData.send_addr;
    _gstResponsePacketData.cmd         = _gstRequestPacketData.cmd;

    _gstResponsePacketData.check_sum = CheckSum((uint8_t*)&_gstResponsePacketData, sizeof(_gstResponsePacketData) - 2, 0);

    UartComHandle.pTxBuf = (uint8_t*)&_gstResponsePacketData;
    UartComHandle.tx_size = sizeof(ProtocalResponsePacket_t);
    UartSend(&UartComHandle);
    if(gBMSFirmwareUpgradeEnd == 1)
    {
        FirmwareCopy();
        enterbootmodejudge();
    }
    return;
}

static bool _VerifyBmsFirmware(void)
{
    uint16_t i = 0;
    uint16_t block_num;
    uint16_t block_num_behind;
    uint8_t rdata[BLOCK_SIZE] = {0};
    uint32_t checksum = 0;
    uint32_t bms_firmware_addr = BMS_FIRMWARE_ADDR;

    block_num = gBMSHardwareLen / BLOCK_SIZE;
    block_num_behind = gBMSHardwareLen % BLOCK_SIZE;

    for(i = 0; i < block_num; i ++)
    {
        spi_flash_read_bytes(bms_firmware_addr, rdata, BLOCK_SIZE);
        checksum += CheckSum(rdata, BLOCK_SIZE, 1);
        bms_firmware_addr += BLOCK_SIZE;
    }

    spi_flash_read_bytes(bms_firmware_addr, rdata, block_num_behind);
    checksum += CheckSum(rdata, block_num_behind, 1);

    if(checksum != gBMSbmsFirmwareChecksum)
    {
        return false;
    }

    return true;
}

static bool _CmdAnalysis(ProtocalRequestPacket_t *p_request_packet)
{
    uint16_t cmd = p_request_packet->cmd;
    static uint32_t base_firmware_addr;
    static uint32_t firmware_checksum_compute;
    static uint32_t firmware_checksum_rec;
    static uint32_t num_index;
    static uint32_t last_num_index;
    switch(cmd)
    {
        case UPGRADE_START:
        {
            /* 初始化程序存放地址 */
            base_firmware_addr          = BMS_FIRMWARE_ADDR;
            _gstResponsePacketData.error_code = ACK_OK;
            firmware_checksum_rec       = 0;
            firmware_checksum_compute   = 0;
            updata_pack_size = *((uint32_t*)&p_request_packet->data_field[0]);
            space_num = *((uint8_t*)&p_request_packet->data_field[4]);
            pack_count = 0;
            break;
        }
        case UPGRADE_BUSY:
        {
            _gstResponsePacketData.error_code = ACK_OK;
            /* 第一包数据 */
            if((base_firmware_addr == BMS_FIRMWARE_ADDR) && (pack_count == 0))
            {
                if(memcmp(&p_request_packet->data_field[2], gBMSFirmwareType, 4) == 0)
                {
                    base_firmware_addr = BMS_FIRMWARE_ADDR;

                }
                else
                {
                    _gstResponsePacketData.error_code = ACK_ERROR;
                    return false;
                }
                /* 固件版本 */
                memcpy(gBMSHardVersion, &p_request_packet->data_field[6], 3);
                /* 固件长度 */
                gBMSHardwareLen = *((uint32_t*)&p_request_packet->data_field[9]);
                /* 固件校验和 */
                firmware_checksum_rec = *((uint32_t*)&p_request_packet->data_field[254]);
            }
            /*第几包序号 */
            num_index = *((uint16_t*)&p_request_packet->data_field[0]);
            spi_flash_write_bytes_with_erase(base_firmware_addr, &p_request_packet->data_field[space_num], p_request_packet->data_field_len - space_num);
            spi_flash_read_bytes(base_firmware_addr, _ProtocolBuf, p_request_packet->data_field_len - space_num);
            if((base_firmware_addr == BMS_FIRMWARE_ADDR) && (pack_count == 0))
            {
                /* 计算 checksum */
                last_num_index = num_index;
                firmware_checksum_compute += CheckSum(&_ProtocolBuf[256], 
                                                       p_request_packet->data_field_len - 256 - space_num, 
                                                       1);  // 32位校验和
            }
            else
            {
                /* 计算 checksum */
                firmware_checksum_compute += CheckSum(_ProtocolBuf, p_request_packet->data_field_len - space_num, 1);
                last_num_index = num_index;
            }
            base_firmware_addr += p_request_packet->data_field_len  - space_num;
            pack_count++;
            break;
        }

        case UPGRADE_END:
        {
            _gstResponsePacketData.error_code = ACK_OK;
            if(firmware_checksum_compute != firmware_checksum_rec)
            {
                _gstResponsePacketData.error_code = ACK_ERROR;
            }
            else
            {
                uint8_t firmware_upgrade[256];
                memset(firmware_upgrade, 0xA0, 256);
                spi_flash_write_bytes_with_erase(UPGRADE_FLAG_ADDR, firmware_upgrade, 256);
                _gstResponsePacketData.error_code = ACK_OK;
                gBMSFirmwareUpgradeEnd = 1;
            }
            break;
        }
        default:
            break;
    }
    return true;
}

static bool _ProtoUnpackJudge(ComRevData_t *p)
{
    uint16_t checksum = 0;

    /* 解包 */
    memcpy(&_gstRequestPacketData, p->pData, 7);
    _gstRequestPacketData.data_field = _ProtocolBuf;
    _gstRequestPacketData.check_sum = (uint16_t)p->pData[p->data_size - 2] + (((uint16_t)p->pData[p->data_size - 1]) << 8);
    memcpy(_ProtocolBuf, (uint8_t*)(p->pData + 7), _gstRequestPacketData.data_field_len);

    checksum = CheckSum(p->pData, (p->data_size - 2), 0);
    if(_gstRequestPacketData.check_sum != checksum)
    {
        _gstResponsePacketData.error_code = ACK_ERROR;
    }
    else
    {
        _CmdAnalysis(&_gstRequestPacketData);
    }

    DataSendPolling();

    return true;
}

void ComDataProcess(void)
{
    static uint32_t boot_run_time = 0;
    static uint32_t boot_run_count = 0;
    ComRevData_t *pCom;

    if(QueueGet(&gComRevQueue, (void *)&pCom) == true)
    {
        if(pCom->data_type == COM_TYPE_UART_DATA)
        {
            /* 下位机串口通信协议解析 */
            _ProtoUnpackJudge(pCom);
            boot_run_count = 0;
        }
    }

    if(tim_difftime_from_now(boot_run_time) > 1000)
    {
        boot_run_time = tim_get_systime();

        if((_gstRequestPacketData.cmd == UPGRADE_START) || (_gstRequestPacketData.cmd == UPGRADE_BUSY) || (_gstRequestPacketData.cmd == UPGRADE_END))
        {
            boot_run_count = 0;
        }
        else
        {
            boot_run_count++;
        }
        if(boot_run_count > 1200)
        {
            BMSBoardPowerOff();
        }
    }
    if(gBMSFirmwareUpgradeEnd == 2)
    {
        rcu_all_reset_flag_clear();
        __set_PRIMASK(1);   // 关闭总中断
        NVIC_SystemReset();
    }
}