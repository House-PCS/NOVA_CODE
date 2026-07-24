/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_modbus.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-11-27
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-11-27    <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_modbus.h"
#include "modbus_crc.h"
#include "dev_tca9555.h"



DevModbus_t gDevModbusBMS;
DevModbus_t gDevModbusPCS;
DevModbus_t gDevModbusCollector;

SlaveUpdataCallFun    SlaveUpataFun;
void RegisterSlaveUpdataFunPoint(SlaveUpdataCallFun call_back_fun)
{
    SlaveUpataFun  = call_back_fun;
}

/**
  * @brief   串口接收数据解析
  * @param   len：数据长度
  * @param   p_data:数据指针
  * @return  None
  * @note    None
  */
static void _RevDataAnalysis(DevModbus_t *p_dev)
{
    ModbusRTURevData_t  rev_data;

    if((p_dev->rxbuf[0] == 0x5A)||(p_dev->rxbuf[0] == 0xA5))
    {
        //回调到升级解析代码
        if(SlaveUpataFun != NULL)
        {
            SlaveUpataFun(p_dev->rxbuf,p_dev->rev_len);
        }
        
        return;
    }

    if(p_dev->rev_len < 5)/*不完整的Modbus数据*/
    {
        return;
    }
    
    if(ModbusCrcJudge(p_dev->rxbuf, p_dev->rev_len) == false)
    {
        return;
    }
    
    memcpy(p_dev->rx_queue_buf[p_dev->rev_index], p_dev->rxbuf, p_dev->rev_len);
    rev_data.data_size = p_dev->rev_len;
    rev_data.p_data = p_dev->rx_queue_buf[p_dev->rev_index];
    if(QueuePut(&p_dev->rev_queue, &rev_data) == false)
    {
        return;
    }
    p_dev->rev_index++;
    p_dev->rev_index %= MODBUS_RTU_REV_QUEUE_DEPTH;
}

/**
  * @brief   中断处理函数
  * @param   None
  * @return  None
  * @note    None
  */
static void _IrqBMSRxProcess()
{
    uint16_t rev_size = 0;
    
	/*DMA方式，空闲中断机制*/
	UartDMARxDisable(&ModbusHostBMSUartHandle);
	gDevModbusBMS.rev_len = UartDMARxDataLength(&ModbusHostBMSUartHandle);	
//在此处判断是否是升级反馈
    _RevDataAnalysis(&gDevModbusBMS);      
	UartDMARxEnable(&ModbusHostBMSUartHandle);
}
static void _IrqPCSRxProcess()
{
    uint16_t rev_size = 0;
    
	/*DMA方式，空闲中断机制*/
	UartDMARxDisable(&ModbusHostPCSUartHandle);
	gDevModbusPCS.rev_len = UartDMARxDataLength(&ModbusHostPCSUartHandle);	
    _RevDataAnalysis(&gDevModbusPCS);      
	UartDMARxEnable(&ModbusHostPCSUartHandle);
}
static void _IrqCollectorRxProcess()
{
    uint16_t rev_size = 0;
    
	/*DMA方式，空闲中断机制*/
	UartDMARxDisable(&ModbusSlaveCollectorUartHandle);
	gDevModbusCollector.rev_len = UartDMARxDataLength(&ModbusSlaveCollectorUartHandle);	
    _RevDataAnalysis(&gDevModbusCollector);      
	UartDMARxEnable(&ModbusSlaveCollectorUartHandle);
}


/**
  * @brief   DMA中断处理函数
  * @param   None
  * @return  None
  * @note    None
  */
static void _DMABMSTxCompleteIrqProcess()
{
    gDevModbusBMS.flag_send_complete = true;
    
}
static void _DMAPCSTxCompleteIrqProcess()
{
    gDevModbusPCS.flag_send_complete = true;
    
}
static void _DMACollectorTxCompleteIrqProcess()
{
    gDevModbusCollector.flag_send_complete = true;
    
}




void ModbusHostRead(SlaveRead_t *p_slave_read,void (*send)(uint8_t *,uint16_t ))
{
    uint16_t len = 0;
    uint16_t crc;
    p_slave_read->data[len++] = p_slave_read->slave_addr;
    p_slave_read->data[len++] = p_slave_read ->func_code;
    p_slave_read->data[len++] = (p_slave_read->reg_addr >> 8) & 0xFF;
    p_slave_read->data[len++] = (p_slave_read->reg_addr) & 0xFF;
    p_slave_read->data[len++] = (p_slave_read->reg_num >> 8) & 0xFF;
    p_slave_read->data[len++] = (p_slave_read->reg_num) & 0xFF;

    crc = CrcModbus(p_slave_read->data, len);
    p_slave_read->data[len++] = crc & 0xFF;
    p_slave_read->data[len++] =(crc >> 8) & 0xFF;

//    ModbusHostBMSUartHandle.pTxBuf[len] = crc & 0xFF;
//    ModbusHostBMSUartHandle.pTxBuf[len + 1] = (crc >> 8) & 0xFF;
//    ModbusHostBMSUartHandle.tx_size = len + 2;



    if(send != NULL)
    {
        send(p_slave_read->data,len);
    }    
}

void ModbusHostWrite(SlaveWrite_t *p_slave_write,void (*send)(uint8_t *,uint16_t ),uint8_t * psend_data)  //psend_data 为上层的buf
{
    uint16_t len = 0;
    uint8_t  i = 0;
    uint8_t  data_num = 0;
    uint16_t crc;
    psend_data[len++] = p_slave_write->slave_addr;
    psend_data[len++] = p_slave_write ->func_code;
    psend_data[len++] = (p_slave_write->reg_addr >> 8) & 0xFF;
    psend_data[len++] = (p_slave_write->reg_addr) & 0xFF;
    psend_data[len++] = (p_slave_write->reg_num >> 8) & 0xFF;
    psend_data[len++] = (p_slave_write->reg_num) & 0xFF;
    data_num          = (p_slave_write->reg_num << 1);
    psend_data[len++] = data_num;

    for(i=0;i<(p_slave_write->reg_num);i++)
    {
        psend_data[2 * i + len] = (p_slave_write->data[i] >> 8) & 0xFF;
        psend_data[2 * i + (len+1)] = p_slave_write->data[i] & 0xFF;        
    }
    len += data_num;


    crc = CrcModbus(psend_data, len);
    psend_data[len++] = crc & 0xFF;
    psend_data[len++] =(crc >> 8) & 0xFF;

    if(send != NULL)
    {
        send(psend_data,len);
    }    
}

uint16_t Modebus_ParseUp(uint8_t *pRecvbuf, uint16_t uRecLen, SlaveRead_t *ptFrameInfo, uint8_t *Pdata)
{
    uint16_t uParseState = MBRTU_OK;
    uint16_t uLength = 0;
    uint16_t slave_addr =0;
    uint8_t  fun_code = 0;
    uint16_t data_num =0;
    uint8_t  i =0;
    uint8_t  *pRecvData= 0;
    if((pRecvbuf == NULL) ||(ptFrameInfo ==NULL))
    {
        uParseState = MBRTU_PARA_INVALID;
        return uParseState;   
    }
//此串口数据在接收缓冲中已经进行了校验。此处不需要再进行额外校验。
    if(uRecLen < 8)
    {
        uParseState = MBRTU_LEN_ERR;
        return uParseState;
    }
    
    /* 获取从机地址 */
    slave_addr = pRecvbuf[uLength++];
    /* 获取功能码 */
    fun_code = pRecvbuf[uLength++];
    switch(fun_code)
    {
        case MBRTU_FC_READ_STANDREG:
        case MBRTU_FC_READ_ORREG:
        {
//读返回
            if(ptFrameInfo->slave_addr != slave_addr)
            {
                  uParseState = MBRTU_ADR_ERR;
                  break;
            }
            data_num = pRecvbuf[uLength++];
            pRecvData = &pRecvbuf[uLength];
            for(i = 0; i < (data_num >> 1); i++)
            {
                Pdata[2 * i] = pRecvData[2 * i + 1];
                Pdata[2 * i + 1] =pRecvData[2 * i];
            } 
            break;
        }
        case MBRTU_FC_SWRITE_STANDREG:
        case MBRTU_FC_WRITE_STANDREG:
        {    
//写返回
            break;
        }
        case MBRTU_FC_READ_STANDREG_ERR:
        case MBRTU_FC_READ_ORREG_ERR:
        case MBRTU_FC_SWRITE_STANDREG_ERR:
        case MBRTU_FC_WRITE_STANDREG_ERR:
        {
//错误
            uParseState = MBRTU_PARA_INVALID;
            break;
        }
        default:
        break;
    }
    
    return uParseState;
}








/**
  * @brief   Modbus发送函数
  * @param   p_data:数据指针
  * @param   len:数据长度
  * @return  None
  * @note    None
  */
void ModbusBMSRTUSend(uint8_t *p_data, uint16_t len)
{
    uint16_t crc;
    
    ModbusHostBMSUartHandle.pTxBuf = p_data;
//    crc = CrcModbus(p_data, len);
//    ModbusHostBMSUartHandle.pTxBuf[len] = crc & 0xFF;
//    ModbusHostBMSUartHandle.pTxBuf[len + 1] = (crc >> 8) & 0xFF;
//    ModbusHostBMSUartHandle.tx_size = len + 2;
    ModbusHostBMSUartHandle.tx_size = len;
    if(gDevModbusBMS.dir_manual == true)
    {
//        GpioBitSet(&ModbusHostBMSDEHandle);
        DevTca9555SetPort0SinglePinStatus((EN_TCA9555_CH)2, 1);//GpioBitSet(&ModbusHostPCSDEHandle);
        gDevModbusBMS.flag_send_complete = false;
        gDevModbusBMS.send_time_out = GetSystick();
        gDevModbusBMS.send_max_time = 1000 * (8 + 4) * ModbusHostBMSUartHandle.tx_size / ModbusHostBMSUartHandle.uart_cfg->baud_rate + 2;
    }
    
    gDevModbusBMS.send(&ModbusHostBMSUartHandle);
}

void ModbusPCSRTUSend(uint8_t *p_data, uint16_t len)
{
    uint16_t crc;
    
    ModbusHostPCSUartHandle.pTxBuf = p_data;
//    crc = CrcModbus(p_data, len);
//    ModbusHostPCSUartHandle.pTxBuf[len] = crc & 0xFF;
//    ModbusHostPCSUartHandle.pTxBuf[len + 1] = (crc >> 8) & 0xFF;
//    ModbusHostPCSUartHandle.tx_size = len + 2;
    ModbusHostPCSUartHandle.tx_size = len;
    if(gDevModbusPCS.dir_manual == true)
    {
        DevTca9555SetPort0SinglePinStatus((EN_TCA9555_CH)3, 1);//GpioBitSet(&ModbusHostPCSDEHandle);
        gDevModbusPCS.flag_send_complete = false;
        gDevModbusPCS.send_time_out = GetSystick();
        gDevModbusPCS.send_max_time = 1000 * (8 + 4) * ModbusHostPCSUartHandle.tx_size / ModbusHostPCSUartHandle.uart_cfg->baud_rate + 2;
    }
    
    gDevModbusPCS.send(&ModbusHostPCSUartHandle);
}
void ModbusCollectorRTUSend(uint8_t *p_data, uint16_t len)
{
    uint16_t crc;
    
    ModbusSlaveCollectorUartHandle.pTxBuf = p_data;
//    crc = CrcModbus(p_data, len);
//    ModbusSlaveCollectorUartHandle.pTxBuf[len] = crc & 0xFF;
//    ModbusSlaveCollectorUartHandle.pTxBuf[len + 1] = (crc >> 8) & 0xFF;
//    ModbusSlaveCollectorUartHandle.tx_size = len + 2;
    ModbusSlaveCollectorUartHandle.tx_size = len;    
    if(gDevModbusCollector.dir_manual == true)
    {
//        GpioBitSet(&ModbusSlaveCollectorDEHandle);
        DevTca9555SetPort0SinglePinStatus((EN_TCA9555_CH)4, 1);//GpioBitSet(&ModbusHostPCSDEHandle);
        gDevModbusCollector.flag_send_complete = false;
        gDevModbusCollector.send_time_out = GetSystick();
        gDevModbusCollector.send_max_time = 1000 * (8 + 4) * ModbusSlaveCollectorUartHandle.tx_size / ModbusSlaveCollectorUartHandle.uart_cfg->baud_rate + 2;
    }
    
    gDevModbusCollector.send(&ModbusSlaveCollectorUartHandle);
}








/**
  * @brief   modbus初始化
  * @param   None
  * @return  None
  * @note    None
  */
void DevModbusRTUInit()
{
    gDevModbusBMS.mode = EN_MODBUS_MODE_HOST_BMS;
    gDevModbusPCS.mode = EN_MODBUS_MODE_HOST_PCS;
    gDevModbusCollector.mode = EN_MODBUS_MODE_SLAVE_COLLECTOR;
    
    QueueInit(&gDevModbusBMS.rev_queue, MODBUS_RTU_REV_QUEUE_DEPTH, sizeof(ModbusRTURevData_t));
    QueueInit(&gDevModbusPCS.rev_queue, MODBUS_RTU_REV_QUEUE_DEPTH, sizeof(ModbusRTURevData_t));
    QueueInit(&gDevModbusCollector.rev_queue, MODBUS_RTU_REV_QUEUE_DEPTH, sizeof(ModbusRTURevData_t));
    
    gDevModbusBMS.dir_manual = true;
    gDevModbusBMS.flag_send_complete = false;
    gDevModbusBMS.send = UartSend;
    gDevModbusBMS.rev_index = 0;
    
    gDevModbusPCS.dir_manual = true;
    gDevModbusPCS.flag_send_complete = false;
    gDevModbusPCS.send = UartSend;
    gDevModbusPCS.rev_index = 0;
    
    gDevModbusCollector.dir_manual = true;
    gDevModbusCollector.flag_send_complete = false;
    gDevModbusCollector.send = UartSend;
    gDevModbusCollector.rev_index = 0;
    
    ModbusHostBMSUartHandle.pRxBuf = gDevModbusBMS.rxbuf;
    ModbusHostBMSUartHandle.rx_size = MODBUS_RTU_RX_SIZE;
    ModbusHostBMSUartHandle.irq_rx_process = _IrqBMSRxProcess;
    ModbusHostBMSUartHandle.dma_tx_complete_process = _DMABMSTxCompleteIrqProcess;
    
    ModbusHostPCSUartHandle.pRxBuf = gDevModbusPCS.rxbuf;
    ModbusHostPCSUartHandle.rx_size = MODBUS_RTU_RX_SIZE;
    ModbusHostPCSUartHandle.irq_rx_process = _IrqPCSRxProcess;
    ModbusHostPCSUartHandle.dma_tx_complete_process = _DMAPCSTxCompleteIrqProcess;
    
    ModbusSlaveCollectorUartHandle.pRxBuf = gDevModbusCollector.rxbuf;
    ModbusSlaveCollectorUartHandle.rx_size = MODBUS_RTU_RX_SIZE;
    ModbusSlaveCollectorUartHandle.irq_rx_process = _IrqCollectorRxProcess;
    ModbusSlaveCollectorUartHandle.dma_tx_complete_process = _DMACollectorTxCompleteIrqProcess;
    
//    ModbusHostBMSBspInit();
//    ModbusHostPCSBspInit();
    ModbusHostCollectorBspInit();
}




/**
  * @brief   大轮询调用
  * @param   None
  * @return  None
  * @note    None
  */
void _DevModbusDEProcess(DevModbus_t *p_dev, Uart_HandleType *p_uart_handle,uint8_t de_ch)
{
    if(p_dev->flag_send_complete == true)
    {
        if(UartSendCompleteFlag(p_uart_handle) == true)/*串口寄存器已经发送完成，为空*/
        {
            DevTca9555SetPort0SinglePinStatus((EN_TCA9555_CH)de_ch, 0);  //GpioBitReset(p_de_handle);
            p_dev->flag_send_complete = false;
        }
        else
        {
            if((uint32_t)(GetSystick() - p_dev->send_time_out) > p_dev->send_max_time)/*发送异常，恢复接收状态*/
            {
                DevTca9555SetPort0SinglePinStatus((EN_TCA9555_CH)de_ch, 0);//GpioBitReset(p_de_handle);
            }
        }
    }
    else/*处理超时*/
    {
//        if(GpioBitGet(p_de_handle) != 0)
        if(DevTca9555GetPort0SinglePinStatus(de_ch) != 0)
        {
            if((uint32_t)(GetSystick() - p_dev->send_time_out) > p_dev->send_max_time)/*发送超时，恢复接收状态*/
            {
                DevTca9555SetPort0SinglePinStatus((EN_TCA9555_CH)de_ch, 0);//GpioBitReset(p_de_handle);
            }
        }
    }
}

void DevModbusDirCtlProcess()
{

    _DevModbusDEProcess(&gDevModbusBMS, &ModbusHostBMSUartHandle, 2);
    _DevModbusDEProcess(&gDevModbusPCS, &ModbusHostPCSUartHandle, 3);
    _DevModbusDEProcess(&gDevModbusCollector, &ModbusSlaveCollectorUartHandle, 4);
}




