/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can.c
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
  * <tr><td>2022-11-10    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "dev_ht7017.h"
#include "bsp.h"

//static void Uart3IrqProcess();
uint8_t gHt7017UartRxBuff[10]={0};
void DevHt7010Init()
{
	DO_SET(B3,1); 	//复位拉高
	Ht7017UartHandle.pRxBuf = gHt7017UartRxBuff;
	Ht7017UartHandle.rx_size = 10;
//	Ht7017UartHandle.irq_rx_process = Uart3IrqProcess;
	Ht7017Uart_Init();
}
//static void Uart3IrqProcess()
//{
//    if(1)
//    { 
//        Ht7017RxTxState.ReceiveFlg=1;
//	}
//    if(1)   //接收缓冲区非空
//    {
//        Ht7017RxTxState.ReceiveNum++;
////        Ht7017UartHandle.pRxBuf++;
//    }
//}  

//void Ht7017UartSend(uint8_t *buff,uint16_t len)
//{    
//////    if(Ht7017RxTxState.Sending == UART_IDLE)
////    {
//        Ht7017UartHandle.pTxBuf = buff;
//        Ht7017UartHandle.tx_size = len;
//        Ht7017RxTxState.Sending = UART_SEND_ING;
//        UartNormalSend(&Ht7017UartHandle);
////    }
//}

unsigned char HT_Read(unsigned char add,unsigned char *pdata)
{
    unsigned char cnt = 0;
    
    pdata[cnt++] = HT_HEAD;
    pdata[cnt++] = add | (HT_RO << 7);
    
    return cnt;
}
unsigned char HT_Write(unsigned char add,unsigned short data,unsigned char *pdata)
{
    unsigned char cnt = 0;
    unsigned char sum = 0;
    
    pdata[cnt++] = HT_HEAD;
    pdata[cnt++] = add | (HT_WR << 7);
    pdata[cnt++] = data>>8;
    pdata[cnt++] = data;
	
	sum = pdata[0] + pdata[1] + pdata[2] + pdata[3] ;
    sum = ~sum;
    pdata[cnt++] = sum;
    return cnt;
}
