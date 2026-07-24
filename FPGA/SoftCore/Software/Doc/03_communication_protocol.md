# 03 通信协议规范

本文描述 SoftCore 对外的两条主通信通路及升级相关帧格式。

---

## 1. 通信栈总览

```
                    ┌─────────────────────────────────────┐
  监控卡/上位机 -->│ FPGA UDP 代理 (共享内存 0x30000000) │
                    │ INT02 中断 → fpga_com               │
                    └──────────────┬──────────────────────┘
                                   │ gUdpSlaveCommSendQueue
                                   ▼
                    ┌─────────────────────────────────────┐
                    │ comm_analysis.SlaveCommProcess()     │
                    │ private_analysis.UdpProtopackJudge() │
                    └──────┬──────────────┬────────────────┘
                           │              │
              读/写镜像     │              │ 写 BMS 入队
              gstBmsReg     │              ▼
              gstDCDCReg    │     gBmsSendQueue → WriteBmsData()
                           │              │
                           │              ▼
                           │     ┌─────────────────┐
                           │     │ UART0 115200     │
                           │     │ mid_comservice   │
                           │     │ 0xA5 帧解析      │
                           │     └────────┬────────┘
                           │              │ gstSciRevQueue
                           │              ▼
                           │     ProtoUnpackJudge / 升级 ACK
                           │
              TX 响应 <────┘
              CACHE_UDPDATA_TX (0x30001000)
              LEN / READY 握手
```

**BMS UART 方向：**

```
BMS --RX--> INT01 --> mid_comservice_data_in()
         --> MidComserviceProtocolProcess() [识别 0xA5]
         --> gstSciRevQueue
         --> SlaveCommProcess() / RevBMSUpdataAnalysis()

SoftCore --TX--> ReadBMSReg / WriteBmsData / BMSUPGradeSend*
         --> Uart0Send()
```

说明：Modbus CRC/结构体保留，但 `ModbusCommProcess()` 已注释，**当前运行不参与**。

---

## 2. UART 私有协议（SoftCore <-> BMS）

### 2.1 物理层

| 项 | 值 |
|----|-----|
| 接口 | UART0 |
| 波特率 | 115200 |
| 时钟基准 | MCLK = 50 MHz |

### 2.2 请求帧（SoftCore → BMS，帧头 `0x5A`）

| Offset | 字段 | 说明 |
|--------|------|------|
| 0 | `0x5A` | 帧头 |
| 1 | `send_addr` | 通常 `0x00` |
| 2 | `recv_addr` | 通常 `0x03` |
| 3:4 | `cmd` | 小端 uint16 |
| 5:6 | `data_len` | 小端 uint16，数据域字节数 |
| 7.. | payload | 可选 |
| N-2:N-1 | checksum | 前面所有字节累加，小端 uint16 |

### 2.3 响应帧（BMS → SoftCore，帧头 `0xA5`）

**写响应（8 字节）：**

```
A5 03 00 | cmd_lo cmd_hi | err_code | chk_lo chk_hi
```

识别命令：`0x8002`、`0x80F0`、`0x80F1`、`0x80FF`。

**读响应（命令 `0x0001`）：**

```
A5 03 00 | 01 00 | payload_len(2) | data... | checksum(2)
```

`payload_len` 来自 byte[6:7]。

### 2.4 命令表

| 命令 | 值 | 方向/用途 |
|------|-----|-----------|
| `READ_BMS_INFO_CMD` | `0x0001` | 读 BMS 信息 |
| `WRITE_BMS_INFO_CMD` | `0x8002` | 写 BMS 寄存器 |
| `BMS_CMD_UPGRATE_Start` | `0x80F0` | 升级开始 |
| `BMS_CMD_UPGRATE_BUSY` | `0x80F1` | 升级数据包 |
| `BMS_CMD_UPGRATE_END` | `0x80FF` | 升级结束 |

### 2.5 校验算法

```text
checksum = sum(所有参与字节) & 0xFFFF   // 小端存放
```

与 `sys/crc.c` 的 Modbus CRC16 **不同**。middleware 与 private_analysis 中均有 `CheckSum()` / `CheckSumFun()`。

### 2.6 中间件解析要点

- 响应头匹配：`{0xA5, 0x03, 0x00}`
- 写响应固定 8 字节；读响应按 `payload_len` 收齐后再校验
- 升级 ACK（0x80F0/F1/FF）通过 `RegisterBMSUpdataFunPoint` 回调到升级模块

---

## 3. FPGA UDP 代理协议（监控卡 <-> SoftCore）

### 3.1 共享内存与握手

| 符号 | 地址 | 说明 |
|------|------|------|
| `CACHE_UDPDATA_RX` | `0x30000000` | 监控卡→软核请求 |
| `CACHE_UDPDATA_TX` | `0x30001000` | 软核→监控卡响应 |
| `LEN` | `0x40000010` | 长度握手 |
| `READY` | `0x40000014` | 就绪握手 |
| `EthIrqClr` | `0x40000018` | 清中断 |

中断：`INT02_Handler`。

### 3.2 帧头结构 `CACHE_UDPDATA_TypeDef`

| Offset | 字段 | 类型 | 说明 |
|--------|------|------|------|
| 0:1 | `device_type` | uint16 | 软核应用侧实际处理 **`0x0003`** |
| 2 | `reserved` | uint8 | 寄存器宽度倍数：0→×1，1→×2，2→×3，3→×4 |
| 3 | `op_cmd` | uint8 | `0x43`=读，`0x45`=写 |
| 4:7 | `addr` | uint32 | 寄存器/内存绝对地址 |
| 8:9 | `len` | uint16 | **寄存器个数**（非字节数） |
| 10: | `data` | bytes | 读写数据 |

数据总长：`data_size = len * reg_num + 10`。

### 3.3 device_type 过滤差异

| 位置 | 行为 |
|------|------|
| `INT02_Handler` | 接受 `0x0001` ~ `0x0005` 入队 |
| `UdpProtopackJudge` | 仅处理 **`0x0003`** |

联调时请确保监控卡侧 `device_type=0x0003`，否则入队后会被应用层丢弃。

### 3.4 地址空间与动作

| 地址范围 | op_cmd | 动作 |
|----------|--------|------|
| `0x30010000`~`0x300100FF` | 0x43 | 读 BMS 镜像 → 写 TX 缓存 |
| `0x30020000`~`0x300200FF` | 0x43 | 读 DCDC 镜像 → 写 TX 缓存 |
| `0x30030000`~`0x300CFFFF` | 0x43 | 读故障录波（Flash `0x004C2000` 起） |
| `0x30010000`~`0x3001FFFF` | 0x45 | 写 BMS → `gBmsSendQueue` |
| `0x30020000`~`0x300200FF` | 0x45 | 写 DCDC hold |
| `0x20000000`~`0x246FFFFF` | 0x45 | 写 DDR 固件 → `WriteDDRSend()` |

### 3.5 故障录波读回帧格式

监控卡读 `0x30030000`~ 时，软核从 Flash 返回，单次长度 **`0x90`（144）字节**，带：

| 字段 | 值 |
|------|-----|
| 帧头 | `A5 5A FE 01` |
| 帧尾 | `00 00 80 7F` |

---

## 4. BMS 固件升级 UART 帧

包大小：`UPGRATE_DATA_SIZE = 1024` 字节/包。

### 4.1 Start（`0x80F0`）

- 固件总长度：4 字节
- `space_num`：1 字节，固定 `0x02`
- 等待 ACK（写响应格式）

### 4.2 Busy（`0x80F1`）

- `index`：2 字节包序号
- 数据：最多 1024 字节
- 校验：checksum32 的低 16 位
- 循环发送直至发完

### 4.3 End（`0x80FF`）

- 无数据域
- 等待 end_ack

升级状态机见 [05_关键业务流程.md](./05_key_workflows.md)。

---

## 5. 主机轮询时序（SoftCore → BMS）

| 参数 | 值 |
|------|-----|
| 轮询周期 | `HOST_COMM_POLLING_TIME = 300 ms` |
| 单次接收超时 | `HOST_COMM_REV_MAX_TIME = 1000 ms` |
| 连续超时清零 | 3 次后清零 BMS 镜像 |

优先级：

1. UDP 请求处理
2. BMS 写队列
3. 空闲轮询（本地读 DCDC 镜像 / UART 读 BMS）

---

## 6. FPGA 通信超时保护

`Commovertime()`（由升级调度路径调用）：

| 阶段 | 行为 |
|------|------|
| 约 100s 无 INT02 | 告警计数 / 置失败倾向 |
| 约 120s | `comm_fail_flag=1`，写 BMS 寄存器 **`0x300100EC`** 关机命令 `{0x08}` |
| LED | `comm_fail_flag` 时 100ms 快闪 |

---

## 7. 与 FPGA SPI 命令的关系

设备层访问 FPGA 寄存器时，SPI 侧也使用：

- 读命令 **`0x43`**
- 写命令 **`0x45`**

与 UDP 代理 `op_cmd` 编码一致，便于软硬件统一语义。
