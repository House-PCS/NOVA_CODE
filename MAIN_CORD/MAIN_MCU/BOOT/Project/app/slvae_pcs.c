#include "slave_pcs.h"
#include "dev_modbus.h"
#include "public_proto.h"


#define SLAVE_COMM_REV_MAX_TIME                     500
#define SLAVE_COMM_INTERVAL_TIME                    500//150
#define SLAVE_COMM_POLLING_TIME                     150//300


static EN_PCS_ARC_READ_EVENT        _PcsArcSlaveModbusCommReadEvent;
static EN_PCS_ARC_POLLING_STATUS    _PcsArcSlavePollingStatus;


Queue_t	                gModbusPcsArcCommSendQueue;
SlaveRead_t             gModbusPcsArcRead;
SlaveWrite_t            gPcsArcModbusSlaveWrite;
uint8_t                 gPcsArcSendData[300];
ModBusPcsArcPara_t      gPcsArcSlavePara;

bool SetArcTestWriteData(void);
typedef struct  _deal_fram_stru_t
{
    uint8_t                                 slave_addr;
    EN_PCS_ARC_READ_EVENT                   envent;
    uint8_t                                 func_code;
    uint16_t                                reg_addr;
    uint8_t                                 *pdata; 
    uint16_t                                reg_num;
}PcsArcReadDealFram_t;

const PcsArcReadDealFram_t  gPcsArcReadDealFramArr[]=
{
    {SLAVE_ARC_ADDR, EN_ARC_READ_INPUT_ARC_EVENT,READ_INPUT_TYPE,   ARC_REG_IN_ADDR_MODE_ID,            (uint8_t *)&(gPcsArcSlavePara.arc_para.arc_input_para),     sizeof(ARCInputPara_t)/sizeof(uint16_t)},
    {SLAVE_ARC_ADDR, EN_ARC_READ_HOLD_EVENT,     READ_HOLD_TYPE,    ARC_REG_HOLD_SLAVE_ADDR,              (uint8_t *)&(gPcsArcSlavePara.arc_para.arc_hold_para),      sizeof(ARCHoldPara_t)/sizeof(uint16_t)},
    {SLAVE_PCS_ADDR, EN_PCS_READ_FRONT_EVENT,    READ_INPUT_TYPE,   PCS_REG_IN_ADDR_MODE_ID,            (uint8_t *)&(gPcsArcSlavePara.pcs_para.pcs_input_front_para),sizeof(PCSInputFrontPara_t)/sizeof(uint16_t)},
    {SLAVE_PCS_ADDR, EN_PCS_READ_BACK_EVENT,     READ_INPUT_TYPE,   PCS_REG_IN_ADDR_LOAD_CONSUM_ON_DAY, (uint8_t *)&(gPcsArcSlavePara.pcs_para.pcs_input_back_para), sizeof(PCSInputbackPara_t)/sizeof(uint16_t)},
    {SLAVE_PCS_ADDR, EN_PCS_HOLD_EVENT,          READ_HOLD_TYPE,    PCS_REG_HOLD_ADDR_BATT_CHG_VOLT,    (uint8_t *)&(gPcsArcSlavePara.pcs_para.pcs_hold_para),       sizeof(PCSHoldPara_t)/sizeof(uint16_t)},

};
#define     MAX_SIZE    (sizeof(gPcsArcReadDealFramArr)/sizeof(gPcsArcReadDealFramArr[0]))

void SlavePcsArcCommInit()
{

    uint16_t  len= 0;
    len = sizeof(SlaveWrite_t);
    QueueInit(&gModbusPcsArcCommSendQueue, PCS_ARC_WRITE_QUEUE_DEPTH, sizeof(SlaveWrite_t));
}

uint8_t     *slave_data_addr =0;
static void _PcsArcAllReadPolling(SlaveRead_t *p_slave_frame)
{

    static uint8_t cnt = 0;
    static EN_PCS_ARC_READ_EVENT event_count = EN_ARC_READ_INPUT_ARC_EVENT;

    p_slave_frame->slave_addr = gPcsArcReadDealFramArr[event_count].slave_addr;
    p_slave_frame->func_code  = gPcsArcReadDealFramArr[event_count].func_code;
    p_slave_frame->reg_addr   = gPcsArcReadDealFramArr[event_count].reg_addr;
    p_slave_frame->reg_num = gPcsArcReadDealFramArr[event_count].reg_num;
    ModbusHostRead(p_slave_frame,ModbusPCSRTUSend);
    event_count++;
    event_count %= EN_PCS_ARC_READ_EVENT_NUM; 
       
}

void    _PcsArcSlaveReadErrorDeal(uint8_t  slave_addr,uint16_t comm_status)
{
    if(slave_addr == SLAVE_ARC_ADDR)
    {
        if(comm_status == MBRTU_OK)
        {
            gPcsArcSlavePara.arc_comm_err_cnt = 0;
        }   
        else
        {
            gPcsArcSlavePara.arc_comm_err_cnt++;
            if(gPcsArcSlavePara.arc_comm_err_cnt > 5)
            {
                gPcsArcSlavePara.arc_comm_err_cnt = 5;
                gPcsArcSlavePara.arc_comm_time_out = 1;
            }
        }
    }
    else if(slave_addr == SLAVE_PCS_ADDR)
    {
        if(comm_status == MBRTU_OK)
        {
            gPcsArcSlavePara.pcs_comm_err_cnt = 0;
        }   
        else
        {
            gPcsArcSlavePara.pcs_comm_err_cnt++;
            if(gPcsArcSlavePara.pcs_comm_err_cnt > 5)
            {
                gPcsArcSlavePara.pcs_comm_err_cnt = 5;
                gPcsArcSlavePara.pcs_comm_time_out = 1;
            }
        }        
    }     
}

uint8_t  tset_en=0;

void TsetPcsInputData(void)
{
    uint16_t i=0;
    uint8_t *p;

    p = (uint8_t *)&gPcsArcSlavePara.pcs_para.pcs_input_front_para;
    for(i=0;i<sizeof(PCSInputFrontPara_t);i++)
    {
        *p=i;
        p++;
    }

    p = (uint8_t *)&gPcsArcSlavePara.pcs_para.pcs_input_back_para;
    for(i=0;i<sizeof(PCSInputbackPara_t);i++)
    {
        *p=i;
        p++;
    }

    p = (uint8_t *)&gPcsArcSlavePara.pcs_para.pcs_hold_para;
    for(i=0;i<sizeof(PCSHoldPara_t);i++)
    {
        *p=i;
        p++;
    }

    p = (uint8_t *)&gPcsArcSlavePara.arc_para;
    for(i=0;i<sizeof(ARCPara_t);i++)
    {
        *p=i;
        p++;
    }
}


void    SlaveArcModbusCommProcess(void)
{
    uint8_t i = 0;
    static ModbusRTURevData_t *p;
    static SlaveWrite_t *pPcsArcSlaveWrite;
    static uint32_t pcs_arc_read_polling_cnt = 0;



//    TsetPcsInputData();


    switch(_PcsArcSlavePollingStatus)
    {
        case EN_PCS_ARC_POLLING_STATUS_IDLE:
        {
            /*空闲时段也要执行Get gModbusRevQueue, 并舍弃，防止队列满导致无法再接收数据*/
            QueueGet(&gDevModbusPCS.rev_queue, (void *)&p);
            if(QueueGet(&gModbusPcsArcCommSendQueue, (void *)&pPcsArcSlaveWrite) == true)/*如果有主动写操作*/
            {
                gModbusPcsArcRead.func_code = pPcsArcSlaveWrite->func_code;  
                ModbusHostWrite(pPcsArcSlaveWrite,ModbusPCSRTUSend,gPcsArcSendData);
                _PcsArcSlavePollingStatus = EN_PCS_ARC_POLLING_STATUS_WAITING_REV;
                pcs_arc_read_polling_cnt = tim_get_systime();  
            }
            else
            {
                /*轮询回读*/
                //定期主动发送主机地址给BMS

                if((uint32_t)(tim_get_systime() - pcs_arc_read_polling_cnt) > SLAVE_COMM_POLLING_TIME)
                {
                    pcs_arc_read_polling_cnt = tim_get_systime();
                    _PcsArcAllReadPolling(&gModbusPcsArcRead);
                    _PcsArcSlavePollingStatus = EN_PCS_ARC_POLLING_STATUS_WAITING_REV;
                }
            }
            break;
        }
        case EN_PCS_ARC_POLLING_STATUS_WAITING_REV:
        {
            if(QueueGet(&gDevModbusPCS.rev_queue, (void *)&p) == true)
            {
                pcs_arc_read_polling_cnt = tim_get_systime();                  
                for(i = 0; i<=MAX_SIZE ;i++)
                {
                    if((gPcsArcReadDealFramArr[i].func_code == gModbusPcsArcRead.func_code) && (gPcsArcReadDealFramArr[i].reg_addr == gModbusPcsArcRead.reg_addr))
                    {
                        break;  //查找出所读取的目的地址
                    }
                }
                if(i == MAX_SIZE)
                {
                    _PcsArcSlavePollingStatus = EN_PCS_ARC_POLLING_STATUS_REV_INTERVAL_WAIT;
                    break;
                }
                if(Modebus_ParseUp((uint8_t *)p->p_data, p->data_size, &gModbusPcsArcRead, gPcsArcReadDealFramArr[i].pdata) == MBRTU_OK)
                {
  
                    _PcsArcSlaveReadErrorDeal(gModbusPcsArcRead.slave_addr,MBRTU_OK);
                }
                else
                {//数据异常

                    _PcsArcSlaveReadErrorDeal(gModbusPcsArcRead.slave_addr,MBRTU_PARA_INVALID);
                }

                _PcsArcSlavePollingStatus = EN_PCS_ARC_POLLING_STATUS_REV_INTERVAL_WAIT;

            }
            if((uint32_t)(tim_get_systime() - pcs_arc_read_polling_cnt) > SLAVE_COMM_REV_MAX_TIME)
            {
                pcs_arc_read_polling_cnt  = tim_get_systime();
                _PcsArcSlavePollingStatus = EN_PCS_ARC_POLLING_STATUS_REV_OVERTIME;
            }
            break;
        }
        case EN_PCS_ARC_POLLING_STATUS_REV_OVERTIME:
        {
            /*做传输超时的处理*/
            _PcsArcSlavePollingStatus = EN_PCS_ARC_POLLING_STATUS_IDLE;
            _PcsArcSlaveReadErrorDeal(gModbusPcsArcRead.slave_addr,MBRTU_PARA_INVALID);
            break;
        }
        case EN_PCS_ARC_POLLING_STATUS_REV_INTERVAL_WAIT:
        {
            if((uint32_t)(tim_get_systime() - pcs_arc_read_polling_cnt) > SLAVE_COMM_INTERVAL_TIME)
            {
                _PcsArcSlavePollingStatus = EN_PCS_ARC_POLLING_STATUS_IDLE;
                pcs_arc_read_polling_cnt = tim_get_systime();    

            }
            break;
        }
        default:
        _PcsArcSlavePollingStatus = EN_PCS_ARC_POLLING_STATUS_IDLE;
        break;
    }        
}


//bool SetArcTestWriteData(void)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = 0x0103;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_ARC_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = 0x1234;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;
//}


//bool SetArcSlaveAddr(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = ARC_REG_HOLD_SET_SLAVE_ADDR_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_ARC_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;        
//}


//bool SetArcCommBaud(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = ARC_REG_HOLD_SET_COMM_BAUD_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_ARC_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;        
//}


//bool SetArcSelfTestCmd(uint16_t cmd)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = ARC_REG_HOLD_ARC_SELFTEST_CMD_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_ARC_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = cmd;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;        
//}

//bool SetArcAlarmThreshold(uint8_t channel,uint16_t val)
//{
//    if((channel > 8) ||(channel == 0))
//    {
//        return false;
//    }
//    gPcsArcModbusSlaveWrite.reg_addr = (ARC_REG_HOLD_ARC_ALARM_BASE_THRESHOLD_ADDR+channel-1);
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_ARC_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true; 
//}

//bool SetImpedanceAlarmRxThreshold(uint8_t channel,uint16_t val)
//{
//    if((channel > 8) ||(channel == 0))
//    {
//        return false;
//    }
//    gPcsArcModbusSlaveWrite.reg_addr = (ARC_REG_IMPEDANCE_ALARM_BASE_THRESHOLD_RX_ADDR+channel-1);
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_ARC_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true; 
//}

//bool SetImpedanceAlarmRyThreshold(uint8_t channel,uint16_t val)
//{
//    if((channel > 8) ||(channel == 0))
//    {
//        return false;
//    }
//    gPcsArcModbusSlaveWrite.reg_addr = (ARC_REG_IMPEDANCE_ALARM_BASE_THRESHOLD_RY_ADDR+channel-1);
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_ARC_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true; 
//}


//bool PcsSetBattChgVolt(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_BATT_CHG_VOLT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetBattChgCurr(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_BATT_CHG_CURR_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetBattChgPower(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_BATT_CHG_POWER_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}
//bool PcsSetChgInputType(uint16_t type)
//{
//    if(type > 4)
//    {
//        return false;
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_CHG_INPUT_TYPE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = type;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetAcOutputVoltRange(uint16_t volt_range)
//{
//    if(volt_range > 4)
//    {
//        return false;
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_AC_OUT_VOLT_LEVEL_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = volt_range;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetAcOutputFreqRange(uint16_t freq_range)
//{
//    if(freq_range > 2)
//    {
//        return false;
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_AC_OUT_FREQ_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = freq_range;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetGridStandard(uint16_t grid_standard)
//{
//    if(grid_standard > 0x18)
//    {
//        return false;
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_GRID_STANDARD_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = grid_standard;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}



//bool PcsSetGridStandardSegmentation(uint16_t grid_standard_seg)
//{
//    if(grid_standard_seg > 2)
//    {
//        return false;
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_GRID_STANDARD_SEG_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = grid_standard_seg;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}



//bool PcsSetBackupOutputStatus(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_BACKUP_OUTPUT_ENABLE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetBattDsgCutOfVoltValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_BATT_DSG_CUT_VOLT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetBattChgVoltProtectValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_BATT_CHG_PROTECT_VOLT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetPvUdpValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_PV_UDP_VOLT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}
//bool PcsSetPvOvpValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_PV_OVP_VOLT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetBattDsgPowerSchedulingValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_BATT_DSG_POWER_SCHEDULING_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetBattChgPowerSchedulingValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_BATT_CHG_POWER_SCHEDULING_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetPvInputMode(uint16_t mode)
//{
//    if(mode > 2)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_PV_INPUT_MODE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = mode;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetDrmStatus(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_DRM_ENABLE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetAntiReverseFlowStatus(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_ANTI_REVERSE_FLOW_ENABLE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetProgramCtrlStatus(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_PORGRAM_CTRL_ENABLE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetOverVoltDeratingStatus(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_OVER_VOLT_DERATING_ENABLE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}
//bool PcsSetOverFreqDeratingStatus(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_OVER_FREQ_DERATING_ENABLE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetUnderVoltDeratingStatus(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_UNDER_VOLT_DERATING_ENABLE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetUnderFreqDeratingStatus(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_UNDER_FREQ_DERATING_ENABLE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetIslandStatus(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_ISLAND_ENABLE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetAcCouplingStatus(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_ACC_COUPLING_ENABLE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetHighLowCrossoverStatus(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_HIGH_LOW_CROSSOVER_ENABLE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSelectCtOrMeter(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SELECT_CT_METER_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetMeterBrand(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SELECT_METER_BRAND_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetCtDirection(uint16_t status)
//{
//    if(status > 1)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_CT_DIRECTION_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetAntiReverseFlowThreshold(uint16_t val)
//{
//    if(val > 5000)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_ANTI_REVERE_FLOW_THRESHOLD_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetRomoteShutdownStatus(uint16_t status)
//{
//    if(status > 2)
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_REMOTE_SHUTDOWN_STATUS_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = status;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}



//bool PcsSetReactivePowerValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_REACTIVE_POWER_VALUE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetReactivePowerCtrlMethod(uint16_t method)
//{
//    if(method > 3)
//    {
//        return false;
//     }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_REACTIVE_POWER_CTRL_METHOD_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = method;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetPowerFactorValue(uint16_t val)
//{
//    if(val > 100)
//    {
//        return false;
//     }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_POWER_FACTOR_SET_VALUE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}



//bool PcsSetActivePowerPersentValue(uint16_t val)
//{
//    if(val > 100)
//    {
//        return false;
//     }
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_ACTIVE_POWER_SET_VALUE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetGridVoltLowLimitValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_GRID_VOLT_LOW_LIMIT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetGridVoltUpperLimitValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_GRID_VOLT_UPPER_LIMIT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetGridFreqLowLimitValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_GRID_FREQ_LOW_LIMIT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetGridFreqUpperLimitValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_GRID_FREQ_UPPER_LIMIT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}



//bool PcsSetBypassVoltLowLimitValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_BYPASS_VOLT_LOW_LIMIT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetBypassVoltUpperLimitValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_BYPASS_VOLT_UPPRR_LIMIT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}




//bool PcsSetBypassFreqLowLimitValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_BYPASS_FREQ_LOW_LIMIT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetBypassFreqUpperLimitValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_BYPASS_FREQ_UPPRR_LIMIT_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetOverVoltDeratingValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_OVER_VOLT_DERATING_VALUE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetOverFreqDeratingValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_OVER_FREQ_DERATING_VALUE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}



//bool PcsSetUnderVoltDeratingValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_UNDER_VOLT_DERATING_VALUE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetUnderFreqDeratingValue(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_UNDER_FREQ_DERATING_VALUE_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetRphaseVoltCalibrationCoefficient_k(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_R_PHASE_VOLT_CALIBRATION_COEFFICIENT_K_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}
//bool PcsSetRphaseVoltCalibrationCoefficient_b(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_R_PHASE_VOLT_CALIBRATION_COEFFICIENT_B_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}



//bool PcsSetSphaseVoltCalibrationCoefficient_k(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_S_PHASE_VOLT_CALIBRATION_COEFFICIENT_K_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}
//bool PcsSetSphaseVoltCalibrationCoefficient_b(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_S_PHASE_VOLT_CALIBRATION_COEFFICIENT_B_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}




//bool PcsSetTphaseVoltCalibrationCoefficient_k(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_T_PHASE_VOLT_CALIBRATION_COEFFICIENT_K_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}
//bool PcsSetTphaseVoltCalibrationCoefficient_b(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_T_PHASE_VOLT_CALIBRATION_COEFFICIENT_B_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}




//bool PcsSetRphaseCurrCalibrationCoefficient_k(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_R_PHASE_CURR_CALIBRATION_COEFFICIENT_K_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}


//bool PcsSetRphaseCurrCalibrationCoefficient_B(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_R_PHASE_CURR_CALIBRATION_COEFFICIENT_B_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}








//bool PcsSetSphaseCurrCalibrationCoefficient_k(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_S_PHASE_CURR_CALIBRATION_COEFFICIENT_K_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetSphaseCurrCalibrationCoefficient_b(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_S_PHASE_CURR_CALIBRATION_COEFFICIENT_B_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}



//bool PcsSetTphaseCurrCalibrationCoefficient_k(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_T_PHASE_CURR_CALIBRATION_COEFFICIENT_K_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetTphaseCurrCalibrationCoefficient_b(uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_T_PHASE_CURR_CALIBRATION_COEFFICIENT_B_ADDR;
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}




//bool PcsSetPvCurrCalibration(uint8_t param_type,uint8_t channel,uint16_t val)
//{

//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    if(param_type == PCS_CALIBRATION_PARAM_K)
//    {
//        gPcsArcModbusSlaveWrite.reg_addr = (PCS_REG_PV_CURR_CALIBRATION_BASE_ADDR + (channel>>1));     
//    }
//    else if(param_type == PCS_CALIBRATION_PARAM_B)
//    {
//        gPcsArcModbusSlaveWrite.reg_addr = (PCS_REG_PV_CURR_CALIBRATION_BASE_ADDR + (channel>>1) + 1);
//    }
//    else
//    {
//        return false;
//    }

//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}
//bool PcsSetPvVoltCalibration(uint8_t param_type,uint8_t channel,uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
//    if(param_type == PCS_CALIBRATION_PARAM_K)
//    {
//        gPcsArcModbusSlaveWrite.reg_addr = (PCS_REG_PV_VOLT_CALIBRATION_BASE_ADDR + (channel>>1));     
//    }
//    else if(param_type == PCS_CALIBRATION_PARAM_B)
//    {
//        gPcsArcModbusSlaveWrite.reg_addr = (PCS_REG_PV_VOLT_CALIBRATION_BASE_ADDR + (channel>>1) + 1);
//    }
//    else
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}

//bool PcsSetBattVoltCalibration(uint8_t param_type,uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;

//    if(param_type == PCS_CALIBRATION_PARAM_K)
//    {
//        gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_BATT_VOLT_CALIBRATE_K_ADDR;     
//    }
//    else if(param_type == PCS_CALIBRATION_PARAM_B)
//    {
//        gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_BATT_VOLT_CALIBRATE_B_ADDR;
//    }
//    else
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}
//bool PcsSetBattCurrCalibration(uint8_t param_type,uint16_t val)
//{
//    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;


//    if(param_type == PCS_CALIBRATION_PARAM_K)
//    {
//        gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_BATT_CURR_CALIBRATE_K_ADDR;     
//    }
//    else if(param_type == PCS_CALIBRATION_PARAM_B)
//    {
//        gPcsArcModbusSlaveWrite.reg_addr = PCS_REG_BATT_CURR_CALIBRATE_BADDR;
//    }
//    else
//    {
//        return false;        
//    }
//    gPcsArcModbusSlaveWrite.reg_num = 1;
//    gPcsArcModbusSlaveWrite.slave_addr =SLAVE_PCS_ADDR;
//    gPcsArcModbusSlaveWrite.data[0] = val;
//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;     
//}
bool CollectWritePcsFun(uint16_t reg_addr,uint16_t reg_num,uint8_t *pdata,uint16_t slave_addr)
{

    gPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gPcsArcModbusSlaveWrite.reg_addr = reg_addr;
    gPcsArcModbusSlaveWrite.reg_num = reg_num;
    gPcsArcModbusSlaveWrite.slave_addr =slave_addr;
    memcpy(&gPcsArcModbusSlaveWrite.data[0],pdata,(reg_num<<1));
//直接按照字节拷贝，会转为了小端
//需要再转为大端
    for(uint8_t i = 0;i< reg_num;i++)
    {
        gPcsArcModbusSlaveWrite.data[i] = DataToUnshort(pdata + i*2, USING_MODE_BIG);
    }
    if(QueuePut(&gModbusPcsArcCommSendQueue, &gPcsArcModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;  

} 

void GetDataFromArc(uint8_t *pdata,uint16_t data_num)
{
    uint16_t    size = 0;
    size = (ARC_REG_IN_MAX_NUM + ARC_REG_HOLD_MAX_NUM);
    if((pdata == NULL)||(data_num > (size<<1)))
    {
        return;
    }
    memcpy(pdata,(uint8_t *)&gPcsArcSlavePara.arc_para.arc_input_para,data_num);
}  

void GetPcsInputFrontData(uint8_t *pdata,uint16_t data_num)
{
    if((pdata == NULL)||(data_num > (PCS_REG_IN_FRONT_MAX_NUM*2)))
    {
        return;
    }
    memcpy(pdata,(uint8_t *)&gPcsArcSlavePara.pcs_para.pcs_input_front_para,data_num);
}
void GetPcsInputBackData(uint8_t *pdata,uint16_t data_num)
{
    if((pdata == NULL)||(data_num > (PCS_REG_IN_BACK_MAX_NUM*2)))
    {
        return;
    }
    memcpy(pdata,(uint8_t *)&gPcsArcSlavePara.pcs_para.pcs_input_back_para,data_num);
}


void GetPcsHoldData(uint8_t *pdata,uint16_t data_num)
{
    if((pdata == NULL)||(data_num > (PCS_REG_HOLD_MAX_REG_NUM*2)))
    {
        return;
    }
    memcpy(pdata,(uint8_t *)&gPcsArcSlavePara.pcs_para.pcs_hold_para,data_num);        
}



