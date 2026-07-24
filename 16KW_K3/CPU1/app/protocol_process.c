/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        protocol_process.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-3-14
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-3-14    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#include "protocol_process.h"
#include "sys_info.h"
#include "flash_default.h"

void ProtocolInit(void)
{
    MidComserviceInit();
    PrivateParamInit(); //私有协议参数初始化
    FirmwareParamInit();
    ModbusRegInit();
    NetworkInit();      //以太网初始化配置,放到最后一步骤初始化，正常500ms闪烁link成功，闪烁2次link失败
    UploadInfoInit();//通信寄存器上传中间变量初始化
//    Init_System_Parameter();

}
void ProtocolProcess(void)
{
    static EN_PROTOCOL_STATUS e_protocol_state = EN_PROTOCOL_IDLE;
    static SciRevData_t *p_com_cmd;   //SciRevData_t
    static uint32_t  timeout = 0;

    EthernetProcess();
    MidComserviceHandle();
    ModbusProcess();

    SystemFirmwareUpdate();

    switch(e_protocol_state)
    {
        case EN_PROTOCOL_IDLE:
        {
            if(QueueGet(&gstSciRevQueue, (void *)&p_com_cmd) == true)
            {
                if(p_com_cmd->data_type == TYPE_MODBUS_HOST_DATA)   //本机作为主机解析的MODBUS HOST寄存器数据
                {
                    e_protocol_state = EN_PROTOCOL_MODBUS_HOST_ANALYSIS;
                }
                else if(p_com_cmd->data_type == TYPE_MODBUS_SLAVE_DATA) //本机作为从机解析的MODBUS SLAVE寄存器数据
                {
                    e_protocol_state = EN_PROTOCOL_MODBUS_SLAVE_ANALYSIS;
                    timeout = TimGetSystime();
                }
                else if(p_com_cmd->data_type == TYPE_COM_RRESPONSE_DATA)    //私有协议读响应包
                {
                    e_protocol_state = EN_PROTOCOL_COM_RRESPONSE_ANALYSIS;
                }
                else if(p_com_cmd->data_type == TYPE_COM_WRESPONSE_DATA)    //私有协议写响应包
                {
                    e_protocol_state = EN_PROTOCOL_COM_WRESPONSE_ANALYSIS;
                }
                else if(p_com_cmd->data_type == TYPE_COM_RREQUEST_DATA)     //私有协议读请求包
                {
                    e_protocol_state = EN_PROTOCOL_COM_RREQUEST_ANALYSIS;
                }
                else if(p_com_cmd->data_type == TYPE_COM_WREQUEST_DATA)     //私有协议写请求包
                {
                    e_protocol_state = EN_PROTOCOL_COM_WREQUEST_ANALYSIS;
                }
            }
            break;
        }
        case EN_PROTOCOL_MODBUS_HOST_ANALYSIS:
        {
            ModbusHostAnalysis(p_com_cmd);
            e_protocol_state = EN_PROTOCOL_IDLE;
            break;
        }
        case EN_PROTOCOL_MODBUS_SLAVE_ANALYSIS:
        {
            if(ModbusCmdAnalysis(p_com_cmd) == 1)
            {
                e_protocol_state = EN_PROTOCOL_IDLE;
            }

            /*超时*/
            if(TimDifftimeFromNow(timeout) > 300)
            {
                e_protocol_state = EN_PROTOCOL_IDLE;
            }
            break;
        }
        case EN_PROTOCOL_COM_RRESPONSE_ANALYSIS:
        {
            break;
        }
        case EN_PROTOCOL_COM_WRESPONSE_ANALYSIS:
        {
            WriteResponseAnalysis(p_com_cmd);
            e_protocol_state = EN_PROTOCOL_IDLE;
            break;
        }
        case EN_PROTOCOL_COM_RREQUEST_ANALYSIS:
        {
            break;
        }
        case EN_PROTOCOL_COM_WREQUEST_ANALYSIS:
        {
            WriteRequestAnalysis(p_com_cmd);
            e_protocol_state = EN_PROTOCOL_IDLE;
            break;
        }
        default:
            break;
    }
}
