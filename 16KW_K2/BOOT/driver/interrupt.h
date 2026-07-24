/*
 * interrupt.h
 *
 *  Created on: 2025年1月6日
 *      Author: Nova007456
 */

#ifndef DEVICE_INTERRUPT_H_
#define DEVICE_INTERRUPT_H_

// 定义回调函数类型（假设需要传递中断标志）
typedef void (*CpuIntCallback)(void);

// 注册回调的接口
void RegisterCpuIntCallback(CpuIntCallback callback);

void ClearCpuInterrupt(void);
void Interrupt_Init(void);

#endif /* DEVICE_INTERRUPT_H_ */
