#include "slave_pcs.h"
#include "dev_modbus.h"
#include "public_proto.h"
#include "dev_udp_comm.h"
#include "queue.h"



Queue_t                 gModbusPcsArcCommSendQueue;
SlaveRead_t             gstModbusPcsArcRead;
SlaveWrite_t            gstPcsArcModbusSlaveWrite;
//uint8_t                 garrPcsArcSendData[300];
uint8_t                 PcsUdpSendBuf[1036];
//static __SDRAM uint8_t PcsUdpSendBuf[1036];


UdpPcsCtrl_t            UdpPcsCtrlInfo;
//ModBusPcsArcPara_t      gstPcsArcSlavePara;

bool SetArcInsulationDetect(uint8_t status)
{
    gstPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gstPcsArcModbusSlaveWrite.reg_addr = ARC_REG_HOLD_ARC_DETECT_INSULATION_EN_ADDR;
    gstPcsArcModbusSlaveWrite.reg_num = 1;
    gstPcsArcModbusSlaveWrite.slave_addr = SLAVE_ARC_ADDR;
    gstPcsArcModbusSlaveWrite.data[0] = status;
    if(QueuePut(&gModbusPcsArcCommSendQueue, &gstPcsArcModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

bool SetArcSysUpgradeStatus(uint8_t status)
{
    gstPcsArcModbusSlaveWrite.func_code = MBRTU_FC_WRITE_STANDREG;
    gstPcsArcModbusSlaveWrite.reg_addr = ARC_REG_HOLD_ARC_SYS_UPGRADE_STATUS_ADDR;
    gstPcsArcModbusSlaveWrite.reg_num = 1;
    gstPcsArcModbusSlaveWrite.slave_addr = SLAVE_ARC_ADDR;
    gstPcsArcModbusSlaveWrite.data[0] = status;
    if(QueuePut(&gModbusPcsArcCommSendQueue, &gstPcsArcModbusSlaveWrite) == false)
    {
        return false;
    }
    return true;
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




typedef struct
{
    uint32_t base_addr;
    uint16_t num;
    void *pdata;
} UdpPcsDataPoint_t;



/**
  * @brief   PCS寄存器
  */
static const UdpPcsDataPoint_t UdpPcsDataPoint[] =
{
    {PCS_REG_FRONT_BASE_ADDR,  PCS_REG_IN_FRONT_MAX_NUM,      &gstUdpPcsSlavePara.pcs_info.pcs_base_info }, //0          ?????地址
    {PCS_REG_HOLD_BASE_ADDR,   PCS_REG_HOLD_MAX_REG_NUM,      &gstUdpPcsSlavePara.pcs_info.set_info }, //1
    {ARC_REG_BASE_ADDR,        ARC_REG_MAX_REG_NUM,           &gstUdpPcsSlavePara.pcs_info.arc_base_info }, //2

};
#define UDP_PCS_POINT_ARR_NUM     (sizeof(UdpPcsDataPoint)/sizeof(UdpPcsDataPoint_t))

/**
  * @brief   PCS读解析，将读回的数据写入寄存器中
  * @param   None
  * @return  None
  * @note    None
  */
void UdpPcsReadParse(uint8_t *p_recv_data, uint16_t len, uint8_t num_id, uint32_t data_start_addr)
{
    uint32_t start_addr;
    uint16_t reg_num;
    uint8_t index;
    uint16_t offset;
    uint16_t i;
    uint32_t reg_addr;
    uint16_t w_buffer;

    uint16_t udp_pcs_point_num = 0;
    const UdpPcsDataPoint_t *upd_data_point;
    upd_data_point = UdpPcsDataPoint;
    udp_pcs_point_num = UDP_PCS_POINT_ARR_NUM;
    start_addr = data_start_addr;
    reg_num = len;

    for(i = 0; i < reg_num; i++)
    {
        reg_addr = start_addr + i;
        for(index = 0; index < udp_pcs_point_num; index++)
        {
            if(NULL != upd_data_point[index].pdata)
            {
                if((reg_addr >= upd_data_point[index].base_addr) && (reg_addr < (upd_data_point[index].base_addr + upd_data_point[index].num)))
                {
//                    offset = reg_addr - upd_data_point[index].base_addr;
//                    w_buffer = p_recv_data[i];
//                    *((uint8_t *)upd_data_point[index].pdata + offset) = (uint8_t)w_buffer;
                    offset = reg_addr - upd_data_point[index].base_addr;
                    w_buffer = DataToUnshort(p_recv_data + i * 2, USING_MODE_BIG);
                    *((uint16_t *)upd_data_point[index].pdata + offset) = w_buffer;
 

                    break;
                }
            }
        }
    }
}

//void GetPcsInputFrontData(uint8_t *pdata, uint16_t data_num)
//{
//    /* ??????重构 */
//    if((pdata == NULL) || (data_num > (PCS_REG_IN_FRONT_MAX_NUM * 2)))          //长度
//    {
//        return;
//    }
//    memcpy(pdata, (uint8_t *)&gstUdpPcsSlavePara.udp_pcs_input_para, data_num);
//}


//void GetPcsHoldData(uint8_t *pdata, uint16_t data_num)
//{
//    /* ??????重构 */
//    if((pdata == NULL) || (data_num > (PCS_REG_HOLD_MAX_REG_NUM * 2)))
//    {
//        return;
//    }
//    memcpy(pdata, (uint8_t *)&gstUdpPcsSlavePara.udp_pcs_hold_para, data_num);
//}


UdpPCSPara_t  gstUdpPcsSlavePara;
uint8_t DspUdpSendBuf[1024];
/**
  * @brief  数据接收回调函数
  * @param   None
  * @return  None
  * @note    None
  */
static void recv_fun_call_back(uint8_t *pdata, uint16_t len, uint8_t num_id)
{
    //获取到数据包
//需要解析
//2字节 device_type
//1字节 reserved
//1字节 opCmd
//4字节 Addr
//2字节 length
//      DATA
//讲读取到数据赋值给对应的寄存器

    uint8_t cnt = 0;
    uint16_t data = 0;
    uint32_t data_addr = 0;
    uint16_t data_num = 0;
    uint16_t i = 0;
    uint8_t *p_recv_data = NULL;
    uint8_t *p_pcs_data = NULL;
    uint8_t cmd = 0;
    if(len < 8)
    {
        return;
    }
    data = *(uint16_t *)&pdata[cnt];

    if(data != 0x0001)
    {
        return;
    }
    cnt += 2;
    data = pdata[cnt++];
    cmd = pdata[cnt++];
    data_addr = *(uint32_t *)&pdata[cnt];
    if(cmd != 0X43)
    {
        //读取操作。
        return;
    }
    cnt += 4;
    data_num = *((uint16_t *)&pdata[cnt]);
    cnt += 2;
    p_recv_data = &pdata[cnt];
    //收到数据
    //数据解析
    UdpPcsReadParse(p_recv_data, data_num, num_id, data_addr);
    //置起接收标志
    UdpPcsCtrlInfo.ctrl_flag.bit_value.recv_flag = 1;
    return;
}



typedef enum
{
    ID_STATUS = 0,
    RECV_STATUS,
    SEND_DELAY_STATUS,
} EN_DSP_READ_EVENT;


EN_DSP_READ_EVENT eDspReadEvent;



#define         SLAVE_DSP_UDP_WRITE_QUEUE_DEPTH         5
Queue_t         gDspUdpSlaveCommSendQueue;

SlaveUdpWrite_t gstDspUdpSlaveWrite;

/**
  * @brief  DSP参数初始化
  * @param   None
  * @return  None
  * @note    None
  */
void DspUdpParaInit(void)
{
    InitalDspUdpPara(recv_fun_call_back);
    QueueInit(&gDspUdpSlaveCommSendQueue, SLAVE_DSP_UDP_WRITE_QUEUE_DEPTH, sizeof(gstDspUdpSlaveWrite));
}

/**
  * @brief  关闭DSP的UDP连接
  * @param   None
  * @return  None
  * @note    None
  */
void ColseDspUdpConnect(void)
{
    udp_disconnect (gstDspUdpCommPara.slave_udp_pcb);
}

/**
  * @brief  打开DSP的UDP连接
  * @param   None
  * @return  None
  * @note    None
  */
void OpenDspUdpConnect(void)
{
    InitialSlaveUdpProc(&gstDspUdpCommPara);
}


/**
  * @brief  发送DSP的UDP数据包
  * @param   None
  * @return  None
  * @note    None
  */
void SendDspUdpData(void)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = 0x01;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = 0x45;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = 0x1000;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 4;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = 0;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = 1;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[2] = 2;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[3] = 3;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[4] = 4;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[5] = 5;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[6] = 6;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[7] ++;
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        ;
    }
}


bool CollectWritePcsFun(uint16_t reg_addr, uint16_t reg_num, uint8_t *pdata, uint8_t using_mode)
{
    uint16_t data_temp = 0;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = reg_addr;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = reg_num;
    for(uint8_t i = 0; i < reg_num; i++)
    {
        data_temp = DataToUnshort(pdata + i * 2, using_mode);
        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[i * 2] = data_temp>>8;
        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[i * 2 + 1] = data_temp;
    }
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;;
    }
    return true;
}

bool SetPcsChgCtrlStatus(uint8_t chg_ctrl)
{
    if(chg_ctrl > 4)
    {
        return false;
    }
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_SET_PCS_CHG_CTRL_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(chg_ctrl >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(chg_ctrl >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}




bool SetPcsSysUpgradeStatus(uint16_t status)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_SYS_UPGRADE_STATUS_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(status >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(status >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}






bool SetPcsDsgCtrlStatus(uint8_t dsg_ctrl)
{
    if(dsg_ctrl > 4)
    {
        return false;
    }
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_SET_PCS_DSG_CTRL_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(dsg_ctrl >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(dsg_ctrl >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

//chg power
bool SetPcsChgVoltCurPowerInfo(uint16_t batt_max_chg_curr, uint32_t batt_max_chg_power)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_BATT_CHG_CURR_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 3;

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(batt_max_chg_curr >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(batt_max_chg_curr >> 0);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[2] = (uint8_t)(batt_max_chg_power >> 24);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[3] = (uint8_t)(batt_max_chg_power >> 16);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[4] = (uint8_t)(batt_max_chg_power >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[5] = (uint8_t)(batt_max_chg_power >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;                        
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)                     
    {
        return false;
    }

    return true;
}
//DSG POWER
bool SetPcsDsgVoltCurPowerInfo(uint16_t batt_max_dsg_curr, uint32_t batt_max_dsg_power)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_BATT_DSG_CURR_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 3;

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(batt_max_dsg_curr >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(batt_max_dsg_curr >> 0);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[2] = (uint8_t)(batt_max_dsg_power >> 24);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[3] = (uint8_t)(batt_max_dsg_power >> 16);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[4] = (uint8_t)(batt_max_dsg_power >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[5] = (uint8_t)(batt_max_dsg_power >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

bool SetPcsChgDsgVoltCurPowerInfo(uint16_t max_chg_cur, uint32_t max_chg_power, uint16_t max_dsg_cur, uint32_t max_dsg_power)
{

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_BATT_CHG_CURR_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 6;

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(max_chg_cur >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(max_chg_cur >> 0);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[2] = (uint8_t)(max_chg_power >> 24);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[3] = (uint8_t)(max_chg_power >> 16);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[4] = (uint8_t)(max_chg_power >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[5] = (uint8_t)(max_chg_power >> 0);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[6] = (uint8_t)(max_dsg_cur >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[7] = (uint8_t)(max_dsg_cur >> 0);    
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[8] = (uint8_t)(max_dsg_power  >> 24);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[9] = (uint8_t)(max_dsg_power  >> 16);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[10] = (uint8_t)(max_dsg_power >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[11] = (uint8_t)(max_dsg_power >> 0);
 
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

bool SetPcsUserType(uint16_t user_type, uint8_t max_user_type_num)
{
    if(user_type >= max_user_type_num)
    {
        return false;
    }
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_SET_PCS_USER_TYPE_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(user_type >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(user_type >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


bool SetPcsMaxPvPower(uint32_t max_pv_power, uint32_t max_inv_power)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_MAX_PV_CHG_POWER_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 4;

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(max_pv_power >> 24);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(max_pv_power >> 16);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[2] = (uint8_t)(max_pv_power >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[3] = (uint8_t)(max_pv_power >> 0);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[4] = (uint8_t)(max_inv_power >> 24);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[5] = (uint8_t)(max_inv_power >> 16);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[6] = (uint8_t)(max_inv_power >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[7] = (uint8_t)(max_inv_power >> 0);

    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

bool SetChgInputType(uint8_t type)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_SET_CHG_INPUT_TYPE_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(type >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(type >> 0);


    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


bool SetPcsResetFactory(void)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_RESET_FACTORY_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = 0x5A;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = 0XA5;
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}
bool SetBattVoltToDsp(uint16_t batt_volt, uint8_t dsp_wake_status)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_BATT_VOLT_TO_DSP_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 2;

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(batt_volt >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(batt_volt >> 0);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[2] = (uint8_t)(dsp_wake_status >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[3] = (uint8_t)(dsp_wake_status >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
//    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
//    {
//        return false;
//    }
    return true;
}


/**
 * @brief  PCS关机控制发送
  * @param   None  
  * @return  None
  * @note    None
  */
bool PCSShutDownSend()
{
    uint8_t data_signal = 1;
    
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_SHUTDOWN_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(data_signal >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(data_signal);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

bool SetPcsElecMeterType(uint16_t meter_type)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_SELECT_METER_BRAND_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;  //2字节

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(meter_type >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(meter_type >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


bool SetPcsRemoteShutdown(uint16_t status)
{
    //PCS_REG_SET_REMOTE_SHUTDOWN_STATUS_ADDR
//    uint8_t buf[20];
    uint16_t   check_sum;
    if(status > 2)
    {
        return false;
    }
//    buf[0] = 0x5A;
//    buf[1] = 0x00;
//    buf[2] = 0x01;
//    buf[3] = 0x000E & 0xFF;
//    buf[4] = (0x000E >> 8) & 0xFF;
//    buf[5] = 0x01;  //LEN
//    buf[6] = 0x00;
//    buf[7] = status;
//    check_sum = _CheckSum(buf, 8);
//    buf[8] = check_sum & 0xFF;
//    buf[9] = (check_sum >> 8) & 0xFF;
////    ModbusPCSRTUSend(buf, 10);
//    gstPcsArcModbusSlaveWrite.func_code = 0x00;  //私有协议
//    gstPcsArcModbusSlaveWrite.reg_addr = PCS_REG_SET_REMOTE_SHUTDOWN_STATUS_ADDR;
//    memcpy((uint8_t *)gstPcsArcModbusSlaveWrite.data, (uint8_t *)buf, 10);

//    if(QueuePut(&gModbusPcsArcCommSendQueue, &gstPcsArcModbusSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_SET_REMOTE_SHUTDOWN_STATUS_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;  //2字节

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(status >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(status >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}


bool SetDspEnterCtTestMode(uint8_t status)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_SET_CT_ENTER_TEST_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;  //2字节

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(status >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(status >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

bool SetDspDrmStatus(uint16_t status)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_DRM_STATUS_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;  //2字节

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(status >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(status >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

bool SetDspLLCEnable()
{
    uint16_t status = 1;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_LLC_ENABLE_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;  //2字节

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(status >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(status >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

bool SetPcsCapInfoToZero(uint16_t status)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = PCS_REG_CLEAN_CURRENT_CAP_ADDR;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;  //2字节

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(status >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(status >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;

}
//pv switch
bool SetPcsActivePowerVoltCtrl(uint16_t pv_switch)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = 0x6291;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;  //2字节

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(pv_switch >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(pv_switch >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

//pf switch
bool SetPcsActivePowerFreqCtrl(uint16_t pf_switch)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = 0x6292;  //寄存器地址暂未确定
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;  //2字节

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(pf_switch >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(pf_switch >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

//backup_en
bool SetPcsBackEnCtrl(uint16_t backup_en)
{
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = 0x6256;  //backup使能信号
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = 1;  //2字节

    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(backup_en >> 8);
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(backup_en >> 0);
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    if(QueuePut(&gDspUdpSlaveCommSendQueue, &gstDspUdpSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

//data_addr 数据地址    data_len 数据字节长度
void SendByteDataToPcs(uint32_t data_addr, uint16_t data_len, uint8_t *pdata)
{
    uint16_t len = 0;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = BYTE_MODE; //字节操作
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr  = data_addr;
    gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = data_len; //字节数
    gstDspUdpSlaveWrite.slave_udp_pcb = gstDspUdpCommPara.slave_udp_pcb;
    memcpy(gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.data, pdata, data_len);
    len = (gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length + 10);
    memcpy(PcsUdpSendBuf, gstDspUdpSlaveWrite.slave_udp_write_data.data, len);

    Lwip_udp_sendto_data(gstDspUdpSlaveWrite.slave_udp_pcb, PcsUdpSendBuf, len, &gstDspUdpCommPara.slave_ip_info.remot_ipaddr, gstDspUdpCommPara.slave_ip_info.remot_port); //PCS通讯
}


/**
  * @brief  DSP通讯处理
  * @param   None
  * @return  None
  * @note    None
  */
void DspCommProc(void)
{
    SlaveUdpWrite_t *pDspUdpSlaveWrite;
    static uint32_t pcs_send_time_delay = 0;
    uint16_t len = 0;
    static uint8_t read_enent_dptr = 0;
    uint16_t data_len = 0;

    switch(eDspReadEvent)
    {
        /* 空闲状态，发送读/写Pack */
        case ID_STATUS:
        {
            //优先进行其他写操作，没有写操作时再进行读操作
            if(QueueGet(&gDspUdpSlaveCommSendQueue, (void *)&pDspUdpSlaveWrite) == true)/*如果有主动写操作*/
            {
                len = ((pDspUdpSlaveWrite->slave_udp_write_data.write_data_t.length << 1) + 10);
                if(len >=  1024)
                {
                    len = 1024;
                }
                //组包
                memcpy(DspUdpSendBuf, pDspUdpSlaveWrite->slave_udp_write_data.data, len);
                //发送
                Lwip_udp_sendto_data(gstDspUdpCommPara.slave_udp_pcb, DspUdpSendBuf, len, &gstDspUdpCommPara.slave_ip_info.remot_ipaddr, \
                                     gstDspUdpCommPara.slave_ip_info.remot_port);
                //发送延时
                eDspReadEvent = SEND_DELAY_STATUS;
            }
            else
            {
                //PCS数据读取
                switch(read_enent_dptr)
                {
                    case 0:
                        //读取输入寄存器
                        read_enent_dptr = 1;
                        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
                        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
                        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_READ_CMD;
                        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr = PCS_REG_FRONT_BASE_ADDR;
                        data_len = 2 * PCS_REG_IN_FRONT_MAX_NUM;

                        break;
                    case 1:
                        //读取保持寄存器
                        read_enent_dptr = 2;
                        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
                        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
                        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_READ_CMD;
                        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr = PCS_REG_HOLD_BASE_ADDR;
                        data_len = 2 * PCS_REG_HOLD_MAX_REG_NUM;

                        break;
                    case 2:
                        //读取拉弧寄存器
                        read_enent_dptr = 0;
                        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_PCS;
                        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
                        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_READ_CMD;
                        gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.addr = ARC_REG_BASE_ADDR;
                        data_len = 2 * ARC_REG_MAX_REG_NUM;

                        break;
                    default:
                        break;
                }
                gstDspUdpSlaveWrite.slave_udp_write_data.write_data_t.length = (data_len >> 1);
                len = 10;
                memcpy(DspUdpSendBuf, gstDspUdpSlaveWrite.slave_udp_write_data.data, len);
                //发送
                Lwip_udp_sendto_data(gstDspUdpCommPara.slave_udp_pcb, DspUdpSendBuf, len, &gstDspUdpCommPara.slave_ip_info.remot_ipaddr, \
                                     gstDspUdpCommPara.slave_ip_info.remot_port);
                UdpPcsCtrlInfo.ctrl_flag.bit_value.recv_flag = 0;
                //等待接收完成
                eDspReadEvent = RECV_STATUS;

            }
            pcs_send_time_delay = tim_get_systime();

            break;
        }
        /* 接收状态处理 */
        case RECV_STATUS:
        {
            //收到DSP数据，刷新状态
            if(UdpPcsCtrlInfo.ctrl_flag.bit_value.recv_flag == 1)
            {
                UdpPcsCtrlInfo.ctrl_flag.bit_value.recv_flag = 0;
                eDspReadEvent = ID_STATUS;
                gstUdpPcsSlavePara.pcs_comm_time_out = 0;
                UdpPcsCtrlInfo.pcs_comm_err_cnt = 0;
            }
            else
            {
                //3S内未收到，通讯超时
                if((uint32_t)(tim_get_systime() - pcs_send_time_delay) > 300)
                {
                    pcs_send_time_delay = tim_get_systime();
                    UdpPcsCtrlInfo.pcs_comm_err_cnt ++;
                    if(UdpPcsCtrlInfo.pcs_comm_err_cnt >= 10)
                    {
                        UdpPcsCtrlInfo.pcs_comm_err_cnt = 10;
                        gstUdpPcsSlavePara.pcs_comm_time_out = 1;
                    }
                    eDspReadEvent = ID_STATUS;
                }
            }
            break;
        }
        /* 发送延时 */
        case SEND_DELAY_STATUS:
        {
            if((uint32_t)(tim_get_systime() - pcs_send_time_delay) > 10)
            {
                pcs_send_time_delay = tim_get_systime();
                eDspReadEvent = ID_STATUS;
            }
            break;
        }
        default:
            break;
    }
}

