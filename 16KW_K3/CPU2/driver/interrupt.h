/*
 * interrupt.h
 *
 * CPU2 interrupt configuration interface.
 */
#ifndef DEVICE_INTERRUPT_H_
#define DEVICE_INTERRUPT_H_

typedef void (*CpuIntCallback)(void);

void RegisterCpuIntCallback(CpuIntCallback callback);
void ClearCpuInterrupt(void);
void Interrupt_Init(void);

#endif /* DEVICE_INTERRUPT_H_ */
