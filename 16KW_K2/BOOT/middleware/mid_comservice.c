/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        mid_comservice.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "mid_comservice.h"

#define RET_OK        (0)
#define RET_ERROR     (1)
#define RET_TIMEOUT     (-1)
#define RET_QUEENFULL     (-2)
#define RET_VERIFY_ERROR  (-3)
#define RET_UNUSE_DATA    (2)

static CommPort_t _garrComPort[EN_COMSERVICE_INDEX_NUM] = {0};
static uint8_t _garrRxBuff0[COMSERVICE_BUFF_SIZE] = {0};
static uint8_t _garrRxBuff1[COMSERVICE_BUFF_SIZE] = {0};
const uint8_t _garrModbusFunCode[] = {0x03, 0x04, 0x06, 0x10};

static SciRevData_t _garrSciData[SCI_REV_QUEUE_DEPTH];
static uint8_t _garrRevData[SCI_REV_QUEUE_DEPTH][COMSERVICE_BUFF_SIZE];
Queue_t gstSciRevQueue;

/*通信管理的默认数据*/
static cmoservice_cfg_t _garrComserviceCfg[] =
{
    {
        \
        .frame_delimiter = 0x5A,\
        .frame_delimiter_ack = 0xA5,\
        .c_frame_len = 7,         \
        .c_frame_len_rep = 8,     \
        .c_check_start = 2,        \
        .c_other_len = 2,          \

        .modbus_addr = MODBUS_HOST0_ADDR,\
        .modbus_type = 1,
        .modbus_slave0_addr = MODBUS_SLAVE0_ADDR,\
        .modbus_slave1_addr = MODBUS_SLAVE1_ADDR,\
        .modbus_slave2_addr = MODBUS_SLAVE2_ADDR,\
    },
    {
        \
        .frame_delimiter = 0x5A,\
        .frame_delimiter_ack = 0xA5,\
        .c_frame_len = 7,         \
        .c_frame_len_rep = 8,     \
        .c_check_start = 2,        \
        .c_other_len = 2,          \

        .modbus_addr = MODBUS_HOST1_ADDR,\
        .modbus_type = 1,
        .modbus_slave0_addr = MODBUS_SLAVE3_ADDR,\
        .modbus_slave1_addr = MODBUS_SLAVE4_ADDR,\
        .modbus_slave2_addr = MODBUS_SLAVE5_ADDR,\
    },
};
static void _MidComserviceComRx(void);
static int32_t _MidComserviceProtocolProcess(uint32_t n_index, uint8_t *p_data, uint32_t n_len);

uint16_t CheckSum(uint8_t *p, uint16_t size)
{
    uint16_t i;
    uint16_t ret = 0;
    for(i = 0; i < size; i++)
    {
        ret += p[i];
    }
    return ret;
}

uint32_t CheckSumFun(uint8_t *pdata, uint16_t len, uint8_t mode)
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

/**
* @brief: MidComserviceInit
* @param: void                        
* @note: 
*
*/
void MidComserviceInit(void)
{
    uint8_t i = 0;

    memset((uint8_t*)&_garrComPort, 0, EN_COMSERVICE_INDEX_NUM * sizeof(CommPort_t));
    _garrComPort[EN_COMSERVICE_COM_INDEX0].rx_obj.p_buf   = _garrRxBuff0;
    _garrComPort[EN_COMSERVICE_COM_INDEX0].rx_obj.n_mem_len = COMSERVICE_BUFF_SIZE;
    _garrComPort[EN_COMSERVICE_COM_INDEX1].rx_obj.p_buf   = _garrRxBuff1;
    _garrComPort[EN_COMSERVICE_COM_INDEX1].rx_obj.n_mem_len = COMSERVICE_BUFF_SIZE;
//    _garrComPort[EN_COMSERVICE_COM_INDEX0].tx_obj.p_buf   = _garrTxBuff;
//    _garrComPort[EN_COMSERVICE_COM_INDEX0].tx_obj.n_mem_len = COMSERVICE_BUFF_SIZE;

    QueueInit(&gstSciRevQueue, SCI_REV_QUEUE_DEPTH, sizeof(SciRevData_t));
    for(i = 0; i < SCI_REV_QUEUE_DEPTH; i++)
    {
        _garrSciData[i].pData = _garrRevData[i];
    }
}

//TODO 系统主中断频繁调用此函数会导致中断异常，将该函数局部变量改为全局变量得以解决，原因未知
int32_t nn_ret = 0;
uint32_t *np_w_cnt =NULL;
uint32_t nn_r_cnt =NULL;

int32_t mid_comservice_data_in(EN_COMSERVICE_INDEX e_index, uint8_t *p_data, uint32_t n_len)
{
    if(e_index > EN_COMSERVICE_INDEX_NUM)
    {
        nn_ret = RET_ERROR;
    }
    if(p_data == NULL)
    {
        nn_ret = RET_ERROR;
    }

    /*判断当前空间是否还有空余可以写入参数*/
    np_w_cnt = &_garrComPort[e_index].rx_obj.n_w_cnt;
    nn_r_cnt = _garrComPort[e_index].rx_obj.n_r_cnt;

//    if(e_index == EN_COMSERVICE_DEBUG_INDEX)
    if(0)
    {
        if(_garrComPort[e_index].rx_obj.p_buf == p_data)
        {
            /*为了减少互相拷贝的时间，共用了部分通信接口的通信内存*/
            *np_w_cnt = n_len;
        }
        else if(*np_w_cnt < nn_r_cnt)
        {
            /*写指针在读指针之前(循环了一圈)*/
            if((*np_w_cnt + n_len) <= nn_r_cnt)
            {
                memcpy(&_garrComPort[e_index].rx_obj.p_buf[*np_w_cnt], p_data, n_len);
                *np_w_cnt += n_len;
            }
            else
            {
                nn_ret = RET_ERROR;
            }
        }
        else
        {
            /*写指针在读指针之后(未循环)*/
            uint32_t n_w_remnant_len = _garrComPort[e_index].rx_obj.n_mem_len - *np_w_cnt;

            if((*np_w_cnt + n_len) <= _garrComPort[e_index].rx_obj.n_mem_len)
            {
                /*队列后方有空可以放下所有数据*/
                memcpy(&_garrComPort[e_index].rx_obj.p_buf[*np_w_cnt], p_data, n_len);
                *np_w_cnt += n_len;
            }
            else if(((*np_w_cnt + n_len) > _garrComPort[e_index].rx_obj.n_mem_len) &&\
                    ((n_len - n_w_remnant_len) <=  nn_r_cnt))
            {
                /*队列后方 + 读指针之前有空可以放下所有数据*/
                memcpy(&_garrComPort[e_index].rx_obj.p_buf[*np_w_cnt], p_data, n_w_remnant_len);
                memcpy(_garrComPort[e_index].rx_obj.p_buf, &p_data[n_w_remnant_len], (n_len - n_w_remnant_len));
                *np_w_cnt = n_len - n_w_remnant_len;
            }
            else
            {
                nn_ret = RET_ERROR;
            }
        }
    }
    else
    {
        if(_garrComPort[e_index].rx_obj.p_buf == p_data)
        {
            /*为了减少互相拷贝的时间，共用了部分通信接口的通信内存*/
            *np_w_cnt = n_len;
        }
        else if((*np_w_cnt + n_len) < _garrComPort[e_index].rx_obj.n_mem_len)
        {
            memcpy(&_garrComPort[e_index].rx_obj.p_buf[*np_w_cnt], p_data, n_len);
            *np_w_cnt += n_len;
        }
        else
        {
            nn_ret = RET_ERROR;
        }
    }

    return nn_ret;
}

/**
* @brief: MidComserviceHandle
* @param: void                        
* @note: 
*
*/
void MidComserviceHandle(void)
{
    _MidComserviceComRx();
}


/**
* @brief: _MidComserviceComRx
* @param: void                        
* @return: 
* @note: 
*
*/
static void _MidComserviceComRx(void)
{
    uint32_t i = 0;
    int32_t  n_ret       = 0;
    uint32_t n_len_first = 0;   /*第一段数据长度*/
    uint32_t n_len_second = 0;  /*第二段折叠回来的数据长度*/

    for(i = 0; i < EN_COMSERVICE_INDEX_NUM; i++)
    {
        if(_garrComPort[i].rx_obj.n_w_cnt != _garrComPort[i].rx_obj.n_r_cnt)
        {
            if(_garrComPort[i].rx_obj.n_w_cnt > _garrComPort[i].rx_obj.n_r_cnt)
            {
                /*写指针在读指针之后*/
                n_len_first = _garrComPort[i].rx_obj.n_w_cnt - _garrComPort[i].rx_obj.n_r_cnt;
            }
            else
            {
                /*写指针在读指针之前*/
                n_len_first   = _garrComPort[i].rx_obj.n_mem_len - _garrComPort[i].rx_obj.n_r_cnt;
                n_len_second  = _garrComPort[i].rx_obj.n_w_cnt;
            }

//            if(i == EN_COMSERVICE_DEBUG_INDEX)
            if(0)
            {
                /*debug 处理*/
                if(n_len_first != 0)
                {
                    //TODO  debug
//                    console_recive_handler(&_garrComPort[i].rx_obj.p_buf[_garrComPort[i].rx_obj.n_r_cnt], n_len_first);
                    _garrComPort[i].rx_obj.n_r_cnt += n_len_first;
                }
                if(n_len_second != 0)
                {
                    //TODO  debug
//                    console_recive_handler(_garrComPort[i].rx_obj.p_buf, n_len_second);
                    _garrComPort[i].rx_obj.n_r_cnt = n_len_second;
                }
            }
            else
            {
                n_ret = _MidComserviceProtocolProcess(i, _garrComPort[i].rx_obj.p_buf, _garrComPort[i].rx_obj.n_w_cnt);
                if(n_ret <= 0)
                {
                    _garrComPort[i].rx_obj.n_w_cnt = 0;
                    _garrComPort[i].rx_obj.n_r_cnt = 0;
                }
            }

        }
    }
}

/**
* @brief: _MidComserviceProtocolProcess
* @param: n_index                     
* @param: p_data                      
* @param: n_len                       
* @return: 
* @note: 
*
*/
static int32_t _MidComserviceProtocolProcess(uint32_t n_index, uint8_t *p_data, uint32_t n_len)
{
    int32_t n_ret = 1;
    uint32_t n_loop = 1;
    static uint8_t rev_index = 0;
    static uint8_t arr_wait_flag[EN_COMSERVICE_INDEX_NUM] = {0, 0};
//    uint8_t *p_found_funcode = NULL;

    do
    {
        n_loop--;

        switch(_garrComPort[n_index].protocol_obj.e_state)
        {
            case EN_PROTOCOL_STATE_IDLE:
            {
                if(!arr_wait_flag[n_index])
                {
                    arr_wait_flag[n_index] = 1;
                    _garrComPort[n_index].n_timeout = TimGetSystime();
                }
                
                /* 确定 包头是否一致 */
                if(n_len < 3)
                {
                    n_ret = RET_UNUSE_DATA;
                }

                else if(p_data[0] == _garrComserviceCfg[n_index].frame_delimiter)
                {
                    if(p_data[2] == COM_ADDR)
                    {
                        /*请求包*/
                        _garrComPort[n_index].pack_request_type = _garrComserviceCfg[n_index].frame_delimiter;
                        /* 包头校验正确，下一步 */
                        _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_FRAME_COLLECT_COM;
                        _garrComPort[n_index].n_timeout = TimGetSystime();
                        n_loop++;
                    }
                    else
                    {
                        /*包头校验错误,直接清除*/
                        n_ret = RET_TIMEOUT;
                        arr_wait_flag[n_index] = 0;
                    }

                }
                else if(p_data[0] == _garrComserviceCfg[n_index].frame_delimiter_ack)
                {
                    
                    if(p_data[2] == COM_ADDR)
                    {
                        /*响应包*/
                        _garrComPort[n_index].pack_request_type = _garrComserviceCfg[n_index].frame_delimiter_ack;
                        /* 包头校验正确，下一步 */
                        _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_FRAME_COLLECT_COM;
                        _garrComPort[n_index].n_timeout = TimGetSystime();
                        n_loop++;
                    } 
                    else
                    {
                        /*包头校验错误,直接清除*/
                        n_ret = RET_TIMEOUT;
                        arr_wait_flag[n_index] = 0;
                    }                    
                }
                else if(memchr(_garrModbusFunCode, p_data[1], sizeof(_garrModbusFunCode)) != NULL)   //确保收到功能码
                {
                    /*本机作为主机解析*/
                    if(_garrComserviceCfg[n_index].modbus_type)
                    {
                        if((p_data[0] == _garrComserviceCfg[n_index].modbus_slave0_addr) || \
                           (p_data[0] == _garrComserviceCfg[n_index].modbus_slave1_addr) || \
                           (p_data[0] == _garrComserviceCfg[n_index].modbus_slave2_addr))
                        {
                            _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_FRAME_COLLECT_MOD;
                            _garrComPort[n_index].n_timeout = TimGetSystime();
                            n_loop++;                            
                        }
                        else
                        {
                            /*非访问本节点*/
                            n_ret = RET_TIMEOUT;
                            arr_wait_flag[n_index] = 0;
                        }

                    }
                    /*本机作为从机解析*/
                    else
                    {
                        if(p_data[0] == _garrComserviceCfg[n_index].modbus_addr)
                        {
                            _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_FRAME_COLLECT_MOD;
                            _garrComPort[n_index].n_timeout = TimGetSystime();
                            n_loop++;                            
                        }
                        else
                        {
                            /*非访问本节点*/
                            n_ret = RET_TIMEOUT;
                            arr_wait_flag[n_index] = 0;
                        }
                    }
                }
                else
                {

                }

                /*等待接收数据超时*/
                if(TimDifftimeFromNow(_garrComPort[n_index].n_timeout) > 70)
                {
                    n_ret = RET_TIMEOUT;
                    arr_wait_flag[n_index] = 0;
                } 
                /*解析到数据头*/
                if(_garrComPort[n_index].protocol_obj.e_state != EN_PROTOCOL_STATE_IDLE)
                {
                    _garrComPort[n_index].n_timeout = TimGetSystime();
                    arr_wait_flag[n_index] = 0;
                }
                _garrComPort[n_index].protocol_obj.n_pake_len = 0;
                
                break;
            }
            case EN_PROTOCOL_STATE_FRAME_COLLECT_COM:
            {
                if((n_index == EN_COMSERVICE_COM_INDEX0) || (n_index == EN_COMSERVICE_COM_INDEX1))
                {
                    /*请求包*/
                    if(_garrComPort[n_index].pack_request_type == _garrComserviceCfg[n_index].frame_delimiter)
                    {
                        /* 是否大于协议链长度 */
                        if(n_len >= _garrComserviceCfg[n_index].c_frame_len)
                        {
                            
                            _garrComPort[n_index].protocol_obj.n_payload_len = p_data[5] + ((uint16_t)p_data[6] << 8);

                            /*cmd最高位为1，表示写请求*/
                            if((p_data[4] >> 7) & 0x01)
                            {
                                _garrComPort[n_index].e_pack_operation_type = EN_PROTOCOL_CODE_WRITE;
                            }
                            else
                            {
                                /*cmd最高位为0，表示读请求*/
                                _garrComPort[n_index].e_pack_operation_type = EN_PROTOCOL_CODE_READ;
                            }
                            /*记录协议中payload长度*/
                            _garrComPort[n_index].protocol_obj.n_pake_len += _garrComserviceCfg[n_index].c_frame_len;
                            _garrComPort[n_index].protocol_obj.n_pake_len += _garrComPort[n_index].protocol_obj.n_payload_len;
                            _garrComPort[n_index].protocol_obj.n_pake_len += _garrComserviceCfg[n_index].c_other_len;
                            _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_DATA_COLLECT_COM;
                            _garrComPort[n_index].n_timeout = TimGetSystime();
                            n_loop++;
                        }
                    }
                    /*响应包*/
                    else if(_garrComPort[n_index].pack_request_type == _garrComserviceCfg[n_index].frame_delimiter_ack)
                    {
                        /* 是否大于协议链长度（读响应） */
                        if(n_len >= _garrComserviceCfg[n_index].c_frame_len_rep)
                        {
                            /*cmd最高位为1，表示请求*/
                            if((p_data[4] >> 7) & 0x01)
                            {
                                _garrComPort[n_index].e_pack_operation_type = EN_PROTOCOL_CODE_WRITE;
                                _garrComPort[n_index].protocol_obj.n_payload_len = p_data[6] + ((uint16_t)p_data[7] << 8);
                        
                                /*记录协议中payload长度*/
                                _garrComPort[n_index].protocol_obj.n_pake_len += _garrComserviceCfg[n_index].c_frame_len_rep;
                                _garrComPort[n_index].protocol_obj.n_pake_len += _garrComPort[n_index].protocol_obj.n_payload_len;
                                _garrComPort[n_index].protocol_obj.n_pake_len += _garrComserviceCfg[n_index].c_other_len;
                                _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_DATA_COLLECT_COM;
                                _garrComPort[n_index].n_timeout = TimGetSystime();                                   
                                n_loop++;
                            }
                            else
                            {
                                /*cmd最高位为0，表示响应*/
                                _garrComPort[n_index].e_pack_operation_type = EN_PROTOCOL_CODE_READ;
                                /*记录协议中payload长度*/
                                _garrComPort[n_index].protocol_obj.n_pake_len += _garrComserviceCfg[n_index].c_frame_len_rep;
                                /*写响应7、8字节就是校验和，所以这里不能加上校验和的长度*/
//                                    _garrComPort[n_index].protocol_obj.n_pake_len += _garrComserviceCfg[n_index].c_other_len;
                                _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_DATA_COLLECT_COM;
                                _garrComPort[n_index].n_timeout = TimGetSystime();                         
                                n_loop++;
                            }
                        }
                    }  

                }

                //一帧100ms超时
                if(TimDifftimeFromNow(_garrComPort[n_index].n_timeout) > 100)
                {
                    n_ret = RET_TIMEOUT;
                    n_loop = 0;
                }
                break;
            }
            case EN_PROTOCOL_STATE_FRAME_COLLECT_MOD:
            {
                /*本机作为主机解析*/
                if(_garrComserviceCfg[n_index].modbus_type)
                {
                    if((p_data[1] == 0x10) || (p_data[1] == 0x06))
                    {
                        /*0x10 | 地址 | 功能码 | 起始地址 | 寄存器数量 | CRC |*/
                        /*0x06| 地址（1字节） | 功能码（0x06） | 寄存器地址（2字节） | 写入值（2字节） | CRC校验（2字节） |*/
                        _garrComPort[n_index].protocol_obj.n_pake_len = 8;
                    }
                    else
                    {
                        /*0x03 0x04*/
                       /* | 地址（1字节） | 功能码（0x03） | 字节数（1字节） | 寄存器值（N字节） | CRC校验（2字节） |*/
                        _garrComPort[n_index].protocol_obj.n_pake_len = p_data[2] + 5;
                    }

                    _garrComPort[n_index].n_timeout = TimGetSystime();
                    _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_DATA_COLLECT_MOD;
                    n_loop++;
                }
                /*本机作为从机解析*/
                else
                {
                    // Modbus 0x03  0x04 0x06协议长度为8个字节   0x10到data_len 是7个字节
                    if(n_len >= 8)
                    {
                        if((n_index == EN_COMSERVICE_COM_INDEX0) || (n_index == EN_COMSERVICE_COM_INDEX1))
                        {
                            /* 计算负载区长度 */
                            _garrComPort[n_index].protocol_obj.n_payload_len = p_data[6];
                        }

                        /*记录协议中payload长度*/
                        if(p_data[1] == 0x10)
                        {
                            // 0x10寄存器7个字节包头+2字节校验+data
                            _garrComPort[n_index].protocol_obj.n_pake_len = _garrComPort[n_index].protocol_obj.n_payload_len + 9;
                        }
                        else
                        {
                            _garrComPort[n_index].protocol_obj.n_pake_len = 8;
                        }
                        _garrComPort[n_index].n_timeout = TimGetSystime();
                        _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_DATA_COLLECT_MOD;
                        n_loop++;
                    }
                }

                //一帧100ms超时
                if(TimDifftimeFromNow(_garrComPort[n_index].n_timeout) > 100)
                {
                    n_ret = RET_TIMEOUT;
                    n_loop = 0;
                }
                break;
            }
            case EN_PROTOCOL_STATE_DATA_COLLECT_COM:
            {
                if(n_len >= _garrComPort[n_index].protocol_obj.n_pake_len)
                {
                    //TODO 放入队列
                    if(QueueIsFull(&gstSciRevQueue) == true)
                    {
                        n_ret = RET_QUEENFULL;
                    }
                    else
                    {
                        if((_garrComPort[n_index].pack_request_type == _garrComserviceCfg[n_index].frame_delimiter) && (_garrComPort[n_index].e_pack_operation_type == EN_PROTOCOL_CODE_WRITE))
                        {
                            _garrSciData[rev_index].data_type = TYPE_COM_WREQUEST_DATA;
//                            printf("TYPE_COM_WREQUEST_DATA\r\n");
                        }
                        else if((_garrComPort[n_index].pack_request_type == _garrComserviceCfg[n_index].frame_delimiter) && (_garrComPort[n_index].e_pack_operation_type == EN_PROTOCOL_CODE_READ))
                        {
                            _garrSciData[rev_index].data_type = TYPE_COM_RREQUEST_DATA;
//                            printf("TYPE_COM_RREQUEST_DATA\r\n");
                        }
                        else if((_garrComPort[n_index].pack_request_type == _garrComserviceCfg[n_index].frame_delimiter_ack) && (_garrComPort[n_index].e_pack_operation_type == EN_PROTOCOL_CODE_WRITE))
                        {
                            _garrSciData[rev_index].data_type = TYPE_COM_WRESPONSE_DATA;
//                            printf("TYPE_COM_WRESPONSE_DATA\r\n");
                        }
                        else if((_garrComPort[n_index].pack_request_type == _garrComserviceCfg[n_index].frame_delimiter_ack) && (_garrComPort[n_index].e_pack_operation_type == EN_PROTOCOL_CODE_READ))
                        {
                            _garrSciData[rev_index].data_type = TYPE_COM_RRESPONSE_DATA;
//                            printf("TYPE_COM_RRESPONSE_DATA\r\n");
                        }

                        _garrSciData[rev_index].sci_index = (EN_COMSERVICE_INDEX)n_index;
                        _garrSciData[rev_index].data_size = _garrComPort[n_index].protocol_obj.n_pake_len;
                        n_ret = RET_OK;
                    }
                    n_loop = 0;
                }
                else
                {
                    //一帧100ms超时
                    if(TimDifftimeFromNow(_garrComPort[n_index].n_timeout) > 300)
                    {
                        n_ret = RET_TIMEOUT;
                        n_loop = 0;
                    }
                }
                break;
            }
            case EN_PROTOCOL_STATE_DATA_COLLECT_MOD:
            {
                if(n_len >= _garrComPort[n_index].protocol_obj.n_pake_len)
                {
                    //TODO 放入队列
                    if(CrcJudge(p_data, _garrComPort[n_index].protocol_obj.n_pake_len) == false)
                    {
                        n_ret = RET_VERIFY_ERROR;
                    }
                    else
                    {
                        if(QueueIsFull(&gstSciRevQueue) == true)
                        {
                            n_ret = RET_QUEENFULL;
                        }
                        else
                        {
                            /*本机作为主机解析*/
                            if(_garrComserviceCfg[n_index].modbus_type)
                            {
                                _garrSciData[rev_index].data_type = TYPE_MODBUS_HOST_DATA;
//                                printf("TYPE_MODBUS_HOST_DATA\r\n");
                            }
                            /*本机作为从机解析*/
                            else
                            {
                                _garrSciData[rev_index].data_type = TYPE_MODBUS_SLAVE_DATA;
//                                printf("TYPE_MODBUS_SLAVE_DATA\r\n");
                            }

                            _garrSciData[rev_index].sci_index = (EN_COMSERVICE_INDEX)n_index;
                            _garrSciData[rev_index].slave_addr = p_data[0];
                            _garrSciData[rev_index].data_size = _garrComPort[n_index].protocol_obj.n_pake_len;
                            n_ret = RET_OK;
                        }
                    }
                    n_loop = 0;
                    }
                    else
                    {
                        //一帧500ms超时
                        if(TimDifftimeFromNow(_garrComPort[n_index].n_timeout) > 500)
                        {
                            n_ret = RET_TIMEOUT;
                            n_loop = 0;
                        }
                    }

                break;
            }
            case EN_PROTOCOL_TIMEOUT:
            {
                if(TimDifftimeFromNow(_garrComPort[n_index].n_timeout) > 50)
                {
                    n_ret = RET_TIMEOUT;
                    n_loop = 0;
                }
                else
                {
                    n_loop++;
                }
                break;
            }
            default:
                break;
        }
    }
    while(n_loop != 0);

    if(n_ret <= RET_OK)
    {
        if(n_ret == RET_OK)
        {
            memcpy(_garrSciData[rev_index].pData, p_data, _garrSciData[rev_index].data_size);
            if(QueuePut(&gstSciRevQueue, &_garrSciData[rev_index]) == false)
            {
                n_ret = RET_QUEENFULL;;
            }
            rev_index++;
            rev_index %= SCI_REV_QUEUE_DEPTH;
        }
        _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_IDLE;
    }
    return n_ret;
}
