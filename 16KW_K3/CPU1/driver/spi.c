/*
 * spi.c
 *
 *  Created on: 2025年1月6日
 *      Author: Nova007456
 */
#include "spi.h"
#include "F28x_Project.h"

static uint16_t _gRdataTemp = 0;

/**
* @brief: _FlashSpiWriteByte
* @param: a
* @return:
* @note:
*/
static void _FlashSpiWriteByte(uint16_t a)
{
    SpiaRegs.SPITXBUF = (a << 8);

    while(SpiaRegs.SPIFFRX.bit.RXFFST !=1) { }
    _gRdataTemp = SpiaRegs.SPIRXBUF;
}
/**
* @brief: _SpiReadByte
* @param: void
* @return:
* @note:
*/
static uint16_t _FlashSpiReadByte(void)
{
    SpiaRegs.SPITXBUF = (0 << 8);
    while(SpiaRegs.SPIFFRX.bit.RXFFST !=1) { }
    _gRdataTemp = SpiaRegs.SPIRXBUF;

    return _gRdataTemp;
}
/**
* @brief: SpiWriteData
* @param: len
* @param: data
* @note:
*/
void SpiWriteData(uint16_t len, uint16_t* data)
{
    uint16_t i = 0;
    for(i = 0; i < len; i ++)
    {
        _FlashSpiWriteByte(data[i]);
    }
}
/**
* @brief: SpiReadData
* @param: len
* @param: data
* @note:
*/
void SpiReadData(uint16_t len, uint16_t* data)
{
    uint16_t i = 0;
    for(i = 0; i < len; i ++)
    {
        data[i] =  _FlashSpiReadByte();
    }
}
/**
* @brief: CH390_Spi_Init
* @param: void
* @return:Nothing
* @note:CH390芯片SPI总函数
*/
void CH390_Spi_Init(void)
{
    EALLOW;
    SpibRegs.SPICCR.bit.SPISWRESET = 0;//复位
    //SpibRegs.SPIFFTX.all = 0xEO40;    //SPI发送寄存器,等价于如下片段配置
    SpibRegs.SPIFFTX.bit.SPIRST = 1;    //第15位:SPIFIFO开始重新发送和接收
    SpibRegs.SPIFFTX.bit.SPIFFENA = 0;  //第14位:禁止FIFI
    SpibRegs.SPIFFTX.bit.TXFIFO = 1;    //第13位:重新使能发送FIFO
    SpibRegs.SPIFFTX.bit.TXFFST = 0;    //第12-8位:发送FIOFO中有几个字,最多16个(0x10000)
    SpibRegs.SPIFFTX.bit.TXFFINT = 0;   //第7位:不产生中断
    SpibRegs.SPIFFTX.bit.TXFFINTCLR = 1;//第6位:清除中断标志位
    SpibRegs.SPIFFTX.bit.TXFFIENA = 0;  //第5位:禁止基于TXFFIVL匹配小于或等于的TXFIFO中断
    SpibRegs.SPIFFTX.bit.TXFFIL = 0;    //第4-0位:发送TXFIFO中断触发级位

    //SpibRegs.SPIFFRX.all = 0x2044;    //SPI接收寄存器,等价于如下片段配置
    SpibRegs.SPIFFRX.bit.RXFFINTCLR=1;  //第14位:清楚FIFO溢出标志位
    SpibRegs.SPIFFRX.bit.RXFIFORESET=1; //第13位:重新使能接收FIFO
    SpibRegs.SPIFFRX.bit.RXFFST = 0;    //第12-8位:接收FIOFO中有几个字,最多16个(0x10000)
    SpibRegs.SPIFFRX.bit.RXFFINT = 1;   //第6位:清除中断标志位
    SpibRegs.SPIFFRX.bit.RXFFIENA = 0;  //第5位:禁止基于RXFFIVL匹配小于或等于的RXFIFO中断
    SpibRegs.SPIFFRX.bit.RXFFIL = 0;    //第4-0位:接收RXFIFO中断触发级位

    SpibRegs.SPICTL.bit.MASTER_SLAVE = 1;   //设定为主机
    SpibRegs.SPICTL.bit.TALK = 1;           //使能发送
    SpibRegs.SPICTL.bit.CLK_PHASE = 0;      //SPI信号延迟半周期，与CLKPOLARITY配合
    SpibRegs.SPICTL.bit.SPIINTENA = 0;      //禁止溢出使能

    SpibRegs.SPIBRR.bit.SPI_BIT_RATE = SPI_BRR;//波特率

    SpibRegs.SPIPRI.bit.FREE = 1;   //忽略中断挂起，SPI继续工作

    SpibRegs.SPICCR.bit.CLKPOLARITY = 1;//0;//上升沿输出数据，下降沿输入数据
    SpibRegs.SPICCR.bit.SPICHAR = 7;//字长控制位 8位
    SpibRegs.SPICCR.bit.SPILBK = 0;     //禁止SPI回送模式
    SpibRegs.SPICCR.bit.SPISWRESET = 1;//SPI准备接受或发送下一个字节
    EDIS;
}
/**
* @brief: Flash_Spi_Init
* @param: void
* @return:Nothing
* @note:Flash芯片SPI总函数
*/
void Flash_Spi_Init(void)
{
    EALLOW;
    SpiaRegs.SPIFFTX.all = 0xE040;
    SpiaRegs.SPIFFRX.all = 0x2044;
    SpiaRegs.SPIFFCT.all = 0x0;
    SpiaRegs.SPICCR.bit.SPISWRESET = 0;
    SpiaRegs.SPICCR.bit.CLKPOLARITY = 0;
    SpiaRegs.SPICCR.bit.SPICHAR = (8-1);
    SpiaRegs.SPICCR.bit.SPILBK = 0;
    SpiaRegs.SPICTL.bit.MASTER_SLAVE = 1;
    SpiaRegs.SPICTL.bit.TALK = 1;
    SpiaRegs.SPICTL.bit.CLK_PHASE = 1;
    SpiaRegs.SPICTL.bit.SPIINTENA = 0;
    SpiaRegs.SPIBRR.bit.SPI_BIT_RATE = SPI_FLASH_BRR;
    SpiaRegs.SPIPRI.bit.FREE = 1;
    SpiaRegs.SPICCR.bit.SPISWRESET = 1;
    EDIS;
}

/**
* @brief: Spi_Init
* @param: void
* @return:Nothing
* @note:SPI总函数
*/
void Spi_Init(void)
{
    EALLOW;
    CH390_Spi_Init();   //CH390的SPIB配置
    Flash_Spi_Init();   //Flash的SPIA配置
    EDIS;
}

