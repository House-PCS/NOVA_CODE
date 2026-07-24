# SoftCore 源码与超详细解析索引

## 1. 源码在哪里？

源码就在仓库根目录，**不在** `Software/Doc` 里。按分层如下：

| 目录 | 职责 | 文件 |
|------|------|------|
| `sys/` | 入口、节拍、队列、CRC | `main.c/h`, `systick.c/h`, `queue.c/h`, `crc.c/h` |
| `middleware/` | UART 帧解析中间件 | `mid_comservice.c/h` |
| `driver/` | 底层驱动 | `uart`, `spi`, `mdio`, `fpga_com` |
| `device/` | 设备封装 | `dev_led`, `dev_phy`, `dev_spiflash`, `dev_ddr` |
| `app/` | 业务协议与状态机 | `private_analysis`, `comm_analysis`, `modbus_analysis`, `firmware_upgrate`, `ddr_process`, `fault_record_process`, `ut_test_mode` |
| `cmsis/` | 启动文件与 CMSIS | `startup_CMSDK_CM0.s`, `system_CMSDK_CM0.c` |
| `softcore.uvprojx` | Keil 工程 | 编译/链接配置 |

绝对路径示例：`e:\FPGA_SoftCore\app\private_analysis.c`

## 2. 非常详细的源码解析在哪里？

在本目录 `Software/Doc/source_analysis/`：

| 文档 | 内容 |
|------|------|
| [app_source_analysis.md](./app_source_analysis.md) | **app 层**逐文件/逐函数解析（协议、轮询、升级、录波） |
| [driver_device_source_analysis.md](./driver_device_source_analysis.md) | **driver + device** 寄存器、中断、初始化副作用 |
| [sys_middleware_source_analysis.md](./sys_middleware_source_analysis.md) | **sys + middleware** 启动链、主循环、协议状态机、队列/CRC |

上级目录 `Software/Doc/01~05_*.md` 是**架构/协议/流程速查**，不是逐行源码解析。

## 3. 建议阅读顺序

1. 源码目录对照（本文第 1 节）
2. `../01_software_architecture.md`（先建立全局图）
3. `sys_middleware_source_analysis.md`（从 main / 中断 / UART 状态机入手）
4. `driver_device_source_analysis.md`（硬件通路）
5. `app_source_analysis.md`（业务与协议细节）
6. `../03_communication_protocol.md` + `../05_key_workflows.md`（协议与流程对照）

## 4. 注意

- 部分文件若工作区缺失，解析依据编译产物 `output/*.lst` / `*.i` 还原（文档内会注明）。
- 文档均为 **UTF-8 BOM**，请用 UTF-8 打开。