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

__attribute__((interrupt)) void cla1Isr1();
__attribute__((interrupt)) void cla1Isr2();
__attribute__((interrupt)) void cla1Isr3();
__attribute__((interrupt)) void cla1Isr4();
__attribute__((interrupt)) void cla1Isr5();
__attribute__((interrupt)) void cla1Isr6();
__attribute__((interrupt)) void cla1Isr7();
__attribute__((interrupt)) void cla1Isr8();

__attribute__((interrupt)) void Cla1Task1();
__attribute__((interrupt)) void Cla1Task2();
__attribute__((interrupt)) void Cla1Task3();
__attribute__((interrupt)) void Cla1Task4();
__attribute__((interrupt)) void Cla1Task5();
__attribute__((interrupt)) void Cla1Task6();
__attribute__((interrupt)) void Cla1Task7();
__attribute__((interrupt)) void Cla1Task8();

void ClearCpuInterrupt(void);
void Interrupt_Init(void);

#endif /* DEVICE_INTERRUPT_H_ */
