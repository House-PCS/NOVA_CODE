#include "dev_mco.h"

#include "n32g4fr_rcc.h"

void DevMcoInit()
{
	RccMcoBspInit();
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
	RCC_ConfigMcoPllClk(RCC_MCO_PLLCLK_DIV12);
	RCC_ConfigMco(RCC_MCO_PLLCLK);
}