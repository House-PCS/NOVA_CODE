/*
 * debug_analysis.c
 *
 *  Created on: Jan 7, 2025
 *      Author: Nova007456
 */
#include "debug_analysis.h"
#include "ethernet_analysis.h"
#include "cla_cpu_share.h"
#include "CH390.h"
#include "sci.h"

uint16_t gDebugTemp[5 + DEBUG_CPU_CHANNEL_NUM * 4];
float32 garrDebugDataCpu[DEBUG_CPU_CHANNEL_NUM];
uint16_t gDebugValue[10];

/**
 * @brief Set CPU debug buffer value
 * @param buff_num Buffer channel index
 * @param value    Value to write
 */
void SetCpuDebugData(uint16_t buff_num,float32 value)
{
    if(buff_num >= DEBUG_CPU_CHANNEL_NUM)
    {
        return;
    }
    garrDebugDataCpu[buff_num] = (float32)value;
}

/**
 * @brief Send debug data via SCID
 */
void ScidSendMessage(void)
{
    uint16_t* data1 = 0;
    uint16_t* data2 = 0;
    uint8_t sum = 0;
    /* Pack and send CLA debug data */
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
    /* Pack and send CPU debug data */
    sum = 1;
    data1 = (uint16_t*)&garrDebugDataCpu;
    gDebugTemp[0] = 0xAA;
    gDebugTemp[1] = 0xBB;
    gDebugTemp[2] = DEBUG_CPU_CHANNEL_NUM * 4 + 2;
    gDebugTemp[3] = 1;
    for(int j = 0; j < DEBUG_CPU_CHANNEL_NUM; j++)
    {
        data2 = data1 + 2 * j;
        gDebugTemp[4 + j * 4] = (uint8_t)(*data2);
        gDebugTemp[5 + j * 4] = (uint8_t)(*data2 >> 8);
        gDebugTemp[6 + j * 4] = (uint8_t)(*(data2 + 1));
        gDebugTemp[7 + j * 4] = (uint8_t)(*(data2 + 1) >> 8);
        sum += gDebugTemp[4 + j * 4] + gDebugTemp[5 + j * 4] + gDebugTemp[6 + j * 4] + gDebugTemp[7 + j * 4];
    }
    gDebugTemp[4 + DEBUG_CPU_CHANNEL_NUM * 4] = (uint8_t)(sum);
    for(int k = 0;(k < (5 + DEBUG_CPU_CHANNEL_NUM * 4));++k)
    {
        ScidSendByte(gDebugTemp[k]);
    }
}

/**
 * @brief Send debug data over Ethernet
 */
void Debug_Send(void)
{
    uint8_t   packet_data[110 + DEBUG_CPU_CHANNEL_NUM * 4];
    uint16_t* data1 = 0;
    uint16_t* data2 = 0;

    if((ch390_get_link_status() == 1) && (GetEthSysUpgradeState() == false))
    {
        for(int i = 0;i < 6;i++) // Broadcast destination (bytes 0-5)
        {
            packet_data[i] = 0xFF;
        }
        /* MAC address */
        packet_data[6] = 0x11;
        packet_data[7] = 0x22;
        packet_data[8] = 0x33;
        packet_data[9] = 0x44;
        packet_data[10] = 0x55;
        packet_data[11] = 0x66;
        /* IPv4 ethertype */
        packet_data[12] = 0x08;
        packet_data[13] = 0x00;
        /* IPv4 header: version 4, 20-byte header */
        packet_data[14] = 0x45;
        packet_data[15] = 0x00; // DSCP and ECN
        packet_data[16] = ((20 + 8 + 68 + DEBUG_CPU_CHANNEL_NUM * 4) >> 8); // Total length (high)
        packet_data[17] = ((20 + 8 + 68 + DEBUG_CPU_CHANNEL_NUM * 4) & 0xFF); // Total length (low)
        packet_data[18] = 0x66; // Identification (high)
        packet_data[19] = 0x88; // Identification (low)
        packet_data[20] = 0x00; // Flags and fragment offset (high)
        packet_data[21] = 0x00; // Flags and fragment offset (low)
        packet_data[22] = 0x80; // TTL
        packet_data[23] = 0x11; // Protocol: UDP
        packet_data[24] = 0x00; // Header checksum (high)
        packet_data[25] = 0x00; // Header checksum (low)

        packet_data[26] = 192; // Source IP
        packet_data[27] = 168;
        packet_data[28] = 1;
        packet_data[29] = 18;

        packet_data[30] = 192; // Destination IP
        packet_data[31] = 168;
        packet_data[32] = 1;
        packet_data[33] = 16;
        /* UDP header */
        packet_data[34] = 0x08; // Source port 2300 (high)
        packet_data[35] = 0xFC;
        packet_data[36] = 0x03; // Destination port 1000 (high)
        packet_data[37] = 0xE8;
        packet_data[38] = (8 + 68 + DEBUG_CPU_CHANNEL_NUM * 4) >> 8; // UDP length (high)
        packet_data[39] = (8 + 68 + DEBUG_CPU_CHANNEL_NUM * 4) & 0xFF; // UDP length (low)
        packet_data[40] = 0x00; // UDP checksum (high)
        packet_data[41] = 0x00; // UDP checksum (low)
        /* Payload */

        /* Pack CLA debug data for Ethernet */
        data1 = (uint16_t*)&garrDebugDataCla;
        for(int j = 0; j < 16; j++)
        {
            data2 = data1 + 2 * j;
            packet_data[42 + j * 4] = (uint8_t)(*data2);
            packet_data[43 + j * 4] = (uint8_t)(*data2 >> 8);
            packet_data[44 + j * 4] = (uint8_t)(*(data2 + 1));
            packet_data[45 + j * 4] = (uint8_t)(*(data2 + 1) >> 8);
        }
        /* Pack CPU debug data for Ethernet */
        data1 = (uint16_t*)&garrDebugDataCpu;
        for(int j = 0; j < DEBUG_CPU_CHANNEL_NUM; j++)
        {
            data2 = data1 + 2 * j;
            packet_data[106 + j * 4] = (uint8_t)(*data2);
            packet_data[107 + j * 4] = (uint8_t)(*data2 >> 8);
            packet_data[108 + j * 4] = (uint8_t)(*(data2 + 1));
            packet_data[109 + j * 4] = (uint8_t)(*(data2 + 1) >> 8);
        }
        packet_data[106 + DEBUG_CPU_CHANNEL_NUM * 4] = 0x00;
        packet_data[107 + DEBUG_CPU_CHANNEL_NUM * 4] = 0x00;
        packet_data[108 + DEBUG_CPU_CHANNEL_NUM * 4] = 0x80;
        packet_data[109 + DEBUG_CPU_CHANNEL_NUM * 4] = 0x7F;

        ch390_send_packet(packet_data, 42 + 68 + DEBUG_CPU_CHANNEL_NUM * 4);
    }
}

void DebugReveiveAnalysis(uint16_t buff[],uint16_t len)
{
    for(uint8_t i = 0;i < len; i++)
    {
        gDebugValue[i] = buff[i];
    }
}
