/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sci.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


//
// Included Files
//
#include "F28x_Project.h"
#include "sci.h"
#include "mid_comservice.h"
//
// Defines
//
#define SCI_RX_ISR_EN   1
#define CPU_FREQ        200E6
#define LSPCLK_FREQ     CPU_FREQ/4
#define SCI_FREQ        (uint32_t)115200
#define SCI_PRD         ((LSPCLK_FREQ/(SCI_FREQ*8))-1)

#define SCID_FREQ        1250000
#define SCID_PRD         ((LSPCLK_FREQ/(SCID_FREQ*8))-1)

#define RS485_DE_GPIO    47
/* 片选口线置低选中  */
#define RE_ENABLE()     GPIO_WritePin(RS485_DE_GPIO, 0);
/* 片选口线置高不选中 */
#define DE_ENABLE()     GPIO_WritePin(RS485_DE_GPIO, 1);
//
// Globals
//
Uint16 sdataA[2];    // Send data for SCI-A
Uint16 rdataA[2];    // Received data for SCI-A
Uint16 rdata_pointA; // Used for checking the received data
uint8_t RxData[512];
static volatile uint16_t _gSciaRecvFlag = 0; // 20260429，优化代码

typedef struct _usart_stru_t
{
    uint8_t     status;
    uint8_t*    pdata;
    uint16_t    index;
} UartFrame_t;

UartFrame_t KeyBoardRxFrame;
//
// Function Prototypes
//
interrupt void SciaTxFifoIsr(void);
interrupt void SciaRxFifoIsr(void);
void SciaFifoInit(void);
void error(void);
void SciaSend(uint8_t* data, uint16_t len);
void SciaMsg(char * msg);
//
// Main
//

#if 0
void main(void)
{
    char *msg;

//
// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the F2837xD_SysCtrl.c file.
//
    InitSysCtrl();

//
// Step 2. Initialize GPIO:
// This example function is found in the F2837xD_Gpio.c file and
// illustrates how to set the GPIO to it's default state.
//
    InitGpio();

//
// For this example, only init the pins for the SCI-A port.
//  GPIO_SetupPinMux() - Sets the GPxMUX1/2 and GPyMUX1/2 register bits
//  GPIO_SetupPinOptions() - Sets the direction and configuration of the GPIOS
// These functions are found in the F2837xD_Gpio.c file.
//
#ifdef _LAUNCHXL_F2837xD
    GPIO_SetupPinMux(85, GPIO_MUX_CPU1, 5);
    GPIO_SetupPinOptions(85, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(84, GPIO_MUX_CPU1, 5);
    GPIO_SetupPinOptions(84, GPIO_OUTPUT, GPIO_ASYNC);
#else
    GPIO_SetupPinMux(RS485_DE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RS485_DE_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    RE_ENABLE();
    GPIO_SetupPinMux(43, GPIO_MUX_CPU1, 15);
    GPIO_SetupPinOptions(43, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(42, GPIO_MUX_CPU1, 15);
    GPIO_SetupPinOptions(42, GPIO_OUTPUT, GPIO_ASYNC);
#endif

//
// Step 3. Clear all interrupts and initialize PIE vector table:
// Disable CPU interrupts
//
    DINT;

//
// Initialize PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the F2837xD_PieCtrl.c file.
//
    InitPieCtrl();

//
// Disable CPU interrupts and clear all CPU interrupt flags:
//
    IER = 0x0000;
    IFR = 0x0000;

//
// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in F2837xD_DefaultIsr.c.
// This function is found in F2837xD_PieVect.c.
//
    InitPieVectTable();

//
// Interrupts that are used in this example are re-mapped to
// ISR functions found within this file.
//
    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.SCIA_RX_INT = &SciaRxFifoIsr;
//   PieVectTable.SCIA_TX_INT = &SciaTxFifoIsr;
    EDIS;    // This is needed to disable write to EALLOW protected registers

//
// Step 4. Initialize the Device Peripherals:
//
    SciaFifoInit();  // Init SCI-A
    KeyBoardRxFrame.pdata = RxData;
//
// Step 5. User specific code, enable interrupts:
//
// Init send data.  After each transmission this data
// will be updated for the next transmission
//
    uint8_t data[256];
    uint16_t j = 0;
    for(j = 0; j < 256; j ++)
    {
        data[j] = j;
    }
    SciaSend(data, 256);

    msg = "0123456789\0";
    SciaMsg(msg);

    msg = "\r\nYou will enter a character, and the DSP will echo it back! \n\0";
    SciaMsg(msg);
//   rdata_pointA = sdataA[0];

//
// Enable interrupts required for this example
//
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1;   // PIE Group 9, INT1
//   PieCtrlRegs.PIEIER9.bit.INTx2 = 1;   // PIE Group 9, INT2
    IER = 0x100;                         // Enable CPU INT
    EINT;

//
// Step 6. IDLE loop. Just sit and loop forever (optional):
//
    for(;;);
}
#endif
//
// error - Function to halt debugger on error
//
//void error(void)
//{
//    asm("     ESTOP0"); // Test failed!! Stop!
//    for (;;);
//}

//
// SciaTxFifoIsr - SCIA Transmit FIFO ISR
//

/**
* @brief: SciaTxFifoIsr
* @param: void
* @return:
* @note:
*
*/
interrupt void SciaTxFifoIsr(void)
{
    Uint16 i;

    for(i=0; i< 2; i++)
    {
        SciaRegs.SCITXBUF.all=sdataA[i];  // Send data
    }

    for(i=0; i< 2; i++)                  // Increment send data for next cycle
    {
        sdataA[i] = (sdataA[i]+1) & 0x00FF;
    }

    SciaRegs.SCIFFTX.bit.TXFFINTCLR=1;   // Clear SCI Interrupt flag
    PieCtrlRegs.PIEACK.all|=0x100;       // Issue PIE ACK
}

//
// SciaRxFifoIsr - SCIA Receive FIFO ISR
//
//interrupt void SciaRxFifoIsr(void)
//{
////    Uint16 i;
////
////    for(i=0;i<2;i++)
////    {
////       rdataA[i]=SciaRegs.SCIRXBUF.all;  // Read data
////    }
////
////    for(i=0;i<2;i++)                     // Check received data
////    {
////       if(rdataA[i] != ( (rdata_pointA+i) & 0x00FF) )
////       {
////           error();
////       }
////    }
//
////    rdata_pointA = (rdata_pointA+1) & 0x00FF;
//
//  rdataA[0] = SciaRegs.SCIRXBUF.all;
//    SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;   // Clear Overflow flag
//    SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag
//
//    PieCtrlRegs.PIEACK.all|=0x100;       // Issue PIE ack
//}
interrupt void SciaRxFifoIsr(void)
{

    uint8_t data = 0;
    //RxData = SciaRegs.SCIRXBUF.all;

    //上一个数据帧应用层未取走，数据丢弃
//  if ((KeyBoardRxFrame.status == 1) || (KeyBoardRxFrame.index >= 40))
//  {
//      KeyBoardRxFrame.pdata[KeyBoardRxFrame.index++] = SciaRegs.SCIRXBUF.all;
//      KeyBoardRxFrame.pdata[KeyBoardRxFrame.index++] = SciaRegs.SCIRXBUF.all;
//      KeyBoardRxFrame.pdata[KeyBoardRxFrame.index++] = SciaRegs.SCIRXBUF.all;
//      KeyBoardRxFrame.pdata[KeyBoardRxFrame.index++] = SciaRegs.SCIRXBUF.all;
//  }else
//  {
//      KeyBoardRxFrame.pdata[KeyBoardRxFrame.index++] = SciaRegs.SCIRXBUF.all;
    data = SciaRegs.SCIRXBUF.all;
    mid_comservice_data_in(EN_COMSERVICE_COM_INDEX, &data, 1);
//      KeyBoardRxFrame.pdata[KeyBoardRxFrame.index++] = SciaRegs.SCIRXBUF.all;
//      KeyBoardRxFrame.pdata[KeyBoardRxFrame.index++] = SciaRegs.SCIRXBUF.all;
//      KeyBoardRxFrame.pdata[KeyBoardRxFrame.index++] = SciaRegs.SCIRXBUF.all;
//      KeyBoardRxFrame.ticker = KeyBoardRxFrame.spacetime;
//  }

    SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;   // Clear Overflow flag
    SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all |= M_INT9;       // Issue PIE ack
    _gSciaRecvFlag = 1;

}

uint16_t GetSciaRecvFlag(void)
{
    return _gSciaRecvFlag;
}

void ClearSciaRecvFlag(void)
{
    _gSciaRecvFlag = 0;
}
/**
* @brief: SciaFifoInit
* @param:
* @note:
*
*/
void SciaFifoInit()
{
    GPIO_SetupPinMux(RS485_DE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RS485_DE_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    RE_ENABLE();
    GPIO_SetupPinMux(43, GPIO_MUX_CPU1, 15);
    GPIO_SetupPinOptions(43, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(42, GPIO_MUX_CPU1, 15);
    GPIO_SetupPinOptions(42, GPIO_OUTPUT, GPIO_ASYNC);

    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.SCIA_RX_INT = &SciaRxFifoIsr;
//   PieVectTable.SCIA_TX_INT = &SciaTxFifoIsr;
    EDIS;

    SciaRegs.SCICCR.all = 0x0007;      // 1 stop bit,  No loopback
    // No parity,8 char bits,
    // async mode, idle-line protocol
    SciaRegs.SCICTL1.all = 0x0003;     // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    SciaRegs.SCICTL2.bit.TXINTENA = 1;
    SciaRegs.SCICTL2.bit.RXBKINTENA = 1;
    SciaRegs.SCIHBAUD.all = ((uint16_t)SCI_PRD  & 0xFF00U) >> 8U;
    SciaRegs.SCILBAUD.all = (uint16_t)SCI_PRD  & 0x00FFU;
    SciaRegs.SCICCR.bit.LOOPBKENA = 0; // Enable loop back
    SciaRegs.SCIFFTX.all = 0xE040;
    SciaRegs.SCIFFRX.all = 0x0021;
    SciaRegs.SCIFFCT.all = 0x00;

    SciaRegs.SCICTL1.all = 0x0023;     // Relinquish SCI from Reset
    SciaRegs.SCIFFTX.bit.TXFIFORESET = 1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;

    KeyBoardRxFrame.pdata = RxData;

#if SCI_RX_ISR_EN
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1;   // PIE Group 9, INT1
    //   PieCtrlRegs.PIEIER9.bit.INTx2 = 1;   // PIE Group 9, INT2
    IER |= M_INT9;                         // Enable CPU INT
    // EINT;
#endif
}

/**
 * @brief Scid初始化
 */
void ScidFifoInit()
{
    GPIO_SetupPinMux(77, GPIO_MUX_CPU2, 6);
    GPIO_SetupPinOptions(77, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(76, GPIO_MUX_CPU2, 6);
    GPIO_SetupPinOptions(76, GPIO_OUTPUT, GPIO_ASYNC);

    EALLOW;  // This is needed to write to EALLOW protected registers
    //PieVectTable.SCIA_RX_INT = &SciaRxFifoIsr;
//   PieVectTable.SCIA_TX_INT = &SciaTxFifoIsr;
    EDIS;

    ScidRegs.SCICCR.all = 0x0007;      // 1 stop bit,  No loopback
    // No parity,8 char bits,
    // async mode, idle-line protocol
    ScidRegs.SCICTL1.all = 0x0003;     // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    ScidRegs.SCICTL2.bit.TXINTENA = 1;
    ScidRegs.SCICTL2.bit.RXBKINTENA = 1;
    ScidRegs.SCIHBAUD.all = ((uint16_t)SCID_PRD & 0xFF00U) >> 8U;
    ScidRegs.SCILBAUD.all = (uint16_t)SCID_PRD & 0x00FFU;
    ScidRegs.SCICCR.bit.LOOPBKENA = 0; // Enable loop back
    ScidRegs.SCIFFTX.all = 0xE040;
    ScidRegs.SCIFFRX.all = 0x0021;
    ScidRegs.SCIFFCT.all = 0x00;

    ScidRegs.SCICTL1.all = 0x0023;     // Relinquish SCI from Reset
    ScidRegs.SCIFFTX.bit.TXFIFORESET = 1;
    ScidRegs.SCIFFRX.bit.RXFIFORESET = 1;

    KeyBoardRxFrame.pdata = RxData;

#if SCI_RX_ISR_EN
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1;   // PIE Group 9, INT1
    //   PieCtrlRegs.PIEIER9.bit.INTx2 = 1;   // PIE Group 9, INT2
    IER |= M_INT9;                         // Enable CPU INT
    // EINT;
#endif
}

/**
* @brief: SciaXmit
* @param: a
* @note:
*
*/
#define SCI_TIMEOUT_COUNT 100000UL
void SciaXmit(int a)
{
    uint32_t timeout = SCI_TIMEOUT_COUNT; // 20260429，优化代码
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0 && timeout--)
    {
        __asm(" NOP");
    }
    if(timeout == 0)
    {
        return;
    }
    SciaRegs.SCITXBUF.all = a;
}

/**
 * @brief Scid发送单字节
 * @param a 发送字节
 */
void ScidXmit(int a)
{
    uint32_t timeout = SCI_TIMEOUT_COUNT; // 20260429，优化代码
    while (ScidRegs.SCIFFTX.bit.TXFFST != 0 && timeout--)
    {
        __asm(" NOP");
    }
    if(timeout == 0)
    {
        return;
    }
    ScidRegs.SCITXBUF.all = a;
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

    DE_ENABLE();
    for(i = 0; i < len; i ++)
    {
        SciaXmit(data[i]);
    }

    while((SciaRegs.SCICTL2.bit.TXRDY == 0) || (SciaRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }
    RE_ENABLE();
}

void SciaSendByte(uint8_t data)
{

    DE_ENABLE();

    SciaXmit(data);

    while((SciaRegs.SCICTL2.bit.TXRDY == 0) || (SciaRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }
    RE_ENABLE();
}

/**
 * @brief Scid发送单字节
 * @param data 发送字节
 */
void ScidSendByte(uint8_t data)
{
    ScidXmit(data);

    while ((ScidRegs.SCICTL2.bit.TXRDY == 0) || (ScidRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }
}

/**
* @brief: SciaMsg
* @param: msg
* @note:
*
*/
void SciaMsg(char * msg)
{
    int i;
    DE_ENABLE();
    i = 0;
    while(msg[i] != '\0')
    {
        SciaXmit(msg[i]);
        i++;
    }
    while((SciaRegs.SCICTL2.bit.TXRDY == 0) || (SciaRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }
    RE_ENABLE();
}
//
// End of file
//
