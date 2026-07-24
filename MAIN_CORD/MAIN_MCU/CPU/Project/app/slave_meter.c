#include "slave_meter.h"
#include "dev_modbus.h"
#include "slave_collect.h"

#define SLAVE_METER_REV_MAX_TIME                                 1000
#define SLAVE_METER_INTERVAL_TIME                                100
#define SLAVE_METER_POLLING_TIME                                 500

#define METER_CONNECT                                           0x7FFF
#define METER_DISCONN                                           0x8000


static EN_METER_POLLING_STATUS      _geMeterSlavePollingStatus;
ModbusReadMeterPara_t               gstMeterPara;
MeterRealValue_t                    gstMeterRealValue;
SlaveRead_t                         gstModbusMeterRead;
SlaveWrite_t                        gstMeterModbusSlaveWrite;
Queue_t                             gModbusMeterCommSendQueue;
uint8_t                             garrMeterSendData[256];
static uint16_t                     _gMeterConnectSta;
static uint8_t                      _garrTxData[16];

typedef struct  _deal_fram_stru_t
{
    uint8_t                                 slave_addr;
    EN_METER_PARA_READ_EVENT                envent;
    uint8_t                                 func_code;
    uint16_t                                reg_addr;
    uint8_t                                 *pdata;
    uint16_t                                reg_num;
} MeterReadDealFram_t;


const MeterReadDealFram_t  garrMeterReadDealFramArr[] =
{
    {SLAVE_METER_ADDR, EN_READ_METER_VOLT_CURR_POWER_EVENT,     METER_READ_INPUT_TYPE,   METER_A_PHASE_VOLT,       (uint8_t *)&(gstMeterPara.volt_curr_power),     sizeof(gstMeterPara.volt_curr_power) / sizeof(uint16_t)},
    {SLAVE_METER_ADDR, EN_READ_METER_FERQUENCY_EVENT,           METER_READ_INPUT_TYPE,   METER_FERQUENCY,          (uint8_t *)&(gstMeterPara.freq),                sizeof(uint16_t)},
    {SLAVE_METER_ADDR, EN_READ_METER_T_ELEC_ENERGY_EVENT,       METER_READ_INPUT_TYPE,   METER_T_ELEC_ENERGY,      (uint8_t *)&(gstMeterPara.t_elec_enegy),        sizeof(uint16_t)},
};

#define MAX_SIZE       (sizeof(garrMeterReadDealFramArr)/ sizeof(garrMeterReadDealFramArr[0]))


static void _MeterSlaveReadErrOrDeal(uint8_t slave_addr, uint16_t comm_status)
{
    if(slave_addr == SLAVE_METER_ADDR)
    {
        if(comm_status == MBRTU_OK)
        {
            gstMeterPara.meter_comm_err_cnt = 0;
            gstMeterPara.meter_comm_time_out = 0;
        }
        else
        {
            gstMeterPara.meter_comm_err_cnt++;
            if(gstMeterPara.meter_comm_err_cnt > 10)
            {
                gstMeterPara.meter_comm_err_cnt = 10;
                gstMeterPara.meter_comm_time_out = 1;
                //超时或者数据错误 认为电表断开
                _gMeterConnectSta = gstCollectInfo.power_data.power_set_info.meter_brand & METER_DISCONN;
            }
        }
    }
}


uint16_t Modbus_Analysis(uint8_t *p_recv_buf, uint16_t rec_len, SlaveRead_t *p_FrameInfo, uint8_t *p_data)
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


static void _MeterAllReadPolling(SlaveRead_t *p_slave_frame)
{
    static uint8_t cnt = 0;
    static EN_METER_PARA_READ_EVENT e_event_count = EN_READ_METER_VOLT_CURR_POWER_EVENT;

    p_slave_frame->slave_addr = garrMeterReadDealFramArr[e_event_count].slave_addr;
    p_slave_frame->func_code  = garrMeterReadDealFramArr[e_event_count].func_code;
    p_slave_frame->reg_addr   = garrMeterReadDealFramArr[e_event_count].reg_addr;
    p_slave_frame->reg_num = garrMeterReadDealFramArr[e_event_count].reg_num;
    ModbusHostRead(p_slave_frame, ModbusVoltmeterRTUSend);
    e_event_count++;
    e_event_count %= EN_READ_METER_EVENT_NUM;
}

void SlaveMeterModbusCommProcess(void)
{
    uint8_t i = 0;
    static uint8_t index = 0;
    static ModbusRTURevData_t *p;
    static SlaveWrite_t *p_meterslavewrite;
    static uint32_t meter_read_polling_cnt = 0;

    switch(_geMeterSlavePollingStatus)
    {
        case EN_METER_POLLING_STATUS_IDLE:
        {
            /*空闲时段也要执行Get gModbusRevQueue, 并舍弃，防止队列满导致无法再接收数据*/
            QueueGet(&gstDevModbusVoltmeterArc.rev_queue, (void *)&p);
            if(QueueGet(&gModbusMeterCommSendQueue, (void *)&p_meterslavewrite) == true)/*如果有主动写操作*/
            {
                if(p_meterslavewrite->func_code == 0x00)
                {
                    //私有协议
                    ModbusVoltmeterRTUSend((uint8_t *)p_meterslavewrite->data, 10);
                    _geMeterSlavePollingStatus = EN_METER_POLLING_STATUS_REV_INTERVAL_WAIT;
                    meter_read_polling_cnt = tim_get_systime();
                }
                else
                {
                    gstModbusMeterRead.func_code = p_meterslavewrite->func_code;
                    gstModbusMeterRead.slave_addr = p_meterslavewrite->slave_addr;
                    ModbusHostWrite(p_meterslavewrite, ModbusVoltmeterRTUSend, garrMeterSendData);
                    _geMeterSlavePollingStatus = EN_METER_POLLING_STATUS_WAITING_REV;
                    meter_read_polling_cnt = tim_get_systime();
                }
            }
            else
            {
                /*轮询回读*/
                //定期主动发送主机地址给电表
                if((uint32_t)(tim_get_systime() - meter_read_polling_cnt) > SLAVE_METER_POLLING_TIME)
                {
                    meter_read_polling_cnt = tim_get_systime();
                    _MeterAllReadPolling(&gstModbusMeterRead);
                    _geMeterSlavePollingStatus = EN_METER_POLLING_STATUS_WAITING_REV;
                }
            }
            break;
        }
        case EN_METER_POLLING_STATUS_WAITING_REV:
        {
            if(QueueGet(&gstDevModbusVoltmeterArc.rev_queue, (void *)&p) == true)
            {
                meter_read_polling_cnt = tim_get_systime();
                for(i = 0; i < MAX_SIZE; i++)
                {
                    if((garrMeterReadDealFramArr[i].func_code == gstModbusMeterRead.func_code) &&
                            (garrMeterReadDealFramArr[i].slave_addr == gstModbusMeterRead.slave_addr) &&
                            (garrMeterReadDealFramArr[i].reg_addr == gstModbusMeterRead.reg_addr))
                    {
                        break;          //确保是读取的信息
                    }
                }

                if(i == MAX_SIZE)
                {
                    //是写命令返回，没有进行判断。
                    _geMeterSlavePollingStatus = EN_METER_POLLING_STATUS_REV_OVERTIME;
                    break;
                }
                if(Modbus_Analysis((uint8_t *)p->p_data, p->data_size, &gstModbusMeterRead, garrMeterReadDealFramArr[i].pdata) == MBRTU_OK)
                {
                    //数据正常
                    _MeterSlaveReadErrOrDeal(gstModbusMeterRead.slave_addr, MBRTU_OK);
                }
                else
                {
                    //数据异常
                    _MeterSlaveReadErrOrDeal(gstModbusMeterRead.slave_addr, MBRTU_PARA_INVALID);
                }
                _geMeterSlavePollingStatus = EN_METER_POLLING_STATUS_REV_INTERVAL_WAIT;
            }
            if((uint32_t)(tim_get_systime() - meter_read_polling_cnt) > SLAVE_METER_REV_MAX_TIME)
            {
                meter_read_polling_cnt = tim_get_systime();
                _geMeterSlavePollingStatus = EN_METER_POLLING_STATUS_REV_OVERTIME;
            }
            break;
        }
        case EN_METER_POLLING_STATUS_REV_OVERTIME:
        {
            /* 做超时处理*/
            meter_read_polling_cnt = tim_get_systime();
            _geMeterSlavePollingStatus = EN_METER_POLLING_STATUS_IDLE;
            _MeterSlaveReadErrOrDeal(gstModbusMeterRead.slave_addr, MBRTU_PARA_INVALID);
            break;
        }
        case EN_METER_POLLING_STATUS_REV_INTERVAL_WAIT:
        {
            if((uint32_t)(tim_get_systime()) - meter_read_polling_cnt > SLAVE_METER_INTERVAL_TIME)
            {
                _geMeterSlavePollingStatus = EN_METER_POLLING_STATUS_IDLE;
                meter_read_polling_cnt = tim_get_systime();
            }
            break;
        }
        default:
        {
            _geMeterSlavePollingStatus = EN_METER_POLLING_STATUS_IDLE;
            break;
        }
    }
}

/**
  * @brief  数据转换
  * @param  NULL
  * @retval NULL
  */
static float _ByteConvert(uint16_t* p_value)
{
    union
    {
        uint32_t    int_value;
        float       float_value;
    } convert;

    convert.int_value = (p_value[0] << 16 | p_value[1]);

    return convert.float_value;
}

/**
  * @brief  数据解析  获取电表数据
  * @param  NULL
  * @retval NULL
  */
MeterRealValue_t GetMeterAllValue(void)
{
    MeterRealValue_t st_meter_value;
    
    st_meter_value.meter_a_volt = (_ByteConvert(gstMeterPara.volt_curr_power.Ua_volt)) * 0.1f;
    st_meter_value.meter_b_volt = (_ByteConvert(gstMeterPara.volt_curr_power.Ub_volt)) * 0.1f;
    st_meter_value.meter_c_volt = (_ByteConvert(gstMeterPara.volt_curr_power.Uc_volt)) * 0.1f;

    st_meter_value.meter_a_curr = (_ByteConvert(gstMeterPara.volt_curr_power.Ia_curr) * 1 * 0.001f);
    st_meter_value.meter_b_curr = (_ByteConvert(gstMeterPara.volt_curr_power.Ib_curr) * 1 * 0.001f);
    st_meter_value.meter_c_curr = (_ByteConvert(gstMeterPara.volt_curr_power.Ic_curr) * 1 * 0.001f);

    st_meter_value.meter_a_active_power = (_ByteConvert(gstMeterPara.volt_curr_power.Pa_power)) * 1 * 0.1f;
    st_meter_value.meter_b_active_power = (_ByteConvert(gstMeterPara.volt_curr_power.Pb_power)) * 1 * 0.1f;
    st_meter_value.meter_c_active_power = (_ByteConvert(gstMeterPara.volt_curr_power.Pc_power)) * 1 * 0.1f;
    st_meter_value.meter_t_active_power = (_ByteConvert(gstMeterPara.volt_curr_power.Pt_power)) * 1 * 0.1f;
    st_meter_value.meter_a_reactive_power = (_ByteConvert(gstMeterPara.volt_curr_power.Qa_power)) * 1 * 0.1f;
    st_meter_value.meter_b_reactive_power = (_ByteConvert(gstMeterPara.volt_curr_power.Qb_power)) * 1 * 0.1f;
    st_meter_value.meter_c_reactive_power = (_ByteConvert(gstMeterPara.volt_curr_power.Qc_power)) * 1 * 0.1f;
    st_meter_value.meter_t_reactive_power = (_ByteConvert(gstMeterPara.volt_curr_power.Qt_power)) * 1 * 0.1f;

    st_meter_value.meter_frequency = (_ByteConvert(gstMeterPara.freq)) * 0.01f;
    st_meter_value.meter_t_elec_enegy = (_ByteConvert(gstMeterPara.t_elec_enegy)) * 1.0f * 1.0f;

    return st_meter_value;
}
/**
  * @brief  获取电表电流值
  * @param  NULL
  * @retval NULL
  */
int16_t GetMeterCurrValue(void)
{
    float curr_tmp = 0;
    int16_t current_val = 0;
    
    curr_tmp = (_ByteConvert(gstMeterPara.volt_curr_power.Ia_curr) * 1 * 0.001f);
    current_val = (int16_t)(curr_tmp * 10);    //电流单位为0.1A
    
    return current_val;
}

/**
  * @brief  获取电表电压值
  * @param  NULL
  * @retval NULL
  */
void CheckMeterConnectStaus(void)
{
    float a_volt = 0;
    float b_volt = 0;
    float c_volt = 0;
    
    a_volt = (_ByteConvert(gstMeterPara.volt_curr_power.Ua_volt)) * 0.1f;
    b_volt = (_ByteConvert(gstMeterPara.volt_curr_power.Ub_volt)) * 0.1f;
    c_volt = (_ByteConvert(gstMeterPara.volt_curr_power.Uc_volt)) * 0.1f;

    if((a_volt > 210.0f && a_volt < 240.0f) && 
       (b_volt > 210.0f && b_volt < 240.0f) && 
       (c_volt > 210.0f && c_volt < 240.0f))  //电压正常 电表连接
    {
        _gMeterConnectSta = gstCollectInfo.power_data.power_set_info.meter_brand & METER_CONNECT;
    }
    else  //电压异常 电表断开
    {
        _gMeterConnectSta = gstCollectInfo.power_data.power_set_info.meter_brand | METER_DISCONN;
    } 
}
/**
  * @brief  返回电表连接与否
  * @param  NULL
  * @retval NULL
  */
uint16_t GetMeterConnectSta(void)
{
    return _gMeterConnectSta;
}
