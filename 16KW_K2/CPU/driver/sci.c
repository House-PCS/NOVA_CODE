/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sci.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-3-6
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-3-6    <td>V1.0.0    <td>            <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "sci.h"
#include "F28x_Project.h"
#include "stdio.h"
#include "string.h"

#define RS485_DE_GPIO   69
/* 片选口线置低选中  */
#define RS485_RE_ENABLE()     GPIO_WritePin(RS485_DE_GPIO, 0);
/* 片选口线置高不选中 */
#define RS485_DE_ENABLE()     GPIO_WritePin(RS485_DE_GPIO, 1);

#define ISO485_DE_GPIO  44
/* 片选口线置低选中  */
#define ISO485_RE_ENABLE()     GPIO_WritePin(ISO485_DE_GPIO, 0);
/* 片选口线置高不选中 */
#define ISO485_DE_ENABLE()     GPIO_WritePin(ISO485_DE_GPIO, 1);



uint16_t garrReadDataBuff[16];

/**
 * @brief Scid发送单字节
 * @param data 发送字节
 */
void ScidSendByte(uint16_t data)
{
    while (ScidRegs.SCIFFTX.bit.TXFFST != 0) {}
    ScidRegs.SCITXBUF.all = (uint16_t)data;
}

#if PRINTF_ENABLE
int fputc(int ch, FILE *fp)
{
    while (ScidRegs.SCIFFTX.bit.TXFFST != 0); // 等待发送缓冲区为空
    ScidRegs.SCITXBUF.all = ch; // 发送字符
    return ch;
}


int fputs(const char *_ptr, FILE *_fp)
{
    uint16_t i = 0;
    uint16_t len = strlen(_ptr);

    for(i = 0; i < len; i++)
    {
        ScidSendByte(_ptr[i]);
    }
//    SCI_writeCharArray(MySCI485_BASE, (uint16_t *)_ptr, len);
    return len;
}
#endif
/**
 * @brief SCID读buff中的数据
 * @param
 */
void ScidReadMessage(void)
{
    Uint16 num = ScidRegs.SCIFFRX.bit.RXFFST;//当前接收Buff中有多少个数据
    if(num < 16)
    {
        for(int i = 0;(i < num);++i)
        {
            garrReadDataBuff[i] = ScidRegs.SCIRXBUF.all&0xFF;
        }
    }
}
/**
* @brief: DebugSciD_Init
* @param: void
* @return:Nothing
* @note:调试的SCID初始化配置
*/
void DebugSciD_Init(void)
{
    EALLOW;
//    PieVectTable.SCID_RX_INT = &ScidRxIsr;
    ScidRegs.SCICCR.all = 0x0007;
    ScidRegs.SCICTL1.all = 0x0003;
    ScidRegs.SCICTL2.bit.TXINTENA = 1;
    ScidRegs.SCICTL2.bit.RXBKINTENA = 1;
    ScidRegs.SCIHBAUD.all = ((uint16_t)SCID_PRD  & 0xFF00U) >> 8U;
    ScidRegs.SCILBAUD.all = (uint16_t)SCID_PRD  & 0x00FFU;
    ScidRegs.SCICCR.bit.LOOPBKENA = 0;
    ScidRegs.SCIFFTX.all = 0xE040;
    ScidRegs.SCIFFRX.all = 0x0021;
    ScidRegs.SCIFFRX.bit.RXFFIL = 11;
    ScidRegs.SCIFFCT.all = 0x00;

    ScidRegs.SCICTL1.all = 0x0023;
    ScidRegs.SCIFFTX.bit.TXFIFORESET = 1;
    ScidRegs.SCIFFRX.bit.RXFIFORESET = 1;

    /*      SciD的接收中断先不使能    */
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieCtrlRegs.PIEIER8.bit.INTx7 = 1;
    IER |= M_INT8;
    EDIS;
}


/**
* @brief: SciaFifoInit(MCU)
* @param:                             
* @note: 
*
*/
void SciaFifoInit()
{
    // GPIO_SetupPinMux(RS485_DE_GPIO, GPIO_MUX_CPU1, 0);
    // GPIO_SetupPinOptions(RS485_DE_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    //RS485_RE_ENABLE();
    GPIO_SetupPinMux(43, GPIO_MUX_CPU1, 15); //RX
    GPIO_SetupPinOptions(43, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(42, GPIO_MUX_CPU1, 15); //TX
    GPIO_SetupPinOptions(42, GPIO_OUTPUT, GPIO_ASYNC);

    EALLOW;  // This is needed to write to EALLOW protected registers
//    PieVectTable.SCIA_RX_INT = &SciaRxFifoIsr;
//   PieVectTable.SCIA_TX_INT = &SciaTxFifoIsr;
    EDIS;

    SciaRegs.SCICCR.all = 0x0007;      // 1 stop bit, ,No loopback , No parity , 8 char bits, async mode, idle-line protocol
    SciaRegs.SCICTL1.all = 0x0003;     // enable TX, RX, internal SCICLK, Disable RX ERR, SLEEP, TXWAKE
    SciaRegs.SCICTL2.bit.TXINTENA = 1;      // Transmitter buffer and shift registers are both empty
    SciaRegs.SCICTL2.bit.RXBKINTENA = 1;    // Receiver-buffer/break interrupt enable.
    SciaRegs.SCIHBAUD.all = ((uint16_t)SCIA_PRD  & 0xFF00U) >> 8U;
    SciaRegs.SCILBAUD.all = (uint16_t)SCIA_PRD  & 0x00FFU;
    SciaRegs.SCICCR.bit.LOOPBKENA = 0; // Enable loop back
    SciaRegs.SCIFFTX.all = 0xE040;
    SciaRegs.SCIFFRX.all = 0x0021;
    SciaRegs.SCIFFCT.all = 0x00;

    SciaRegs.SCICTL1.all = 0x0023;     // Relinquish SCI from Reset
    SciaRegs.SCIFFTX.bit.TXFIFORESET = 1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;
}



/**
* @brief: ScibFifoInit(RS485 ARC)
* @param:                             
* @note: 
*
*/
void ScibFifoInit()
{
    GPIO_SetupPinMux(RS485_DE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RS485_DE_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    RS485_RE_ENABLE();
    GPIO_SetupPinMux(71, GPIO_MUX_CPU1, 6);
    GPIO_SetupPinOptions(71, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(70, GPIO_MUX_CPU1, 6);
    GPIO_SetupPinOptions(70, GPIO_OUTPUT, GPIO_ASYNC);

    EALLOW;  // This is needed to write to EALLOW protected registers
//    PieVectTable.SCIA_RX_INT = &SciaRxFifoIsr;
//   PieVectTable.SCIA_TX_INT = &SciaTxFifoIsr;
    EDIS;

    ScibRegs.SCICCR.all = 0x0007;      // 1 stop bit,  No loopback
    // No parity,8 char bits,
    // async mode, idle-line protocol
    ScibRegs.SCICTL1.all = 0x0003;     // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    ScibRegs.SCICTL2.bit.TXINTENA = 1;
    ScibRegs.SCICTL2.bit.RXBKINTENA = 1;
    ScibRegs.SCIHBAUD.all = ((uint16_t)SCIB_PRD  & 0xFF00U) >> 8U;
    ScibRegs.SCILBAUD.all = (uint16_t)SCIB_PRD  & 0x00FFU;
    ScibRegs.SCICCR.bit.LOOPBKENA = 0; // Enable loop back
    ScibRegs.SCIFFTX.all = 0xE040;
    ScibRegs.SCIFFRX.all = 0x0021;
    ScibRegs.SCIFFCT.all = 0x00;

    ScibRegs.SCICTL1.all = 0x0023;     // Relinquish SCI from Reset
    ScibRegs.SCIFFTX.bit.TXFIFORESET = 1;
    ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;

}

/**
* @brief: ScicFifoInit(ISO485 Menter)
* @param:                             
* @note: 
*
*/
void ScicFifoInit()
{
    GPIO_SetupPinMux(ISO485_DE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(ISO485_DE_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    ISO485_RE_ENABLE();
    GPIO_SetupPinMux(57, GPIO_MUX_CPU1, 6);
    GPIO_SetupPinOptions(57, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(56, GPIO_MUX_CPU1, 6);
    GPIO_SetupPinOptions(56, GPIO_OUTPUT, GPIO_ASYNC);

    EALLOW;  // This is needed to write to EALLOW protected registers
//    PieVectTable.SCIA_RX_INT = &SciaRxFifoIsr;
//   PieVectTable.SCIA_TX_INT = &SciaTxFifoIsr;
//    EDIS;

    ScicRegs.SCICCR.all = 0x0007;      // 1 stop bit,  No loopback
    // No parity,8 char bits,
    // async mode, idle-line protocol
    ScicRegs.SCICTL1.all = 0x0003;     // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    ScicRegs.SCICTL2.bit.TXINTENA = 1;
    ScicRegs.SCICTL2.bit.RXBKINTENA = 1;
    ScicRegs.SCIHBAUD.all = ((uint16_t)SCIC_PRD  & 0xFF00U) >> 8U;
    ScicRegs.SCILBAUD.all = (uint16_t)SCIC_PRD  & 0x00FFU;
    ScicRegs.SCICCR.bit.LOOPBKENA = 0; // Enable loop back
    ScicRegs.SCIFFTX.all = 0xE040;
    ScicRegs.SCIFFRX.all = 0x0021;
    ScicRegs.SCIFFCT.all = 0x00;

    ScicRegs.SCICTL1.all = 0x0023;     // Relinquish SCI from Reset
    ScicRegs.SCIFFTX.bit.TXFIFORESET = 1;
    ScicRegs.SCIFFRX.bit.RXFIFORESET = 1;

    EDIS;
}


/**
* @brief: Sci_Init
* @param: void
* @return:Nothing
* @note:SCI初始化配置
*/
void Sci_Init(void)
{
    EALLOW;
    DebugSciD_Init();   //调试串口配置
    SciaFifoInit();     //MCU协处理器
    ScibFifoInit();     //ARC485
    ScicFifoInit();     //电表485
    EDIS;
}

/**
* @brief: SciaXmit
* @param: a
* @note:
*
*/
void SciaXmit(int a)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    SciaRegs.SCITXBUF.all = a;
}

/**
* @brief: SciaSend
* @param: data
* @param: len
* @note:
*
*/
void SciaSend(uint8_t* data, uint16_t len)
{
    int i;

    for(i = 0; i < len; i ++)
    {
        SciaXmit(data[i]);
    }

    while((SciaRegs.SCICTL2.bit.TXRDY == 0) || (SciaRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }

}


void SciaSendByte(uint8_t data)
{

    SciaXmit(data);

    while((SciaRegs.SCICTL2.bit.TXRDY == 0) || (SciaRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }

}



/**
* @brief: ScibXmit
* @param: a
* @note:
*
*/
void ScibXmit(int a)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0) {}
    ScibRegs.SCITXBUF.all = a;
}


/**
* @brief: ScibSend
* @param: data
* @param: len
* @note:
*
*/
void ScibSend(uint8_t* data, uint16_t len)
{
    int i;

    RS485_DE_ENABLE();
    for(i = 0; i < len; i ++)
    {
        ScibXmit(data[i]);
    }

    while((ScibRegs.SCICTL2.bit.TXRDY == 0) || (ScibRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }
    RS485_RE_ENABLE();
}


void ScibSendByte(uint8_t data)
{

    RS485_DE_ENABLE();

    ScibXmit(data);

    while((ScibRegs.SCICTL2.bit.TXRDY == 0) || (ScibRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }
    RS485_RE_ENABLE();
}

/**
* @brief: ScicXmit
* @param: a
* @note:
*
*/
void ScicXmit(int a)
{
    while (ScicRegs.SCIFFTX.bit.TXFFST != 0) {}
    ScicRegs.SCITXBUF.all = a;
}


/**
* @brief: ScicSend
* @param: data
* @param: len
* @note:
*
*/
void ScicSend(uint8_t* data, uint16_t len)
{
    int i;

    ISO485_DE_ENABLE();
    for(i = 0; i < len; i ++)
    {
        ScicXmit(data[i]);
    }

    while((ScicRegs.SCICTL2.bit.TXRDY == 0) || (ScicRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }
    ISO485_RE_ENABLE();
}

void ScicSendByte(uint8_t data)
{

    ISO485_DE_ENABLE();

    ScicXmit(data);

    while((ScicRegs.SCICTL2.bit.TXRDY == 0) || (ScicRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }
    ISO485_RE_ENABLE();
}

