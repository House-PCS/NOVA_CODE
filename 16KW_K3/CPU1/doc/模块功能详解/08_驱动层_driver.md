# 08 驱动层 driver

对应源码：`driver/*`（不含 TI `source/` CSL）

---

## 1. 总览

driver 层把 F28375D 片内外设封装成工程可调用的 Init/读写接口，并定义引脚与频率常量。应用与 device 层不应直接散落配置寄存器（理想情况）。

---

## 2. GPIO（`gpio.c/h`）

### 2.1 职责

集中配置：逆变桥、PV、继电器、CH390、SPI Flash、SCI、ADC 外部模拟开关、TZ 故障输入、风扇、功率板 Boot ID 等。

### 2.2 关键引脚（摘录）

| 功能 | GPIO |
|------|------|
| LED | 99 |
| Flash CS | 61 |
| 继电器 | 87~94 |
| RCMU 自检 | 28 |
| 电弧输入 | 54 |
| 逆变 PWM | 0~11 |
| PV PWM | 14~17 |
| CH390 SPIB | 63/64/65，CS66，RST67，INT68 |

### 2.3 关键 API

- `Gpio_Init()`：按子功能顺序配置全部复用  
- `Set*RlyState()`：逻辑开关 → 电平；市电继电器同时控 88/89  
- `PoBootStateRead()`：GPIO50~53 读四位功率板 ID  

硬件保护：GPIO82/83 经 Input XBAR → TZ1/TZ2，供 PWM Trip Zone 封波。

---

## 3. ADC（`adc.c/h`）

### 3.1 配置要点

- 12-bit 单端，ADCA~D  
- `ACQPS_SYS_CLKS=29`，预分频=14  
- 触发：`TRIGSEL=18` → **EPWM7 SOCB**  

### 3.2 `AdcConfig()` 步骤

1. 时钟/分辨率/单端/转换完成时刻/上电  
2. 延时约 2 ms 等模拟稳定  
3. 配置各 SOC 通道、采样窗、EPWM7 触发  

通道覆盖：逆变 V/I、PV×4 V/I、市电/发电机/负载三相、CT、RCMU、母线正负压/电流、N-PE、NTC 等。

应用由 `sample_update` 读 `ADCRESULT`。

---

## 4. PWM（`pwm.c/h`）

| 模块 | EPWM | 频率 | 备注 |
|------|------|------|------|
| 逆变 | 1~6 | 20 kHz | 死区 1 µs，互补 |
| PV | 8~9 | 40 kHz | 死区 0.5 µs |
| CPU ISR | 7 | 80 kHz | ADC SOCA/SOCB + 中断 |
| CLA/风扇 | 10 | 20 kHz | CLA Task1 触发 |
| 风扇辅 | 11 | 20 kHz | |

### 关键行为

- `InvEpwmxInit`：中心对齐、影子比较、死区、TZ 单次封波、启动即软件 Trip  
- `CpuInterruptEpwmxInit(7)` / `ClaInterruptEpwmxInit(10)`  
- `PvEpwmUpdata` / `FanEpwmUpdata`：占空比限 `(0, 0.98)`；失能强制 Trip  
- `ClearEpwmTZ`：解除 1~6、8~11 一次性 Trip  

---

## 5. SCI（`sci.c/h`）

| 口 | 波特率 | 用途 | 引脚 |
|----|--------|------|------|
| SCID | 115200 | 调试/printf | 76/77 |
| SCIA | 115200 | 协处理器 | 42/43 |
| SCIB | 115200 | ARC RS485 | 70/71，DE=69 |
| SCIC | 9600 | 电表 RS485 | 56/57，DE=44 |

`ScibSend`/`ScicSend`：发前开 DE，等 TXRDY/TXEMPTY，再回接收。  
调试：`ScidSendByte`、`fputc`/`fputs` 阻塞发送。

---

## 6. CAN（`can.c/h`）

- CANA：RX/TX = 36/37；CANB：39/38  
- 500 kbps，源时钟 200 MHz  
- 扩展帧 TX ID `0x18FF1234`，RX `0x1800C4A0`  
- API：`CanInit`、`setCAN*BitRate`、`sendCAN*Message`、`getCAN*Message`  

当前更像基础验证层，未见完整业务协议/ISR 分发。

---

## 7. CAP（`cap.c/h`）

ECAP1 作 32-bit 自由运行计时器，测 CLA 任务耗时。  
`Cap_Init` → `_InitEcapAsTimer(1)`：关捕获中断、连续模式、`CAP1=0xFFFFFFFF`、启 TSCTR。  
CLA Task1 前后读 `ECap1Regs.TSCTR` → `gClaRunTime`。

---

## 8. CPU 定时器（`cpu_timers.c/h`）

- Timer0/1：1 s；Timer2：1 ms（系统毫秒时基）  
- `cpu_timer2_isr`：仅递增 `InterruptCount`  
- `CpuTimerFlag()`：主循环根据系统时间置 1/5/10/20/100/500/1000 ms **粘性标志**  
- 消费者必须调用 `Timers_xxx()` 清除  
- `TimDifftime()`：处理 32-bit 回绕  

---

## 9. 中断与 CLA 装载（`interrupt.c`）

### `_CLA_ConfigClaMemory`

Flash→RAM 拷贝 CLA 代码/常量/数学表；初始化 CPU↔CLA Message RAM；LS0~4 程序，LS5 共享数据。

### `_CLA_InitCpu1Cla1`

设 8 个 Task 向量，仅使能 Task1；触发源 EPWM10 INT。

### `CPU_Interrupt`

调应用回调（`CpuIntServiceFun`），清 EPWM7/PIE 标志。  
`RegisterCpuIntCallback` 完成注册。

时序：**EPWM7=80 kHz CPU ISR；EPWM10=20 kHz CLA Task1**。

---

## 10. system / dual_core

### `system.h`

- CPU 200 MHz，LSPCLK 50 MHz，PWM 时钟 100 MHz  
- 定义 `EN_CPU_INTERRUPT_TIME` 时隙枚举  

### `dual_core.c`

- CPU1 持有外设；GS0/1/2/4 给 CPU2；保护 GS3/4 防 DMA 写  
- `DualCore_BootCpu2WithTimeout`：查 BootROM/IPC，Flash boot CPU2  
- `DualCore_HandshakeCpu1`：Flag4/Flag5 握手  
- `DualCore_CheckCpu2RunOnline`：心跳两次不变判离线  

由 `main` 早期分配资源，再引导握手。

---

## 附录：本章源码

> 以下为本章涉及的全部自研源码原文（不含 TI `include/`、`source/` 库）。

### `driver/adc.c`

```c
/*
 * ADCadc.c
 *
 *  Created on: 2025��10��13��
 *      Author: Nova006348
 */
#include <adc.h>

/**
 * @brief AdcConfig
 * @param
 */
void AdcConfig(void)
{
    EALLOW;
        //write configurations
        AdcaRegs.ADCCTL2.bit.PRESCALE = 14;
        AdcbRegs.ADCCTL2.bit.PRESCALE = 14;
        AdccRegs.ADCCTL2.bit.PRESCALE = 14;
        AdcdRegs.ADCCTL2.bit.PRESCALE = 14;
        AdcaRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
        AdcaRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
        AdcbRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
        AdcbRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
        AdccRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
        AdccRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
        AdcdRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
        AdcdRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
        AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
        AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;
        AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;
        AdcdRegs.ADCCTL1.bit.INTPULSEPOS = 1;
        AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
        AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;
        AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;
        AdcdRegs.ADCCTL1.bit.ADCPWDNZ = 1;
        AdcaRegs.ADCSOCPRICTL.bit.SOCPRIORITY = 1; //SOC0����������ȼ�

        DELAY_US(2000);

        /*     Adc A0    */
        AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;                  //���A���ѹ
        AdcaRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc A1    */
        AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;                  //���A�����
        AdcaRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc A2    */
        AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2;                  //PV1����
        AdcaRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcaRegs.ADCSOC3CTL.bit.CHSEL = 2;                  //PV2����
        AdcaRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcaRegs.ADCSOC4CTL.bit.CHSEL = 2;                  //PV3����
        AdcaRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcaRegs.ADCSOC5CTL.bit.CHSEL = 2;                  //PV4����
        AdcaRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc A3    */
        AdcaRegs.ADCSOC6CTL.bit.CHSEL = 3;                  //�е�B���ѹ
        AdcaRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcaRegs.ADCSOC7CTL.bit.CHSEL = 3;                  //�����B���ѹ
        AdcaRegs.ADCSOC7CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC7CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcaRegs.ADCSOC8CTL.bit.CHSEL = 3;                  //����B���ѹ
        AdcaRegs.ADCSOC8CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC8CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcaRegs.ADCSOC9CTL.bit.CHSEL = 3;                  //�̵ܼ����뵥�̵����м��B���ѹ(ADC_RelayMid_VOL_B)
        AdcaRegs.ADCSOC9CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC9CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc A4    */
        AdcaRegs.ADCSOC10CTL.bit.CHSEL = 4;                  //�е�A���ѹ
        AdcaRegs.ADCSOC10CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC10CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcaRegs.ADCSOC11CTL.bit.CHSEL = 4;                  //�����A���ѹ
        AdcaRegs.ADCSOC11CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC11CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcaRegs.ADCSOC12CTL.bit.CHSEL = 4;                  //����A���ѹ
        AdcaRegs.ADCSOC12CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC12CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcaRegs.ADCSOC13CTL.bit.CHSEL = 4;                  //�̵ܼ����뵥�̵����м��A���ѹ(ADC_RelayMid_VOL_A)
        AdcaRegs.ADCSOC13CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC13CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc A5    */
        AdcaRegs.ADCSOC14CTL.bit.CHSEL = 5;                  //A��CT����
        AdcaRegs.ADCSOC14CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcaRegs.ADCSOC14CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc B0    */
        AdcbRegs.ADCSOC0CTL.bit.CHSEL = 0;                  //���B���ѹ
        AdcbRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc B1    */
        AdcbRegs.ADCSOC1CTL.bit.CHSEL = 1;                  //���B�����
        AdcbRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc B2    */
        AdcbRegs.ADCSOC2CTL.bit.CHSEL = 2;                  //B��CT����
        AdcbRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcbRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc B3    */
        AdcbRegs.ADCSOC3CTL.bit.CHSEL = 3;                  //�е�C���ѹ
        AdcbRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcbRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcbRegs.ADCSOC4CTL.bit.CHSEL = 3;                  //�����C���ѹ
        AdcbRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcbRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcbRegs.ADCSOC5CTL.bit.CHSEL = 3;                  //����C���ѹ
        AdcbRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcbRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcbRegs.ADCSOC6CTL.bit.CHSEL = 3;                  //�̵ܼ����뵥�̵����м��A���ѹ(ADC_RelayMid_VOL_C)
        AdcbRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcbRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc C2    */
        AdccRegs.ADCSOC2CTL.bit.CHSEL = 2;                  //PV1��ѹ
        AdccRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdccRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdccRegs.ADCSOC3CTL.bit.CHSEL = 2;                  //PV2��ѹ
        AdccRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdccRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdccRegs.ADCSOC4CTL.bit.CHSEL = 2;                  //PV3��ѹ
        AdccRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdccRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdccRegs.ADCSOC5CTL.bit.CHSEL = 2;                  //PV4��ѹ
        AdccRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdccRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc C3    */
        AdccRegs.ADCSOC6CTL.bit.CHSEL = 3;                  //�е�B�����
        AdccRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdccRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdccRegs.ADCSOC7CTL.bit.CHSEL = 3;                  //�����B�����
        AdccRegs.ADCSOC7CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdccRegs.ADCSOC7CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdccRegs.ADCSOC8CTL.bit.CHSEL = 3;                  //����B�����
        AdccRegs.ADCSOC8CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdccRegs.ADCSOC8CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdccRegs.ADCSOC9CTL.bit.CHSEL = 3;                  //Ԥ��
        AdccRegs.ADCSOC9CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdccRegs.ADCSOC9CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc C4    */
        AdccRegs.ADCSOC10CTL.bit.CHSEL = 4;                  //�е�A�����
        AdccRegs.ADCSOC10CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdccRegs.ADCSOC10CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdccRegs.ADCSOC11CTL.bit.CHSEL = 4;                  //�����A�����
        AdccRegs.ADCSOC11CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdccRegs.ADCSOC11CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdccRegs.ADCSOC12CTL.bit.CHSEL = 4;                  //����A�����
        AdccRegs.ADCSOC12CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdccRegs.ADCSOC12CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdccRegs.ADCSOC13CTL.bit.CHSEL = 4;                  //Ԥ��
        AdccRegs.ADCSOC13CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdccRegs.ADCSOC13CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc D0    */
        AdcdRegs.ADCSOC0CTL.bit.CHSEL = 0;                  //���C���ѹ
        AdcdRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc D1    */
        AdcdRegs.ADCSOC1CTL.bit.CHSEL = 1;                  //���C�����
        AdcdRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc D2    */
        AdcdRegs.ADCSOC2CTL.bit.CHSEL = 2;                  //�е�C�����
        AdcdRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcdRegs.ADCSOC3CTL.bit.CHSEL = 2;                  //�����C�����
        AdcdRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcdRegs.ADCSOC4CTL.bit.CHSEL = 2;                  //����C�����
        AdcdRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcdRegs.ADCSOC5CTL.bit.CHSEL = 2;                  //Ԥ��
        AdcdRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc D3    */
        AdcdRegs.ADCSOC6CTL.bit.CHSEL = 3;                  //C��CT����
        AdcdRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc D4    */
        AdcdRegs.ADCSOC7CTL.bit.CHSEL = 4;                  //RCMU��ѹ(©������ѹ)
        AdcdRegs.ADCSOC7CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC7CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc D14    */
        AdcdRegs.ADCSOC8CTL.bit.CHSEL = 14;                 //BUS����ѹ
        AdcdRegs.ADCSOC8CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC8CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcdRegs.ADCSOC9CTL.bit.CHSEL = 14;                 //BUS����ѹ
        AdcdRegs.ADCSOC9CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC9CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcdRegs.ADCSOC10CTL.bit.CHSEL = 14;                 //BUS��ƽ������
        AdcdRegs.ADCSOC10CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC10CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ

        AdcdRegs.ADCSOC11CTL.bit.CHSEL = 14;                 //�е�N��PE��ѹ
        AdcdRegs.ADCSOC11CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC11CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        /*     Adc D15    */
        AdcdRegs.ADCSOC12CTL.bit.CHSEL = 15;                 //�¶Ȼ�ȡ
        AdcdRegs.ADCSOC12CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //ʹ�ò�������ʱ��
        AdcdRegs.ADCSOC12CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //����Դ
        EDIS;
}
```

### `driver/adc.h`

```c
/*
 * ADCadc.h
 *
 *  Created on: 2025年10月13日
 *      Author: Nova006348
 */

#ifndef DRIVER_ADC_H_
#define DRIVER_ADC_H_

#include "F28x_Project.h"
#include "system.h"

#define ACQPS_SYS_CLKS      (29)
#define ADC_TRIG_SOURCE     (18)            //触发源为EPWM7的SOCB事件

void AdcConfig(void);

#endif /* DRIVER_ADC_H_ */
```

### `driver/can.c`

```c
#include "F28x_Project.h"
#include "can.h"

//
// Globals
//
volatile unsigned long i;
unsigned char garrTXMsgData[8] = {0x55, 0xAA, 0x00, 0x00, 0x55, 0xAA, 0x00, 0x00}; // TX Data
unsigned char garrCanaRXMsgData[8] = {0, 0, 0, 0, 0, 0, 0, 0};         // RX Data
unsigned char garrCanbRXMsgData[8] = {0, 0, 0, 0, 0, 0, 0, 0};         // RX Data
uint32_t messageSize = sizeof(garrTXMsgData);          // Message Size (DLC)

static const uint16_t canBitValues[] =
{
    0x1100, // TSEG2 2, TSEG1 2, SJW 1, Divide 5
    0x1200, // TSEG2 2, TSEG1 3, SJW 1, Divide 6
    0x2240, // TSEG2 3, TSEG1 3, SJW 2, Divide 7
    0x2340, // TSEG2 3, TSEG1 4, SJW 2, Divide 8
    0x3340, // TSEG2 4, TSEG1 4, SJW 2, Divide 9
    0x3440, // TSEG2 4, TSEG1 5, SJW 2, Divide 10
    0x3540, // TSEG2 4, TSEG1 6, SJW 2, Divide 11
    0x3640, // TSEG2 4, TSEG1 7, SJW 2, Divide 12
    0x3740  // TSEG2 4, TSEG1 8, SJW 2, Divide 13
};

typedef enum
{
        //! Transmit message object.
        MSG_OBJ_TYPE_TRANSMIT,

        //! Receive message object.
        MSG_OBJ_TYPE_RECEIVE
}
msgObjType;

//
// Function Prototypes
//
uint32_t setCANABitRate(uint32_t sourceClock, uint32_t bitRate);
uint32_t setCANBBitRate(uint32_t sourceClock, uint32_t bitRate);
void  setupCANAMessageObject(uint32_t objID, uint32_t msgID, msgObjType msgType);
void  setupCANBMessageObject(uint32_t objID, uint32_t msgID, msgObjType msgType);
void sendCANAMessage(uint32_t objID);
void sendCANBMessage(uint32_t objID);
bool getCANAMessage(uint32_t objID);
bool getCANBMessage(uint32_t objID);

void CanInit(void)
{
    GPIO_SetupPinMux(36, GPIO_MUX_CPU1, 6);  //GPIO36 - CANRXA
    GPIO_SetupPinMux(37, GPIO_MUX_CPU1, 6);  //GPIO37 - CANTXA
    GPIO_SetupPinOptions(36, GPIO_INPUT, GPIO_ASYNC);
    GPIO_SetupPinOptions(37, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(39, GPIO_MUX_CPU1, 6);  //GPIO39 - CANRXB
    GPIO_SetupPinMux(38, GPIO_MUX_CPU1, 6);  //GPIO38 - CANTXB
    GPIO_SetupPinOptions(39, GPIO_INPUT, GPIO_ASYNC);
    GPIO_SetupPinOptions(38, GPIO_OUTPUT, GPIO_PUSHPULL);

    //
    // Initialize the CAN-A controller
    //
        InitCANA();
        CanaRegs.CAN_RAM_INIT.bit.RAM_INIT_DONE = 1;
        CanaRegs.CAN_CTL.bit.CCE = 1;
        InitCANB();
        CanbRegs.CAN_RAM_INIT.bit.RAM_INIT_DONE = 1;
        CanbRegs.CAN_CTL.bit.CCE = 1;

    //
    // Setup CAN to be clocked off the SYSCLKOUT
    //
        ClkCfgRegs.CLKSRCCTL2.bit.CANABCLKSEL = 0;
        ClkCfgRegs.CLKSRCCTL2.bit.CANBBCLKSEL = 0;

    //
    // Set up the bit rate for the CAN bus.  This function sets up the CAN
    // bus timing for a nominal configuration.
    // In this example, the CAN bus is set to 500 kbps.
    //
    // Consult the TRM for more information about
    // CAN peripheral clocking.
    //
        setCANABitRate(200000000, 500000);
        setCANBBitRate(200000000, 500000);


        CanaRegs.CAN_CTL.bit.Init = 0;
        CanaRegs.CAN_CTL.bit.CCE = 0;
        CanbRegs.CAN_CTL.bit.Init = 0;
        CanbRegs.CAN_CTL.bit.CCE = 0;

    //
    // Initialize the message object that will be used for sending CAN
    // messages.
    //
        setupCANAMessageObject(CAN_TX_MSG_OBJ, CAN_MSG_ID, MSG_OBJ_TYPE_TRANSMIT);
        setupCANBMessageObject(CAN_TX_MSG_OBJ, CAN_MSG_ID, MSG_OBJ_TYPE_TRANSMIT);

    //
    // Initialize the message object that will be used for receiving CAN
    // messages.
    //
        setupCANAMessageObject(CANA_RX_MSG_OBJ, CAN_MSG_REC_ID, MSG_OBJ_TYPE_RECEIVE);
        setupCANBMessageObject(CANB_RX_MSG_OBJ, CAN_MSG_REC_ID, MSG_OBJ_TYPE_RECEIVE);


}

#if 1
void CANTransmitMsg(void)
{
    uint16_t i = 0;
    for(i = 0; i < TXCOUNT; i++)
    {
        //
        // Send the CAN message using object number 1 (not the same thing as
        // CAN ID, which is also 1 in this example).  This function will cause
        // the message to be transmitted right away.
        //
        sendCANAMessage(CAN_TX_MSG_OBJ);
        sendCANBMessage(CAN_TX_MSG_OBJ);

        //
        // Delay 0.25 second before continuing
        //
//        DELAY_US(250000);

        //
        // Get the receive message
        //
//        getCANAMessage(CANA_RX_MSG_OBJ);
//        getCANBMessage(CANB_RX_MSG_OBJ);

        //
        // Increment the value in the transmitted message data.
        //
//        ucTXMsgData[0] += 0x1;
//        ucTXMsgData[1] += 0x1;
//        ucTXMsgData[2] += 0x1;
//        ucTXMsgData[3] += 0x1;

        //
        // Reset data if exceeds a byte
        //
//        if(garrCanaRXMsgData[0] == 0x55)
//        {
//            while(1);
//        }
//        if(garrCanbRXMsgData[0] == 0x55)
//        {
//            while(1);
//        }
//        if(ucTXMsgData[2] > 0xFF)
//        {
//            ucTXMsgData[2] = 0;
//        }
//        if(ucTXMsgData[3] > 0xFF)
//        {
//            ucTXMsgData[3] = 0;
//        }
    }
}
#endif
//
// setCANABitRate - Set the CAN bit rate based on device clock (Hz)
//                 and desired bit rate (Hz)
//
uint32_t setCANABitRate(uint32_t sourceClock, uint32_t bitRate)
{
    uint32_t desiredRatio;
    uint32_t canBits;
    uint32_t preDivide;
    uint32_t regValue;
    uint16_t canControlValue;

    //
    // Calculate the desired clock rate.
    //
    desiredRatio = sourceClock / bitRate;

    //
    // Make sure that the Desired Ratio is not too large.  This enforces the
    // requirement that the bit rate is larger than requested.
    //
    if((sourceClock / desiredRatio) > bitRate)
    {
        desiredRatio += 1;
    }

    //
    // Check all possible values to find a matching value.
    //
    while(desiredRatio <= CAN_MAX_PRE_DIVISOR * CAN_MAX_BIT_DIVISOR)
    {
        //
        // Loop through all possible CAN bit divisors.
        //
        for(canBits = CAN_MAX_BIT_DIVISOR;
            canBits >= CAN_MIN_BIT_DIVISOR;
            canBits--)
        {
            //
            // For a given CAN bit divisor save the pre divisor.
            //
            preDivide = desiredRatio / canBits;

            //
            // If the calculated divisors match the desired clock ratio then
            // return these bit rate and set the CAN bit timing.
            //
            if((preDivide * canBits) == desiredRatio)
            {
                //
                // Start building the bit timing value by adding the bit timing
                // in time quanta.
                //
                regValue = canBitValues[canBits - CAN_MIN_BIT_DIVISOR];

                //
                // To set the bit timing register, the controller must be
                // placed
                // in init mode (if not already), and also configuration change
                // bit enabled.  The state of the register should be saved
                // so it can be restored.
                //
                canControlValue = CanaRegs.CAN_CTL.all;
                CanaRegs.CAN_CTL.bit.Init = 1;
                CanaRegs.CAN_CTL.bit.CCE = 1;

                //
                // Now add in the pre-scalar on the bit rate.
                //
                regValue |= ((preDivide - 1) & CAN_BTR_BRP_M) |
                            (((preDivide - 1) << 10) & CAN_BTR_BRPE_M);

                //
                // Set the clock bits in the and the bits of the
                // pre-scalar.
                //
                CanaRegs.CAN_BTR.all = regValue;

                //
                // Restore the saved CAN Control register.
                //
                CanaRegs.CAN_CTL.all = canControlValue;

                //
                // Return the computed bit rate.
                //
                return(sourceClock / ( preDivide * canBits));
            }
        }

        //
        // Move the divisor up one and look again.  Only in rare cases are
        // more than 2 loops required to find the value.
        //
        desiredRatio++;
    }
    return 0;
}


//
// setCANBBitRate - Set the CAN bit rate based on device clock (Hz)
//                 and desired bit rate (Hz)
//
uint32_t setCANBBitRate(uint32_t sourceClock, uint32_t bitRate)
{
    uint32_t desiredRatio;
    uint32_t canBits;
    uint32_t preDivide;
    uint32_t regValue;
    uint16_t canControlValue;

    //
    // Calculate the desired clock rate.
    //
    desiredRatio = sourceClock / bitRate;

    //
    // Make sure that the Desired Ratio is not too large.  This enforces the
    // requirement that the bit rate is larger than requested.
    //
    if((sourceClock / desiredRatio) > bitRate)
    {
        desiredRatio += 1;
    }

    //
    // Check all possible values to find a matching value.
    //
    while(desiredRatio <= CAN_MAX_PRE_DIVISOR * CAN_MAX_BIT_DIVISOR)
    {
        //
        // Loop through all possible CAN bit divisors.
        //
        for(canBits = CAN_MAX_BIT_DIVISOR;
            canBits >= CAN_MIN_BIT_DIVISOR;
            canBits--)
        {
            //
            // For a given CAN bit divisor save the pre divisor.
            //
            preDivide = desiredRatio / canBits;

            //
            // If the calculated divisors match the desired clock ratio then
            // return these bit rate and set the CAN bit timing.
            //
            if((preDivide * canBits) == desiredRatio)
            {
                //
                // Start building the bit timing value by adding the bit timing
                // in time quanta.
                //
                regValue = canBitValues[canBits - CAN_MIN_BIT_DIVISOR];

                //
                // To set the bit timing register, the controller must be
                // placed
                // in init mode (if not already), and also configuration change
                // bit enabled.  The state of the register should be saved
                // so it can be restored.
                //
                canControlValue = CanbRegs.CAN_CTL.all;
                CanbRegs.CAN_CTL.bit.Init = 1;
                CanbRegs.CAN_CTL.bit.CCE = 1;

                //
                // Now add in the pre-scalar on the bit rate.
                //
                regValue |= ((preDivide - 1) & CAN_BTR_BRP_M) |
                            (((preDivide - 1) << 10) & CAN_BTR_BRPE_M);

                //
                // Set the clock bits in the and the bits of the
                // pre-scalar.
                //
                CanbRegs.CAN_BTR.all = regValue;

                //
                // Restore the saved CAN Control register.
                //
                CanbRegs.CAN_CTL.all = canControlValue;

                //
                // Return the computed bit rate.
                //
                return(sourceClock / ( preDivide * canBits));
            }
        }

        //
        // Move the divisor up one and look again.  Only in rare cases are
        // more than 2 loops required to find the value.
        //
        desiredRatio++;
    }
    return 0;
}

//
// setupMessageObject - Setup message object as Transmit or Receive
//
void setupCANAMessageObject(uint32_t objID, uint32_t msgID, msgObjType msgType)
{
    //
    // Use Shadow variable for IF1CMD. IF1CMD should be written to in
    // single 32-bit write.
    //
    union CAN_IF1CMD_REG CAN_IF1CMD_SHADOW;

    //
    // Wait for busy bit to clear.
    //
    while(CanaRegs.CAN_IF1CMD.bit.Busy)
    {
    }

    //
    // Clear and Write out the registers to program the message object.
    //
    CAN_IF1CMD_SHADOW.all = 0;
    CanaRegs.CAN_IF1MSK.all = 0;
    CanaRegs.CAN_IF1ARB.all = 0;
    CanaRegs.CAN_IF1MCTL.all = 0;

    //
    // Set the Control, Mask, and Arb bit so that they get transferred to the
    // Message object.
    //
    CAN_IF1CMD_SHADOW.bit.Control = 1;
    CAN_IF1CMD_SHADOW.bit.Arb = 1;
    CAN_IF1CMD_SHADOW.bit.Mask = 1;
    CAN_IF1CMD_SHADOW.bit.DIR = 1;

    //
    // Set direction to transmit
    //
    if(msgType == MSG_OBJ_TYPE_TRANSMIT)
    {
        CanaRegs.CAN_IF1ARB.bit.Dir = 1;
    }

    //
    // Set Message ID (this example assumes 11 bit ID mask)
    //
    CanaRegs.CAN_IF1ARB.bit.ID = (msgID << CAN_MSG_ID_SHIFT);
    CanaRegs.CAN_IF1ARB.bit.MsgVal = 1;
    CanaRegs.CAN_IF1ARB.bit.Xtd = 1;

    //
    // Set the data length since this is set for all transfers.  This is
    // also a single transfer and not a FIFO transfer so set EOB bit.
    //
    CanaRegs.CAN_IF1MCTL.bit.DLC = messageSize;
    CanaRegs.CAN_IF1MCTL.bit.EoB = 1;

    //
    // Transfer data to message object RAM
    //
    CAN_IF1CMD_SHADOW.bit.MSG_NUM = objID;
    CanaRegs.CAN_IF1CMD.all = CAN_IF1CMD_SHADOW.all;
}


//
// setupMessageObject - Setup message object as Transmit or Receive
//
void setupCANBMessageObject(uint32_t objID, uint32_t msgID, msgObjType msgType)
{
    //
    // Use Shadow variable for IF1CMD. IF1CMD should be written to in
    // single 32-bit write.
    //
    union CAN_IF1CMD_REG CAN_IF1CMD_SHADOW;

    //
    // Wait for busy bit to clear.
    //
    while(CanbRegs.CAN_IF1CMD.bit.Busy)
    {
    }

    //
    // Clear and Write out the registers to program the message object.
    //
    CAN_IF1CMD_SHADOW.all = 0;
    CanbRegs.CAN_IF1MSK.all = 0;
    CanbRegs.CAN_IF1ARB.all = 0;
    CanbRegs.CAN_IF1MCTL.all = 0;

    //
    // Set the Control, Mask, and Arb bit so that they get transferred to the
    // Message object.
    //
    CAN_IF1CMD_SHADOW.bit.Control = 1;
    CAN_IF1CMD_SHADOW.bit.Arb = 1;
    CAN_IF1CMD_SHADOW.bit.Mask = 1;
    CAN_IF1CMD_SHADOW.bit.DIR = 1;

    //
    // Set direction to transmit
    //
    if(msgType == MSG_OBJ_TYPE_TRANSMIT)
    {
        CanbRegs.CAN_IF1ARB.bit.Dir = 1;
    }

    //
    // Set Message ID (this example assumes 11 bit ID mask)
    //
    CanbRegs.CAN_IF1ARB.bit.ID = (msgID << CAN_MSG_ID_SHIFT);
    CanbRegs.CAN_IF1ARB.bit.MsgVal = 1;
    CanbRegs.CAN_IF1ARB.bit.Xtd = 1;

    //
    // Set the data length since this is set for all transfers.  This is
    // also a single transfer and not a FIFO transfer so set EOB bit.
    //
    CanbRegs.CAN_IF1MCTL.bit.DLC = messageSize;
    CanbRegs.CAN_IF1MCTL.bit.EoB = 1;

    //
    // Transfer data to message object RAM
    //
    CAN_IF1CMD_SHADOW.bit.MSG_NUM = objID;
    CanbRegs.CAN_IF1CMD.all = CAN_IF1CMD_SHADOW.all;
}

//
// sendCANMessage - Transmit data from the specified message object
//
void sendCANAMessage(uint32_t objID)
{
    //
    // Use Shadow variable for IF1CMD. IF1CMD should be written to in
    // single 32-bit write.
    //
    union CAN_IF1CMD_REG CAN_IF1CMD_SHADOW;

    //
    // Wait for busy bit to clear.
    //
    while(CanaRegs.CAN_IF1CMD.bit.Busy)
    {
    }

    //
    // Write data to transfer into DATA-A and DATA-B interface registers
    //
    uint16_t index;
    for(index = 0; index < messageSize; index++)
    {
        switch(index)
        {
            case 0:
                CanaRegs.CAN_IF1DATA.bit.Data_0 = garrTXMsgData[index];
                break;
            case 1:
                CanaRegs.CAN_IF1DATA.bit.Data_1 = garrTXMsgData[index];
                break;
            case 2:
                CanaRegs.CAN_IF1DATA.bit.Data_2 = garrTXMsgData[index];
                break;
            case 3:
                CanaRegs.CAN_IF1DATA.bit.Data_3 = garrTXMsgData[index];
                break;
            case 4:
                CanaRegs.CAN_IF1DATB.bit.Data_4 = garrTXMsgData[index];
                break;
            case 5:
                CanaRegs.CAN_IF1DATB.bit.Data_5 = garrTXMsgData[index];
                break;
            case 6:
                CanaRegs.CAN_IF1DATB.bit.Data_6 = garrTXMsgData[index];
                break;
            case 7:
                CanaRegs.CAN_IF1DATB.bit.Data_7 = garrTXMsgData[index];
                break;
        }
    }

    //
    // Set Direction to write and set DATA-A/DATA-B to be transfered to
    // message object
    //
    CAN_IF1CMD_SHADOW.all = 0;
    CAN_IF1CMD_SHADOW.bit.DIR = 1;
    CAN_IF1CMD_SHADOW.bit.DATA_A = 1;
    CAN_IF1CMD_SHADOW.bit.DATA_B = 1;

    //
    // Set Tx Request Bit
    //
    CAN_IF1CMD_SHADOW.bit.TXRQST = 1;

    //
    // Transfer the message object to the message object specified by
    // objID.
    //
    CAN_IF1CMD_SHADOW.bit.MSG_NUM = objID;
    CanaRegs.CAN_IF1CMD.all = CAN_IF1CMD_SHADOW.all;
}


//
// sendCANMessage - Transmit data from the specified message object
//
void sendCANBMessage(uint32_t objID)
{
    //
    // Use Shadow variable for IF1CMD. IF1CMD should be written to in
    // single 32-bit write.
    //
    union CAN_IF1CMD_REG CAN_IF1CMD_SHADOW;

    //
    // Wait for busy bit to clear.
    //
    while(CanbRegs.CAN_IF1CMD.bit.Busy)
    {
    }

    //
    // Write data to transfer into DATA-A and DATA-B interface registers
    //
    uint16_t index;
    for(index = 0; index < messageSize; index++)
    {
        switch(index)
        {
            case 0:
                CanbRegs.CAN_IF1DATA.bit.Data_0 = garrTXMsgData[index];
                break;
            case 1:
                CanbRegs.CAN_IF1DATA.bit.Data_1 = garrTXMsgData[index];
                break;
            case 2:
                CanbRegs.CAN_IF1DATA.bit.Data_2 = garrTXMsgData[index];
                break;
            case 3:
                CanbRegs.CAN_IF1DATA.bit.Data_3 = garrTXMsgData[index];
                break;
            case 4:
                CanbRegs.CAN_IF1DATB.bit.Data_4 = garrTXMsgData[index];
                break;
            case 5:
                CanbRegs.CAN_IF1DATB.bit.Data_5 = garrTXMsgData[index];
                break;
            case 6:
                CanbRegs.CAN_IF1DATB.bit.Data_6 = garrTXMsgData[index];
                break;
            case 7:
                CanbRegs.CAN_IF1DATB.bit.Data_7 = garrTXMsgData[index];
                break;
        }
    }

    //
    // Set Direction to write and set DATA-A/DATA-B to be transfered to
    // message object
    //
    CAN_IF1CMD_SHADOW.all = 0;
    CAN_IF1CMD_SHADOW.bit.DIR = 1;
    CAN_IF1CMD_SHADOW.bit.DATA_A = 1;
    CAN_IF1CMD_SHADOW.bit.DATA_B = 1;

    //
    // Set Tx Request Bit
    //
    CAN_IF1CMD_SHADOW.bit.TXRQST = 1;

    //
    // Transfer the message object to the message object specified by
    // objID.
    //
    CAN_IF1CMD_SHADOW.bit.MSG_NUM = objID;
    CanbRegs.CAN_IF1CMD.all = CAN_IF1CMD_SHADOW.all;
}


//
// getCANMessage - Check the message object for new data.
//                 If new data, data written into array and return true.
//                 If no new data, return false.
//
bool getCANAMessage(uint32_t objID)
{
    bool status;

    //
    // Use Shadow variable for IF2CMD. IF2CMD should be written to in
    // single 32-bit write.
    //
    union CAN_IF2CMD_REG CAN_IF2CMD_SHADOW;

    //
    // Set the Message Data A, Data B, and control values to be read
    // on request for data from the message object.
    //
    CAN_IF2CMD_SHADOW.all = 0;
    CAN_IF2CMD_SHADOW.bit.Control = 1;
    CAN_IF2CMD_SHADOW.bit.DATA_A = 1;
    CAN_IF2CMD_SHADOW.bit.DATA_B = 1;

    //
    // Transfer the message object to the message object IF register.
    //
    CAN_IF2CMD_SHADOW.bit.MSG_NUM = objID;
    CanaRegs.CAN_IF2CMD.all = CAN_IF2CMD_SHADOW.all;

    //
    // Wait for busy bit to clear.
    //
    while(CanaRegs.CAN_IF2CMD.bit.Busy)
    {
    }

    //
    // See if there is new data available.
    //
    if(CanaRegs.CAN_IF2MCTL.bit.NewDat == 1)
    {
        //
        // Read out the data from the CAN registers.
        //
        uint16_t index;
        for(index = 0; index < messageSize; index++)
        {
            switch(index)
            {
                case 0:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATA.bit.Data_0;
                break;
                case 1:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATA.bit.Data_1;
                break;
                case 2:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATA.bit.Data_2;
                break;
                case 3:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATA.bit.Data_3;
                break;
                case 4:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATB.bit.Data_4;
                break;
                case 5:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATB.bit.Data_5;
                break;
                case 6:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATB.bit.Data_6;
                break;
                case 7:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATB.bit.Data_7;
                break;
            }
        }

        //
        // Populate Shadow Variable
        //
        CAN_IF2CMD_SHADOW.all = CanaRegs.CAN_IF2CMD.all;

        //
        // Clear New Data Flag
        //
        CAN_IF2CMD_SHADOW.bit.TxRqst = 1;

        //
        // Transfer the message object to the message object IF register.
        //
        CAN_IF2CMD_SHADOW.bit.MSG_NUM = objID;
        CanaRegs.CAN_IF2CMD.all = CAN_IF2CMD_SHADOW.all;

        status = true;
    }
    else
    {
        status = false;
    }

    return(status);
}

//
// getCANMessage - Check the message object for new data.
//                 If new data, data written into array and return true.
//                 If no new data, return false.
//
bool getCANBMessage(uint32_t objID)
{
    bool status;

    //
    // Use Shadow variable for IF2CMD. IF2CMD should be written to in
    // single 32-bit write.
    //
    union CAN_IF2CMD_REG CAN_IF2CMD_SHADOW;

    //
    // Set the Message Data A, Data B, and control values to be read
    // on request for data from the message object.
    //
    CAN_IF2CMD_SHADOW.all = 0;
    CAN_IF2CMD_SHADOW.bit.Control = 1;
    CAN_IF2CMD_SHADOW.bit.DATA_A = 1;
    CAN_IF2CMD_SHADOW.bit.DATA_B = 1;

    //
    // Transfer the message object to the message object IF register.
    //
    CAN_IF2CMD_SHADOW.bit.MSG_NUM = objID;
    CanbRegs.CAN_IF2CMD.all = CAN_IF2CMD_SHADOW.all;

    //
    // Wait for busy bit to clear.
    //
    while(CanbRegs.CAN_IF2CMD.bit.Busy)
    {
    }

    //
    // See if there is new data available.
    //
    if(CanbRegs.CAN_IF2MCTL.bit.NewDat == 1)
    {
        //
        // Read out the data from the CAN registers.
        //
        uint16_t index;
        for(index = 0; index < messageSize; index++)
        {
            switch(index)
            {
                case 0:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATA.bit.Data_0;
                break;
                case 1:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATA.bit.Data_1;
                break;
                case 2:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATA.bit.Data_2;
                break;
                case 3:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATA.bit.Data_3;
                break;
                case 4:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATB.bit.Data_4;
                break;
                case 5:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATB.bit.Data_5;
                break;
                case 6:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATB.bit.Data_6;
                break;
                case 7:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATB.bit.Data_7;
                break;
            }
        }

        //
        // Populate Shadow Variable
        //
        CAN_IF2CMD_SHADOW.all = CanbRegs.CAN_IF2CMD.all;

        //
        // Clear New Data Flag
        //
        CAN_IF2CMD_SHADOW.bit.TxRqst = 1;

        //
        // Transfer the message object to the message object IF register.
        //
        CAN_IF2CMD_SHADOW.bit.MSG_NUM = objID;
        CanbRegs.CAN_IF2CMD.all = CAN_IF2CMD_SHADOW.all;

        status = true;
    }
    else
    {
        status = false;
    }

    return(status);
}
```

### `driver/can.h`

```c
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TXCOUNT  100
//#define CAN_MSG_ID              0x111
#define CAN_MSG_ID              0x18FF1234
#define CAN_MSG_REC_ID          0x1800C4A0
#define CAN_TX_MSG_OBJ          1
#define CANA_RX_MSG_OBJ         2
#define CANB_RX_MSG_OBJ         3
#define CANA_BASE               0x00048000
#define CANB_BASE               0x0004A000
#define CAN_MAX_BIT_DIVISOR     (13)   // The maximum CAN bit timing divisor
#define CAN_MIN_BIT_DIVISOR     (5)    // The minimum CAN bit timing divisor
#define CAN_MAX_PRE_DIVISOR     (1024) // The maximum CAN pre-divisor
#define CAN_MIN_PRE_DIVISOR     (1)    // The minimum CAN pre-divisor
#define CAN_BTR_BRP_M           (0x3F)
#define CAN_BTR_BRPE_M          (0xF0000)
#define CAN_MSG_ID_SHIFT        0U

void CanInit(void);
void CANTransmitMsg(void);
void sendCANAMessage(uint32_t objID);
void sendCANBMessage(uint32_t objID);
bool getCANAMessage(uint32_t objID);
bool getCANBMessage(uint32_t objID);

extern unsigned char garrTXMsgData[8]; // TX Data
extern unsigned char garrCanaRXMsgData[8];
extern unsigned char garrCanbRXMsgData[8];
#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif
```

### `driver/cap.c`

```c
/*
 * cap.c
 *
 *  Created on: 2026��1��13��
 *      Author: Nova006348
 */

#include <cap.h>

volatile struct ECAP_REGS *ECap[] =
{
    0,&ECap1Regs, &ECap2Regs, &ECap3Regs, &ECap4Regs, &ECap5Regs, &ECap6Regs
};

/**
* @brief: InitEcapAsTimer
* @param: void
* @return:Nothing
* @note:��������Ҫ����CPU�ж�ʱ�����
*/
static void _InitEcapAsTimer(uint16_t num)
{
    //ʹ��eCAP1ʱ��
//    CpuSysRegs.PCLKCR3.bit.ECAP1 = 1;
    (*ECap[num]).ECCTL2.bit.TSCTRSTOP = 0;     // ȷ��������ֹͣ
    (*ECap[num]).ECCTL1.bit.CAPLDEN = 0;       // ���ò���Ĵ������أ���ʱ��ģʽ����Ҫ��
    (*ECap[num]).ECEINT.all = 0x0000;          // �������в����ж�
    (*ECap[num]).ECCLR.all = 0xFFFF;           // ��������жϱ�־
    //����Ϊ�������еĶ�ʱ���������ǲ���ģʽ��
    (*ECap[num]).ECCTL2.bit.CONT_ONESHT = 0;   // ����ģʽ
    (*ECap[num]).ECCTL2.bit.STOP_WRAP = 0;     //�������ﵽ����ֵ(CAP1)����㣨Wrap��
    (*ECap[num]).ECCTL2.bit.SYNCI_EN = 0;      //�����ⲿͬ�����루��ʱ���������У�
    (*ECap[num]).ECCTL2.bit.SYNCO_SEL = 2;     // ����ͬ�����
    (*ECap[num]).ECCTL2.bit.REARM = 1;         // ����ʹ�����أ�������ģʽ��Ҳ����
    //�ر����в�����ؼ��Ͳ�ָ�λ
    (*ECap[num]).ECCTL1.bit.CAP1POL = 0;
    (*ECap[num]).ECCTL1.bit.CAP2POL = 0;
    (*ECap[num]).ECCTL1.bit.CAP3POL = 0;
    (*ECap[num]).ECCTL1.bit.CAP4POL = 0;
    (*ECap[num]).ECCTL1.bit.CTRRST1 = 0;       // �ر��¼�1�ļ�������λ���ǲ��ģʽ��
    (*ECap[num]).ECCTL1.bit.CTRRST2 = 0;       // �ر��¼�2�ļ�������λ
    (*ECap[num]).ECCTL1.bit.CTRRST3 = 0;       // �ر��¼�3�ļ�������λ
    (*ECap[num]).ECCTL1.bit.CTRRST4 = 0;       // �ر��¼�4�ļ�������λ
    //���ü��������ڣ�����ֵ��Ϊ���ֵ���Ի����ļ������� ��CAP1�Ĵ����ڶ�ʱ��ģʽ����Ϊ���ڼĴ���ʹ��
    (*ECap[num]).CAP1 = 0xFFFFFFFF;            // ��������0��������ֵ�����
    //���������
    (*ECap[num]).ECCTL2.bit.TSCTRSTOP = 1;
    //����Ҫ���ò���Ĵ������غͲ����ж�
    // (*ECap[num]).ECCTL1.bit.CAPLDEN = 1;
    // (*ECap[num]).ECEINT.bit.CEVT4 = 1;
}

/**
* @brief: Cap_Init
* @param: void
* @return:Nothing
* @note:CAP��ʼ��
*/
void Cap_Init(void)
{
#ifdef EN_CLA_CONTROL_INT_TIME_MEASURE
    _InitEcapAsTimer(1);
#endif
}
```

### `driver/cap.h`

```c
/*
 * cap.h
 *
 *  Created on: 2026��1��13��
 *      Author: Nova006348
 */

#ifndef DRIVER_CAP_H_
#define DRIVER_CAP_H_

#include "F28x_Project.h"
#include "system.h"

#define EN_CLA_CONTROL_INT_TIME_MEASURE     (1)

void Cap_Init(void);
#endif /* DRIVER_CAP_H_ */
```

### `driver/cpu_timers.c`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        cpu_timers.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


//
// Included Files
//
#include "cpu_timers.h"
#include "F28x_Project.h"
//
// Function Prototypes
//
__interrupt void cpu_timer0_isr(void);
__interrupt void cpu_timer1_isr(void);
__interrupt void cpu_timer2_isr(void);

static TimerState_t _gTimer;
//
// cpu_timer0_isr - CPU Timer0 ISR with interrupt counter
//
__interrupt void cpu_timer0_isr(void)
{
    CpuTimer0.InterruptCount++;

    //
    // Acknowledge this interrupt to receive more interrupts from group 1
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

//
// cpu_timer1_isr - CPU Timer1 ISR
//
#ifdef EN_CLA_RUN_TIME_MEASURE

#else
__interrupt void cpu_timer1_isr(void)
{
    CpuTimer1.InterruptCount++;
}
#endif

//
// cpu_timer2_isr CPU Timer2 ISR
//
__interrupt void cpu_timer2_isr(void)
{
    CpuTimer2.InterruptCount++;
//    ServiceDog();
}

uint32_t TimGetSystime(void)
{
    return CpuTimer2.InterruptCount;
}

/**
  * @brief  Calculate the difference of two time values.
  * This function will handle overflow problem.
  * @param  time[IN]: First time value to calculate
  * @param  new_time[IN]: 2nd time value to calculate
  * @return Time difference of 2 time values.
  */
uint32_t TimDifftime(uint32_t time, uint32_t new_time)
{
    uint32_t diffTime = 0;
    if (new_time >= time) /* not overflow */
    {
        diffTime = (new_time - time);
    }
    else
    {
        diffTime = new_time + (0xffffffffUL - time);
    }
    return (uint32_t)(diffTime);
}

/**
  * @brief  Calculate the time difference from now.
  * @param  time[IN]: Previous recorded time value
  * @return Time difference.
  */
uint32_t TimDifftimeFromNow(uint32_t time)
{
    return TimDifftime(time, CpuTimer2.InterruptCount);
}

void CpuTimerFlag(void)
{
    if(TimDifftimeFromNow(_gTimer.Time) >= 1) //1ms��ʱ��
    {
        _gTimer.Timer_1ms_Count++;
        _gTimer.Timer_5ms_Count++;
        _gTimer.Timer_10ms_Count++;
        _gTimer.Timer_20ms_Count++;
        _gTimer.Timer_100ms_Count++;
        _gTimer.Timer_500ms_Count++;
        _gTimer.Timer_1000ms_Count++;

        if(1)
        {
            _gTimer.Timer_1ms_Flag = 1;
            _gTimer.Timer_1ms_Count = 0;
        }
        if(_gTimer.Timer_5ms_Count >= 5)
        {
            _gTimer.Timer_5ms_Flag = 1;
            _gTimer.Timer_5ms_Count = 0;
        }
        if(_gTimer.Timer_10ms_Count >= 10)
        {
            _gTimer.Timer_10ms_Flag = 1;
            _gTimer.Timer_10ms_Count = 0;
        }
        if(_gTimer.Timer_20ms_Count >= 20)
        {
            _gTimer.Timer_20ms_Flag = 1;
            _gTimer.Timer_20ms_Count = 0;
        }
        if(_gTimer.Timer_100ms_Count >= 100)
        {
            _gTimer.Timer_100ms_Flag = 1;
            _gTimer.Timer_100ms_Count = 0;
        }
        if(_gTimer.Timer_500ms_Count >= 500)
        {
            _gTimer.Timer_500ms_Flag = 1;
            _gTimer.Timer_500ms_Count = 0;
        }
        if(_gTimer.Timer_1000ms_Count >= 1000)
        {
            _gTimer.Timer_1000ms_Flag = 1;
            _gTimer.Timer_1000ms_Count = 0;
        }

        _gTimer.Time = TimGetSystime();
    }
}
uint16_t Get_1ms_Flag(void)
{
    return (_gTimer.Timer_1ms_Flag);
}
uint16_t Get_5ms_Flag(void)
{
    return (_gTimer.Timer_5ms_Flag);
}
uint16_t Get_10ms_Flag(void)
{
    return (_gTimer.Timer_10ms_Flag);
}
uint16_t Get_20ms_Flag(void)
{
    return (_gTimer.Timer_20ms_Flag);
}
uint16_t Get_100ms_Flag(void)
{
    return (_gTimer.Timer_100ms_Flag);
}
uint16_t Get_500ms_Flag(void)
{
    return (_gTimer.Timer_500ms_Flag);
}
uint16_t Get_1s_Flag(void)
{
    return (_gTimer.Timer_1000ms_Flag);
}
void Timers_1ms(void)
{
    _gTimer.Timer_1ms_Flag = 0;
}
void Timers_5ms(void)
{
    _gTimer.Timer_5ms_Flag = 0;
}
void Timers_10ms(void)
{
    _gTimer.Timer_10ms_Flag = 0;
}
void Timers_20ms(void)
{
    _gTimer.Timer_20ms_Flag = 0;
}
void Timers_100ms(void)
{
    _gTimer.Timer_100ms_Flag = 0;
}
void Timers_500ms(void)
{
    _gTimer.Timer_500ms_Flag = 0;
}

void Timers_1s(void)
{
    _gTimer.Timer_1000ms_Flag = 0;
}

void CpuTimer_Init(void)
{
//
// Interrupts that are used in this example are re-mapped to
// ISR functions found within this file.
//
    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
#ifdef EN_CLA_RUN_TIME_MEASURE
#else
    PieVectTable.TIMER1_INT = &cpu_timer1_isr;
#endif

    PieVectTable.TIMER2_INT = &cpu_timer2_isr;
    EDIS;    // This is needed to disable write to EALLOW protected registers

//
// Step 4. Initialize the Device Peripheral. This function can be
//         found in F2837xD_CpuTimers.c
//
    InitCpuTimers();   // For this example, only initialize the Cpu Timers

//
// Configure CPU-Timer 0, 1, and 2 to interrupt every second:
// 200MHz CPU Freq, 1 second Period (in uSeconds)
//
    ConfigCpuTimer(&CpuTimer0, 200, 1000000);
#ifdef EN_CLA_RUN_TIME_MEASURE
    ConfigCpuTimer(&CpuTimer1, 200, 0xFFFFFFFF);
#else
    ConfigCpuTimer(&CpuTimer1, 200, 1000000);
#endif

    ConfigCpuTimer(&CpuTimer2, 200, 1000);

//
// To ensure precise timing, use write-only instructions to write to the
// entire register. Therefore, if any of the configuration bits are changed in
// ConfigCpuTimer and InitCpuTimers (in F2837xD_cputimervars.h), the below
// settings must also be updated.
//
    CpuTimer0Regs.TCR.all = 0x4000;
#ifdef EN_CLA_RUN_TIME_MEASURE
    CpuTimer1Regs.TCR.all = 0x0000;
#else
    CpuTimer1Regs.TCR.all = 0x4000;
#endif

    CpuTimer2Regs.TCR.all = 0x4000;

//
// Step 5. User specific code, enable interrupts:
//

//
// Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
// which is connected to CPU-Timer 1, and CPU int 14, which is connected
// to CPU-Timer 2:
//
    IER |= M_INT1;
    IER |= M_INT13;
    IER |= M_INT14;

//
// Enable TINT0 in the PIE: Group 1 interrupt 7
//
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
}
```

### `driver/cpu_timers.h`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        cpu_timers.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __CPU_TIMERS_H__
#define __CPU_TIMERS_H__
#include "F28x_Project.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EN_CLA_RUN_TIME_MEASURE (0)

/**
 * @brief ��ʱ��״̬�ṹ��
 */
typedef struct timer_state_stru_t
{
    uint32_t Time;                  //����
    uint16_t Timer_1ms_Count;       //��ʱ��1ms
    uint16_t Timer_5ms_Count;       //��ʱ��5ms
    uint16_t Timer_10ms_Count;      //��ʱ��10ms
    uint16_t Timer_20ms_Count;      //��ʱ��20ms
    uint16_t Timer_100ms_Count;     //��ʱ��100ms
    uint16_t Timer_500ms_Count;
    uint16_t Timer_1000ms_Count;    //��ʱ��1000ms

    uint16_t Timer_1ms_Flag;        //��ʱ��1ms��־λ ��1��Ч
    uint16_t Timer_5ms_Flag;        //��ʱ��5ms��־λ ��1��Ч
    uint16_t Timer_10ms_Flag;       //��ʱ��10ms��־λ ��1��Ч
    uint16_t Timer_20ms_Flag;       //��ʱ��20ms��־λ ��1��Ч
    uint16_t Timer_100ms_Flag;      //��ʱ��100ms��־λ ��1��Ч
    uint16_t Timer_500ms_Flag;      //��ʱ��100ms��־λ ��1��Ч
    uint16_t Timer_1000ms_Flag;     //��ʱ��1000ms��־λ ��1��Ч
} TimerState_t;

void CpuTimer_Init(void);
uint32_t TimGetSystime(void);
uint32_t TimDifftimeFromNow(uint32_t time);
void CpuTimerFlag(void);
uint32_t TimDifftime(uint32_t time, uint32_t new_time);
uint16_t Get_1ms_Flag(void);
uint16_t Get_5ms_Flag(void);
uint16_t Get_10ms_Flag(void);
uint16_t Get_20ms_Flag(void);
uint16_t Get_100ms_Flag(void);
uint16_t Get_500ms_Flag(void);
uint16_t Get_1s_Flag(void);

void Timers_1ms(void);
void Timers_5ms(void);
void Timers_10ms(void);
void Timers_20ms(void);
void Timers_100ms(void);
void Timers_500ms(void);
void Timers_1s(void);

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================
```

### `driver/dual_core.c`

```c
/*
 * dual_core.c
 *
 * CPU1: CPUSEL/GS ownership, CPU2 boot (no InitIpc), IPC handshake.
 */
#include "dual_core.h"
#include "F28x_Project.h"
#include "F2837xD_Ipc_drivers.h"
#include "F2837xD_Examples.h"

static bool s_cpu2PowerOnOnline = false;  /* CPU2开机在线标志 */
static bool s_cpu2RunOnline = false;    /* CPU2运行在线标志 */
static uint32_t s_cpu2RunCheckLastValue = 0;
static uint8_t s_cpu2RunUnchangedCnt = 0U;

static bool DualCore_WaitCondition(bool (*cond)(void), uint32_t timeoutMs)
{
    uint32_t elapsedMs = 0U;

    while (cond() == false)
    {
        DELAY_MS(1U);
        elapsedMs++;
        if (elapsedMs >= timeoutMs)
        {
            return false;
        }
    }

    return true;
}

static bool DualCore_IsBootSystemReady(void)
{
    return ((IPCGetBootStatus() & 0x0000000FU) == C2_BOOTROM_BOOTSTS_SYSTEM_READY);
}

static bool DualCore_AreBootIpcFlagsFree(void)
{
    return (IPCLtoRFlagBusy(IPC_FLAG0) == 0U) &&
           (IPCLtoRFlagBusy(IPC_FLAG31) == 0U);
}

void DualCore_ConfigCpuSel(void)
{
    EALLOW;

    DevCfgRegs.CPUSEL0.all  = 0U;
    DevCfgRegs.CPUSEL1.all  = 0U;
    DevCfgRegs.CPUSEL2.all  = 0U;
    DevCfgRegs.CPUSEL4.all  = 0U;
    DevCfgRegs.CPUSEL5.all  = 0U;
    DevCfgRegs.CPUSEL6.all  = 0U;
    DevCfgRegs.CPUSEL7.all  = 0U;
    DevCfgRegs.CPUSEL8.all  = 0U;
    DevCfgRegs.CPUSEL9.all  = 0U;
    DevCfgRegs.CPUSEL11.all = 0U;
    DevCfgRegs.CPUSEL12.all = 0U;
    DevCfgRegs.CPUSEL14.all = 0U;

    EDIS;
}

void DualCore_ConfigGsRamMaster(void)
{
    EALLOW;

    MemCfgRegs.GSxMSEL.all = (GS0_ACCESS | GS1_ACCESS | GS2_ACCESS |
                               GS4_ACCESS);

    MemCfgRegs.GSxMSEL.bit.MSEL_GS5  = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS6  = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS7  = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS8  = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS9  = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS10 = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS11 = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS12 = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS13 = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS14 = 0;
    MemCfgRegs.GSxMSEL.bit.MSEL_GS15 = 0;

    EDIS;
}

static void DualCore_ConfigGsRamAccessProtect(void)
{
    EALLOW;

    MemCfgRegs.GSxACCPROT0.bit.CPUWRPROT_GS3 = 0;
    MemCfgRegs.GSxACCPROT0.bit.DMAWRPROT_GS3 = 1;
    MemCfgRegs.GSxACCPROT1.bit.CPUWRPROT_GS4 = 0;
    MemCfgRegs.GSxACCPROT1.bit.DMAWRPROT_GS4 = 1;

    EDIS;
}

void DualCore_InitEarly(void)
{
    DualCore_ConfigCpuSel();
    DualCore_ConfigGsRamMaster();
    DualCore_ConfigGsRamAccessProtect();
    s_cpu2PowerOnOnline = false;  /* CPU2开机在线标志 */
    s_cpu2RunOnline = true;    /* CPU2运行在线标志 */
}

DualCore_Status_t DualCore_BootCpu2WithTimeout(uint32_t timeoutMs)
{
    uint32_t bootStatus;

    bootStatus = IPCGetBootStatus() & 0x0000000FU;
    if (bootStatus == C2_BOOTROM_BOOTSTS_C2TOC1_BOOT_CMD_ACK)
    {
        return DUAL_CORE_ERR_ALREADY_BOOTED;
    }

    if (DualCore_WaitCondition(DualCore_IsBootSystemReady, timeoutMs) == false)
    {
        return DUAL_CORE_ERR_SYSTEM_NOT_READY;
    }

    if (DualCore_WaitCondition(DualCore_AreBootIpcFlagsFree, timeoutMs) == false)
    {
        return DUAL_CORE_ERR_IPC_BUSY;
    }

    if (IPCBootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH) != STATUS_PASS)
    {
        return DUAL_CORE_ERR_INVALID_MODE;
    }

    return DUAL_CORE_OK;
}

DualCore_Status_t DualCore_HandshakeCpu1(uint32_t timeoutMs)
{
    uint32_t elapsedMs = 0U;

    IPCLtoRFlagSet(IPC_DUAL_FLAG_CPU1_READY);

    while (IPCLtoRFlagBusy(IPC_DUAL_FLAG_CPU1_READY) != 0U)
    {
        DELAY_MS(1U);
        elapsedMs++;
        if (elapsedMs >= timeoutMs)
        {
            return DUAL_CORE_ERR_CPU2_NOT_ALIVE;
        }
    }

    elapsedMs = 0U;
    while (IPCRtoLFlagBusy(IPC_DUAL_FLAG_CPU2_READY) == 0U)
    {
        DELAY_MS(1U);
        elapsedMs++;
        if (elapsedMs >= timeoutMs)
        {
            return DUAL_CORE_ERR_CPU2_NOT_ALIVE;
        }
    }

    IPCRtoLFlagAcknowledge(IPC_DUAL_FLAG_CPU2_READY);
    s_cpu2PowerOnOnline = true;

    return DUAL_CORE_OK;
}

bool DualCore_IsCpu2PowerOnOnline(void)
{
    return s_cpu2PowerOnOnline;
}

void DualCore_SetCpu2PowerOnOnline(bool online)
{
    s_cpu2PowerOnOnline = online;
}

bool DualCore_IsCpu2RunOnline(void)
{
    return s_cpu2RunOnline;
}

void DualCore_CheckCpu2RunOnline(uint32_t value)
{
    if (value != s_cpu2RunCheckLastValue)
    {
        s_cpu2RunOnline = true;
        s_cpu2RunUnchangedCnt = 0U;
    }
    else
    {
        s_cpu2RunUnchangedCnt++;
        if (s_cpu2RunUnchangedCnt >= 2U)
        {
            s_cpu2RunOnline = false;
        }
    }

    s_cpu2RunCheckLastValue = value;
}

bool DualCore_TriggerCpu2Event(void)
{
    if (s_cpu2PowerOnOnline == false)
    {
        return false;
    }

    if (IPCLtoRFlagBusy(IPC_DUAL_FLAG_CPU1_EVENT) != 0U)
    {
        return false;
    }

    IPCLtoRFlagSet(IPC_DUAL_FLAG_CPU1_EVENT);
    return true;
}
```

### `driver/dual_core.h`

```c
/*
 * dual_core.h
 *
 * CPU1 dual-core resource ownership and CPU2 boot/handshake (TI dma_ex1 / ram_ex1 style).
 */
#ifndef DRIVER_DUAL_CORE_H_
#define DRIVER_DUAL_CORE_H_

#include <stdbool.h>
#include <stdint.h>

/* IPC flags for startup barrier (FLAG4/5 avoid PIE IPC0~3) */
#define IPC_DUAL_FLAG_CPU1_READY    0x00000010U  /* IPC_FLAG4: CPU1 peripherals ready */
#define IPC_DUAL_FLAG_CPU2_READY    0x00000020U  /* IPC_FLAG5: CPU2 init ready */
#define IPC_DUAL_FLAG_CPU1_EVENT    0x00000001U  /* IPC_FLAG0: CPU1 event interrupt to CPU2 */
#define IPC_DUAL_GS_CPU2_MASK       0x00000017U  /* GS0|GS1|GS2|GS4 master -> CPU2 */
#define IPC_DUAL_BOOT_TIMEOUT_MS         1000U
#define IPC_DUAL_HANDSHAKE_TIMEOUT_MS    3000U

typedef enum
{
    DUAL_CORE_OK = 0,
    DUAL_CORE_ERR_ALREADY_BOOTED,
    DUAL_CORE_ERR_INVALID_MODE,
    DUAL_CORE_ERR_SYSTEM_NOT_READY,
    DUAL_CORE_ERR_IPC_BUSY,
    DUAL_CORE_ERR_CPU2_NOT_ALIVE
} DualCore_Status_t;

void DualCore_ConfigCpuSel(void);
void DualCore_ConfigGsRamMaster(void);
void DualCore_InitEarly(void);

DualCore_Status_t DualCore_BootCpu2WithTimeout(uint32_t timeoutMs);
DualCore_Status_t DualCore_HandshakeCpu1(uint32_t timeoutMs);

bool DualCore_IsCpu2PowerOnOnline(void);
void DualCore_SetCpu2PowerOnOnline(bool online);

bool DualCore_IsCpu2RunOnline(void);
void DualCore_CheckCpu2RunOnline(uint32_t value);

bool DualCore_TriggerCpu2Event(void);

#endif /* DRIVER_DUAL_CORE_H_ */
```

### `driver/gpio.c`

```c
/*
 * gpio.c
 *
 *  Created on: 2025��1��6��
 *      Author: Nova007456
 */
#include "F28x_Project.h"
#include "system.h"
#include "gpio.h"

/**
* @brief: LedGpioToggle
* @param: void                        
* @note: 
*
*/
void LedGpioToggle(void)
{
    GPIO_WritePin(LED_GPIO, GPIO_TOGGLE);
}

/**
* @brief: SetLedGpioState
* @param: void                        
* @note: 
*
*/
void SetLedGpioState(uint8_t value)
{
    if(value)
    {
        GPIO_WritePin(LED_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(LED_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetLedGpioState
* @param: void
* @note:
*
*/
void SetCpuTestGpioState(uint8_t value)
{
    if(value)
    {
        GPIO_WritePin(CPU_TEST_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(CPU_TEST_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetFlashCsGpioState
* @param: void
* @note:
*
*/
void SetFlashCsGpioState(uint8_t value)
{
    if(value)
    {
        GPIO_WritePin(FLASH_CS_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(FLASH_CS_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetEXCRlyState
* @param: void
* @note: ���ü̵������EXC״̬
*/
void SetEXCRlyState(uint16_t value)
{
    if(value == 1)//�̵����պ�
    {
        GPIO_WritePin(RY_EXC_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY_EXC_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetLoadRlyState
* @param: void
* @note: ���ø��ؼ̵���״̬
*/
void SetLoadRlyState(uint16_t value)
{
    if(value == 1)//�̵����պ�
    {
        GPIO_WritePin(RY_LOAD_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY_LOAD_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetInvMainRlyState
* @param: void
* @note: ����������̵���״̬
*/
void SetInvMainRlyState(uint16_t value)
{
    if(value == 1)//�̵����պ�
    {
        GPIO_WritePin(RY_INV_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY_INV_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetGridRlyState
* @param: void
* @note: �����е粢���̵���״̬
*/
void SetGridRlyState(uint16_t value)
{
    if(value == 1)//�̵����պ�
    {
        GPIO_WritePin(RY1_Grid_GPIO, GPIO_SET);
        GPIO_WritePin(RY2_Grid_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY1_Grid_GPIO, GPIO_CLEAR);
        GPIO_WritePin(RY2_Grid_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetGenRlyState
* @param: void
* @note: ���÷�����̵���״̬
*/
void SetGenRlyState(uint16_t value)
{
    if(value == 1)//�̵����պ�
    {
        GPIO_WritePin(RY_GEN_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY_GEN_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetPERlyState
* @param: void
* @note: ����N����PE���Ӽ̵���
*/
void SetPERlyState(uint16_t value)
{
    if(value == 1)//�̵����պ�
    {
        GPIO_WritePin(RY_N_PE_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY_N_PE_GPIO, GPIO_CLEAR);
    }
}

/**
* @brief: SetGenStarRlyState
* @param: void
* @note: ����GEN�Լ�״̬��Ĭ�Ͽ����Լ�
*/
void SetGenStarRlyState(uint16_t value)
{
    if(value == 1)
    {
        GPIO_WritePin(RY_GEN_START_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY_GEN_START_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetRcmuChkState
* @param: void
* @note: ����RCMU�Լ�״̬��Ĭ�Ͽ����Լ�
*/
void SetRcmuChkState(uint16_t value)
{
    if(value == 1)
    {
        GPIO_WritePin(RCMU_CHK_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RCMU_CHK_GPIO, GPIO_CLEAR);
    }
}

/**
* @brief: ARC_IO_Read
* @param: void
* @note: ����N����PE���Ӽ̵���
*/
uint16_t ArcIOStateRead(void)
{
    volatile uint16_t gpio_state;
    gpio_state = GPIO_ReadPin(ARC_STATE_GPIO);
    return gpio_state;
}
/**
* @brief: PoBootStateRead
* @param: void
* @note: ���ʰ�BOOT״̬��ȡ
*/

uint16_t PoBootStateRead(void)
{
    volatile uint16_t boot_id_state[6];
    boot_id_state[0] = GPIO_ReadPin(BOOT0_STATE_GPIO);
    boot_id_state[1] = GPIO_ReadPin(BOOT1_STATE_GPIO);
    boot_id_state[2] = GPIO_ReadPin(BOOT2_STATE_GPIO);
    boot_id_state[3] = GPIO_ReadPin(BOOT3_STATE_GPIO);
    boot_id_state[4] =  ((boot_id_state[3] & 0X01) << 3) | ((boot_id_state[2] & 0X01) << 2) | ((boot_id_state[1] & 0X01) << 1) | (boot_id_state[0] && 0X01);
    return boot_id_state[4];
}
/**
* @brief: _AdcGpioSwitchInit
* @param: void
* @note:ADC��GPIO�л���ʼ��
*/
static void _AdcGpioSwitchInit(void)
{
    EALLOW;
    /*      ADC   PV��GPIO����*/
    GpioCtrlRegs.GPBPUD.bit.GPIO34    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPBMUX1.bit.GPIO34   = 0;//����ΪGPIO
    GpioCtrlRegs.GPBCSEL1.bit.GPIO34  = 0;//�����CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO34    = 1;//����Ϊ�������
    GpioDataRegs.GPBCLEAR.bit.GPIO34  = 1;//����Ϊ�͵�ƽ

    GpioCtrlRegs.GPBPUD.bit.GPIO35    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPBMUX1.bit.GPIO35   = 0;//����ΪGPIO
    GpioCtrlRegs.GPBCSEL1.bit.GPIO35  = 0;//�����CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO35    = 1;//����Ϊ�������
    GpioDataRegs.GPBCLEAR.bit.GPIO35  = 1;//����Ϊ�͵�ƽ
    /*      ADC  S0��S1�Ĳ���ͨ��GPIO   AC_BUS  */
    GpioCtrlRegs.GPBPUD.bit.GPIO40    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPBMUX1.bit.GPIO40   = 0;//����ΪGPIO
    GpioCtrlRegs.GPBCSEL2.bit.GPIO40  = 0;//�����CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO40    = 1;//����Ϊ�������
    GpioDataRegs.GPBCLEAR.bit.GPIO40  = 1;//����Ϊ�͵�ƽ

    GpioCtrlRegs.GPBPUD.bit.GPIO41    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPBMUX1.bit.GPIO41   = 0;//����ΪGPIO
    GpioCtrlRegs.GPBCSEL2.bit.GPIO41  = 0;//�����CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO41    = 1;//����Ϊ�������
    GpioDataRegs.GPBCLEAR.bit.GPIO41  = 1;//����Ϊ�͵�ƽ
    /*  �¶�  TempS0��TempS1��TempS2�Ĳ���ͨ��GPIO   */
    GpioCtrlRegs.GPAPUD.bit.GPIO25    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX2.bit.GPIO25   = 0;//����ΪGPIO
    GpioCtrlRegs.GPACSEL4.bit.GPIO25  = 0;//�����CPU1
    GpioCtrlRegs.GPADIR.bit.GPIO25    = 1;//����Ϊ�������
    GpioDataRegs.GPACLEAR.bit.GPIO25  = 1;//����Ϊ�͵�ƽ

    GpioCtrlRegs.GPAPUD.bit.GPIO26    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX2.bit.GPIO26   = 0;//����ΪGPIO
    GpioCtrlRegs.GPACSEL4.bit.GPIO26  = 0;//�����CPU1
    GpioCtrlRegs.GPADIR.bit.GPIO26    = 1;//����Ϊ�������
    GpioDataRegs.GPACLEAR.bit.GPIO26  = 1;//����Ϊ�͵�ƽ

    GpioCtrlRegs.GPAPUD.bit.GPIO27    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX2.bit.GPIO27   = 0;//����ΪGPIO
    GpioCtrlRegs.GPACSEL4.bit.GPIO27  = 0;//�����CPU1
    GpioCtrlRegs.GPADIR.bit.GPIO27    = 1;//����Ϊ�������
    GpioDataRegs.GPACLEAR.bit.GPIO27  = 1;//����Ϊ�͵�ƽ
    EDIS;
}
/**
* @brief: _InvGpioInit
* @param: void
* @return:Nothing
* @note:�����GPIO����
*/
static void _InvGpioInit(void)
{
    EALLOW;
    /*      ��� Q1�� Q3��    */
    GpioCtrlRegs.GPAPUD.bit.GPIO0   = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO0  = 1;//����ΪEPWM1A

    GpioCtrlRegs.GPAPUD.bit.GPIO1   = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO1  = 1;//����ΪEPWM1B

    /*      ��� Q2�� Q4��    */
    GpioCtrlRegs.GPAPUD.bit.GPIO2    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO2   = 1;//����ΪEPWM2A

    GpioCtrlRegs.GPAPUD.bit.GPIO3    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO3   = 1;//����ΪEPWM2B

    /*      ��� Q5�� Q7��    */
    GpioCtrlRegs.GPAPUD.bit.GPIO4    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO4   = 1;//����ΪEPWM3A

    GpioCtrlRegs.GPAPUD.bit.GPIO5    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO5   = 1;//����ΪEPWM3B

    /*      ��� Q6�� Q8��    */
    GpioCtrlRegs.GPAPUD.bit.GPIO6    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO6   = 1;//����ΪEPWM4A

    GpioCtrlRegs.GPAPUD.bit.GPIO7    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO7   = 1;//����ΪEPWM4B

    /*      ��� Q9�� Q11��    */
    GpioCtrlRegs.GPAPUD.bit.GPIO8    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO8   = 1;//����ΪEPWM5A

    GpioCtrlRegs.GPAPUD.bit.GPIO9    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO9   = 1;//����ΪEPWM5B

    /*      ��� Q10�� Q12��    */
    GpioCtrlRegs.GPAPUD.bit.GPIO10   = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO10  = 1;//����ΪEPWM6A

    GpioCtrlRegs.GPAPUD.bit.GPIO11   = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO11  = 1;//����ΪEPWM6B

    EDIS;
}
/**
* @brief: _EpwmTZGpioInit
* @param: void
* @return:Nothing
* @note:EPWM��TZ��GPIO����
*/
static void _EpwmTZGpioInit(void)
{
    EALLOW;
    /*      ������TZ�Ⲩ    */
    GpioCtrlRegs.GPCPUD.bit.GPIO82 = 1;  //��ֹ�ڲ�����
    GpioCtrlRegs.GPCQSEL2.bit.GPIO82 = 3;//��ϵͳʱ�Ӳ�ͬ��
    InputXbarRegs.INPUT1SELECT = 82;     //����TZ1ΪGPIO82
    /*      ĸ�߹���TZ�Ⲩ    */
    GpioCtrlRegs.GPCPUD.bit.GPIO83 = 1;  //��ֹ�ڲ�����
    GpioCtrlRegs.GPCQSEL2.bit.GPIO83 = 3;//��ϵͳʱ�Ӳ�ͬ��
    InputXbarRegs.INPUT2SELECT = 83;     //����TZ2ΪGPIO83
    EDIS;
}
/**
* @brief: _DebugSciGpioInit
* @param: void
* @return:Nothing
* @note:���Ե�SCID��GPIO��ʼ������
*/
static void _DebugSciGpioInit(void)
{
    EALLOW;
    /*      SciD��GPIO����    */
    GpioCtrlRegs.GPCPUD.bit.GPIO76    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPCGMUX1.bit.GPIO76  = 1;//����ΪSCID TX����
    GpioCtrlRegs.GPCMUX1.bit.GPIO76   = 2;//����ΪSCID TX����
    GpioCtrlRegs.GPCCSEL2.bit.GPIO76  = 0;//�����CPU1

    GpioCtrlRegs.GPCPUD.bit.GPIO77    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPCGMUX1.bit.GPIO77  = 1;//����ΪSCID RX����
    GpioCtrlRegs.GPCMUX1.bit.GPIO77   = 2;//����ΪSCID RX����
    GpioCtrlRegs.GPCCSEL2.bit.GPIO77  = 0;//�����CPU1
    EDIS;
}
/**
* @brief: _LEDGpioInit
* @param: void
* @return:Nothing
* @note:���е�GPIO����
*/
static void _LEDGpioInit(void)
{
    GPIO_SetupPinMux(LED_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(LED_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

}
/**
* @brief: _ArcGpioInit
* @param: void
* @return:Nothing
* @note:�������GPIO����
*/
static void _ArcGpioInit(void)
{
    GPIO_SetupPinMux(ARC_STATE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(ARC_STATE_GPIO, GPIO_INPUT, 0);
}
/**
* @brief: _RelayGpioInit
* @param: void
* @return:Nothing
* @note:�̵���GPIO����
*/
static void _RelayGpioInit(void)
{
    EALLOW;
    /*Relay_Total��GPIO����*/
    GPIO_SetupPinMux(RY_INV_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY_INV_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY_INV_GPIO, GPIO_CLEAR);
    /*Relay_BackUP��GPIO����*/
    GPIO_SetupPinMux(RY_LOAD_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY_LOAD_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY_LOAD_GPIO, GPIO_CLEAR);
    /*Relay_GRID��GPIO����*/
    GPIO_SetupPinMux(RY1_Grid_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY1_Grid_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(RY2_Grid_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY2_Grid_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY1_Grid_GPIO, GPIO_CLEAR);
    GPIO_WritePin(RY2_Grid_GPIO, GPIO_CLEAR);
    /*Relay_GEN��GPIO����*/
    GPIO_SetupPinMux(RY_GEN_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY_GEN_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY_GEN_GPIO, GPIO_CLEAR);
    /*RY_N_PE_GPIO��GPIO����*/
    GPIO_SetupPinMux(RY_N_PE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY_N_PE_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY_N_PE_GPIO, GPIO_CLEAR);
    /*RY_EXC_CON��GPIO����*/
    GPIO_SetupPinMux(RY_EXC_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY_EXC_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY_EXC_GPIO, GPIO_SET);
    /*Relay_GEN_Start��GPIO����*/
    GPIO_SetupPinMux(RY_GEN_START_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY_GEN_START_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY_GEN_START_GPIO, GPIO_SET);
    EDIS;
}
/**
* @brief: _PvGpioInit
* @param: void
* @return:Nothing
* @note:���GPIO����
*/
static void _PvGpioInit(void)
{
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO14   = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO14  = 1;//����ΪEPWM8A

    GpioCtrlRegs.GPAPUD.bit.GPIO15   = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX1.bit.GPIO15  = 1;//����ΪEPWM8B

    GpioCtrlRegs.GPAPUD.bit.GPIO16    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX2.bit.GPIO16   = 1;//����ΪEPWM9A
    GpioCtrlRegs.GPAGMUX2.bit.GPIO16  = 1;//����ΪEPWM9A

    GpioCtrlRegs.GPAPUD.bit.GPIO17    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPAMUX2.bit.GPIO17   = 1;//����ΪEPWM9B
    GpioCtrlRegs.GPAGMUX2.bit.GPIO17  = 1;//����ΪEPWM9B
    EDIS;
}
/**
* @brief: _CH390GpioInit
* @param: void
* @return:Nothing
* @note:CH390��GPIO����
*/
static void _CH390GpioInit(void)
{
    EALLOW;
    /*      SPIB��GPIO����    */
    GpioCtrlRegs.GPBPUD.bit.GPIO63 = 1;//ʹ���ڲ�����
    GpioCtrlRegs.GPCPUD.bit.GPIO64 = 1;//ʹ���ڲ�����
    GpioCtrlRegs.GPCPUD.bit.GPIO65 = 1;//ʹ���ڲ�����

    GpioCtrlRegs.GPBQSEL2.bit.GPIO63 = 3;//����Ϊ����ʱ�ӣ���ϵͳʱ�Ӳ�ͬ��
    GpioCtrlRegs.GPCQSEL1.bit.GPIO64 = 3;//����Ϊ����ʱ�ӣ���ϵͳʱ�Ӳ�ͬ��
    GpioCtrlRegs.GPCQSEL1.bit.GPIO65 = 3;//����Ϊ����ʱ�ӣ���ϵͳʱ�Ӳ�ͬ��

    GpioCtrlRegs.GPBGMUX2.bit.GPIO63 = 3; //����ΪSIMO
    GpioCtrlRegs.GPCGMUX1.bit.GPIO64 = 3; //����ΪSOMI
    GpioCtrlRegs.GPCGMUX1.bit.GPIO65 = 3; //����ΪSPI��CLK

    GpioCtrlRegs.GPBMUX2.bit.GPIO63 = 3; //����ΪSIMO
    GpioCtrlRegs.GPCMUX1.bit.GPIO64 = 3; //����ΪSOMI
    GpioCtrlRegs.GPCMUX1.bit.GPIO65 = 3; //����ΪSPI��CLK

    /*      CH390_CS��GPIO66����    */
    GpioCtrlRegs.GPCPUD.bit.GPIO66    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPCMUX1.bit.GPIO66   = 0;//����ΪGPIO
    GpioCtrlRegs.GPCCSEL1.bit.GPIO66  = 0;//�����CPU1
    GpioCtrlRegs.GPCDIR.bit.GPIO66    = 1;//����Ϊ�������
    GpioDataRegs.GPCSET.bit.GPIO66 = 1;

    /*      CH390_RST��GPIO67����    */
    GpioCtrlRegs.GPCPUD.bit.GPIO67    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPCMUX1.bit.GPIO67   = 0;//����ΪGPIO
    GpioCtrlRegs.GPCCSEL1.bit.GPIO67  = 0;//�����CPU1
    GpioCtrlRegs.GPCDIR.bit.GPIO67    = 1;//����Ϊ�������
    GpioDataRegs.GPCSET.bit.GPIO67 = 1;

    /*      CH390_INT��GPIO68����    */
    GpioCtrlRegs.GPCPUD.bit.GPIO68    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPCMUX1.bit.GPIO68   = 0;//����ΪGPIO
    GpioCtrlRegs.GPCCSEL1.bit.GPIO68  = 0;//�����CPU1
    GpioCtrlRegs.GPCDIR.bit.GPIO68    = 0;//����Ϊ��������
    EDIS;
}

/**
* @brief: _FlashGpioInit
* @param: void
* @return:Nothing
* @note:Flash��GPIO����
*/
static void _FlashGpioInit(void)
{
    EALLOW;
    /*      SPIA��GPIO����    */
    GpioCtrlRegs.GPBPUD.bit.GPIO58 = 1;//ʹ���ڲ�����
    GpioCtrlRegs.GPBPUD.bit.GPIO59 = 1;//ʹ���ڲ�����
    GpioCtrlRegs.GPBPUD.bit.GPIO60 = 1;//ʹ���ڲ�����

    GpioCtrlRegs.GPBQSEL2.bit.GPIO58 = 3;//����Ϊ����ʱ�ӣ���ϵͳʱ�Ӳ�ͬ��
    GpioCtrlRegs.GPBQSEL2.bit.GPIO59 = 3;//����Ϊ����ʱ�ӣ���ϵͳʱ�Ӳ�ͬ��
    GpioCtrlRegs.GPBQSEL2.bit.GPIO60 = 3;//����Ϊ����ʱ�ӣ���ϵͳʱ�Ӳ�ͬ��

    GpioCtrlRegs.GPBGMUX2.bit.GPIO58 = 3; //����ΪSIMO
    GpioCtrlRegs.GPBGMUX2.bit.GPIO59 = 3; //����ΪSOMI
    GpioCtrlRegs.GPBGMUX2.bit.GPIO60 = 3; //����ΪSPI��CLK

    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 3; //����ΪSIMO
    GpioCtrlRegs.GPBMUX2.bit.GPIO59 = 3; //����ΪSOMI
    GpioCtrlRegs.GPBMUX2.bit.GPIO60 = 3; //����ΪSPI��CLK

    /*      Flash_CS��GPIO61����    */
    GpioCtrlRegs.GPBPUD.bit.GPIO61    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPBMUX2.bit.GPIO61   = 0;//����ΪGPIO
    GpioCtrlRegs.GPBCSEL4.bit.GPIO61  = 0;//�����CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO61    = 1;//����Ϊ�������
    GpioDataRegs.GPBSET.bit.GPIO61 = 1;
    EDIS;
}

/**
* @brief: _HC245GpioInit
* @param: void
* @return:Nothing
* @note:HC245��GPIO��ʼ������
*/
static void _HC245GpioInit(void)
{
    EALLOW;
    /*      HC245��ADDC  GPIO45����    */
    GpioCtrlRegs.GPBPUD.bit.GPIO45    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPBMUX1.bit.GPIO45   = 0;//����ΪGPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO45    = 1;//����Ϊ�������
    GpioDataRegs.GPBSET.bit.GPIO45    = 0;//�õ�
    GpioCtrlRegs.GPBCSEL2.bit.GPIO45  = 1;//�����CLA
    /*      HC245��PV_FAN  GPIO46����    */
    GpioCtrlRegs.GPBPUD.bit.GPIO46    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPBMUX1.bit.GPIO46   = 0;//����ΪGPIO
    GpioCtrlRegs.GPBCSEL2.bit.GPIO46  = 0;//�����CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO46    = 1;//����Ϊ�������
    GpioDataRegs.GPBSET.bit.GPIO46    = 1;//�ø�
    /*      HC245��Relay  GPIO47����    */
    GpioCtrlRegs.GPBPUD.bit.GPIO47    = 1;//��ֹ�ڲ�����
    GpioCtrlRegs.GPBMUX1.bit.GPIO47   = 0;//����ΪGPIO
    GpioCtrlRegs.GPBCSEL2.bit.GPIO47  = 0;//�����CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO47    = 1;//����Ϊ�������
    GpioDataRegs.GPBSET.bit.GPIO47    = 1;//�ø�
    EDIS;
}
/**
* @brief: _PwBootGpioInit
* @param: void
* @return:Nothing
* @note:���ʰ�BOOT_IO��ʼ������
*/
static void _PwBootGpioInit(void)
{
    EALLOW;
    /*      BOOT_ID��ʼ������    */
    GPIO_SetupPinMux(BOOT0_STATE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BOOT0_STATE_GPIO, GPIO_INPUT, 0);

    GPIO_SetupPinMux(BOOT1_STATE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BOOT1_STATE_GPIO, GPIO_INPUT, 0);

    GPIO_SetupPinMux(BOOT2_STATE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BOOT2_STATE_GPIO, GPIO_INPUT, 0);

    GPIO_SetupPinMux(BOOT3_STATE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BOOT3_STATE_GPIO, GPIO_INPUT, 0);
    EDIS;
}
/**
* @brief: _TestGpio_Init
* @param: void
* @return:Nothing
* @note:����GPIO��ʼ������
*/
static void _TestGpio_Init(void)
{
    EALLOW;
    /*      �������Ԥ��GPIO ��������������      */
//    GPIO_SetupPinMux(CPU_TEST_GPIO, GPIO_MUX_CPU1, 0);
//    GPIO_SetupPinOptions(CPU_TEST_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

    GPIO_SetupPinMux(CLA_TEST_GPIO, GPIO_MUX_CPU1CLA, 0);
    GPIO_SetupPinOptions(CLA_TEST_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

    EDIS;
}
/**
* @brief: _RcmuChkGpioInit
* @param: void
* @return:Nothing
* @note:RCMU�Լ�����
*/
static void _RcmuChkGpioInit(void)
{
    EALLOW;

    GPIO_SetupPinMux(RCMU_CHK_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RCMU_CHK_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    EDIS;
}

/**
* @brief: _FanGpioInit
* @param: void
* @return:Nothing
* @note:����GPIO����
*/
static void _FanGpioInit(void)
{
    EALLOW;

    /*�������GPIO����*/
    /*      EPWM10    */
    GPIO_SetupPinMux(FAN1_OUT_GPIO, GPIO_MUX_CPU1, 5); // ��������ѡ��1��ӦEPWM10A
    GPIO_SetupPinOptions(FAN1_OUT_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

//    GPIO_SetupPinMux(FAN2_OUT_GPIO, GPIO_MUX_CPU1, 5); // ��������ѡ��1��ӦEPWM10A
//    GPIO_SetupPinOptions(FAN2_OUT_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
//
//    /*      EPWM11    */
//    GPIO_SetupPinMux(FAN3_OUT_GPIO, GPIO_MUX_CPU1, 5); // ��������ѡ��1��ӦEPWM10A
//    GPIO_SetupPinOptions(FAN3_OUT_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
//
//    GPIO_SetupPinMux(FAN4_OUT_GPIO, GPIO_MUX_CPU1, 5); // ��������ѡ��1��ӦEPWM10A
//    GPIO_SetupPinOptions(FAN4_OUT_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

    /*���ȹ��ϼ��GPIO����*/
    GPIO_SetupPinMux(FAN1_CAP_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(FAN1_CAP_GPIO, GPIO_INPUT, 0);

//    GPIO_SetupPinMux(FAN2_CAP_GPIO, GPIO_MUX_CPU1, 0);
//    GPIO_SetupPinOptions(FAN2_CAP_GPIO, GPIO_INPUT, 0);
//
//    GPIO_SetupPinMux(FAN3_CAP_GPIO, GPIO_MUX_CPU1, 0);
//    GPIO_SetupPinOptions(FAN3_CAP_GPIO, GPIO_INPUT, 0);
//
//    GPIO_SetupPinMux(FAN4_CAP_GPIO, GPIO_MUX_CPU1, 0);
//    GPIO_SetupPinOptions(FAN4_CAP_GPIO, GPIO_INPUT, 0);
    EDIS;
}
/**
* @brief: Gpio_Init
* @param: void
* @return:Nothing
* @note:GPIO�����ܺ���(������������)
*/
void Gpio_Init(void)
{
    EALLOW;
    _InvGpioInit();         //���GPIO��ʼ��
    _DebugSciGpioInit();    //���Դ���GPIO��ʼ��
    _LEDGpioInit();         //ָʾ��GPIO����
    _RelayGpioInit();       //�̵���GPIO����
    _HC245GpioInit();      //HC245��GPIO���� EPWM�����ͼ̵���������
    _PvGpioInit();          //���GPIO����
    _CH390GpioInit();      //CH390��GPIO����
    _AdcGpioSwitchInit();    //ADC�����л�GPIO����
    _EpwmTZGpioInit();    //EPWM��TZ�����Ⲩ����
    _FlashGpioInit();      //Flash��Gpio����
    _TestGpio_Init();        //���Ե�GPIO����
    _ArcGpioInit();   /*����������IO����*/
    _RcmuChkGpioInit(); /*RCMU�Լ�GPIO����*/
    _PwBootGpioInit();
    _FanGpioInit();
    EDIS;
}
```

### `driver/gpio.h`

```c
/*
 * gpio.h
 *
 *  Created on: 2025��1��6��
 *      Author: Nova007456
 */

#ifndef DEVICE_GPIO_H_
#define DEVICE_GPIO_H_
#include "system.h"


#define LED_GPIO               (99)
#define FLASH_CS_GPIO          (61)


#define RY_LOAD_GPIO           (90)
#define RY_INV_GPIO            (87)
#define RY1_Grid_GPIO          (88)
#define RY2_Grid_GPIO          (89)
#define RY_GEN_GPIO            (92)
#define RY_N_PE_GPIO           (91)
#define RY_EXC_GPIO            (93)

#define RY_GEN_START_GPIO      (94)

#define RCMU_CHK_GPIO          (28)

#define ARC_STATE_GPIO         (54)


#define BOOT0_STATE_GPIO      (50)
#define BOOT1_STATE_GPIO      (51)
#define BOOT2_STATE_GPIO      (52)
#define BOOT3_STATE_GPIO      (53)

#define CPU_TEST_GPIO      (93)       /*��GPIO�����*/
#define CLA_TEST_GPIO      (33)

// ����ת�ټ��GPIO����
#define FAN1_CAP_GPIO           (80)
#define FAN2_CAP_GPIO           (81)
#define FAN3_CAP_GPIO           (52)
#define FAN4_CAP_GPIO           (53)
//�������GPIO����
#define FAN1_OUT_GPIO           (18)
#define FAN2_OUT_GPIO           (19)
#define FAN3_OUT_GPIO           (20)
#define FAN4_OUT_GPIO           (21)


void LedGpioToggle(void);
void SetLedGpioState(uint8_t value);

void SetFlashCsGpioState(uint8_t value);

void AdcGpioSwitch(EN_CPU_INTERRUPT_TIME vlaue);
void SetCpuTestGpioState(uint8_t value);

void SetLoadRlyState(uint16_t value);
void SetInvMainRlyState(uint16_t value);
void SetGridRlyState(uint16_t value);
void SetGenRlyState(uint16_t value);
void SetPERlyState(uint16_t value);
void Gpio_Init(void);
void SetEXCRlyState(uint16_t value);

uint16_t ArcIOStateRead(void);
void SetRcmuChkState(uint16_t value);
void SetGenStarRlyState(uint16_t value);
uint16_t PoBootStateRead(void);
#endif /* DEVICE_GPIO_H_ */
```

### `driver/interrupt.c`

```c
/*
 * interrupt.c
 *
 *  Created on: 2025年1月6日
 *      Author: Nova007456
 */
#include "interrupt.h"
#include "F28x_Project.h"
#include "F2837xD_Mcbsp.h"
// 静态变量保存上层注册的回调函数指针
static CpuIntCallback pCallbackCpuInt = NULL;
//CLA搬运地址定义
extern uint32_t Cla1funcsRunStart, Cla1funcsLoadStart, Cla1funcsLoadSize;
extern uint32_t Cla1ConstRunStart, Cla1ConstLoadStart, Cla1ConstLoadSize;
extern uint32_t CLA1mathTablesRunStart, CLA1mathTablesLoadStart, CLA1mathTablesLoadSize;

__attribute__((interrupt)) void cla1Isr1 (){}
__attribute__((interrupt)) void cla1Isr2 (){}
__attribute__((interrupt)) void cla1Isr3 (){}
__attribute__((interrupt)) void cla1Isr4 (){}
__attribute__((interrupt)) void cla1Isr5 (){}
__attribute__((interrupt)) void cla1Isr6 (){}
__attribute__((interrupt)) void cla1Isr7 (){}
__attribute__((interrupt)) void cla1Isr8 (){}
/**
* @brief: CLA_configClaMemory
* @param: void
* @return:Nothing
* @note:CLA_内存分配函数，初始化调用一次
*/
static void _CLA_ConfigClaMemory(void)
{
    EALLOW;
    //从Flash拷贝CLA的代码到内部的RAM
    memcpy((uint32_t *)&Cla1funcsRunStart, (uint32_t *)&Cla1funcsLoadStart,
           (uint32_t)&Cla1funcsLoadSize);
    memcpy((uint32_t *)&Cla1ConstRunStart, (uint32_t *)&Cla1ConstLoadStart,//数据区，存放常量的地方
           (uint32_t)&Cla1ConstLoadSize);
    memcpy((uint32_t *)&CLA1mathTablesRunStart, (uint32_t *)&CLA1mathTablesLoadStart,//数据区，存放常量的地方
           (uint32_t)&CLA1mathTablesLoadSize);

    MemCfgRegs.MSGxINIT.bit.INIT_CLA1TOCPU = 1;//启动CLA1到CPU的RAM的初始化
    while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CLA1TOCPU != 1){};//RAM初始化完成了后继续运行

    MemCfgRegs.MSGxINIT.bit.INIT_CPUTOCLA1 = 1;//启动CPU到CLA1的RAM的初始化
    while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CPUTOCLA1 != 1){};//RAM初始化完成了后继续运行

    MemCfgRegs.LSxMSEL.bit.MSEL_LS0 = 1;    //配置MSEL_LS0这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS0 = 1;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    MemCfgRegs.LSxMSEL.bit.MSEL_LS1 = 1;    //配置MSEL_LS1这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS1 = 1;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    MemCfgRegs.LSxMSEL.bit.MSEL_LS2 = 1;    //配置MSEL_LS2这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS2 = 1;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    MemCfgRegs.LSxMSEL.bit.MSEL_LS3 = 1;    //配置MSEL_LS3这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS3 = 1;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    MemCfgRegs.LSxMSEL.bit.MSEL_LS4 = 1;    //配置MSEL_LS2这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS4 = 1;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器

    MemCfgRegs.LSxMSEL.bit.MSEL_LS5 = 1;    //配置MSEL_LS2这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS5 = 0;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    EDIS;
}
/**
* @brief: CLA_initCpu1Cla1
* @param: void
* @return:Nothing
* @note:CLA_配置中断函数，目前配置为只触发TASK函数（触发源37就是PWM2INT），不触发Task执行后的中断函数
*/
static void _CLA_InitCpu1Cla1(void)
{
    //CLA 支持八项任务。任务 1 的优先级最高，任务 8 的优先级最低
    EALLOW;
    Cla1Regs.MVECT1 = (uint16_t)(&Cla1Task1);
    Cla1Regs.MVECT2 = (uint16_t)(&Cla1Task2);
    Cla1Regs.MVECT3 = (uint16_t)(&Cla1Task3);
    Cla1Regs.MVECT4 = (uint16_t)(&Cla1Task4);
    Cla1Regs.MVECT5 = (uint16_t)(&Cla1Task5);
    Cla1Regs.MVECT6 = (uint16_t)(&Cla1Task6);
    Cla1Regs.MVECT7 = (uint16_t)(&Cla1Task7);
    Cla1Regs.MVECT8 = (uint16_t)(&Cla1Task8);

//  Cla1Regs.MCTL.bit.IACKE = 1;    //启动IACK
//  Cla1Regs.MIER.all = (M_INT1 | M_INT8);//启用中断1和8
    Cla1Regs.MIER.all = (M_INT1);//启用中断1和8

    PieVectTable.CLA1_1_INT = &cla1Isr1;
    PieVectTable.CLA1_2_INT = &cla1Isr2;
    PieVectTable.CLA1_3_INT = &cla1Isr3;
    PieVectTable.CLA1_4_INT = &cla1Isr4;
    PieVectTable.CLA1_5_INT = &cla1Isr5;
    PieVectTable.CLA1_6_INT = &cla1Isr6;
    PieVectTable.CLA1_7_INT = &cla1Isr7;
    PieVectTable.CLA1_8_INT = &cla1Isr8;

    DmaClaSrcSelRegs.CLA1TASKSRCSEL1.bit.TASK1 = 45;//选择EPWM10_INT触发源给TASK1

    PieCtrlRegs.PIEIER11.all = 0xFFFF;
    IER |= (M_INT11);
    EDIS;
}
/**
* @brief: Cla_Interrupt_Init
* @param: void
* @return:Nothing
* @note:CLA中断配置总函数
*/
void Cla_Interrupt_Init(void)
{
    _CLA_ConfigClaMemory();  //地址内存配置
    _CLA_InitCpu1Cla1();     //CLA中断配置
}
/**
* @brief: Cla_Interrupt_Init
* @param: void
* @return:Nothing
* @note:注册回调函数的接口
*/
void RegisterCpuIntCallback(CpuIntCallback callback)
{
    pCallbackCpuInt = callback;
}

/**
* @brief: CPU_Interrupt
* @param: void
* @return:Nothing
* @note:CPU中断服务函数
*/
interrupt void CPU_Interrupt(void)
{
    if (pCallbackCpuInt != NULL)
    {
        pCallbackCpuInt(); // 调用上层注册的回调函数
    }
    ClearCpuInterrupt();
}
/**
* @brief: Cpu_Interrupt_Init
* @param: void
* @return:Nothing
* @note:CPU触发源配置函数
*/
void Cpu_Interrupt_Init(void)
{
    EALLOW;
    /*      PIE中断向量表配置        */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;     //向CPU请求中断，从这里就开始准备执行中断
    PieVectTable.EPWM7_INT = &CPU_Interrupt;    //EPWM7中断函数
    PieCtrlRegs.PIEIER3.bit.INTx7 = 1;          // EPWM7在中断向量表中3-5的位置
    IER |= M_INT3;
    EDIS;
}
/**
* @brief: ClearCpuInterrupt
* @param: void
* @return:Nothing
* @note:清除CPU中断标志位
*/
void ClearCpuInterrupt(void)
{
    /*清除中断标志,且对下次中断发出请求*/
    EPwm7Regs.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}
/**
* @brief: Interrupt_Init
* @param: void
* @return:Nothing
* @note:中断配置总函数
*/
void Interrupt_Init(void)
{
    Cla_Interrupt_Init();   //CLA中断配置
    Cpu_Interrupt_Init();   //CPU中断配置
}
```

### `driver/interrupt.h`

```c
/*
 * interrupt.h
 *
 *  Created on: 2025��1��6��
 *      Author: Nova007456
 */

#ifndef DEVICE_INTERRUPT_H_
#define DEVICE_INTERRUPT_H_

// ����ص��������ͣ�������Ҫ�����жϱ�־��
typedef void (*CpuIntCallback)(void);

// ע��ص��Ľӿ�
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
```

### `driver/pwm.c`

```c
/*
 * pwm.c
 *
 *  Created on: 2025��1��6��
 *      Author: Nova007456
 */
#include "F28x_Project.h"
#include "adc.h"
#include "pwm.h"

volatile struct EPWM_REGS *EPwm[] =
{
    0,&EPwm1Regs, &EPwm2Regs, &EPwm3Regs, &EPwm4Regs, &EPwm5Regs, &EPwm6Regs,
    &EPwm7Regs, &EPwm8Regs, &EPwm9Regs, &EPwm10Regs, &EPwm11Regs, &EPwm12Regs
};
/**
* @brief: ClearEpwmTz
* @param: void
* @return:Nothing
* @note:���EPWM��TZ����
*/
void ClearEpwmTz(uint16_t num)
{
    (*EPwm[num]).TZCLR.bit.OST = 1;                        //���TZ����
}
/**
  * @brief  FanEpwmUpdata
  * @param  pvdutyaddress:�洢pvռ�ձȵ������ַ;en_pv��PVģ��ʹ��
  * @retval None
  * @note   None
*/
uint16_t FanEpwmUpdata(float * dutyaddress, uint16_t en_pv)
{
    float duty[4];
    if(en_pv == 0)
    {
        EALLOW;
        EPwm10Regs.TZFRC.bit.OST = 1;
        EPwm11Regs.TZFRC.bit.OST = 1;
        EDIS;
        return 0;
    }
    else
    {
        EALLOW;
        EPwm10Regs.TZCLR.bit.OST = 1;
        EPwm11Regs.TZCLR.bit.OST = 1;
        EDIS;
    }

    if(dutyaddress[0] > 0 && dutyaddress[0] < 0.98)
    {
        duty[0] = dutyaddress[0];
    }
    else
    {
        duty[0] = 0;
    }
    if(dutyaddress[1] > 0 && dutyaddress[1] < 0.98)
    {
        duty[1] = dutyaddress[1];
    }
    else
    {
        duty[1] = 0;
    }
    if(dutyaddress[2] > 0 && dutyaddress[2] < 0.98)
    {
        duty[2] = dutyaddress[2];
    }
    else
    {
        duty[2] = 0;
    }
    if(dutyaddress[3] > 0 && dutyaddress[3] < 0.98)
    {
        duty[3] = dutyaddress[3];
    }
    else
    {
        duty[3] = 0;
    }
    EPwm10Regs.CMPA.bit.CMPA = (uint32_t)(EPwm10Regs.TBPRD * (1 - duty[0]));
    EPwm10Regs.CMPB.bit.CMPB = (uint32_t)(EPwm10Regs.TBPRD * (1 - duty[1]));
    EPwm11Regs.CMPA.bit.CMPA = (uint32_t)(EPwm11Regs.TBPRD * (1 - duty[2]));
    EPwm11Regs.CMPB.bit.CMPB = (uint32_t)(EPwm11Regs.TBPRD * (1 - duty[3]));
    return 1;
}
/**
  * @brief  PvEpwmUpdata
  * @param  pvdutyaddress:�洢pvռ�ձȵ������ַ;en_pv��PVģ��ʹ��
  * @retval None
  * @note   None
*/
//uint16_t PvEpwmUpdata(float pvduty,uint16_t channel, uint16_t en_pv)
//{
//    float dutypv;
//
//    if(en_pv == 0)
//    {
//        EALLOW;
//        EPwm8Regs.TZFRC.bit.OST = 1;
//        EPwm9Regs.TZFRC.bit.OST = 1;
//        EDIS;
//        return 0;
//    }
//    else
//    {
//        EALLOW;
//        EPwm8Regs.TZCLR.bit.OST = 1;
//        EPwm9Regs.TZCLR.bit.OST = 1;
//        EDIS;
//    }
//
//    if((pvduty> 0 )&&(pvduty < 0.98f))
//    {
//        dutypv = pvduty ;
//    }
//    else
//    {
//        dutypv  = 0;
//    }
//
//    switch(channel)
//    {
//         case 0:
//
//             EPwm8Regs.CMPA.bit.CMPA = (uint32_t)(EPwm8Regs.TBPRD * (1 - dutypv));
//         break;
//
//         case 1:
//
//             EPwm8Regs.CMPB.bit.CMPB = (uint32_t)(EPwm8Regs.TBPRD * (1 - dutypv));
//
//         break;
//
//         case 2:
//
//            EPwm9Regs.CMPA.bit.CMPA = (uint32_t)(EPwm9Regs.TBPRD * (1 - dutypv));
//
//         break;
//
//         default:
//
//         break;
//
//    }
//
//    return 1;
//}
/**
  * @brief  PvEpwmUpdata
  * @param  pvdutyaddress:�洢pvռ�ձȵ������ַ;en_pv��PVģ��ʹ��
  * @retval None
  * @note   None
*/
uint16_t PvEpwmUpdata(float * pvdutyaddress, uint16_t en_pv)
{
    float dutypv[4];

    if(en_pv == 0)
    {
        EALLOW;
        EPwm8Regs.TZFRC.bit.OST = 1;
        EPwm9Regs.TZFRC.bit.OST = 1;
        EDIS;
        return 0;
    }
    if(pvdutyaddress[0] > 0 && pvdutyaddress[0] < 0.98)
    {
        dutypv[0] = pvdutyaddress[0];
    }
    else
    {
        dutypv[0] = 0;
    }
    if(pvdutyaddress[1] > 0 && pvdutyaddress[1] < 0.98)
    {
        dutypv[1] = pvdutyaddress[1];
    }
    else
    {
        dutypv[1] = 0;
    }
    if(pvdutyaddress[2] > 0 && pvdutyaddress[2] < 0.98)
    {
        dutypv[2] = pvdutyaddress[2];
    }
    else
    {
        dutypv[2] = 0;
    }
    if(pvdutyaddress[3] > 0 && pvdutyaddress[3] < 0.98)
    {
        dutypv[3] = pvdutyaddress[3];
    }
    else
    {
        dutypv[3] = 0;
    }

    EPwm8Regs.CMPA.bit.CMPA = (uint32_t)(EPwm8Regs.TBPRD * (1 - dutypv[0]));
    EPwm8Regs.CMPB.bit.CMPB = (uint32_t)(EPwm8Regs.TBPRD * (1 - dutypv[1]));
    EPwm9Regs.CMPA.bit.CMPA = (uint32_t)(EPwm9Regs.TBPRD * (1 - dutypv[2]));
    EPwm9Regs.CMPB.bit.CMPB = (uint32_t)(EPwm9Regs.TBPRD * (dutypv[3]));
    EALLOW;
    EPwm8Regs.TZCLR.bit.OST = 1;
    EPwm9Regs.TZCLR.bit.OST = 1;
    EDIS;
    return 1;
}
/**
* @brief: _InvEpwmxInit
* @param: void
* @return:Nothing
* @note:���12��IGBT��EPWM���ú���
*/
void InvEpwmxInit(uint16_t num)
{
    /*      ʱ��ģ��        */
    (*EPwm[num]).TBCTL.bit.PRDLD = TB_SHADOW;              //ʹ��TBģ��Ӱ�ӼĴ������ض�ʱ����Ӱ�ӼĴ�����ֵ�Ż�����������Ĵ�����
    (*EPwm[num]).TBPRD = (Uint16)(INV_PWM_PERIOD >> 1);    //�趨PWMƵ��             /*PWMƵ��Ϊ20K,TBCLK/(2*TBPRD)*/
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //������λ�Ĵ���ֵ         /*��PWM����λģ���0��ʼ������û����λƫ��*/
    (*EPwm[num]).TBCTR = 0;                                //������д0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //������Ϊ��������
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //����ʱ�ӷ�Ƶ(��Ƶϵ��Ϊ1)     /*Ĭ�ϲ���Ƶ*/
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //��׼ʱ�ӷ�Ƶλ(��Ƶϵ��Ϊ1)   /*Ĭ�ϲ���Ƶ*/
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_ENABLE;             //ʹ�ܼ�����λ�Ĵ���TBPHS�е�ֵ
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;         //ͬ�������źŴ�����������λͬ��
    /*      �����޶�ģ��        */
    (*EPwm[num]).CMPA.bit.CMPA = (*EPwm[num]).TBPRD;       //ռ�ձ���0                    /*������������2.5ʱ������Ӧ����*/
    (*EPwm[num]).CMPB.bit.CMPB = 0;                        //ռ�ձ���0                    /*����CMPBΪ0*/
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //ӳ��ģʽ
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //������Ϊ0ʱ����ռ�ձ�
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //ӳ��ģʽ
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //������Ϊ0ʱ����ռ�ձ�          /*ʹ��Ӱ�ӼĴ�������������Ϊ0ʱ���������¼�*/
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;                           /*���CMPA��CMPB�����¼�����*/
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //��ʱ�������ֵ����CMPAʱ�����ڵ���,ʹEPWMA����ߵ�ƽ
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //��ʱ�������ֵ����CMPAʱ�����ڵݼ�,ʹEPWMA����͵�ƽ
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_CLEAR;                //��ʱ�������ֵ����CMPBʱ�����ڵ���,ʹEPWMB����͵�ƽ
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_SET;                  //��ʱ�������ֵ����CMPBʱ�����ڵݼ�,ʹEPWMB����ߵ�ƽ
    /*      ����ģ��        */
    (*EPwm[num]).DBCTL.bit.IN_MODE = DBA_ALL;              //EPWMxA��Ϊ�����ؼ��½���ʱ���ź�Դ    /*��������ģ�������ź�ԴΪEPWMAX*/
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;      //ʹ�������ؼ��½��ص���ʱ�ź�          /*��ȫʹ�������������������½���ͬʱʹ��*/
    (*EPwm[num]).DBCTL.bit.POLSEL = DB_ACTV_HIC;           //���߻��� EPWMxB��ת����             /*�ߵ�ƽ��Ч��EPWMA��Ϊԭʼ�ź��Ҿ��������½�������EPWMBֱ�����������A�źŽ��з�ת��δʹ��CMPB�ź�*/
    (*EPwm[num]).DBRED.bit.DBRED = (Uint16)INV_DEADBAND;   //����������ʱ��1us                 /�ߵ�ƽ��ʱʱ��/     T=DBRED/TBCLK
    (*EPwm[num]).DBFED.bit.DBFED = (Uint16)INV_DEADBAND;   //�½�������ʱ��1us                 /*�͵�ƽ��ʱʱ��*/
    /*      TZ����ģ��        */
    (*EPwm[num]).TZSEL.bit.OSHT1 = 1;                      //ʹ��TZ1(������)��ΪEPWM�ĵ��ι��ϱ�����������
    (*EPwm[num]).TZSEL.bit.OSHT2 = 1;                      //ʹ��TZ2(ĸ�߹���)��ΪEPWM�ĵ��ι��ϱ�����������
    (*EPwm[num]).TZCTL.bit.TZA = TZ_FORCE_LO;              //TZ����ʱǿ��EPWMxAΪ�͵�ƽ
    (*EPwm[num]).TZCTL.bit.TZB = TZ_FORCE_LO;              //TZ����ʱǿ��EPWMxBΪ�͵�ƽ
    (*EPwm[num]).TZFRC.bit.OST = 1;                        //����TZ����                   /*�����������*/
}
/**
* @brief: CpuInterruptEpwmxInit
* @param: void
* @return:Nothing
* @note:CPU�жϵ�EPWM���ú���
*/
void CpuInterruptEpwmxInit(uint16_t num)
{
    /*      ʱ��ģ��        */
    (*EPwm[num]).TBCTL.bit.PRDLD = TB_SHADOW;              //ʹ��TBģ��Ӱ�ӼĴ���
    (*EPwm[num]).TBPRD = (CPU_INTERRUPT_PWM_PERIOD >> 1);    //�趨PWMƵ��    ����PWMƵ��Ϊ80K
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //������λ�Ĵ���ֵ
    (*EPwm[num]).TBCTR = 0;                                //������д0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //������Ϊ��������
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //����ʱ�ӷ�Ƶ(��Ƶϵ��Ϊ1)
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //��׼ʱ�ӷ�Ƶλ(��Ƶϵ��Ϊ1)
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_ENABLE;             //���������λ�Ĵ���TBPHS�е�ֵ
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;         //ͬ�������źŴ���ͬ���¼�
    /*      �����޶�ģ��        */
    (*EPwm[num]).CMPA.bit.CMPA = (*EPwm[num]).TBPRD;       //ռ�ձ���0
    (*EPwm[num]).CMPB.bit.CMPB = 0;                        //ռ�ձ���0
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //ӳ��ģʽ
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //������Ϊ0ʱ����ռ�ձ�
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //ӳ��ģʽ
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //������Ϊ0ʱ����ռ�ձ�
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //��ʱ�������ֵ����CMPAʱ�����ڵ���,ʹEPWMA����ߵ�ƽ
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //��ʱ�������ֵ����CMPAʱ�����ڵݼ�,ʹEPWMA����͵�ƽ
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_CLEAR;                //��ʱ�������ֵ����CMPAʱ�����ڵ���,ʹEPWMB����͵�ƽ
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_SET;                  //��ʱ�������ֵ����CMPAʱ�����ڵݼ�,ʹEPWMB����ߵ�ƽ
    /*      ����ģ��        */
    (*EPwm[num]).DBCTL.bit.IN_MODE = DBA_ALL;              //EPWMxA��Ϊ�����ؼ��½���ʱ���ź�Դ
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;      //ʹ�������ؼ��½��ص���ʱ�ź�
    (*EPwm[num]).DBCTL.bit.POLSEL = DB_ACTV_HI;            //�����������Ϊ�ߵ�ƽ��Ч
    (*EPwm[num]).DBRED.bit.DBRED = (Uint16)CPU_INTERRUPT_DEADBAND;//����������ʱ��1us
    (*EPwm[num]).DBFED.bit.DBFED = (Uint16)CPU_INTERRUPT_DEADBAND;//�½�������ʱ��1us
    /*      ����CPU�жϵĴ����¼�        */
    (*EPwm[num]).CMPA.bit.CMPA = ACQPS_SYS_CLKS;            //Ԥ�����ʱ��    29
    (*EPwm[num]).CMPB.bit.CMPB = (*EPwm[num]).TBPRD - ACQPS_SYS_CLKS;       //625
    (*EPwm[num]).ETSEL.bit.SOCASEL = ET_CTRD_CMPA;          //TBCTR = CMPAʱ,�Ҽ������ݼ�ʱ���������¼�    /*�¼��ڼ��������¼����ҵ���CMPAʱ����SOCA��ADC���ת����*/
    (*EPwm[num]).ETSEL.bit.SOCBSEL = ET_CTRU_CMPB;          //TBCTR = CMPBʱ,�Ҽ���������ʱ���������¼�     /*�¼��ڼ��������ϼ����ҵ���CMPBʱ����SOCB��ADC���ת����*/
    (*EPwm[num]).ETPS.bit.SOCAPRD = ET_1ST;                 //ÿ����һ���¼�,����ADC����ź�
    (*EPwm[num]).ETSEL.bit.SOCAEN = 1;                      //ʹ��SOCA��ADC�������
    (*EPwm[num]).ETPS.bit.SOCBPRD = ET_1ST;                 //ÿ����һ���¼�,����ADC����ź�
    (*EPwm[num]).ETSEL.bit.SOCBEN = 1;                      //ʹ��SOCB��ADC�������
    /*      ѡ��CPU�ж϶�Ӧ�Ĵ����¼�        */
    (*EPwm[num]).ETSEL.bit.INTSEL = ET_CTR_ZERO;            //TBCTR = 0 ʱ����                         /*������Ϊ0ʱ�����ж��¼�*/
    (*EPwm[num]).ETSEL.bit.INTEN = 1;                       //ʹ���ж��ź�
    (*EPwm[num]).ETPS.bit.INTPRD = ET_1ST;                  //ÿ����һ�β����ж��ź�EPWMx_INT
    (*EPwm[num]).ETCLR.bit.INT = 1;                         //����ж��ź�EPWMx_INT��־λ
}
/**
* @brief: ClaInterruptEpwmxInit
* @param: void
* @return:Nothing
* @note:CLA�ж�  FAN��EPWM���ú���
*/
void ClaInterruptEpwmxInit(uint16_t num)
{
    /*      ʱ��ģ��        */
    (*EPwm[num]).TBCTL.bit.PRDLD = TB_SHADOW;              //ʹ��TBģ��Ӱ�ӼĴ���
    (*EPwm[num]).TBPRD = (Uint16)(CLA_INTERRUPT_PWM_PERIOD >> 1);//�趨PWMƵ��
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //������λ�Ĵ���ֵ
    (*EPwm[num]).TBCTR = 0;                                //������д0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //������Ϊ��������
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //����ʱ�ӷ�Ƶ(��Ƶϵ��Ϊ1)
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //��׼ʱ�ӷ�Ƶλ(��Ƶϵ��Ϊ1)
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_ENABLE;             //���������λ�Ĵ���TBPHS�е�ֵ
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;         //ͬ�������źŴ���ͬ���¼�
    /*      �����޶�ģ��        */
    (*EPwm[num]).CMPA.bit.CMPA = (*EPwm[num]).TBPRD ;       //ռ�ձ���0
    (*EPwm[num]).CMPB.bit.CMPB = (*EPwm[num]).TBPRD ;       //ռ�ձ���0
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //ӳ��ģʽ
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //������Ϊ0ʱ����ռ�ձ�
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //ӳ��ģʽ
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //������Ϊ0ʱ����ռ�ձ�
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //��ʱ�������ֵ����CMPAʱ�����ڵ���,ʹEPWMA����ߵ�ƽ
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //��ʱ�������ֵ����CMPAʱ�����ڵݼ�,ʹEPWMA����͵�ƽ
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_SET;                  //��ʱ�������ֵ����CMPAʱ�����ڵ���,ʹEPWMB����ߵ�ƽ
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_CLEAR;                //��ʱ�������ֵ����CMPAʱ�����ڵݼ�,ʹEPWMB����͵�ƽ
    /*      ����ģ��        */
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_DISABLE; // ����������ֱͨģʽ
    /*      TZ����ģ��        */
    (*EPwm[num]).TZCTL.bit.TZA = TZ_FORCE_LO;              //TZ����ʱǿ��EPWMxAΪ�͵�ƽ
    (*EPwm[num]).TZCTL.bit.TZB = TZ_FORCE_LO;              //TZ����ʱǿ��EPWMxBΪ�͵�ƽ
    (*EPwm[num]).TZFRC.bit.OST = 1;                        //����TZ����
    /*      ѡ��CLA�ж϶�Ӧ�Ĵ����¼�        */
    (*EPwm[num]).ETSEL.bit.INTSEL = ET_CTR_ZERO;            //TBCTR = 0 ʱ����
    (*EPwm[num]).ETSEL.bit.INTEN = 1;                       //ʹ���ж��ź�
    (*EPwm[num]).ETPS.bit.INTPRD = ET_1ST;                  //ÿ����һ�β����ж��ź�EPWMx_INT
    (*EPwm[num]).ETCLR.bit.INT = 1;                         //����ж��ź�EPWMx_INT��־λ
}
/**
* @brief: _FANEpwmxInit
* @param: void
* @return:Nothing
* @note:���ȵ�EPWM���ú���
*/
void FanEpwmxInit(uint16_t num)
{
    /*      ʱ��ģ��        */
    (*EPwm[num]).TBCTL.bit.PRDLD = TB_SHADOW;              //ʹ��TBģ��Ӱ�ӼĴ���
    (*EPwm[num]).TBPRD = (Uint16)(FAN_PWM_PERIOD >> 1);    //�趨PWMƵ��             /*���ȵ�PWMƵ��Ϊ20K,TBCLK/(2*TBPRD)*/
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //������λ�Ĵ���ֵ
    (*EPwm[num]).TBCTR = 0;                                //������д0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //������Ϊ��������
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //����ʱ�ӷ�Ƶ(��Ƶϵ��Ϊ1)     /*Ĭ�ϲ���Ƶ*/
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //��׼ʱ�ӷ�Ƶλ(��Ƶϵ��Ϊ1)   /*Ĭ�ϲ���Ƶ*/
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_ENABLE;             //ʹ�ܼ�����λ�Ĵ���TBPHS�е�ֵ
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;         //ͬ�������źŴ�����������λͬ��
    /*      �����޶�ģ��        */
    (*EPwm[num]).CMPA.bit.CMPA = (*EPwm[num]).TBPRD;       //ռ�ձ���0
    (*EPwm[num]).CMPB.bit.CMPB = (*EPwm[num]).TBPRD;       //ռ�ձ���0
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //ӳ��ģʽ
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //������Ϊ0ʱ����ռ�ձ�
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //ӳ��ģʽ
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //������Ϊ0ʱ����ռ�ձ�
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //��ʱ�������ֵ����CMPAʱ�����ڵ���,ʹEPWMA����ߵ�ƽ
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //��ʱ�������ֵ����CMPAʱ�����ڵݼ�,ʹEPWMA����͵�ƽ
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_SET;                  //��ʱ�������ֵ����CMPBʱ�����ڵ���,ʹEPWMB����͵�ƽ
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_CLEAR;                //��ʱ�������ֵ����CMPBʱ�����ڵݼ�,ʹEPWMB����ߵ�ƽ
    /*      ����ģ��        */
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_DISABLE; // ����������ֱͨģʽ
    /*      TZ����ģ��        */
    (*EPwm[num]).TZCTL.bit.TZA = TZ_FORCE_LO;              //TZ����ʱǿ��EPWMxAΪ�͵�ƽ
    (*EPwm[num]).TZCTL.bit.TZB = TZ_FORCE_LO;              //TZ����ʱǿ��EPWMxBΪ�͵�ƽ
    (*EPwm[num]).TZFRC.bit.OST = 1;                        //����TZ����                   /*�����������*/
}
/**
* @brief: InterruptEpwm_Init
* @param: void
* @return:Nothing
* @note:CLA�ж������ܺ���
*/
void InterruptEpwm_Init(void)
{
    /*      EPWM��ʼ��        */
    CpuInterruptEpwmxInit(7);                   //ռ��EPWM7��Ϊ����Դ,����80K��CPU�ж�
    /*CLA && FAN��EPWM��ʼ��*/
    ClaInterruptEpwmxInit(10);                  //ռ��EPWM10��Ϊ����Դ,����20K��CLA�ж�,�ö˿�EPWM��Ϊ���ȴ���
    FanEpwmxInit(11);                           /*EPWM11��EPWM����*/
}
/**
* @brief: InvEpwm_Init
* @param: void
* @return:Nothing
* @note:�����PWM����
*/
void InvEpwm_Init(void)
{
    InvEpwmxInit(1);                        //EPWM1 A �� Q1  B�� Q3
    InvEpwmxInit(2);                        //EPWM2 A �� Q2  B�� Q4
    InvEpwmxInit(3);                        //EPWM3 A �� Q5  B�� Q7
    InvEpwmxInit(4);                        //EPWM4 A �� Q6  B�� Q8
    InvEpwmxInit(5);                        //EPWM5 A �� Q9  B�� Q11
    InvEpwmxInit(6);                        //EPWM6 A �� Q10 B�� Q12
}
/*
* @brief: PVEpwmxInit
* @param: void
* @return:Nothing
* @note:�����EPWM���ú���
*/
void PvEpwmxInit(Uint16 num)
{
    /*      ʱ��ģ��        */
    (*EPwm[num]).TBCTL.bit.PRDLD = TB_SHADOW;              //ʹ��TBģ��Ӱ�ӼĴ���
    (*EPwm[num]).TBPRD = (Uint16)(PV_PWM_PERIOD >> 1);    //�趨PWMƵ��     /*20K*/
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //������λ�Ĵ���ֵ
    (*EPwm[num]).TBCTR = 0;                                //������д0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //������Ϊ��������
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //����ʱ�ӷ�Ƶ(��Ƶϵ��Ϊ1)
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //��׼ʱ�ӷ�Ƶλ(��Ƶϵ��Ϊ1)
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_DISABLE;             //��ֹ������λ�Ĵ���TBPHS�е�ֵ
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;         //CTR=0,ʱ��������TBCTR=0
    /*      �����޶�ģ��        */
    (*EPwm[num]).CMPA.bit.CMPA = (*EPwm[num]).TBPRD ;       //ռ�ձ���0
    (*EPwm[num]).CMPB.bit.CMPB = (*EPwm[num]).TBPRD ;       //ռ�ձ���0
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //ӳ��ģʽ
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //������Ϊ0ʱ����ռ�ձ�
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //ӳ��ģʽ
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //������Ϊ0ʱ����ռ�ձ�
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //��ʱ�������ֵ����CMPAʱ�����ڵ���,ʹEPWMA����ߵ�ƽ
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //��ʱ�������ֵ����CMPAʱ�����ڵݼ�,ʹEPWMA����͵�ƽ
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_SET;                  //��ʱ�������ֵ����CMPBʱ�����ڵ���,ʹEPWMB����͵�ƽ
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_CLEAR;                //��ʱ�������ֵ����CMPBʱ�����ڵݼ�,ʹEPWMB����ߵ�ƽ
    /*      ����ģ��        */
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_DISABLE;            // ����������ֱͨģʽ
    /*      TZ����ģ��        */
    (*EPwm[num]).TZSEL.bit.OSHT1 = 1;                      //ʹ��TZ1(������)��ΪEPWM�ĵ��ι��ϱ�����������
    (*EPwm[num]).TZSEL.bit.OSHT2 = 1;                      //ʹ��TZ2(ĸ�߹���)��ΪEPWM�ĵ��ι��ϱ�����������
    (*EPwm[num]).TZCTL.bit.TZA = TZ_FORCE_LO;              //TZ����ʱǿ��EPWMxAΪ�͵�ƽ
    (*EPwm[num]).TZCTL.bit.TZB = TZ_FORCE_LO;              //TZ����ʱǿ��EPWMxBΪ�͵�ƽ
    (*EPwm[num]).TZFRC.bit.OST = 1;                        //����TZ����
}
/**
* @brief: PvEpwm_Init
* @param: void
* @return:Nothing
* @note:�����PWM����
*/
void PvEpwm_Init(void)
{
    PvEpwmxInit(8);                        //EPWM8A �� PV1 EPWM8B �� PV2
    PvEpwmxInit(9);                        //EPWM9A �� PV3 EPWM9B �� PV4
}
/**
* @brief: ClearEpwmTZ
* @param: void
* @return:Nothing
* @note:���EPWM��TZ������־
*/
void ClearEpwmTZ(void)
{
    EALLOW;
    /*      �����TZ�������        */
    ClearEpwmTz(1);
    ClearEpwmTz(2);
    ClearEpwmTz(3);
    ClearEpwmTz(4);
    ClearEpwmTz(5);
    ClearEpwmTz(6);
    /*      PV����TZ�������        */
    ClearEpwmTz(8);
    ClearEpwmTz(9);
    /*���Ȳ���TZ�������*/
    ClearEpwmTz(10);
    ClearEpwmTz(11);
    EDIS;
}
/**
* @brief: EPwm_Init
* @param: void
* @return:Nothing
* @note:PWM�����ܺ���
*/
void EPwm_Init(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;   //��ֹ����EPWMʱ��ʱ��
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;   //ʹ������EPWMʱ��ʱ��

    InterruptEpwm_Init();                   //�ж�EPWM��ʼ��
    InvEpwm_Init();                         //���EPWM��ʼ��
    PvEpwm_Init();                          //Pv��EPWM��ʼ��
    /*����һ�μ�����ͬ������EPWM1����ͬ���¼�����λͬ����ر�����EPWMͬ�����ã��������������λ�¼�*/
    EPwm1Regs.TBCTL.bit.SWFSYNC = 1; // ����һ����λͬ��
    EPwm1Regs.TBCTL.bit.SWFSYNC = 0; // ����һ����λͬ��
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm5Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm6Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm7Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm8Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm9Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm10Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm11Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm12Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EDIS;
}
```

### `driver/pwm.h`

```c
/*
 * pwm.h
 *
 *  Created on: 2025��1��6��
 *      Author: Nova007456
 */
#ifndef DEVICE_PWM_H_
#define DEVICE_PWM_H_

#include "system.h"

#define TO_EVEN(x) ((int)(x) - ((int)(x) % 2))

#define INV_PWM_FREQUENCY   (20*1000)
#define INV_DEADBAND_US     (1.0)
#define INV_PWM_PERIOD      ((PWMSYSCLOCK_FREQ)/(INV_PWM_FREQUENCY))       //100*1000000/20*1000
#define INV_DEADBAND        (INV_DEADBAND_US * PWMSYSCLOCK_FREQ * 0.000001)

#define PV_PWM_FREQUENCY   (40*1000ul)
#define PV_DEADBAND_US     (0.5)
#define PV_PWM_PERIOD      (TO_EVEN((PWMSYSCLOCK_FREQ/PV_PWM_FREQUENCY)))
#define PV_DEADBAND        (PV_DEADBAND_US * PWMSYSCLOCK_FREQ * 0.000001)

#define CPU_INTERRUPT_PWM_FREQUENCY   (80*1000ul)
#define CPU_INTERRUPT_DEADBAND_US     (0.5)
#define CPU_INTERRUPT_PWM_PERIOD      ((PWMSYSCLOCK_FREQ)/(CPU_INTERRUPT_PWM_FREQUENCY))
#define CPU_INTERRUPT_DEADBAND        (CPU_INTERRUPT_DEADBAND_US * PWMSYSCLOCK_FREQ * 0.000001)

#define CLA_INTERRUPT_PWM_FREQUENCY   (20*1000)
#define CLA_INTERRUPT_DEADBAND_US     (0.5)
#define CLA_INTERRUPT_PWM_PERIOD      ((PWMSYSCLOCK_FREQ)/(CLA_INTERRUPT_PWM_FREQUENCY))
#define CLA_INTERRUPT_DEADBAND        (CLA_INTERRUPT_DEADBAND_US * PWMSYSCLOCK_FREQ * 0.000001)


#define FAN_PWM_FREQUENCY   (20*1000)
#define FAN_DEADBAND_US     (1.0)
#define FAN_PWM_PERIOD      ((PWMSYSCLOCK_FREQ)/(FAN_PWM_FREQUENCY))       //100*1000000/20*1000
#define FAN_DEADBAND        (FAN_DEADBAND_US * PWMSYSCLOCK_FREQ * 0.000001)

void EPwm_Init(void);
void ClearEpwmTZ(void);
//uint16_t PvEpwmUpdata(float pvduty,uint16_t channel, uint16_t en_pv);
uint16_t PvEpwmUpdata(float * pvdutyaddress, uint16_t en_pv);
uint16_t FanEpwmUpdata(float * dutyaddress, uint16_t en_pv);
#endif /* DEVICE_PWM_H_ */
```

### `driver/sci.c`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sci.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-3-6
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-3-6    <td>V1.0.0    <td>            <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "sci.h"
#include "F28x_Project.h"
#include "stdio.h"
#include "string.h"

#define RS485_DE_GPIO   69
/* Ƭѡ�����õ�ѡ��  */
#define RS485_RE_ENABLE()     GPIO_WritePin(RS485_DE_GPIO, 0);
/* Ƭѡ�����ø߲�ѡ�� */
#define RS485_DE_ENABLE()     GPIO_WritePin(RS485_DE_GPIO, 1);

#define ISO485_DE_GPIO  44
/* Ƭѡ�����õ�ѡ��  */
#define ISO485_RE_ENABLE()     GPIO_WritePin(ISO485_DE_GPIO, 0);
/* Ƭѡ�����ø߲�ѡ�� */
#define ISO485_DE_ENABLE()     GPIO_WritePin(ISO485_DE_GPIO, 1);



uint16_t garrReadDataBuff[16];

/**
 * @brief Scid���͵��ֽ�
 * @param data �����ֽ�
 */
void ScidSendByte(uint16_t data)
{
    while (ScidRegs.SCIFFTX.bit.TXFFST != 0) {}
    ScidRegs.SCITXBUF.all = (uint16_t)data;
}

#if PRINTF_ENABLE
int fputc(int ch, FILE *fp)
{
    while (ScidRegs.SCIFFTX.bit.TXFFST != 0); // �ȴ����ͻ�����Ϊ��
    ScidRegs.SCITXBUF.all = ch; // �����ַ�
    return ch;
}


int fputs(const char *_ptr, FILE *_fp)
{
    uint16_t i = 0;
    uint16_t len = strlen(_ptr);

    for(i = 0; i < len; i++)
    {
        ScidSendByte(_ptr[i]);
    }
//    SCI_writeCharArray(MySCI485_BASE, (uint16_t *)_ptr, len);
    return len;
}
#endif
/**
 * @brief SCID��buff�е�����
 * @param
 */
void ScidReadMessage(void)
{
    Uint16 num = ScidRegs.SCIFFRX.bit.RXFFST;//��ǰ����Buff���ж��ٸ�����
    if(num < 16)
    {
        for(int i = 0;(i < num);++i)
        {
            garrReadDataBuff[i] = ScidRegs.SCIRXBUF.all&0xFF;
        }
    }
}
/**
* @brief: DebugSciD_Init
* @param: void
* @return:Nothing
* @note:���Ե�SCID��ʼ������
*/
void DebugSciD_Init(void)
{
    EALLOW;
//    PieVectTable.SCID_RX_INT = &ScidRxIsr;
    ScidRegs.SCICCR.all = 0x0007;
    ScidRegs.SCICTL1.all = 0x0003;
    ScidRegs.SCICTL2.bit.TXINTENA = 1;
    ScidRegs.SCICTL2.bit.RXBKINTENA = 1;
    ScidRegs.SCIHBAUD.all = ((uint16_t)SCID_PRD  & 0xFF00U) >> 8U;
    ScidRegs.SCILBAUD.all = (uint16_t)SCID_PRD  & 0x00FFU;
    ScidRegs.SCICCR.bit.LOOPBKENA = 0;
    ScidRegs.SCIFFTX.all = 0xE040;
    ScidRegs.SCIFFRX.all = 0x0021;
    ScidRegs.SCIFFRX.bit.RXFFIL = 11;
    ScidRegs.SCIFFCT.all = 0x00;

    ScidRegs.SCICTL1.all = 0x0023;
    ScidRegs.SCIFFTX.bit.TXFIFORESET = 1;
    ScidRegs.SCIFFRX.bit.RXFIFORESET = 1;

    /*      SciD�Ľ����ж��Ȳ�ʹ��    */
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieCtrlRegs.PIEIER8.bit.INTx7 = 1;
    IER |= M_INT8;
    EDIS;
}


/**
* @brief: SciaFifoInit(MCU)
* @param:                             
* @note: 
*
*/
void SciaFifoInit()
{
    // GPIO_SetupPinMux(RS485_DE_GPIO, GPIO_MUX_CPU1, 0);
    // GPIO_SetupPinOptions(RS485_DE_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    //RS485_RE_ENABLE();
    GPIO_SetupPinMux(43, GPIO_MUX_CPU1, 15); //RX
    GPIO_SetupPinOptions(43, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(42, GPIO_MUX_CPU1, 15); //TX
    GPIO_SetupPinOptions(42, GPIO_OUTPUT, GPIO_ASYNC);

    EALLOW;  // This is needed to write to EALLOW protected registers
//    PieVectTable.SCIA_RX_INT = &SciaRxFifoIsr;
//   PieVectTable.SCIA_TX_INT = &SciaTxFifoIsr;
    EDIS;

    SciaRegs.SCICCR.all = 0x0007;      // 1 stop bit, ,No loopback , No parity , 8 char bits, async mode, idle-line protocol
    SciaRegs.SCICTL1.all = 0x0003;     // enable TX, RX, internal SCICLK, Disable RX ERR, SLEEP, TXWAKE
    SciaRegs.SCICTL2.bit.TXINTENA = 1;      // Transmitter buffer and shift registers are both empty
    SciaRegs.SCICTL2.bit.RXBKINTENA = 1;    // Receiver-buffer/break interrupt enable.
    SciaRegs.SCIHBAUD.all = ((uint16_t)SCIA_PRD  & 0xFF00U) >> 8U;
    SciaRegs.SCILBAUD.all = (uint16_t)SCIA_PRD  & 0x00FFU;
    SciaRegs.SCICCR.bit.LOOPBKENA = 0; // Enable loop back
    SciaRegs.SCIFFTX.all = 0xE040;
    SciaRegs.SCIFFRX.all = 0x0021;
    SciaRegs.SCIFFCT.all = 0x00;

    SciaRegs.SCICTL1.all = 0x0023;     // Relinquish SCI from Reset
    SciaRegs.SCIFFTX.bit.TXFIFORESET = 1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;
}



/**
* @brief: ScibFifoInit(RS485 ARC)
* @param:                             
* @note: 
*
*/
void ScibFifoInit()
{
    GPIO_SetupPinMux(RS485_DE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RS485_DE_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    RS485_RE_ENABLE();
    GPIO_SetupPinMux(71, GPIO_MUX_CPU1, 6);
    GPIO_SetupPinOptions(71, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(70, GPIO_MUX_CPU1, 6);
    GPIO_SetupPinOptions(70, GPIO_OUTPUT, GPIO_ASYNC);

    EALLOW;  // This is needed to write to EALLOW protected registers
//    PieVectTable.SCIA_RX_INT = &SciaRxFifoIsr;
//   PieVectTable.SCIA_TX_INT = &SciaTxFifoIsr;
    EDIS;

    ScibRegs.SCICCR.all = 0x0007;      // 1 stop bit,  No loopback
    // No parity,8 char bits,
    // async mode, idle-line protocol
    ScibRegs.SCICTL1.all = 0x0003;     // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    ScibRegs.SCICTL2.bit.TXINTENA = 1;
    ScibRegs.SCICTL2.bit.RXBKINTENA = 1;
    ScibRegs.SCIHBAUD.all = ((uint16_t)SCIB_PRD  & 0xFF00U) >> 8U;
    ScibRegs.SCILBAUD.all = (uint16_t)SCIB_PRD  & 0x00FFU;
    ScibRegs.SCICCR.bit.LOOPBKENA = 0; // Enable loop back
    ScibRegs.SCIFFTX.all = 0xE040;
    ScibRegs.SCIFFRX.all = 0x0021;
    ScibRegs.SCIFFCT.all = 0x00;

    ScibRegs.SCICTL1.all = 0x0023;     // Relinquish SCI from Reset
    ScibRegs.SCIFFTX.bit.TXFIFORESET = 1;
    ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;

}

/**
* @brief: ScicFifoInit(ISO485 Menter)
* @param:                             
* @note: 
*
*/
void ScicFifoInit()
{
    GPIO_SetupPinMux(ISO485_DE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(ISO485_DE_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    ISO485_RE_ENABLE();
    GPIO_SetupPinMux(57, GPIO_MUX_CPU1, 6);
    GPIO_SetupPinOptions(57, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(56, GPIO_MUX_CPU1, 6);
    GPIO_SetupPinOptions(56, GPIO_OUTPUT, GPIO_ASYNC);

    EALLOW;  // This is needed to write to EALLOW protected registers
//    PieVectTable.SCIA_RX_INT = &SciaRxFifoIsr;
//   PieVectTable.SCIA_TX_INT = &SciaTxFifoIsr;
//    EDIS;

    ScicRegs.SCICCR.all = 0x0007;      // 1 stop bit,  No loopback
    // No parity,8 char bits,
    // async mode, idle-line protocol
    ScicRegs.SCICTL1.all = 0x0003;     // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    ScicRegs.SCICTL2.bit.TXINTENA = 1;
    ScicRegs.SCICTL2.bit.RXBKINTENA = 1;
    ScicRegs.SCIHBAUD.all = ((uint16_t)SCIC_PRD  & 0xFF00U) >> 8U;
    ScicRegs.SCILBAUD.all = (uint16_t)SCIC_PRD  & 0x00FFU;
    ScicRegs.SCICCR.bit.LOOPBKENA = 0; // Enable loop back
    ScicRegs.SCIFFTX.all = 0xE040;
    ScicRegs.SCIFFRX.all = 0x0021;
    ScicRegs.SCIFFCT.all = 0x00;

    ScicRegs.SCICTL1.all = 0x0023;     // Relinquish SCI from Reset
    ScicRegs.SCIFFTX.bit.TXFIFORESET = 1;
    ScicRegs.SCIFFRX.bit.RXFIFORESET = 1;

    EDIS;
}


/**
* @brief: Sci_Init
* @param: void
* @return:Nothing
* @note:SCI��ʼ������
*/
void Sci_Init(void)
{
    EALLOW;
    DebugSciD_Init();   //���Դ�������
    SciaFifoInit();     //MCUЭ������
    ScibFifoInit();     //ARC485
    ScicFifoInit();     //���485

    EDIS;
}

/**
* @brief: SciaXmit
* @param: a
* @note:
*
*/
void SciaXmit(int a)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    SciaRegs.SCITXBUF.all = a;
}

/**
* @brief: SciaSend
* @param: data
* @param: len
* @note:
*
*/
void SciaSend(uint8_t* data, uint16_t len)
{
    int i;

    for(i = 0; i < len; i ++)
    {
        SciaXmit(data[i]);
    }

    while((SciaRegs.SCICTL2.bit.TXRDY == 0) || (SciaRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }

}


void SciaSendByte(uint8_t data)
{

    SciaXmit(data);

    while((SciaRegs.SCICTL2.bit.TXRDY == 0) || (SciaRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }

}



/**
* @brief: ScibXmit
* @param: a
* @note:
*
*/
void ScibXmit(int a)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0) {}
    ScibRegs.SCITXBUF.all = a;
}


/**
* @brief: ScibSend
* @param: data
* @param: len
* @note:
*
*/
void ScibSend(uint8_t* data, uint16_t len)
{
    int i;

    RS485_DE_ENABLE();
    for(i = 0; i < len; i ++)
    {
        ScibXmit(data[i]);
    }

    while((ScibRegs.SCICTL2.bit.TXRDY == 0) || (ScibRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }
    RS485_RE_ENABLE();
}

void ScibSendByte(uint8_t data)
{

    RS485_DE_ENABLE();

    ScibXmit(data);

    while((ScibRegs.SCICTL2.bit.TXRDY == 0) || (ScibRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }
    RS485_RE_ENABLE();
}

/**
* @brief: ScicXmit
* @param: a
* @note:
*
*/
void ScicXmit(int a)
{
    while (ScicRegs.SCIFFTX.bit.TXFFST != 0) {}
    ScicRegs.SCITXBUF.all = a;
}


/**
* @brief: ScicSend
* @param: data
* @param: len
* @note:
*
*/
void ScicSend(uint8_t* data, uint16_t len)
{
    int i;

    ISO485_DE_ENABLE();
    for(i = 0; i < len; i ++)
    {
        ScicXmit(data[i]);
    }

    while((ScicRegs.SCICTL2.bit.TXRDY == 0) || (ScicRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }
    ISO485_RE_ENABLE();
}

void ScicSendByte(uint8_t data)
{

    ISO485_DE_ENABLE();

    ScicXmit(data);

    while((ScicRegs.SCICTL2.bit.TXRDY == 0) || (ScicRegs.SCICTL2.bit.TXEMPTY == 0))
    {

    }
    ISO485_RE_ENABLE();
}
```

### `driver/sci.h`

```c
/*
 * sci.h
 *
 *  Created on: 2025��1��6��
 *      Author: Nova007456
 */
#include "F28x_Project.h"
#include "system.h"

#ifndef DEVICE_SCI_H_
#define DEVICE_SCI_H_

#define PRINTF_ENABLE   (1)

#define SCID_FREQ        115200//1250000
#define SCID_PRD         ((LSPCLK_FREQ/(SCID_FREQ*8))-1)

#define SCIA_FREQ        (uint32_t)115200
#define SCIA_PRD         ((LSPCLK_FREQ/(SCIA_FREQ*8))-1)

#define SCIB_FREQ        (uint32_t)115200
#define SCIB_PRD         ((LSPCLK_FREQ/(SCIB_FREQ*8))-1)

#define SCIC_FREQ        (uint32_t)9600 //memter
#define SCIC_PRD         ((LSPCLK_FREQ/(SCIC_FREQ*8))-1)



void Sci_Init(void);
void ScidSendByte(uint16_t data);
void SciaSend(uint8_t* data, uint16_t len);
void ScibSend(uint8_t* data, uint16_t len);
void ScicSend(uint8_t* data, uint16_t len);
void ScicFifoInit();

#endif /* DEVICE_SPI_H_ */
```

### `driver/spi.c`

```c
/*
 * spi.c
 *
 *  Created on: 2025��1��6��
 *      Author: Nova007456
 */
#include "spi.h"
#include "F28x_Project.h"

static uint16_t _gRdataTemp = 0;

/**
* @brief: _FlashSpiWriteByte
* @param: a
* @return:
* @note:
*/
static void _FlashSpiWriteByte(uint16_t a)
{
    SpiaRegs.SPITXBUF = (a << 8);

    while(SpiaRegs.SPIFFRX.bit.RXFFST !=1) { }
    _gRdataTemp = SpiaRegs.SPIRXBUF;
}
/**
* @brief: _SpiReadByte
* @param: void
* @return:
* @note:
*/
static uint16_t _FlashSpiReadByte(void)
{
    SpiaRegs.SPITXBUF = (0 << 8);
    while(SpiaRegs.SPIFFRX.bit.RXFFST !=1) { }
    _gRdataTemp = SpiaRegs.SPIRXBUF;

    return _gRdataTemp;
}
/**
* @brief: SpiWriteData
* @param: len
* @param: data
* @note:
*/
void SpiWriteData(uint16_t len, uint16_t* data)
{
    uint16_t i = 0;
    for(i = 0; i < len; i ++)
    {
        _FlashSpiWriteByte(data[i]);
    }
}
/**
* @brief: SpiReadData
* @param: len
* @param: data
* @note:
*/
void SpiReadData(uint16_t len, uint16_t* data)
{
    uint16_t i = 0;
    for(i = 0; i < len; i ++)
    {
        data[i] =  _FlashSpiReadByte();
    }
}
/**
* @brief: CH390_Spi_Init
* @param: void
* @return:Nothing
* @note:CH390оƬSPI�ܺ���
*/
void CH390_Spi_Init(void)
{
    EALLOW;
    SpibRegs.SPICCR.bit.SPISWRESET = 0;//��λ
    //SpibRegs.SPIFFTX.all = 0xEO40;    //SPI���ͼĴ���,�ȼ�������Ƭ������
    SpibRegs.SPIFFTX.bit.SPIRST = 1;    //��15λ:SPIFIFO��ʼ���·��ͺͽ���
    SpibRegs.SPIFFTX.bit.SPIFFENA = 0;  //��14λ:��ֹFIFI
    SpibRegs.SPIFFTX.bit.TXFIFO = 1;    //��13λ:����ʹ�ܷ���FIFO
    SpibRegs.SPIFFTX.bit.TXFFST = 0;    //��12-8λ:����FIOFO���м�����,���16��(0x10000)
    SpibRegs.SPIFFTX.bit.TXFFINT = 0;   //��7λ:�������ж�
    SpibRegs.SPIFFTX.bit.TXFFINTCLR = 1;//��6λ:����жϱ�־λ
    SpibRegs.SPIFFTX.bit.TXFFIENA = 0;  //��5λ:��ֹ����TXFFIVLƥ��С�ڻ���ڵ�TXFIFO�ж�
    SpibRegs.SPIFFTX.bit.TXFFIL = 0;    //��4-0λ:����TXFIFO�жϴ�����λ

    //SpibRegs.SPIFFRX.all = 0x2044;    //SPI���ռĴ���,�ȼ�������Ƭ������
    SpibRegs.SPIFFRX.bit.RXFFINTCLR=1;  //��14λ:���FIFO�����־λ
    SpibRegs.SPIFFRX.bit.RXFIFORESET=1; //��13λ:����ʹ�ܽ���FIFO
    SpibRegs.SPIFFRX.bit.RXFFST = 0;    //��12-8λ:����FIOFO���м�����,���16��(0x10000)
    SpibRegs.SPIFFRX.bit.RXFFINT = 1;   //��6λ:����жϱ�־λ
    SpibRegs.SPIFFRX.bit.RXFFIENA = 0;  //��5λ:��ֹ����RXFFIVLƥ��С�ڻ���ڵ�RXFIFO�ж�
    SpibRegs.SPIFFRX.bit.RXFFIL = 0;    //��4-0λ:����RXFIFO�жϴ�����λ

    SpibRegs.SPICTL.bit.MASTER_SLAVE = 1;   //�趨Ϊ����
    SpibRegs.SPICTL.bit.TALK = 1;           //ʹ�ܷ���
    SpibRegs.SPICTL.bit.CLK_PHASE = 0;      //SPI�ź��ӳٰ����ڣ���CLKPOLARITY���
    SpibRegs.SPICTL.bit.SPIINTENA = 0;      //��ֹ���ʹ��

    SpibRegs.SPIBRR.bit.SPI_BIT_RATE = SPI_BRR;//������

    SpibRegs.SPIPRI.bit.FREE = 1;   //�����жϹ���SPI��������

    SpibRegs.SPICCR.bit.CLKPOLARITY = 1;//0;//������������ݣ��½�����������
    SpibRegs.SPICCR.bit.SPICHAR = 7;//�ֳ�����λ 8λ
    SpibRegs.SPICCR.bit.SPILBK = 0;     //��ֹSPI����ģʽ
    SpibRegs.SPICCR.bit.SPISWRESET = 1;//SPI׼�����ܻ�����һ���ֽ�
    EDIS;
}
/**
* @brief: Flash_Spi_Init
* @param: void
* @return:Nothing
* @note:FlashоƬSPI�ܺ���
*/
void Flash_Spi_Init(void)
{
    EALLOW;
    SpiaRegs.SPIFFTX.all = 0xE040;
    SpiaRegs.SPIFFRX.all = 0x2044;
    SpiaRegs.SPIFFCT.all = 0x0;
    SpiaRegs.SPICCR.bit.SPISWRESET = 0;
    SpiaRegs.SPICCR.bit.CLKPOLARITY = 0;
    SpiaRegs.SPICCR.bit.SPICHAR = (8-1);
    SpiaRegs.SPICCR.bit.SPILBK = 0;
    SpiaRegs.SPICTL.bit.MASTER_SLAVE = 1;
    SpiaRegs.SPICTL.bit.TALK = 1;
    SpiaRegs.SPICTL.bit.CLK_PHASE = 1;
    SpiaRegs.SPICTL.bit.SPIINTENA = 0;
    SpiaRegs.SPIBRR.bit.SPI_BIT_RATE = SPI_FLASH_BRR;
    SpiaRegs.SPIPRI.bit.FREE = 1;
    SpiaRegs.SPICCR.bit.SPISWRESET = 1;
    EDIS;
}

/**
* @brief: Spi_Init
* @param: void
* @return:Nothing
* @note:SPI�ܺ���
*/
void Spi_Init(void)
{
    EALLOW;
    CH390_Spi_Init();   //CH390��SPIB����
    Flash_Spi_Init();   //Flash��SPIA����
    EDIS;
}
```

### `driver/spi.h`

```c
/*
 * spi.h
 *
 *  Created on: 2025��1��6��
 *      Author: Nova007456
 */

#ifndef DEVICE_SPI_H_
#define DEVICE_SPI_H_

#include "F28x_Project.h"

#define SPI_BRR        ((200E6 / 4) / 125E5) - 1
#define SPI_FLASH_BRR  ((200E6 / 4) / 125E5) - 1

void Spi_Init(void);
void SpiWriteData(uint16_t len, uint16_t* data);
void SpiReadData(uint16_t len, uint16_t* data);

#endif /* DEVICE_SPI_H_ */
```

### `driver/system.c`

```c
/*
 * system.c
 *
 *  Created on: 2025��1��6��
 *      Author: Nova007456
 */

#include "system.h"
#include "F28x_Project.h"

/**
* @brief: SysRestart
* @param: void
* @note: ϵͳ��λ����
*/
void SysRestart(void)
{
    EALLOW;
    WdRegs.WDCR.all = 0x00af;
    WdRegs.WDCR.bit.WDCHK = 2;
    EDIS;
}
```

### `driver/system.h`

```c
/*
 * system.h
 *
 *  Created on: 2025��1��6��
 *      Author: Nova007456
 */
#ifndef SYSTEM_H_
#define SYSTEM_H_


#define CPU_SYS_CLOCK       (200*1000000)
#define LSPCLK_FREQ         (CPU_SYS_CLOCK/4)
#define PWMSYSCLOCK_FREQ    (100*1000000)

//#define LOW_VOL_TEST                 (1)//��ѹ���Ի���

//����ö��(CPU�ж�ʱ���л�״̬)
typedef enum
{
    EN_CPU_INTERRUPT_TIME_0,
    EN_CPU_INTERRUPT_TIME_1,
    EN_CPU_INTERRUPT_TIME_2,
    EN_CPU_INTERRUPT_TIME_3,
    EN_CPU_INTERRUPT_TIME_4,
} EN_CPU_INTERRUPT_TIME;

typedef struct _cpu_interrupt_t
{
    EN_CPU_INTERRUPT_TIME time;
} CpuInterrupt_t;

void SysRestart(void);

//#define _LAUNCHXL_F28379D

#endif /* DEVICE_SYSTEM_H_ */
```

