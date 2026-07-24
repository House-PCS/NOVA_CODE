#include "slave_coprocessor.h"
#include "dev_modbus.h"
#include "slave_collect.h"
#include "dev_key.h"


#define SLAVE_COPRO_REV_MAX_TIME                                 1000
#define SLAVE_COPRO_INTERVAL_TIME                                50
#define SLAVE_COPRO_POLLING_TIME                                 100



MainCardCopInfo_t   gstMainCardCopInfo;

static EN_COPRO_POLLING_STATUS      _geCoproSlavePollingStatus;
CoprocessorPara_t                   gstCoprocessorPara;
SlaveRead_t                 gstModbusCoproRead;
SlaveWrite_t                gstCoproModbusSlaveWrite;
Queue_t                     gModbusCoproCommSendQueue;
uint8_t                     _garrCoproSendData[256];
static uint8_t              _garrTxData[2100];
uint8_t     MoodLedCtrlDataBuf[30];
uint8_t     MoodLedCtrlFlag = 0;

typedef struct  _deal_fram_stru_t
{
    uint8_t                                 slave_addr;
    EN_COPRO_PARA_READ_EVENT                envent;
    uint8_t                                 func_code;
    uint16_t                                reg_addr;
    uint8_t                                 *pdata;
    uint16_t                                reg_num;
} CoprocessorReadDealFram_t;


const CoprocessorReadDealFram_t  garrCoprocessorReadDealFramArr[] =
{
//    {SLAVE_COPROCESSOR_ADDR, EN_READ_COPRO_START_LED_EVENT,      READ_COPRO_HOLD_TYPE,   START_ATMOS_LED_REG_ADDR,       (uint8_t *)&(gstCoprocessorPara.CoproHoldData.start_staus),     sizeof(CoproHoldPara_t) / sizeof(uint16_t)},   //0xA000~0xA007
//    {SLAVE_COPROCESSOR_ADDR, EN_READ_COPRO_START_LED_EVENT,      READ_COPRO_HOLD_TYPE,   MAIN_COP_UPGRADE_REG_ADDR,       
//    (uint8_t *)&(gstMainCardCopInfo.ctrl_info.upgrade_status),     sizeof(MainCardCopCtrlInfo_t) / sizeof(uint16_t)},   //0xA000~0xA007
#if 0  
    {SLAVE_COPROCESSOR_ADDR, EN_READ_COPRO_BASE_INFO_EVENT,      READ_COPRO_HOLD_TYPE,   MAIN_COP_BASE_INFO_REG_ADDR,       
    (uint8_t *)&(gstCollectInfo.maincard_data.inside_info.main_cop_info.base_info),     sizeof(MainCardCopBaseInfo_t) / sizeof(uint16_t),},
    {SLAVE_COPROCESSOR_ADDR, EN_READ_COPRO_RUN_STAT_EVENT,      READ_COPRO_HOLD_TYPE,   MAIN_COP_RUNSTAT_INFO_REG_ADDR,       
    (uint8_t *)&(gstCollectInfo.maincard_data.inside_info.main_cop_info.status_info),     sizeof(MainCardCopStatusInfo_t) / sizeof(uint16_t),},
    {SLAVE_COPROCESSOR_ADDR, EN_READ_COPRO_CTRL_INFO_EVENT,      READ_COPRO_HOLD_TYPE,   MAIN_COP_UPGRADE_REG_ADDR,       
    (uint8_t *)&(gstCollectInfo.maincard_data.inside_info.main_cop_info.ctrl_info),     sizeof(MainCardCopCtrlInfo_t) / sizeof(uint16_t),},
#endif
    {SLAVE_COPROCESSOR_ADDR, EN_READ_COPRO_BASE_INFO_EVENT,      READ_COPRO_HOLD_TYPE,   MAIN_COP_BASE_INFO_REG_ADDR,       
    (uint8_t *)&(gstMainCardCopInfo.base_info),     sizeof(MainCardCopBaseInfo_t) / sizeof(uint16_t),},
    {SLAVE_COPROCESSOR_ADDR, EN_READ_COPRO_RUN_STAT_EVENT,      READ_COPRO_HOLD_TYPE,   MAIN_COP_RUNSTAT_INFO_REG_ADDR,       
    (uint8_t *)&(gstMainCardCopInfo.status_info),     sizeof(MainCardCopStatusInfo_t) / sizeof(uint16_t),},
    {SLAVE_COPROCESSOR_ADDR, EN_READ_COPRO_CTRL_INFO_EVENT,      READ_COPRO_HOLD_TYPE,   MAIN_COP_UPGRADE_REG_ADDR,       
    (uint8_t *)&(gstMainCardCopInfo.ctrl_info),     sizeof(MainCardCopCtrlInfo_t) / sizeof(uint16_t),},
    
    
};

#define MAX_SIZE       (sizeof(garrCoprocessorReadDealFramArr)/ sizeof(garrCoprocessorReadDealFramArr[0]))

void SlaveCorproCommInit(void)
{
    QueueInit(&gModbusCoproCommSendQueue, CORPRO_WRITE_QUEUE_DEPTH, sizeof(SlaveWrite_t));
}



static void _CoproSlaveReadErrOrDeal(uint8_t slave_addr, uint16_t comm_status)
{
    if(slave_addr == SLAVE_COPROCESSOR_ADDR)
    {
        if(comm_status == MBRTU_OK)
        {
            gstCoprocessorPara.copro_comm_err_cnt = 0;
            gstCoprocessorPara.copro_comm_time_out = 0;
        }
        else
        {
            gstCoprocessorPara.copro_comm_err_cnt++;
            if(gstCoprocessorPara.copro_comm_err_cnt > 10)
            {
                gstCoprocessorPara.copro_comm_err_cnt = 10;
                gstCoprocessorPara.copro_comm_time_out = 1;
            }
        }
    }
}


uint16_t Modbus_ParaAnalysis(uint8_t *p_recv_buf, uint16_t rec_len, SlaveRead_t *p_FrameInfo, uint8_t *p_data)
{
    uint16_t para_state = MBRTU_OK;
    uint16_t i = 0;
    uint16_t length = 0;
    uint16_t slave_addr = 0;
    uint16_t data_num = 0;
    uint8_t fun_code = 0;
    uint8_t  *p_recv_data = 0;

    if(p_recv_buf == NULL || (p_FrameInfo == NULL))
    {
        para_state = MBRTU_PARA_INVALID;
        return para_state;
    }

    if(rec_len < 9)
    {
        para_state = MBRTU_LEN_ERR;
        return para_state;
    }

    //获取从机地址
    slave_addr = p_recv_buf[length++];
    //获取功能码
    fun_code = p_recv_buf[length++];

    switch(fun_code)
    {
        case MBRTU_FC_READ_STANDREG:
        case MBRTU_FC_READ_ORREG:
        {
//读返回
            if(p_FrameInfo->slave_addr != slave_addr)
            {
                para_state = MBRTU_ADR_ERR;
                break;
            }
            //数据长度
            data_num = p_recv_buf[length++];
            p_recv_data = &p_recv_buf[length];
            //数据
            for(i = 0; i < (data_num >> 1); i++)
            {
                p_data[2 * i] = p_recv_data[2 * i + 1];
                p_data[2 * i + 1] = p_recv_data[2 * i];
            }
            break;
        }
        case MBRTU_FC_SWRITE_STANDREG:
        case MBRTU_FC_WRITE_STANDREG:
        {
//写返回
            p_FrameInfo->slave_addr = slave_addr;
            break;
        }
        case MBRTU_FC_READ_STANDREG_ERR:
        case MBRTU_FC_READ_ORREG_ERR:
        case MBRTU_FC_SWRITE_STANDREG_ERR:
        case MBRTU_FC_WRITE_STANDREG_ERR:
        {
//错误
            para_state = MBRTU_PARA_INVALID;
            break;
        }
        default:
            break;
    }
    return para_state;
}


static void _CoproAllReadPolling(SlaveRead_t *p_slave_frame)
{
    static uint8_t cnt = 0;
    static EN_COPRO_PARA_READ_EVENT e_event_count = EN_READ_COPRO_BASE_INFO_EVENT;

    p_slave_frame->slave_addr = garrCoprocessorReadDealFramArr[e_event_count].slave_addr;
    p_slave_frame->func_code  = garrCoprocessorReadDealFramArr[e_event_count].func_code;
    p_slave_frame->reg_addr   = garrCoprocessorReadDealFramArr[e_event_count].reg_addr;
    p_slave_frame->reg_num = garrCoprocessorReadDealFramArr[e_event_count].reg_num;
    //if(gstCoprocessorPara.copro_busy_flag != 1)
    if(dma_transfer_number_get(ModbusHostCoprocessorUartHandle.uart_cfg->dma_tx_peri, ModbusHostCoprocessorUartHandle.uart_cfg->dma_tx_channel) == 0)
    {
        ModbusHostRead(p_slave_frame, ModbusCoprocessorRTUSend);
    }
    e_event_count++;
    e_event_count %= EN_READ_COPRO_EVENT_NUM;
}



void SlaveCoproModbusCommProcess(void)
{
    uint8_t i = 0;
    static uint8_t index = 0;
    static ModbusRTURevData_t *p;
    static SlaveWrite_t *p_coprolavewrite;
    static uint32_t copro_read_polling_cnt = 0;

    switch(_geCoproSlavePollingStatus)
    {
        case EN_COPRO_POLLING_STATUS_IDLE:
        {
            /*空闲时段也要执行Get gModbusRevQueue, 并舍弃，防止队列满导致无法再接收数据*/

            if(MoodLedCtrlFlag ==1)
            {
                MoodLedCtrlFlag = 0;
//                ModbusCoprocessorRTUSend(MoodLedCtrlDataBuf, 17);
                _geCoproSlavePollingStatus = EN_COPRO_POLLING_STATUS_REV_INTERVAL_WAIT;
                copro_read_polling_cnt = tim_get_systime();
            }
            else
            {

                QueueGet(&gstDevModbusCoprocessor.rev_queue, (void *)&p);
                if(QueueGet(&gModbusCoproCommSendQueue, (void *)&p_coprolavewrite) == true)/*如果有主动写操作*/
                {
                    if(p_coprolavewrite->func_code == 0x00)
                    {
                        //私有协议
                        ModbusCoprocessorRTUSend((uint8_t *)p_coprolavewrite->data, 10);
                        _geCoproSlavePollingStatus = EN_COPRO_POLLING_STATUS_REV_INTERVAL_WAIT;
                        copro_read_polling_cnt = tim_get_systime();
                    }
                    else
                    {
                        gstModbusCoproRead.func_code = p_coprolavewrite->func_code;
                        gstModbusCoproRead.slave_addr = p_coprolavewrite->slave_addr;
                        ModbusHostWrite(p_coprolavewrite, ModbusCoprocessorRTUSend, _garrCoproSendData);
                        _geCoproSlavePollingStatus = EN_COPRO_POLLING_STATUS_WAITING_REV;
                        copro_read_polling_cnt = tim_get_systime();
                    }
                }
                else
                {
                    /*轮询回读*/
                    if((uint32_t)(tim_get_systime() - copro_read_polling_cnt) > SLAVE_COPRO_POLLING_TIME)
                    {
                        copro_read_polling_cnt = tim_get_systime();
                        _CoproAllReadPolling(&gstModbusCoproRead);
                        _geCoproSlavePollingStatus = EN_COPRO_POLLING_STATUS_WAITING_REV;
                    }
                }   
            }
            break;
        }
        case EN_COPRO_POLLING_STATUS_WAITING_REV:
        {
            if(QueueGet(&gstDevModbusCoprocessor.rev_queue, (void *)&p) == true)
            {
                copro_read_polling_cnt = tim_get_systime();
                for(i = 0; i < MAX_SIZE; i++)
                {
                    if((garrCoprocessorReadDealFramArr[i].func_code == gstModbusCoproRead.func_code) &&
                            (garrCoprocessorReadDealFramArr[i].slave_addr == gstModbusCoproRead.slave_addr) &&
                            (garrCoprocessorReadDealFramArr[i].reg_addr == gstModbusCoproRead.reg_addr))
                    {
                        break;          //确保是读取的信息
                    }
                }

                if(i == MAX_SIZE)
                {
                    //是写命令返回，没有进行判断。
                    _geCoproSlavePollingStatus = EN_COPRO_POLLING_STATUS_REV_OVERTIME;
                    break;
                }
                if(Modbus_ParaAnalysis((uint8_t *)p->p_data, p->data_size, &gstModbusCoproRead, garrCoprocessorReadDealFramArr[i].pdata) == MBRTU_OK)
                {
                    //数据正常
                    _CoproSlaveReadErrOrDeal(gstModbusCoproRead.slave_addr, MBRTU_OK);
                }
                else
                {
                    //数据异常
                    _CoproSlaveReadErrOrDeal(gstModbusCoproRead.slave_addr, MBRTU_PARA_INVALID);
                }
                _geCoproSlavePollingStatus = EN_COPRO_POLLING_STATUS_REV_INTERVAL_WAIT;
            }
            if((uint32_t)(tim_get_systime() - copro_read_polling_cnt) > SLAVE_COPRO_REV_MAX_TIME)
            {
                copro_read_polling_cnt = tim_get_systime();
                _geCoproSlavePollingStatus = EN_COPRO_POLLING_STATUS_REV_OVERTIME;
            }
            break;
        }
        case EN_COPRO_POLLING_STATUS_REV_OVERTIME:
        {
            /* 做超时处理*/
            copro_read_polling_cnt = tim_get_systime();
            _geCoproSlavePollingStatus = EN_COPRO_POLLING_STATUS_IDLE;
            _CoproSlaveReadErrOrDeal(gstModbusCoproRead.slave_addr, MBRTU_PARA_INVALID);
            break;
        }
        case EN_COPRO_POLLING_STATUS_REV_INTERVAL_WAIT:
        {
            if((uint32_t)(tim_get_systime()) - copro_read_polling_cnt > SLAVE_COPRO_INTERVAL_TIME)
            {
                _geCoproSlavePollingStatus = EN_COPRO_POLLING_STATUS_IDLE;
                copro_read_polling_cnt = tim_get_systime();
            }
            break;
        }
        default:
        {
            _geCoproSlavePollingStatus = EN_COPRO_POLLING_STATUS_IDLE;
            break;
        }
    }
}          


bool SetStartLedStaInfo(uint16_t status)
{
    gstCoproModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gstCoproModbusSlaveWrite.reg_addr = START_ATMOS_LED_REG_ADDR;
    gstCoproModbusSlaveWrite.reg_num = 1;
    gstCoproModbusSlaveWrite.slave_addr = SLAVE_COPROCESSOR_ADDR;
    gstCoproModbusSlaveWrite.data[0] = status;
    if(QueuePut(&gModbusCoproCommSendQueue, &gstCoproModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

bool SetSysLedStaInfo(uint16_t status)
{
    gstCoproModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gstCoproModbusSlaveWrite.reg_addr = SYS_ATMOS_LED_REG_ADDR;
    gstCoproModbusSlaveWrite.reg_num = 1;
    gstCoproModbusSlaveWrite.slave_addr = SLAVE_COPROCESSOR_ADDR;
    gstCoproModbusSlaveWrite.data[0] = status;
    if(QueuePut(&gModbusCoproCommSendQueue, &gstCoproModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
bool SetCollectorLedStaInfo(uint16_t status)
{
    gstCoproModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gstCoproModbusSlaveWrite.reg_addr = COLLECTOR_ATMOS_LED_REG_ADDR;
    gstCoproModbusSlaveWrite.reg_num = 1;
    gstCoproModbusSlaveWrite.slave_addr = SLAVE_COPROCESSOR_ADDR;
    gstCoproModbusSlaveWrite.data[0] = status;
    if(QueuePut(&gModbusCoproCommSendQueue, &gstCoproModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
bool SetPCSLedStaInfo(uint16_t status)
{
    gstCoproModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gstCoproModbusSlaveWrite.reg_addr = PCS_ATMOS_LED_REG_ADDR;
    gstCoproModbusSlaveWrite.reg_num = 1;
    gstCoproModbusSlaveWrite.slave_addr = SLAVE_COPROCESSOR_ADDR;
    gstCoproModbusSlaveWrite.data[0] = status;
    if(QueuePut(&gModbusCoproCommSendQueue, &gstCoproModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
bool SetConfirmLedStaInfo(uint16_t status)
{
    gstCoproModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gstCoproModbusSlaveWrite.reg_addr = MAIN_COP_CONFIRM_KEY_REG_ADDR;
    gstCoproModbusSlaveWrite.reg_num = 1;
    gstCoproModbusSlaveWrite.slave_addr = SLAVE_COPROCESSOR_ADDR;
    gstCoproModbusSlaveWrite.data[0] = status;
    if(QueuePut(&gModbusCoproCommSendQueue, &gstCoproModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
bool SetESCLedStaInfo(uint16_t status)
{
    gstCoproModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gstCoproModbusSlaveWrite.reg_addr = MAIN_COP_ESC_KEY_REG_ADDR;
    gstCoproModbusSlaveWrite.reg_num = 1;
    gstCoproModbusSlaveWrite.slave_addr = SLAVE_COPROCESSOR_ADDR;
    gstCoproModbusSlaveWrite.data[0] = status;
    if(QueuePut(&gModbusCoproCommSendQueue, &gstCoproModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

bool SetUpKeyLedStaInfo(uint16_t status)
{
    gstCoproModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gstCoproModbusSlaveWrite.reg_addr = MAIN_COP_UP_KEY_REG_ADDR;
    gstCoproModbusSlaveWrite.reg_num = 1;
    gstCoproModbusSlaveWrite.slave_addr = SLAVE_COPROCESSOR_ADDR;
    gstCoproModbusSlaveWrite.data[0] = status;
    if(QueuePut(&gModbusCoproCommSendQueue, &gstCoproModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
 
bool SetDownKeyLedStaInfo(uint16_t status)
{
    gstCoproModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gstCoproModbusSlaveWrite.reg_addr = MAIN_COP_DOWN_KEY_REG_ADDR;
    gstCoproModbusSlaveWrite.reg_num = 1;
    gstCoproModbusSlaveWrite.slave_addr = SLAVE_COPROCESSOR_ADDR;
    gstCoproModbusSlaveWrite.data[0] = status;
    if(QueuePut(&gModbusCoproCommSendQueue, &gstCoproModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}   

bool SetMoodLedStaInfo(uint32_t clour_value,uint16_t mode,uint16_t period)
{
//    gstCoproModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gstCoproModbusSlaveWrite.reg_addr = MOOD_LED_REG_ADDR;
//    gstCoproModbusSlaveWrite.reg_num = 4;
//    gstCoproModbusSlaveWrite.slave_addr = SLAVE_COPROCESSOR_ADDR;

//    gstCoproModbusSlaveWrite.data[0] = (uint16_t )clour_value;
//    gstCoproModbusSlaveWrite.data[1] = (uint16_t )(clour_value>>16);
//    gstCoproModbusSlaveWrite.data[2] = mode;
//    gstCoproModbusSlaveWrite.data[3] = period;

    uint16_t crc =0;
    uint8_t len = 0;
    gstCoprocessorPara.copro_busy_flag = 1;
    MoodLedCtrlDataBuf[len++] = SLAVE_COPROCESSOR_ADDR;
    MoodLedCtrlDataBuf[len++] = MBRTU_FC_WRITE_STANDREG;
    MoodLedCtrlDataBuf[len++] = (uint8_t)(MAIN_COP_LED_MODE_REG_ADDR >>8);
    MoodLedCtrlDataBuf[len++] = (uint8_t)MAIN_COP_LED_MODE_REG_ADDR;
    MoodLedCtrlDataBuf[len++] = 0;
    MoodLedCtrlDataBuf[len++] = 4;
    MoodLedCtrlDataBuf[len++] = 8;

    MoodLedCtrlDataBuf[len++] =  (uint8_t)(clour_value>>8);
    MoodLedCtrlDataBuf[len++] =  (uint8_t)(clour_value);
    MoodLedCtrlDataBuf[len++] =  (uint8_t)(clour_value>>24);
    MoodLedCtrlDataBuf[len++] = (uint8_t)(clour_value>>16);
    MoodLedCtrlDataBuf[len++] = (uint8_t)(mode>>8);
    MoodLedCtrlDataBuf[len++] = (uint8_t)(mode);
    MoodLedCtrlDataBuf[len++] = (uint8_t)(period>>8);
    MoodLedCtrlDataBuf[len++] = (uint8_t)(period);


    crc = CrcModbus(MoodLedCtrlDataBuf, len);
    MoodLedCtrlDataBuf[len++] = crc & 0xFF;
    MoodLedCtrlDataBuf[len++] = (crc >> 8) & 0xFF;
    ModbusCoprocessorRTUSend(MoodLedCtrlDataBuf, 17);
    
    gstCoprocessorPara.copro_busy_flag = 0;
//    MoodLedCtrlFlag = 1;
//    if(QueuePut(&gModbusCoproCommSendQueue, &gstCoproModbusSlaveWrite) == false)
//    {
//        return false;
//    }
    return true;
}  

/**
  * @brief   设置协处理器升级状态
  * @param   None
  * @return  None
  * @note    None
  */
bool SetMainCopSysUpgradeStatus(uint16_t status)
{
    gstCoproModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gstCoproModbusSlaveWrite.reg_addr = MAIN_COP_UPGRADE_REG_ADDR;
    gstCoproModbusSlaveWrite.reg_num = 1;
    gstCoproModbusSlaveWrite.slave_addr = SLAVE_COPROCESSOR_ADDR;
    gstCoproModbusSlaveWrite.data[0] = status;
    if(QueuePut(&gModbusCoproCommSendQueue, &gstCoproModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
/**
  * @brief   监控卡给协处理器下发SOC值
  * @param   None
  * @return  None
  * @note    None
  */
bool SetPackSocInfo(uint16_t pack_soc)
{
    gstCoproModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gstCoproModbusSlaveWrite.reg_addr = MAIN_COP_DISP_SOC_REG_ADDR;
    gstCoproModbusSlaveWrite.reg_num = 1;
    gstCoproModbusSlaveWrite.slave_addr = SLAVE_COPROCESSOR_ADDR;
    gstCoproModbusSlaveWrite.data[0] = pack_soc;
    if(QueuePut(&gModbusCoproCommSendQueue, &gstCoproModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/**
  * @brief   按键灯轮询处理
  * @param   None
  * @return  None
  * @note    None
  */
void KeyLedProcess(void)
{
    uint8_t i = 0;
    for(i = 0; i < EN_KEY_SRC_NUM; i++)
    {
        if(DevKeyPressed((EN_KEY_SRC)i) == true)
        {
            if(i == EN_KEY_SRC_LEFT)
            {      
                SetConfirmLedStaInfo(1);     //正确映射
            }
            if(i == EN_KEY_SRC_RIGHT)
            {
                SetESCLedStaInfo(1);
            }
            if(i == EN_KEY_SRC_CONFIRM)
            {
                SetUpKeyLedStaInfo(1);
            }
            if(i == EN_KEY_SRC_ESC)
            {
                SetDownKeyLedStaInfo(1);
            }
        }
    }

}

static uint16_t _CheckSum(uint8_t *p, uint16_t size)
{
    uint16_t i;
    uint16_t ret = 0;
    for(i = 0; i < size; i++)
    {
        ret += p[i];
    }
    return ret;
}

void SendMainCopUpgradeData(uint16_t pack_id, uint16_t data_len, uint8_t *pdata)
{
    uint16_t check_sum;
    uint16_t signal_pack_size;
    signal_pack_size = data_len + 2; //单包长度为size + 拓展长度
    
    _garrTxData[0] = 0x5A;
    _garrTxData[1] = 0x00;
    _garrTxData[2] = 0x80;
    _garrTxData[3] = 0X80F1 & 0xFF;
    _garrTxData[4] = (0X80F1 >> 8) & 0xFF;
    _garrTxData[5] = signal_pack_size & 0xFF;
    _garrTxData[6] = (signal_pack_size >> 8) & 0xFF;
    _garrTxData[7] =  pack_id & 0xFF;
    _garrTxData[8] = (pack_id >> 8) & 0xFF;

//    spi_flash_read_bytes(BMS_FIRMWARE_ADDR + offset, &_upgrate_data[9], size);
    memcpy(&_garrTxData[9], pdata, data_len);
    check_sum = _CheckSum(_garrTxData, data_len + 9);

    _garrTxData[9 + data_len] = check_sum & 0xFF;
    _garrTxData[10 + data_len] = (check_sum >> 8) & 0xFF;

    ModbusCoprocessorRTUSend(_garrTxData, data_len + 11);
}

