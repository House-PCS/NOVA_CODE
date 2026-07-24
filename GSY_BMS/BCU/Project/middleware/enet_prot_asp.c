/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
  
#include "enet_prot_asp.h"
#include "fpara_asp.h"
//#include "hello_gigadevice.h"
#include "prot_modbus_debug.h"
#include "prot_modbus_ems.h"
#include <string.h>

#define DATA_MAX_SIZE    350
struct data {
    uint16_t length;
    uint8_t  bytes[DATA_MAX_SIZE];
};
/*!
    \brief      this function is called when an error occurs on the connection
    \param[in]  arg: user supplied argument
    \param[in]  err: error value
    \param[out] none
    \retval     none
*/
static void enet_debug_conn_err(void *arg, err_t err)
{
    struct data *data;
    data = (struct data *)arg;

    mem_free(data);
}
/*!
    \brief      called when a data is received on the telnet connection
    \param[in]  arg: the user argument
    \param[in]  pcb: the tcp_pcb that has received the data
    \param[in]  p: the packet buffer
    \param[in]  err: the error value linked with the received data
    \param[out] none
    \retval     err_t: error value
*/
static err_t enet_debug_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    struct pbuf *q;
    struct data *data = (struct data *)arg;
    char *c;
    int i;
    struct data send_data;

    /* we perform here any necessary processing on the pbuf */
    if(p != NULL) {
        /* we call this function to tell the LwIp that we have processed the data */
        /* this lets the stack advertise a larger window, so more data can be received*/
        tcp_recved(pcb, p->tot_len);

        /* check the data if NULL, no data passed, return withh illegal argument error */
        if(!data) {
            pbuf_free(p);
            return ERR_ARG;
        }

        for(q = p; q != NULL; q = q->next) {
            c = q->payload;
            for(i = 0; i < q->len; i++) {
                if(data->length < DATA_MAX_SIZE) {
                    /* save the received data to data->bytes */
                    data->bytes[data->length++] = c[i];
                }
            }
        }

        if(data->length < 6) {

//            tcp_write(pcb, HELLO, strlen(HELLO), 1);
//            printf("\n\rGigaDevice\n\rTelnet %s %s", HELLO, data->bytes);
            tcp_write(pcb, data->bytes, data->length, TCP_WRITE_FLAG_COPY);            
        }
        else
        {
            send_data.length = ModbusSlaveProtProce(MODBUS_TCP, &ProtModbusSlaveCfg_Debug, data->bytes,	\
							data->length, send_data.bytes);
            if(send_data.length >= 1)
            {
                tcp_write(pcb, send_data.bytes, send_data.length, TCP_WRITE_FLAG_COPY);
            }
        }
        data->length = 0;
        /* end of processing, we free the pbuf */
        pbuf_free(p);

    } else if(err == ERR_OK) {
        /* when the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
        /* we free the allocated memory and we close the connection */
        mem_free(data);
        return tcp_close(pcb);
    }
    return ERR_OK;
}
/*!
    \brief      this function when the Telnet connection is established
    \param[in]  arg: user supplied argument
    \param[in]  pcb: the tcp_pcb which accepted the connection
    \param[in]  err: error value
    \param[out] none
    \retval     err_t: error value
*/
static err_t enet_debug_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    /* tell LwIP to associate this structure with this connection. */
    tcp_arg(pcb, mem_calloc(sizeof(struct data), 1));

    /* configure LwIP to use our call back functions. */
    tcp_err(pcb, enet_debug_conn_err);
    tcp_recv(pcb, enet_debug_recv);

    /* send out the first message */
//    tcp_write(pcb, iptxt, strlen((char *)iptxt), 1);
//    sprintf((char *)iptxt, "You telnet computer's IP is: %d.%d.%d.%d\n", iptab[3], iptab[2], iptab[1], iptab[0]);
//    printf("%s\r\n", iptxt);
//    tcp_write(pcb, gd32_str, strlen((char *)gd32_str), 1);
//    tcp_write(pcb, GREETING, strlen(GREETING), 1);

    return ERR_OK;
}
static err_t enet_ems_accept(void *arg, struct tcp_pcb *pcb, err_t err);


#define TCP_SERVER_PROT_NUM     2
static TcpServerProtType TcpServerProt[TCP_SERVER_PROT_NUM]={
    {77,enet_debug_accept},     //默认端口，debug协议必须放在第一个
    {24,enet_ems_accept},
};
void tcp_server_prot_init(void)
{
    struct tcp_pcb *pcb;
    
#if (ENEN_PARA_FPARA == 1)
    for(uint8_t i = 1; i < TCP_SERVER_PROT_NUM; i++)
    {
        TcpServerProt[i].port = IR_PARA_ENET_Port;    
    }
#endif
    for(uint8_t i = 0; i < TCP_SERVER_PROT_NUM; i++)
    {
        /* create a new TCP control block */
        pcb = tcp_new();

        /* assign to the new pcb a local IP address and a port number */
        /* using IP_ADDR_ANY allow the pcb to be used by any local interface */
        tcp_bind(pcb, IP_ADDR_ANY, TcpServerProt[i].port);

        /* set the connection to the LISTEN state */
        pcb = tcp_listen(pcb);

        /* Specify the function to be called when a connection is established */
        tcp_accept(pcb, TcpServerProt[i].accept);
    }
}
///////////////////////////////////TCP_Client/////////////////

/* 定义端口号 */
#define TCP_REMOTE_PORT    8000 /* 远端端口 */
#define TCP_LOCAL_PORT     2980  /* 本地端口 */
 
/******************************************************************************
 * 描述  : 数据接收回调函数
 * 参数  : -
 * 返回  : -
******************************************************************************/
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb,
                             struct pbuf *p, err_t err)
{
    uint32_t i;
    
    /* 数据回传 */
    //tcp_write(tpcb, p->payload, p->len, 1);
    
    if (p != NULL)
    {
        struct pbuf *ptmp = p;
        
        /* 打印接收到的数据 */
        printf("get msg from %d:%d:%d:%d port:%d:\r\n",
            *((uint8_t *)&tpcb->remote_ip.addr),
            *((uint8_t *)&tpcb->remote_ip.addr + 1),
            *((uint8_t *)&tpcb->remote_ip.addr + 2),
            *((uint8_t *)&tpcb->remote_ip.addr + 3),
            tpcb->remote_port);
        
        while(ptmp != NULL)
        {
            for (i = 0; i < p->len; i++)
            {
                printf("%c", *((char *)p->payload + i));
            }
            
            ptmp = p->next;
        }
        
        printf("\r\n");
        
        tcp_recved(tpcb, p->tot_len);
        
        /* 释放缓冲区数据 */
        pbuf_free(p);
    }
    else if (err == ERR_OK)
    {
        printf("tcp client closed\r\n");
        
        tcp_recved(tpcb, p->tot_len);
        
        return tcp_close(tpcb);
    }
 
    return ERR_OK;
}
 
/******************************************************************************
 * 描述  : 连接服务器回调函数
 * 参数  : -
 * 返回  : -
******************************************************************************/
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
//    printf("tcp client connected  \r\n");
    
    tcp_write(tpcb, "tcp client connected", strlen("tcp client connected"), 0);

    /* 注册接收回调函数 */
    tcp_recv(tpcb, tcp_client_recv);
 
    return ERR_OK;
}
/* TCP客户端连接服务器错误回调函数 */
static void TCPClientConnectError(void *arg, err_t err)
{
  /* 重新启动连接 */
  uint8_t i = 0;
//    tcp_client_init();
}
struct tcp_pcb *client_pcb;
    ip_addr_t serverIp;
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
    tcp_connect(client_pcb, &serverIp, TCP_REMOTE_PORT, tcp_client_connected);
}
void tcp_client_init(void)
{
    
 
    /* 服务器IP */
    IP4_ADDR(&serverIp, 10, 50, 3, 140);

//#if (ENEN_PARA_FPARA == 1)
//    for(uint8_t i = 1; i < TCP_SERVER_PROT_NUM; i++)
//    {
//        TcpServerProt[i].port = IR_PARA_ENET_Port;    
//    }
//#endif
    /* create a new TCP control block */
    client_pcb = tcp_new();

    /* assign to the new pcb a local IP address and a port number */
    /* using IP_ADDR_ANY allow the pcb to be used by any local interface */
    tcp_bind(client_pcb, IP_ADDR_ANY, TCP_LOCAL_PORT);
//tcp_poll(client_pcb,tcp_client_poll,10);
    /* 连接服务器 */
    tcp_connect(client_pcb, &serverIp, TCP_REMOTE_PORT, tcp_client_connected);
    tcp_err(client_pcb, TCPClientConnectError);
}
void lwip_netif_status_callback(struct netif *netif)
{
    if((netif->flags & NETIF_FLAG_UP) != 0) {
        /* initilaize the helloGigadevice module telnet 23 */
        //TCP_Server
        tcp_server_prot_init();
        //TCP_Client
//        tcp_client_init();
        //UDP
    }
}

/////////////////////ems/////////////////
static void enet_ems_conn_err(void *arg, err_t err)
{
    struct data *data;
    data = (struct data *)arg;

    mem_free(data);
}
/*!
    \brief      called when a data is received on the telnet connection
    \param[in]  arg: the user argument
    \param[in]  pcb: the tcp_pcb that has received the data
    \param[in]  p: the packet buffer
    \param[in]  err: the error value linked with the received data
    \param[out] none
    \retval     err_t: error value
*/
static err_t enet_ems_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    struct pbuf *q;
    struct data *data = (struct data *)arg;
    char *c;
    int i;
    struct data send_data;

    /* we perform here any necessary processing on the pbuf */
    if(p != NULL) {
        /* we call this function to tell the LwIp that we have processed the data */
        /* this lets the stack advertise a larger window, so more data can be received*/
        tcp_recved(pcb, p->tot_len);

        /* check the data if NULL, no data passed, return withh illegal argument error */
        if(!data) {
            pbuf_free(p);
            return ERR_ARG;
        }

        for(q = p; q != NULL; q = q->next) {
            c = q->payload;
            for(i = 0; i < q->len; i++) {
                if(data->length < DATA_MAX_SIZE) {
                    /* save the received data to data->bytes */
                    data->bytes[data->length++] = c[i];
                }
            }
        }

        if(data->length < 6) {

//            tcp_write(pcb, HELLO, strlen(HELLO), 1);
            tcp_write(pcb, data->bytes, data->length, TCP_WRITE_FLAG_COPY);
//            printf("\n\rGigaDevice\n\rTelnet %s %s", HELLO, data->bytes);
            
        }
        else
        {
            send_data.length = ModbusSlaveProtProce(MODBUS_TCP, &ProtModbusSlaveCfg_Ems, data->bytes,	\
							data->length, send_data.bytes);
            if(send_data.length >= 1)
            {
                tcp_write(pcb, send_data.bytes, send_data.length, TCP_WRITE_FLAG_COPY);
            }
        }
        data->length = 0;
        /* end of processing, we free the pbuf */
        pbuf_free(p);

    } else if(err == ERR_OK) {
        /* when the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
        /* we free the allocated memory and we close the connection */
        mem_free(data);
        return tcp_close(pcb);
    }
    return ERR_OK;
}
static err_t ems_poll(void *arg, struct tcp_pcb *tpcb)
{
    uint16_t i = 0;
    
    i++;
}
/*!
    \brief      this function when the Telnet connection is established
    \param[in]  arg: user supplied argument
    \param[in]  pcb: the tcp_pcb which accepted the connection
    \param[in]  err: error value
    \param[out] none
    \retval     err_t: error value
*/
static err_t enet_ems_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    /* tell LwIP to associate this structure with this connection. */
    tcp_arg(pcb, mem_calloc(sizeof(struct data), 1));

    /* configure LwIP to use our call back functions. */
    tcp_err(pcb, enet_ems_conn_err);
    tcp_recv(pcb, enet_ems_recv);
//tcp_poll(pcb,ems_poll,2);
    /* send out the first message */
//    tcp_write(pcb, iptxt, strlen((char *)iptxt), 1);
//    sprintf((char *)iptxt, "You telnet computer's IP is: %d.%d.%d.%d\n", iptab[3], iptab[2], iptab[1], iptab[0]);
//    printf("%s\r\n", iptxt);
//    tcp_write(pcb, gd32_str, strlen((char *)gd32_str), 1);
//    tcp_write(pcb, GREETING, strlen(GREETING), 1);

    return ERR_OK;
}