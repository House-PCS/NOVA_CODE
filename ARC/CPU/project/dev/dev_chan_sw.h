#ifndef __DEV_CHAN_SW_H_
#define __DEV_CHAN_SW_H_


typedef enum
{
    EN_CHANNAL1 =0,
    EN_CHANNAL2,
    EN_CHANNAL3,
    EN_CHANNAL4,
    EN_CHANNAL_NUM
}EN_CHANNAL;

extern EN_CHANNAL channal_switch;
void dev_chan_sw(EN_CHANNAL channal_switch);
int  dev_chan_detect();
#endif
