# Roca2-16KH FPGA SoftCore 软件文档

> 产品输出名：`Roca2-16KH_Fpga_Softcore`  
> 目标平台：ARM Cortex-M0（CMSDK_CM0）软核  
> 版权：西安诺瓦星云科技股份有限公司（NovaStar）

## 你要找的两样东西

| 需求 | 位置 |
|------|------|
| **源码本身** | 仓库根目录：`app/` `middleware/` `driver/` `device/` `sys/` `cmsis/` |
| **非常详细的源码解析** | [`source_analysis/`](./source_analysis/README.md)（逐文件、逐函数、宏/结构体/数据流） |

## A. 源码级超详细解析（重点）

见目录：[source_analysis/README.md](./source_analysis/README.md)

- [app 层](./source_analysis/app_source_analysis.md)
- [driver/device 层](./source_analysis/driver_device_source_analysis.md)
- [sys/middleware 层](./source_analysis/sys_middleware_source_analysis.md)

（另有副本：`06_driver_device_source_analysis.md`）

## B. 架构 / 协议 / 流程速查

| 文档 | 说明 |
|------|------|
| [01_software_architecture.md](./01_software_architecture.md) | 总体架构、启动、主循环 |
| [02_module_interfaces.md](./02_module_interfaces.md) | 模块接口一览 |
| [03_communication_protocol.md](./03_communication_protocol.md) | UART / UDP 协议 |
| [04_memory_map_constants.md](./04_memory_map_constants.md) | 地址与常量 |
| [05_key_workflows.md](./05_key_workflows.md) | 升级 / 录波等业务流程 |

## 快速结论

- SoftCore 为 FPGA 内嵌 Cortex-M0 通信管控中间层，无 RTOS。
- UART0(115200) <-> BMS（私有协议 0x5A/0xA5）；FPGA 共享内存 + INT02 <-> 监控卡。
- 实际 Scatter：IROM/IRAM 各 16KB；系统时钟 50 MHz。