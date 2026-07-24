/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
  
#include "prot_uart_asp.h"
#include "mcu_bsp.h"

#include "prot_modbus_debug.h"


PROT_UART_ASP ProtUartAspCfg[RS485_NUM]={
	{PORT_SLAVE,1,0},
    {PORT_SLAVE,1,0},
};
MODBUS_SLAVE_CFG* ProtModbusSlaveAspCfg[RS485_NUM][1]={
// {&ProtModbusSlaveCfg_Debug,},      //485_1
// {&ProtModbusSlaveCfg_Debug,},      //485_2
    {NULL},{NULL}
};
/** 
* @brief  串口测试回复
* @param  port：端口号
* @return  返回待发送数据字节数
* @note:
*
*/
uint8_t UartRxTxTest(uint8_t port){
	uint8_t i = 0;
	for(i = 0; i < Rs485RxTxState[port].ReceiveNum; i++)
    {
		RS485_BspHandle[port].pTxBuf[i] = RS485_BspHandle[port].pRxBuf[i] + 1;
	}
	return Rs485RxTxState[port].ReceiveNum;
}
VofaInfoType VofaInfo ={0};
/** 
* @brief  485接收处理函数
* @param  port：端口号
* @return  none
* @note:
*
*/
void Rs485ReceivHandler(uint8_t port){
	uint8_t i = 0;
	uint16_t SendNum = 0;
	switch((uint8_t)ProtUartAspCfg[port].PortType)
    {
		case PORT_SLAVE:
        {
			if(Rs485RxTxState[port].ReceiveNum > 4)
            {		
				for(i = 0;i < ProtUartAspCfg[port].ProtModbusNum; i++){
					SendNum = ModbusSlaveProtProce(MODBUS_RTU, ProtModbusSlaveAspCfg[port][i], RS485_BspHandle[port].pRxBuf,	\
							Rs485RxTxState[port].ReceiveNum, RS485_BspHandle[port].pTxBuf);
					if(SendNum > 0)
                    {
                        break;
                    }
                }
				/////非标协议处理////
				
				//////////
			}else{//串口测试程序
                //////////VOFA+//////////////////
                if(RS485_BspHandle[port].pRxBuf[0] == 0x76 && RS485_BspHandle[port].pRxBuf[1] == 0x6F &&
                    RS485_BspHandle[port].pRxBuf[2] == 0x66 )
                {
                    if(RS485_BspHandle[port].pRxBuf[3] == 0x61)
                    {
                        VofaInfo.vofa_en = 1;
                        VofaInfo.prot = port;
                    }
                    else if(RS485_BspHandle[port].pRxBuf[3] == 0x62)
                    {
                        VofaInfo.vofa_en = 0;
                    }
                }
                else
                {
                    SendNum=UartRxTxTest(port);
                }
			}
			if(SendNum > 0)
            {
                RS485_SendData(port,SendNum);
			}
			break;
		}
		case PORT_HOST:{
			
			break;
		}
		case PORT_FDX:{
			
			break;
		}
            default :	break;
	}
	Rs485RxTxState[port].ReceiveFlg=false;
	Rs485RxTxState[port].ReceiveNum=0;
	
}            
/** 
* @brief  485协议处理主函数
* @param  none
* @return  none
* @note:
*
*/
void ProtRs485Main(){
	uint8_t i=0;
	
	for(i=0;i<RS485_NUM;i++){
		//Rx
		if(Rs485RxTxState[i].ReceiveFlg){
			Rs485ReceivHandler(i);
		}
	}
}


void VofaSend(uint8_t *buff,uint16_t len)
{
    static uint32_t time  = 0;
    
    if(VofaInfo.vofa_en &&(tim_get_systime() - time >= 500))
    {
        memcpy(RS485_BspHandle[VofaInfo.prot].pTxBuf,buff,len);
        RS485_SendData(VofaInfo.prot,len);
        time = tim_get_systime();
    }
}
