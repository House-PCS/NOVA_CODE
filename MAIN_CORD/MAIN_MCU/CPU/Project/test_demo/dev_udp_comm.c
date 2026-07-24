/*!
    \file    gd32f450i_eval.c
    \brief   firmware functions to manage leds, keys, COM ports
    
    \version 2024-01-15, V3.2.0, firmware for GD32F4xx
*/

/*
    Copyright (c) 2024, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32f450i_eval.h"
#include "udp.h"
#include <string.h>


//    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
struct udp_pcb *udppcb;
uint8_t g_lwip_demo_sendbuf[1024]="hello nova ";
void Lwip_udp_send_data(void)
{
    struct pbuf *ptr;

    uint16_t i = 0;
    for(i = 11; i<1024;i+=2)
    {
        g_lwip_demo_sendbuf[i] = (uint8_t)i;
        g_lwip_demo_sendbuf[i+1] = (uint8_t)(i>>8);
    }
//strlen((char *)g_lwip_demo_sendbuf)
    ptr = pbuf_alloc(PBUF_TRANSPORT, 1024 , PBUF_POOL);
    if(ptr)
    {
        pbuf_take(ptr,(char *)g_lwip_demo_sendbuf,1024);//strlen((char *)g_lwip_demo_sendbuf)
        udp_send(udppcb,ptr);
        pbuf_free(ptr);
    
    }
}

uint8_t g_lwip_send_flag=0;
void lwip_udp_connection_close(struct udp_pcb *upcb)
{
    udp_disconnect(upcb);
    udp_remove(upcb);
    g_lwip_send_flag &=~(1<<5);
}

void lwip_udp_set_remotip()
{
    
}
#define  LWIP_DEMO_PORT  80
#define  LWIP_REMOT_PORT 81
uint8_t g_lwip_demo_recv_buf[512];
#define  LWIP_DEMO_RX_BUF 1024;
uint32_t  udp_total_test_size =0;
uint32_t  udp_pack_count=0;
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
        upcb->remote_ip = *addr;
        upcb->remote_port = port;
        g_lwip_send_flag |= 1<<6;
        udp_pack_count++;
        udp_total_test_size += data_len;
        pbuf_free(p);
        p=NULL;
        
    }
    else
    {
    
    }
    
}


void TestUdpProc(void)
{

    udppcb = udp_new();
    ip_addr_t rempt_ipaddr;
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    err_t err;
    if(udppcb)
    {
        IP4_ADDR(&rempt_ipaddr,192,168,1,10);
//        IP4_ADDR(&rempt_ipaddr,255,255,255,255);
        IP4_ADDR(&ipaddr,192,168,1,2);
        err = udp_connect(udppcb,&rempt_ipaddr,LWIP_REMOT_PORT);//远程端口
        if(err ==ERR_OK)
        {
            err = udp_bind(udppcb,&ipaddr,LWIP_DEMO_PORT);//本地端口
            if(err == ERR_OK)
            {
                udp_recv(udppcb,lwip_udp_callback,NULL);
 
            }
        }
        
    }


}
