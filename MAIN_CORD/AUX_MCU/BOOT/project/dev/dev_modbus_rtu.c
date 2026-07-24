


/************************************************************************
*                                 Head Files                            *
************************************************************************/
#include "dev_modbus_rtu.h"

/************************************************************************
*                                Macro Definition                       *
************************************************************************/
//#define COLLECT_SLAVE_ADR   0X01

/************************************************************************
*                         Global Function Realize                       *
************************************************************************/

static uint8_t  _gSlaveCommAddr = 0x01;


/************************************************************************
@ Function Name: MBRTU_ParseUp
@ Description  : modbus parse data(BMS been slave)
@ Input    Para: Recive buf, len, frame
@ Output   Para: None
@ Return  Value: Success & Failed

*************************************************************************/
uint16_t MBRTU_ParseUp(uint8_t *p_recv_buf, uint16_t rec_len, ModbusRtuFrame_t *ptFrameInfo, uint16_t *real_len)
{
    uint8_t  reg_size    = 0;
    uint16_t length     = 0;
    uint16_t crc_check   = 0;
    uint16_t parse_state = MBRTU_OK;


    /* 校验输入参数的有效性 */
    if((NULL == p_recv_buf) || (NULL == ptFrameInfo))
    {
        parse_state = MBRTU_PARA_INVALID;
        return parse_state;
    }

    if(rec_len < 7)
    {
        parse_state = MBRTU_LEN_ERR;
        return parse_state;
    }
    /* 获取从机地址 */
    ptFrameInfo->addr = p_recv_buf[length++];
    if(ptFrameInfo->addr != _gSlaveCommAddr)//_gSlaveCommAddr   COLLECT_SLAVE_ADR
    {
        parse_state = MBRTU_ADR_ERR;
        return parse_state;
    }

    /* 获取功能码 */
    ptFrameInfo->fun = p_recv_buf[length++];

    switch(ptFrameInfo->fun)
    {
        /* 03功能码 */
        case MBRTU_FC_READ_STANDREG:
        case MBRTU_FC_READ_ORREG:
            ptFrameInfo->data_space.start_addr = DataToUnshort(&p_recv_buf[length], USING_MODE_BIG);
            length += 2;
            ptFrameInfo->data_space.reg_len = DataToUnshort(&p_recv_buf[length], USING_MODE_BIG);
            length += 2;
            if(ptFrameInfo->data_space.reg_len > 125)
            {
                parse_state = MBRTU_LEN_ERR;
                return parse_state;
            }
            break;

        /* 06功能码 */
        case MBRTU_FC_SWRITE_STANDREG:
            ptFrameInfo->data_space.start_addr = DataToUnshort(&p_recv_buf[length], USING_MODE_BIG);
            length += 2;
            ptFrameInfo->data_space.reg_len = 1;
            memcpy(ptFrameInfo->data_space.arr_data, &p_recv_buf[length], 2);
            length += 2;
            break;

        case MBRTU_FC_WRITE_STANDREG:
            ptFrameInfo->data_space.start_addr = DataToUnshort(&p_recv_buf[length], USING_MODE_BIG);
            length += 2;
            ptFrameInfo->data_space.reg_len = DataToUnshort(&p_recv_buf[length], USING_MODE_BIG);
            length += 2;
            reg_size = ptFrameInfo->data_space.reg_len * 2;
            if( p_recv_buf[length++] != reg_size)
            {
                parse_state = MBRTU_LEN_ERR;
                return parse_state;
            }
            else
            {
                memcpy(ptFrameInfo->data_space.arr_data, &p_recv_buf[length], reg_size);
                length += reg_size;
            }
            break;

        default:
            parse_state = MBRTU_FC_ERR;
            return parse_state;
    }
    crc_check = CalCrcShort(p_recv_buf, length);
    ptFrameInfo->crc = DataToUnshort(&p_recv_buf[length], USING_MODE_SMALL);
    if(ptFrameInfo->crc != crc_check)
    {
        parse_state = MBRTU_PARA_INVALID;
        return parse_state;
    }
    length += 2;
    *real_len = length;

    return parse_state;
}

/************************************************************************
@ Function Name: MBRTU_PackUp
@ Description  : modbus pack data(BMS been slave)
@ Input    Para: Recive buf, len, frame
@ Output   Para: None
@ Return  Value: Success & Failed
*************************************************************************/

/*
将pst_frame_info 格式转化为  数组格式（方便UART发送给上位机）
*/
uint16_t MBRTU_PackUp(uint8_t *p_send_buf, ModbusRtuFrame_t *pst_frame_info)
{
    uint8_t  get_size      = 0;
    uint16_t crc_check     = 0;
    uint16_t send_frame_len = 0;
    uint16_t pack_state   = MBRTU_OK;

    /* 判定发送缓存和帧的有效性,无效时返回无效参数 */
    if((NULL == p_send_buf) || (NULL == pst_frame_info))
    {
        pack_state = MBRTU_PARA_INVALID;
        return pack_state;
    }
    else
    {
        /* 打包地址和功能码 */
        p_send_buf[send_frame_len++] = pst_frame_info->addr;
        p_send_buf[send_frame_len++] = pst_frame_info->fun;

        switch(pst_frame_info->fun)
        {
            /* 03、04功能码正常应答 */
            case MBRTU_FC_READ_STANDREG:
            case MBRTU_FC_READ_ORREG:
                get_size = pst_frame_info->data_space.reg_len * 2;

                /* 数据字节数占一个字节最大255,但255不是2的倍数 */
                if(get_size >= 255)
                {
                    return MBRTU_PARA_INVALID;
                }
                else
                {
                    /* 打包数据字节数 */
                    p_send_buf[send_frame_len++] = get_size;

                    /* 塞数据进发送Buffer */
                    memcpy(&p_send_buf[send_frame_len], pst_frame_info->data_space.arr_data, get_size);
                    send_frame_len += get_size;
                }
                break;

            /* 06功能码正常应答,12字节 */
            case MBRTU_FC_SWRITE_STANDREG:
                DataToUnchar((p_send_buf + send_frame_len), pst_frame_info->data_space.start_addr, USING_MODE_BIG);
                send_frame_len += 2;
                memcpy((p_send_buf + send_frame_len), pst_frame_info->data_space.arr_data, 2);
                send_frame_len += 2;
                break;

            /* 10功能码正常应答,12字节 */
            case MBRTU_FC_WRITE_STANDREG:
                DataToUnchar( (p_send_buf + send_frame_len), pst_frame_info->data_space.start_addr, USING_MODE_BIG);
                send_frame_len += 2;
                DataToUnchar( (p_send_buf + send_frame_len), pst_frame_info->data_space.reg_len, USING_MODE_BIG);
                send_frame_len += 2;
                break;

            /* 异常应答，9字节 */
            case MBRTU_FC_READ_STANDREG_ERR:
            case MBRTU_FC_READ_ORREG_ERR:
            case MBRTU_FC_SWRITE_STANDREG_ERR:
            case MBRTU_FC_WRITE_STANDREG_ERR:
                p_send_buf[send_frame_len++] = pst_frame_info->data_space.arr_data[0];
                break;

            default:
                break;
        }
        crc_check = CalCrcShort(p_send_buf, send_frame_len);
        DataToUnchar(&p_send_buf[send_frame_len], crc_check, USING_MODE_SMALL);
        send_frame_len += 2;
    }

    return send_frame_len;
}

void SetCommAddr(uint8_t addr)
{
    _gSlaveCommAddr = addr;
}
