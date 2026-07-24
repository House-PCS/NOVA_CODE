//------------------------------------------------------------------------------
/*File Name         :   EthUdp.h
**Original Author   :   lvyb
**Creation Date     :   
**Description       :   UDP固件库函数头文件

*/
//------------------------------------------------------------------------------

#ifndef _ETHUDP_H
#define _ETHUDP_H

//------------------------------------------------------------------------------
/*Description       :   头文件
*/
//------------------------------------------------------------------------------

#include "system_CMSDK_CM0.h"


//------------------------------------------------------------------------------
/*Description       :   宏定义
*/
//------------------------------------------------------------------------------

typedef struct
{
  __IO   uint32_t  DATA0;
  __IO   uint32_t  DATA1;
  __IO   uint32_t  DATA2;
}UDP_CACHE_TypeDef;

typedef struct
{
  __IO   uint32_t  Length;
  __IO   uint32_t  Ready;
  __IO   uint32_t  EthIrqClr;
}UDP_CTRL_TypeDef;

#define UDP_CACHE_RX                ((UDP_CACHE_TypeDef*)   0x30000000)
#define UDP_CACHE_TX                ((UDP_CACHE_TypeDef*)   0x30001000)
#define UDP_CTRL                    ((UDP_CTRL_TypeDef*)    0x40000010)


//------------------------------------------------------------------------------
/*Description       :   函数声明
*/
//------------------------------------------------------------------------------
void EthUdpInit(void);
void INT02_Handler(void);
void EthUdpDataTransfer(void);


#endif
