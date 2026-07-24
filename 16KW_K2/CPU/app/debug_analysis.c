/*
 * debug_analysis.c
 *
 *  Created on: 2025年1月7日
 *      Author: Nova007456
 */
#include "debug_analysis.h"
#include "ethernet_analysis.h"
#include "cla_control.h"
#include "CH390.h"
#include "sci.h"

uint16_t gDebugTemp[69];
float32 garrDebugDataCpu[16];
uint16_t gDebugValue[10];

/**
 * @brief CPU的调试Buff赋值
 * @param data 发送字节
 */
void SetCpuDebugData(uint16_t buff_num,float32 value)
{
    garrDebugDataCpu[buff_num] = (float32)value;
}

/**
 * @brief Scid发送
 * @param data 发送
 */
void ScidSendMessage(void)
{
    uint16_t* data1 = 0;
    uint16_t* data2 = 0;
    uint8_t sum = 0;
    /*      CLA调试数据串口外发    */
    data1 = (uint16_t*)&garrDebugDataCla;
    gDebugTemp[0] = 0xAA;
    gDebugTemp[1] = 0xBB;
    gDebugTemp[2] = 16 * 4 + 2;
    gDebugTemp[3] = 0;
    for(int j = 0; j < 16; j++)
    {
        data2 = data1 + 2 * j;
        gDebugTemp[4 + j * 4] = (uint8_t)(*data2);
        gDebugTemp[5 + j * 4] = (uint8_t)(*data2 >> 8);
        gDebugTemp[6 + j * 4] = (uint8_t)(*(data2 + 1));
        gDebugTemp[7 + j * 4] = (uint8_t)(*(data2 + 1) >> 8);
        sum += gDebugTemp[4 + j * 4] + gDebugTemp[5 + j * 4] + gDebugTemp[6 + j * 4] + gDebugTemp[7 + j * 4];
    }
    gDebugTemp[8 + 15 * 4] = (uint8_t)(sum);
    for(int k = 0;(k < (9 + 15 * 4));++k)
    {
        ScidSendByte(gDebugTemp[k]);
    }
    /*      CPU调试数据串口外发    */
    sum = 1;
    data1 = (uint16_t*)&garrDebugDataCpu;
    gDebugTemp[0] = 0xAA;
    gDebugTemp[1] = 0xBB;
    gDebugTemp[2] = 16 * 4 + 2;
    gDebugTemp[3] = 1;
    for(int j = 0; j < 16; j++)
    {
        data2 = data1 + 2 * j;
        gDebugTemp[4 + j * 4] = (uint8_t)(*data2);
        gDebugTemp[5 + j * 4] = (uint8_t)(*data2 >> 8);
        gDebugTemp[6 + j * 4] = (uint8_t)(*(data2 + 1));
        gDebugTemp[7 + j * 4] = (uint8_t)(*(data2 + 1) >> 8);
        sum += gDebugTemp[4 + j * 4] + gDebugTemp[5 + j * 4] + gDebugTemp[6 + j * 4] + gDebugTemp[7 + j * 4];
    }
    gDebugTemp[8 + 15 * 4] = (uint8_t)(sum);
    for(int k = 0;(k < (9 + 15 * 4));++k)
    {
        ScidSendByte(gDebugTemp[k]);
    }
}

/**
 * @name Debug_Send
 * @brief
 */

void Debug_Send(void)
{
    uint8_t   packet_data[175];
    uint16_t* data1 = 0;
    uint16_t* data2 = 0;

    if((ch390_get_link_status() == 1) && (GetEthSysUpgradeState() == false))
    {
        for(int i = 0;i < 6;i++)//0-5广播目标
        {
            packet_data[i] = 0xFF;
        }
        /*      MAC部分    */
        packet_data[6] = 0x11;
        packet_data[7] = 0x22;
        packet_data[8] = 0x33;
        packet_data[9] = 0x44;
        packet_data[10] = 0x55;
        packet_data[11] = 0x66;
        /*      IPV4    */
        packet_data[12] = 0x08;
        packet_data[13] = 0x00;
        /*      网络版本，如下为网络层    */
        packet_data[14] = 0x45;//版本为4,网络层共20个字节
        packet_data[15] = 0x00;//DSCP区分服务代码点
        packet_data[16] = ((20 + 8 + 132) >> 8);//长度高位，包含网络层+应用层+数据层长度
        packet_data[17] = ((20 + 8 + 132) & 0xFF);//长度低位，包含网络层+应用层+数据层长度
        packet_data[18] = 0x66;//报文ID标识,高位
        packet_data[19] = 0x88;//报文ID标识.低位
        packet_data[20] = 0x00;//分包标识.高位
        packet_data[21] = 0x00;//分包标识.高位
        packet_data[22] = 0x80;//存活时间
        packet_data[23] = 0x11;//UDP协议
        packet_data[24] = 0x00;//头部检查高位
        packet_data[25] = 0x00;//头部检查低位

        packet_data[26] = 192;//源地址
        packet_data[27] = 168;
        packet_data[28] = 1;
        packet_data[29] = 18;

        packet_data[30] = 192;//目标地址
        packet_data[31] = 168;
        packet_data[32] = 1;
        packet_data[33] = 16;
        /*      如下为应用层    */
        packet_data[34] = 0x08;//源端口2300
        packet_data[35] = 0xFC;
        packet_data[36] = 0x03;  //目标端口1000
        packet_data[37] = 0xE8;
        packet_data[38] = (8 + 132) >> 8;//应用层加数据长度
        packet_data[39] = (8 + 132) & 0xFF;
        packet_data[40] = 0x00;//校验码
        packet_data[41] = 0x00;//校验码
        /*      如下为数据层    */

        /*      CLA调试数据串口外发    */
        data1 = (uint16_t*)&garrDebugDataCla;
        for(int j = 0; j < 16; j++)
        {
            data2 = data1 + 2 * j;
            packet_data[42 + j * 4] = (uint8_t)(*data2);
            packet_data[43 + j * 4] = (uint8_t)(*data2 >> 8);
            packet_data[44 + j * 4] = (uint8_t)(*(data2 + 1));
            packet_data[45 + j * 4] = (uint8_t)(*(data2 + 1) >> 8);
        }
        /*      CPU调试数据以太网外发    */
        data1 = (uint16_t*)&garrDebugDataCpu;
        for(int j = 0; j < 16; j++)
        {
            data2 = data1 + 2 * j;
            packet_data[106 + j * 4] = (uint8_t)(*data2);
            packet_data[107 + j * 4] = (uint8_t)(*data2 >> 8);
            packet_data[108 + j * 4] = (uint8_t)(*(data2 + 1));
            packet_data[109 + j * 4] = (uint8_t)(*(data2 + 1) >> 8);
        }
        packet_data[170] = 0x00;
        packet_data[171] = 0x00;
        packet_data[172] = 0x80;
        packet_data[173] = 0x7F;

        ch390_send_packet(packet_data, 42 + 132);
    }
}

void DebugReveiveAnalysis(uint16_t buff[],uint16_t len)
{
    for(uint8_t i = 0;i < len; i++)
    {
        gDebugValue[i] = buff[i];
    }
}
