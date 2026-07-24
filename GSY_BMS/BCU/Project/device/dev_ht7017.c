/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_ht7017.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-09
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-09     <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_ht7017.h"

static HTUartRevMsg_t _gHTUartRevMsg;

static uint8_t _gRxBuf[64];
static uint8_t _rev_data;
static EN_DEV_HT7017_STAT _gStatus = EN_DEV_HT7017_STAT_IDLE;
static EN_DEV_HT7017_ACK_RES _gAckRes = EN_DEV_HT7017_ACK_RES_NONE;
static uint8_t rx_ack_addr = 0;

static void _IrqRxProcess(void);

/**
  * @brief   HT7017设备初始化
  * @param   none
  * @return  NULL
  */
void DevHT7017Init()
{
    /*循环buf初始化*/
    _gHTUartRevMsg.buf_in_addr = 0;
    _gHTUartRevMsg.buf_out_addr = 0;
    _gHTUartRevMsg.p_buf = _gRxBuf;
    _gHTUartRevMsg.buf_depth = sizeof(_gRxBuf);
    _gHTUartRevMsg.valid_num = 0;
    
    /*外设驱动初始化*/
    gUartHT7017Handle.pRxBuf = &_rev_data;
    gUartHT7017Handle.rx_size = 1;//不用DMA时，该字段不使用
    gUartHT7017Handle.irq_rx_process = _IrqRxProcess;
    HT7017BspInit();
    
    delay_1ms(20);//确保HT7017上电后内部电源系统建立完成
}


/**
  * @brief   串口中断回调函数
  * @param   none
  * @return  NULL
  */
static void _IrqRxProcess()
{
    if(_gHTUartRevMsg.valid_num >= _gHTUartRevMsg.buf_depth)//缓冲区已满，数据丢弃
    {
        return;
    }
    _gHTUartRevMsg.p_buf[_gHTUartRevMsg.buf_in_addr] = *gUartHT7017Handle.pRxBuf;
    _gHTUartRevMsg.buf_in_addr++;
    _gHTUartRevMsg.buf_in_addr %= _gHTUartRevMsg.buf_depth;
    _gHTUartRevMsg.valid_num++;
}


/**
  * @brief   HT7017写操作
  * @param   cmd:写命令
  * @param   p_data:写数据，固定2个字节
  * @return  NULL
  */
void DevHT7017WriteCmd(uint8_t cmd, uint8_t *p_data)
{
    uint8_t tx_buf[5];
    uint8_t sum = 0;
    
    if(_gStatus != EN_DEV_HT7017_STAT_IDLE)
    {
        return;
    }
    
    tx_buf[0] = HT_HEAD;
    tx_buf[1] = cmd | (1 << 7);
    tx_buf[2] = p_data[0];
    tx_buf[3] = p_data[1];
    sum = tx_buf[0] + tx_buf[1] + tx_buf[2] + tx_buf[3];
    sum = ~sum;
    tx_buf[4] = sum;
    
    gUartHT7017Handle.pTxBuf = tx_buf;
    gUartHT7017Handle.tx_size = 5;
    _gStatus = EN_DEV_HT7017_STAT_WAIT_WRITE_ACK;
    UartSend(&gUartHT7017Handle);  
}


/**
  * @brief   HT7017写操作
  * @param   cmd:写命令
  * @param   p_data:写数据，固定2个字节
  * @return  NULL
  */
void DevHT7017ReadCmd(uint8_t cmd, uint8_t *p_data)
{
    uint8_t tx_buf[2];
    
    if(_gStatus != EN_DEV_HT7017_STAT_IDLE)
    {
        return;
    }
    
    tx_buf[0] = HT_HEAD;
    tx_buf[1] = cmd & (~(1 << 7));
    
    gUartHT7017Handle.pTxBuf = tx_buf;
    gUartHT7017Handle.tx_size = 2;
    _gStatus = EN_DEV_HT7017_STAT_WAIT_READ_ACK;
    UartSend(&gUartHT7017Handle);  
}

/**
  * @brief   HT7017写响应
  * @param   NULL
  * @return  回复结果
  */
EN_DEV_HT7017_ACK_RES DevHT7017WriteAck()
{
    if(_gStatus == EN_DEV_HT7017_STAT_ACKED || _gStatus == EN_DEV_HT7017_STAT_NO_ACK)
    {
        _gStatus = EN_DEV_HT7017_STAT_IDLE;
    }
    return _gAckRes;
}

/**
  * @brief   HT7017读响应
  * @param   p_data:响应的数据指针，固定3个字节
  * @return  回复结果
  */
EN_DEV_HT7017_ACK_RES DevHT7017ReadAck(uint8_t *p_data)
{
    if(_gStatus == EN_DEV_HT7017_STAT_ACKED)
    {
        _gStatus = EN_DEV_HT7017_STAT_IDLE;
    }
    else if(_gStatus == EN_DEV_HT7017_STAT_NO_ACK)
    {
        _gStatus = EN_DEV_HT7017_STAT_IDLE;
    }
    if(_gAckRes == EN_DEV_HT7017_ACK_RES_OK)
    {
        p_data[0] = _gHTUartRevMsg.p_buf[rx_ack_addr % _gHTUartRevMsg.buf_depth];
        p_data[1] = _gHTUartRevMsg.p_buf[(rx_ack_addr + 1) % _gHTUartRevMsg.buf_depth];
        p_data[2] = _gHTUartRevMsg.p_buf[(rx_ack_addr + 2) % _gHTUartRevMsg.buf_depth];
    }
    return _gAckRes;
}



#define HT_ACK_MAX_TIMEOUT      100
/**
  * @brief   HT7017串口接收数据处理大轮询
  * @param   none
  * @return  NULL
  */
void DevHT7017Process()
{
    static uint32_t time_count = 0;
    switch(_gStatus)
    {
        case EN_DEV_HT7017_STAT_IDLE:
        {
            //空闲模式不允许HT7017回复数据，回复了自动丢弃
            _gHTUartRevMsg.valid_num = 0;
            _gHTUartRevMsg.buf_out_addr = _gHTUartRevMsg.buf_in_addr;
            time_count = tim_get_systime();
            _gAckRes = EN_DEV_HT7017_ACK_RES_NONE;
            break;
        }
        case EN_DEV_HT7017_STAT_WAIT_WRITE_ACK:
        {
            if(_gHTUartRevMsg.valid_num != 0)
            {
                if(_gHTUartRevMsg.valid_num == 1)
                {
                    if(_gHTUartRevMsg.p_buf[_gHTUartRevMsg.buf_out_addr] == HT_ACK_OK)
                    {
                        _gAckRes = EN_DEV_HT7017_ACK_RES_OK;
                    }
                    else if(_gHTUartRevMsg.p_buf[_gHTUartRevMsg.buf_out_addr] == HT_ACK_ERROR)
                    {
                        _gAckRes = EN_DEV_HT7017_ACK_RES_INVALID_CHECKSUM;
                    }
                    else
                    {
                        _gAckRes = EN_DEV_HT7017_ACK_RES_ABNORMAL;
                    }
                }
                else
                {
                    _gAckRes = EN_DEV_HT7017_ACK_RES_ABNORMAL;
                }
                _gStatus = EN_DEV_HT7017_STAT_ACKED;
            }
            if((uint32_t)(tim_get_systime() - time_count) > HT_ACK_MAX_TIMEOUT)
            {
                _gStatus = EN_DEV_HT7017_STAT_NO_ACK;
                _gAckRes = EN_DEV_HT7017_ACK_RES_TIMEOUT;
            }
            break;
        }
        
        case EN_DEV_HT7017_STAT_WAIT_READ_ACK:
        {
            if(_gHTUartRevMsg.valid_num != 0)
            {
                if(_gHTUartRevMsg.valid_num > 4)
                {
                    _gAckRes = EN_DEV_HT7017_ACK_RES_ABNORMAL;
                    _gStatus = EN_DEV_HT7017_STAT_ACKED;
                }
                else if(_gHTUartRevMsg.valid_num == 4)
                {
                    uint8_t sum = 0;
                    
                    sum = _gHTUartRevMsg.p_buf[_gHTUartRevMsg.buf_out_addr % _gHTUartRevMsg.buf_depth] + 
                          _gHTUartRevMsg.p_buf[(_gHTUartRevMsg.buf_out_addr + 1) % _gHTUartRevMsg.buf_depth] + 
                          _gHTUartRevMsg.p_buf[(_gHTUartRevMsg.buf_out_addr + 2) % _gHTUartRevMsg.buf_depth];
                    sum = ~sum;
                    //数据校验判断 
                    if(sum != _gHTUartRevMsg.p_buf[_gHTUartRevMsg.buf_out_addr + 3])//数据正确
                    {
                        rx_ack_addr = _gHTUartRevMsg.buf_out_addr;
                        _gAckRes = EN_DEV_HT7017_ACK_RES_OK;
                    }
                    else
                    {
                        _gAckRes = EN_DEV_HT7017_ACK_RES_INVALID_CHECKSUM;
                    }
                    _gStatus = EN_DEV_HT7017_STAT_ACKED;
                }
            }
            if((uint32_t)(tim_get_systime() - time_count) > HT_ACK_MAX_TIMEOUT)
            {
                _gStatus = EN_DEV_HT7017_STAT_NO_ACK;
                _gAckRes = EN_DEV_HT7017_ACK_RES_TIMEOUT;
            }
            break;
        }
        case EN_DEV_HT7017_STAT_ACKED://给应用层预留回读的状态时间
        {
            if((uint32_t)(tim_get_systime() - time_count) > HT_ACK_MAX_TIMEOUT)
            {
                _gStatus = EN_DEV_HT7017_STAT_IDLE;
            }
            break;
        }
        case EN_DEV_HT7017_STAT_NO_ACK://给应用层预留回读的状态时间
        {
            if((uint32_t)(tim_get_systime() - time_count) > 2 * HT_ACK_MAX_TIMEOUT)
            {
                _gStatus = EN_DEV_HT7017_STAT_IDLE;
            }
            break;
        }
        default:break;
    }
}









