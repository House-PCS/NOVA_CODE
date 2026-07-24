#include "fwdgt.h"

#define FWDGT_RST_TIME  1000    //µ¥Î»£º1ms
void FWDGT_Init()
{
    fwdgt_config(FWDGT_RST_TIME,FWDGT_PSC_DIV32);
    DBG_CTL1 |= DBG_CTL1_FWDGT_HOLD;

    fwdgt_enable();
}
void FWDGT_Feed()
{
    fwdgt_counter_reload();
}
