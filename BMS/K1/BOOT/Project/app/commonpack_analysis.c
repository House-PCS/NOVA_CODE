#include "commonpack_analysis.h"
#include "math.h"
#include "mid_shell.h"
#define LOG_TAG     "fwupgrade"

#define FIRST_PACK_INQUIRE_MODBUS_DATA_TIME         (1000)

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

} EN_FIRMWARE_UPGRADE_STATUS;

typedef struct
{
    uint32_t n_len;                             //子包数据长度    
    uint32_t n_offset;                          //当前地址偏移量
    uint32_t signal_pack_id;                    //单包流水号
    uint32_t fw_pack_size;                      //固件包总长度
    uint8_t  bms_fw_pack_space_size;            //固件子包拓展长度
    uint32_t disconnected_max_time;             //未接收到固件包的时间
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
static void _BmsDataSend(uint16_t size, uint32_t offset);//MPPT数据发送中
static void _BmsEndSend(void);//数据发送结束
static bool _RevUpdataAckAnalysis(uint8_t *p, uint16_t n_need_ack);
uint8_t UpgrateFirmwareProcess(void);
/* 所有pack的模式判断 */
void _AllPackCcModeJudge(uint8_t* p_last_cc_flag, uint8_t* p_cc_flag, EN_PACK_CURRENT_MODE* p_current_mode);
/* 所有pack满充空放时MOS及CC控制 */
void _AllPackFullOrEmptyPackCcAndMosProcess(uint8_t* p_last_cc_flag, uint8_t* p_cc_flag, DevUartPackInfo_t *p_modbus_info);


/**
* @brief CheckSumFun
* @param *pdata: data
* @param len: len
* @param mode: 0-checksum16     1-checksum32
* @return
* - checksum
* @note
*/
static uint32_t CheckSumFun(uint8_t *pdata, uint16_t len, uint8_t mode)
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
            bms_fw_all_pack_sumcheck = 0;
            
            gUartPackInfo.p_modbus_only_read_reg[0]->up_finish_flag = 0;
            _gBMSSysInfo.bms_upgrade_ing_flag = true;
            
            LedTimeSetFunc(LED_STATUS_UPGRADE);
//            LOG_DEBUG(LOG_TAG, "FW Rev Start");
            //刷新超时时间
            _gBmsFirmwareInfo.disconnected_max_time = tim_get_systime();
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
                _gBmsFirmwareInfo.fw_pack_size = *((uint32_t*)&p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size + 7]);
                /* 固件校验和 */
                bms_fw_rec_sumcheck = *((uint32_t*)&p_request_packet->data_field[_gBmsFirmwareInfo.bms_fw_pack_space_size + 256 - 4]);

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
                memset(firmware_upgrade, 0xA0, 256);
                spi_flash_write_bytes_with_erase(UPGRADE_FLAG_ADDR, firmware_upgrade, 256);
                //检测所有Pack是否升级完毕，全部升级完毕后应答成功，否则应答升级中
                _FirstPackUpEndAckProcess(&_UpResponsePacket.error_code);
                
                if(gUartPackInfo.p_modbus_only_read_reg[0]->up_finish_flag == 0)
                {
                    gUartPackInfo.p_modbus_only_read_reg[0]->up_finish_flag = 1;
                    //准备下发固件包
                    _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_ALL_PACK_UPGRADE_PACK_DOWN_SEND;
                }
            }
            //升级结束继续AFE采集
            _gBMSSysInfo.bms_upgrade_ing_flag = false;
            LedTimeSetFunc(LED_STATUS_NORMAL);
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
    memcpy(_ProtocolBuf, (uint8_t*)(p_com_revdata->pData + 7), _UpRequestPacket.data_field_len);
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
//                    NOVA_SET_BIT(gEmsErrorInfo.value, (modbus_data_read_addr - MODBUS_ADDR_PACK0));
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
    /* 组包 */
    memcpy((uint8_t*)&garrComTxBuf[EN_UART_CONNECT_DIR_UP], p_data, data_field_len);
    /*发送*/
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]->pTxBuf = (uint8_t*)&garrComTxBuf[EN_UART_CONNECT_DIR_UP];
    _gUartComHandle[EN_UART_CONNECT_DIR_UP]->tx_size = data_field_len;
    CommonPackSend(_gUartComHandle[EN_UART_CONNECT_DIR_UP]->pTxBuf, _gUartComHandle[EN_UART_CONNECT_DIR_UP]->tx_size, _gUartComHandle[EN_UART_CONNECT_DIR_UP]);
}

void _UpgradePackDownSend(uint8_t* p_data, uint16_t data_field_len)
{
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
    static bool     is_final_data_pack_falg = 0;

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
            resend_count    = 0;
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
            status = EN_FIRMWARE_UPGRADE_STATUS_IDLE;
            return 1;
        }

        /*升级失败*/
        case EN_FIRMWARE_UPGRADE_STATUS_ERROR:
        {
            status = EN_FIRMWARE_UPGRADE_STATUS_IDLE;
            return 2;
        }

        /*升级超时*/
        case EN_FIRMWARE_UPGRADE_STATUS_TIMEOUT:
        {
            status = EN_FIRMWARE_UPGRADE_STATUS_IDLE;
            return 3;
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


void HostSlaveBmsUpgradeTest()
{
    _geCommonPackRequestStatus = COMMON_PACK_REQUEST_STATUS_ALL_PACK_UPGRADE_PACK_DOWN_SEND;
}




