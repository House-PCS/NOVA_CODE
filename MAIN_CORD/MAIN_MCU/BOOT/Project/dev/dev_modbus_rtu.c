


/************************************************************************
*                                 Head Files                            *
************************************************************************/
#include "dev_modbus_rtu.h"

/************************************************************************
*                                Macro Definition                       *
************************************************************************/	
#define COLLECT_SLAVE_ADR   0X01
static uint8_t  slave_comm_addr = 0X01;
/************************************************************************
*                         Global Function Realize                       *
************************************************************************/
/************************************************************************
@ Function Name: MBRTU_ParseUp
@ Description  : modbus parse data(BMS been slave)
@ Input    Para: Recive buf, len, frame
@ Output   Para: None
@ Return  Value: Success & Failed

*************************************************************************/
uint16_t MBRTU_ParseUp(uint8_t *pRecvbuf, uint16_t uRecLen, T_ModbusRtuFrame *ptFrameInfo, uint16_t *pRealLen)
{
    uint8_t  uRegSize    = 0;
    uint16_t uLength     = 0;
    uint16_t uCrcCheck   = 0;
    uint16_t uParseState = MBRTU_OK;
            
            
  /* 校验输入参数的有效性 */
    if((NULL == pRecvbuf) || (NULL == ptFrameInfo))
    {
        uParseState = MBRTU_PARA_INVALID;
        return uParseState;
    }

    if(uRecLen < 8)
    {
        uParseState = MBRTU_LEN_ERR;
        return uParseState;
    }

    /* 获取从机地址 */
    ptFrameInfo->uAddr = pRecvbuf[uLength++];
    if(ptFrameInfo->uAddr != slave_comm_addr)
    {
        uParseState = MBRTU_ADR_ERR;
        return uParseState;
    }
    
    /* 获取功能码 */
    ptFrameInfo->uFuc = pRecvbuf[uLength++];

    switch(ptFrameInfo->uFuc)
    {
        /* 03功能码 */
        case MBRTU_FC_READ_STANDREG:
        case MBRTU_FC_READ_ORREG:
            ptFrameInfo->tDataspace.uStartAddr = DataToUnshort(&pRecvbuf[uLength], USING_MODE_BIG);
            uLength += 2;
            ptFrameInfo->tDataspace.uRegLen = DataToUnshort(&pRecvbuf[uLength], USING_MODE_BIG);
            uLength += 2;
            if(ptFrameInfo->tDataspace.uRegLen > 125)
            {
                uParseState = MBRTU_LEN_ERR;
                return uParseState;
            }
            break;
            
        /* 06功能码 */
        case MBRTU_FC_SWRITE_STANDREG:
            ptFrameInfo->tDataspace.uStartAddr = DataToUnshort(&pRecvbuf[uLength], USING_MODE_BIG);
            uLength += 2;
            ptFrameInfo->tDataspace.uRegLen = 1;
            memcpy(ptFrameInfo->tDataspace.uData, &pRecvbuf[uLength], 2);
            uLength += 2;
            break;

        case MBRTU_FC_WRITE_STANDREG:
            ptFrameInfo->tDataspace.uStartAddr = DataToUnshort(&pRecvbuf[uLength], USING_MODE_BIG);
            uLength += 2;
            ptFrameInfo->tDataspace.uRegLen = DataToUnshort(&pRecvbuf[uLength], USING_MODE_BIG);
            uLength += 2;
            uRegSize = ptFrameInfo->tDataspace.uRegLen * 2;
            if( pRecvbuf[uLength++] != uRegSize)
            {
                uParseState = MBRTU_LEN_ERR;
                return uParseState;
            }
            else
            {
                memcpy(ptFrameInfo->tDataspace.uData, &pRecvbuf[uLength], uRegSize);
                uLength += uRegSize;
            }
            break;

        default:
              uParseState = MBRTU_FC_ERR;
              return uParseState;
    }
    uCrcCheck = CalCrcShort(pRecvbuf, uLength);	
    ptFrameInfo->uCrc = DataToUnshort(&pRecvbuf[uLength], USING_MODE_SMALL);
    if(ptFrameInfo->uCrc != uCrcCheck)
    {
        uParseState = MBRTU_PARA_INVALID;
        return uParseState;
    }
    uLength += 2;
    *pRealLen = uLength;
    
    return uParseState;
}

/************************************************************************
@ Function Name: MBRTU_PackUp
@ Description  : modbus pack data(BMS been slave)
@ Input    Para: Recive buf, len, frame
@ Output   Para: None
@ Return  Value: Success & Failed
*************************************************************************/

/*
将ptFrameinfo 格式转化为  数组格式（方便UART发送给上位机）
*/
uint16_t MBRTU_PackUp(uint8_t *pSendbuf, T_ModbusRtuFrame *ptFrameinfo)
{
    uint8_t  RegSize      = 0;
    uint16_t CrcCheck     = 0;
    uint16_t SendFrameLen = 0;
    uint16_t uPackState   = MBRTU_OK;
    
    /* 判定发送缓存和帧的有效性,无效时返回无效参数 */
    if((NULL == pSendbuf) || (NULL == ptFrameinfo))
    {
      uPackState = MBRTU_PARA_INVALID;
      return uPackState;
    }
    else
    {
        /* 打包地址和功能码 */
        pSendbuf[SendFrameLen++] = ptFrameinfo->uAddr;
        pSendbuf[SendFrameLen++] = ptFrameinfo->uFuc;

        switch(ptFrameinfo->uFuc)
        {
            /* 03、04功能码正常应答 */
            case MBRTU_FC_READ_STANDREG:
            case MBRTU_FC_READ_ORREG:
                RegSize = ptFrameinfo->tDataspace.uRegLen * 2;

                /* 数据字节数占一个字节最大255,但255不是2的倍数 */
                if(RegSize >= 255)
                {
                    return MBRTU_PARA_INVALID;
                }
                else
                {					
                    /* 打包数据字节数 */
                    pSendbuf[SendFrameLen++] = RegSize;
                    
                    /* 塞数据进发送Buffer */
                    memcpy(&pSendbuf[SendFrameLen], ptFrameinfo->tDataspace.uData, RegSize);
                    SendFrameLen += RegSize;
                }
                break;
                
            /* 06功能码正常应答,12字节 */
            case MBRTU_FC_SWRITE_STANDREG:
                DataToUnchar((pSendbuf + SendFrameLen), ptFrameinfo->tDataspace.uStartAddr, USING_MODE_BIG);
                SendFrameLen += 2;
                memcpy((pSendbuf + SendFrameLen), ptFrameinfo->tDataspace.uData, 2);
                SendFrameLen += 2;
                break;
                
            /* 10功能码正常应答,12字节 */	
            case MBRTU_FC_WRITE_STANDREG:
                DataToUnchar( (pSendbuf + SendFrameLen), ptFrameinfo->tDataspace.uStartAddr, USING_MODE_BIG);
                SendFrameLen += 2;
                DataToUnchar( (pSendbuf + SendFrameLen), ptFrameinfo->tDataspace.uRegLen, USING_MODE_BIG);
                SendFrameLen += 2;
                break;
            
            /* 异常应答，9字节 */
            case MBRTU_FC_READ_STANDREG_ERR:		
            case MBRTU_FC_READ_ORREG_ERR:	
            case MBRTU_FC_SWRITE_STANDREG_ERR:							
            case MBRTU_FC_WRITE_STANDREG_ERR:		
                pSendbuf[SendFrameLen++] = ptFrameinfo->tDataspace.uData[0];
                break;
                
            default:
                break;
        }
        CrcCheck = CalCrcShort(pSendbuf, SendFrameLen);
        DataToUnchar(&pSendbuf[SendFrameLen], CrcCheck, USING_MODE_SMALL);
        SendFrameLen += 2;
    }

    return SendFrameLen;
}

void SetCommAddr(uint8_t addr)
{
    slave_comm_addr = addr;
}
uint8_t GetCommAddr(void)
{
    return (uint8_t)slave_comm_addr;
}
