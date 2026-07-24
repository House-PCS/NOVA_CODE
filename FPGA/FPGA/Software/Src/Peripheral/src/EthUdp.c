//------------------------------------------------------------------------------
/*File Name         :   EthUdp.c
**Original Author   :   lvyb
**Creation Date     :   
**Description       :   TIMER固件库函数

*/
//------------------------------------------------------------------------------
#include "EthUdp.h"

//------------------------------------------------------------------------------
/*Function Name     :   EthUdpInit 
**Description       :   EthUdp初始化
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------
void EthUdpInit()
{
    UDP_CTRL->Ready = 0x00;
    UDP_CTRL->EthIrqClr = 0x01; 
    NVIC_EnableIRQ(INT02_IRQn);
}

//------------------------------------------------------------------------------
/*Function Name     :   INT02_Handler 
**Description       :   EthUdp接收数据中断函数
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------
void INT02_Handler()
{
    EthUdpDataTransfer();
    UDP_CTRL->Length = 0x08;
    UDP_CTRL->Ready = 0x01;
    UDP_CTRL->Ready = 0x00;
    UDP_CTRL->EthIrqClr = 0x01;
}

//------------------------------------------------------------------------------
/*Function Name     :   INT02_Handler 
**Description       :   EthUdp接收数据中断函数
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------
void EthUdpDataTransfer()
{
    uint32_t UdpDataTemp;

    UdpDataTemp = UDP_CACHE_RX->DATA0;
    UDP_CACHE_TX->DATA0 = UdpDataTemp;

    UdpDataTemp = UDP_CACHE_RX->DATA1;
    UDP_CACHE_TX->DATA1 = UdpDataTemp;
}



