//------------------------------------------------------------------------------
/*File Name         :   Uart.c
**Original Author   :   lvyb
**Creation Date     :   
**Description       :   TIMER固件库函数

*/
//------------------------------------------------------------------------------
#include "Uart.h"

//------------------------------------------------------------------------------
/*Function Name     :   Uart0Init 
**Description       :   Uart初始化
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

void UartInit()
{
    UART0->BAUDDIV = XTAL / UART_BUAD;
    UART0->CTRL    = 0x0B;
    NVIC_EnableIRQ(INT01_IRQn);
}

//------------------------------------------------------------------------------
/*Function Name     :   INT01_Handler 
**Description       :   Uart0中断函数
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

void INT01_Handler()
{    
    char Data;
    UART0->INTCLEAR = 0x02;
    Data = UART0->DATA;
    UartSendByte(Data);
}

//------------------------------------------------------------------------------
/*Function Name     :   UartSendByte 
**Description       :   Uart0发送字节函数
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

void UartSendByte(char Data)
{
    while ((UART0->STATE & 1));
    UART0->DATA = Data;
}

//------------------------------------------------------------------------------
/*Function Name     :   fputc 
**Description       :   Printf发送函数底层函数
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------
#if 1
    #pragma import(__use_no_semihosting)             
                    
    struct __FILE 
    { 
        int handle; 
     
    }; 
     
    FILE __stdout;       
    //定义_sys_exit()以避免使用半主机模式    
    void _sys_exit(int x) 
    { 
        x = x; 
    }

    int fputc(int ch, FILE *stream)
    {
        /* 堵塞判断串口是否发送完成 */
        while((UART0->STATE & 1));
        /* 串口发送完成，将该字符发送 */
        UART0->DATA = (uint8_t) ch;
        return ch;
    }
#endif
//------------------------------------------------------------------------------
/*Function Name     :   UartTxTest 
**Description       :   Uart发送测试
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

int UartTxTest()
{
    int i = 0;
    int TxDataCnt = 0;
    for(i = 0; i < 20; i++)
    {
        printf("%d\t", TxDataCnt);
        TxDataCnt += 1;
        Delay(400);
    }
    
    if(TxDataCnt == 20)
    {   
        printf("\n");
    }

    return TxDataCnt;

}


