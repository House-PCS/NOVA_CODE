

#ifndef _ENET_H__
#define _ENET_H__

//#include "netif.h"
#include "gd32f4xx_enet.h"
#include "stdbool.h"


typedef enum
{
	EN_ENET_COM_MII,
	EN_ENET_COM_RMII
}EN_ENET_COM_TYPE;


typedef struct _enet_handle_stru_t
{
    //复位引脚配置，没有可忽略
    uint32_t                rst_gpiox;
    uint32_t                rst_gpio_pinx;
    
    bool 	                smi_sim;
    uint32_t                mdc_gpiox;
    uint32_t                mdc_gpio_pinx;
    uint32_t                mdio_gpiox;
    uint32_t                mdio_gpio_pinx;
    
    EN_ENET_COM_TYPE        com_type;
    
//	void                	(*irq_tx_process)();
//	void                	(*irq_rx0_process)();
//	void                	(*irq_rx1_process)();
}Enet_HandleType;


/* function declarations */
/* setup ethernet system(GPIOs, clocks, MAC, DMA, systick) */
void EnetInit(Enet_HandleType *pHandle);

#endif /* GD32F4xx_ENET_EVAL_H */
