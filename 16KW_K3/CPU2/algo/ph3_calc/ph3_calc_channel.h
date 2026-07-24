/*
 * ph3_calc_channel.h
 *
 * 三相计算通道：采样输入与调度运行
 */
#ifndef ALGO_PH3_CALC_CHANNEL_H_
#define ALGO_PH3_CALC_CHANNEL_H_

#include "ph3_calc_def.h"

void Ph3Calc_ChannelInit(Ph3Calc_Channel_t* ch,
                         Ph3Calc_PfMode_e pf_mode,
                         Ph3Calc_PllMode_e pll_mode);
void Ph3Calc_ChannelSetSample(Ph3Calc_Channel_t* ch,
                              float32 va, float32 vb, float32 vc,
                              float32 ia, float32 ib, float32 ic);
void Ph3Calc_ChannelRun(Ph3Calc_Channel_t* ch);

#endif /* ALGO_PH3_CALC_CHANNEL_H_ */
