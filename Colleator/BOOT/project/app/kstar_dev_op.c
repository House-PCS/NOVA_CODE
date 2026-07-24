
#include "kstar_dev_op.h"
#include "sys_info.h"

Queue_t    gKstarOpQueue;

static uint8_t  _gKstarReqPacket[320];

static uint16_t  _gKstarBaseInfoCache[800];
static uint16_t  _gKstarSysInfoCache[40];

static KstarOpInfo_t _gKstarOpInfo;

/*第一个字段是寄存器起始地址，第二个字段是寄存器数量*/
static uint16_t _gKstarBaseInfoRegDiv[][2] = {{3000, 87}, {3097, 91}, {3193, 53}, {3251, 68}, 
                                              {3441, 34}, {3485, 34}, {3529, 34}, {3573, 34}, {3617, 34}, {3661, 34}, {3705, 34}, {3749, 34}};
static bool     _gKstarBaseInfoGetFlag[] =   {false,      false,      false,      false,
                                              false,      false,      false,      false,       false,      false,      false,      false};

                                                  
void KstarDevOpInit()
{
    QueueInit(&gKstarOpQueue, KSTAR_DEV_OP_QUEUE_DEPTH, sizeof(KstarDevOp_t));
}







/**
  * @brief  Send Modbus func04 Data.
  * @param  modbus format
  * @return NULL
  * @note   none
  */
static void _ModbusReadInput(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num)
{
    p_packet[0] = slave_addr;
    p_packet[1] = EN_MODBUS_FUNC_CODE_READ_INPUT;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (reg_num >> 8) & 0xFF;
    p_packet[5] = reg_num & 0xFF;
    
    ModbusRTUSend(p_packet, 6);
}


/**
  * @brief  Send Modbus func03 Data.
  * @param  modbus format
  * @return NULL
  * @note   none
  */
static void _ModbusReadRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num)
{
    p_packet[0] = slave_addr;
    p_packet[1] = EN_MODBUS_FUNC_CODE_READ_REMAIN;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (reg_num >> 8) & 0xFF;
    p_packet[5] = reg_num & 0xFF;
    
    ModbusRTUSend(p_packet, 6);
}


/**
  * @brief  Send Modbus func10 Data.
  * @param  modbus format
  * @return NULL
  * @note   none
  */
static void _ModbusWriteMultiRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_value)
{
    p_packet[0] = slave_addr;
    p_packet[1] = EN_MODBUS_FUNC_CODE_WRITE_MULTI;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (reg_num >> 8) & 0xFF;
    p_packet[5] = reg_num & 0xFF;
    p_packet[6] = reg_num * 2;
    memcpy(&p_packet[7], p_reg_value, p_packet[6]);
    
    ModbusRTUSend(p_packet, p_packet[6] + 7);
}


/**
  * @brief  Send Modbus func06 Data.
  * @param  modbus format
  * @return NULL
  * @note   none
  */
static void _ModbusWriteSingleRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t value)
{
    p_packet[0] = slave_addr;
    p_packet[1] = EN_MODBUS_FUNC_CODE_WRITE_MULTI;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (value >> 8) & 0xFF;
    p_packet[5] = value & 0xFF;
    
    ModbusRTUSend(p_packet, 6);
}




/**
  * @brief  Kstar base info read.
  * @param  modbus format
  * @return NULL
  * @note   寄存器较多，分多次回读，3000~3086, 3097~3187, 3193~3245, 3251~3318, 
                               PACK:3441~3474, 3485~3518, 3529~3562, 3573~3606, 3617~3650, 3661~3694, 3705~3738, 3749~3782
  */
static void _KstarReadBaseInfo(uint16_t reg_addr, uint16_t reg_num)
{
    _ModbusReadInput(_gKstarReqPacket, KSTAR_DEV_ADDR, reg_addr, reg_num);
}

/**
  * @brief  Kstar sys info read.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _KstarReadSysInfo(uint16_t reg_addr, uint16_t reg_num)
{
    _ModbusReadRemain(_gKstarReqPacket, KSTAR_DEV_ADDR, reg_addr, reg_num);
}







/**
  * @brief  Kstar base info copy to gKstarInvBaseInfo.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _KstarBaseInfoUpdate()
{
    uint16_t *p_data = (uint16_t *)&gKstarInvBaseInfo;
    uint8_t *p_origin = (uint8_t *)_gKstarBaseInfoCache;

    for(uint16_t i = 0; i < sizeof(KstarInvBaseInfo_t) / 2; i++)
    {
        p_data[i] = (p_origin[2 * i] << 8) | p_origin[2 * i + 1];
    }
    gCollectorDevInfo.kstar_base_info_updated = true;
}

/**
  * @brief  Kstar sys info copy to gKstarInvSysInfo.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _KstarSysInfoUpdate()
{
    uint16_t *p_data = (uint16_t *)&gKstarInvSysInfo;
    uint8_t *p_origin = (uint8_t *)_gKstarSysInfoCache;

    for(uint16_t i = 0; i < sizeof(KstarInvSysInfo_t) / 2; i++)
    {
        p_data[i] = (p_origin[2 * i] << 8) | p_origin[2 * i + 1];
    }
    gCollectorDevInfo.kstar_sys_info_updated = true;
}







/*大轮询调用*/
void KstarDevOpProcess()
{
    static uint32_t timing_read_count = 0xFFFFFFFF - 5 * 1000;
    static KstarDevOp_t *p_dev_op_get;
    static ModbusRTURevData_t *p_modbus_dev_rev;
    /*每隔一定时间，发送读设备寄存器队列*/
    if((uint32_t)(tim_get_systime() - timing_read_count) > gCollectorDevInfo.read_pcs_period * 1000)
    {
        KstarDevOp_t dev_op;
        
        dev_op.slave_addr = KSTAR_DEV_ADDR;
        
        /*sys info*/ //先读sys info(非关键信息，避免系统一上电，串口第一个字节多带一个0x00导致读取失败的情况)
        dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;
        dev_op.op_type = EN_KSTAR_OP_TYPE_READ_SYS_INFO;
        dev_op.reg_addr = 3200;
        dev_op.reg_num = 19;
        QueuePut(&gKstarOpQueue, &dev_op);
        
        /*base info*/
        dev_op.op_type = EN_KSTAR_OP_TYPE_READ_BASE_INFO;
        for(uint8_t i = 0; i < sizeof(_gKstarBaseInfoRegDiv)/sizeof(_gKstarBaseInfoRegDiv[0]); i++)
        {
            dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_INPUT;
            dev_op.reg_addr = _gKstarBaseInfoRegDiv[i][0];
            dev_op.reg_num = _gKstarBaseInfoRegDiv[i][1];
            QueuePut(&gKstarOpQueue, &dev_op);
        }
        
        timing_read_count = tim_get_systime();
    }
    
    switch(_gKstarOpInfo.status)
    {
        case EN_KSTAR_OP_STATUS_IDLE:
        {
            /*IDLE模式下不接受从机发的数据*/
            QueueClear(&gModbusMTURevQueue);
            /*检测gKstarOpQueue队列，判断是否有需要操作从机的命令*/
            if(QueueGet(&gKstarOpQueue, (void *)&p_dev_op_get) == true)
            {
                switch(p_dev_op_get->op_type)
                {
                    case EN_KSTAR_OP_TYPE_READ_BASE_INFO:
                    {
                        if(p_dev_op_get->reg_addr >= _gKstarBaseInfoRegDiv[4 + _gKstarOpInfo.pack_num][0])/*自动跳过不存在的电池包数据获取*/
                        {
                            break;
                        }
                        _KstarReadBaseInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
                        _gKstarOpInfo.status = EN_KSTAR_OP_STATUS_WAIT_ACK;
                        _gKstarOpInfo.op_timing_count = tim_get_systime();
                        
                        break;
                    }
                    case EN_KSTAR_OP_TYPE_READ_SYS_INFO:
                    {
                        _KstarReadSysInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
                        _gKstarOpInfo.status = EN_KSTAR_OP_STATUS_WAIT_ACK;
                        _gKstarOpInfo.op_timing_count = tim_get_systime();
                        break;
                    }
                    default:break;
                }
            }
            break;
        }
        case EN_KSTAR_OP_STATUS_WAIT_ACK:
        {
            if(QueueGet(&gModbusMTURevQueue, (void *)&p_modbus_dev_rev) == true)
            {
                uint8_t rev_slave_addr = p_modbus_dev_rev->p_data[0];
                uint8_t rev_func_code = p_modbus_dev_rev->p_data[1];
                
                if(rev_slave_addr != p_dev_op_get->slave_addr)
                {
                    break;//不是给本设备的包
                }
                if((rev_func_code & 0x80) == 0x80)//逆变器设备通信异常
                {
                    _gKstarOpInfo.rev_error_code = rev_func_code & 0x7F;
                    _gKstarOpInfo.slave_ack_timeout = false;
                    _gKstarOpInfo.status = EN_KSTAR_OP_STATUS_WAIT_GAP;
                    _gKstarOpInfo.op_timing_count = tim_get_systime();
                    gCollectorDevInfo.inv_comm_flag = EN_INV_COMM_ERROR_CODE;
                    break;
                }
                if(rev_func_code != p_dev_op_get->func_code)
                {
                    break;//功能码不匹配
                }
                gCollectorDevInfo.inv_comm_flag = EN_INV_COMM_OK;
                switch(p_dev_op_get->op_type)
                {
                    case EN_KSTAR_OP_TYPE_READ_BASE_INFO:
                    {
                        uint16_t byte_num = p_modbus_dev_rev->p_data[2];
                        
                        if(p_dev_op_get->reg_num * 2 != byte_num)//数据请求和回读的长度不匹配
                        {
                            _gKstarOpInfo.rev_error_code = EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG;
                            _gKstarOpInfo.slave_ack_timeout = false;
                            _gKstarOpInfo.status = EN_KSTAR_OP_STATUS_WAIT_GAP;
                            _gKstarOpInfo.op_timing_count = tim_get_systime();
                        }
                        else//数据OK
                        {
                            uint8_t index = 0;
                            
                            for(index = 0; index < sizeof(_gKstarBaseInfoRegDiv)/sizeof(_gKstarBaseInfoRegDiv[0]); index++)
                            {
                                if(_gKstarBaseInfoRegDiv[index][0] == p_dev_op_get->reg_addr)
                                {
                                    break;
                                }
                            }
                            if(index == sizeof(_gKstarBaseInfoRegDiv)/sizeof(_gKstarBaseInfoRegDiv[0]))
                            {
                                _gKstarOpInfo.status = EN_KSTAR_OP_STATUS_WAIT_GAP;
                                _gKstarOpInfo.op_timing_count = tim_get_systime();
                                break;
                            }
                            
                            memcpy((uint8_t *)&_gKstarBaseInfoCache[p_dev_op_get->reg_addr - 3000], &p_modbus_dev_rev->p_data[3], byte_num);
                            _gKstarBaseInfoGetFlag[index] = true;
                            _gKstarOpInfo.status = EN_KSTAR_OP_STATUS_ACK_ANALYSIS;
                            _gKstarOpInfo.op_timing_count = tim_get_systime();
                            
                            if(index == 0)//第一包中包含了电池包数，3077
                            {
                                _gKstarOpInfo.pack_num = (p_modbus_dev_rev->p_data[154 + 3] << 8) | p_modbus_dev_rev->p_data[155 + 3];
                                if(_gKstarOpInfo.pack_num > 8)
                                {
                                    _gKstarOpInfo.pack_num = 8;
                                }
                            }
                        }
                        break;
                    }
                    case EN_KSTAR_OP_TYPE_READ_SYS_INFO:
                    {
                        uint16_t byte_num = p_modbus_dev_rev->p_data[2];
                        
                        if(p_dev_op_get->reg_num * 2 != byte_num)//数据请求和回读的长度不匹配
                        {
                            _gKstarOpInfo.rev_error_code = EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG;
                            _gKstarOpInfo.slave_ack_timeout = false;
                            _gKstarOpInfo.status = EN_KSTAR_OP_STATUS_WAIT_GAP;
                            _gKstarOpInfo.op_timing_count = tim_get_systime();
                        }
                        else//数据OK
                        {
                            memcpy((uint8_t *)&_gKstarSysInfoCache[p_dev_op_get->reg_addr - 3200], &p_modbus_dev_rev->p_data[3], byte_num);
                            _gKstarOpInfo.status = EN_KSTAR_OP_STATUS_ACK_ANALYSIS;
                            _gKstarOpInfo.op_timing_count = tim_get_systime();
                        }
                    }
                    default:break;
                }
            }
            else
            {
                if((uint32_t)(tim_get_systime() - _gKstarOpInfo.op_timing_count) > KSTAR_DEV_ACK_TIME_OUT)
                {
                    _gKstarOpInfo.slave_ack_timeout = true;
                    _gKstarOpInfo.status = EN_KSTAR_OP_STATUS_WAIT_GAP;
                    _gKstarOpInfo.op_timing_count = tim_get_systime();
                    gCollectorDevInfo.inv_comm_flag = EN_INV_COMM_FAIL;
                }
            }
            break;
        }
        case EN_KSTAR_OP_STATUS_ACK_ANALYSIS:
        {
            uint8_t base_info_div_num = 4 + _gKstarOpInfo.pack_num;
            if(p_dev_op_get->op_type == EN_KSTAR_OP_TYPE_READ_BASE_INFO)
            {
                /*base info copy*/
                uint8_t index = 0;
                for(index = 0; index < base_info_div_num; index++)
                {
                    if(_gKstarBaseInfoGetFlag[index] == false)
                    {
                        break;
                    }
                }
                if(index == base_info_div_num)
                {
                    _KstarBaseInfoUpdate();
                }
            }
            else if(p_dev_op_get->op_type == EN_KSTAR_OP_TYPE_READ_SYS_INFO)
            {
                /*sys info copy*/
                _KstarSysInfoUpdate();
            }
            _gKstarOpInfo.status = EN_KSTAR_OP_STATUS_WAIT_GAP;
            _gKstarOpInfo.op_timing_count = tim_get_systime();
            break;
        }
        case EN_KSTAR_OP_STATUS_WAIT_GAP:
        {
            if((uint32_t)(tim_get_systime() - _gKstarOpInfo.op_timing_count) > KSTAR_DEV_POLLING_GAP)
            {
                _gKstarOpInfo.status = EN_KSTAR_OP_STATUS_IDLE;
            }
            break;
        }
        
        default:break;
    }
    
    
    if(_gKstarOpInfo.status != EN_KSTAR_OP_STATUS_IDLE)
    {
        if(gCollectorDevInfo.inv_comm_flag != EN_INV_COMM_OK)
        {
            if(p_dev_op_get->op_type == EN_KSTAR_OP_TYPE_READ_BASE_INFO)
            {
                gCollectorDevInfo.kstar_base_info_updated = false;
            }
            else if(p_dev_op_get->op_type == EN_KSTAR_OP_TYPE_READ_SYS_INFO)
            {
                gCollectorDevInfo.kstar_sys_info_updated = false;
            }
        }
    }
}









