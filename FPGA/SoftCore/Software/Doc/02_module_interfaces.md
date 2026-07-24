# 02 模块接口说明

本文按分层列出各源码模块的用途、关键 API、数据结构与依赖。

---

## 1. SYS 层

### 1.1 `sys/main.c` / `sys/main.h`

| 项 | 内容 |
|----|------|
| 用途 | 程序入口；聚合初始化与主循环 |
| API | `main()` |
| 依赖 | systick、dev_led、uart、mid_comservice、comm_analysis、fpga_com、firmware_upgrate、dev_phy、ut_test_mode、spi、dev_spiflash、fault_record_process |

### 1.2 `sys/systick.c` / `sys/systick.h`

| 项 | 内容 |
|----|------|
| 用途 | 1ms 系统 tick、阻塞延时、软延时、时间差计算 |
| 全局 | `SysTimingMs`（毫秒计数） |
| API | `SystickInit()`, `DelayMs()`, `TimGetSystime()`, `TimDifftime()`, `TimDifftimeFromNow()`, `TimSlotCheck()`, `SoftDelayus()`, `SoftDelayms()` |
| 中断 | `SysTick_Handler()` → `_TimUpdateSystime()` |
| 配置 | `LOAD = SystemCoreClock/1000` |
| 宏 | `SYS_TICK` → `0xE000E010` |

说明：`SoftDelayus/ms()` 为忙等，注释提示需实测校准（内部按 `50*usec/5` 一类循环估算）。

### 1.3 `sys/queue.c` / `sys/queue.h`

| 项 | 内容 |
|----|------|
| 用途 | 固定长度环形消息队列 |
| 结构体 | `Queue_t`：`pData`, `msg_in`, `msg_out`, `len_per_queue`, `depth`, `valid_num` |
| API | `QueueInit()`（malloc）、`QueueInit_1()`（静态缓冲）、`QueueDeInit()`, `QueuePut()`, `QueueGet()`, `QueueRead()`, `QueueClear()`, `QueueQuery()`, `QueueIsFull()` |

工程中多处队列深度为 **2**（SCI / UDP / BMS 写队列）。

### 1.4 `sys/crc.c` / `sys/crc.h`

| 项 | 内容 |
|----|------|
| 用途 | Modbus CRC16、字节序转换、U8/U16 拷贝 |
| API | `CrcModbus()`, `CrcJudge()`, `BEBufToUint16()`, `TabLsbToMsb()`, `TabMsbToLsb()`, `MemcpyU16toU8SwapByte()`, `MemcpyU16toU8()`, `MemcpyU8toU16()` |
| CRC | 初始值 `0xFFFF`，Modbus 标准查表法 |

注意：UART 私有协议校验使用 **字节累加 CheckSum**（见 middleware/app），与本模块 CRC 不同。

---

## 2. DRIVER 层

### 2.1 `driver/uart.c` / `driver/uart.h`

| 项 | 内容 |
|----|------|
| 用途 | CMSDK UART0，连接 BMS |
| 基址 | `UART0` = **`0x41001000`** |
| 波特率 | **115200**，`MCLK=50MHz`，`BAUDDIV=MCLK/115200` |
| CTRL | `0x0B`（TX/RX 使能 + RX 中断） |
| 中断 | `INT01_IRQn`，`INT01_Handler()` 检测 RX 中断 bit1 |
| API | `Uart0Init()`, `Uart0Send()`, `Uart0SendByte()`, `Uart0DataGet()` |
| 回调 | `void (*Uart0IrqRxProcess)()` — 由 mid_comservice 注册 |

### 2.2 `driver/spi.c` / `driver/spi.h`

| 项 | 内容 |
|----|------|
| 用途 | CMSDK 自定义 SPI 控制器 |
| 基址 | **`0x41002000`** |
| 结构体 | `CMSDK_SPI_TypeDef`：`reg_spi_mode`, `reg_freq_coe`, `reg_cs`, `reg_txd`, `reg_rxd`, `write_tx_ready` |
| 初始化 | CS=1，freq_coe=1，**SPI Mode 3** |
| API | `SpiInit()`, `SpiSendByte()`, `SpiSend()`, `SPIDataGet()`, `SpiRead()` |
| 副作用 | `SpiInit()` 内调用 `SpiFlashReadJEDECID(&gDevSpiFlash)` |

### 2.3 `driver/mdio.c` / `driver/mdio.h`

| 项 | 内容 |
|----|------|
| 用途 | FPGA GPIO 模拟 MDIO/SMI（IEEE 802.3 Clause 22） |
| 寄存器 | `0x40000020` PHY 复位；`0x40000024` MDC；`0x40000028` MDIO；`0x4000002C` 方向（0 入 / 1 出） |
| API | `SmiWriteBits()`, `SmiReadBits()`, `SmiSendTa()`, `SmiIdleState()`, `SmiRstLow/High()`, `MdioSet2Input/Output()` |
| 时序 | `_SmiDelay()` 循环约 20 次 |

### 2.4 `driver/fpga_com.c` / `driver/fpga_com.h`

| 项 | 内容 |
|----|------|
| 用途 | FPGA<->软核 UDP 代理共享内存 + 中断接收 |
| 结构体 | `CACHE_UDPDATA_TypeDef`（`#pragma pack(1)`）：`device_type`(u16), `reserved`(u8), `op_cmd`(u8), `addr`(u32), `len`(u16) |
| RX/TX | `CACHE_UDPDATA_RX=0x30000000`，`CACHE_UDPDATA_TX=0x30001000` |
| 握手 | `LEN=0x40000010`，`READY=0x40000014`，`EthIrqClr=0x40000018` |
| 中断 | `INT02_Handler()`：清中断 → 解析 device_type → 入 `gUdpSlaveCommSendQueue` |
| device_type | 中断侧接受 `0x0001~0x0005` |
| reserved | 寄存器宽度倍数：0→×1，1→×2，2→×3，3→×4 |
| 数据长度 | `data_size = len * reg_num + 10` |
| 超时 | `Commovertime()`：长时间无中断置 `comm_fail_flag`，约 120s 写 BMS 关机 |
| API | `FpgaComInit()`, `Commovertime()` |
| 全局 | `comm_fail_flag`, `comm_overtime_count`, `fpga_count` |

---

## 3. DEVICE 层

### 3.1 `device/dev_led.c` / `device/dev_led.h`

| 项 | 内容 |
|----|------|
| 用途 | 单路系统 LED |
| IO | `LED_IO = 0x40000000` |
| 枚举 | `EN_LED_SRC`（`LED_SRC_SYS`）、`EN_LED_HW_MODE`、`EN_LED_STATUS`、`EN_LED_HW_ACTIVE_LEVEL` |
| 结构体 | `LedInfo_t`：hw_mode, active_level, status, period, brightness, tim_ch |
| API | `DevLedInit()`, `DevLedProcess()`, `LedCtlOn/Off/Blink()`, `DevLedInitSuccess/Failure()`, `LedBlinkTime()` |
| 行为 | 默认 1s 闪烁；`comm_fail_flag` 时 period=**100ms**；低电平有效 |

### 3.2 `device/dev_phy.c` / `device/dev_phy.h`

| 项 | 内容 |
|----|------|
| 用途 | Realtek **RTL8201F** 初始化（100M 全双工 RMII） |
| API | `DevPhy8201Init()`, `ConfigRtl8201(phy_addr)` |
| 流程 | 复位 100ms → 配置 PHY addr **0x00** 与 **0x01**（各重试 3 次） |
| 关键寄存器 | Reg0=`0xA100` 复位 → `0x2100` 强制百兆全双工；Page31 Reg16=`0x1FFE` RMII；LED 页配置 |
| 失败 | `DevLedInitFailure()`（5s 慢闪） |

### 3.3 `device/dev_spiflash.c` / `device/dev_spiflash.h`

| 项 | 内容 |
|----|------|
| 用途 | SPI NOR Flash 驱动 + FPGA SPI 寄存器访问 |
| 结构体 | `DevSpiFlash_t`：`arr_id[4]`, `n_capacity_MB` |
| 全局 | `gDevSpiFlash` |
| 页/扇区 | Page=256B，Sector=`0x1000`，Block32K=`0x8000`，Block64K=`0x10000` |
| ≥32MB | 使用 4-byte 地址命令（`FLASH_*_4BYTE`） |
| 命令宏 | `FLASH_LOW_READ=0x03`，`FLASH_PAGE_PROGRAM=0x02`，`FLASH_SECTOR_ERASE=0x20`，`FLASH_BLOCK_ERASE_64K=0xD8`，`FLASH_JEDEC_ID=0x9F`，`FLASH_WRITE_ENABLE=0x06` 等 |
| API | `SpiFlashReadData/AllData`, `SpiFlashPageProgram`, `SpiFlashWriteBytesWith/WithoutErase`, `SpiFlashEraseSector/Block64K/Chip`, `SpiFlashReadJEDECID`, `SpiWriteFpga`, `SpiReadFpga` |
| FPGA SPI | 写命令 **0x45**，读命令 **0x43**，16-bit 地址 + 数据 |

容量判定（JEDEC byte2）：0x14→1MB … 0x19→32MB，0x20/0x1A/0x3A→64MB。

### 3.4 `device/dev_ddr.c` / `device/dev_ddr.h`

| 项 | 内容 |
|----|------|
| 用途 | 监控卡经 UDP 写入 DDR 固件区的软核侧处理 |
| DDR 基址 | `DDR = 0x20000000`（`CMSDK_DDR_TypeDef`，每单元 16 字节） |
| 固件区 | FPGA=`0x24000000`，BMS=`0x24400000` |
| 结构体 | `DDR_Firmware_Info_t`：BMS/FPGA 各自 checksum_com/rece、pack_count、firmware_success、len |
| 固定 RAM | `bms_ddr_temp[256] @ 0x00007070`，`fpga_ddr_temp[256] @ 0x00007170` |
| API | `WriteDDRSend(start_addr, p, len)` |
| 逻辑 | 按 128 字节对齐写 DDR；首包解析固件长度（offset 7–10）与校验和（252–255）；累加 `CheckSumFun()` |

---

## 4. MIDDLEWARE 层

### 4.1 `middleware/mid_comservice.c` / `middleware/mid_comservice.h`

| 项 | 内容 |
|----|------|
| 用途 | UART 接收缓冲 + Nova 私有**响应帧**解析（`0xA5` 开头） |
| 缓冲 | `COMSERVICE_BUFF_SIZE = 550` |
| 队列 | `gstSciRevQueue`，深度 `SCI_REV_QUEUE_DEPTH=2` |
| 结构体 | `CommPort_t`、`SciRevData_t`、`cmoservice_cfg_t` |
| 状态机 | `EN_PROTOCOL_STATE_*`：`RECIVR_DATA_START` → `COM_DATA_BUSY` → `COM_DATA_END` |
| 响应头 | `_garrReponsePackHead[] = {0xA5, 0x03, 0x00}` |
| 识别命令 | 写响应：`0x8002`/`0x80F0`/`0x80F1`/`0x80FF`；读响应：`0x0001` |
| 升级回调 | `RegisterBMSUpdataFunPoint(BMSUpdataCallFun)`，收到 0x80F0/F1/FF 时调用 |
| API | `MidComserviceInit()`, `MidComserviceHandle()`, `MidComserviceProtocolProcess()`, `mid_comservice_data_in()`, `CheckSum()` |
| CheckSum | 字节累加 uint16（非 CRC） |

数据类型枚举含：`TYPE_COM_RRESPONSE_DATA`、`TYPE_COM_WRESPONSE_DATA`、`TYPE_MODBUS_DATA` 等。

---

## 5. APP 层

### 5.1 `app/modbus_analysis.c` / `.h`

| 项 | 内容 |
|----|------|
| 用途 | BMS/DCDC **寄存器镜像**定义与初始化（Modbus 风格 struct） |
| 镜像地址 | `gstBmsReg → 0x30010000`，`gstDCDCReg → 0x30020000` |
| 从机地址宏 | PCS=0x01，DCDC=0x02，MPPT=0x03，BMS=0x04 |
| 核心结构体 | `BMSOnlyReadPara_t`, `BMSHoldPara_t`, `BMSPara_t`, `DCDCOnlyReadPara_t`, `DCDCHoldPara_t`, `DCDCPara_t` |
| 全局 | `gSlaveBMSPara`, `gTempSlaveDCDCPara`, `extend_version[24]` |
| API | `BmsRegInit()`, `DCDCRegInit()` |
| DCDCRegInit | 从 Flash **`0x00C00000`** 读 24 字节扩展版本到 `version_extend` |

`ModbusCommProcess()` 整段注释，**运行时未启用** Modbus 主机轮询。

DCDC Hold 关键字段：

- `ip_address_high/low`
- `updata_status`（升级状态位图）
- `Shutdown`, `DCDC_OutputEn`, `recorden_status`
- `softreboot`：`0x50`=APP0，`0x51`=BOOT

### 5.2 `app/private_analysis.c` / `.h`

| 项 | 内容 |
|----|------|
| 用途 | Nova 私有协议组包/解包、UDP 分发、BMS 读写 |
| 命令 | 读 `0x0001`，写 `0x8002`，升级 `0x80F0/0x80F1/0x80FF` |
| 缓冲 | `COM_RX_SIZE=1100`，`garrBMSTxbuf[600]`，`garrDCDCTxbuf[600]` |
| 队列 | `gBmsSendQueue`, `gUdpSlaveCommSendQueue`, `gBMSModebusRevQueue` |
| API | `SlaveCommInit()`, `ReadBMSReg()`, `WriteBmsData()`, `WriteDCDCData()`, `ProtoUnpackJudge()`, `UdpProtopackJudge()`, `BMSWriteDataSeed()`, `RevBMSUpdataAnalysis()`, `CheckSum()`, `CheckSumFun()` |

**UDP 地址空间分发（`UdpProtopackJudge`）：**

| 地址范围 | op_cmd | 动作 |
|----------|--------|------|
| `0x30010000`~`0x300100FF` | 0x43 | 读 BMS 镜像 → TX 缓存 |
| `0x30020000`~`0x300200FF` | 0x43 | 读 DCDC 镜像 → TX 缓存 |
| `0x30030000`~`0x300CFFFF` | 0x43 | 读故障录波（Flash `0x004C2000` 起，每帧 0x90 字节） |
| `0x30010000`~`0x3001FFFF` | 0x45 | 写 BMS → 入 `gBmsSendQueue` |
| `0x30020000`~`0x300200FF` | 0x45 | 写 DCDC hold |
| `0x20000000`~`0x246FFFFF` | 0x45 | 写 DDR 固件 → `WriteDDRSend()` |

### 5.3 `app/comm_analysis.c` / `.h`

| 项 | 内容 |
|----|------|
| 用途 | 主机轮询状态机（软核作为 BMS 上位） |
| 超时 | `HOST_COMM_REV_MAX_TIME=1000ms`，`HOST_COMM_POLLING_TIME=300ms` |
| 状态 | `EN_HOST_POLLING_STATUS_*` |
| API | `SlaveCommProcess()` |

流程要点：

1. 取 UDP 队列 → `UdpProtopackJudge()`
2. 空闲优先处理 `gBmsSendQueue` 写 BMS
3. 否则 300ms 轮询：`ReadDCDCReg()`（本地 memcpy）/ `ReadBMSReg()`（UART）
4. 等 SCI 队列响应 → `ProtoUnpackJudge()`
5. BMS 连续 3 次超时 → 清零 BMS 镜像

### 5.4 `app/ddr_process.c` / `.h`

| 项 | 内容 |
|----|------|
| 用途 | DDR 固件包完整性校验 |
| API | `DDRFirmwareAnalysis()` |
| 魔数 | BMS `{0x0C,0x0C,0x0C,0x03}`；FPGA `{0x0C,0x0C,0x0C,0x04}` |
| 条件 | `ddr_*_check_sum_com == ddr_*_check_sum_rece` 且非 0 → `ddr_*_firmware_success=1` |

### 5.5 `app/firmware_upgrate.c` / `.h`

| 项 | 内容 |
|----|------|
| 用途 | FPGA Flash 升级 + BMS UART 升级 + 状态回写 |
| Flash | 固件 `0x00261000`；版本 `0x00C00000`；升级标志 `0x00BFF000`（`0xA0`） |
| 结构体 | `BMSFirmwareUpgrateInfo_t`, `FPGAFirmwareUpgrateInfo_t` |
| 包大小 | **1024** 字节/包 |
| API | `UpdataStatusAnalysis()`, `FPGAFirmwareAnalysis()`, `BMSUpgrateFirmwareProcess()`, `BMSUPGradeSendStart/Busy/End()`, `FirmwareUpgrate()`, `SoftReboot()` |
| 软复位 | 写 **`0x40000030 = 1`** |

BMS 升级 UART 包：

- Start (`0x80F0`)：固件总长度(4B) + space_num(1B，固定 `0x02`)
- Busy (`0x80F1`)：index(2B) + 数据 + checksum32 低 16 位
- End (`0x80FF`)：无数据域

### 5.6 `app/fault_record_process.c` / `.h`

| 项 | 内容 |
|----|------|
| 用途 | FPGA 故障录波：DDR 环形区 → SPI Flash |
| Flash 基址 | **`0x004C2000`** |
| DDR 环形区 | **`0x21000000`~`0x2108FFFF`**（589824 字节） |
| 总量 | `144 * 4096 = 589824` 字节 |
| 触发 | `recorden==1` 且 `recorden_status==0` |
| 完成 | 置 `recorden_status=1` |
| API | `FPGARecordPorcess()`, `FPGAFaultAnalysis()` |
| 块大小 | 每次读 DDR 1024B，写 Flash 后地址环形卷回 |

### 5.7 `app/ut_test_mode.c` / `.h`

| 项 | 内容 |
|----|------|
| 用途 | DDR 读写单元测试 |
| API | `ut_test_process()` |
| 默认 | `ut_mode=0` 关闭 |
| 行为 | 每 2s 写 16 字节到 `0x20000000` 并读回比较 |

---

## 6. 模块一览表

| 模块 | 层 | 主要对外接口 |
|------|----|--------------|
| main | sys | `main` |
| systick | sys | `SystickInit`, `DelayMs`, `TimGetSystime`… |
| queue | sys | `QueueInit/_1`, `QueuePut/Get` |
| crc | sys | `CrcModbus`, 字节序拷贝 |
| uart | driver | `Uart0Init/Send` |
| spi | driver | `SpiInit/Send/Read` |
| mdio | driver | `SmiWrite/ReadBits`… |
| fpga_com | driver | `FpgaComInit`, `Commovertime` |
| dev_led | device | `DevLedInit/Process` |
| dev_phy | device | `DevPhy8201Init` |
| dev_spiflash | device | Flash 读写擦、FPGA SPI |
| dev_ddr | device | `WriteDDRSend` |
| mid_comservice | mid | `MidComserviceInit/Handle` |
| modbus_analysis | app | `BmsRegInit`, `DCDCRegInit` |
| private_analysis | app | 组包解包、UDP 分发 |
| comm_analysis | app | `SlaveCommProcess` |
| ddr_process | app | `DDRFirmwareAnalysis` |
| firmware_upgrate | app | `UpdataStatusAnalysis`… |
| fault_record_process | app | `FPGARecordPorcess` |
| ut_test_mode | app | `ut_test_process` |
