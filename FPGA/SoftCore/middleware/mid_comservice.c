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

#define COMSERVICE_BUFF_SIZE  (550)
#define MODBUS_ADDR       (0x06)

#define RET_OK              (0)
#define RET_ERROR           (1)
#define RET_TIMEOUT         (-1)
#define RET_QUEENFULL       (-2)
#define RET_VERIFY_ERROR    (-3)
#define RET_UNUSE_DATA      (2)

static CommPort_t _garrComPort[EN_COMSERVICE_INDEX_NUM];

static uint8_t _garrRxBuff[COMSERVICE_BUFF_SIZE];
static uint8_t _garrTxBuff[COMSERVICE_BUFF_SIZE];
static uint8_t _garrReponsePackHead[] = {0xA5, 0x03, 0x00};


SciRevData_t garrSciData[SCI_REV_QUEUE_DEPTH];
static uint8_t _garrRevData[SCI_REV_QUEUE_DEPTH][COMSERVICE_BUFF_SIZE];
Queue_t gstSciRevQueue;
/*通信管理的默认数据*/
static cmoservice_cfg_t _garrComserviceCfg[EN_COMSERVICE_INDEX_NUM] =
{
    {
        .n_frame_delimiter = 0, \
        .n_frame_delimiter_ack = 0, \
        .c_frame_len = 0,         \
        .c_frame_len_rep = 0,     \
        .c_check_start = 0,        \
        .c_other_len = 0,          \
    },
    {
        .n_frame_delimiter = 0x5A, \
        .n_frame_delimiter_ack = 0xA5, \
        .c_frame_len = 7,         \
        .c_frame_len_rep = 8,     \
        .c_check_start = 2,        \
        .c_other_len = 2,          \
    },
};
static void _MidComserviceComRx(void);
static void _DataIn(void);
static void _DataIn(void)
{
    uint8_t data = 0;
    data = Uart0DataGet();
    mid_comservice_data_in(EN_COMSERVICE_COM_INDEX, &data, 1);
}

BMSUpdataCallFun    BMSUpataFun;
void RegisterBMSUpdataFunPoint(BMSUpdataCallFun call_back_fun)
{
    BMSUpataFun  = call_back_fun;
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
    _garrComPort[EN_COMSERVICE_COM_INDEX].rx_obj.p_buf   = _garrRxBuff;
    _garrComPort[EN_COMSERVICE_COM_INDEX].rx_obj.n_mem_len = COMSERVICE_BUFF_SIZE;
    _garrComPort[EN_COMSERVICE_COM_INDEX].tx_obj.p_buf   = _garrTxBuff;
    _garrComPort[EN_COMSERVICE_COM_INDEX].tx_obj.n_mem_len = COMSERVICE_BUFF_SIZE;

    QueueInit(&gstSciRevQueue, SCI_REV_QUEUE_DEPTH, sizeof(SciRevData_t));
    for(i = 0; i < SCI_REV_QUEUE_DEPTH; i++)
    {
        garrSciData[i].pData = _garrRevData[i];
    }

    Uart0IrqRxProcess = &_DataIn;
}

//TODO 系统主中断频繁调用此函数会导致中断异常，将该函数局部变量改为全局变量得以解决，原因未知
int32_t nn_ret = 0;
uint32_t *np_w_cnt = NULL;
uint32_t nn_r_cnt = NULL;

int32_t mid_comservice_data_in(en_comservice_index_t e_index, uint8_t *p_data, uint32_t len)
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

    if(e_index == EN_COMSERVICE_DEBUG_INDEX)
    {
        if(_garrComPort[e_index].rx_obj.p_buf == p_data)
        {
            /*为了减少互相拷贝的时间，共用了部分通信接口的通信内存*/
            *np_w_cnt = len;
        }
        else if(*np_w_cnt < nn_r_cnt)
        {
            /*写指针在读指针之前(循环了一圈)*/
            if((*np_w_cnt + len) <= nn_r_cnt)
            {
                memcpy(&_garrComPort[e_index].rx_obj.p_buf[*np_w_cnt], p_data, len);
                *np_w_cnt += len;
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

            if((*np_w_cnt + len) <= _garrComPort[e_index].rx_obj.n_mem_len)
            {
                /*队列后方有空可以放下所有数据*/
                memcpy(&_garrComPort[e_index].rx_obj.p_buf[*np_w_cnt], p_data, len);
                *np_w_cnt += len;
            }
            else if(((*np_w_cnt + len) > _garrComPort[e_index].rx_obj.n_mem_len) && \
                    ((len - n_w_remnant_len) <=  nn_r_cnt))
            {
                /*队列后方 + 读指针之前有空可以放下所有数据*/
                memcpy(&_garrComPort[e_index].rx_obj.p_buf[*np_w_cnt], p_data, n_w_remnant_len);
                memcpy(_garrComPort[e_index].rx_obj.p_buf, &p_data[n_w_remnant_len], (len - n_w_remnant_len));
                *np_w_cnt = len - n_w_remnant_len;
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
            *np_w_cnt = len;
        }
        else if((*np_w_cnt + len) < _garrComPort[e_index].rx_obj.n_mem_len)
        {
            memcpy(&_garrComPort[e_index].rx_obj.p_buf[*np_w_cnt], p_data, len);
            *np_w_cnt += len;
        }
        else
        {
            nn_ret = RET_ERROR;
        }
    }

    return nn_ret;
}


bool MidComserviceProcess()  //0 关机 1 -开机
{
    if(QueuePut(&gstSciRevQueue, &garrSciData[1]) == false)
    {
        return false;
    }
    return true;
}

/**
* @brief: _MidComserviceProtocolProcess
* @param: index
* @param: p_data
* @param: len
* @return:
* @note:
*
*/

uint32_t num = 0;

int32_t  MidComserviceProtocolProcess(uint32_t index, uint8_t *p_data, uint32_t len)
{
    uint32_t n_loop = 1;
    int32_t  n_ret = 99;
    static  uint8_t rev_index = 0;
    static EN_PROTOCOL_STATE _gProtocolState = EN_PROTOCOL_STATE_RECIVR_DATA_START;
    static uint8_t wait_flag = 0;
    static uint32_t recv_times = 0;
    do
    {
        n_loop--;
        switch(_gProtocolState)
        {
            case EN_PROTOCOL_STATE_RECIVR_DATA_START:
            {
                if(!wait_flag)
                {
                    wait_flag = 1;
                    recv_times = TimGetSystime();
                }

                /* 确定 包头是否一致 */
                if(len < 3)
                {
                    n_ret = RET_UNUSE_DATA;
                }
                else if(memcmp(&_garrReponsePackHead, p_data, 3) == 0)
                {
                    /* 包头校验正确，下一步 */
                    _gProtocolState = EN_PROTOCOL_STATE_COM_DATA_BUSY;
                    n_loop++;
                }
//                else if(*p_data == MODBUS_ADDR) //收到功能码
//                {

//                }
                else
                {

                }
                if(TimDifftimeFromNow(recv_times) > 100)
                {
                    n_ret = RET_TIMEOUT;
                    wait_flag = 0;
                }
                if(_gProtocolState != EN_PROTOCOL_STATE_RECIVR_DATA_START)
                {
                    wait_flag = 0;
                }
                _garrComPort[index].protocol_obj.pake_len = 0;
                break;
            }
            case EN_PROTOCOL_STATE_COM_DATA_BUSY:
            {
                /* 是否大于协议链长度 */
                if(len >= _garrComserviceCfg[index].c_frame_len_rep)
                {
                    if(index == EN_COMSERVICE_COM_INDEX)
                    {
                        if((((p_data[3] | (uint16_t)p_data[4] << 8) == 0x8002) || ((p_data[3] | (uint16_t)p_data[4] << 8) == 0x80F0) || ((p_data[3] | (uint16_t)p_data[4] << 8) == 0x80F1)  || ((p_data[3] | (uint16_t)p_data[4] << 8) == 0x80FF)) && (CheckSum(p_data, 6) == (p_data[6] + ((uint16_t)p_data[7] << 8))))
                        {
                            //写响应，直接放入队列
                            _gProtocolState = EN_PROTOCOL_STATE_COM_DATA_END;
                            garrSciData[rev_index].data_type = TYPE_COM_WRESPONSE_DATA;//写响应
                            /*计算数据总长度*/
                            _garrComPort[index].protocol_obj.pake_len = 8;
                        }
                        else if((p_data[3] | (uint16_t)p_data[4] << 8) == 0x0001)//一次性回读所有BMS的数据——读响应
                        {
                            /* 计算数据域长度——读响应*/
                            _garrComPort[index].protocol_obj.payload_len = p_data[6] + ((uint16_t)p_data[7] << 8);
                            garrSciData[rev_index].data_type = TYPE_COM_RRESPONSE_DATA;//读响应

                            /*计算数据总长度*/
                            _garrComPort[index].protocol_obj.pake_len += _garrComserviceCfg[index].c_frame_len_rep;
                            _garrComPort[index].protocol_obj.pake_len += _garrComPort[index].protocol_obj.payload_len;
                            _garrComPort[index].protocol_obj.pake_len += _garrComserviceCfg[index].c_other_len;
                            _gProtocolState = EN_PROTOCOL_STATE_COM_DATA_END;
                            n_loop++;
                        }
                    }
                }
                break;
            }

//            case EN_PROTOCOL_STATE_FRAME_COLLECT_COM:
//            {
//                /* 是否大于协议链长度 */
//                if(len >= _garrComserviceCfg[index].c_frame_len)
//                {
//                    if(index == EN_COMSERVICE_COM_INDEX)
//                    {
//                        ///* 自定义数据包第一层通信解析 */
//                        //protocal_request_packet_t *p_head = (protocal_request_packet_t*)p_data;
//                        /* 计算负载区长度 */
//                        _garrComPort[index].protocol_obj.payload_len = p_data[len - 2] | ((uint16_t)p_data[len - 1] << 8);
//                    }

//                    /*记录协议中payload长度*/
//                    _garrComPort[index].protocol_obj.pake_len += _garrComserviceCfg[index].c_frame_len;
//                    _garrComPort[index].protocol_obj.pake_len += _garrComPort[index].protocol_obj.payload_len;
//                    _garrComPort[index].protocol_obj.pake_len += _garrComserviceCfg[index].c_other_len;
//                    _gProtocolState = EN_PROTOCOL_STATE_COM_DATA_END;

//                    garrSciData[rev_index].data_type = TYPE_COM_RRESPONSE_DATA;
//                    garrSciData[rev_index].data_size = _garrComPort[index].protocol_obj.pake_len;

//                }
//                else if(TimDifftimeFromNow(comm_timeout) > 500)
//                {
//                    _gProtocolState = EN_PROTOCOL_TIMEOUT;
//                }
//                break;
//            }

//            case EN_PROTOCOL_STATE_MODBUS_DATA_BUSY:
//            {
//                // Modbus 0x03  0x04 0x06协议长度为8个字节   0x10到data_len 是7个字节
//                if(len >= 8)
//                {
//                    if(index == EN_COMSERVICE_COM_INDEX)
//                    {
//                        /* 计算负载区长度 */
//                        _garrComPort[index].protocol_obj.payload_len = p_data[6];
//                    }

//                    /*记录协议中payload长度*/
//                    if(p_data[1] == 0x10)
//                    {
//                        // 0x10寄存器7个字节包头+2字节校验+data
//                        _garrComPort[index].protocol_obj.pake_len = _garrComPort[index].protocol_obj.payload_len + 9;
//                    }
//                    _gProtocolState = EN_PROTOCOL_STATE_DATA_COLLECT_MOD;
//                }
//                break;
//            }

            case EN_PROTOCOL_STATE_COM_DATA_END:
            {
                if(len >= _garrComPort[index].protocol_obj.pake_len)
                {
                    if(QueueIsFull(&gstSciRevQueue) == true)//TODO 放入队列
                    {
                        n_ret = RET_QUEENFULL;
                    }
                    if(_garrComPort[index].protocol_obj.pake_len == 8)
                    {
                        if(((p_data[3] | (uint16_t)p_data[4] << 8) == 0x80F0) || ((p_data[3] | (uint16_t)p_data[4] << 8) == 0x80F1)  || ((p_data[3] | (uint16_t)p_data[4] << 8) == 0x80FF))
                        {
                            //判断是属于升级程序
                            if(BMSUpataFun != 0)
                            {
                                BMSUpataFun(p_data, 8);
                                _gProtocolState = EN_PROTOCOL_STATE_RECIVR_DATA_START;
                            }
                        }
                        garrSciData[rev_index].data_type = TYPE_COM_WRESPONSE_DATA;
                        garrSciData[rev_index].data_size = _garrComPort[index].protocol_obj.pake_len;
                        n_ret = RET_OK;
                    }
                    else
                    {
                        garrSciData[rev_index].data_type = TYPE_COM_RRESPONSE_DATA;
                        garrSciData[rev_index].data_size = _garrComPort[index].protocol_obj.pake_len;
                        n_ret = RET_OK;
                    }
                    _gProtocolState = EN_PROTOCOL_STATE_RECIVR_DATA_START;
                }
                n_loop = 0;
                break;
            }
//            case EN_PROTOCOL_STATE_DATA_COLLECT_MOD:
//            {
//                if(p_data[1] == 0x10)
//                {
//                    if(len >= _garrComPort[index].protocol_obj.pake_len)
//                    {
//                        //TODO 放入队列
//                        if(CrcJudge(p_data, _garrComPort[index].protocol_obj.pake_len) == false)
//                        {
//                            n_ret = RET_VERIFY_ERROR;
//                        }
//                        else
//                        {
//                            if(QueueIsFull(&gstSciRevQueue) == true)
//                            {
//                                n_ret = RET_QUEENFULL;
//                            }
//                            else
//                            {
//                                garrSciData[rev_index].data_type = TYPE_MODBUS_DATA;
//                                garrSciData[rev_index].slave_addr = p_data[0];
//                                garrSciData[rev_index].data_size = _garrComPort[index].protocol_obj.pake_len;
//                                n_ret = RET_OK;
//                            }
//                        }
//                    }
//                    else
//                    {

//                    }
//                }
//                else
//                {
//                    if(CrcJudge(p_data, 8) == false)
//                    {
//                        n_ret = RET_VERIFY_ERROR;
//                    }
//                    else
//                    {
//                        //TODO 放入队列
//                        if(QueueIsFull(&gstSciRevQueue) == true)
//                        {
//                            n_ret = RET_QUEENFULL;
//                        }
//                        else
//                        {
//                            garrSciData[rev_index].data_type = TYPE_MODBUS_DATA;
//                            garrSciData[rev_index].slave_addr = p_data[0];
//                            garrSciData[rev_index].data_size = 8;
//                        }
//                    }
//                }
//                break;
//            }

//            case EN_PROTOCOL_TIMEOUT:
//            {
//                if(TimDifftimeFromNow(comm_timeout) > 100)
//                {
//                    comm_timeout = TimGetSystime();
//                    comm_timeout_count++;
//                }
//                if(comm_timeout_count > 10)
//                {
//                    n_ret = RET_TIMEOUT;
//                    comm_timeout_count = 0;
//                    memset((uint8_t*)&gSlavePara.bms_para.bms_only_read_para, 0, sizeof(BMSOnlyReadPara_t));
//                    _gProtocolState = EN_PROTOCOL_STATE_RECIVR_DATA_START;
//                }
//                break;
//            }

            default:
                break;
        }
    }
    while(n_loop != 0);

    if(n_ret <= RET_OK)
    {
        if(n_ret == RET_OK)
        {
            num = n_ret;
            memcpy(garrSciData[rev_index].pData, p_data, garrSciData[rev_index].data_size);
            if(QueuePut(&gstSciRevQueue, &garrSciData[rev_index]) == false)
            {
                n_ret = RET_QUEENFULL;
            }
            rev_index++;
            rev_index %= SCI_REV_QUEUE_DEPTH;
        }
        _gProtocolState = EN_PROTOCOL_STATE_RECIVR_DATA_START;
    }
    return n_ret;
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
            if(i == EN_COMSERVICE_DEBUG_INDEX)
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
                n_ret = MidComserviceProtocolProcess(i, _garrComPort[i].rx_obj.p_buf, _garrComPort[i].rx_obj.n_w_cnt);
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
* @brief: MidComserviceHandle
* @param: void
* @note:
*
*/
void MidComserviceHandle(void)
{
//    static uint32_t scia_recover_time = 0;
//    static uint16_t sica_norecv_count = 0;

//    /* 解决Scia偶现无法接收数据 */
//    if(TimDifftimeFromNow(scia_recover_time) > 1000)
//    {
//        scia_recover_time = TimGetSystime();
//        //TODO
//        if(GetSciaRecvFlag())
//        {
//            ClearSciaRecvFlag();
//            sica_norecv_count = 0;
//        }
//        else
//        {
//            sica_norecv_count++;

//            if(sica_norecv_count > 3)
//            {
//                SciaFifoInit();
//                sica_norecv_count = 0;
//            }
//        }
//    }

    _MidComserviceComRx();
}
