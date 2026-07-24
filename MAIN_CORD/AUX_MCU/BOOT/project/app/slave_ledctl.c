/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        slave_ledctl.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-10-22
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-10-22    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#include "slave_ledctl.h"



EN_MAINCARD_POLLING_STATUS   geMainCardSlavePollingStatus;
ModbusRtuFrame_t gstLedCtlModbusData;
LedCtrlPara_t    gstLedCtrlParaData;

static uint8_t _garrLedCommTxDataBuf[256]={1,2,3,4,5};
ReceiveDataPack_t _gstReceivePackBuff[RECV_BUFF_SIZE];
static LedCtlTxData_t _gstComLedCtlTxData;


static const ModbusDataPoint_t LedCtrlDataPoint[] =
{
    {LEDCTL_START_MODE_ADDR,            LEDCTL_START_MODE_NUM,              EN_DATA_U16,    EN_ATTR_RW,    &gstLedCtrlParaData.start_led_mode                 },//0   0xA000
//    {LEDCTL_SYS_MODE_ADDR,              LEDCTL_SYS_MODE_NUM,                EN_DATA_U16,    EN_ATTR_RW,    &gstLedCtrlParaData.sys_led_mode                   },//1   0xA001
//    {LEDCTL_COLLECTOR_MODE_ADDR,        LEDCTL_COLLECTOR_MODE_NUM,          EN_DATA_U16,    EN_ATTR_RW,    &gstLedCtrlParaData.collector_led_mode             },//2   0xA002
//    {LEDCTL_PCS_MODE_ADDR,              LEDCTL_PCS_MODE_NUM,                EN_DATA_U16,    EN_ATTR_RW,    &gstLedCtrlParaData.pcs_led_mode                   },//3   0xA003
//    {LEDCTL_CONFIRM_KEY_MODE_ADDR,      LEDCTL_CONFIRM_KEY_MODE_NUM,        EN_DATA_U16,    EN_ATTR_RW,    &gstLedCtrlParaData.confirm_led_mode               },//4   0xA004
//    {LEDCTL_ESC_KEY_MODE_ADDR,          LEDCTL_ESC_KEY_MODE_NUM,            EN_DATA_U16,    EN_ATTR_RW,    &gstLedCtrlParaData.esc_led_mode                   },//5   0xA005
//    {LEDCTL_UP_KEY_MODE_ADDR,           LEDCTL_UP_KEY_MODE_NUM,             EN_DATA_U16,    EN_ATTR_RW,    &gstLedCtrlParaData.up_led_mode                    },//6   0xA006
//    {LEDCTL_DOWN_KEY_MODE_ADDR,         LEDCTL_DOWN_KEY_MODE_NUM,           EN_DATA_U16,    EN_ATTR_RW,    &gstLedCtrlParaData.down_led_mode                  },//7   0xA007
//    {LEDCTL_RERSVER_ADDR,               LEDCTL_RERSVER_NUM,                 EN_DATA_U16,    EN_ATTR_RW,    &gstLedCtrlParaData.reserved[0]                    },//8   0xA008~0xA01C
};
#define LED_CTRL_DATA_NUM     (sizeof(LedCtrlDataPoint)/sizeof(ModbusDataPoint_t))

const ModbusDataPoint_t *ModbusUserData = LedCtrlDataPoint;
const uint16_t          ModbusUserDataPointNum = LED_CTRL_DATA_NUM;



uint16_t ResponseParaUp(uint8_t * p_rev_data, uint16_t rev_len, ReceiveDataPack_t * p_pack_para, uint16_t real_len)
{
    uint8_t reg_size = 0;
    uint16_t length = 0;
    uint16_t ret;
    
    
    return ret;
}



void SlaveLedCommInit(void)
{
      _gstComLedCtlTxData.pData = _garrLedCommTxDataBuf;  
}



void SlaveLedCtlModbusCommProcess(void)
{
    static ModbusRTURevData_t *p;
    static uint32_t led_ctl_comm_time_delay_cnt = 0;
    uint16_t mbrtu_state;
    uint16_t led_ctl_recv_len;

//    if(tim_get_systime() - led_ctl_comm_time_delay_cnt > 500)
//    {
//        led_ctl_comm_time_delay_cnt = tim_get_systime();
//        _gstComLedCtlTxData.len = 5;
//        ModbusRTUSend(_gstComLedCtlTxData.pData, _gstComLedCtlTxData.len);
//    }
//    return;

    switch(geMainCardSlavePollingStatus)
    {
        case EN_MAINCARD_POLLING_STATUS_IDLE:
        {
            //从串口队列中获取数据
            if(QueueGet(&gDevModbus.rev_queue, (void*)&p) == true)
            {
                //对数据进行分析
                mbrtu_state = MBRTU_ParseUp(p->p_data, p->data_size, &gstLedCtlModbusData, &led_ctl_recv_len);
                if(MBRTU_LEN_ERR == mbrtu_state)
                {
                    //数据长度
                    break;
                }
                if(MBRTU_OK != mbrtu_state)
                {
                    //需要返回异常
                    gstLedCtlModbusData.fun |= 0x80;
                    if(MBRTU_FC_ERR == mbrtu_state)
                    {
                        gstLedCtlModbusData.data_space.arr_data[0] = INVALID_FUNC;
                    }
                    else if(MBRTU_ADR_ERR == mbrtu_state)
                    {
                        gstLedCtlModbusData.data_space.arr_data[0] = INVALID_ADDR;
                    }
                    else
                    {
                        gstLedCtlModbusData.data_space.arr_data[0] = INVALID_DATA;
                    }
                    _gstComLedCtlTxData.len = MBRTU_PackUp(_gstComLedCtlTxData.pData, &gstLedCtlModbusData);
                    geMainCardSlavePollingStatus = EN_MAINCARD_POLLING_STATUS_WAITING_TX;
                    led_ctl_comm_time_delay_cnt = tim_get_systime();
                }
                else
                {
                    //数据正常
                    if((MBRTU_FC_READ_STANDREG == gstLedCtlModbusData.fun) || (MBRTU_FC_READ_ORREG == gstLedCtlModbusData.fun))
                    {
                        //读取数据
                        ModbusReadParse(&gstLedCtlModbusData);
                        _gstComLedCtlTxData.len = MBRTU_PackUp(_gstComLedCtlTxData.pData, &gstLedCtlModbusData);
                        geMainCardSlavePollingStatus = EN_MAINCARD_POLLING_STATUS_WAITING_TX;
                        led_ctl_comm_time_delay_cnt = tim_get_systime();
                    }
                    else if((MBRTU_FC_SWRITE_STANDREG == gstLedCtlModbusData.fun) || (MBRTU_FC_WRITE_STANDREG == gstLedCtlModbusData.fun))
                    {
                        //写数据
                        ModbusWriteParse(&gstLedCtlModbusData);
                        _gstComLedCtlTxData.len = MBRTU_PackUp(_gstComLedCtlTxData.pData, &gstLedCtlModbusData);
                        geMainCardSlavePollingStatus = EN_MAINCARD_POLLING_STATUS_WAITING_TX;
                        led_ctl_comm_time_delay_cnt = tim_get_systime();
                    }
                }
            }
            break;
        }
        case EN_MAINCARD_POLLING_STATUS_WAITING_TX:
        {
            if(tim_get_systime() - led_ctl_comm_time_delay_cnt > 50)
            {
                led_ctl_comm_time_delay_cnt = tim_get_systime();
                geMainCardSlavePollingStatus = EN_MAINCARD_POLLING_STATUS_INTERVAL_WAIT;
                ModbusRTUSend(_gstComLedCtlTxData.pData, _gstComLedCtlTxData.len);
            }
            break;
        }
        case EN_MAINCARD_POLLING_STATUS_INTERVAL_WAIT:
        {
            if(tim_get_systime() - led_ctl_comm_time_delay_cnt > 50)
            {
                geMainCardSlavePollingStatus = EN_MAINCARD_POLLING_STATUS_IDLE;
            }
            break;
        }

        default:
            break;
    }
}

void WordExchange(uint16_t *pword)
{
    uint16_t buffer;

    buffer = *pword;
    *pword = (buffer >> 8) | (buffer << 8);
}


/*
将数据从modbus_point 对应位置读取到 ptFrame-> data_space
以便 modeBus再进行转换为串口发送数组中
*/
void ModbusReadParse(ModbusRtuFrame_t *ptFrame)
{
    uint16_t start_addr;
    uint16_t reg_num;
    uint8_t buffer[256];
    uint8_t index;
    uint8_t offset;
    uint16_t i = 0;
    uint16_t reg_addr;
    uint16_t w_buffer;

    const ModbusDataPoint_t * modbus_point;
    uint16_t modbus_point_num;

    modbus_point = ModbusUserData;
    modbus_point_num = ModbusUserDataPointNum;

    start_addr = ptFrame->data_space.start_addr;
    reg_num = ptFrame->data_space.reg_len;
    memset(buffer, 0, 256);

    for(i = 0; i < reg_num; i++)
    {
        reg_addr = start_addr + i;
        for(index = 0; index < modbus_point_num; index++)
        {
            if((NULL != modbus_point[index].base_addr) && ((EN_ATTR_RO == modbus_point[index].attr_rw) || (EN_ATTR_RW == modbus_point[index].attr_rw)))
            {
                if((reg_addr >= modbus_point[index].base_addr) && (reg_addr < (modbus_point[index].base_addr + modbus_point[index].num)))
                {
                    offset = reg_addr - modbus_point[index].base_addr;
                    if((EN_DATA_U8 == modbus_point[index].data_type) || (EN_DATA_S8 == modbus_point[index].data_type))
                    {
                        w_buffer = *((uint8_t *)modbus_point[index].pdata + offset);
                    }
                    else if((EN_DATA_U32))
                    {
                        if((offset % 2) == 0)
                        {
                            w_buffer = *((uint16_t*)modbus_point[index].pdata + offset + 1);
                        }
                        else
                        {
                            w_buffer = *((uint16_t*)modbus_point[index].pdata + offset - 1);
                        }
                    }
                    else if((EN_DATA_STRING == modbus_point[index].data_type))
                    {
                        w_buffer = *((uint16_t*)modbus_point[index].pdata + offset);
                        WordExchange(&w_buffer);
                    }
                    else
                    {
                        w_buffer = *((uint16_t *)modbus_point[index].pdata + offset);
                    }
                    memmove(buffer + i * 2, &w_buffer, 2);
                    break;
                }
            }
        }
    }
    DataModeChange(ptFrame->data_space.arr_data, buffer, USING_MODE_BIG, reg_num * 2);
}


/*
此函数的作用  时将MODEBUS中的数据转换到实际的对应寄存器中。
将ptFrame 中的数据转化到modbus_point 所对应的映射地址中
将从上位机接收到的数据通过解析完成之后 在写入MCU
*/
bool ModbusWriteParse(ModbusRtuFrame_t *ptFrame)
{
    uint16_t start_addr;
    uint16_t reg_num;
    uint16_t i = 0;
    uint16_t index;
    uint16_t offset;
    uint16_t reg_addr;
    uint16_t w_buffer;
    bool write_status = true;


    const  ModbusDataPoint_t * modbus_point;
    uint16_t modbus_point_num;

    modbus_point = ModbusUserData;
    modbus_point_num = ModbusUserDataPointNum;

    start_addr = ptFrame->data_space.start_addr;
    reg_num = ptFrame->data_space.reg_len;

    for(i = 0; i < reg_num; i++)
    {
        reg_addr = start_addr + i;
        for(index = 0; index < modbus_point_num; index++)
        {
            if((reg_addr >= modbus_point[index].base_addr) && (reg_addr < modbus_point[index].base_addr + modbus_point[index].num))
            {
                offset = reg_addr - modbus_point[index].base_addr;
                w_buffer = DataToUnshort(ptFrame->data_space.arr_data + i * 2, USING_MODE_BIG);
                if((EN_DATA_U8 == modbus_point[index].data_type) || (EN_DATA_S8 == modbus_point[index].data_type))
                {
                    *((uint8_t*)modbus_point[index].pdata + offset) = (uint8_t)w_buffer;
                }
                else if(EN_DATA_S32 == modbus_point[index].data_type || (EN_DATA_S32 == modbus_point[index].data_type))
                {
                    if((offset % 2) == 0)
                    {
                        *((uint16_t *)modbus_point[index].pdata + offset + 1) = w_buffer;
                    }
                    else
                    {
                        *((uint16_t *)modbus_point[index].pdata + offset - 1) = w_buffer;
                    }
                }
                else if(EN_DATA_STRING == modbus_point[index].data_type)
                {
                    WordExchange(&w_buffer);
                    *((uint16_t *)modbus_point[index].pdata + offset) = w_buffer;
                }
                else
                {
                    *((uint16_t *)modbus_point[index].pdata + offset) = w_buffer;
                }
                break;
            }
            else
            {
                write_status = false;
                break;
            }
        }
        if((index == modbus_point_num) || (write_status == false))
        {
            write_status = false;
            break;
        }
    }
    return write_status;
}
/**
  * @brief   获取状态氛围灯
  * @param   index: 状态枚举
  * @return  sta:各自状态
  * @note    None
  */
uint16_t GetAtmosLedStatus(EN_ATMOS_LED_STATUS index)
{
    uint16_t led_sta = EN_ATMOS_LED_START_STA;
    switch(index)
    {
        case EN_ATMOS_LED_START_STA:
        {
            led_sta = gstLedCtrlParaData.start_led_mode;
            break; 
        }
        case EN_ATMOS_LED_SYS_STA:
        {
            led_sta = gstLedCtrlParaData.sys_led_mode;
            break;
        }
        case EN_ATMOS_LED_COLLECT_STA:
        {
            led_sta = gstLedCtrlParaData.collector_led_mode;
            break;
        }
        case EN_ATMOS_LED_PCS_STA:
        {
            led_sta = gstLedCtrlParaData.pcs_led_mode;
            break;
        }
        case EN_ATMOS_LED_CONFIRM_KEY_STA:
        {
            led_sta = gstLedCtrlParaData.confirm_led_mode;
        }
        case EN_ATMOS_LED_ESC_KEY_STA:
        {
            led_sta = gstLedCtrlParaData.esc_led_mode;
        }
        case EN_ATMOS_LED_UP_KEY_STA:
        {
            led_sta = gstLedCtrlParaData.up_led_mode;
        }
        case EN_ATMOS_LED_DOWN_KEY_STA:
        {
            led_sta = gstLedCtrlParaData.down_led_mode;
        }
        default:
            break;
    }
    return led_sta;
}

/**
  * @brief   获取按键灯状态
  * @param   index: 状态枚举
  * @return  sta:各自状态
  * @note    None
  */
uint16_t GetKeyAtmosLedStatus(EN_KEY_ATMOS_LED_STATUS sta)
{
    uint16_t key_led_sta;
    switch(sta)
    {
        case EN_ATMOS_LED_CONFIRM_STA:
        {
            key_led_sta = gstLedCtrlParaData.confirm_led_mode;
            gstLedCtrlParaData.confirm_led_mode = 0;
            break;
        }
        case EN_ATMOS_LED_ESC_STA:
        {
            key_led_sta = gstLedCtrlParaData.esc_led_mode;
            gstLedCtrlParaData.esc_led_mode = 0;
            break;
        }
        case EN_ATMOS_LED_UP_STA:
        {
            key_led_sta = gstLedCtrlParaData.up_led_mode;
            gstLedCtrlParaData.up_led_mode = 0;
            break;
        }
        case EN_ATMOS_LED_DOWN_STA:
        {
            key_led_sta = gstLedCtrlParaData.down_led_mode;
            gstLedCtrlParaData.down_led_mode = 0;
            break;
        }
        default:
            break;
    }
    return key_led_sta;
}
