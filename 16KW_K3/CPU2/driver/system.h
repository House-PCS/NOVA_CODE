/*
 * system.h
 *
 *  Created on: 2025Äê1ÔÂ6ÈÕ
 *      Author: Nova007456
 */
#ifndef SYSTEM_H_
#define SYSTEM_H_

#define CPU_SYS_CLOCK       (200*1000000)
#define LSPCLK_FREQ         (CPU_SYS_CLOCK/4)
#define PWMSYSCLOCK_FREQ    (100*1000000)

void SysRestart(void);

#endif /* SYSTEM_H_ */
