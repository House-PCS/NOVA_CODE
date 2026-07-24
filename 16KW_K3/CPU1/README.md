# Roca PCS CPU1 固件

基于 TI TMS320F28375D 的三相混合储能 PCS **CPU1** 工程。

## 文档

详细模块功能说明（不含 TI 芯片支持库）见：

**[doc/模块功能详解/00_文档索引.md](doc/模块功能详解/00_文档索引.md)**

含：系统架构、应用层全模块、驱动/设备/中间件/算法层详解，以及非库源码附录。

## 目录结构（自研）

| 目录 | 说明 |
|------|------|
| `app/` | 应用：主循环、保护、通信、能量管理、业务 FSM |
| `algo/` | CLA 逆变控制与 CPU?CLA 共享 |
| `device/` | 风扇、NTC、CT、继电器、RCMU、CH390 等 |
| `driver/` | GPIO/ADC/PWM/SCI/CAN/定时器/双核等封装 |
| `middleware/` | 队列、CRC、滤波、通信帧解析、FSM 框架 |
| `cmd/` | 链接脚本 |
| `include/` `source/` | TI 库（文档不展开） |
