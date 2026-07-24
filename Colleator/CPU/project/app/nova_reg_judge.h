
#ifndef _NOVA__REG_JUDGE_H_
#define _NOVA__REG_JUDGE_H_



#include "sys_info.h"

bool NovaRegBaseJudge(uint16_t reg_addr, uint16_t reg_data);
bool NovaRegInvJudge(uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data);
bool NovaRegRemoteJudge(uint16_t reg_addr, uint16_t reg_data);

#endif














