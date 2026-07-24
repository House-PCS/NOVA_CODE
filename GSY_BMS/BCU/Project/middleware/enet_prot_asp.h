

#ifndef _ENET_PROT_ASP_H__
#define _ENET_PROT_ASP_H__


#include "lwip/tcp.h"

typedef struct _tcp_server_prot_stru_t
{
    uint16_t    port;
    tcp_accept_fn accept;
    
}TcpServerProtType; 





#endif
