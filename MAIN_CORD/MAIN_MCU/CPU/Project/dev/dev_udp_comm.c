#include "dev_udp_comm.h"
#include "udp.h"
#include <string.h>



SlaveUdpCommPara_t gstDspUdpCommPara;
SlaveUdpCommPara_t gstBmsUdpCommPara;
SlaveIpInfo  gstBmsBroadcastIpInfo;
SlaveIpInfo  gstBmsDefaultIpInfo;
SlaveIpInfo  gstBmsIpInfo[BMS_MAX_NUM];
void Lwip_udp_send_data(struct udp_pcb *udppcb,uint8_t *pdata,uint16_t len)
{
    struct pbuf *ptr;

//    uint16_t i = 0;
//    for(i = 11; i<1024;i+=2)
//    {
//        g_lwip_demo_sendbuf[i] = (uint8_t)i;
//        g_lwip_demo_sendbuf[i+1] = (uint8_t)(i>>8);
//    }
//strlen((char *)g_lwip_demo_sendbuf)
    ptr = pbuf_alloc(PBUF_TRANSPORT, len , PBUF_POOL);
    if(ptr)
    {
        pbuf_take(ptr,(char *)pdata,len);//strlen((char *)g_lwip_demo_sendbuf)
        udp_send(udppcb,ptr);
        pbuf_free(ptr);
    
    }
}                                                     
void Lwip_udp_sendto_data(struct udp_pcb *udppcb,uint8_t *pdata,uint16_t len,const ip_addr_t *dst_ip, u16_t dst_port)
{
    struct pbuf *ptr;

    ptr = pbuf_alloc(PBUF_TRANSPORT, len , PBUF_POOL);
    if(ptr)
    {
        pbuf_take(ptr,(char *)pdata,len);//strlen((char *)g_lwip_demo_sendbuf)
        udp_sendto(udppcb,ptr,dst_ip,dst_port);
        pbuf_free(ptr);
    
    }        
}

//3、err_t udp_sendto(struct udp_pcb *pcb, struct pbuf *p, 
//                    struct ip_addr *dst_ip, u16_t dst_port)
uint8_t g_lwip_send_flag=0;
void lwip_udp_connection_close(struct udp_pcb *upcb)
{
    udp_disconnect(upcb);
    udp_remove(upcb);
//    g_lwip_send_flag &=~(1<<5);
}




uint8_t g_lwip_demo_recv_buf[1500];
#define  LWIP_DEMO_RX_BUF 1500;
static uint32_t  udp_total_test_size =0;
static uint32_t  udp_pack_count=0;
static void lwip_udp_callback(void *arg,struct udp_pcb *upcb,struct pbuf *p,const ip_addr_t *addr,uint16_t port)
{
    uint32_t data_len =0;
    struct pbuf *q;
    uint32_t  lwip_demo_rx_buf_size = LWIP_DEMO_RX_BUF;
    if(p!=NULL)
    {
        memset(g_lwip_demo_recv_buf,0,sizeof(g_lwip_demo_recv_buf));
        for(q=p;q!=NULL;q=q->next)
        {
              
            if(q->len > (lwip_demo_rx_buf_size - data_len))
            {
                memcpy(g_lwip_demo_recv_buf+data_len,q->payload,(lwip_demo_rx_buf_size - data_len));
            }
            else
            {
                memcpy(g_lwip_demo_recv_buf+data_len,q->payload,q->len);
            }
            data_len += q->len;
            if(data_len >lwip_demo_rx_buf_size)
                break;
        }
//        udp_pack_count++;
//        udp_total_test_size += data_len;
        pbuf_free(p);
        p=NULL;

        if(upcb == gstDspUdpCommPara.slave_udp_pcb)
        {
            if(gstDspUdpCommPara.callback != NULL)
            {
                gstDspUdpCommPara.callback(g_lwip_demo_recv_buf,data_len,0);
            } 
        }
        else 
        {
            for(uint8_t i =0; i< BMS_MAX_NUM;i++)
            {
                if(addr->addr  == (u32_t)(gstBmsIpInfo[i].remot_ipaddr.addr))
                {
                    if(gstBmsUdpCommPara.callback != NULL)
                    {
                        gstBmsUdpCommPara.callback(g_lwip_demo_recv_buf,data_len,i);
                    }                    
                }

                if(addr->addr  == (u32_t)(gstBmsDefaultIpInfo.remot_ipaddr.addr))
                {
                   
                    if(gstBmsUdpCommPara.callback != NULL)
                    {
                        gstBmsUdpCommPara.callback(g_lwip_demo_recv_buf,data_len,0);
                    }  
                }
            } 
        }
    } 
}




void InitialSlaveUdpProc(SlaveUdpCommPara_t *udp_comm_para)
{
    err_t err;
    if(udp_comm_para->slave_udp_pcb)
    {
        err = udp_connect(udp_comm_para->slave_udp_pcb,IP_ADDR_ANY,udp_comm_para ->slave_ip_info.remot_port);//远程端口
        if(err ==ERR_OK)
        {
            err = udp_bind(udp_comm_para->slave_udp_pcb,&udp_comm_para->slave_ip_info.local_ipaddr,udp_comm_para ->slave_ip_info.local_port);//本地端口
            if(err == ERR_OK)
            {
                udp_recv(udp_comm_para->slave_udp_pcb,lwip_udp_callback,NULL);//udp_comm_para->lwip_udp_callback
 
            }
        }
        
    }    
}

void InitalDspUdpPara(recv_call_back_fun dsp_call_back_fun)
{

    gstDspUdpCommPara.slave_udp_pcb = udp_new();
    IP4_ADDR(&gstDspUdpCommPara.slave_ip_info.remot_ipaddr,192,168,1,9);
    IP4_ADDR(&gstDspUdpCommPara.slave_ip_info.local_ipaddr,192,168,1,102);

    gstDspUdpCommPara.slave_ip_info.remot_port = DSP_REMOT_PORT;
    gstDspUdpCommPara.slave_ip_info.local_port = LOCAL_PORT;
    gstDspUdpCommPara.callback = dsp_call_back_fun;
    InitialSlaveUdpProc(&gstDspUdpCommPara);
}


void InitalBmsUdpPara(recv_call_back_fun bms_call_back_fun)
{
    gstBmsUdpCommPara.slave_udp_pcb = udp_new(); 
    IP4_ADDR(&gstBmsUdpCommPara.slave_ip_info.remot_ipaddr,192,168,1,255);
    gstBmsUdpCommPara.slave_ip_info.remot_port = BMS_REMOT_PORT;
    IP4_ADDR(&gstBmsUdpCommPara.slave_ip_info.local_ipaddr,192,168,1,102); 
    gstBmsUdpCommPara.slave_ip_info.local_port = BMS_LOCAL_PORT;
    gstBmsUdpCommPara.callback = bms_call_back_fun;

    InitialSlaveUdpProc(&gstBmsUdpCommPara);  

    IP4_ADDR(&gstBmsBroadcastIpInfo.remot_ipaddr,192,168,1,255);   //广播
    gstBmsBroadcastIpInfo.remot_port = BMS_REMOT_PORT;
    IP4_ADDR(&gstBmsDefaultIpInfo.remot_ipaddr,192,168,1,10);   //上电默认IP
    gstBmsDefaultIpInfo.remot_port = BMS_REMOT_PORT;

    for(uint8_t i =0; i<BMS_MAX_NUM;i++)
    {
        IP4_ADDR(&gstBmsIpInfo[i].remot_ipaddr,192,168,1,(BMS_DEFAULT_IP+i));   //num1
        gstBmsIpInfo[i].remot_port = BMS_REMOT_PORT;        
    }
   
}


