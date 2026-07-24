# 04 内存映射与常量速查

---

## 1. SoftCore 本地代码/数据区

| 区域 | 起始 | 大小 | 说明 |
|------|------|------|------|
| IROM | `0x00000000` | 16KB (`0x4000`) | Scatter 实际代码区 |
| IRAM | `0x00004000` | 16KB (`0x4000`) | Scatter 实际 RAM |
| Stack | — | 512B (`0x200`) | Startup |
| Heap | — | 256B (`0x100`) | Startup |

固定地址缓冲（源码中显式定位）：

| 符号/缓冲 | 地址 | 说明 |
|-----------|------|------|
| `bms_ddr_temp[256]` | `0x00007070` | BMS DDR 临时区 |
| `fpga_ddr_temp[256]` | `0x00007170` | FPGA DDR 临时区 |
| `fpga_firmware_data_save` | `0x5380` | FPGA 升级 1024B 缓存 |

---

## 2. FPGA 映射外设与共享区

### 2.1 控制寄存器（`0x4000xxxx`）

| 符号 | 地址 | 说明 |
|------|------|------|
| `LED_IO` | `0x40000000` | 系统 LED |
| `LEN` | `0x40000010` | UDP 长度握手 |
| `READY` | `0x40000014` | UDP 就绪握手 |
| `EthIrqClr` | `0x40000018` | 清以太网/通信中断 |
| PHY RST | `0x40000020` | PHY 复位（0 低 / 1 高） |
| MDC | `0x40000024` | MDIO 时钟 |
| MDIO | `0x40000028` | MDIO 数据 |
| MDIO DIR | `0x4000002C` | 方向（0 入 / 1 出） |
| SoftReboot | `0x40000030` | 写 1 软复位 |

### 2.2 CMSDK 外设（`0x4100xxxx`）

| 符号 | 地址 |
|------|------|
| UART0 | `0x41001000` |
| SPI | `0x41002000` |
| SysTick | `0xE000E010` |

### 2.3 Cache / 镜像（`0x300xxxxx`）

| 符号 | 地址 | 说明 |
|------|------|------|
| `CACHE_UDPDATA_RX` | `0x30000000` | UDP 请求区 |
| `CACHE_UDPDATA_TX` | `0x30001000` | UDP 响应区 |
| BMS 寄存器镜像 `gstBmsReg` | `0x30010000` | BMS Para |
| DCDC 寄存器镜像 `gstDCDCReg` | `0x30020000` | DCDC Para |
| 故障录波读窗口 | `0x30030000`~`0x300CFFFF` | 映射到 Flash 录波区 |

### 2.4 DDR 区

| 区域 | 地址 | 说明 |
|------|------|------|
| CMSDK DDR 基址 | `0x20000000` | 通用 DDR 访问口 |
| 故障录波环形区 | `0x21000000`~`0x2108FFFF` | 589824 字节 |
| FPGA 固件 DDR | `0x24000000` | 升级包落点 |
| BMS 固件 DDR | `0x24400000` | 升级包落点 |
| UDP 写固件允许范围 | `0x20000000`~`0x246FFFFF` | `WriteDDRSend` |

---

## 3. SPI Flash 布局

| 区域 | 地址 | 说明 |
|------|------|------|
| FPGA 应用固件 | `0x00261000` | 升级写入目标 |
| 故障录波 | `0x004C2000` | DDR→Flash 落盘 |
| 升级标志 | `0x00BFF000` | 标志值 **`0xA0`** |
| 版本扩展信息 | `0x00C00000` | 256B；`DCDCRegInit` 读 24B |

Flash 几何：

| 项 | 大小 |
|----|------|
| Page | 256 B |
| Sector | `0x1000` (4KB) |
| Block 32K | `0x8000` |
| Block 64K | `0x10000` |

---

## 4. 缓冲与队列尺寸

| 宏/变量 | 值 |
|---------|-----|
| `COMSERVICE_BUFF_SIZE` | 550 |
| `COM_RX_SIZE` | 1100 |
| `garrBMSTxbuf` / `garrDCDCTxbuf` | 600 |
| `SCI_REV_QUEUE_DEPTH` | 2 |
| `REV_QUEUE_DEPTH` | 2 |
| `SLAVE_WRITE_QUEUE_DEPTH` | 2 |
| `UPGRATE_DATA_SIZE` | 1024 |
| `FAULT_RECORD_DATA_SIZE` | 1024 |
| `arr_buff`（dev_ddr） | 1050 |
| 故障录波单帧读回 | `0x90` (144) 字节 |
| 录波总量 | `144 * 4096 = 589824` 字节 |

---

## 5. 协议与魔数常量

| 常量 | 值 |
|------|-----|
| UART 波特率 | 115200 |
| 请求帧头 | `0x5A` |
| 响应帧头 | `0xA5` |
| UDP/FPGA SPI 读 | `0x43` |
| UDP/FPGA SPI 写 | `0x45` |
| BMS 读/写命令 | `0x0001` / `0x8002` |
| 升级 Start/Busy/End | `0x80F0` / `0x80F1` / `0x80FF` |
| BMS 固件魔数 | `0C 0C 0C 03` |
| FPGA 固件魔数 | `0C 0C 0C 04` |
| 故障录波帧头 | `A5 5A FE 01` |
| 故障录波帧尾 | `00 00 80 7F` |
| 升级标志 | `0xA0` |
| softreboot APP0/BOOT | `0x50` / `0x51` |

从机地址宏（Modbus 风格，镜像用）：

| 角色 | 地址 |
|------|------|
| PCS | `0x01` |
| DCDC | `0x02` |
| MPPT | `0x03` |
| BMS | `0x04` |

---

## 6. `updata_status` 位含义

位于 `DCDCHoldPara_t.updata_status`（从代码行为推断）：

| Bit / 掩码 | 含义 |
|------------|------|
| bit0 | 升级进行中 |
| bit1 | 非首包 IP 复位触发 |
| bit4 | 首包 IP（192.168.11.1）复位触发 |
| bit8 | 含 BMS 固件包 |
| bit10 | 含 FPGA 固件包 |
| bit12 | DDR 校验完成阶段 |
| `0x2000` (bit13) | 升级**成功** |
| `0x4000` (bit14) | 升级**失败** |

`FirmwareUpgrate()` 回写成功/失败位。

---

## 7. 超时与周期参数

| 宏/行为 | 值 |
|---------|-----|
| `HOST_COMM_POLLING_TIME` | 300 ms |
| `HOST_COMM_REV_MAX_TIME` | 1000 ms |
| `UPGRATE_TIME_OUT` | 3000 ms |
| BMS 读超时清零 | 连续 3 次 |
| 通信超时告警 | 约 100 s |
| 通信超时关机 | 约 120 s，写 `0x300100EC` = `{0x08}` |
| LED 正常周期 | 1000 ms |
| LED 通信失败 | 100 ms |
| LED PHY 失败 | 5000 ms |
| SysTick | 1 ms |
| UT 测试间隔 | 2 s（默认关闭） |

---

## 8. PHY 相关常量

| 项 | 值 |
|----|-----|
| PHY 地址 | `0x00`，`0x01` |
| 复位等待 | 100 ms |
| 配置重试 | 各 3 次 |
| Reg0 复位值 | `0xA100` |
| Reg0 运行值 | `0x2100`（100M 全双工） |
| Page31 Reg16 | `0x1FFE`（RMII） |

---

## 9. 工程/输出名速查

| 项 | 值 |
|----|-----|
| Keil 工程 | `softcore.uvprojx` |
| Target | `Debug` |
| 输出名 | `Roca2-16KH_Fpga_Softcore` |
| 系统时钟（源码） | 50 MHz |
| 版权 | 西安诺瓦星云科技股份有限公司 |
