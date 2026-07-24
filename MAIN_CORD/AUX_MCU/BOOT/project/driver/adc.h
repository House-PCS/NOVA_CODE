
#ifndef __ADC_H_
#define __ADC_H_

#include "stdbool.h"
#include "drv_opt.h"
#include "systick.h"
#include "dma.h"

#define CHANNEL_NUM_PER_ADC		18
typedef enum
{
#if USE_ADC0
    ADC0_PORT,
#endif
#if USE_ADC1
    ADC1_PORT,
#endif
#if USE_ADC2
    ADC2_PORT,
#endif
#if USE_ADC3
    ADC3_PORT,
#endif   
    ADC_NUM
}EN_ADCx_PORT;

typedef struct _adc_config_channel_stru_t
{
	bool					enable;		
    uint8_t                 chl_id;
	GPIO_Module*		    adc_gpiox;
	uint32_t				adc_gpio_pinx;
}Adc_Config_Channel;


typedef struct _adc_config_stru_t
{
	
	Adc_Config_Channel		channel_config[CHANNEL_NUM_PER_ADC];
	uint32_t			    rcc_adc;
	ADC_Module*			    adc_peri;
    
    bool                    continuous_mode;
    bool                    scan_mode;
    
    /*DMA相关参数*/
    bool                    en_dma; 
    DMA_ChannelType*        dma_peri_channel;//bsp无需配置，驱动自行计算
    DMA_Module*             dma_peri;
    dma_channel_enum        dma_channel;    
	
	bool					en_irq;/*所有ADC共用一个中断*/
	uint32_t				nvic_irq;
	uint8_t					irq_prio;
	uint8_t					irq_sub_prio;
	
	bool                    en_dma_irq;
    uint8_t                 nvic_dma_irq;
    uint8_t                 irq_dma_prio;
    uint8_t                 irq_dma_sub_prio;
}Adc_Config;

typedef struct _adc_handle_stru_t
{
    EN_ADCx_PORT            adcx;
	Adc_Config				*adc_cfg;	
    uint8_t                 chl_valid_number;
	uint8_t					current_chl;
//    uint32_t                *pRData;
	uint16_t				*pRData;
    uint32_t                continuous_num;//连续转换多少次后停止，主要用于DMA数据传输量，0表示无限次，DMA则配为循环模式
	float					*pRValtage;
    
    void                    (*dma_complete_process)();
}Adc_HandleType;

#define ADC_CHL(chl_num)			chl_num		








void AdcInit(Adc_HandleType *pHandle);
void AdcStartConv(Adc_HandleType *pHandle);
void AdcStopConv(Adc_HandleType *pHandle);
bool AdcVoltageGet(Adc_HandleType *pHandle);
void AdcDMAIrqProcess(Adc_HandleType *pHandle);

#endif













