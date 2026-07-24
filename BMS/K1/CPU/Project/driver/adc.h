
#ifndef __ADC_H_
#define __ADC_H_


#include "gd32f30x_adc.h"
#include "stdbool.h"
#include "drv_opt.h"
#include "systick.h"

#define CHANNEL_NUM_PER_ADC		18

/*前期仅支持独立模式，规则通道，单次转换*/
typedef struct _adc_config_channel_stru_t
{
	bool					enable;		
    uint8_t                 chl_num;
	uint32_t				adc_gpiox;
	uint32_t				adc_gpio_pinx;
}Adc_Config_Channel;



typedef struct _adc_config_stru_t
{
	
	Adc_Config_Channel		channel_config[CHANNEL_NUM_PER_ADC];
	rcu_periph_enum			rcu_adc;
	uint32_t				adc_peri;
	
	bool					en_irq;/*所有ADC共用一个中断*/
	uint32_t				nvic_irq;
	uint8_t					irq_prio;
	uint8_t					irq_sub_prio;
	
	
}Adc_Config;



typedef struct _adc_handle_stru_t
{
	Adc_Config				*adc_cfg;	
	uint8_t					current_chl;
	uint16_t				*pRData;
	float					*pRValtage;
}Adc_HandleType;


#define ADC_CHL(chl_num)			chl_num		



void AdcClkInit(void);


void AdcInit(Adc_HandleType *pHandle);
bool AdcVoltageGet(Adc_HandleType *pHandle);


#endif 




















