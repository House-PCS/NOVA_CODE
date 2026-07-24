
#include "slave_bms.h"
#include "dev_modbus.h"
#include "public_proto.h"
#define SLAVE_COMM_BMS_REV_MAX_TIME                 500
#define SLAVE_COMM_BMS_INTERVAL_TIME                150
#define SLAVE_COMM_BMS_POLLING_TIME                 150//300



static EN_BMS_POLLING_STATUS    _BmsSlavePollingStatus;
static EN_BMS_READ_EVENT        _BmsModbusCommReadEvent;
Queue_t	                        gBmsModbusSlaveCommSendQueue;
SlaveWrite_t                    gBmsModbusSlaveWrite;
SlaveRead_t                     gBmsModbusSlaveRead;

uint8_t                         gBmsSendData[300];
SlaveWrite_t                    gBmsSlaveWrite;
SlaveRead_t                     gBmsReadData_t;

BmsModBusSlavePara_t            gBmsSlavePara;
typedef struct  _deal_fram_stru_t
{
    uint8_t                                 slave_addr;
    EN_BMS_READ_EVENT                       envent;
    uint8_t                                 func_code;
    uint16_t                                reg_addr;
    uint8_t                                 *pdata; 
    uint16_t                                reg_num;
}BmsReadDealFram_t;
const BmsReadDealFram_t  gBmsReadDealFramArr[]=
{
    {SLAVE_BMS_ADDR, EN_BMS_READ_EVENT_FRONT,      READ_INPUT_TYPE,BMS_REG_ADDR_MODE_ID,        (uint8_t *)&(gBmsSlavePara.bms_para[0].bms_front_para), sizeof(BMSFrontPara_t)/sizeof(uint16_t)  },
    {SLAVE_BMS_ADDR, EN_BMS_READ_EVENT_BACK,       READ_INPUT_TYPE,BMS_REG_ADDR_PACK_CYCLE,     (uint8_t *)&(gBmsSlavePara.bms_para[0].bms_back_para),  sizeof(BMSBackPara_t)/sizeof(uint16_t)   },
    {SLAVE_BMS_ADDR, EN_BMS_HOLD_EVENT,            READ_HOLD_TYPE, BMS_REG_ADDR_HOLD_REG_ADDR,  (uint8_t *)&(gBmsSlavePara.bms_para[0].bms_hold_para),  sizeof(BMSHoldPara_t)/sizeof(uint16_t)   },
    {SLAVE_BMS1_ADDR,EN_BMS_READ_EVENT_ADD1_FRONT, READ_INPUT_TYPE,BMS_REG_ADDR_MODE_ID,        (uint8_t *)&(gBmsSlavePara.bms_para[1].bms_back_para),  sizeof(BMSFrontPara_t)/sizeof(uint16_t)  },
    {SLAVE_BMS1_ADDR,EN_BMS_READ_EVENT_ADD1_BACK,  READ_INPUT_TYPE,BMS_REG_ADDR_PACK_CYCLE,     (uint8_t *)&(gBmsSlavePara.bms_para[1].bms_front_para), sizeof(BMSBackPara_t)/sizeof(uint16_t)   },
    {SLAVE_BMS1_ADDR,EN_BMS_HOLD_EVENT_ADD1,       READ_HOLD_TYPE, BMS_REG_ADDR_HOLD_REG_ADDR,  (uint8_t *)&(gBmsSlavePara.bms_para[1].bms_hold_para),  sizeof(BMSHoldPara_t)/sizeof(uint16_t)   },
    {SLAVE_BMS2_ADDR,EN_BMS_READ_EVENT_ADD2_FRONT, READ_INPUT_TYPE,BMS_REG_ADDR_MODE_ID,        (uint8_t *)&(gBmsSlavePara.bms_para[2].bms_front_para), sizeof(BMSFrontPara_t)/sizeof(uint16_t)  },
    {SLAVE_BMS2_ADDR,EN_BMS_READ_EVENT_ADD2_BACK,  READ_INPUT_TYPE,BMS_REG_ADDR_PACK_CYCLE,     (uint8_t *)&(gBmsSlavePara.bms_para[2].bms_back_para),  sizeof(BMSBackPara_t)/sizeof(uint16_t)   },
    {SLAVE_BMS2_ADDR,EN_BMS_HOLD_EVENT_ADD2,       READ_HOLD_TYPE, BMS_REG_ADDR_HOLD_REG_ADDR,  (uint8_t *)&(gBmsSlavePara.bms_para[2].bms_hold_para),  sizeof(BMSHoldPara_t)/sizeof(uint16_t)   },
    {SLAVE_BMS3_ADDR,EN_BMS_READ_EVENT_ADD3_FRONT, READ_INPUT_TYPE,BMS_REG_ADDR_MODE_ID,        (uint8_t *)&(gBmsSlavePara.bms_para[3].bms_front_para), sizeof(BMSFrontPara_t)/sizeof(uint16_t)  },
    {SLAVE_BMS3_ADDR,EN_BMS_READ_EVENT_ADD3_BACK,  READ_INPUT_TYPE,BMS_REG_ADDR_PACK_CYCLE,     (uint8_t *)&(gBmsSlavePara.bms_para[3].bms_back_para),  sizeof(BMSBackPara_t)/sizeof(uint16_t)   },
    {SLAVE_BMS3_ADDR,EN_BMS_HOLD_EVENT_ADD3,       READ_HOLD_TYPE, BMS_REG_ADDR_HOLD_REG_ADDR,  (uint8_t *)&(gBmsSlavePara.bms_para[3].bms_hold_para),  sizeof(BMSHoldPara_t)/sizeof(uint16_t)   },
};
#define     MAX_SIZE    (sizeof(gBmsReadDealFramArr)/sizeof(gBmsReadDealFramArr[0]))

void SlaveBmsCommInit()
{
    QueueInit(&gBmsModbusSlaveCommSendQueue, SLAVE_BMS_WRITE_QUEUE_DEPTH, sizeof(gBmsModbusSlaveWrite));
}


static void _BmsSlaveAllReadPolling(SlaveRead_t *p_slave_frame)
{
    static uint8_t cnt = 0;
    static EN_BMS_READ_EVENT event_count = EN_BMS_READ_EVENT_FRONT;//    uint8_t max_event_count =0;

    p_slave_frame->slave_addr = gBmsReadDealFramArr[event_count].slave_addr;
    p_slave_frame->func_code  = gBmsReadDealFramArr[event_count].func_code;
    p_slave_frame->reg_addr   = gBmsReadDealFramArr[event_count].reg_addr;
    p_slave_frame->reg_num = gBmsReadDealFramArr[event_count].reg_num;
    ModbusHostRead(p_slave_frame,ModbusBMSRTUSend);
    event_count++;
    event_count %= EN_BMS_READ_EVENT_NUM; 
}


void    _BmsSlaveReadErrorDeal(uint8_t  slave_addr,uint16_t comm_status)
{
    uint8_t i=0;
    i = (slave_addr & 0x000f);
    if(comm_status == MBRTU_OK)
    {//清空对应的标志位
        gBmsSlavePara.bms_comm_err_cnt[i] = 0;
        gBmsSlavePara.bms_com_time_out[i] = 0;
    }
    else
    {//
        gBmsSlavePara.bms_comm_err_cnt[i]++;
        if(gBmsSlavePara.bms_comm_err_cnt[i] >= 5)
        {
            gBmsSlavePara.bms_comm_err_cnt[i] = 5;
            gBmsSlavePara.bms_com_time_out[i] = 1;            
        }
    }      
}


void TsetBmsInputData(void)
{
    uint16_t i=0;
    uint8_t j =0;
    uint8_t *p=NULL;

    for(j = 0;j<3;j++)
    {
         p = (uint8_t *)&gBmsSlavePara.bms_para[j].bms_front_para;
        for(i=0;i<sizeof(BMSFrontPara_t);i++)
        {
            *p=i;
            p++;
        }

        p = (uint8_t *)&gBmsSlavePara.bms_para[j].bms_back_para;
        for(i=0;i<sizeof(BMSBackPara_t);i++)
        {
            *p=i;
            p++;
        } 

        p = (uint8_t *)&gBmsSlavePara.bms_para[j].bms_hold_para;
        for(i=0;i<sizeof(BMSHoldPara_t);i++)
        {
            *p=i;
            p++;
        } 
    }

}

void    SlaveBmsModbusCommProcess()
{
    uint8_t i = 0;
    static ModbusRTURevData_t *p;
    static SlaveWrite_t *pBmsSlaveWrite;
    static uint32_t bms_slave_read_polling_cnt = 0;

    TsetBmsInputData();
    switch(_BmsSlavePollingStatus)
    {
        case EN_SLAVE_POLLING_STATUS_IDLE:
        {
            /*空闲时段也要执行Get gModbusRevQueue, 并舍弃，防止队列满导致无法再接收数据*/
            QueueGet(&gDevModbusBMS.rev_queue, (void *)&p);
            if(QueueGet(&gBmsModbusSlaveCommSendQueue, (void *)&pBmsSlaveWrite) == true)/*如果有主动写操作*/
            {
                gBmsReadData_t.func_code = pBmsSlaveWrite->func_code;  
                ModbusHostWrite(pBmsSlaveWrite,ModbusBMSRTUSend,gBmsSendData);
                _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_WAITING_REV;
                bms_slave_read_polling_cnt = tim_get_systime();  
            }
            else
            {
                /*轮询回读*/

                //定时发送给BMS心跳包
                if((uint32_t)(tim_get_systime() - bms_slave_read_polling_cnt) > SLAVE_COMM_BMS_POLLING_TIME)
                {
                    bms_slave_read_polling_cnt = tim_get_systime();
                    _BmsSlaveAllReadPolling(&gBmsReadData_t);
                    _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_WAITING_REV;
                }
            }
            break;
        }
        case EN_SLAVE_POLLING_STATUS_WAITING_REV:
        {

            if(QueueGet(&gDevModbusBMS.rev_queue, (void *)&p) == true)
            {
                bms_slave_read_polling_cnt = tim_get_systime(); 

                for(i = 0; i<=MAX_SIZE ;i++)
                {
                    if((gBmsReadDealFramArr[i].func_code == gBmsReadData_t.func_code) && (gBmsReadDealFramArr[i].reg_addr == gBmsReadData_t.reg_addr))
                    {
                        break;  //查找出所读取的目的地址
                    }
                }

                if(i == MAX_SIZE)
                {
                    _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_REV_INTERVAL_WAIT;
                    break;
                }
                if(Modebus_ParseUp((uint8_t *)p->p_data, p->data_size, &gBmsReadData_t, gBmsReadDealFramArr[i].pdata) == MBRTU_OK)
                {    
                    _BmsSlaveReadErrorDeal(gBmsReadData_t.slave_addr,MBRTU_OK);
                }
                else
                {//数据异常

                    _BmsSlaveReadErrorDeal(gBmsReadData_t.slave_addr,MBRTU_PARA_INVALID);
                }

                _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_REV_INTERVAL_WAIT;
            }
            if((uint32_t)(tim_get_systime() - bms_slave_read_polling_cnt) > SLAVE_COMM_BMS_REV_MAX_TIME)
            {
                bms_slave_read_polling_cnt = tim_get_systime();
                _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_REV_OVERTIME;
            }
            break;
        }
        case EN_SLAVE_POLLING_STATUS_REV_OVERTIME:
        {
            /*做传输超时的处理*/
            _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;
            _BmsSlaveReadErrorDeal(gBmsReadData_t.slave_addr,MBRTU_PARA_INVALID);
            break;
        }
        case EN_SLAVE_POLLING_STATUS_REV_INTERVAL_WAIT:
        {
            if((uint32_t)(tim_get_systime() - bms_slave_read_polling_cnt) > SLAVE_COMM_BMS_INTERVAL_TIME)
            {
                _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;
                bms_slave_read_polling_cnt = tim_get_systime();    

            }
            break;
        }
        default:
        _BmsSlavePollingStatus = EN_SLAVE_POLLING_STATUS_IDLE;
        break;
    }        
}





/*BMS放电MOS控制 mode:0-关闭，1-打开*/
bool BMSDsgMosCtl(uint8_t mode)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_DSG_MOS_CTL;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = mode;
    
    if(mode > 1)
    {
        return false;
    }
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
/*BMS写操作*/
/*BMS充电MOS控制 mode:0-关闭，1-打开*/
bool BMSChgMosCtl(uint8_t mode)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CHG_MOS_CTL;
    gBmsModbusSlaveWrite.reg_num  =1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = mode;

    if(mode > 1)
    {
        return false;
    }
    
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/*BMS预放电MOS控制 mode:0-关闭，1-打开*/
bool BMSPreDsgMosCtl(uint8_t mode)
{

    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_PRE_MOS_CTL;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = mode;
    if(mode > 1)
    {
        return false;
    }
    
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/*BMS限流模块 mode:0-关闭，1-打开*/
bool BMSLimitCurrCtrl(uint8_t mode)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_LIMIT_CURR_CTRL;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = mode;
     
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


/*BMS状态 status:0-开机，1-待机 2-关机*/
bool BMSSetShutDown(uint8_t status)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_SHUTDOWN;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = status;
     
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/*BMS 设置RTC time*/
bool BMS_RtcTimeSet(uint32_t rtc_time)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_RTC_TIME;
    gBmsModbusSlaveWrite.reg_num = 2;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = (uint16_t)(rtc_time & 0x0000ffff);    //先发送低16bit
    gBmsModbusSlaveWrite.data[1] = (uint16_t)(rtc_time>>16);

    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/*BMS heat ctrl  mode 0-关闭 1 打开*/
bool BMSHeatCtrl(uint8_t mode)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_HEAT_CTRL;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = mode;
     
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


/*BMS 欠压一级*/
bool BMSSetCellUvLevel1(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CELL_UV_LV1;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/*BMS 欠压二级*/
bool BMSSetCellUvLevel2(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CELL_UV_LV2;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}



/*BMS 欠压三级*/
bool BMSSetCellUvLevel3(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CELL_UV_LV3;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/*BMS 过压1级*/
bool BMSSetCellOvLevel1(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CELL_OV_LV1;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
/*BMS 过压2级*/
bool BMSSetCellOvLevel2(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CELL_OV_LV2;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/*BMS 过压3级*/
bool BMSSetCellOvLevel3(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CELL_OV_LV3;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
/*PACK 欠压1级*/
bool BMSSetPackUvLevel1(uint32_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_PACK_UV_LV1;
    gBmsModbusSlaveWrite.reg_num = 2;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = (uint16_t)(val & 0x0000ffff);    //先发送低16bit
    gBmsModbusSlaveWrite.data[1] = (uint16_t)(val>>16);
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
/*PACK 过压1级*/
bool BMSSetPackOvLevel1(uint32_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_PACK_OV_LV1;
    gBmsModbusSlaveWrite.reg_num = 2;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = (uint16_t)(val & 0x0000ffff);    //先发送低16bit
    gBmsModbusSlaveWrite.data[1] = (uint16_t)(val>>16);
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
/*BMS 过温度1级*/
bool BMSSetCellOtLevel1(int16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CELL_OT_LV1;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
/*BMS 过温度2级*/
bool BMSSetCellOtLevel2(int16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CELL_OT_LV2;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
/*BMS 过温度3级*/
bool BMSSetCellOtLevel3(int16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CELL_OT_LV3;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
/*BMS 低温度3级*/
bool BMSSetCellUtLevel1(int16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CELL_UT_LV1;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
/*BMS MOS 过温1级*/
bool BMSMosOtLevel1(int16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_MOS_OT_LV1;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


/*BMS MOS 过温2级*/
bool BMSMosOtLevel2(int16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_MOS_OT_LV2;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/*BMS MOS 过温3级*/
bool BMSMosOtLevel3(int16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_MOS_OT_LV3;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}



/*BMS 放电过流1级*/
bool BMSDsgOcLevel1(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_DSG_OC_LV1;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


/*BMS 放电过流2级*/
bool BMSDsgOcLevel2(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_DSG_OC_LV2;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


/*BMS 放电过流3级*/
bool BMSDsgOcLevel3(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_DSG_OC_LV3;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


/*BMS 充电过流1级*/
bool BMSChgOcLevel1(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CHG_OC_LV1;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/*BMS 充电过流2级*/
bool BMSChgOcLevel2(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CHG_OC_LV2;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/*BMS 充电过流3级*/
bool BMSChgOcLevel3(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CHG_OC_LV3;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


/*BMS 温度diff*/
bool BMSCellTempDiff(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CELL_TEMP_DIFF;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


/*BMS 电压diff*/
bool BMSCellBatVoltDiff(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CELL_VOLT_DIFF;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


/*BMS soh low level1*/
bool BMSSohLowLevel1(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_SOH_LOW_LV1;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


/*BMS soh low level2*/
bool BMSSohLowLevel2(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_SOH_LOW_LV2;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

///*BMS */
//bool BMS_ParaSet(uint16_t addr,uint16_t val)
//{
////	if(val > 1)
////	{
////			return false;
////	}
//    gBmsModbusSlaveWrite.data_type =0x00;
//    gBmsModbusSlaveWrite.reg_addr = addr;
//    gBmsModbusSlaveWrite.reg_num = 1;
//    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
//    gBmsModbusSlaveWrite.word_data[0] = val;

//    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;
//}



/*BMS  0 正常  1 清除*/
bool BMS_CleanHistoryErr(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_CLEAN_HISTORY_ERR;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/*BMS 0 正常  1 强制OCV*/
bool BMSForceOcv(uint16_t val)
{
    gBmsModbusSlaveWrite.func_code =MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = BMS_REG_ADDR_FORCE_OCV;
    gBmsModbusSlaveWrite.reg_num = 1;
    gBmsModbusSlaveWrite.slave_addr = SLAVE_BMS_ADDR;
    gBmsModbusSlaveWrite.data[0] = val;
    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


bool CollectWriteBmsFun(uint16_t reg_addr,uint16_t reg_num,uint8_t *pdata,uint16_t slave_addr)
{

    gBmsModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gBmsModbusSlaveWrite.reg_addr = reg_addr;
    gBmsModbusSlaveWrite.reg_num = reg_num;
    gBmsModbusSlaveWrite.slave_addr =slave_addr;
    memcpy(&gBmsModbusSlaveWrite.data[0],pdata,(reg_num<<1));

    for(uint8_t i = 0;i< reg_num;i++)
    {
        gBmsModbusSlaveWrite.data[i] = DataToUnshort(pdata + i*2, USING_MODE_BIG);
    }

    if(QueuePut(&gBmsModbusSlaveCommSendQueue, &gBmsModbusSlaveWrite) == false)
    {
        return false;
    }
    return true; 
}



void    GetDataFromBms(uint8_t *pdata,uint16_t data_num,uint8_t batt_num)
{

    uint16_t   size=0;
    size = (BMS_INPUT_FRONT_REG_MAX_NUM + BMS_INPUT_BACK_REG_MAX_NUM + BMS_HOLD_REG_MAX_NUM);
    if((pdata == NULL)||(data_num > (size<<1)))
    {
        return;
    }   
    for(uint8_t i=0;i<batt_num;i++)
    {
        memcpy(pdata,(uint8_t *)&gBmsSlavePara.bms_para[i].bms_front_para,data_num);
        pdata += data_num;  
    }
}


void    GetBmsInputFrontData(uint8_t *pdata,uint16_t data_num,uint8_t batt_num)
{
    if((pdata == NULL)||(data_num > (BMS_INPUT_FRONT_REG_MAX_NUM*2)))
    {
        return;
    }
    memcpy(pdata,(uint8_t *)&gBmsSlavePara.bms_para[batt_num].bms_front_para,data_num);
}

void    GetBmsInputBackData(uint8_t *pdata,uint16_t data_num,uint8_t batt_num)
{
    if((pdata == NULL)||(data_num > (BMS_INPUT_BACK_REG_MAX_NUM*2)))
    {
        return;
    }
    memcpy(pdata,(uint8_t *)&gBmsSlavePara.bms_para[batt_num].bms_back_para,data_num);
}
void    GetBmsHoldData(uint8_t *pdata,uint16_t data_num,uint8_t batt_num)
{
    if((pdata == NULL)||(data_num > (BMS_HOLD_REG_MAX_NUM*2)))
    {
        return;
    }
    memcpy(pdata,(uint8_t *)&gBmsSlavePara.bms_para[batt_num].bms_hold_para,data_num);        
}






