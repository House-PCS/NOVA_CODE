

#ifndef DEV_UDP_COMM_H
#define DEV_UDP_COMM_H


#define BYTE_MODE           0
#define WORD_MODE           1  

#define  BMS_MAX_NUM            (6)


#define  LOCAL_PORT             2000
#define  DSP_REMOT_PORT         2001


#define  BMS_LOCAL_PORT         1234
#define  BMS_REMOT_PORT         1234


#define  BMS_DEFAULT_IP         11ul


#define UDP_WRITE_CMD           (0x45)
#define UDP_READ_CMD            (0X43)




#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32f4xx.h"
#include "udp.h"
#include <string.h>
#pragma pack(1)


typedef struct _slave_udp_tset
{
    ip_addr_t remot_ipaddr;
    ip_addr_t local_ipaddr;
    ip_addr_t netmask;
    uint32_t remot_port;
    uint32_t local_port;
} SlaveIpInfo;

typedef struct _slave_udp_comm_para_stru_t
{
    struct udp_pcb *slave_udp_pcb;
//    ip_addr_t remot_ipaddr;
//    ip_addr_t local_ipaddr;
//    ip_addr_t netmask;
//    uint32_t remot_port;
//    uint32_t local_port;
    SlaveIpInfo slave_ip_info;
    void (*callback)(uint8_t *addr,uint16_t len,uint8_t num_id);
} SlaveUdpCommPara_t;






typedef union _slave_udp_write_union_t
{

    struct 
    {
        uint16_t  device_type;
        uint8_t   reserved;
        uint8_t   opcmd;
        uint32_t  addr;
        uint16_t  length;  
        uint8_t   data[1036-9];        
    }write_data_t;
    uint8_t data[1036];
 
} SlaveUdpDataWrite_u;


typedef struct _slave_udp_write_stru_t
{
    
    SlaveUdpDataWrite_u slave_udp_write_data;
    struct udp_pcb *slave_udp_pcb;   
    SlaveIpInfo     slave_ip_port;

} SlaveUdpWrite_t;

#pragma pack()


typedef void(*recv_call_back_fun)(uint8_t *pdata,uint16_t len,uint8_t mum_id);


extern  SlaveUdpCommPara_t gstDspUdpCommPara;
extern  SlaveUdpCommPara_t gstBmsUdpCommPara;
extern  SlaveIpInfo  gstBmsIpInfo[BMS_MAX_NUM];
extern  SlaveIpInfo  gstBmsDefaultIpInfo;
//void Lwip_udp_send_data(void);
void TestUdpProc(void);
void InitialSlaveUdpProc(SlaveUdpCommPara_t *udp_comm_para);
void Lwip_udp_send_data(struct udp_pcb *udppcb,uint8_t *pdata,uint16_t len);
void Lwip_udp_sendto_data(struct udp_pcb *udppcb,uint8_t *pdata,uint16_t len,const ip_addr_t *dst_ip, u16_t dst_port);

void InitalDspUdpPara(recv_call_back_fun dsp_call_back_fun);
void InitalBmsUdpPara(recv_call_back_fun bms_call_back_fun);
#ifdef __cplusplus
}
#endif

#endif /* GD32F450I_EVAL_H */
