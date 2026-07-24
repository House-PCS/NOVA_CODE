/*
 * dev_dual_core_receive.c
 */
#include "dev_dual_core_receive.h"
#include "F2837xD_Examples.h"

#pragma CODE_SECTION(DevDcr_GetCpuAdcResult, ".TI.ramfunc");
#pragma CODE_SECTION(DevDcr_GetCpu1ToCpu2MsgPtr, ".TI.ramfunc");

static volatile CpuAdc_t *s_adc_result = (volatile CpuAdc_t *)0;
static volatile StsInputStatus_t *s_sts_input = (volatile StsInputStatus_t *)0;

static volatile void *DevDcr_GetCpu1AdcAddress(void)
{
    volatile DevDct_Cpu1ToCpu2Msg_t *msg = DevDcr_GetCpu1ToCpu2Msg();

    return (volatile void *)(msg->cpu1_adc_result_addr);
}

static volatile void *DevDcr_GetCpu1StsInputAddress(void)
{
    volatile DevDct_Cpu1ToCpu2Msg_t *msg = DevDcr_GetCpu1ToCpu2Msg();

    return (volatile void *)(msg->sts_input_addr);
}

bool DevDcr_ExchangeAddress(uint32_t timeoutMs)
{
    uint32_t waitMs = 0U;

    while (waitMs < timeoutMs)
    {
        s_adc_result = (volatile CpuAdc_t *)DevDcr_GetCpu1AdcAddress();
        s_sts_input = (volatile StsInputStatus_t *)DevDcr_GetCpu1StsInputAddress();
        if ((s_adc_result != (volatile CpuAdc_t *)0) && (s_sts_input != (volatile StsInputStatus_t *)0))
        {
            return true;
        }

        DELAY_MS(1U);
        waitMs++;
    }

    return false;
}

volatile CpuAdc_t *DevDcr_GetCpuAdcResult(void)
{
    return s_adc_result;
}
volatile StsInputStatus_t *DevDcr_GetStsInput(void)
{
    return s_sts_input;
}
volatile DevDct_Cpu1ToCpu2Msg_t *DevDcr_GetCpu1ToCpu2MsgPtr(void)
{
    return DevDcr_GetCpu1ToCpu2Msg();
}

void DevDcr_ApplyPh3RunFlag(Ph3Calc_Channel_t *ch,
                            const volatile CpuAdcPh3RunFlag_t *flag)
{
    if((ch == NULL) || (flag == NULL))
    {
        return;
    }

    ch->enable_volt_pll = flag->enable_volt_pll;
    ch->enable_curr_pll = flag->enable_curr_pll;
    ch->enable_power_calc = flag->enable_power_calc;
}
