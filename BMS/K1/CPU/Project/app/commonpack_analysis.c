#include "commonpack_analysis.h"
#include "current_control.h"
#include "math.h"
#include "mid_shell.h"
#include "dev_led.h"
#include "bms_protocol.h"
#include "dev_spiflash.h"

#define LOG_TAG     "fwupgrade"

#define FIRST_PACK_INQUIRE_MODBUS_DATA_TIME         (800)

/*普通数据包串口与modbus串口公用，但普通数据包所使用的串口不存在主从的概念，
哪个串口收到上一级的数据包，哪个是与上一级相连的串口，另一个是与下一级相连的串口
*/
typedef enum
{
    EN_UART_CONNECT_DIR_UP     ,
    EN_UART_CONNECT_DIR_DOWN   ,
    EN_UART_CONNECT_DIR_NUM
} EN_UART_CONNECT_DIR;


/*普通数据包下发状态*/
typedef enum
{
    COMMON_PACK_REQUEST_STATUS_IDLE                                                 ,          /*空闲*/

    COMMON_PACK_REQUEST_STATUS_ALL_PACK_UPGRADE_PACK_DOWN_SEND                      ,          /*升级包下发*/
    COMMON_PACK_REQUEST_STATUS_ALL_PACK_MODBUS_PACK_UP_SEND                         ,          /*本机modbus数据上发*/

    COMMON_PACK_REQUEST_STATUS_FIRST_PACK_MODBUS_PACK_GET                           ,          /*modbus数据获取*/
    COMMON_PACK_REQUEST_STATUS_FIRST_PACK_WAIT_MODBUS_PACK_ACK                      ,          /*等待modbus数据获取应答*/
    COMMON_PACK_REQUEST_STATUS_FIRST_PACK_CC_CHANGE_CMD_DOWN_SEND                   ,          /*限流开启命令下发*/
    COMMON_PACK_REQUEST_STATUS_FIRST_PACK_WAIT_CC_ON_ACK                            ,          /*等待限流应答*/

    COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_MODBUS_ADDR_DOWN_SEND                    ,          /*modbus地址下发*/
    COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_WAIT_MODBUS_ADDR_ACK                     ,          /*等待modbus地址应答*/
    COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_MODBUS_PACK_GET_CMD_DOWN_SEND            ,          /*modbus数据获取命令下发*/
    COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_WAIT_MODBUS_PACK_ACK_AND_UP_SEND         ,          /*等待modbus数据应答并且上发*/
    COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_CC_CHANGE_CMD_DOWN_TRANSMIT              ,          /*限制命令向下转发*/
    COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_WAIT_CC_CHANGE_CMD_ACK_AND_UP_TRANSMIT   ,          /*等待限流命令并上发*/
    
    COMMON_PACK_REQUEST_STATUS_FINAL_PACK_CC_CHANGE_CMD_ACK                         ,          /*限制命令应答*/
    COMMON_PACK_REQUEST_STATUS_ENDURANCE_TEST_DATA_ACK                              ,          /*耐久度测试数据应答*/
    
    COMMON_PACK_REQUEST_STATUS_ALL_PACK_SHUT_DOWN_SEND                           ,          /* 关机命令下发 */
} EN_COMMON_PACK_REQUEST_STATUS;


/*固件升级状态*/
typedef enum
{
    EN_FIRMWARE_UPGRADE_STATUS_IDLE                            ,          /*空闲*/
    EN_FIRMWARE_UPGRADE_STATUS_SEND_START_PACK                 ,          /*发送起始包*/
    EN_FIRMWARE_UPGRADE_STATUS_SEND_DATA_PACK                  ,          /*发送数据包*/
    EN_FIRMWARE_UPGRADE_STATUS_SEND_END_PACK                   ,          /*发送停止包*/
    EN_FIRMWARE_UPGRADE_STATUS_WAIT_ACK                        ,          /*等待应答*/
    EN_FIRMWARE_UPGRADE_STATUS_ERROR                           ,          /*升级失败*/
    EN_FIRMWARE_UPGRADE_STATUS_TIMEOUT                         ,          /*升级超时*/
    EN_FIRMWARE_UPGRADE_STATUS_SUCCESS                         ,          /*升级成功*/
    EN_FIRMWARE_UPGRADE_STATUS_END                             ,          /*升级结束*/
} EN_FIRMWARE_UPGRADE_STATUS;

typedef struct
{
    uint32_t n_len;                             //子包数据长度    
    uint32_t n_offset;                          //当前地址偏移量
    uint32_t signal_pack_id;                    //单包流水号
    uint32_t fw_pack_size;                      //固件包总长度
    uint8_t  bms_fw_pack_space_size;            //固件子包拓展长度
    uint32_t disconnected_max_time;             //未接收到固件包的时间
    uint8_t  first_rev_end_flag;                //每次升级中第一次接收end标志
} BmsFirmwareInfo_t;

BmsFirmwareInfo_t  _gBmsFirmwareInfo;

Uart_HandleType*   _gUartComHandle[EN_UART_CONNECT_DIR_NUM]         = {&Uart1_ModS_Handle, &Uart2_ModS_Handle};

/*与上一环相连，主串口（modbus的从机串口）的接收请求与发送响应数据包*/
static protocal_request_packet_t     _UpRequestPacket               = {0};
static protocal_response_packet_t    _UpResponsePacket              = {0};

/*与下一环相连，副串口（modbus的主机串口）的接收响应与发送请求数据包*/
static protocal_request_packet_t     _DownRequestPacket             = {0};
static protocal_response_packet_t    _DownResponsePacket            = {0};

ProtocalReadResponsePacket_t         gstReadResponsePacketData      = {0};
ProtocalReadResponsePacket_t         _gstUpReadResponsePacketData   = {0};

static uint8_t   _ProtocolBuf[COM_RX_SIZE]                          = {0};

static uint8_t _upgrate_data[COM_RX_SIZE];                                              /*更新数据数组*/


uint8_t   garrComRxBuf[COM_RX_SIZE];
uint8_t   garrComTxBuf[EN_UART_CONNECT_DIR_NUM][COM_TX_SIZE];
uint8_t   gBMSHardVersion[3]   = {0};                           /*BMS硬件版本        */
uint32_t  gBMSHardwareLen;
uint8_t   gBMSFirmwareType[4]  = {0x0C, 0x0C, 0x0C, 0x03};      /*BMS升级固件头      */


uint8_t garrCcFlag[BMS_PACK_MAX_NUM + 1];                       /* 所有Pack的限流标志 */

Queue_t gComRevQueue;

static EN_COMMON_PACK_REQUEST_STATUS _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_IDLE;

/*****************函数*********************/
/*地址设置数据包下发*/
void _CommonPackDownSend(uint8_t n_send_addr, uint8_t n_recv_addr, uint16_t n_cmd, uint8_t* p_data, uint16_t data_field_len);
/*地址设置数据包应答上发*/
void _AddrSetAckUpSend(ModbusRTURevData_t *p_com_revdata, bool n_isFinalBMS);
/*固件包应答上发*/
void _FWPackAckUpSend(void);
/*数据包转发*/
void _CommonPackTransmit(Uart_HandleType* p_uarthandle, ModbusRTURevData_t *p_pack);
/*设置上下串口*/
void _UpUartSet(Uart_HandleType* p_uarthandle);
/*modbus寄存器数据上发*/
bool _ModbusRegUpSend(void);
/*modbus寄存器数据向上转发*/
void _CommonPackUpSend(uint8_t* p_data, uint16_t data_field_len);
/*发送限流开启命令函数*/
void _CurrentControlCmd(DevUartPackInfo_t *p_modbus_info, uint8_t n_target_bms_addr);
/*限流应答函数*/
bool _AckUpSend(uint8_t n_cmd, uint8_t n_error_code);

bool _ExternalFlashFirmwareFlagAnalysis(void);
static void _BmsPrepareSend(void);//准备发送
static void _BmsDataSend(uint16_t size, uint32_t offset);//数据发送中
static void _BmsEndSend(void);//数据发送结束
static bool _RevUpdataAckAnalysis(uint8_t *p, uint16_t n_need_ack);
uint8_t UpgrateFirmwareProcess(void);
/* 所有pack的模式判断 */
void _AllPackCcModeJudge(uint8_t* p_last_cc_flag, uint8_t* p_cc_flag, EN_PACK_CURRENT_MODE* p_current_mode);
/* 所有pack满充空放时MOS及CC控制 */
void _AllPackFullOrEmptyPackCcAndMosProcess(uint8_t* p_last_cc_flag, uint8_t* p_cc_flag, DevUartPackInfo_t *p_modbus_info);

static bool _VerifyBmsFirmware(uint32_t bms_firmware_checksum)
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
        checksum += CheckSumFun(rdata, BLOCK_SIZE, 1);
        bms_firmware_addr += BLOCK_SIZE;
    }

    spi_flash_read_bytes(bms_firmware_addr, rdata, block_num_behind);
    checksum += CheckSumFun(rdata, block_num_behind, 1);

    if(checksum != bms_firmware_checksum)
    {
        return false;
    }

    return true;
}


/**
* @brief  : 第一个Pack应答升级结束
 * @para   :
 * @note   : None
 * @return : 是否接收成功
 */
void _FirstPackUpEndAckProcess(uint8_t* up_flag)
{
    //如果不是第一个Pack或只有一个Pack，该函数不运行
    if((gUartPackInfo.ModbusAddr != MODBUS_ADDR_PACK0) && (gUartPackInfo.ModbusAddr != MODBUS_ADDR_DEFAULT_VALUE))
    {
        return;
    }
    //查看所有电池包是否升级完成
    for(int i = 0; i < gUartPackInfo.bms_num; i++)
    {
        if(gUartPackInfo.p_modbus_only_read_reg[i]->up_finish_flag == 0)
        {
            //有未升级完成的Pack，应答信号改为升级中
           *up_flag = ACK_UP_ING;
            return;
        }
        if(gUartPackInfo.p_modbus_only_read_reg[i]->up_finish_flag == 2)
        {
            //有升级失败的Pack，应答信号改为失败
           *up_flag = ACK_ERROR;
            return;
        }
    }
    //所有Pack均升级完成，应答信号改为升级完成
    *up_flag = ACK_OK;
}

/**
 * @brief  : 升级命令解析，进行固件升级
 * @para   : p_request_packet : 固件包
 * @note   : None
 * @return : None
 */
static bool _CmdAnalysis(protocal_request_packet_t *p_request_packet)
{

    static uint32_t bms_firmware_addr;
    static uint16_t bms_fw_pack_id;                       //固件包流水号
    static uint32_t bms_fw_rec_sumcheck;                  //BMS升级固件和校验(接收)
    static uint32_t bms_fw_rec_old_signal_pack_id;        //上一包单包pack的id号
    static uint32_t bms_fw_rec_old_signal_pack_sumcheck;  //上一包单包pack的固件和校验
    static uint32_t bms_fw_all_pack_sumcheck;             //总包pack的固件和校验
    
    uint16_t        cmd                  = p_request_packet->cmd;

    _UpResponsePacket.error_code         = ACK_OK;
    gstReadResponsePacketData.error_code = ACK_OK;

    switch(cmd)
    {
        case UPGRADE_START:
        {
            /* 初始化程序存放地址 */
            bms_firmware_addr = BMS_FIRMWARE_ADDR;
            bms_fw_pack_id = 0;
            _gBmsFirmwareInfo.bms_fw_pack_space_size = p_request_packet->data_field[4];       //拓展长度
            if(_gBmsFirmwareInfo.bms_fw_pack_space_size > 20)
            {   //拓展字节数最大为20个
                _UpResponsePacket.error_code = 0X01;/*ACK_ERROR*/
                gUartPackInfo.p_modbus_only_read_reg[0]->up_finish_flag = 2;
            }
            else
            {
                bms_fw_all_pack_sumcheck = 0;
                gUartPackInfo.p_modbus_only_read_reg[0]->up_finish_flag = 0;
                gPackStatus.ocv_process_flag = 0;
                _gBMSSysInfo.bms_upgrade_ing_flag = true;
                _gBmsFirmwareInfo.first_rev_end_flag = 0;       //清除
                /* 升级期间暂停AFE采集 */
//                SoxInfoSave(&gPackStatus);
                SoxInfoSaveInUpgrade(&gPackStatus);
                
                LedCtlBlink(LED_SRC_SYS, LED_BLINK_TIME_UPGRAD);
//                LOG_DEBUG(LOG_TAG, "FW Rev Start");
                //刷新超时时间
                _gBmsFirmwareInfo.disconnected_max_time = tim_get_systime();
            }
            
            break;
        }
        case UPGRADE_BUSY:
        {
            //接收流水号
            bms_fw_pack_id = (uint32_t)p_request_packet->data_field[1] << 8 | p_request_packet->data_field[0];
            uint16_t bms_fw_pack_size = p_request_packet->data_field_len - _gBmsFirmwareInfo.bms_fw_pack_space_size;
            /* 第一包数据 */
            if(bms_firmware_addr == BMS_FIRMWARE_ADDR)
            {
                if(memcmp(&p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size], gBMSFirmwareType, 4) == 0)
                {
                    bms_firmware_addr = BMS_FIRMWARE_ADDR;
                }
                else
                {
                    return false;
                }
                /* 固件版本 */
                memcpy(gBMSHardVersion, &p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size + 4], 3);
                /* 固件长度 */
//                gBMSHardwareLen = *((uint32_t*)&p_request_packet->data_field[bms_fw_pack_space_size + 7]);
                _gBmsFirmwareInfo.fw_pack_size = ((uint32_t)p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size + 7]       ) |
                                                 ((uint32_t)p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size + 8]  << 8 ) |
                                                 ((uint32_t)p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size + 9]  << 16) |
                                                 ((uint32_t)p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size + 10] << 24) ;
                                                           
                /* 固件校验和 */
//                bms_fw_rec_sumcheck = *((uint32_t*)&p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size + 256 - 4]);
                bms_fw_rec_sumcheck = ((uint32_t)p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size + 256 - 4]      ) | 
                                      ((uint32_t)p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size + 256 - 3] << 8 ) | 
                                      ((uint32_t)p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size + 256 - 2] << 16) | 
                                      ((uint32_t)p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size + 256 - 1] << 24) ;
            }

            spi_flash_write_bytes_with_erase(bms_firmware_addr, &p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size], bms_fw_pack_size);
            spi_flash_read_bytes(bms_firmware_addr, _ProtocolBuf, bms_fw_pack_size);

            if(bms_firmware_addr == BMS_FIRMWARE_ADDR)
            {
                /* 计算 checksum */
                
            }
            else
            {
                //如果连续发两包相同的流水号，则减去旧的和校验，加上新的和校验
                if(bms_fw_rec_old_signal_pack_id == bms_fw_pack_id)
                {
                    bms_fw_all_pack_sumcheck -= bms_fw_rec_old_signal_pack_sumcheck;
                }
                /* 计算 checksum */
                bms_fw_rec_old_signal_pack_sumcheck = CheckSumFun(_ProtocolBuf, bms_fw_pack_size, 1);
                bms_fw_all_pack_sumcheck += bms_fw_rec_old_signal_pack_sumcheck;
                
            }
            
            //地址偏移(第1包数据长度固定为256字节，后续固件包长度为UPGRADE_DATA_PACK_SIZE)
            bms_firmware_addr =  BMS_FIRMWARE_ADDR + 256 + (bms_fw_pack_id * UPGRADE_DATA_PACK_SIZE);
            bms_fw_rec_old_signal_pack_id = bms_fw_pack_id;
//            LOG_DEBUG(LOG_TAG, "Rev Pack ID : %d", bms_fw_pack_id);
            //刷新超时时间
            _gBmsFirmwareInfo.disconnected_max_time = tim_get_systime();
            break;
        }
        case UPGRADE_END:
        {
            //和校验从第0包开始，所以bms_fw_pack_id实际为(pack_num + 1)
            if(bms_fw_all_pack_sumcheck != bms_fw_rec_sumcheck)
            {
                _UpResponsePacket.error_code = 0X01;/*ACK_ERROR*/
                gUartPackInfo.p_modbus_only_read_reg[0]->up_finish_flag = 2;
//                LOG_INFO(LOG_TAG, "bms_firmware_checksum error!!!  0X%x -- 0X%x", bms_fw_rec_sumcheck, bms_fw_all_pack_sumcheck);
            }
            else
            {
                uint8_t firmware_upgrade[256];
                if(_gBmsFirmwareInfo.first_rev_end_flag == 0)
                {
                    memset(firmware_upgrade, 0xA0, 256);
                    spi_flash_write_bytes_with_erase(UPGRADE_FLAG_ADDR, firmware_upgrade, 256);
                    _gBmsFirmwareInfo.first_rev_end_flag = 1;
                }
                
                //检测所有Pack是否升级完毕，全部升级完毕后应答成功，否则应答升级中
                _FirstPackUpEndAckProcess(&_UpResponsePacket.error_code);
                
                if(gUartPackInfo.p_modbus_only_read_reg[0]->up_finish_flag == 0)
                {
                    gUartPackInfo.p_modbus_only_read_reg[0]->up_finish_flag = 1;
//                    SoxInfoLoad(&gPackStatus);
                    SoxInfoLoadInUpgrade(&gPackStatus);
                    SoxInfoLoadInUpgrade(&gPackStatus_up);
                    //准备下发固件包
                    _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_ALL_PACK_UPGRADE_PACK_DOWN_SEND;
                }
            }
            //升级结束继续AFE采集
            _gBMSSysInfo.bms_upgrade_ing_flag = false;
            LedCtlBlink(LED_SRC_SYS, LED_BLINK_TIME_NORMAL);
            break;
        }
        /*设置Modbus_Addr*/
        case MODBUS_ADDR_SET:
        {
            //设置modbus地址
            gUartPackInfo.ModbusAddr = p_request_packet->data_field[0];
            //将接收到此指令的串口设置为从机
            ModbusHostOrSlaveSet(_gUartComHandle[EN_UART_CONNECT_DIR_UP], EN_MODBUS_HOST_SLAVE_TYPE_SLAVE);
            //除第一个Pack外，后续Pack不允许打开强充
            if(gUartPackInfo.ModbusAddr != MODBUS_ADDR_PACK0)
            {
                _gBMSSysInfo.force_chg_allow_flag = 2;
            }
            //准备下发
            _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_MODBUS_ADDR_DOWN_SEND;
            
            break;
        }
        /*modbus数据包获取*/
        case MODBUS_PACK_GET:
        {
            if(p_request_packet->data_field[0] == gUartPackInfo.ModbusAddr)
            {
                //是本机的modbus地址，上传本机数据
                _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_ALL_PACK_MODBUS_PACK_UP_SEND;

            }
            else
            {
//                _UpResponsePacket.error_code = 0X01;
                //不是本机的modbus地址，下发
                _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_MODBUS_PACK_GET_CMD_DOWN_SEND;
            }
            break;
        }
        /*限流设置*/
        case CC_ON_CMD_SEND:
        {
            if(p_request_packet->data_field[0] == gUartPackInfo.ModbusAddr)
            {
                //是本机的modbus地址，进行动作并应答
                //接收所有的限流标志
                memcpy(garrCcFlag, p_request_packet->data_field, (BMS_PACK_MAX_NUM + 1));
                //接收自己的限流标志
                gPackStatus.cc_mode = (EN_CC_MODE)(p_request_packet->data_field[1 + gUartPackInfo.ModbusAddr - MODBUS_ADDR_PACK0]);
                CcCmdRevProcess(gPackStatus.cc_mode);
                
                if(gUartPackInfo.is_final_bms == false)
                {
                    //不是最后一个BMS，限流下发
                    _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_CC_CHANGE_CMD_DOWN_TRANSMIT;
                }
                else
                {
                    //是最后一个BMS，应答
                    _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_FINAL_PACK_CC_CHANGE_CMD_ACK;
                }

            }
            else
            {
                _UpResponsePacket.error_code = ACK_ERROR;/*ACK_ERROR*/ 
            }
            break;
        }
        /* 获取耐久度测试数据 */
        case ENDURANCE_TEST_DATA_GET:
        {
            _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_ENDURANCE_TEST_DATA_ACK;
            break;
        }
        /* 耐久度测试清除 */
        case ENDURANCE_TEST_DATA_CLEAR:
        {
            //清除
            spi_flash_auto_erase_sector(ENDURANCE_TEST_ERROR_INFO_FLASH_ADDR          , 0X8000);
            spi_flash_auto_erase_sector(ENDURANCE_TEST_ERROR_INFO_FLASH_ADDR + 0X8000 , 0X8000);
            spi_flash_auto_erase_sector(ENDURANCE_TEST_ERROR_INFO_FLASH_ADDR + 0X10000, 0X8000);
            spi_flash_auto_erase_sector(ENDURANCE_TEST_ERROR_INFO_FLASH_ADDR + 0X18000, 0X8000);
            break;
        }
        /* 串口方向定向 */
        case UART_DIR_CALI_CMD:
        {
            //将接收到此指令的串口设置为从机
//            LOG_DEBUG(LOG_TAG, "--S Rev Dir Cmd--\n\r");
            ModbusHostOrSlaveSet(_gUartComHandle[EN_UART_CONNECT_DIR_UP], EN_MODBUS_HOST_SLAVE_TYPE_SLAVE);
            break;
        }
        /* 关机指令 */
        case SHUTDOWN_CMD:
        {
            //接收到关机指令
            _gBMSSysInfo.bms_shutdown &= 0X00FF;
            _gBMSSysInfo.bms_shutdown |= (MONITOR_OFF_CMD << 8);
            _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_ALL_PACK_SHUT_DOWN_SEND;
            break;
        }
        default:
        {
            _UpResponsePacket.error_code = ACK_ERROR;/*ACK_ERROR*/
        }
            break;
    }

    return true;

}

/**
* @brief  : 处理与上一级相连的数据包，特定协议数据包处理函数（固件升级、历史电量上报、Modbus_Addr地址设置）
 * @para   :
 * @note   : None
 * @return : 是否接收成功
 */
bool UpCommonPackProcess()
{
    static ModbusRTURevData_t   *p_com_revdata = NULL;
    static uint8_t  n_ack_send_flag                 =   0;
    static uint32_t n_ack_send_wait_time            =   0;
    uint16_t    checksum                            =   0 ;

    //AFE采样停止状态下10s内没有接收到固件包
    if((_gBMSSysInfo.bms_upgrade_ing_flag == true) && (tim_difftime_from_now(_gBmsFirmwareInfo.disconnected_max_time) > 10000))
    {
        //升级结中断继续AFE采集
        _gBMSSysInfo.bms_upgrade_ing_flag = false;
    }
    
    if((tim_difftime_from_now(n_ack_send_wait_time) > 100) && (n_ack_send_flag == 1))
    {
        n_ack_send_flag = 0;
        n_ack_send_wait_time = tim_get_systime();
        /*升级包应答*/
        _FWPackAckUpSend();
    }

    if(QueueGet(&gUpCommonPackQueue, (void *)&p_com_revdata) == false)
    {
        return false;
    }

    /* 解包 */
    memcpy(&_UpRequestPacket, p_com_revdata->pData, 7);
    _UpRequestPacket.data_field = _ProtocolBuf;

    /*和校验*/
    _UpRequestPacket.check_sum = (uint16_t)p_com_revdata->pData[p_com_revdata->data_size - 2] + (((uint16_t)p_com_revdata->pData[p_com_revdata->data_size - 1]) << 8);
    
    if(_UpRequestPacket.data_field_len < COM_RX_SIZE)
    {
        memcpy(_ProtocolBuf, (uint8_t*)(p_com_revdata->pData + 7), _UpRequestPacket.data_field_len);
    }
    checksum = CheckSumFun(p_com_revdata->pData, (p_com_revdata->data_size - 2), 0);

    if(_UpRequestPacket.check_sum != checksum)
    {
        _UpResponsePacket.error_code = ACK_ERROR;
    }
    else
    {
        /*确定上下串口*/
        _UpUartSet(p_com_revdata->p_uarthandle);
        //通讯成功标志设置
        gUartPackInfo.com_success_flag = 1;
        /*固件包处理*/
        _CmdAnalysis(&_UpRequestPacket);
    }

    if((_UpRequestPacket.cmd == UPGRADE_START) || (_UpRequestPacket.cmd == UPGRADE_BUSY) || (_UpRequestPacket.cmd == UPGRADE_END))
    {
        /*升级包应答*/
        n_ack_send_flag = 1;
        n_ack_send_wait_time = tim_get_systime();
    }

    return true;
}


/**
 * @brief  : 处理与下一级相连的数据包，特定协议数据包处理函数（固件升级、历史电量上报、Modbus_Addr地址设置）
 * @para   :
 * @note   : None
 * @return : None
 */

void DownCommonPackProcess()
{
    static EN_COMMON_PACK_REQUEST_STATUS _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;

    static uint32_t modbus_addr_response_time         = 0     ;  /*modbus地址设置等待回应时间*/
    static uint32_t upgrade_wait_ack_time             = 0     ;  /*升级应答等待时间*/
    static uint32_t modbus_data_inquire_time          = 0     ;  /*modbus数据查询时间*/
    static uint32_t modbus_data_get_wait_ack_time     = 0     ;  /*modbus数据获取时间*/
    static uint16_t rev_error_count[BMS_PACK_MAX_NUM]         ;  /*接收错误应答及超时次数*/
    
    static uint32_t endurance_test_data_inquire_time  = 0     ;  /*耐久度测试数据查询时间*/
    static uint8_t modbus_data_read_addr              = MODBUS_ADDR_PACK0  ;
    uint8_t modbus_next_addr                          = 0     ;

    ModbusRTURevData_t *p_com_revdata                   = NULL  ;
    static ModbusRTURevData_t default_rev_data                ; /* 默认数据地址，防止出现空指针 */

    switch(_down_common_pack_request_status)
    {

        case COMMON_PACK_REQUEST_STATUS_IDLE:
        {

            /*正在执行的标志与从上一级接收的标志隔离开，防止出现打架的情况*/

            _down_common_pack_request_status = _geCommonPackRequestStatus;

            //第一个电池包，已建立通讯链路,通讯正常，且在非复位状态下才能进行数据查询及限流设置
            if((gUartPackInfo.ModbusAddr == MODBUS_ADDR_PACK0) &&(gUartPackInfo.com_success_flag == 1) && 
                (gUartPackInfo.com_out_flag == 0)&& (NOVA_GET_BIT(garrModbusWriteReadReg[0].sys_up_status, 1) == 0))
            {
                if(gUartPackInfo.bms_num > 1)    /*第一个电池包,且不止一个电池包*/
                {
                    /*定时查询后续电池包数据*/
                    /*只有在完成上一级的请求后才会查询*/
                    if((tim_difftime_from_now(modbus_data_inquire_time) > FIRST_PACK_INQUIRE_MODBUS_DATA_TIME) && _geCommonPackRequestStatus == COMMON_PACK_REQUEST_STATUS_IDLE &&\
                        gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] == true)
                    {
                        /*串口忙碌标志置位*/
                        gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = false;
                        /*获取系统时间*/
                        modbus_data_inquire_time  = tim_get_systime();
                        _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_FIRST_PACK_MODBUS_PACK_GET;
                    }
                    
                }
                else if((_gBMSSysInfo.bms_run_status >= EN_BMS_RUNSTATE_NORMAL_PRE_PROCESS) && \
                        (gUartPackInfo.bms_num == 1) && \
                        _geCommonPackRequestStatus == COMMON_PACK_REQUEST_STATUS_IDLE)   
                {
                    //第一个电池包，只有一个电池包，bms已进入正常运行模式，且在没有处理其他功能的情况下才能进行自我限流
                    _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_FIRST_PACK_CC_CHANGE_CMD_DOWN_SEND;
                }
            }

            _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_IDLE;
            break;
        }

        /*固件包向下转发*/
        case COMMON_PACK_REQUEST_STATUS_ALL_PACK_UPGRADE_PACK_DOWN_SEND:
        {
            gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = false;
            /*固件下发*/
            if(UpgrateFirmwareProcess() != 0)
            {
                /*更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况*/
                if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
                {
                    modbus_data_inquire_time  = tim_get_systime() - 10;
                }
                gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
            }
            break;
        }
        /*********************************地址设置指令转发及应答**********************************/
        /*向下转发地址写请求包*/
        case COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_MODBUS_ADDR_DOWN_SEND:
        {
            //非工装测试模式，向下发送，工装模式下直接发送本机地址
            if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl != 1)
            {
                //第一个Pack立即回复地址设置成功指令
                if(gUartPackInfo.ModbusAddr == MODBUS_ADDR_PACK0)
                {
                    gstReadResponsePacketData.data_field_len = 1;
                    gstReadResponsePacketData.error_code = ACK_OK;
                    memcpy((garrComTxBuf[EN_UART_CONNECT_DIR_UP] + sizeof(ProtocalReadResponsePacket_t)), &gUartPackInfo.ModbusAddr, gstReadResponsePacketData.data_field_len);
                    //上发
                    p_com_revdata = &default_rev_data;
                    _AddrSetAckUpSend(p_com_revdata, gUartPackInfo.is_final_bms);
                }
                //modbus地址+1下发
                modbus_next_addr = gUartPackInfo.ModbusAddr + 1;
                _CommonPackDownSend(_UpRequestPacket.send_addr, _UpRequestPacket.recv_addr, _UpRequestPacket.cmd, &modbus_next_addr, sizeof(modbus_next_addr));
            }
            //获取系统时间
            modbus_addr_response_time = tim_get_systime();
            _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_WAIT_MODBUS_ADDR_ACK;

            break;
        }
        /*等待ACK*/
        case COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_WAIT_MODBUS_ADDR_ACK:
            //等待应答
            if(tim_get_systime() - modbus_addr_response_time < SetMaxWaitTime(gUartPackInfo.ModbusAddr))
            {
                if(QueueGet(&gDownCommonPackQueue, (void *)&p_com_revdata) == false)
                {
                    return;
                }

                //在规定时间内接收到下一级回应的地址包，将下一级回应的地址反馈到上一级
                gUartPackInfo.is_final_bms = false;
                //接收最后一个电池包的地址
                gUartPackInfo.final_bms_addr = p_com_revdata->pData[sizeof(ProtocalReadResponsePacket_t)];
                //回包赋值
                gstReadResponsePacketData.error_code = ACK_OK;
                //计算bms总数量
                gUartPackInfo.bms_num = gUartPackInfo.final_bms_addr - MODBUS_ADDR_PACK0 + 1;
                
                //第一个Pack不向监控卡应答，其余Pack向上一级BMS应答
                if(gUartPackInfo.ModbusAddr != MODBUS_ADDR_PACK0)
                {
                    //上发
                    _AddrSetAckUpSend(p_com_revdata, gUartPackInfo.is_final_bms);
                }
                
                //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
                if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
                {
                    modbus_data_inquire_time  = tim_get_systime() - 10;
                }
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
            }
            else
            {
                gUartPackInfo.is_final_bms = true;
                gUartPackInfo.final_bms_addr = gUartPackInfo.ModbusAddr;

                //超时，向上发送自己的地址
                gstReadResponsePacketData.data_field_len = 1;
                gstReadResponsePacketData.error_code = ACK_OK;
                memcpy((garrComTxBuf[EN_UART_CONNECT_DIR_UP] + sizeof(ProtocalReadResponsePacket_t)), &gUartPackInfo.ModbusAddr, gstReadResponsePacketData.data_field_len);
                
                if(gUartPackInfo.ModbusAddr != MODBUS_ADDR_PACK0)
                {
                    //上发
                    p_com_revdata = &default_rev_data;
                    _AddrSetAckUpSend(p_com_revdata, gUartPackInfo.is_final_bms);
                }

                //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
                if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
                {
                    modbus_data_inquire_time  = tim_get_systime() - 10;
                }
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
            }
            break;
        /*********************************MODBUS数据获取，限流设置状态机**********************************/
        /*modbus数据获取，只有第一个电池包会进行这一步*/
        case COMMON_PACK_REQUEST_STATUS_FIRST_PACK_MODBUS_PACK_GET:
        {
            //设置查询地址
            modbus_data_read_addr    += 1;
            if(modbus_data_read_addr > gUartPackInfo.final_bms_addr)
            {
                modbus_data_read_addr = gUartPackInfo.ModbusAddr + 1;
            }
            //请求包下发
            _CommonPackDownSend(MONITORING_CARD_ID, BMS_ID, MODBUS_PACK_GET, &modbus_data_read_addr, 1);
            _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_FIRST_PACK_WAIT_MODBUS_PACK_ACK;
            //获取系统时间
            modbus_data_get_wait_ack_time = tim_get_systime();
            break;
        }

        /*等待modbus数据获取应答*/
        case COMMON_PACK_REQUEST_STATUS_FIRST_PACK_WAIT_MODBUS_PACK_ACK:
        {
            if(tim_get_systime() - modbus_data_get_wait_ack_time > SetMaxWaitTime(gUartPackInfo.ModbusAddr))
            {
                //记录通讯故障次数
                rev_error_count[modbus_data_read_addr - MODBUS_ADDR_PACK0]++;
                if(rev_error_count[modbus_data_read_addr - MODBUS_ADDR_PACK0] > 10)
                {
                    NOVA_SET_BIT(gEmsErrorInfo.value, (modbus_data_read_addr - MODBUS_ADDR_PACK0));
                }
                
                //等待超时，数据异常待处理
                //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
                if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
                {
                    modbus_data_inquire_time  = tim_get_systime() - 10;
                }
                gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
                return;
            }

            if(QueueGet(&gDownCommonPackQueue, (void *)&p_com_revdata) == false)
            {
                return;
            }

            //p_com_revdata->pData此时的结构：
            //[ModbusAddr, ModbusOnlyReadReg, ModbusWriteReadReg]

            //判断寄存器所属的BMS
            uint8_t bms_addr = p_com_revdata->pData[sizeof(ProtocalReadResponsePacket_t)];
            //根据地址存放到对应的寄存器地址中
            memcpy(gUartPackInfo.p_modbus_only_read_reg[bms_addr - MODBUS_ADDR_PACK0] , &p_com_revdata->pData[sizeof(ProtocalReadResponsePacket_t) + 1]                                      , gUartPackInfo.modbus_only_read_reg_size );
            memcpy(gUartPackInfo.p_modbus_write_read_reg[bms_addr - MODBUS_ADDR_PACK0], &p_com_revdata->pData[sizeof(ProtocalReadResponsePacket_t) + 1 + gUartPackInfo.modbus_only_read_reg_size], gUartPackInfo.modbus_write_read_reg_size);

            //接收数据正确，刷新通讯故障次数
            rev_error_count[modbus_data_read_addr - MODBUS_ADDR_PACK0] = 0;
            
            //收集完所有Pack的数据包后发送一次限流指令
            if(bms_addr == gUartPackInfo.final_bms_addr)
            {
                modbus_data_get_wait_ack_time = tim_get_systime();
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_FIRST_PACK_CC_CHANGE_CMD_DOWN_SEND;
            }
            else
            {
                //等待超时，数据异常待处理
                //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
                if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
                {
                    modbus_data_inquire_time  = tim_get_systime() - 10;
                }
                gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
            }
            
            break;
        }
        /*限流开启命令下发*/
        case COMMON_PACK_REQUEST_STATUS_FIRST_PACK_CC_CHANGE_CMD_DOWN_SEND:
        {
            if(tim_difftime_from_now(modbus_data_get_wait_ack_time) < 10)
            {
                return ;
            }
            //根据电压处理限流命令，向下一级BMS发送
            _CurrentControlCmd(&gUartPackInfo, (gUartPackInfo.ModbusAddr + 1));
            //只有一个Pack时不会向下发送限流Cmd，只会处理自己的限流Cmd
            if(gUartPackInfo.bms_num == 1)
            {
                //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
                if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
                {
                    modbus_data_inquire_time  = tim_get_systime() - 10;
                }
                gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
            }
            else
            {
                //获取系统时间
                modbus_data_get_wait_ack_time = tim_get_systime();
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_FIRST_PACK_WAIT_CC_ON_ACK;
            }
            break;
        }

        /*等待限流命令应答*/
        case COMMON_PACK_REQUEST_STATUS_FIRST_PACK_WAIT_CC_ON_ACK:
        {
            if(tim_get_systime() - modbus_data_get_wait_ack_time > SetMaxWaitTime(gUartPackInfo.ModbusAddr))
            {
                //等待超时，数据异常待处理
                //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
                if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
                {
                    modbus_data_inquire_time  = tim_get_systime() - 10;
                }
                gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
                return;
            }
            
            if(QueueGet(&gDownCommonPackQueue, (void *)&p_com_revdata) == false)
            {
                return;
            }
            //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
            if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
            {
                modbus_data_inquire_time  = tim_get_systime() - 10;
            }
            gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
            _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;

            break;
        }
        /*modbus数据包上发*/
        case COMMON_PACK_REQUEST_STATUS_ALL_PACK_MODBUS_PACK_UP_SEND:
        {
            
            if(_ModbusRegUpSend() == true)
            {
                //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
                if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
                {
                    modbus_data_inquire_time  = tim_get_systime() - 10;
                }
                gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
            }
            break;
        }

        /*获取modbus数据包命令向下转发*/
        case COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_MODBUS_PACK_GET_CMD_DOWN_SEND:
        {
            if(gUartPackInfo.is_final_bms == true)
            {
                //最后一个pack不再继续向下发送
                //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
                if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
                {
                    modbus_data_inquire_time  = tim_get_systime() - 10;
                }
                gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
                return;
            }
            //命令下发
            uint8_t target_bms_addr = _UpRequestPacket.data_field[0];
            _CommonPackDownSend(_UpRequestPacket.send_addr, _UpRequestPacket.recv_addr, _UpRequestPacket.cmd, &target_bms_addr , 1);

            modbus_data_get_wait_ack_time = tim_get_systime();

            _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_WAIT_MODBUS_PACK_ACK_AND_UP_SEND;
            break;
        }

        /*等待modbus数据应答向并且上发*/
        case COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_WAIT_MODBUS_PACK_ACK_AND_UP_SEND:
        {
            if(tim_get_systime() - modbus_data_get_wait_ack_time > SetMaxWaitTime(gUartPackInfo.ModbusAddr))
            {
                //等待超时，数据异常待处理
                //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
                if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
                {
                    modbus_data_inquire_time  = tim_get_systime() - 10;
                }
                gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
                return;
            }

            if(QueueGet(&gDownCommonPackQueue, (void *)&p_com_revdata) == false)
            {
                return;
            }
            
            if(p_com_revdata->pData[0] != COMMON_PACK_RESPONSE_PACKET_HEAD)
            {
                return;
            }
            
            //上发
            _CommonPackUpSend(p_com_revdata->pData, p_com_revdata->data_size);

            //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
            if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
            {
                modbus_data_inquire_time  = tim_get_systime() - 10;
            }
            gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
            _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
            break;
        }

        /*限流命令应答*/
        case COMMON_PACK_REQUEST_STATUS_FINAL_PACK_CC_CHANGE_CMD_ACK:
        {
            static bool sending_flag = false;
            static uint32_t wait_time = 0;   
            
            if(sending_flag == false)
            {
                wait_time = tim_get_systime();
                sending_flag = true;
            }
            
            if(tim_difftime_from_now(wait_time) < 10)
            {
                return;
            }
            _AckUpSend(CC_ON_CMD_SEND, ACK_OK);
            //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
            if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
            {
                modbus_data_inquire_time  = tim_get_systime() - 10;
            }
            gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
            _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
            
            break;
        }
        /*限流命令向下转发*/
        case COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_CC_CHANGE_CMD_DOWN_TRANSMIT:
        {
            //命令下发     
            garrCcFlag[0] += 1;
            _CommonPackDownSend(_DownRequestPacket.send_addr, _UpRequestPacket.recv_addr, CC_ON_CMD_SEND, garrCcFlag , (BMS_PACK_MAX_NUM + 1));
            modbus_data_get_wait_ack_time = tim_get_systime();

            _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_WAIT_CC_CHANGE_CMD_ACK_AND_UP_TRANSMIT;
            break;
        }
        /*等待限流应答并向上转发*/
        case COMMON_PACK_REQUEST_STATUS_MIDDLE_PACK_WAIT_CC_CHANGE_CMD_ACK_AND_UP_TRANSMIT:
        {
            if(tim_get_systime() - modbus_data_get_wait_ack_time > SetMaxWaitTime(gUartPackInfo.ModbusAddr))
            {
                _AckUpSend(CC_ON_CMD_SEND, ACK_ERROR);
                //等待超时，数据异常待处理
                //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
                if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
                {
                    modbus_data_inquire_time  = tim_get_systime() - 10;
                }
                gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
                return;
            }

            if(QueueGet(&gDownCommonPackQueue, (void *)&p_com_revdata) == false)
            {
                return;
            }

            //上发
            _CommonPackUpSend(p_com_revdata->pData, p_com_revdata->data_size);

            //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
            if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
            {
                modbus_data_inquire_time  = tim_get_systime() - 10;
            }
            gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
            _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
            break;
        }
        /* 应答耐久度测试数据 */
        case COMMON_PACK_REQUEST_STATUS_ENDURANCE_TEST_DATA_ACK:
        {
            if(tim_difftime_from_now(endurance_test_data_inquire_time) < 50)
            {
                return;
            }
            
            static uint8_t data_buff[256];
            static uint32_t read_addr = ENDURANCE_TEST_ERROR_INFO_FLASH_ADDR;
            
            memset(data_buff, 0, 256);
            //读取保存的数据
            spi_flash_read_bytes(read_addr, data_buff, 256);
            
            //data_buff前三个字节全为FF，判断所有数据已读完，后续地址无数据，返回IDLE
            if((data_buff[0] == 0XFF) && (data_buff[1] == 0XFF) && (data_buff[2] == 0XFF))
            {
                read_addr = ENDURANCE_TEST_ERROR_INFO_FLASH_ADDR;
                //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
                if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
                {
                    modbus_data_inquire_time  = tim_get_systime() - 10;
                }
                gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
                _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
            }
            else
            {
                //地址偏移
                read_addr += 256;
                //不改变状态机状态，再次发送，直至所有数据发送完毕
            }  
            
            //向外发送
            _CommonPackUpSend(data_buff,256);
            //设置发送间隔
            endurance_test_data_inquire_time  = tim_get_systime();
            break;
        }
        
        /* 关机命令下发 */
        case COMMON_PACK_REQUEST_STATUS_ALL_PACK_SHUT_DOWN_SEND:
        {
            gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = false;
            //非最后一个Pack，命令向下转发   
            if(gUartPackInfo.is_final_bms != true)
            {
                uint16_t shutdown_status = (uint16_t)MONITOR_OFF_CMD << 8;
                _CommonPackDownSend(MONITORING_CARD_ID, BMS_ID, SHUTDOWN_CMD, (uint8_t*)&shutdown_status, 0);
            }                
            //更新modbus数据查询时间， 如果剩余查询时间过短，则更新查询时间为10ms，防止出现数据区域数据被覆盖的情况
            if(tim_difftime_from_now(modbus_data_inquire_time) > (FIRST_PACK_INQUIRE_MODBUS_DATA_TIME - 10))
            {
                modbus_data_inquire_time  = tim_get_systime() - 10;
            }
            gUartPackInfo.send_and_ack_complete_flag[EN_UART_CONNECT_DIR_DOWN] = true;
            _down_common_pack_request_status = COMMON_PACK_REQUEST_STATUS_IDLE;
            break;
        }
        default :
            break;
    }
}

/**
 * @brief  : 数据包下发
 * @para   :
 * @note   : None
 * @return : None
 */
void _CommonPackDownSend(uint8_t n_send_addr, uint8_t n_recv_addr, uint16_t n_cmd, uint8_t* p_data, uint16_t data_field_len)
{
    if(data_field_len > COM_RX_SIZE)
    {
        return;
    }
    
    uint16_t n_check_sum = 0;

    /* 组包 */
    _DownRequestPacket.packet_head    = COMMON_PACK_REQUST_PACKET_HEAD;
    _DownRequestPacket.send_addr      = n_send_addr;   /*地址还是上一级发送的地址，永远是监控卡*/
    _DownRequestPacket.recv_addr      = n_recv_addr;
    _DownRequestPacket.cmd            = n_cmd;
    _DownRequestPacket.data_field_len = data_field_len;
    _DownRequestPacket.data_field     = p_data;

    /*数据头放入发送区*/
    memcpy((uint8_t*)&garrComTxBuf[EN_UART_CONNECT_DIR_DOWN], (uint8_t*)&_DownRequestPacket, WRITE_REQUEST_FIXED_HEAD_SIZE);
    /*数据放入*/
    memcpy((uint8_t*)&garrComTxBuf[EN_UART_CONNECT_DIR_DOWN] + WRITE_REQUEST_FIXED_HEAD_SIZE, p_data, data_field_len);
    /*计算和校验*/
    n_check_sum  = CheckSumFun(garrComTxBuf[EN_UART_CONNECT_DIR_DOWN], (WRITE_REQUEST_FIXED_HEAD_SIZE + data_field_len), 0);
    /*和校验放入发送区*/
    memcpy((uint8_t*)&garrComTxBuf[EN_UART_CONNECT_DIR_DOWN] + WRITE_REQUEST_FIXED_HEAD_SIZE + data_field_len, (uint8_t*)&n_check_sum, 2);
    /*发送*/
    _gUartComHandle[EN_UART_CONNECT_DIR_DOWN]->pTxBuf = (uint8_t*)&garrComTxBuf[EN_UART_CONNECT_DIR_DOWN];
    _gUartComHandle[EN_UART_CONNECT_DIR_DOWN]->tx_size = WRITE_REQUEST_FIXED_HEAD_SIZE + data_field_len + sizeof(n_check_sum);
    CommonPackSend(_gUartComHandle[EN_UART_CONNECT_DIR_DOWN]->pTxBuf, _gUartComHandle[EN_UART_CONNECT_DIR_DOWN]->tx_size, _gUartComHandle[EN_UART_CONNECT_DIR_DOWN]);
}

void _CommonPackUpSend(uint8_t* p_data, uint16_t data_field_len)
{
    if(data_field_len > COM_RX_SIZE)
    {
        return;
    }
    /* 组包 */
    memcpy((uint8_t*)&garrComTxBuf[EN_UART_CONNECT_DIR_UP], p_data, data_field_len);
    /*发送*/
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]->pTxBuf = (uint8_t*)&garrComTxBuf[EN_UART_CONNECT_DIR_UP];
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]->tx_size = data_field_len;
    CommonPackSend(_gUartComHandle[EN_UART_CONNECT_DIR_UP]->pTxBuf, _gUartComHandle[EN_UART_CONNECT_DIR_UP]->tx_size, _gUartComHandle[EN_UART_CONNECT_DIR_UP]);
}

void _UpgradePackDownSend(uint8_t* p_data, uint16_t data_field_len)
{
    if(data_field_len > COM_RX_SIZE)
    {
        return;
    }
    /* 组包 */
    memcpy((uint8_t*)&garrComTxBuf[EN_UART_CONNECT_DIR_DOWN], p_data, data_field_len);
    /*发送*/
    _gUartComHandle[EN_UART_CONNECT_DIR_DOWN]->pTxBuf = (uint8_t*)&garrComTxBuf[EN_UART_CONNECT_DIR_DOWN];
    _gUartComHandle[EN_UART_CONNECT_DIR_DOWN]->tx_size = data_field_len;
    CommonPackSend(_gUartComHandle[EN_UART_CONNECT_DIR_DOWN]->pTxBuf, _gUartComHandle[EN_UART_CONNECT_DIR_DOWN]->tx_size, _gUartComHandle[EN_UART_CONNECT_DIR_DOWN]);
}

/**
 * @brief  : modbus寄存器数据上发
 * @para   :
 * @note   : None
 * @return : None
 */
bool _ModbusRegUpSend()
{ 
    uint16_t check_sum = 0;
    static bool sending_flag = false;
    static uint32_t wait_time = 0;   
    
    if(sending_flag == false)
    {
        wait_time = tim_get_systime();
        sending_flag = true;
    }
    
    if(tim_difftime_from_now(wait_time) < 50)
    {
        return false;
    }
    
    /* 组包 */
    /*数据包固定头装包*/
    gstReadResponsePacketData.packet_head    = COMMON_PACK_RESPONSE_PACKET_HEAD;
    gstReadResponsePacketData.send_addr      = BMS_ID;                              /*发送地址设为自己的地址*/
    gstReadResponsePacketData.recv_addr      = MONITORING_CARD_ID;                  /*接收地址监控卡*/
    gstReadResponsePacketData.cmd            = MODBUS_PACK_GET;                     /*指令*/
    gstReadResponsePacketData.data_field_len = 1 + gUartPackInfo.modbus_only_read_reg_size + gUartPackInfo.modbus_write_read_reg_size;

    /*数据体装包*/
    memcpy( garrComTxBuf[EN_UART_CONNECT_DIR_UP]                                                                                , (uint8_t*)&gstReadResponsePacketData     , sizeof(ProtocalReadResponsePacket_t));
    memcpy((garrComTxBuf[EN_UART_CONNECT_DIR_UP] + sizeof(ProtocalReadResponsePacket_t))                                          , &gUartPackInfo.ModbusAddr                  , 1                                   );
    memcpy((garrComTxBuf[EN_UART_CONNECT_DIR_UP] + sizeof(ProtocalReadResponsePacket_t) + 1)                                       , gUartPackInfo.p_modbus_only_read_reg[0]      , gUartPackInfo.modbus_only_read_reg_size );
    memcpy((garrComTxBuf[EN_UART_CONNECT_DIR_UP] + sizeof(ProtocalReadResponsePacket_t) + 1 + gUartPackInfo.modbus_only_read_reg_size) , gUartPackInfo.p_modbus_write_read_reg[0]     , gUartPackInfo.modbus_write_read_reg_size);

    if(gstReadResponsePacketData.error_code == ACK_ERROR)
    {
        /* ack error data_len 后跟 check_sum */
        check_sum = CheckSumFun(garrComTxBuf[EN_UART_CONNECT_DIR_UP], sizeof(ProtocalReadResponsePacket_t), 0);
        memcpy((garrComTxBuf[EN_UART_CONNECT_DIR_UP] + sizeof(ProtocalReadResponsePacket_t)), (uint8_t*)&check_sum, sizeof(check_sum));
    }
    else
    {
        /* ack ok data_len 后跟 data + check_sum */
        check_sum = CheckSumFun(garrComTxBuf[EN_UART_CONNECT_DIR_UP], gstReadResponsePacketData.data_field_len + sizeof(ProtocalReadResponsePacket_t), 0);
        memcpy((garrComTxBuf[EN_UART_CONNECT_DIR_UP] + sizeof(ProtocalReadResponsePacket_t) + gstReadResponsePacketData.data_field_len), (uint8_t*)&check_sum, sizeof(check_sum));
    }

    /*发送*/
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]->pTxBuf = (uint8_t*)&garrComTxBuf[EN_UART_CONNECT_DIR_UP];
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]->tx_size = gstReadResponsePacketData.data_field_len + sizeof(ProtocalReadResponsePacket_t) + sizeof(check_sum);
    CommonPackSend(_gUartComHandle[EN_UART_CONNECT_DIR_UP]->pTxBuf, _gUartComHandle[EN_UART_CONNECT_DIR_UP]->tx_size, _gUartComHandle[EN_UART_CONNECT_DIR_UP]);
    sending_flag = false;
    return true;
}


/**
 * @brief  : 地址设置数据包向上反馈
 * @para   : n_isFinalBMS：是否是最后一个BMS
 * @note   : None
 * @return : None
 */
void _AddrSetAckUpSend(ModbusRTURevData_t *p_com_revdata, bool n_isFinalBMS)
{
    uint16_t check_sum = 0;

    /* 组包 */
    if((n_isFinalBMS == false) && (gUartPackInfo.ModbusAddr != MODBUS_ADDR_PACK0))
    {
        /*不是最后一个地址数据包，向上转发*/
        memcpy((uint8_t*)&gstReadResponsePacketData, (uint8_t*)p_com_revdata->pData, 8);
        memcpy((garrComTxBuf[EN_UART_CONNECT_DIR_UP] + sizeof(ProtocalReadResponsePacket_t)), (uint8_t*)(p_com_revdata->pData + 8), gstReadResponsePacketData.data_field_len);
    }
    else
    {
        /*最后一个BMS,或第一个BMS*/

        gstReadResponsePacketData.packet_head = COMMON_PACK_RESPONSE_PACKET_HEAD;
        gstReadResponsePacketData.send_addr   = BMS_ID;                              /*发送地址设为自己的地址*/
        gstReadResponsePacketData.recv_addr   = MONITORING_CARD_ID;                  /*接收地址监控卡*/
        gstReadResponsePacketData.cmd         = MODBUS_ADDR_SET;                     /*指令*/
        gstReadResponsePacketData.data_field_len = 1;
        garrComTxBuf[EN_UART_CONNECT_DIR_UP][0] = gUartPackInfo.ModbusAddr;

    }

    memcpy(garrComTxBuf[EN_UART_CONNECT_DIR_UP], (uint8_t*)&gstReadResponsePacketData, sizeof(ProtocalReadResponsePacket_t));
    if(gstReadResponsePacketData.error_code == ACK_ERROR)
    {
        /* ack error data_len 后跟 check_sum */
        check_sum = CheckSumFun(garrComTxBuf[EN_UART_CONNECT_DIR_UP], sizeof(ProtocalReadResponsePacket_t), 0);
        memcpy((garrComTxBuf[EN_UART_CONNECT_DIR_UP] + sizeof(ProtocalReadResponsePacket_t)), (uint8_t*)&check_sum, sizeof(check_sum));
    }
    else
    {
        /* ack ok data_len 后跟 data + check_sum */
        check_sum = CheckSumFun(garrComTxBuf[EN_UART_CONNECT_DIR_UP], gstReadResponsePacketData.data_field_len + sizeof(ProtocalReadResponsePacket_t), 0);
        memcpy((garrComTxBuf[EN_UART_CONNECT_DIR_UP] + sizeof(ProtocalReadResponsePacket_t) + gstReadResponsePacketData.data_field_len), (uint8_t*)&check_sum, sizeof(check_sum));
    }

    /*发送*/
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]->pTxBuf = (uint8_t*)&garrComTxBuf[EN_UART_CONNECT_DIR_UP];
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]->tx_size = gstReadResponsePacketData.data_field_len + sizeof(ProtocalReadResponsePacket_t) + sizeof(check_sum);
    delay_1ms(5);
    CommonPackSend(_gUartComHandle[EN_UART_CONNECT_DIR_UP]->pTxBuf, _gUartComHandle[EN_UART_CONNECT_DIR_UP]->tx_size, _gUartComHandle[EN_UART_CONNECT_DIR_UP]);

    return;
}

/**
 * @brief  : 固件包向上反馈
 * @para   : read_responsed：
 * @note   : None
 * @return : None
 */
void _FWPackAckUpSend()
{
    uint16_t check_sum = 0;

    /* 组包 */
    _UpResponsePacket.packet_head = COMMON_PACK_RESPONSE_PACKET_HEAD;
    _UpResponsePacket.send_addr   = _UpRequestPacket.recv_addr;
    _UpResponsePacket.recv_addr   = _UpRequestPacket.send_addr;
    _UpResponsePacket.cmd         = _UpRequestPacket.cmd;

    _UpResponsePacket.check_sum = CheckSumFun((uint8_t*)&_UpResponsePacket, sizeof(_UpResponsePacket) - 2, 0);
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]->pTxBuf = (uint8_t*)&_UpResponsePacket;
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]->tx_size = sizeof(protocal_response_packet_t);
    CommonPackSend(_gUartComHandle[EN_UART_CONNECT_DIR_UP]->pTxBuf, _gUartComHandle[EN_UART_CONNECT_DIR_UP]->tx_size, _gUartComHandle[EN_UART_CONNECT_DIR_UP]);

    return;
}

/**
* @brief   : 限流应答上发
* @para    : None
 * @note   : None
 * @return : None
 */
bool _AckUpSend(uint8_t n_cmd, uint8_t n_error_code)
{
    
    _UpResponsePacket.packet_head = COMMON_PACK_RESPONSE_PACKET_HEAD;
    _UpResponsePacket.send_addr   = BMS_ID;                             /*发送地址设为自己的地址*/
    _UpResponsePacket.recv_addr   = MONITORING_CARD_ID;                 /*接收地址监控卡*/
    _UpResponsePacket.cmd         = n_cmd;                              /*指令*/
    _UpResponsePacket.error_code  = n_error_code;                       /*错误码*/
    _UpResponsePacket.check_sum   = CheckSumFun((uint8_t*)&_UpResponsePacket, sizeof(_UpResponsePacket) - 2, 0);    /*和校验*/

    //应答上发
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]->pTxBuf = (uint8_t*)&_UpResponsePacket;
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]->tx_size = sizeof(protocal_response_packet_t);
    CommonPackSend(_gUartComHandle[EN_UART_CONNECT_DIR_UP]->pTxBuf, _gUartComHandle[EN_UART_CONNECT_DIR_UP]->tx_size, _gUartComHandle[EN_UART_CONNECT_DIR_UP]);
    return true;
}

/**
* @brief   : 普通数据包转发
* @para    : None
 * @note   : None
 * @return : None
 */
void _CommonPackTransmit(Uart_HandleType* p_uarthandle, ModbusRTURevData_t *p_pack)
{
    /*因为是转发，所以要从另一个串口转出*/
    if(p_uarthandle->uart_cfg->rcu_uart == EN_UART_CONNECT_DIR_UP)
    {
        CommonPackSend(p_pack->pData, p_pack->data_size, _gUartComHandle[EN_UART_CONNECT_DIR_DOWN]);
    }
    else
    {
        CommonPackSend(p_pack->pData, p_pack->data_size, _gUartComHandle[EN_UART_CONNECT_DIR_UP]);
    }

}


/**
 * @brief  : 设置上串口与下串口
 * @para   :
 * @note   : None
 * @return : None
 */
void _UpUartSet(Uart_HandleType* p_uarthandle)
{
    /*与期望串口顺序一致，不动作*/
    if(_gUartComHandle[EN_UART_CONNECT_DIR_UP]->uart_cfg->uart_peri == p_uarthandle->uart_cfg->uart_peri)
    {
        return;
    }
    /*不一致，调换上下串口*/
    Uart_HandleType* p_uart                    =  _gUartComHandle[EN_UART_CONNECT_DIR_UP];
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]    =  _gUartComHandle[EN_UART_CONNECT_DIR_DOWN];
    _gUartComHandle[EN_UART_CONNECT_DIR_DOWN]  =  p_uart;
}


/*********************************************升级相关***************************************************/

/**
 * @brief  : 固件包下发
 * @para   :
 * @note   : None
 * @return : 0：升级中，
             1:升级成功，
             2:升级错误，
             3:升级超时
 */
uint8_t UpgrateFirmwareProcess()
{
    static EN_FIRMWARE_UPGRADE_STATUS status = EN_FIRMWARE_UPGRADE_STATUS_IDLE;
    static uint32_t n_wait_time               = 0;
    static uint32_t n_wait_ack_time           = 0;
    static uint16_t need_ack                  = 0;
    static uint16_t resend_count              = 0;      //重发次数
    static bool     is_final_data_pack_falg   = 0;
    static uint8_t return_val                 = 0;

    bool        flag                          = false;
    ModbusRTURevData_t *p_com_revdata         = NULL  ;

    switch(status)
    {
        /*空闲*/
        case EN_FIRMWARE_UPGRADE_STATUS_IDLE:
        {
            if(_ExternalFlashFirmwareFlagAnalysis() == true)
            {
                n_wait_time = tim_get_systime();
                _gBMSSysInfo.bms_upgrade_ing_flag = true;
                return_val = 0;
                resend_count = 0;
                status = EN_FIRMWARE_UPGRADE_STATUS_SEND_START_PACK;
            }
            break;
        }

        /*发送起始包*/
        case EN_FIRMWARE_UPGRADE_STATUS_SEND_START_PACK:
        {
            if(tim_get_systime() - n_wait_time < 100)
            {
                return 0;
            }
            //起始包下发
            _BmsPrepareSend();
//            LOG_DEBUG(LOG_TAG, "Send FW Pack Start");

            need_ack        = UPGRADE_START;
            n_wait_ack_time = tim_get_systime();
            status          = EN_FIRMWARE_UPGRADE_STATUS_WAIT_ACK;
            break;
        }


        /*发送数据包*/
        case EN_FIRMWARE_UPGRADE_STATUS_SEND_DATA_PACK:
        {
            if(tim_difftime_from_now(n_wait_time) < 100)
            {
                return 0;
            }

            uint32_t data_pack_len = 0;

            //待发固件包长度计算
            if(_gBmsFirmwareInfo.n_len - _gBmsFirmwareInfo.n_offset > UPGRADE_DATA_PACK_SIZE)
            {
                //第一包数据固定256字节，后续数据长度为UPGRADE_DATA_PACK_SIZE
                if(_gBmsFirmwareInfo.n_offset == 0)
                {
                    data_pack_len = 256;
                }
                else
                {
                    data_pack_len = UPGRADE_DATA_PACK_SIZE;
                }
                is_final_data_pack_falg = false;
            }
            else
            {
                data_pack_len = _gBmsFirmwareInfo.n_len - _gBmsFirmwareInfo.n_offset;
                is_final_data_pack_falg = true;
            }
            //下发数据包
            _BmsDataSend(data_pack_len, _gBmsFirmwareInfo.n_offset);

            //设置偏移量
            _gBmsFirmwareInfo.n_offset += data_pack_len;
            _gBmsFirmwareInfo.signal_pack_id++;
            
//            LOG_DEBUG(LOG_TAG, "Send Pack ID : %d", _gBmsFirmwareInfo.signal_pack_id);
            need_ack        = UPGRADE_BUSY;
            n_wait_ack_time = tim_get_systime();

            status          = EN_FIRMWARE_UPGRADE_STATUS_WAIT_ACK;
            break;
        }

        /*发送停止包*/
        case EN_FIRMWARE_UPGRADE_STATUS_SEND_END_PACK:
        {
            if(tim_difftime_from_now(n_wait_time) < 100)
            {
                return 0;
            }

            //下发结束包
            _BmsEndSend();
//            LOG_DEBUG(LOG_TAG, "Send FW Pack End");

            n_wait_ack_time = tim_get_systime();
            need_ack        = UPGRADE_END;
            status          = EN_FIRMWARE_UPGRADE_STATUS_WAIT_ACK;
            break;
        }

        /*等待应答*/
        case EN_FIRMWARE_UPGRADE_STATUS_WAIT_ACK:
        {
            if(tim_get_systime() - n_wait_ack_time > 4000)
            {
                //应答超时，终止下发
                status = EN_FIRMWARE_UPGRADE_STATUS_TIMEOUT;
            }

            if(QueueGet(&gDownCommonPackQueue, (void *)&p_com_revdata) == false)
            {
                return 0;
            }

            //判断应答信号是否正确
            if(_RevUpdataAckAnalysis(p_com_revdata->pData , need_ack) == false)
            {
                
//                LOG_DEBUG(LOG_TAG, "resend_count : %d", resend_count);
                //重新发送
                switch(need_ack)
                {
                    case UPGRADE_START:
                    {
                        status = EN_FIRMWARE_UPGRADE_STATUS_SEND_START_PACK;
                        break;
                    }
                    case UPGRADE_BUSY:
                    {   //重发同一个数据包
                        _gBmsFirmwareInfo.signal_pack_id--;
                        status = EN_FIRMWARE_UPGRADE_STATUS_SEND_DATA_PACK;
                        break;
                    }
                    case UPGRADE_END:
                    {
                        status = EN_FIRMWARE_UPGRADE_STATUS_SEND_END_PACK;
                        break;
                    }
                    default:
                    { 
                        status = EN_FIRMWARE_UPGRADE_STATUS_ERROR;
                        break;
                    }
                }
                
                //应答信号有问题
                if(resend_count >= 5)
                {
                    status = EN_FIRMWARE_UPGRADE_STATUS_ERROR;
 //                   LOG_DEBUG(LOG_TAG, "ack error stop pack : %d", _gBmsFirmwareInfo.signal_pack_id);
                }
                //重发次数 + 1;
                resend_count++;
            }
            else
            {
                //清除重发计数
                resend_count = 0;
                //应答信号正确，继续发送数据
                if(need_ack == UPGRADE_END)
                {
                    //接收到结束应答，升级完成
                    status = EN_FIRMWARE_UPGRADE_STATUS_SUCCESS;
                }
                else
                {
                    if(is_final_data_pack_falg == true)
                    {
                        //发送过最后一包数据后发送END
                        status = EN_FIRMWARE_UPGRADE_STATUS_SEND_END_PACK;
                        is_final_data_pack_falg = false;
                    }
                    else
                    {
                        //继续发送数据
                        status = EN_FIRMWARE_UPGRADE_STATUS_SEND_DATA_PACK;
                    }
                }
            }
            n_wait_time = tim_get_systime();
            break;
        }

        /*升级成功*/
        case EN_FIRMWARE_UPGRADE_STATUS_SUCCESS:
        {
            status = EN_FIRMWARE_UPGRADE_STATUS_END;
            n_wait_time = tim_get_systime();
            return_val = 1;
            break;
        }

        /*升级失败*/
        case EN_FIRMWARE_UPGRADE_STATUS_ERROR:
        {
            status = EN_FIRMWARE_UPGRADE_STATUS_END;
            n_wait_time = tim_get_systime();
            return_val = 2;
            break;
        }

        /*升级超时*/
        case EN_FIRMWARE_UPGRADE_STATUS_TIMEOUT:
        {
            status = EN_FIRMWARE_UPGRADE_STATUS_END;
            n_wait_time = tim_get_systime();
            return_val = 3;
            break;
        }
        case EN_FIRMWARE_UPGRADE_STATUS_END:
        {
            if(tim_difftime_from_now(n_wait_time) > 5000)
            {
//                LOG_DEBUG(LOG_TAG, "FW Quit\n\r");
                _gBMSSysInfo.bms_upgrade_ing_flag = false;
               status = EN_FIRMWARE_UPGRADE_STATUS_IDLE;
                return return_val; 
            }
            break; 
        }
        default:
            break;
    }
    return 0;
}








/**
 * @brief  : 外部flash固件标志判断
 * @para   :
 * @note   : None
 * @return : None
 */
static bool _ExternalFlashFirmwareFlagAnalysis()
{
    uint8_t temp[256];
    uint32_t offset = 0;//偏移量

    /*读取第一个文件头*/
    offset = 0;

    spi_flash_read_bytes(BMS_FIRMWARE_ADDR + offset, temp, 256);
    if(temp[0] != 0x0C || temp[1] != 0x0C || temp[2] != 0x0C)//开始判断表头是否为0X0A
    {
        return false;   //固件升级错误
    }
    if(temp[3] != 0x03)/*bms*/ //固件头是bms
    {
        return false;   //固件头不对则直接判定为错误
    }

    //接收数据长度
    _gBmsFirmwareInfo.n_len = (temp[7] | (temp[8] << 8) | (temp[9] << 16) | (temp[10] << 24));
    _gBmsFirmwareInfo.n_offset = offset;
    _gBmsFirmwareInfo.signal_pack_id = 0;

    return true;
}





/**
 * @brief  : 固件起始包
 * @para   :
 * @note   : None
 * @return : None
 */
static void _BmsPrepareSend()//准备发送
{
    uint16_t check_sum;
    _upgrate_data[0] = 0x5A;
    _upgrate_data[1] = 0x00;
    _upgrate_data[2] = 0x03;
    _upgrate_data[3] = UPGRADE_START & 0xFF;                //cmd
    _upgrate_data[4] = (UPGRADE_START >> 8) & 0xFF;         
    _upgrate_data[5] = 0x05;                                //携带数据长度
    _upgrate_data[6] = 0x00;
    memcpy(&_upgrate_data[7], &_gBmsFirmwareInfo.fw_pack_size, sizeof(uint32_t));   //固件包长度
    _upgrate_data[11] = _gBmsFirmwareInfo.bms_fw_pack_space_size;
    check_sum = CheckSumFun(_upgrate_data, 12, 0);

    _upgrate_data[12] = check_sum & 0xFF;
    _upgrate_data[13] = (check_sum >> 8) & 0xFF;

    _UpgradePackDownSend(_upgrate_data, 14);//MODEBUS
}

/**
 * @brief  : 固件数据包
 * @para   :
 * @note   : None
 * @return : None
 */
static void _BmsDataSend(uint16_t size, uint32_t offset)
{
    uint16_t check_sum;
    uint16_t signal_pack_size;
    signal_pack_size = size + _gBmsFirmwareInfo.bms_fw_pack_space_size; //单包长度为size + 拓展长度
    
    _upgrate_data[0] = 0x5A;
    _upgrate_data[1] = 0x00;
    _upgrate_data[2] = 0x03;
    _upgrate_data[3] = UPGRADE_BUSY & 0xFF;
    _upgrate_data[4] = (UPGRADE_BUSY >> 8) & 0xFF;
    _upgrate_data[5] = signal_pack_size & 0xFF;
    _upgrate_data[6] = (signal_pack_size >> 8) & 0xFF;
    _upgrate_data[7] =  _gBmsFirmwareInfo.signal_pack_id & 0xFF;
    _upgrate_data[8] = (_gBmsFirmwareInfo.signal_pack_id >> 8) & 0xFF;

    spi_flash_read_bytes(BMS_FIRMWARE_ADDR + offset, &_upgrate_data[9], size);

    check_sum = CheckSumFun(_upgrate_data, size + 9, 0);

    _upgrate_data[9 + size] = check_sum & 0xFF;
    _upgrate_data[10 + size] = (check_sum >> 8) & 0xFF;

    _UpgradePackDownSend(_upgrate_data, size + 11);
}

/**
 * @brief  : 固件结束包
 * @para   :
 * @note   : None
 * @return : None
 */
static void _BmsEndSend()//数据发送结束
{
    uint16_t check_sum;
    _upgrate_data[0] = 0x5A;
    _upgrate_data[1] = 0x00;
    _upgrate_data[2] = 0x03;
    _upgrate_data[3] = UPGRADE_END & 0xFF;
    _upgrate_data[4] = (UPGRADE_END >> 8) & 0xFF;
    _upgrate_data[5] = 0x00;
    _upgrate_data[6] = 0x00;

    check_sum = CheckSumFun(_upgrate_data, 7, 0);

    _upgrate_data[7] = check_sum & 0xFF;
    _upgrate_data[8] = (check_sum >> 8) & 0xFF;

    _UpgradePackDownSend(_upgrate_data, 9);
}



/**
 * @brief  : 固件应答包解析
 * @para   : p         :应答包指针
             n_need_ack:需要获取的应答信号
 * @note   : 判断接收的应答信号与需要的应答信号是否一致，且是否有错误出现
 * @return : None
 */
static bool _RevUpdataAckAnalysis(uint8_t *p, uint16_t need_ack)
{

    uint16_t error_code = p[5];
    uint16_t rec_cmd  = (uint16_t)(((uint16_t)p[4]) << 8 | (uint16_t)p[3]);

    if(error_code != 0)
    {
        return false;
    }

    if(rec_cmd != need_ack)
    {
        return false;
    }

    return true;
}



/**
 * @brief  : 所有电池包欠压的情况下充电MOS及限流处理（只有第一个电池包会进行）
 * @para   : 
 * @note   : None
 * @return : None
 */
void _AllPackUvCcAndMosProcess(uint8_t* p_cc_flag, DevUartPackInfo_t *p_modbus_info)
{
    static bool pack_uv_flag[BMS_PACK_MAX_NUM];         //所有Pack欠压标志
    static uint8_t rev_count[BMS_PACK_MAX_NUM];         //欠压恢复计数
    uint32_t pack_vol = 0;                              //电池包电压
    uint32_t max_pack_vol = 0;                          //最高压电池包的电压值
    uint8_t  uv_pack_num = 0;                           //欠压的电池包数量
    
    if(p_modbus_info == NULL)
    {
        return;
    }
    //一个Pack（代表没有设置地址），不进行此部分
    if(gUartPackInfo.bms_num == 1)
    {
        return;
    }
    
    //寻找电压值最大的电池包
    for(int i = 0 ; i < gUartPackInfo.bms_num ; i++)
    {
        //接收电池包电压
        pack_vol = (uint32_t)(max_pack_vol > p_modbus_info->p_modbus_only_read_reg[i]->cell_all_vdc_high << 16) || \
                             (max_pack_vol > p_modbus_info->p_modbus_only_read_reg[i]->cell_all_vdc_low);
        //接收电压值最高的电池包的ID
        if(pack_vol > max_pack_vol)
        {
            max_pack_vol = pack_vol;
        }
        
        //电池包欠压数量统计
        if(p_modbus_info->p_modbus_only_read_reg[i]->cell_min_voltage < DISCHARGE_MIN_VOLTAGE + 100)
        {
            pack_uv_flag[i] = 1;
            rev_count[i] = 0;
        }
        else
        {
            //恢复计数（延迟）
            rev_count[i]++;
            rev_count[i] = rev_count[i] >= 10 ? 10 : rev_count[i];
            
            if(rev_count[i] >= 4)
            {
                pack_uv_flag[i] = 0;
            }
        }
        
        uv_pack_num += pack_uv_flag[i];
    }
    
    //所有包均欠压
    if(gUartPackInfo.bms_num == uv_pack_num)
    {
        //PV启机的前提下，所有电池欠压时，强充标志置1(只允许运行一次)
        if((_gBMSSysInfo.force_chg_allow_flag == 0) && (_gBMSSysInfo.bms_uv_pv_start_flag == true) && \
            (GetBMSState() == EN_BMS_RUNSTATE_NORMAL))
        {
//            //重新进行预充

            _gBMSSysInfo.force_chg_allow_flag = 1;
            BmsUvStartProcess();
        }
        //所有包均欠压，且强充未禁止，且不在关机状态,开启起一个Pack的放电MOS
        if((_gBMSSysInfo.force_chg_allow_flag == 1) &&
            GetBMSState() != EN_BMS_RUNSTATE_OFF_PRE_PROCESS &&
            GetBMSState() != EN_BMS_RUNSTATE_OFF &&
            GetBMSState() != EN_BMS_RUNSTATE_OFF_OVER &&
            GetBMSState() != EN_BMS_RUNSTATE_RESTART
            )
        {
            //第一个Pack的充放电MOS强制打开
            p_cc_flag[0] = EN_CC_MODE_OPEN_CHG_MOS;
        }
    }
    
    //部分电池包的电压是正常电压，且该Pack已经度过预充阶段（防止PV启机的情况下，电池包放电放欠压后被开启强充）
    if((uv_pack_num < gUartPackInfo.bms_num) && GetBMSState() > EN_BMS_RUNSTATE_POWER_HOLD)
    {
        //禁止开强充
        _gBMSSysInfo.force_chg_allow_flag = 2;
    }
    
}

/**
 * @brief  : 发送限流开启命令函数
 * @para   : p_modbus_info : modbus信息结构体
 * @note   :
 * @return : None
 */
void _CurrentControlCmd(DevUartPackInfo_t *p_modbus_info, uint8_t target_bms_addr)
{
    //电池包电压
    uint32_t arr_pack_vol_temp[BMS_PACK_MAX_NUM] = {0, 0, 0};
    //开启限流标志数组
    static uint8_t arr_is_pack_cc_on_flag[BMS_PACK_MAX_NUM] = {0, 0, 0};
    //上一次发送的标志
    static uint8_t arr_last_flag[BMS_PACK_MAX_NUM] = {0, 0, 0};
    //电池包当前电流
    int32_t  arr_pack_cur_temp[BMS_PACK_MAX_NUM] = {0, 0, 0};
    static EN_PACK_CURRENT_MODE arr_current_mode[BMS_PACK_MAX_NUM] = {EN_PACK_CURRENT_IDLE_MODE, EN_PACK_CURRENT_IDLE_MODE, EN_PACK_CURRENT_IDLE_MODE};
    //电压最大最小值
    uint32_t vol_max_val = 0;         //0.1V
    uint32_t vol_min_val = 600;       //0.1V

    //modbus只读寄存器指针
    ModbusOnlyReadReg_t* p_only_read_reg = NULL;

    //电池包数量限制
    if(gUartPackInfo.bms_num > BMS_PACK_MAX_NUM)
    {
        gUartPackInfo.bms_num = BMS_PACK_MAX_NUM;
    }

    //电池包数据接收
    for(int i = 0 ; i < gUartPackInfo.bms_num ; i++)
    {
        //接收只读寄存器指针
        p_only_read_reg = p_modbus_info->p_modbus_only_read_reg[i];
        //接收电池包电压
        arr_pack_vol_temp[i]  = (uint32_t)(p_only_read_reg->cell_all_vdc_high << 16) | p_only_read_reg->cell_all_vdc_low;
        //接收电池包电流
        arr_pack_cur_temp[i]  = (uint32_t)(p_only_read_reg->cell_current_high << 16) | p_only_read_reg->cell_current_low;
        
        //查找最大最小值
        if(arr_pack_vol_temp[i] > vol_max_val)
        {
            vol_max_val       = arr_pack_vol_temp[i];
        }
        if(arr_pack_vol_temp[i] < vol_min_val)
        {
            vol_min_val       = arr_pack_vol_temp[i];
        }
        
        //判断充放电模式( 0.1A )
        if(arr_pack_cur_temp[i] > (PACK_CURRENT_MIN_THRESHOLD / 100))
        {
            arr_current_mode[i] = EN_PACK_CURRENT_DISCHARGE_MODE;
        }
        else if(arr_pack_cur_temp[i] < (0 - (PACK_CURRENT_MIN_THRESHOLD / 100)))
        {
            arr_current_mode[i] = EN_PACK_CURRENT_CHARGE_MODE;
        }
        else
        {
            arr_current_mode[i] = EN_PACK_CURRENT_IDLE_MODE;
        }
    }
    

    //与最大电压相差0.5V以上，强制开启限流,相差小于0.3V,关闭强开（单位0.1V）
    for(int i = 0 ; i < gUartPackInfo.bms_num ; i++)
    {

        if(vol_max_val - arr_pack_vol_temp[i] >= 5)
        {
            arr_is_pack_cc_on_flag[i] = EN_CC_MODE_OPEN_CC;
        }

        if(vol_max_val - arr_pack_vol_temp[i] <= 3)
        {
            arr_is_pack_cc_on_flag[i] = EN_CC_MODE_IDLE;
        }
    }
    
    //如果所有电池包处于idle状态，代表已均衡，不进行后续判断处理
    for(int idle_count = 0; idle_count < gUartPackInfo.bms_num ; idle_count++ )
    {
        if(arr_is_pack_cc_on_flag[idle_count] != EN_CC_MODE_IDLE)
        {
            _AllPackCcModeJudge(arr_last_flag, arr_is_pack_cc_on_flag, arr_current_mode);
            break;
        }

    }
    
    //关闭满电Pack的充电MOS及CC，关闭空电Pack的放电MOS
    _AllPackFullOrEmptyPackCcAndMosProcess(arr_last_flag, arr_is_pack_cc_on_flag, p_modbus_info);
    //所有电池包欠压时，MOS处理
    _AllPackUvCcAndMosProcess(arr_is_pack_cc_on_flag, &gUartPackInfo);
    
    //第一个电池包自身设置限流
    //BMS正常运行时才可以对MOS进行控制
    if(_gBMSSysInfo.bms_run_status == EN_BMS_RUNSTATE_NORMAL)
    {
        gPackStatus.cc_mode = (EN_CC_MODE)arr_is_pack_cc_on_flag[0];
        CcCmdRevProcess(gPackStatus.cc_mode);
    }
    
    //标志记录
    memcpy(arr_last_flag, arr_is_pack_cc_on_flag, BMS_PACK_MAX_NUM);

    //数据结构：slave_modbus_addr, flag1, flag2, flag3
    uint8_t arr_send_data_temp[BMS_PACK_MAX_NUM + 1];
    memset(arr_send_data_temp, 0, (BMS_PACK_MAX_NUM + 1));
    
    arr_send_data_temp[0] = target_bms_addr;
    //组包
    memcpy(&arr_send_data_temp[1], arr_is_pack_cc_on_flag, BMS_PACK_MAX_NUM);
    
    //只有一个Pack时，不向下进行发送
    if(gUartPackInfo.bms_num == 1)
    {
        return;
    }
    //发送限流开启标志
    _CommonPackDownSend(MONITORING_CARD_ID, BMS_ID, CC_ON_CMD_SEND, arr_send_data_temp, (BMS_PACK_MAX_NUM + 1));

}

/**
 * @brief  : 所有电池包模式判断（只有第一个电池包会进行）
 * @para   : 
 * @note   : None
 * @return : None
 */
void _AllPackCcModeJudge(uint8_t* p_last_cc_flag, uint8_t* p_cc_flag, EN_PACK_CURRENT_MODE* p_current_mode)
{
    //所有电池包充放电模式判断
    //   int i = 0;
    int mos_close_pack_count = 0;
    int chg_pack_count = 0;
    int dsg_pack_count = 0;
    
    //统计处于充电、放电、断开状态的pack包数量
    for(int i = 0; i < gUartPackInfo.bms_num; i++)
    {
        if(p_last_cc_flag[i] == EN_CC_MODE_CLOSE_CHG_MOS_AND_CC || p_last_cc_flag[i] == EN_CC_MODE_CLOSE_DSG_MOS)
        {
            mos_close_pack_count++;
        }
        else
        {
            if(p_current_mode[i] == EN_PACK_CURRENT_CHARGE_MODE)
            {
                chg_pack_count++;
            }
            else if(p_current_mode[i] == EN_PACK_CURRENT_DISCHARGE_MODE)
            {
                dsg_pack_count++;
            }
        }
        
    }
    
    //有充电有放电，没有进行外接充放电，或进行小功率充放电
    if((chg_pack_count != 0 && dsg_pack_count != 0)  || (chg_pack_count == 0 && dsg_pack_count == 0))
    {
        for(int i = 0; i < gUartPackInfo.bms_num; i++)
        {
            //打开电池包关闭的充放电MOS
            if(p_last_cc_flag[i] == EN_CC_MODE_CLOSE_CHG_MOS_AND_CC && p_cc_flag[i] != EN_CC_MODE_OPEN_CC)
            {
                p_cc_flag[i] = EN_CC_MODE_OPEN_CHG_MOS;
            }
            if(p_last_cc_flag[i] == EN_CC_MODE_CLOSE_DSG_MOS)
            {
                p_cc_flag[i] = EN_CC_MODE_OPEN_DSG_MOS;
            }
        }
        
    }
    //处于外接放电状态
    else if((chg_pack_count == 0) && ((dsg_pack_count + mos_close_pack_count) == gUartPackInfo.bms_num))
    {
        //所有电池包同时处在放电状态下，开启所有电池包的充电mos
        //防止电流从限流模块走
        for(int i = 0 ; i < gUartPackInfo.bms_num ; i++)
        {
            //开启放电mos
            p_cc_flag[i] = EN_CC_MODE_OPEN_CHG_MOS; 
        }

    }
    //处于外接充电状态
    else if((dsg_pack_count == 0) && ((chg_pack_count + mos_close_pack_count) == gUartPackInfo.bms_num))
    {
        //所有电池包同时处在充电状态下，开启所有电池包的充电mos
        for(int i = 0 ; i < gUartPackInfo.bms_num ; i++)
        {
            //开启放电mos
            p_cc_flag[i] = EN_CC_MODE_OPEN_CHG_MOS; 
        }
        
    }
}


/**
 * @brief  : 满电或空电电池包MOS及限流处理（只有第一个电池包会进行）
 * @para   : 
 * @note   : None
 * @return : None
 */
void _AllPackFullOrEmptyPackCcAndMosProcess(uint8_t* p_last_cc_flag, uint8_t* p_cc_flag, DevUartPackInfo_t *p_modbus_info)
{
    static bool     full_chg_flag[BMS_PACK_MAX_NUM];     //满充标志
    static bool     empty_dsg_flag[BMS_PACK_MAX_NUM];    //空放标志
    
    uint32_t max_cell_vol = 0;                           //最高电压电芯的电压值
    uint32_t min_cell_vol = 0;                           //最低电压电芯的电压值
    int32_t  pack_current = 0;                           //电池包电流
    int32_t  delta_low_temp_recover_cell_v = 0;          //电池包低温恢复电压阈值增量
        
    if(p_modbus_info == NULL)
    {
        return;
    }
    
    for(int i = 0 ; i < gUartPackInfo.bms_num ; i++)
    {
        //接收电池包电压
        max_cell_vol = p_modbus_info->p_modbus_only_read_reg[i]->cell_max_voltage;
        min_cell_vol = p_modbus_info->p_modbus_only_read_reg[i]->cell_min_voltage;
        
        //接收电池包电流
        pack_current = (p_modbus_info->p_modbus_only_read_reg[i]->cell_current_high << 16) | p_modbus_info->p_modbus_only_read_reg[i]->cell_current_low;
        //电压阈值恢复增量设置(10度以下才起作用)
        delta_low_temp_recover_cell_v  = (100 - p_modbus_info->p_modbus_only_read_reg[i]->cell_min_temp) / 2;
        //限值
        delta_low_temp_recover_cell_v = delta_low_temp_recover_cell_v < 0 ? 0 : delta_low_temp_recover_cell_v;
        
        //电池包满充、空放判断
        if(max_cell_vol > CHARGE_MAX_VOLTAGE + 20)
        {
            full_chg_flag[i] = 1;
        }
        else if(max_cell_vol < (CHARGE_MAX_VOLTAGE - 200))
        {
            full_chg_flag[i] = 0;
        }
        
        if(min_cell_vol < DISCHARGE_MIN_VOLTAGE - 10)
        {
            empty_dsg_flag[i] = 1;
        }
        else if(min_cell_vol > (DISCHARGE_MIN_VOLTAGE + 200 + delta_low_temp_recover_cell_v))
        {
            empty_dsg_flag[i] = 0;
        }
        
        //满充或空放电池包关断对应MOS(0.1A)
        if(full_chg_flag[i] == 1)
        {
            //处于放电状态，且充电MOS及限流已被关闭，开启充电MOS或限流(0.1A)
            if(pack_current >= 5)
            {
                if(p_last_cc_flag[i] == EN_CC_MODE_CLOSE_CHG_MOS_AND_CC)
                {
                    p_cc_flag[i] = EN_CC_MODE_IDLE;
                }
            }
            else if(pack_current < 3)
            {
                p_cc_flag[i] = EN_CC_MODE_CLOSE_CHG_MOS_AND_CC;
            }
        }
        
        if(empty_dsg_flag[i] == 1)
        {
            
            //处于充电状态，且放电MOS已被关闭，开启放电MOS(0.1A)
            if(pack_current <= -5)
            {
                if(p_last_cc_flag[i] == EN_CC_MODE_CLOSE_DSG_MOS)
                {
                    p_cc_flag[i] = EN_CC_MODE_OPEN_DSG_MOS;
                }
            }
            else if(pack_current > -3)
            {   //不处于充电状态，关闭放电MOS，防止过渡放电
                p_cc_flag[i] = EN_CC_MODE_CLOSE_DSG_MOS;
            }
                
        }
        else
        {
            //电压恢复后开启MOS（放电MOS无法通过正常的限流判断恢复，所以需要单独恢复）
            if(p_last_cc_flag[i] == EN_CC_MODE_CLOSE_DSG_MOS)
            {
                p_cc_flag[i] = EN_CC_MODE_OPEN_DSG_MOS;
            }
        }
    }
    
    if(_gBMSSysInfo.force_chg_allow_flag != 2)
    {
        p_cc_flag[0] = EN_CC_MODE_OPEN_DSG_MOS;
    }
    
}


/**
 * @brief  : 关机命令下发
 * @para   : 
 * @note   : None
 * @return : None
 */
void ShutDownCmdSend()
{
    _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_ALL_PACK_SHUT_DOWN_SEND;
}



