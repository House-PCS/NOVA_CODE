# 02 应用层（app/）详细说明

本章对 `app/` 下每个文件做**函数级**说明：职责、数据结构、调用关系、运行上下文。

---

## 2.1 `main.c` — 入口与后台调度

### 职责

CPU2 唯一入口：完成最小系统初始化、双核握手、业务模块初始化，然后进入**无阻塞主循环**，用软定时标志驱动慢速任务。

### `void main(void)` 流程

详见 [01_系统总览与架构.md](./01_系统总览与架构.md) 第 1.3 节。主循环核心逻辑：

```c
for (;;) {
    CpuTimerFlag();                    // 根据 Timer2 生成 1/5/10/20/100/500/1000 ms 标志
    CpuControl_Ph3DataUpdate();        // 有数据则拷贝 RMS/功率等到 GS4 + STS 慢更新
    if (Get_1ms_Flag()) {
        CpuControl_Ph3FlagUpdate();    // 同步通道使能与 STS 命令
        Timers_1ms();                  // 清标志
    }
    // 其余档位标志目前仅清除，预留扩展
}
```

### 设计要点

- **实时工作不在 main**：20 kHz 三相与 STS 快路径全在 IPC ISR。
- 握手失败会死循环，防止在共享区未就绪时乱跑算法。
- `DevDcr_ExchangeAddress` 返回值被强制忽略：即使地址交换超时也会继续，依赖后续空指针检查保护。

---

## 2.2 `cpu_control` — 控制门面与 IPC 服务函数

### 职责

把“初始化 / 中断服务 / 后台更新”收敛成对外 API，避免 `main` 直接依赖过多模块。

### 静态变量

| 变量 | 含义 |
|------|------|
| `s_interruptRunCnt` | 本次 ISR 执行占用的 CpuTimer1 tick 数 |
| `s_interruptQIdleCnt` | 距上次 ISR 退出的空闲 tick 数 |
| `s_interruptExitTim` | 上次 ISR 退出时 Timer1 计数值 |

### `void CpuIntServiceFun(void)`（`.TI.ramfunc`，IPC ISR）

**调用者**：`CPU_Interrupt`（经回调指针）

逐步行为：

1. 读 `CpuTimer1Regs.TIM` 得到 `intCont`
2. 相对 `s_interruptExitTim` 计算空闲计数（处理 PRD 回绕）
3. `DualDataShare_Ph3IfcRunDone()` — 心跳 +1
4. `Ph3CalcRun_Run()` — 五通道电量快路径
5. `Ph3_Sts_Run()` — STS 快路径
6. 再读 TIM，计算本次 ISR 运行 tick
7. 保存退出时刻，写回 `isr_run_cnt` / `isr_idle_cnt` 到消息区

这是 CPU2 **最关键的实时入口**。

### 其它函数

| 函数 | 作用 | 调用者 |
|------|------|--------|
| `GetCpuInterruptRunCont` | 读运行 tick | 调试用，工程内无其它调用 |
| `GetCpuInterruptQIdleCnt` | 读空闲 tick | 调试用 |
| `CpuIntFunInit` | 注册回调 + `Interrupt_Init` | `main` |
| `CpuControl_Ph3CalcInit` | → `Ph3CalcRun_Init` | `main` |
| `CpuControl_Ph3StsInit` | → `Ph3_Sts_Init` | `main` |
| `CpuControl_Ph3DataUpdate` | → DataUpdate + STS 慢更新 | `main` 每圈 |
| `CpuControl_Ph3FlagUpdate` | → FlagUpdate + STS 标志 | `main` 1 ms |

---

## 2.3 `dual_data_share` — CPU2→CPU1 消息区

### 职责

封装固定地址 `0x3F800` 的消息结构访问，向 CPU1 公布结果指针与运行健康度。

### 结构体 `DevDct_Cpu2ToCpu1Msg_t`

见总览 1.4.1。通过宏：

```c
#define DualDataShare_GetCpu2ToCpu1Msg() \
    ((volatile DevDct_Cpu2ToCpu1Msg_t *)(0x0003F800UL))
```

直接把结构体映射到硬件消息 RAM，无需链接器 section。

### 函数说明

#### `void DualDataShare_Init(void)`

写入：

- `cpu2_ph3_ifc_addr = Ph3CalcIfcUpdate_Get()`
- `sts_output_addr = StsOutputUpdate_Get()`
- `cpu2_debug_data_addr = GetCpuDebugDataPtr()`
- 清零 `isr_run_cnt / heartbeat_cnt / isr_idle_cnt`

#### `void DualDataShare_Ph3IfcRunDone(void)`（ramfunc）

心跳递增，到 `0xFFFFFFFF` 回绕为 0。  
调用点：`CpuIntServiceFun` 入口 + `Ph3CalcRun_Run` 末尾。

#### `DualDataShare_SetIsrRunCnt / SetIsrQIdleCnt`

由 `CpuIntServiceFun` 写测时结果。

---

## 2.4 `dev_dual_core_receive` — 接收 CPU1 共享数据

### 职责

解析 `0x3FC00` 消息，缓存 ADC / STS 输入指针，并把通道使能标志应用到 `Ph3Calc_Channel_t`。

### 关键类型

#### `CpuAdcPh3RunFlag_t`

三路使能位：`enable_volt_pll`、`enable_curr_pll`、`enable_power_calc`。

#### `StsInputStatus_t`

| 字段 | 含义 |
|------|------|
| `sts_run_en` | STS 算法总使能 |
| `force_gen_mode` | 强制柴发模式（相对电网模式） |
| `en_connect / en_disconnect` | 并网/离网命令 |
| `warn_clear / err_clear` | 告警/故障清除 |

#### `CpuAdc_t`

与 CPU1 `gstAdcResult` **布局必须一致**，包含：

- 逆变 / 电网 / 负载 / 柴发 / CT 三相电压电流
- 继电器中点三相电压、漏电流
- 母线正负电压与电流、PV、温度等

#### `DevDct_Cpu1ToCpu2Msg_t`

ADC 地址 + 五通道使能 + STS 输入地址。

### 静态缓存

- `s_adc_result`、`s_sts_input`：地址交换成功后缓存，ISR 快速读取。

### 函数说明

#### `bool DevDcr_ExchangeAddress(uint32_t timeoutMs)`

每 1 ms 轮询一次 CPU1 消息中的 ADC 与 STS 地址，两者均非空则成功；超时返回 `false`。

#### `DevDcr_GetCpuAdcResult / GetStsInput / GetCpu1ToCpu2MsgPtr`

返回缓存指针或消息区指针。被 `Ph3CalcRun_*`、`Ph3_Sts_*` 广泛使用。

#### `void DevDcr_ApplyPh3RunFlag(Ph3Calc_Channel_t *ch, const volatile CpuAdcPh3RunFlag_t *flag)`

把三使能拷到通道结构。空指针直接返回。

---

## 2.5 `ph3_calc_run` — 五通道三相运行时

### 职责

管理五个 `Ph3Calc_Channel_t` 实例，在 ISR 中喂采样并跑算法，在后台把结果推到共享接口。

### 全局 / 静态

| 符号 | 说明 |
|------|------|
| `s_grid/s_gen/s_load/s_inv/s_ct` | 五通道状态机 |
| `s_busCurrFilter` | 母线电流块平均滤波（窗 400） |
| `s_dataUpdateFlag` | ISR 置位，后台消费后清零 |

### `void Ph3CalcRun_Init(void)`

- 四个 NORMAL + 一个 INV（`s_inv`）PF 模式
- 全部使用 `PH3_CALC_PLL_MODE_POSSEQ`
- 母线滤波窗 = `PH3_CALC_FILTER_DEFAULT_WIN`（400，约 20 ms @ 20 kHz）

### `void Ph3CalcRun_Run(void)`（ramfunc，IPC ISR）

1. 取 ADC 指针，空则返回  
2. 对五通道依次：`SetSample` → `ChannelRun` → `InterfaceAngleUpdate`（**只写角度到 GS4**）  
3. CT 通道电压故意取 **电网电压**，电流取 CT  
4. 母线：  
   - `curr_filter = Filter_Run(bus_curr)`  
   - `pos/neg_vol` 直接取 ADC  
   - `vol_filter = pos + neg`  
   - `power_filter = vol × curr`  
5. 心跳 +1  
6. `MidVolAndRcmuRms_Run(...)`，频率用逆变 `freq_a`  
7. `s_dataUpdateFlag = true`

### `void Ph3CalcRun_FlagUpdate(void)`（1 ms 后台）

从 CPU1 消息把五通道使能应用到 `s_*`。

### `void Ph3CalcRun_DataUpdate(void)`（主循环后台）

若 flag 为真：对五通道调用 `Ph3CalcIfcUpdate`（全量拷贝 RMS/功率/正负序等），再 `MidVolAndRcmuRms_Update`。

---

## 2.6 `ph3_calc_ifc_update` — GS4 电量接口缓冲

### 职责

定义并维护 CPU1 消费的完整电量结构 `DualDataShare_Ph3Ifc_t`，放置在 `ramgs1`（GS4）。

### `DualDataShare_Ph3Ifc_t` 内容

- `grid/gen/load/inv/ct`：各一个 `Ph3Calc_Interface_t`
- `bus`：母线滤波电压/电流/功率/正负压
- `rcmu_curr_rms`、`mid_vol_[abc]_rms`

### 函数

| 函数 | 作用 |
|------|------|
| `Ph3CalcIfcUpdate_Init` | 清零全部接口与母线/rcmu |
| `Ph3CalcIfcUpdate` | 转调 `Ph3Calc_InterfaceUpdate` |
| `Ph3CalcIfcAngleUpdate` | 转调 `Ph3Calc_InterfaceAngleUpdate` |
| `Ph3CalcIfcUpdate_Get` | 返回 `&gstPh3CalcIfcUpdate` |

全局：`DualDataShare_Ph3Ifc_t gstPh3CalcIfcUpdate`（`#pragma DATA_SECTION(..., "ramgs1")`）。

---

## 2.7 `ph3_sts_run` — STS 并网状态机胶水层

### 职责

把 ADC 瞬时量、通道 RMS/角度/频率，装配成 AlgoFw STS 输入；把 STS 输出、告警、故障、连接状态写到 GS4 供 CPU1 读取。

> **AlgoFw 源码不在本仓库**，此处只解释胶水层逻辑。

### 关键对象

| 对象 | 位置 | 含义 |
|------|------|------|
| `_gstStsInput` | 静态 | STS 算法输入缓冲 |
| `_gstStsOutput` | `ramgs1` | 对外输出状态包 |
| `_gstStsInputStatus` | 全局 | CPU1 命令镜像 |
| `_gstStsAdjustInput` | 全局 | 相位/电压调整器输入 |
| `_gstStsFlagUpdateFlag` | 静态 | ISR→后台慢更新握手 |

#### `StsOutputStatus_t`

含：`sts_output`、`sts_err`、`sts_warn`、`sts_state`、连接中/已连接/断开中/已断开、是否柴发模式。

### 函数详解

#### `void Ph3_Sts_FlagUpdate(void)` — 1 ms

把 CPU1 `StsInputStatus` 镜像到 `_gstStsInputStatus`。

#### `void Ph3_Sts_InstDateUpdate(void)` — ISR 内

- 若 `sts_is_gen_mode`：用柴发瞬时电压、`s_gen` 的 `theta_sin`、`freq_a→w0`  
- 否则：用电网对应量  
- 逆变侧始终用 `s_inv` / 逆变瞬时电压  
- `Sts_Set_InputData`，并置 `_gstStsFlagUpdateFlag=1`

#### `void ph3Sts_Adjust_DataUpdate(void)` — ISR 内

组装 Adjust 输入：逆变/并网点的 `w0`、`theta`、三相 RMS 均值，以及当前 STS state。

#### `void Ph3_Sts_DataSlowUpdate(void)` — 后台

1. 无 flag 则返回  
2. 同步 `force_gen_mode` 到输出侧  
3. 按模式选 gen 或 grid 的 RMS 填 `connected_*`  
4. 填逆变 RMS  
5. `Sts_WarnCheck_Run` / `Sts_FaultCheck_Run`  
6. 刷新 err/warn/连接状态机标志到 `_gstStsOutput`

#### `void Ph3_Sts_Init(void)`

`Sts_Init` + 清零输入输出 + `Sts_Adjust_Init`。

#### `void Ph3_Sts_Run(void)` — IPC ISR

- **`sts_run_en==1`**：设 connect/清故障 → InstUpdate → `Sts_Run` → Adjust → 写 `gStsData.adjust_*` → `Sts_Get_OutputData`  
- **否则**：调用 `Ph3_Sts_Init()`（禁用即复位）

---

## 2.8 `mid_vol_and_rcmu_rms` — 中点电压与漏电流统计

### 职责

对继电器中点三相电压与 RCMU 漏电流做**按电网周期的均方统计**，结果经后台拷入 GS4 接口。

### 关键参数

- 漏电流 LPF：一阶 Butterworth，`fc=500 Hz`，`fs=20 kHz`
- 周期样点数：\(N = 20000 / clamp(f, 30, 70)\)；`f<=0` 时按 50 Hz

### `void MidVolAndRcmuRms_Run(...)`（ISR）

1. 漏电流经 LPF 得 `s_leak_curr_filter`  
2. 累加 \(v_a^2,v_b^2,v_c^2,i_{filt}^2\)  
3. 满 `s_cycle_sample_count` 后：输出 **均方**（`sum/N`），**未做开方**  
4. 清累加，按频率重算下周期 `N`

### 实现注意（文档化已知行为）

1. **命名含 rms，实际输出为均方（Mean Square）**，若需真 RMS 需再 `sqrt`。  
2. 初始化后 `s_cycle_sample_count==0`，首周期进入结束分支时存在 **/0 风险**（实现层细节）。  
3. 频率输入来自逆变 `freq_a`，用于估计周期长度。

### `MidVolAndRcmuRms_Update`

把本地结果拷到 `DualDataShare_Ph3Ifc_t` 的 rcmu/mid 字段。

---

## 2.9 应用层调用关系小结

```
main
  └─ CpuControl_* 门面
       ├─ ISR: CpuIntServiceFun
       │     ├─ DualDataShare_*
       │     ├─ Ph3CalcRun_Run → Channel×5 / IfcAngle / MidVol
       │     └─ Ph3_Sts_Run
       ├─ 后台 DataUpdate: Ph3CalcRun_DataUpdate + Ph3_Sts_DataSlowUpdate
       └─ 1ms FlagUpdate: Ph3CalcRun_FlagUpdate + Ph3_Sts_FlagUpdate
```

完整源码见：[附录A_应用层源码.md](./附录A_应用层源码.md)

下一步：[03_算法层_algo详细说明.md](./03_算法层_algo详细说明.md)
