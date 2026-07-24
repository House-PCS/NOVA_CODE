
#ifndef __KSTAR_REG_JUDGE_H_
#define __KSTAR_REG_JUDGE_H_



#include "sys_info.h"

bool KstarRegBaseJudge(uint16_t reg_addr, uint16_t reg_data);
bool KstarRegInvJudge(uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data);
bool KstarRegRemoteJudge(uint16_t reg_addr, uint16_t reg_data);

#endif














