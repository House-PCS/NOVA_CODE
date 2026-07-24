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


DevModbus_t gstDevModbusCollector;
DevModbus_t gstDevModbusVoltmeterArc;
DevModbus_t gstDevModbusCoprocessor;
DevModbus_t gstDevModbusPCS;

static uint8_t _garrDevComUartTestData[4] = {0x12, 0x23, 0x34, 0x45};
static uint8_t _garrDevComUartTestErrData[2] = {0xbb, 0x55};
uint16_t gTestModeFlag = 0;

volatile uint8_t gBmsData =0;
SlaveUpdataCallFun    gpfunSlaveUpataFun;
ToolsTestRs485CallFun gpfunToolsTestRs485Fun;
ProgramSnCallBackFun  gpfunProgramSnFun;

void RegisterProgramSnFun(ProgramSnCallBackFun prgram_sn_call_back_fun)
{
    gpfunProgramSnFun = prgram_sn_call_back_fun;
}

void RegisterSlaveUpdataFunPoint(SlaveUpdataCallFun call_back_fun)
{
    gpfunSlaveUpataFun  = call_back_fun;
}

void RegistTestRs485FunPoint(ToolsTestRs485CallFun call_back_fun)
{
    gpfunToolsTestRs485Fun = call_back_fun;
}



/**
  * @brief   设置进入测试模式的标志
  * @param   len：数据长度
  * @param   p_data:数据指针
  * @return  None
  * @note    用于串口通信测试
  */
void SetEnterTestModeFlag(uint16_t test_flag)
{
    gTestModeFlag = test_flag;
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
    ModbusRTURevData_t  st_rev_data;

    if((p_dev->rxbuf[0] == 0x5A) || (p_dev->rxbuf[0] == 0xA5))
    {
        //回调到升级解析代码
        if(gpfunSlaveUpataFun != NULL)
        {
            gpfunSlaveUpataFun(p_dev->rxbuf, p_dev->rev_len);
        }
        return;
    }
    if(strncmp("NOVA", (char *)p_dev->rxbuf, 4) == 0)
    {
        //program sn

        if(gpfunProgramSnFun != NULL)
        {
            gpfunProgramSnFun(p_dev->rxbuf, p_dev->rev_len);
        }
        return;
    }

    if(gTestModeFlag == 1)  //工装模式下 测试串口功能
    {
        if(gpfunToolsTestRs485Fun != NULL)
        {
            if(p_dev->rxbuf[0] == 0x12 && p_dev->rxbuf[1] == 0x23 && p_dev->rxbuf[2] == 0x34 && p_dev->rxbuf[3] == 0x45)
            {
                gpfunToolsTestRs485Fun(p_dev);
            }
        }
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
    st_rev_data.data_size = p_dev->rev_len;
    st_rev_data.p_data = p_dev->rx_queue_buf[p_dev->rev_index];
    if(QueuePut(&p_dev->rev_queue, &st_rev_data) == false)
    {
        return;
    }
    p_dev->rev_index++;
    p_dev->rev_index %= MODBUS_RTU_REV_QUEUE_DEPTH;
}



static void _IrqCollectorRxProcess()
{
    uint16_t rev_size = 0;
    gBmsData = 3;
    /*DMA方式，空闲中断机制*/
    UartDMARxDisable(&ModbusSlaveCollectorUartHandle);
    gstDevModbusCollector.rev_len = UartDMARxDataLength(&ModbusSlaveCollectorUartHandle);
    _RevDataAnalysis(&gstDevModbusCollector);
    UartDMARxEnable(&ModbusSlaveCollectorUartHandle);
}

static void _IrqVoltmeterArcRxProcess()
{
//    uint16_t rev_size = 0;

    /*DMA方式，空闲中断机制*/
    UartDMARxDisable(&ModbusSlaveVoltmeterArcUartHandle);
    gstDevModbusVoltmeterArc.rev_len = UartDMARxDataLength(&ModbusSlaveVoltmeterArcUartHandle);
    _RevDataAnalysis(&gstDevModbusVoltmeterArc);
    UartDMARxEnable(&ModbusSlaveVoltmeterArcUartHandle);
}
static void _IrqCoprocessorRxProcess()
{
//    uint16_t rev_size = 0;

    /*DMA方式，空闲中断机制*/
    UartDMARxDisable(&ModbusHostCoprocessorUartHandle);
    gstDevModbusCoprocessor.rev_len = UartDMARxDataLength(&ModbusHostCoprocessorUartHandle);
    _RevDataAnalysis(&gstDevModbusCoprocessor);
    UartDMARxEnable(&ModbusHostCoprocessorUartHandle);
}

static void _IrqPCSRxProcess()
{
    uint16_t rev_size = 0;
    gBmsData = 2;
    /*DMA方式，空闲中断机制*/
    UartDMARxDisable(&ModbusHostPCSUartHandle);
    gstDevModbusPCS.rev_len = UartDMARxDataLength(&ModbusHostPCSUartHandle);
    _RevDataAnalysis(&gstDevModbusPCS);
    UartDMARxEnable(&ModbusHostPCSUartHandle);
}

/**
  * @brief   DMA中断处理函数
  * @param   None
  * @return  None
  * @note    None
  */
static void _DMACollectorTxCompleteIrqProcess()
{
    gstDevModbusCollector.flag_send_complete = true;
}

static void _DMAVoltmeterTxCompleteIrqProcess()
{
    gstDevModbusVoltmeterArc.flag_send_complete = true;
}

static void _DMACoprocessorTxCompleteIrqProcess()
{
    gstDevModbusCoprocessor.flag_send_complete = true;
}

static void _DMAPCSTxCompleteIrqProcess()
{
    gstDevModbusPCS.flag_send_complete = true;
}


void ModbusHostRead(SlaveRead_t *p_slave_read, void (*send)(uint8_t *, uint16_t ))
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
    p_slave_read->data[len++] = (crc >> 8) & 0xFF;

//    ModbusHostBMSUartHandle.pTxBuf[len] = crc & 0xFF;
//    ModbusHostBMSUartHandle.pTxBuf[len + 1] = (crc >> 8) & 0xFF;
//    ModbusHostBMSUartHandle.tx_size = len + 2;



    if(send != NULL)
    {
        send(p_slave_read->data, len);
    }
}

void ModbusHostWrite(SlaveWrite_t *p_slave_write, void (*send)(uint8_t *, uint16_t ), uint8_t * psend_data) //psend_data 为上层的buf
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

    for(i = 0; i < (p_slave_write->reg_num); i++)
    {
        psend_data[2 * i + len] = (p_slave_write->data[i] >> 8) & 0xFF;
        psend_data[2 * i + (len + 1)] = p_slave_write->data[i] & 0xFF;
    }
    len += data_num;


    crc = CrcModbus(psend_data, len);
    psend_data[len++] = crc & 0xFF;
    psend_data[len++] = (crc >> 8) & 0xFF;

    if(send != NULL)
    {
        send(psend_data, len);
    }
}
uint16_t Modebus_ParseUp(uint8_t *p_recv_buf, uint16_t rec_len, SlaveRead_t *ptFrameInfo, uint8_t *Pdata)
{
    uint16_t uParseState = MBRTU_OK;
    uint16_t uLength = 0;
    uint16_t slave_addr = 0;
    uint8_t  fun_code = 0;
    uint16_t data_num = 0;
    uint8_t  i = 0;
    uint8_t  *pRecvData = 0;
    if((p_recv_buf == NULL) || (ptFrameInfo == NULL))
    {
        uParseState = MBRTU_PARA_INVALID;
        return uParseState;
    }
//此串口数据在接收缓冲中已经进行了校验。此处不需要再进行额外校验。
    if(rec_len < 8)
    {
        uParseState = MBRTU_LEN_ERR;
        return uParseState;
    }

    /* 获取从机地址 */
    slave_addr = p_recv_buf[uLength++];
    /* 获取功能码 */
    fun_code = p_recv_buf[uLength++];

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
            data_num = p_recv_buf[uLength++];
            pRecvData = &p_recv_buf[uLength];
            for(i = 0; i < (data_num >> 1); i++)
            {
                Pdata[2 * i] = pRecvData[2 * i + 1];
                Pdata[2 * i + 1] = pRecvData[2 * i];
            }
            break;
        }
        case MBRTU_FC_SWRITE_STANDREG:
        case MBRTU_FC_WRITE_STANDREG:
        {
//写返回
            ptFrameInfo->slave_addr = slave_addr;
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


void ModbusCollectorRTUSend(uint8_t *p_data, uint16_t len)
{
    uint16_t crc;

    ModbusSlaveCollectorUartHandle.pTxBuf = p_data;
//    crc = CrcModbus(p_data, len);
//    ModbusSlaveCollectorUartHandle.pTxBuf[len] = crc & 0xFF;
//    ModbusSlaveCollectorUartHandle.pTxBuf[len + 1] = (crc >> 8) & 0xFF;
//    ModbusSlaveCollectorUartHandle.tx_size = len + 2;
    ModbusSlaveCollectorUartHandle.tx_size = len;
    if(gstDevModbusCollector.dir_manual == true)
    {
        GpioBitSet(&ModbusSlaveCollectorDEHandle);
        gstDevModbusCollector.flag_send_complete = false;
        gstDevModbusCollector.send_time_out = GetSystick();
        gstDevModbusCollector.send_max_time = 1000 * (8 + 4) * ModbusSlaveCollectorUartHandle.tx_size / ModbusSlaveCollectorUartHandle.uart_cfg->baud_rate + 2;
    }

    gstDevModbusCollector.send(&ModbusSlaveCollectorUartHandle);
}

void ModbusVoltmeterRTUSend(uint8_t *p_data, uint16_t len)
{
    uint16_t crc;

    ModbusSlaveVoltmeterArcUartHandle.pTxBuf = p_data;
//    crc = _CrcModbus(p_data, len);
//    ModbusSlaveCollectorUartHandle.pTxBuf[len] = crc & 0xFF;
//    ModbusSlaveCollectorUartHandle.pTxBuf[len + 1] = (crc >> 8) & 0xFF;
//    ModbusSlaveCollectorUartHandle.tx_size = len + 2;
    ModbusSlaveVoltmeterArcUartHandle.tx_size = len;
    if(gstDevModbusVoltmeterArc.dir_manual == true)
    {
        GpioBitSet(&ModbusSlaveVoltmeterArcDEHandle);
        gstDevModbusVoltmeterArc.flag_send_complete = false;
        gstDevModbusVoltmeterArc.send_time_out = tim_get_systime();
        gstDevModbusVoltmeterArc.send_max_time = 1000 * (8 + 4) * ModbusSlaveVoltmeterArcUartHandle.tx_size /
                                               ModbusSlaveVoltmeterArcUartHandle.uart_cfg->baud_rate + 2;
    }

    gstDevModbusVoltmeterArc.send(&ModbusSlaveVoltmeterArcUartHandle);
}

void ModbusCoprocessorRTUSend(uint8_t *p_data, uint16_t len)
{
    uint16_t crc;

    ModbusHostCoprocessorUartHandle.pTxBuf = p_data;
//    crc = _CrcModbus(p_data, len);
//    ModbusSlaveCollectorUartHandle.pTxBuf[len] = crc & 0xFF;
//    ModbusSlaveCollectorUartHandle.pTxBuf[len + 1] = (crc >> 8) & 0xFF;
//    ModbusSlaveCollectorUartHandle.tx_size = len + 2;
    ModbusHostCoprocessorUartHandle.tx_size = len;
    if(gstDevModbusCoprocessor.dir_manual == true)
    {
//        GpioBitSet(&ModbusSlaveCollectorDEHandle);
        gstDevModbusCoprocessor.flag_send_complete = false;
        gstDevModbusCoprocessor.send_time_out = tim_get_systime();
        gstDevModbusCoprocessor.send_max_time = 1000 * (8 + 4) * ModbusHostCoprocessorUartHandle.tx_size /
                                               ModbusHostCoprocessorUartHandle.uart_cfg->baud_rate + 2;
    }

    gstDevModbusCoprocessor.send(&ModbusHostCoprocessorUartHandle);
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
    if(gstDevModbusPCS.dir_manual == true)
    {
        GpioBitSet(&ModbusHostPCSDEHandle);
        gstDevModbusPCS.flag_send_complete = false;
        gstDevModbusPCS.send_time_out = GetSystick();
        gstDevModbusPCS.send_max_time = 1000 * (8 + 4) * ModbusHostPCSUartHandle.tx_size / ModbusHostPCSUartHandle.uart_cfg->baud_rate + 2;
    }

    gstDevModbusPCS.send(&ModbusHostPCSUartHandle);
}

/**
  * @brief   modbus初始化
  * @param   None
  * @return  None
  * @note    None
  */
void DevModbusRTUInit()
{
    gstDevModbusCollector.mode    = EN_MODBUS_MODE_SLAVE_COLLECTOR;
    gstDevModbusVoltmeterArc.mode = EN_MODBUS_MODE_HOST_VOLTMETER_ARC;
    gstDevModbusCoprocessor.mode  = EN_MODBUS_MODE_HOST_COPROCOESSOR;
    gstDevModbusPCS.mode = EN_MODBUS_MODE_HOST_PCS;

    QueueInit(&gstDevModbusCollector.rev_queue, MODBUS_RTU_REV_QUEUE_DEPTH, sizeof(ModbusRTURevData_t));
    QueueInit(&gstDevModbusVoltmeterArc.rev_queue, MODBUS_RTU_REV_QUEUE_DEPTH, sizeof(ModbusRTURevData_t));
    QueueInit(&gstDevModbusCoprocessor.rev_queue, MODBUS_RTU_REV_QUEUE_DEPTH, sizeof(ModbusRTURevData_t));
    QueueInit(&gstDevModbusPCS.rev_queue, MODBUS_RTU_REV_QUEUE_DEPTH, sizeof(ModbusRTURevData_t));

    gstDevModbusCollector.dir_manual = true;
    gstDevModbusCollector.flag_send_complete = false;
    gstDevModbusCollector.send = UartSend;
    gstDevModbusCollector.rev_index = 0;

    gstDevModbusVoltmeterArc.dir_manual = true;
    gstDevModbusVoltmeterArc.flag_send_complete = false;
    gstDevModbusVoltmeterArc.send = UartSend;
    gstDevModbusVoltmeterArc.rev_index = 0;
    
    gstDevModbusCoprocessor.dir_manual = true;
    gstDevModbusCoprocessor.flag_send_complete = false;
    gstDevModbusCoprocessor.send = UartSend;
    gstDevModbusCoprocessor.rev_index = 0;
    
    gstDevModbusPCS.dir_manual = true;
    gstDevModbusPCS.flag_send_complete = false;
    gstDevModbusPCS.send = UartSend;
    gstDevModbusPCS.rev_index = 0;


    ModbusSlaveCollectorUartHandle.pRxBuf = gstDevModbusCollector.rxbuf;
    ModbusSlaveCollectorUartHandle.rx_size = MODBUS_RTU_RX_SIZE;
    ModbusSlaveCollectorUartHandle.irq_rx_process = _IrqCollectorRxProcess;
    ModbusSlaveCollectorUartHandle.dma_tx_complete_process = _DMACollectorTxCompleteIrqProcess;

    ModbusSlaveVoltmeterArcUartHandle.pRxBuf = gstDevModbusVoltmeterArc.rxbuf;
    ModbusSlaveVoltmeterArcUartHandle.rx_size = MODBUS_RTU_RX_SIZE;
    ModbusSlaveVoltmeterArcUartHandle.irq_rx_process = _IrqVoltmeterArcRxProcess;
    ModbusSlaveVoltmeterArcUartHandle.dma_tx_complete_process = _DMAVoltmeterTxCompleteIrqProcess;
    
    ModbusHostCoprocessorUartHandle.pRxBuf = gstDevModbusCoprocessor.rxbuf;
    ModbusHostCoprocessorUartHandle.rx_size = MODBUS_RTU_RX_SIZE;
    ModbusHostCoprocessorUartHandle.irq_rx_process = _IrqCoprocessorRxProcess;
    ModbusHostCoprocessorUartHandle.dma_tx_complete_process = _DMACoprocessorTxCompleteIrqProcess;   
    
    ModbusHostPCSUartHandle.pRxBuf = gstDevModbusPCS.rxbuf;
    ModbusHostPCSUartHandle.rx_size = MODBUS_RTU_RX_SIZE;
    ModbusHostPCSUartHandle.irq_rx_process = _IrqPCSRxProcess;
    ModbusHostPCSUartHandle.dma_tx_complete_process = _DMAPCSTxCompleteIrqProcess;

    ModbusHostCollectorBspInit();
    ModbusHostVoltmeterArcBspInit();
    ModbusHostCoprocessorBspInit();
    ModbusHostPCSBspInit();
}




/**
  * @brief   大轮询调用
  * @param   None
  * @return  None
  * @note    None
  */
void _DevModbusDEProcess(DevModbus_t *p_dev, Uart_HandleType *p_uart_handle, Gpio_HandleType *p_de_handle)
{
    if(p_dev->flag_send_complete == true)
    {
        if(UartSendCompleteFlag(p_uart_handle) == true)/*串口寄存器已经发送完成，为空*/
        {
            GpioBitReset(p_de_handle);
            p_dev->flag_send_complete = false;
        }
        else
        {
            if((uint32_t)(GetSystick() - p_dev->send_time_out) > p_dev->send_max_time)/*发送异常，恢复接收状态*/
            {
                GpioBitReset(p_de_handle);
            }
        }
    }
    else/*处理超时*/
    {
        if(GpioBitGet(p_de_handle) != 0)
        {
            if((uint32_t)(GetSystick() - p_dev->send_time_out) > p_dev->send_max_time)/*发送超时，恢复接收状态*/
            {
                GpioBitReset(p_de_handle);
            }
        }
    }
}

void DevModbusDirCtlProcess()
{
    _DevModbusDEProcess(&gstDevModbusCollector, &ModbusSlaveCollectorUartHandle, &ModbusSlaveCollectorDEHandle);
    _DevModbusDEProcess(&gstDevModbusVoltmeterArc, &ModbusSlaveVoltmeterArcUartHandle, &ModbusSlaveVoltmeterArcDEHandle);
    _DevModbusDEProcess(&gstDevModbusPCS, &ModbusHostPCSUartHandle, &ModbusHostPCSDEHandle);
}




