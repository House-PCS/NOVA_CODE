# 03 算法层（algo/）详细说明

本章覆盖 `algo/ph3_calc/` 三相电量算法，以及 `cla_*` 预留代码。所有热路径函数均在 `.TI.ramfunc` 中于 RAM 执行。

---

## 3.1 模块总览与调用链

```
Ph3Calc_ChannelRun
 ├─ ChannelRunVoltPll
 │    ├─ PosNegRun(电压)     （PLL_MODE_POSSEQ）
 │    └─ PllRun(α+, β+)  或  PllRunAbc
 ├─ ChannelRunCurrPll
 │    ├─ PosNegRun(电流, θ=电压theta_cos)
 │    └─ PllRun(α+, β+)  或  PllRunAbc
 └─ [enable_power_calc]
      ├─ PowerUpdateFactor      （相位差 → pf / sin）
      ├─ RmsStateRun × 3        （分相周期 RMS/有功/视在）
      ├─ PowerUpdateReactive    （无功 + 合相）
      └─ ChannelUpdateFreq × 3  （f=1/T，限幅 40–70 Hz）
```

配置常量集中在 `ph3_calc_cfg.h`；数据结构在 `ph3_calc_def.h`；数学封装在 `ph3_calc_math.h`（`cosf/sinf/sqrtf`）。

---

## 3.2 `ph3_calc_cfg.h` — 算法参数

| 宏 | 值 | 含义 |
|----|-----|------|
| `PH3_CALC_TS` | `50 µs` | 控制周期（1/20 kHz） |
| `PH3_CALC_PLL_KP` | `1.43` | PLL 比例增益 |
| `PH3_CALC_PLL_KI` | `317` | PLL 积分增益 |
| `PH3_CALC_PLL_W_NOMINAL` | `100π` | 额定角频率（50 Hz） |
| `PH3_CALC_PLL_W_LIMIT` | `50π` | 角频率偏差限幅 |
| `PH3_CALC_PLL_FREQ_MIN/MAX` | `40 / 70` | 频率输出限幅 (Hz) |
| `PH3_CALC_RMS_COUNT_MIN` | `250` | 最短周期点数（约 80 Hz 上限） |
| `PH3_CALC_RMS_COUNT_MAX` | `666` | 最长周期点数（约 30 Hz 下限） |
| `PH3_CALC_POSNEG_BP_FREQ` | `50` | 正负序 90° 移相滤波器中心频率 |

`PH3_CALC_VALUE_LIMIT(x,max,min)`：三元限幅宏。

---

## 3.3 `ph3_calc_def.h` — 核心数据结构

### `Ph3Calc_Ac_t`

瞬时值 `inst` + 有效值 `rms`。

### `Ph3Calc_Phase_t`

单相：电压/电流 AC + 功率（有功/无功/视在/pf）。

### `Ph3Calc_Pll_t`

SRF-PLL 状态：`kp/ki/step`、`theta_temp/theta_cos/theta_sin`、积分、`vd/vq/error/w/freq`。

### `Ph3Calc_PosNegData_t`

Clarke 结果、90° 移相中间量、正/负序 αβ、Park 后的 `d/q_pos/neg`。

### 枚举

- **PF 模式**  
  - `NORMAL`：`θ_diff = θv − θi`  
  - `INV`：`θ_diff = −(θv − (θi + π))`（逆变功率方向定义）
- **PLL 模式**  
  - `ABC`：Clarke 后直接锁相  
  - `POSSEQ`：正负序后对正序锁相（本工程五通道均用此模式）

### `Ph3Calc_RmsState_t`

周期累加器：`count`、`vol/cur/vi` 平方和积、上一拍电压、过零插值起点、周期时间。

### `Ph3Calc_Channel_t`

完整单通道：三相相位、电压/电流 PLL、电压/电流正负序、三相 RMS 状态、合相功率、频率、模式、三使能位。

---

## 3.4 `ph3_calc_channel` — 单通道调度

### `Ph3Calc_ChannelInit(ch, pf_mode, pll_mode)`

清零所有相位/功率/频率；使能默认全开；初始化 PosNeg、RMS、PLL（步长 `PH3_CALC_TS`）。

### `Ph3Calc_ChannelSetSample(ch, va..ic)`

写入六路瞬时值到 `phase_[abc].vol/cur.inst`。

### 静态 `ChannelRunVoltPll / ChannelRunCurrPll`

- **POSSEQ**：`PosNegRun` → 用正序 αβ 调 `PllRun`  
- **ABC**：`PllRunAbc`  
- 电流 PosNeg 的 Park 角传入 **`volt_pll.theta_cos`**，使电流 dq 与电压同步坐标系对齐。

### `Ph3Calc_ChannelRun(ch)`

1. 电压 PLL：使能则跑，否则 `PllInit + PosNegInit` 复位  
2. 电流 PLL：同上  
3. 功率使能：Factor → RMS×3 → Reactive → 分相频率；`volt_pll.freq = freq_a`  
4. 功率禁用：清零 RMS/功率/频率并重 Init RMS

### 静态 `ChannelUpdateFreq(cycle_time, *freq)`

\(f = 1/T\)，限幅到 40–70 Hz；`cycle_time<=0` 或空指针直接返回。

---

## 3.5 `ph3_calc_pll` — 正序 SRF-PLL

### 原理

同步旋转坐标系锁相：把 αβ 变换到 dq，用 `vq` 作误差，PI 调节角频率，积分得 θ。

### 公式（`Ph3Calc_PllRun`）

\[
\begin{aligned}
v_d &= \alpha\cos\theta + \beta\sin\theta \\
v_q &= -\alpha\sin\theta + \beta\cos\theta \\
e &= v_q \\
I &\leftarrow I + T_s\, e \quad (\text{限幅}\pm W_{limit}) \\
\omega &= K_p e + K_i I \quad (\text{限幅}) \\
\theta &\leftarrow \theta + (\omega + \omega_0)\,T_s \quad (\text{wrap 到}\pm 2\pi)
\end{aligned}
\]

输出约定：

- `theta_cos = theta_temp`（主角度）
- `theta_sin = theta_cos + π/2`（正交角，供接口/STS 使用）

### `Ph3Calc_PllRunAbc`

先 Clarke：

\[
\begin{aligned}
\alpha &= (v_a - 0.5 v_b - 0.5 v_c)\cdot\frac{2}{3} \\
\beta &= (0.866(v_b - v_c))\cdot\frac{2}{3}
\end{aligned}
\]

再调用 `PllRun`。

### `Ph3Calc_PllInit`

装入 Kp/Ki/step，复位全部状态。

---

## 3.6 `ph3_calc_posneg` — 正负序分离

### 步骤（`Ph3Calc_PosNegRun`）

1. **Clarke** 得 α、β  
2. **一阶 90° 移相**（双线性，中心频率 50 Hz）：

\[
\begin{aligned}
w &= 2\pi f_{bp} \\
a_1 &= \frac{w T_s - 2}{w T_s + 2},\quad
b_0 = \frac{-2 + w T_s}{w T_s + 2},\quad
b_1 = 1 \\
\alpha_{90} &= -a_1\alpha_{90,prev} + b_0\alpha + b_1\alpha_{prev}
\end{aligned}
\]

β 同理。

3. **正负序组合**：

\[
\begin{aligned}
\alpha^+ &= 0.5\alpha - 0.5\beta_{90},\quad
\beta^+ = 0.5\beta + 0.5\alpha_{90} \\
\alpha^- &= 0.5\alpha + 0.5\beta_{90},\quad
\beta^- = 0.5\beta - 0.5\alpha_{90}
\end{aligned}
\]

4. **Park**：正序用 \(+\theta\)，负序用 \(-\theta\)，得到 `d/q_pos/neg`。

### `Ph3Calc_PosNegInit`

清零移相状态与正负序/dq（瞬时/αβ 字段在 Init 中部分被注释，首次 Run 会覆盖）。

---

## 3.7 `ph3_calc_rms` — 周期 RMS / 有功 / 视在

### 每拍累加

\[
\sum v^2,\ \sum i^2,\ \sum vi,\quad count \mathrel{+}= 1
\]

### 周期结束条件

- **过零**：上一拍电压 < 0 且当前 ≥ 0，且 `count > 250`  
- **或超时**：`count > 666`

### 过零线性插值修正周期长度

\[
\alpha_{end} = -\frac{v_{prev}}{v_{curr}-v_{prev}}
\]

有效时：

\[
count \leftarrow (count-1) + \alpha_{end} + (1-\alpha_{start})
\]

并把本周期 `α_end` 存为下周期 `period_alpha_start`。

### 输出

\[
\begin{aligned}
V_{rms} &= \sqrt{\sum v^2 / N} \\
I_{rms} &= \sqrt{\sum i^2 / N} \\
P &= \sum vi / N \\
S &= V_{rms}\, I_{rms} \\
T_{cycle} &= N\cdot T_s
\end{aligned}
\]

然后清零累加器，保存当前电压为下一拍的 `vol_rms_input_1`。

---

## 3.8 `ph3_calc_power` — 功率因数与无功

### `Ph3Calc_PowerUpdateFactor`

- NORMAL：`θ_diff = θv_cos − θi_cos`  
- INV：`θ_diff = −(θv_cos − (θi_cos + π))`  
- wrap 到 \([-π, π]\)  
- `power_factor = cos(θ_diff)`  
- `power_refactor = sin(θ_diff)`（无功符号因子）

### `Ph3Calc_PowerUpdateReactive`

当前采用：

\[
Q = \mathrm{sign}(\sin\theta_{diff})\cdot\sqrt{S^2 - P^2}
\]

（代码里：先算正根，若 `power_refactor < 0` 取负。）

然后：

- 各相 `pf` 共用通道 `power_factor`  
- 合相：三相 P/Q/S 分别求和

注释中保留备选公式 `Q = sin(θ)·S`，当前未启用。

---

## 3.9 `ph3_calc_filter` — 块平均滤波

用于母线电流。

### 行为

- 累加样本，`count++`  
- 满 `window_size`（默认 400）：`output = sum/count`，清零累加  
- 未满：返回**旧** `output`（保持上次均值）

API：`Init / Reset / Run / GetOutput`。

---

## 3.10 `ph3_calc_interface` — 对外接口打包

把内部 `Channel` 状态整理成 CPU1 可读的扁平结构。

### `Ph3Calc_InterfaceAngleUpdate`（ISR 快路径）

```text
vol.theta  = normalize(volt_pll.theta_sin) ∈ [0, 2π)
curr.theta = normalize(curr_pll.theta_sin)
```

注意：头文件注释写“取自 theta_cos”，实现实际用 **theta_sin** 并归一化。

### `Ph3Calc_InterfaceUpdate`（后台慢路径）

拷贝：瞬时、RMS、频率、分相/合相功率、pf、RMS count、电压/电流正负序 dq 全套。

### `Ph3Calc_InterfaceInit`

全字段清零。

---

## 3.11 CLA 预留（`cla_control` / `cla_cpu_share`）

### `cla_control.cla`

- `CLA_Task` / `Cla1Task1`：仅预留清中断（代码注释掉）  
- `Cla1Task2`～`Task8`：空函数  

### `cla_control.h`

声明 `CLA_Task(void)`。

### `cla_cpu_share.c/.h`

空实现，无共享变量定义。

### 工程现状

`interrupt.c` 明确不使能 CLA；算法全部在 C28x。链接脚本已分配 LS0–5 等 CLA 区，便于后续迁移。

---

## 3.12 算法层设计要点汇总

1. **正序锁相 + 电流用电压角 Park**：提高不平衡工况下锁相稳健性，并保证电流 dq 与电压同坐标系。  
2. **过零插值 RMS**：提高频率与有效值精度，避免整数采样点量化误差。  
3. **INV PF 模式**：对逆变通道单独定义相位差，匹配功率方向约定。  
4. **使能位独立**：可单独关电压 PLL / 电流 PLL / 功率计算，关时复位对应状态，避免脏数据。  
5. **接口快慢分离**：角度 ISR 写，RMS/功率后台写，降低 GS4 撕裂。

完整源码见：[附录B_算法层源码.md](./附录B_算法层源码.md)

下一步：[04_驱动层与中间件详细说明.md](./04_驱动层与中间件详细说明.md)
