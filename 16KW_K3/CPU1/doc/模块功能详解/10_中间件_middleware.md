# 10 中间件 middleware

对应源码：`middleware/*`

---

## 1. 通用 FSM：`BizFw_Fsm`

### 结构

状态含：状态 ID、运行函数、退出函数、转移表、累计运行次数。

### `BizFw_Fsm_Update`

1. 执行当前 `run`，计数 +1  
2. 顺序测转移条件  
3. 命中：`on_exit` → 转移动作 → 切状态 → 新状态计数清零  

应用：`biz_fsm_interface` 每 10 ms 调用。

---

## 2. CRC：`crc`

- `CrcModbus`：查表 CRC16  
- `CrcJudge`：比较帧尾低/高字节  
- 另有大端 16-bit 组合、字节翻转、U16/U8 拷贝  

用于 Modbus 帧校验。

---

## 3. 队列：`queue`

定长元素环形 FIFO：写/读索引、深度、有效项数。

| API | 行为 |
|-----|------|
| `QueueInit` | 堆分配 |
| `QueueInit_1` | 使用调用方缓冲 |
| `QueuePut` | 拷贝入队 |
| `QueueGet` | 出队返回队头地址 |
| `QueueRead` | 窥视 |
| `QueueClear` | 丢弃队头 |

应用：串口完整帧入 `gstSciRevQueue`。  

**风险**：`QueueDeInit` 无条件 `free`；若对象由 `QueueInit_1` 初始化会错误释放。

---

## 4. 滤波

### `mid_filter`

- 一阶巴特沃斯：双线性 `a0/a1/b1`，`y[n]=a0x[n]+a1x[n-1]-b1y[n-1]`  
- 二阶巴特沃斯：ζ≈0.7071  

### `mid_sliding_filter`

调用方持有缓冲；维护运行和、索引、满窗；O(1) 更新。

应用：RCMU 用一阶；NTC 用滑动平均。

---

## 5. 条件累积 / 延时触发

### `cond_cum_countTrigger`

`CondCum_CountTrigger_Run`：真累加，假清零或递减；达阈值调 hook 并清计数。

### `flag_dly_trigger`

把上述封装为“延时置位/延时清除”宏。  
运行函数可放 `.TI.ramfunc`，适合 ISR 故障去抖。  

应用：`fault` 的 ERRDLY 类宏。

---

## 6. 通信服务：`mid_comservice`

### 职责

三路串口累计接收 → 识别 **PCS 私有协议** 或 **Modbus RTU** → 完整帧入队。

### 三路缓冲

ARC / 电表 / 协处理器；每路约 2112 B；完成帧队列深度 3。

### 私有协议

- 请求头 `0x5A`，应答头 `0xA5`  
- 按命令最高位区分读写  
- 累加和逻辑存在，部分路径未见强制校验调用  

### Modbus

支持 0x03/0x04/0x06/0x10；按长度收集后 `CrcJudge`。

### API

- `mid_comservice_data_in`：ISR/驱动写入字节  
- `MidComserviceHandle`：主循环推进状态机、入队  

### 风险

- 边界判断 `>` 与 `>=` 不一致可能导致越界  
- 缓冲写满错误时全局返回值未在入口复位  

---

## 7. 在系统中的位置

```
SCI 字节 ──► mid_comservice ──► 帧队列 ──► protocol_process
fault 宏 ──► cond_cum / flag_dly
BizFSM ──► BizFw_Fsm
Modbus ──► crc
RCMU/NTC ──► filter / sliding_filter
```

---

## 附录：本章源码

> 以下为本章涉及的全部自研源码原文（不含 TI `include/`、`source/` 库）。

### `middleware/BizFw_Fsm.c`

```c
/**
 * @file BizFw_Fsm.c
 * @brief 有限状态机通用框架实现
 */

#include "BizFw_Fsm.h"

int BizFw_Fsm_Init(BizFw_Fsm_t* fsm, BizFw_FsmState_t* initial_state)
{
    if(fsm == NULL || initial_state == NULL) {
        return -1;
    }

    fsm->current_state = initial_state;
    fsm->current_state_id = initial_state->state_id;
    initial_state->running_time_count = 0U;

    return 0;
}

int BizFw_Fsm_Update(BizFw_Fsm_t* fsm, void* context)
{
    uint16_t i;

    if(fsm == NULL || fsm->current_state == NULL) {
        return -1;
    }

    if(fsm->current_state->function != NULL) {
        fsm->current_state->function(context);
        fsm->current_state->running_time_count++;
    }

    for(i = 0; i < fsm->current_state->transition_count; ++i) {
        BizFw_FsmTransition_t* transition = &fsm->current_state->transitions[i];

        if(transition->condition != NULL && transition->condition(context)) {
            BizFw_FsmState_t* leaving_state = fsm->current_state;

            if(transition->next_state == NULL) {
                return -1;
            }

            if(leaving_state->on_exit != NULL) {
                leaving_state->on_exit(context);
            }

            if(transition->action != NULL) {
                transition->action(context);
            }

            fsm->current_state = transition->next_state;
            fsm->current_state_id = fsm->current_state->state_id;
            fsm->current_state->running_time_count = 0U;

            break;
        }
    }

    return 0;
}
```

### `middleware/BizFw_Fsm.h`

```c
/**
 * @file BizFw_Fsm.h
 * @brief
 */

#ifndef BIZFW_FSM_H
#define BIZFW_FSM_H

#include "F28x_Project.h"

typedef struct bizfw_fsm_transition_stru_t
{
    uint16_t (*condition)(void* context);
    void (*action)(void* context);
    struct bizfw_fsm_state_stru_t* next_state;
} BizFw_FsmTransition_t;

typedef struct bizfw_fsm_state_stru_t
{
    uint16_t state_id;
    uint16_t transition_count;
    void (*function)(void* context);
    void (*on_exit)(void* context); /**< 离开本状态时调用（转移条件已满足、切换前） */
    BizFw_FsmTransition_t* transitions;
    uint32_t running_time_count;
} BizFw_FsmState_t;

typedef struct bizfw_fsm_stru_t
{
    uint16_t current_state_id;
    BizFw_FsmState_t* current_state;
} BizFw_Fsm_t;

int BizFw_Fsm_Init(BizFw_Fsm_t* fsm, BizFw_FsmState_t* initial_state);
int BizFw_Fsm_Update(BizFw_Fsm_t* fsm, void* context);

#endif /* BIZFW_FSM_H */
```

### `middleware/cond_cum_countTrigger.c`

```c
/**
 * @file cond_cum_countTrigger.c
 * @brief Conditional cumulative count trigger
 */

#include "cond_cum_countTrigger.h"

void CondCum_CountTrigger_Init(CondCum_CountTrigger_t* trigger, Uint32 threshold)
{
    if(trigger == 0) {
        return;
    }

    trigger->count = 0;
    trigger->count_interval = 1;
    trigger->trig_threshold = threshold;
    trigger->trigged = 0U;
    trigger->trig_hook = 0;
    trigger->hook_param = 0;
    trigger->mode = COND_CUM_COUNT_TRIGGER_MODE_DECREASE;
}

void CondCum_CountTrigger_Reset(CondCum_CountTrigger_t* trigger)
{
    if(trigger == 0) {
        return;
    }

    trigger->trigged = 0U;
    trigger->count = 0;
}

#pragma CODE_SECTION(CondCum_CountTrigger_Run, ".TI.ramfunc");
void CondCum_CountTrigger_Run(CondCum_CountTrigger_t* trigger, Uint16 value)
{
    if(trigger == 0) {
        return;
    }

    if(value == 1U) {
        trigger->count += trigger->count_interval;
    } else if(value == 0U) {
        if(trigger->mode == COND_CUM_COUNT_TRIGGER_MODE_CLEAR) {
            trigger->count = 0;
        } else {
            trigger->count -= trigger->count_interval;
        }
    }

    if(trigger->count < 0) {
        trigger->count = 0;
    }

    if(trigger->count >= (int32)trigger->trig_threshold) {
        trigger->trigged = 1U;
        if(trigger->trig_hook != 0) {
            trigger->trig_hook(trigger->hook_param);
        }
        trigger->count = 0;
    }
}
```

### `middleware/cond_cum_countTrigger.h`

```c
/**
 * @file cond_cum_countTrigger.h
 * @brief Conditional cumulative count trigger
 */

#ifndef COND_CUM_COUNT_TRIGGER_H
#define COND_CUM_COUNT_TRIGGER_H

#include "F28x_Project.h"

typedef enum cond_cum_count_trigger_mode_e
{
    COND_CUM_COUNT_TRIGGER_MODE_DECREASE = 0,
    COND_CUM_COUNT_TRIGGER_MODE_CLEAR = 1
} CondCum_CountTriggerMode_e;

typedef struct cond_cum_count_trigger_stru_t
{
    int32 count;
    int32 count_interval;
    Uint32 trig_threshold;
    Uint16 trigged;
    void (*trig_hook)(void* param);
    void* hook_param;
    CondCum_CountTriggerMode_e mode;
} CondCum_CountTrigger_t;

void CondCum_CountTrigger_Init(CondCum_CountTrigger_t* trigger, Uint32 threshold);
void CondCum_CountTrigger_Reset(CondCum_CountTrigger_t* trigger);
void CondCum_CountTrigger_Run(CondCum_CountTrigger_t* trigger, Uint16 value);

#endif /* COND_CUM_COUNT_TRIGGER_H */
```

### `middleware/crc.c`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        crc.c
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

#include "crc.h"


static const uint8_t _CrcHi[] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
// CRC ���ֽ�
static const uint8_t _CrcLo[] =
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

uint16_t CrcModbus(uint8_t *p, uint16_t len)
{
    uint8_t ucCRCHi = 0xFF; /* ��CRC�ֽڳ�ʼ�� */
    uint8_t ucCRCLo = 0xFF; /* ��CRC �ֽڳ�ʼ�� */
    uint16_t usIndex;  /* CRCѭ���е����� */
    while (len--)
    {
        usIndex = ucCRCLo ^ *p++; /* ����CRC */
        ucCRCLo = ucCRCHi ^ _CrcHi[usIndex];
        ucCRCHi = _CrcLo[usIndex];
    }
    return ((uint16_t)ucCRCHi << 8 | ucCRCLo);
}

bool CrcJudge(uint8_t *p, uint16_t len)
{
    uint16_t calc_crc, act_crc;

    act_crc = (p[len - 1] << 8) | (p[len - 2]);
    calc_crc = CrcModbus(p, len - 2);
    if(act_crc == calc_crc)
    {
        return true;
    }
    return false;
}

uint16_t BEBufToUint16(uint8_t *_pBuf)
{
    return (((uint16_t)_pBuf[0] << 8) | _pBuf[1]);
}

void TabLsbToMsb(uint8_t *str, uint16_t len)
{
    int j = 0;

    for(j = 0; j < len / 2; j++)
    {
        str[j] ^= str[len - j - 1];
        str[len - j - 1] ^= str[j];
        str[j] ^= str[len - j - 1];
    }
}

uint8_t MemcpyU16toU8SwapByte(uint8_t* buf, uint16_t* data, uint16_t u8_data_len)
{
    uint16_t i = 0;

    for(i = 0; i < u8_data_len; i ++)
    {
        if(i % 2 == 0)
        {
            buf[i] = data[i/2] >> 8;
        }
        else
        {
            buf[i] = data[i/2] & 0x00FF;
        }

    }
    return 0;
}

uint8_t MemcpyU16toU8(uint8_t* buf, uint16_t* data, uint16_t u8_data_len)
{
    uint16_t i = 0;

    for(i = 0; i < u8_data_len; i ++)
    {
        if(i % 2 == 0)
        {
            buf[i] = data[i/2] & 0x00FF;
        }
        else
        {
            buf[i] = data[i/2] >> 8;
        }

    }
    return 0;
}

uint8_t MemcpyU8toU16(uint16_t* buf, uint16_t* data, uint16_t u16_data_len)
{
    uint16_t i = 0;

    for(i = 0; i < u16_data_len; i ++)
    {
        buf[i] = data[i * 2]| (data[i * 2 + 1]  << 8);
    }
    return 0;
}
```

### `middleware/crc.h`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        crc.h
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

#ifndef __CRC_H_
#define __CRC_H_

#include "F28x_Project.h"

uint16_t CrcModbus(uint8_t *p, uint16_t len);
bool CrcJudge(uint8_t *p, uint16_t len);
uint16_t BEBufToUint16(uint8_t *_pBuf);
void TabLsbToMsb(uint8_t *str, uint16_t len);
uint8_t MemcpyU16toU8SwapByte(uint8_t* buf, uint16_t* data, uint16_t u8_data_len);
uint8_t MemcpyU16toU8(uint8_t* buf, uint16_t* data, uint16_t u8_data_len);
uint8_t MemcpyU8toU16(uint16_t* buf, uint16_t* data, uint16_t u16_data_len);

#endif
```

### `middleware/flag_dly_trigger.c`

```c
/**
 * @file flag_dly_trigger.c
 * @brief Generic debounced flag trigger hook
 */

#include "flag_dly_trigger.h"

void FlagDly_SetHook(void* param)
{
    uint8_t* flag = (uint8_t*)param;

    if(flag != 0) {
        *flag = 1U;
    }
}

void FlagDly_ClearHook(void* param)
{
    uint8_t* flag = (uint8_t*)param;

    if(flag != 0) {
        *flag = 0U;
    }
}

void FlagDly_SetResetHook(void* param)
{
    uint8_t* flag = (uint8_t*)param;

    if(flag != 0) {
        *flag = 0U;
    }
}

void FlagDly_ClearResetHook(void* param)
{
    uint8_t* flag = (uint8_t*)param;

    if(flag != 0) {
        *flag = 1U;
    }
}
```

### `middleware/flag_dly_trigger.h`

```c
/**
 * @file flag_dly_trigger.h
 * @brief Debounced flag trigger macros (DECLARE/RUN/RESET style)
 */

#ifndef FLAG_DLY_TRIGGER_H
#define FLAG_DLY_TRIGGER_H

#include "F28x_Project.h"
#include "cond_cum_countTrigger.h"

void FlagDly_SetHook(void* param);
void FlagDly_ClearHook(void* param);
void FlagDly_SetResetHook(void* param);
void FlagDly_ClearResetHook(void* param);

#define FLAG_DLY_SET_DECLARE(id, p_flag, threshold) \
static CondCum_CountTrigger_t _flag_dly_set_##id = { \
    .trigged = 0, \
    .count = 0, \
    .count_interval = 1, \
    .hook_param = (p_flag), \
    .trig_hook = FlagDly_SetHook, \
    .trig_threshold = (Uint32)(threshold), \
};

#define FLAG_DLY_SET_RUN(id, cmd) \
{cmd} \
CondCum_CountTrigger_Run(&_flag_dly_set_##id, rst);

#define FLAG_DLY_CLEAR_DECLARE(id, p_flag, threshold) \
static CondCum_CountTrigger_t _flag_dly_clear_##id = { \
    .trigged = 0, \
    .count = 0, \
    .count_interval = 1, \
    .hook_param = (p_flag), \
    .trig_hook = FlagDly_ClearHook, \
    .trig_threshold = (Uint32)(threshold), \
};

#define FLAG_DLY_CLEAR_RUN(id, cmd) \
{cmd} \
CondCum_CountTrigger_Run(&_flag_dly_clear_##id, rst);

#define FLAG_DLY_PAIR_DECLARE(id, p_flag, threshold) \
    FLAG_DLY_CLEAR_DECLARE(id, p_flag, threshold) \
    FLAG_DLY_SET_DECLARE(id##_recover, p_flag, threshold)

#define FLAG_DLY_SET_RESET(id, p_flag) \
    do { \
        CondCum_CountTrigger_Reset(&_flag_dly_set_##id); \
        FlagDly_SetResetHook(p_flag); \
    } while(0)

#define FLAG_DLY_CLEAR_RESET(id, p_flag) \
    do { \
        CondCum_CountTrigger_Reset(&_flag_dly_clear_##id); \
        FlagDly_ClearResetHook(p_flag); \
    } while(0)

#endif /* FLAG_DLY_TRIGGER_H */
```

### `middleware/mid_comservice.c`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        mid_comservice.c
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

#include "mid_comservice.h"

#define RET_OK        (0)
#define RET_ERROR     (1)
#define RET_TIMEOUT     (-1)
#define RET_QUEENFULL     (-2)
#define RET_VERIFY_ERROR  (-3)
#define RET_UNUSE_DATA    (2)

static CommPort_t _garrComPort[EN_COMSERVICE_INDEX_NUM] = {0};
static uint8_t _garrRxBuff0[COMSERVICE_BUFF_SIZE] = {0};
static uint8_t _garrRxBuff1[COMSERVICE_BUFF_SIZE] = {0};
static uint8_t _garrRxBuff2[COMSERVICE_BUFF_SIZE] = {0};
const uint8_t _garrModbusFunCode[] = {0x03, 0x04, 0x06, 0x10};

static SciRevData_t _garrSciData[SCI_REV_QUEUE_DEPTH];
static uint8_t _garrRevData[SCI_REV_QUEUE_DEPTH][COMSERVICE_BUFF_SIZE];
Queue_t gstSciRevQueue;

/*ͨ�Ź����Ĭ������*/
static cmoservice_cfg_t _garrComserviceCfg[] =
{
    {
        \
        .frame_delimiter = 0x5A,\
        .frame_delimiter_ack = 0xA5,\
        .c_frame_len = 7,         \
        .c_frame_len_rep = 8,     \
        .c_check_start = 2,        \
        .c_other_len = 2,          \

        .modbus_addr = MODBUS_HOST0_ADDR,\
        .modbus_type = 1,\
        .modbus_slave0_addr = MODBUS_SLAVE0_ADDR,\
        .modbus_slave1_addr = MODBUS_SLAVE1_ADDR,\
        .modbus_slave2_addr = MODBUS_SLAVE2_ADDR,\
    },
    {
        \
        .frame_delimiter = 0x5A,\
        .frame_delimiter_ack = 0xA5,\
        .c_frame_len = 7,         \
        .c_frame_len_rep = 8,     \
        .c_check_start = 2,        \
        .c_other_len = 2,          \

        .modbus_addr = MODBUS_HOST1_ADDR,\
        .modbus_type = 1,\
        .modbus_slave0_addr = MODBUS_SLAVE3_ADDR,\
        .modbus_slave1_addr = MODBUS_SLAVE4_ADDR,\
        .modbus_slave2_addr = MODBUS_SLAVE5_ADDR,\
    },
    {
        \
        .frame_delimiter = 0x5A,\
        .frame_delimiter_ack = 0xA5,\
        .c_frame_len = 7,         \
        .c_frame_len_rep = 8,     \
        .c_check_start = 2,        \
        .c_other_len = 2,          \

        .modbus_addr = MODBUS_HOST2_ADDR,\
        .modbus_type = 1,\
        .modbus_slave0_addr = MODBUS_SLAVE6_ADDR,\
        .modbus_slave1_addr = MODBUS_SLAVE7_ADDR,\
        .modbus_slave2_addr = MODBUS_SLAVE8_ADDR,\
    },
};
static void _MidComserviceComRx(void);
static int32_t _MidComserviceProtocolProcess(uint32_t n_index, uint8_t *p_data, uint32_t n_len);

uint16_t CheckSum(uint8_t *p, uint16_t size)
{
    uint16_t i;
    uint16_t ret = 0;
    for(i = 0; i < size; i++)
    {
        ret += p[i];
    }
    return ret;
}

uint32_t CheckSumFun(uint8_t *pdata, uint16_t len, uint8_t mode)
{
    uint16_t checksum16 = 0;
    uint32_t checksum32 = 0;
    uint16_t i = 0;

    if(mode)
    {
        for(i = 0; i < len; i ++)
        {
            checksum32 += pdata[i];
        }

        return checksum32;
    }
    else
    {
        for(i = 0; i < len; i ++)
        {
            checksum16 += pdata[i];
        }
        return checksum16;
    }
}

/**
* @brief: MidComserviceInit
* @param: void                        
* @note: 
*
*/
void MidComserviceInit(void)
{
    uint8_t i = 0;

    memset((uint8_t*)&_garrComPort, 0, EN_COMSERVICE_INDEX_NUM * sizeof(CommPort_t));
    _garrComPort[EN_COMSERVICE_COM_INDEX0].rx_obj.p_buf   = _garrRxBuff0;
    _garrComPort[EN_COMSERVICE_COM_INDEX0].rx_obj.n_mem_len = COMSERVICE_BUFF_SIZE;
    _garrComPort[EN_COMSERVICE_COM_INDEX1].rx_obj.p_buf   = _garrRxBuff1;
    _garrComPort[EN_COMSERVICE_COM_INDEX1].rx_obj.n_mem_len = COMSERVICE_BUFF_SIZE;
    _garrComPort[EN_COMSERVICE_COM_INDEX2].rx_obj.p_buf   = _garrRxBuff2;
    _garrComPort[EN_COMSERVICE_COM_INDEX2].rx_obj.n_mem_len = COMSERVICE_BUFF_SIZE;
//    _garrComPort[EN_COMSERVICE_COM_INDEX0].tx_obj.p_buf   = _garrTxBuff;
//    _garrComPort[EN_COMSERVICE_COM_INDEX0].tx_obj.n_mem_len = COMSERVICE_BUFF_SIZE;

    QueueInit(&gstSciRevQueue, SCI_REV_QUEUE_DEPTH, sizeof(SciRevData_t));
    for(i = 0; i < SCI_REV_QUEUE_DEPTH; i++)
    {
        _garrSciData[i].pData = _garrRevData[i];
    }
}

//TODO ϵͳ���ж�Ƶ�����ô˺����ᵼ���ж��쳣�����ú����ֲ�������Ϊȫ�ֱ������Խ����ԭ��δ֪
int32_t nn_ret = 0;
uint32_t *np_w_cnt =NULL;
uint32_t nn_r_cnt =NULL;

int32_t mid_comservice_data_in(EN_COMSERVICE_INDEX e_index, uint8_t *p_data, uint32_t n_len)
{
    if(e_index > EN_COMSERVICE_INDEX_NUM)
    {
        nn_ret = RET_ERROR;
    }
    if(p_data == NULL)
    {
        nn_ret = RET_ERROR;
    }

    /*�жϵ�ǰ�ռ��Ƿ��п������д�����*/
    np_w_cnt = &_garrComPort[e_index].rx_obj.n_w_cnt;
    nn_r_cnt = _garrComPort[e_index].rx_obj.n_r_cnt;

//    if(e_index == EN_COMSERVICE_DEBUG_INDEX)
    if(0)
    {
        if(_garrComPort[e_index].rx_obj.p_buf == p_data)
        {
            /*Ϊ�˼��ٻ��࿽����ʱ�䣬�����˲���ͨ�Žӿڵ�ͨ���ڴ�*/
            *np_w_cnt = n_len;
        }
        else if(*np_w_cnt < nn_r_cnt)
        {
            /*дָ���ڶ�ָ��֮ǰ(ѭ����һȦ)*/
            if((*np_w_cnt + n_len) <= nn_r_cnt)
            {
                memcpy(&_garrComPort[e_index].rx_obj.p_buf[*np_w_cnt], p_data, n_len);
                *np_w_cnt += n_len;
            }
            else
            {
                nn_ret = RET_ERROR;
            }
        }
        else
        {
            /*дָ���ڶ�ָ��֮��(δѭ��)*/
            uint32_t n_w_remnant_len = _garrComPort[e_index].rx_obj.n_mem_len - *np_w_cnt;

            if((*np_w_cnt + n_len) <= _garrComPort[e_index].rx_obj.n_mem_len)
            {
                /*���к��пտ��Է�����������*/
                memcpy(&_garrComPort[e_index].rx_obj.p_buf[*np_w_cnt], p_data, n_len);
                *np_w_cnt += n_len;
            }
            else if(((*np_w_cnt + n_len) > _garrComPort[e_index].rx_obj.n_mem_len) &&\
                    ((n_len - n_w_remnant_len) <=  nn_r_cnt))
            {
                /*���к� + ��ָ��֮ǰ�пտ��Է�����������*/
                memcpy(&_garrComPort[e_index].rx_obj.p_buf[*np_w_cnt], p_data, n_w_remnant_len);
                memcpy(_garrComPort[e_index].rx_obj.p_buf, &p_data[n_w_remnant_len], (n_len - n_w_remnant_len));
                *np_w_cnt = n_len - n_w_remnant_len;
            }
            else
            {
                nn_ret = RET_ERROR;
            }
        }
    }
    else
    {
        if(_garrComPort[e_index].rx_obj.p_buf == p_data)
        {
            /*Ϊ�˼��ٻ��࿽����ʱ�䣬�����˲���ͨ�Žӿڵ�ͨ���ڴ�*/
            *np_w_cnt = n_len;
        }
        else if((*np_w_cnt + n_len) < _garrComPort[e_index].rx_obj.n_mem_len)
        {
            memcpy(&_garrComPort[e_index].rx_obj.p_buf[*np_w_cnt], p_data, n_len);
            *np_w_cnt += n_len;
        }
        else
        {
            nn_ret = RET_ERROR;
        }
    }

    return nn_ret;
}

/**
* @brief: MidComserviceHandle
* @param: void                        
* @note: 
*
*/
void MidComserviceHandle(void)
{
    _MidComserviceComRx();
}


/**
* @brief: _MidComserviceComRx
* @param: void                        
* @return: 
* @note: 
*
*/
static void _MidComserviceComRx(void)
{
    uint32_t i = 0;
    int32_t  n_ret       = 0;
    uint32_t n_len_first = 0;   /*��һ�����ݳ���*/
    uint32_t n_len_second = 0;  /*�ڶ����۵����������ݳ���*/

    for(i = 0; i < EN_COMSERVICE_INDEX_NUM; i++)
    {
        if(_garrComPort[i].rx_obj.n_w_cnt != _garrComPort[i].rx_obj.n_r_cnt)
        {
            if(_garrComPort[i].rx_obj.n_w_cnt > _garrComPort[i].rx_obj.n_r_cnt)
            {
                /*дָ���ڶ�ָ��֮��*/
                n_len_first = _garrComPort[i].rx_obj.n_w_cnt - _garrComPort[i].rx_obj.n_r_cnt;
            }
            else
            {
                /*дָ���ڶ�ָ��֮ǰ*/
                n_len_first   = _garrComPort[i].rx_obj.n_mem_len - _garrComPort[i].rx_obj.n_r_cnt;
                n_len_second  = _garrComPort[i].rx_obj.n_w_cnt;
            }

//            if(i == EN_COMSERVICE_DEBUG_INDEX)
            if(0)
            {
                /*debug ����*/
                if(n_len_first != 0)
                {
                    //TODO  debug
//                    console_recive_handler(&_garrComPort[i].rx_obj.p_buf[_garrComPort[i].rx_obj.n_r_cnt], n_len_first);
                    _garrComPort[i].rx_obj.n_r_cnt += n_len_first;
                }
                if(n_len_second != 0)
                {
                    //TODO  debug
//                    console_recive_handler(_garrComPort[i].rx_obj.p_buf, n_len_second);
                    _garrComPort[i].rx_obj.n_r_cnt = n_len_second;
                }
            }
            else
            {
                n_ret = _MidComserviceProtocolProcess(i, _garrComPort[i].rx_obj.p_buf, _garrComPort[i].rx_obj.n_w_cnt);
                if(n_ret <= 0)
                {
                    _garrComPort[i].rx_obj.n_w_cnt = 0;
                    _garrComPort[i].rx_obj.n_r_cnt = 0;
                }
            }

        }
    }
}

/**
* @brief: _MidComserviceProtocolProcess
* @param: n_index                     
* @param: p_data                      
* @param: n_len                       
* @return: 
* @note: 
*
*/
static int32_t _MidComserviceProtocolProcess(uint32_t n_index, uint8_t *p_data, uint32_t n_len)
{
    int32_t n_ret = 1;
    uint32_t n_loop = 1;
    static uint8_t rev_index = 0;
    static uint8_t arr_wait_flag[EN_COMSERVICE_INDEX_NUM] = {0, 0};
//    uint8_t *p_found_funcode = NULL;

    do
    {
        n_loop--;

        switch(_garrComPort[n_index].protocol_obj.e_state)
        {
            case EN_PROTOCOL_STATE_IDLE:
            {
                if(!arr_wait_flag[n_index])
                {
                    arr_wait_flag[n_index] = 1;
                    _garrComPort[n_index].n_timeout = TimGetSystime();
                }
                
                /* ȷ�� ��ͷ�Ƿ�һ�� */
                if(n_len < 3)
                {
                    n_ret = RET_UNUSE_DATA;
                }

                else if(p_data[0] == _garrComserviceCfg[n_index].frame_delimiter)
                {
                    if(p_data[2] == COM_ADDR)
                    {
                        /*�����*/
                        _garrComPort[n_index].pack_request_type = _garrComserviceCfg[n_index].frame_delimiter;
                        /* ��ͷУ����ȷ����һ�� */
                        _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_FRAME_COLLECT_COM;
                        _garrComPort[n_index].n_timeout = TimGetSystime();
                        n_loop++;
                    }
                    else
                    {
                        /*��ͷУ�����,ֱ�����*/
                        n_ret = RET_TIMEOUT;
                        arr_wait_flag[n_index] = 0;
                    }

                }
                else if(p_data[0] == _garrComserviceCfg[n_index].frame_delimiter_ack)
                {
                    
                    if(p_data[2] == COM_ADDR)
                    {
                        /*��Ӧ��*/
                        _garrComPort[n_index].pack_request_type = _garrComserviceCfg[n_index].frame_delimiter_ack;
                        /* ��ͷУ����ȷ����һ�� */
                        _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_FRAME_COLLECT_COM;
                        _garrComPort[n_index].n_timeout = TimGetSystime();
                        n_loop++;
                    } 
                    else
                    {
                        /*��ͷУ�����,ֱ�����*/
                        n_ret = RET_TIMEOUT;
                        arr_wait_flag[n_index] = 0;
                    }                    
                }
                else if(memchr(_garrModbusFunCode, p_data[1], sizeof(_garrModbusFunCode)) != NULL)   //ȷ���յ�������
                {
                    /*������Ϊ��������*/
                    if(_garrComserviceCfg[n_index].modbus_type)
                    {
                        if((p_data[0] == _garrComserviceCfg[n_index].modbus_slave0_addr) || \
                           (p_data[0] == _garrComserviceCfg[n_index].modbus_slave1_addr) || \
                           (p_data[0] == _garrComserviceCfg[n_index].modbus_slave2_addr))
                        {
                            _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_FRAME_COLLECT_MOD;
                            _garrComPort[n_index].n_timeout = TimGetSystime();
                            n_loop++;                            
                        }
                        else
                        {
                            /*�Ƿ��ʱ��ڵ�*/
                            n_ret = RET_TIMEOUT;
                            arr_wait_flag[n_index] = 0;
                        }

                    }
                    /*������Ϊ�ӻ�����*/
                    else
                    {
                        if(p_data[0] == _garrComserviceCfg[n_index].modbus_addr)
                        {
                            _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_FRAME_COLLECT_MOD;
                            _garrComPort[n_index].n_timeout = TimGetSystime();
                            n_loop++;                            
                        }
                        else
                        {
                            /*�Ƿ��ʱ��ڵ�*/
                            n_ret = RET_TIMEOUT;
                            arr_wait_flag[n_index] = 0;
                        }
                    }
                }
                else
                {

                }

                /*�ȴ��������ݳ�ʱ ����3�ֽ� 9600��������Ҫ4ms*/
                if(TimDifftimeFromNow(_garrComPort[n_index].n_timeout) > 10)
                {
                    n_ret = RET_TIMEOUT;
                    arr_wait_flag[n_index] = 0;
                } 
                /*����������ͷ*/
                if(_garrComPort[n_index].protocol_obj.e_state != EN_PROTOCOL_STATE_IDLE)
                {
                    _garrComPort[n_index].n_timeout = TimGetSystime();
                    arr_wait_flag[n_index] = 0;
                }
                _garrComPort[n_index].protocol_obj.n_pake_len = 0;
                
                break;
            }
            case EN_PROTOCOL_STATE_FRAME_COLLECT_COM:
            {
                if((n_index == EN_COMSERVICE_COM_INDEX0) || (n_index == EN_COMSERVICE_COM_INDEX1) || (n_index == EN_COMSERVICE_COM_INDEX2))
                //if((n_index == EN_COMSERVICE_COM_INDEX0) || (n_index == EN_COMSERVICE_COM_INDEX1))
                {
                    /*�����*/
                    if(_garrComPort[n_index].pack_request_type == _garrComserviceCfg[n_index].frame_delimiter)
                    {
                        /* �Ƿ����Э�������� */
                        if(n_len >= _garrComserviceCfg[n_index].c_frame_len)
                        {
                            
                            _garrComPort[n_index].protocol_obj.n_payload_len = p_data[5] + ((uint16_t)p_data[6] << 8);

                            /*cmd���λΪ1����ʾд����*/
                            if((p_data[4] >> 7) & 0x01)
                            {
                                _garrComPort[n_index].e_pack_operation_type = EN_PROTOCOL_CODE_WRITE;
                            }
                            else
                            {
                                /*cmd���λΪ0����ʾ������*/
                                _garrComPort[n_index].e_pack_operation_type = EN_PROTOCOL_CODE_READ;
                            }
                            /*��¼Э����payload����*/
                            _garrComPort[n_index].protocol_obj.n_pake_len += _garrComserviceCfg[n_index].c_frame_len;
                            _garrComPort[n_index].protocol_obj.n_pake_len += _garrComPort[n_index].protocol_obj.n_payload_len;
                            _garrComPort[n_index].protocol_obj.n_pake_len += _garrComserviceCfg[n_index].c_other_len;
                            _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_DATA_COLLECT_COM;
                            _garrComPort[n_index].n_timeout = TimGetSystime();
                            n_loop++;
                        }
                    }
                    /*��Ӧ��*/
                    else if(_garrComPort[n_index].pack_request_type == _garrComserviceCfg[n_index].frame_delimiter_ack)
                    {
                        /* �Ƿ����Э�������ȣ���Ӧ���� */
                        if(n_len >= _garrComserviceCfg[n_index].c_frame_len_rep)
                        {
                            /*cmd���λΪ1����ʾд��Ӧ��*/
                            if((p_data[4] >> 7) & 0x01)
                            {
                                _garrComPort[n_index].e_pack_operation_type = EN_PROTOCOL_CODE_WRITE;
                                /*��¼Э����payload����*/
                                _garrComPort[n_index].protocol_obj.n_pake_len += _garrComserviceCfg[n_index].c_frame_len_rep;
                                /*д��Ӧ7��8�ֽھ���У��ͣ��������ﲻ�ܼ���У��͵ĳ���*/
//                                    _garrComPort[n_index].protocol_obj.n_pake_len += _garrComserviceCfg[n_index].c_other_len;
                                _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_DATA_COLLECT_COM;
                                _garrComPort[n_index].n_timeout = TimGetSystime();                                   
                                n_loop++;
                            }
                            else
                            {
                                /*cmd���λΪ0����ʾ����Ӧ��*/
                                _garrComPort[n_index].e_pack_operation_type = EN_PROTOCOL_CODE_READ;
                                _garrComPort[n_index].protocol_obj.n_payload_len = p_data[6] + ((uint16_t)p_data[7] << 8);

                                /*��¼Э����payload����*/
                                _garrComPort[n_index].protocol_obj.n_pake_len += _garrComserviceCfg[n_index].c_frame_len_rep;
                                _garrComPort[n_index].protocol_obj.n_pake_len += _garrComPort[n_index].protocol_obj.n_payload_len;
                                _garrComPort[n_index].protocol_obj.n_pake_len += _garrComserviceCfg[n_index].c_other_len;
                                _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_DATA_COLLECT_COM;
                                _garrComPort[n_index].n_timeout = TimGetSystime();                         
                                n_loop++;
                            }
                        }
                    }  

                }

                //һ֡100ms��ʱ
                if(TimDifftimeFromNow(_garrComPort[n_index].n_timeout) > 30)
                {
                    n_ret = RET_TIMEOUT;
                    n_loop = 0;
                }
                break;
            }
            case EN_PROTOCOL_STATE_FRAME_COLLECT_MOD:
            {
                /*������Ϊ��������*/
                if(_garrComserviceCfg[n_index].modbus_type)
                {
                    if((p_data[1] == 0x10) || (p_data[1] == 0x06))
                    {
                        /*0x10 | ��ַ | ������ | ��ʼ��ַ | �Ĵ������� | CRC |*/
                        /*0x06| ��ַ��1�ֽڣ� | �����루0x06�� | �Ĵ�����ַ��2�ֽڣ� | д��ֵ��2�ֽڣ� | CRCУ�飨2�ֽڣ� |*/
                        _garrComPort[n_index].protocol_obj.n_pake_len = 8;
                    }
                    else
                    {
                        /*0x03 0x04*/
                       /* | ��ַ��1�ֽڣ� | �����루0x03�� | �ֽ�����1�ֽڣ� | �Ĵ���ֵ��N�ֽڣ� | CRCУ�飨2�ֽڣ� |*/
                        _garrComPort[n_index].protocol_obj.n_pake_len = p_data[2] + 5;
                    }

                    _garrComPort[n_index].n_timeout = TimGetSystime();
                    _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_DATA_COLLECT_MOD;
                    n_loop++;
                }
                /*������Ϊ�ӻ�����*/
                else
                {
                    // Modbus 0x03  0x04 0x06Э�鳤��Ϊ8���ֽ�   0x10��data_len ��7���ֽ�
                    if(n_len >= 8)
                    {
                        if((n_index == EN_COMSERVICE_COM_INDEX0) || (n_index == EN_COMSERVICE_COM_INDEX1) || (n_index == EN_COMSERVICE_COM_INDEX2))
                        //if((n_index == EN_COMSERVICE_COM_INDEX0) || (n_index == EN_COMSERVICE_COM_INDEX1))
                        {
                            /* ���㸺�������� */
                            _garrComPort[n_index].protocol_obj.n_payload_len = p_data[6];
                        }

                        /*��¼Э����payload����*/
                        if(p_data[1] == 0x10)
                        {
                            // 0x10�Ĵ���7���ֽڰ�ͷ+2�ֽ�У��+data
                            _garrComPort[n_index].protocol_obj.n_pake_len = _garrComPort[n_index].protocol_obj.n_payload_len + 9;
                        }
                        else
                        {
                            _garrComPort[n_index].protocol_obj.n_pake_len = 8;
                        }
                        _garrComPort[n_index].n_timeout = TimGetSystime();
                        _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_DATA_COLLECT_MOD;
                        n_loop++;
                    }
                }

                //һ֡100ms��ʱ
                if(TimDifftimeFromNow(_garrComPort[n_index].n_timeout) > 100)
                {
                    n_ret = RET_TIMEOUT;
                    n_loop = 0;
                }
                break;
            }
            case EN_PROTOCOL_STATE_DATA_COLLECT_COM:
            {
                if(n_len >= _garrComPort[n_index].protocol_obj.n_pake_len)
                {
                    //TODO �������
                    if(QueueIsFull(&gstSciRevQueue) == true)
                    {
                        n_ret = RET_QUEENFULL;
                    }
                    else
                    {
                        if((_garrComPort[n_index].pack_request_type == _garrComserviceCfg[n_index].frame_delimiter) && (_garrComPort[n_index].e_pack_operation_type == EN_PROTOCOL_CODE_WRITE))
                        {
                            _garrSciData[rev_index].data_type = TYPE_COM_WREQUEST_DATA;
//                            printf("TYPE_COM_WREQUEST_DATA\r\n");
                        }
                        else if((_garrComPort[n_index].pack_request_type == _garrComserviceCfg[n_index].frame_delimiter) && (_garrComPort[n_index].e_pack_operation_type == EN_PROTOCOL_CODE_READ))
                        {
                            _garrSciData[rev_index].data_type = TYPE_COM_RREQUEST_DATA;
//                            printf("TYPE_COM_RREQUEST_DATA\r\n");
                        }
                        else if((_garrComPort[n_index].pack_request_type == _garrComserviceCfg[n_index].frame_delimiter_ack) && (_garrComPort[n_index].e_pack_operation_type == EN_PROTOCOL_CODE_WRITE))
                        {
                            _garrSciData[rev_index].data_type = TYPE_COM_WRESPONSE_DATA;
//                            printf("TYPE_COM_WRESPONSE_DATA\r\n");
                        }
                        else if((_garrComPort[n_index].pack_request_type == _garrComserviceCfg[n_index].frame_delimiter_ack) && (_garrComPort[n_index].e_pack_operation_type == EN_PROTOCOL_CODE_READ))
                        {
                            _garrSciData[rev_index].data_type = TYPE_COM_RRESPONSE_DATA;
//                            printf("TYPE_COM_RRESPONSE_DATA\r\n");
                        }

                        _garrSciData[rev_index].sci_index = (EN_COMSERVICE_INDEX)n_index;
                        _garrSciData[rev_index].data_size = _garrComPort[n_index].protocol_obj.n_pake_len;
                        n_ret = RET_OK;
                    }
                    n_loop = 0;
                }
                else
                {
                    //һ֡100ms��ʱ
                    if(TimDifftimeFromNow(_garrComPort[n_index].n_timeout) > 300)
                    {
                        n_ret = RET_TIMEOUT;
                        n_loop = 0;
                    }
                }
                break;
            }
            case EN_PROTOCOL_STATE_DATA_COLLECT_MOD:
            {
                if(n_len >= _garrComPort[n_index].protocol_obj.n_pake_len)
                {
                    //TODO �������
                    if(CrcJudge(p_data, _garrComPort[n_index].protocol_obj.n_pake_len) == false)
                    {
                        n_ret = RET_VERIFY_ERROR;
                    }
                    else
                    {
                        if(QueueIsFull(&gstSciRevQueue) == true)
                        {
                            n_ret = RET_QUEENFULL;
                        }
                        else
                        {
                            /*������Ϊ��������*/
                            if(_garrComserviceCfg[n_index].modbus_type)
                            {
                                _garrSciData[rev_index].data_type = TYPE_MODBUS_HOST_DATA;
//                                printf("TYPE_MODBUS_HOST_DATA\r\n");
                            }
                            /*������Ϊ�ӻ�����*/
                            else
                            {
                                _garrSciData[rev_index].data_type = TYPE_MODBUS_SLAVE_DATA;
//                                printf("TYPE_MODBUS_SLAVE_DATA\r\n");
                            }

                            _garrSciData[rev_index].sci_index = (EN_COMSERVICE_INDEX)n_index;
                            _garrSciData[rev_index].slave_addr = p_data[0];
                            _garrSciData[rev_index].data_size = _garrComPort[n_index].protocol_obj.n_pake_len;
                            n_ret = RET_OK;
                        }
                    }
                    n_loop = 0;
                    }
                    else
                    {
                        //һ֡500ms��ʱ
                        if(TimDifftimeFromNow(_garrComPort[n_index].n_timeout) > 500)
                        {
                            n_ret = RET_TIMEOUT;
                            n_loop = 0;
                        }
                    }

                break;
            }
            case EN_PROTOCOL_TIMEOUT:
            {
                if(TimDifftimeFromNow(_garrComPort[n_index].n_timeout) > 50)
                {
                    n_ret = RET_TIMEOUT;
                    n_loop = 0;
                }
                else
                {
                    n_loop++;
                }
                break;
            }
            default:
                break;
        }
    }
    while(n_loop != 0);

    if(n_ret <= RET_OK)
    {
        if(n_ret == RET_OK)
        {
            memcpy(_garrSciData[rev_index].pData, p_data, (_garrSciData[rev_index].data_size <= COMSERVICE_BUFF_SIZE)?_garrSciData[rev_index].data_size:COMSERVICE_BUFF_SIZE);
            if(QueuePut(&gstSciRevQueue, &_garrSciData[rev_index]) == false)
            {
                n_ret = RET_QUEENFULL;
            }
            rev_index++;
            rev_index %= SCI_REV_QUEUE_DEPTH;
        }
        _garrComPort[n_index].protocol_obj.e_state = EN_PROTOCOL_STATE_IDLE;
    }
    return n_ret;
}
```

### `middleware/mid_comservice.h`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        mid_comservice.h
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


#ifndef __MID_COMSERVICE_H__
#define __MID_COMSERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "F28x_Project.h"
#include "cpu_timers.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "queue.h"
#include "crc.h"
#include "sci.h"

#define SCI_REV_QUEUE_DEPTH     (3)
//#define SCI_REV_QUEUE_DEPTH     (2)
#define COMSERVICE_BUFF_SIZE    (2112)
#define MODBUS_HOST0_ADDR        (0x01)
#define MODBUS_HOST1_ADDR        (0x01)
#define MODBUS_HOST2_ADDR        (0x01)

#define MODBUS_SLAVE0_ADDR      (0x05)  /*��һ·���� slave0-slave2*/   //arc
#define MODBUS_SLAVE1_ADDR      (0x05)
#define MODBUS_SLAVE2_ADDR      (0x05)
#define MODBUS_SLAVE3_ADDR      (0x01)  /*�ڶ�·���� slave3-slave5*/  //memter
#define MODBUS_SLAVE4_ADDR      (0x01)
#define MODBUS_SLAVE5_ADDR      (0x01)

//TODO:��·���ڴӻ���ַ��ȷ��
#define MODBUS_SLAVE6_ADDR      (0x81) /*����·���� slave6-slave8*/  //MCUЭ������
#define MODBUS_SLAVE7_ADDR      (0x81)
#define MODBUS_SLAVE8_ADDR      (0x81)

#define COM_ADDR                (0x00)


typedef enum
{
    EN_COMSERVICE_COM_INDEX0,       // ARC
    EN_COMSERVICE_COM_INDEX1,       // menter
    EN_COMSERVICE_COM_INDEX2,       // MCUЭ������
	EN_COMSERVICE_INDEX_NUM,
    EN_COMSERVICE_INDEX_INVALID = 0xff,
} EN_COMSERVICE_INDEX;

typedef enum
{
    EN_PROTOCOL_STATE_IDLE = 0,         /**/
    EN_PROTOCOL_STATE_FRAME_COLLECT_COM,    /**/
	EN_PROTOCOL_STATE_FRAME_COLLECT_MOD,    /**/
    EN_PROTOCOL_STATE_DATA_COLLECT_COM,     /**/
	EN_PROTOCOL_STATE_DATA_COLLECT_MOD,     /**/
    EN_PROTOCOL_STATE_RESPONSE,         /**/
	EN_PROTOCOL_TIMEOUT,
}en_protocol_state_t;

typedef enum
{
    EN_PROTOCOL_CODE_READ = 0,        /**/
    EN_PROTOCOL_CODE_WRITE,           /**/
}en_protocol_code_t;

typedef struct
{
    uint8_t  *p_buf;       /*����ָ��*/
    uint32_t  n_mem_len;   /*�ڴ��С*/
    uint32_t  n_w_cnt;     /*���ݶ�����*/
    uint32_t  n_r_cnt;     /*����д����*/
}transmission_obj_t;

typedef struct
{
    en_protocol_state_t e_state;         /*Э�鴦���״̬*/
    en_protocol_code_t  e_code;          /*�������*/
    uint32_t            n_payload_reg;   /*�û����ݼĴ���*/
    uint32_t            n_payload_len;   /*�û��������ݳ���*/
    uint32_t            n_request_len;   /*�û����󳤶�*/
    uint32_t            n_pake_len;      /*���ݰ��ܳ���*/
    uint32_t            n_pake_code;     /*���ݰ�������*/
    uint32_t            n_pake_ack;      /*��Ӧ��ack��*/
    uint32_t            n_dev_index;     /*ͨ�ŵ�������*/
    int32_t (*pfun_read_process) (uint8_t *p_data, uint32_t n_reg, uint16_t n_len, uint8_t *p_w_data, uint16_t *pn_w_len);  /*��Э�鴦�����*/
    int32_t (*pfun_write_process)(uint8_t *p_data, uint32_t n_reg, uint16_t n_len, uint8_t *p_w_data, uint16_t *pn_w_len);  /*дЭ�鴦�����*/
}protocol_obj_t;

typedef struct
{
	uint8_t  rx_complete_flag;

    protocol_obj_t     protocol_obj;                                          /*Э�鴦�����ݶ���*/
//    int32_t (*pfun_tx[COMSERVICE_DOWN_NUM])(uint32_t n_index, uint8_t *p_data, uint32_t n_len);    /*���ͻص�*/
//    uint32_t n_txfun_index[COMSERVICE_DOWN_NUM];                              /*��Ӧ���ͽӿڵ�����*/
    uint32_t           n_rx_index;                                            /*��Ӧ���յĽӿ�������DMA��Ҫ*/
    uint32_t           n_dev_index;                                           /*����ͨ�ŵ�������*/
    uint8_t            c_downup_flag;                                         /*ת��ͨ���ı�־*/
    transmission_obj_t rx_obj;                                                /*�������ݶ���*/
//    transmission_obj_t tx_obj;                                                /*�������ݶ���*/
    uint32_t           n_timeout;
    uint8_t            pack_request_type;                                         /* 0x5A:���� 0xA5����Ӧ */
    en_protocol_code_t e_pack_operation_type;                                     
}CommPort_t;
//
typedef struct
{
    uint8_t    frame_delimiter;   /*frame const string*/
    uint8_t    frame_delimiter_ack;   /*frame const string*/
    uint8_t     c_frame_len;        /*�������̰�ͷ*/
    uint8_t     c_frame_len_rep;        /*��Ӧ����̰�ͷ*/
    uint8_t     c_check_start;      /*У����ʼλ��*/
    uint8_t     c_other_len;        /*У��ͳ���*/

    uint8_t     modbus_addr;        /*modbus������ַ */
    uint8_t     modbus_type;        /*0���ӻ�       1������*/
    uint8_t     modbus_slave0_addr;        /*modbus�ӻ���ַ */
    uint8_t     modbus_slave1_addr;        /*modbus�ӻ���ַ */
    uint8_t     modbus_slave2_addr;        /*modbus�ӻ���ַ */
}cmoservice_cfg_t;


typedef struct  /*��ͷ7�ֽ�*/
{
    uint8_t  packet_head ;      /* �̶�Ϊ0x5A */
    uint8_t  send_addr;         /* ���ͷ��ĵ�ַ */
    uint8_t  recv_addr;         /* ���շ��ĵ�ַ */
    uint16_t cmd_low;               /* ��ͬcmd��Ӧ�����ݸ�ʽ��ͬ */
	uint16_t cmd_high;               /* ��ͬcmd��Ӧ�����ݸ�ʽ��ͬ */
    uint16_t data_field_len_low;    /* ��д��������ݳ��ȣ��ֽ�Ϊ��λ */
    uint16_t data_field_len_high;    /* ��д��������ݳ��ȣ��ֽ�Ϊ��λ */
    uint16_t  *data_field;       /* data�� */
    uint16_t check_sum_low;         /* У���() */
    uint16_t check_sum_high;         /* У���() */
} protocal_request_packet_t;


/* protocal response packet */
typedef struct  /*��ͷ �ֽ�*/
{
	uint16_t  packet_head ;      /* �̶�Ϊ0xA5 */
	uint16_t  send_addr;         /* ���ͷ��ĵ�ַ */
	uint16_t  recv_addr;         /* ���շ��ĵ�ַ */
    uint16_t cmd_low;               /* ��ͬcmd��Ӧ�����ݸ�ʽ��ͬ */
	uint16_t cmd_high;               /* ��ͬcmd��Ӧ�����ݸ�ʽ��ͬ */
	uint16_t  error_code;        /* error  */
    uint16_t check_sum_low;         /* У���() */
    uint16_t check_sum_high;         /* У���() */
} protocal_response_packet_t;

typedef struct  _protocal_response_stru_t/*��ͷ �ֽ�*/
{
	uint16_t  packet_head ;      /* �̶�Ϊ0xA5 */
	uint16_t  send_addr;         /* ���ͷ��ĵ�ַ */
	uint16_t  recv_addr;         /* ���շ��ĵ�ַ */
    uint16_t cmd_low;               /* ��ͬcmd��Ӧ�����ݸ�ʽ��ͬ */
    uint16_t cmd_high;               /* ��ͬcmd��Ӧ�����ݸ�ʽ��ͬ */
    uint16_t  error_code;        /* error  */
    uint16_t data_field_len_low;    /* ��д��������ݳ��ȣ��ֽ�Ϊ��λ */
    uint16_t data_field_len_high;    /* ��д��������ݳ��ȣ��ֽ�Ϊ��λ */
}ProtocalReadResponsePacket_t;

typedef enum
{
	TYPE_MODBUS_HOST_DATA,      //������Ϊ�������յ�������
    TYPE_MODBUS_SLAVE_DATA,     //������Ϊ�ӻ����յ�������
    TYPE_COM_RRESPONSE_DATA,
    TYPE_COM_WRESPONSE_DATA,
    TYPE_COM_RREQUEST_DATA,
    TYPE_COM_WREQUEST_DATA,
	TYPE_COM_DATA,
}EN_UART_DATA_TYPE;

typedef struct _comm_host_rev_data_stru_t
{
    EN_COMSERVICE_INDEX         sci_index;
    uint8_t                     slave_addr;
    EN_UART_DATA_TYPE           data_type;
	uint16_t			        data_size;
	uint8_t				        *pData;
}SciRevData_t;




extern Queue_t	gstSciRevQueue;


void MidComserviceInit(void);
int32_t mid_comservice_data_in(EN_COMSERVICE_INDEX e_index, uint8_t *p_data, uint32_t n_len);
void MidComserviceHandle(void);
void ScidRxHandler(uint8_t data);
void ScidSendMsgToBuff(const char* format, ...);
void ScidSendMsg();
uint16_t CheckSum(uint8_t *p, uint16_t size);   
uint32_t CheckSumFun(uint8_t *pdata, uint16_t len, uint8_t mode);

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================
```

### `middleware/mid_filter.c`

```c
/**
 * @file mid_filter.c
 * @brief First/second-order Butterworth low-pass filters
 */

#include "mid_filter.h"

#define MID_FILTER_PI           (3.1415926f)
#define MID_FILTER_BUTTER2_ZETA (0.707106781f)

void Mid_Filter1OrderButter_Init(Mid_Filter1OrderButter_t* filter, float32 cutoff_hz, float32 sample_hz)
{
    float32 wt;
    float32 den;

    if ((filter == NULL) || (cutoff_hz <= 0.0f) || (sample_hz <= 0.0f))
    {
        return;
    }

    /* Bilinear transform: yn = a0*x(n) + a1*x(n-1) - b1*y(n-1) */
    wt = 2.0f * MID_FILTER_PI * cutoff_hz / sample_hz;
    den = wt + 2.0f;
    filter->a0 = wt / den;
    filter->a1 = filter->a0;
    filter->b1 = (wt - 2.0f) / den;

    Mid_Filter1OrderButter_Reset(filter);
}

void Mid_Filter1OrderButter_Reset(Mid_Filter1OrderButter_t* filter)
{
    if (filter == NULL)
    {
        return;
    }

    filter->yn = 0.0f;
    filter->yn_last1 = 0.0f;
    filter->xn_last1 = 0.0f;
}

float32 Mid_Filter1OrderButter_Update(Mid_Filter1OrderButter_t* filter, float32 input)
{
    if (filter == NULL)
    {
        return input;
    }

    filter->yn = input * filter->a0 + filter->a1 * filter->xn_last1 - filter->b1 * filter->yn_last1;
    filter->xn_last1 = input;
    filter->yn_last1 = filter->yn;

    return filter->yn;
}

void Mid_Filter2OrderButter_Init(Mid_Filter2OrderButter_t* filter, float32 cutoff_hz, float32 sample_hz)
{
    float32 wc;
    float32 ts;
    float32 b0;
    float32 a0;
    float32 a1;
    float32 a2;

    if ((filter == NULL) || (cutoff_hz <= 0.0f) || (sample_hz <= 0.0f))
    {
        return;
    }

    /* Same coefficient design as Second_Order_ButterWorth in cla_control.cla */
    wc = 2.0f * MID_FILTER_PI * cutoff_hz;
    ts = 1.0f / sample_hz;
    b0 = wc * wc * ts * ts;
    a0 = b0 + 4.0f * MID_FILTER_BUTTER2_ZETA * wc * ts + 4.0f;
    a1 = 2.0f * b0 - 8.0f;
    a2 = b0 - 4.0f * MID_FILTER_BUTTER2_ZETA * wc * ts + 4.0f;

    filter->nb0 = b0 / a0;
    filter->nb1 = 2.0f * filter->nb0;
    filter->na1 = a1 / a0;
    filter->na2 = a2 / a0;

    Mid_Filter2OrderButter_Reset(filter);
}

void Mid_Filter2OrderButter_Reset(Mid_Filter2OrderButter_t* filter)
{
    if (filter == NULL)
    {
        return;
    }

    filter->xn_last1 = 0.0f;
    filter->xn_last2 = 0.0f;
    filter->yn = 0.0f;
    filter->yn_last1 = 0.0f;
    filter->yn_last2 = 0.0f;
}

float32 Mid_Filter2OrderButter_Update(Mid_Filter2OrderButter_t* filter, float32 input)
{
    if (filter == NULL)
    {
        return input;
    }

    filter->yn = filter->nb0 * (filter->xn_last2 + input)
               + filter->nb1 * filter->xn_last1
               - filter->na1 * filter->yn_last1
               - filter->na2 * filter->yn_last2;

    filter->xn_last2 = filter->xn_last1;
    filter->xn_last1 = input;
    filter->yn_last2 = filter->yn_last1;
    filter->yn_last1 = filter->yn;

    return filter->yn;
}
```

### `middleware/mid_filter.h`

```c
/**
 * @file mid_filter.h
 * @brief First/second-order Butterworth low-pass filters
 */

#ifndef MID_FILTER_H
#define MID_FILTER_H

#include "F28x_Project.h"

typedef struct mid_filter_1order_butter_stru_t
{
    float32 a0;
    float32 a1;
    float32 b1;
    float32 yn;
    float32 yn_last1;
    float32 xn_last1;
} Mid_Filter1OrderButter_t;

typedef struct mid_filter_2order_butter_stru_t
{
    float32 nb0;
    float32 nb1;
    float32 na1;
    float32 na2;
    float32 xn_last1;
    float32 xn_last2;
    float32 yn;
    float32 yn_last1;
    float32 yn_last2;
} Mid_Filter2OrderButter_t;

void Mid_Filter1OrderButter_Init(Mid_Filter1OrderButter_t* filter, float32 cutoff_hz, float32 sample_hz);
void Mid_Filter1OrderButter_Reset(Mid_Filter1OrderButter_t* filter);
float32 Mid_Filter1OrderButter_Update(Mid_Filter1OrderButter_t* filter, float32 input);

void Mid_Filter2OrderButter_Init(Mid_Filter2OrderButter_t* filter, float32 cutoff_hz, float32 sample_hz);
void Mid_Filter2OrderButter_Reset(Mid_Filter2OrderButter_t* filter);
float32 Mid_Filter2OrderButter_Update(Mid_Filter2OrderButter_t* filter, float32 input);

#endif /* MID_FILTER_H */
```

### `middleware/mid_sliding_filter.c`

```c
/**
 * @file mid_sliding_filter.c
 * @brief Sliding-window moving-average filter
 */

#include "mid_sliding_filter.h"

void Mid_SlidingFilter_Init(Mid_SlidingFilter_t* filter, float32* buffer, uint16_t win_size)
{
    uint16_t i;

    if ((filter == NULL) || (buffer == NULL) || (win_size == 0U))
    {
        return;
    }

    filter->buffer = buffer;
    filter->win_size = win_size;

    for (i = 0U; i < win_size; i++)
    {
        filter->buffer[i] = 0.0f;
    }

    Mid_SlidingFilter_Reset(filter);
}

void Mid_SlidingFilter_Reset(Mid_SlidingFilter_t* filter)
{
    if (filter == NULL)
    {
        return;
    }

    filter->index = 0U;
    filter->is_full = 0U;
    filter->sum = 0.0f;
}

float32 Mid_SlidingFilter_Update(Mid_SlidingFilter_t* filter, float32 sample)
{
    uint16_t count;

    if ((filter == NULL) || (filter->buffer == NULL) || (filter->win_size == 0U))
    {
        return sample;
    }

    if (filter->is_full != 0U)
    {
        filter->sum -= filter->buffer[filter->index];
    }

    filter->sum += sample;
    filter->buffer[filter->index] = sample;
    filter->index++;

    if (filter->index >= filter->win_size)
    {
        filter->index = 0U;
        filter->is_full = 1U;
    }

    count = (filter->is_full != 0U) ? filter->win_size : filter->index;
    if (count == 0U)
    {
        return sample;
    }

    return filter->sum / (float32)count;
}
```

### `middleware/mid_sliding_filter.h`

```c
/**
 * @file mid_sliding_filter.h
 * @brief Sliding-window moving-average filter (caller-owned buffer)
 */

#ifndef MID_SLIDING_FILTER_H
#define MID_SLIDING_FILTER_H

#include "F28x_Project.h"

typedef struct mid_sliding_filter_stru_t
{
    float32* buffer;
    uint16_t   win_size;
    uint16_t   index;
    uint16_t   is_full;
    float32  sum;
} Mid_SlidingFilter_t;

void Mid_SlidingFilter_Init(Mid_SlidingFilter_t* filter, float32* buffer, uint16_t win_size);
void Mid_SlidingFilter_Reset(Mid_SlidingFilter_t* filter);
float32 Mid_SlidingFilter_Update(Mid_SlidingFilter_t* filter, float32 sample);

#endif /* MID_SLIDING_FILTER_H */
```

### `middleware/queue.c`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        queue.c
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

#include "queue.h"



/**
* @brief: QueueInit
* @param: obj                         
* @param: queue_depth                 
* @param: queue_len                   
* @return: 
* @note: queue_len��ʾÿ�����еĴ�С��queue_depth��ʾ��Ҫ���ö��ٶ�����ȣ���ҪΪ����pData����queue_depth * queue_len��С���ڴ�ռ�
*
*/
bool QueueInit(Queue_t *obj, uint16_t queue_depth, uint32_t queue_len)
{
    obj->msg_in = 0;
    obj->msg_out = 0;
    obj->valid_num = 0;
    obj->depth = queue_depth;
    obj->len_per_queue = queue_len;

    obj->pData = NULL;

    obj->pData = (uint8_t *)malloc(queue_len * queue_depth);/*ʹ�öѿռ䣬��Ҫ��.s�ļ���heap_size�ĳɺ���ֵ��Ҳ���Բ���malloc�����ô�����ķ�ʽ*/

    if(obj->pData == NULL)
    {
        return false;
    }
    return true;
}

/**
* @brief: QueueInit_1
* @param: obj                         
* @param: buff                        
* @param: queue_depth                 
* @param: queue_len                   
* @return: 
* @note: 
*
*/
bool QueueInit_1(Queue_t *obj, void* buff, uint16_t queue_depth, uint32_t queue_len)
{
    obj->msg_in = 0;
    obj->msg_out = 0;
    obj->valid_num = 0;
    obj->depth = queue_depth;
    obj->len_per_queue = queue_len;

    obj->pData = NULL;

//  obj->pData = (uint8_t *)malloc(queue_len * queue_depth);/*ʹ�öѿռ䣬��Ҫ��.s�ļ���heap_size�ĳɺ���ֵ��Ҳ���Բ���malloc�����ô�����ķ�ʽ*/
    obj->pData = (uint8_t *)buff;
    if(obj->pData == NULL)
    {
        return false;
    }
    return true;
}

/**
* @brief: QueueDeInit
* @param: obj                         
* @note: 
*
*/
void QueueDeInit(Queue_t *obj)
{
    obj->msg_in = 0;
    obj->msg_out = 0;
    obj->valid_num = 0;
    obj->depth = 0;
    if(obj->pData != NULL)
    {
        free(obj->pData);/*��malloc��Ӧ*/
    }
}

/**
* @brief: QueueQuery
* @param: obj                         
* @return: true����������
* @note: 
*
*/
bool QueueQuery(Queue_t *obj)
{
    if(obj->pData == NULL)
    {
        return true;
    }
    if(obj->valid_num >= obj->depth)
    {
        return true;
    }
    return false;
}

/**
* @brief: QueuePut
* @param: obj                         
* @param: p                           
* @return: 
* @note: 
*
*/
bool QueuePut(Queue_t *obj, void *p)
{
    if(obj->pData == NULL)
    {
        return false;
    }
    if(obj->valid_num >= obj->depth)
    {
        return false;
    }
    memcpy((uint8_t *)(&((uint8_t*)obj->pData)[obj->msg_in * obj->len_per_queue]), (uint8_t *)p, obj->len_per_queue);
    obj->msg_in++;
    obj->msg_in %= obj->depth;
    obj->valid_num++;
    return true;
}

/**
* @brief: QueueGet
* @param: obj                         
* @param: p                           
* @return: 
* @note: 
*
*/
bool QueueGet(Queue_t *obj, void **p)
{
    if(obj->pData == NULL)
    {
        return false;
    }
    if(obj->valid_num == 0)
    {
        return false;
    }
    *p = (void *)(&((uint8_t*)obj->pData)[obj->msg_out * obj->len_per_queue]);
    obj->msg_out++;
    obj->msg_out %= obj->depth;
    obj->valid_num--;
    return true;
}

/**
* @brief: QueueRead
* @param: obj                         
* @param: p                           
* @return: 
* @note: 
*
*/
bool QueueRead(Queue_t *obj, void **p)
{
    if(obj->pData == NULL)
    {
        return false;
    }
    if(obj->valid_num == 0)
    {
        return false;
    }
    *p = (void *)(&((uint8_t*)obj->pData)[obj->msg_out * obj->len_per_queue]);
//  obj->msg_out++;
//  obj->msg_out %= obj->depth;
//  obj->valid_num--;
    return true;
}

/**
* @brief: QueueClear
* @param: obj                         
* @return: 
* @note: 
*
*/
bool QueueClear(Queue_t *obj)
{
    if(obj->pData == NULL)
    {
        return false;
    }
    if(obj->valid_num == 0)
    {
        return false;
    }
//  *p = (void *)(&((uint8_t*)obj->pData)[obj->msg_out * obj->len_per_queue]);
    obj->msg_out++;
    obj->msg_out %= obj->depth;
    obj->valid_num--;
    return true;
}

/**
* @brief: QueueIsFull
* @param: obj                         
* @return: 
* @note: 
*
*/
bool QueueIsFull(Queue_t *obj)
{
    if(obj->pData == NULL)
    {
        return true;
    }
    if(obj->valid_num >= obj->depth)
    {
        return true;
    }
    return false;
}
```

### `middleware/queue.h`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        queue.h
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

#ifndef __QUEUE_H_
#define __QUEUE_H_

#include "F28x_Project.h"
#include "stdlib.h"
#include "stdbool.h"
#include "stdint.h"
#include "string.h"
typedef struct _queue_stru_t
{
    void        *pData;
    uint8_t     msg_in;/*�����ʱ��Ӧ�Ķ��б��*/
    uint8_t     msg_out;/*������ʱ��Ӧ�Ķ��б��*/
    uint16_t    len_per_queue;
    uint16_t    depth;
    uint16_t    valid_num;

} Queue_t;



bool QueueInit(Queue_t *obj, uint16_t queue_depth, uint32_t queue_len);
bool QueueInit_1(Queue_t *obj, void* buff,uint16_t queue_depth, uint32_t queue_len);
void QueueDeInit(Queue_t *obj);
bool QueueQuery(Queue_t *obj);
bool QueuePut(Queue_t *obj, void *p);
bool QueueGet(Queue_t *obj, void **p);
bool QueueRead(Queue_t *obj, void **p);
bool QueueIsFull(Queue_t *obj);
bool QueueClear(Queue_t *obj);


#endif
```

