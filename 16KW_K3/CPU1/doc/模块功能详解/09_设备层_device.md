# 09 设备层 device

对应源码：`device/*`

---

## 1. 风扇 `fan`

- `FAN_NUM=1`，反馈 GPIO80，PWM 用 EPWM10/11  
- 状态机：`INIT → SELF_TEST → NORMAL`；故障 `FAULT → RECOVERY → 再自检`  
- `FanProcessAll()`：主循环读 NTC 最高温，算档位更新 PWM  
- 温控阈值（约）：低于 75/75/60°C 关；超 85/85/70°C→50%；超 105/105/80°C→80%；80% 有滞回  
- `_FanFaultProcess`：约 0.25 s 窗统计反馈占空比，脉冲模式高电平宜 30%~70%  
- 快故障 ISR 调 `FanFaultProcessAll()`  
- 注意：循环条件 `i <= FAN_NUM` 在 NUM=1 时多一次空调用（有边界保护）

---

## 2. NTC `ntc` / `dev_ntc`

- 三路：机箱 / PV / 逆变  
- 参数：B=4050，R25=10k，3.3V 上拉，10k 上拉，75k 下拉  
- `CalculateTemp`：电压→分压电阻→B 公式；`_ApproLn` 15 阶近似 ln  
- `NTCProcess`：每次一路，三轮一轮完整  
- 滤波：0.99/0.01 IIR → 30 点滑动平均；窗口有效后每 10 次判断线  
- 断线：≤-45°C 或 ≥160°C；恢复 -40~155°C  
- 采样层把 `Temp_A1/A2/A4` 注入；风扇读滤波温度  

---

## 3. CT `ct`

- `CtOutInfo_t`：各相与总 P/Q/S  
- `CtAutoCorrentPower`：比较电网与 CT 功率符号，设每相 ±1 极性  
- `CtReversePower`：按用户反向选项与自动极性修正 P/Q（S 不反向）  
- `CtbreakJudge`：每次一相，30 点滑窗，电流标准差 <0.8 判断线；三次覆盖三相  
- 快故障 ISR 调用  

---

## 4. 继电器 `relay_ctrl`

### 控制

`RelayControl`：逻辑类型 → GPIO；维护 `open_flag`（实际语义偏“命令闭合”）。

### 自检 `RelaySelfTest` 状态机

`IDLE → CHECK_GRID → RELAY_CLOSE_TEST → 完成/失败`

1. 启动后等 200 ms  
2. 断 GRID/GEN/LOAD  
3. 等 1 s：市电三相正常、其它节点失电，累计 5 次后吸合 GRID  
4. 再等 1 s：中点瞬时电压跟随市电，GEN/LOAD/INV 不应带电  
5. 连续成功通过并断 GRID；累计到 0 则失败  

由 `post` 填充瞬时值与 RMS 后周期调用。

---

## 5. RCMU 漏电流 `rcmu`

设计为约 20 kHz ISR 调用。

`LeakCurrProcess`：

1. 初始化约 500 Hz 一阶巴特沃斯  
2. 按逆变频率（限 30~70 Hz）定每周波样点  
3. 滤波、累平方、完整周波出 RMS  
4. 状态机：预检 → 自检 → 5 s 延时 → 正常检测 / 锁定故障  

自检：预检 RMS <30 mA 才拉 GPIO28；再要求 40~60 mA 连续 5 次。  
正常故障：280 mA×12 周波，或 150/60/30 mA 阶跃检测。  
输入来自 `gstAdcResult.leak_curr`。

---

## 6. CH390 以太网 `CH390` / `CH390_Interface`

- SPIB + CS66 + RST67 + INT68  
- Interface：SPI 单字节阻塞交换；寄存器与 RX/TX SRAM 顺序读写  
- `ch390_receive_packet`：RX Ready；异常复位 RX FIFO；读 4B 头/状态/长度；≤1536 读帧  
- `ch390_send_packet`：等发送完成，写 TX SRAM，设长度，请求发送  
- 另有 PHY/EEPROM、MAC、多播哈希、RX 过滤、睡眠、WoL、GPIO、INT 极性等基础接口  
- `CH390Init`：软件复位后最多等约 5 s 链路  
- `CH390RestartProcess`：非阻塞重启，约 100 ms 轮询，最多约 5 s  

应用层多为轮询收包（未见完整 GPIO68 外部中断业务路径）。

---

## 7. LED `dev_led`

- GPIO99，低有效  
- ON/OFF、任意周期闪烁、待机单闪、成功双闪、失败四闪  
- 基于 `TimGetSystime()` 非阻塞  

---

## 8. Flash 接口 `dev_flash.h`

定义 SPI Flash 命令、页 256B、扇区 4KiB、块 64KiB、升级结构 `SpiFlash_UpData_t` 与擦读写接口。  
本目录若无实现 `.c`，则以工程其它位置实现为准，本文只记录接口契约。

---

## 附录：本章源码

> 以下为本章涉及的全部自研源码原文（不含 TI `include/`、`source/` 库）。

### `device/CH390.c`

```c
/********************************** (C) COPYRIGHT *****************************
 * File Name          : CH390.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2024/08/20
 * Description        : CH390 Ethernet controller source file
 ******************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 ******************************************************************************/
#include "CH390.h"
#include "CH390_Interface.h"
#include "stdio.h"
#include "cpu_timers.h"

static uint16_t _gCH390Restart = 0;
/**
 * @name ch390_receive_packet
 * @brief Receive packet
 * @param buff - Size equal to CH390_PKT_MAX
 * @param rx_status - Output abnormal status while receiving packet.
 *                    It has the same meaning as RSR(06h).
 * @return Packet length
 */
uint32_t ch390_receive_packet(uint8_t *buff, uint8_t *rx_status)
{
    uint8_t rx_ready;
    uint16_t rx_len = 0;
	uint8_t ReceiveData[4];

    // Check packet ready or not
    ch390_read_reg(CH390_MRCMDX);
    rx_ready = ch390_read_reg(CH390_MRCMDX);

    // if rxbyte != 1 or 0 reset device
    if (rx_ready & CH390_PKT_ERR)
    {
        // Reset RX FIFO pointer
        ch390_write_reg(CH390_RCR, 0);        //RX disable
        ch390_write_reg(CH390_MPTRCR, 0x01);  //Reset RX FIFO pointer
        ch390_write_reg(CH390_MRRH, 0x0c);
        ch390_delay_us(1000);
        ch390_write_reg(CH390_RCR, RCR_RXEN | RCR_DIS_CRC); //RX Enable
        return 0;
    }
    if (!(rx_ready & CH390_PKT_RDY))
    {
        return 0;
    }

    ch390_read_mem(ReceiveData, 4);

    *rx_status = ReceiveData[1];
    rx_len = ReceiveData[2] | (ReceiveData[3] << 8);

    if(rx_len <= CH390_PKT_MAX)
    {
        ch390_read_mem(buff, rx_len);
    }

    if ((*rx_status & 0x3f) || (rx_len > CH390_PKT_MAX))
    {
        return 0;
    }
    return rx_len;
}


/**
 * @name ch390_send_packet
 * @brief Send packet
 * @param buff - Data to be sent
 * @param length - Less than 3k bytes.
 */
void ch390_send_packet(uint8_t *buff, uint16_t length)
{
    // Wait until last transmit complete
    while(ch390_read_reg(CH390_TCR) & TCR_TXREQ);
    // Write data to SRAM
    ch390_write_mem(buff, length);

    // Set current packet length
    ch390_write_reg(CH390_TXPLL, length & 0xff);
    ch390_write_reg(CH390_TXPLH, (length >> 8) & 0xff);
    // Issue transmit request
    ch390_send_request();
}

/**
 * @name ch390_send_request
 * @brief Issue transmit request
 */
void ch390_send_request()
{
    uint8_t tcr = ch390_read_reg(CH390_TCR);
    ch390_write_reg(CH390_TCR, tcr | TCR_TXREQ);
}

/**
 * @name ch390_drop_packet
 * @brief Drop packet in RX SRAM if don't want to read it. This function
 *        modify the memory data read pointer and skip specified length
 * @param len - Skip length, length of the current packet.
 */
void ch390_drop_packet(uint16_t len)
{
    uint16_t mdr = ch390_read_reg(CH390_MRRL) | (ch390_read_reg(CH390_MRRH) << 8);
#ifdef CH390_INTERFACE_16_BIT
    mdr = mdr + (len + 1) / 2 * 2;
#else
    mdr = mdr + len;
#endif
    mdr = mdr < 0x4000 ? mdr : mdr - 0x3400;
    ch390_write_reg(CH390_MRRL, mdr & 0xff);
    ch390_write_reg(CH390_MRRH, (mdr >> 8) & 0xff);
}

/**
 * @name ch390_read_phy
 * @brief Read PHY register
 * @param reg - PHY register address
 */
uint16_t ch390_read_phy(uint8_t reg)
{
    ch390_write_reg(CH390_EPAR, CH390_PHY | reg);
    // Chose PHY, send read command
    ch390_write_reg(CH390_EPCR, EPCR_ERPRR | EPCR_EPOS);
    while(ch390_read_reg(CH390_EPCR) & 0x01);
    // Clear read command
    ch390_write_reg(CH390_EPCR, 0x00);
    return (ch390_read_reg(CH390_EPDRH) << 8) |
           (ch390_read_reg(CH390_EPDRL) & 0xFF);
}

/**
 * @name ch390_write_phy
 * @brief Write PHY register
 * @param reg - PHY register address
 * @param value - Value to be written
 */
void ch390_write_phy(uint8_t reg, uint16_t value)
{
    ch390_write_reg(CH390_EPAR, CH390_PHY | reg);
    ch390_write_reg(CH390_EPDRL, (value & 0xff));        // Low byte
    ch390_write_reg(CH390_EPDRH, ((value >> 8) & 0xff)); // High byte
    // Chose PHY, send write command
    ch390_write_reg(CH390_EPCR, 0x0A);
    while(ch390_read_reg(CH390_EPCR) & 0x01);
    // Clear write command
    ch390_write_reg(CH390_EPCR, 0x00);
}

/**
 * @name ch390_write_eeprom
 * @brief Write EEPROM register
 * @param reg - EEPROM register address
 * @param value - Value to be written
 */
void ch390_write_eeprom(uint8_t reg, uint16_t value)
{
    ch390_write_reg(CH390_EPAR, reg);
    ch390_write_reg(CH390_EPDRL, (value & 0xff));        // Low byte
    ch390_write_reg(CH390_EPDRH, ((value >> 8) & 0xff)); // High byte
    // Chose EEPROM, send write command
    ch390_write_reg(CH390_EPCR, EPCR_ERPRW);
    while(ch390_read_reg(CH390_EPCR) & 0x01);
    // Clear write command
    ch390_write_reg(CH390_EPCR, 0x00);
}

/**
 * @name ch390_software_reset
 * @brief Software reset CH390 by NCR
 */
void ch390_software_reset()
{
    ch390_write_reg(CH390_NCR, NCR_RST);
    ch390_delay_us(10);
    ch390_write_reg(CH390_NCR, 0);
    ch390_write_reg(CH390_NCR, NCR_RST);
    ch390_delay_us(10);
}

/**
 * @name ch390_default_config
 * @brief Config CH390 with default options:
 *        LED mode 1;
 *        Enable transmit check sum generation;
 *        Enable RX;
 *        Enable all interrupt and PAR
 */
void ch390_default_config()
{
    // CH390 has built-in MAC, this is not necessary
    // uint8_t mac_addr[6] = { 0x50, 0x54, 0x7B, 0x84, 0x00, 0x73 };
    // Multicast address hash table
    uint8_t multicase_addr[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    ch390_set_phy_mode(CH390_AUTO);
    // Clear status
    ch390_write_reg(CH390_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END);
    ch390_write_reg(CH390_ISR, 0xFF);         // Clear interrupt status
    ch390_write_reg(CH390_TCR2, 0x80);        // LED mode 1
    ch390_write_reg(CH390_TCSCR, TCSCR_ALL);  // Enable check sum generation

    // ch390_set_mac_address(mac_addr);
    ch390_set_multicast(multicase_addr);

    // Enable all interrupt and PAR
    ch390_write_reg(CH390_IMR, IMR_ALL);
    // Enable RX
    ch390_write_reg(CH390_RCR, RCR_DIS_CRC | RCR_RXEN);
}

/**
 * @name ch390_set_phy_mode
 * @brief Set PHY mode and enable PHY.
 *        PHY mode: Auto-negotiation, 10M/100M, full-duplex/half-duplex
 * @param mode - PHY mode
 */
void ch390_set_phy_mode(enum ch390_phy_mode mode)
{
    uint16_t BMCR_value = 0;
    uint16_t ANAR_value = 0;
    switch (mode)
    {
    case CH390_10MFD:
        BMCR_value = 0x1100;
        ANAR_value = 0x41;
        break;
    case CH390_100MFD:
        BMCR_value = 0x3100;
        ANAR_value = 0x101;
        break;
    case CH390_AUTO:
        BMCR_value = 0x1000;
        ANAR_value = 0x01E1;
        break;
    }
    ch390_write_phy(CH390_PHY_BMCR, BMCR_value);
    ch390_write_phy(CH390_PHY_ANAR, ANAR_value);
    ch390_write_reg(CH390_GPR, 0x00);   // Enable PHY
}

/**
 * @name ch390_set_mac_address
 * @brief Set mac address
 * @param mac_addr - 6-byte length mac address array
 */
void ch390_set_mac_address(uint8_t *mac_addr)
{
    uint8_t i;
    for (i = 0; i < 6; i++)
    {
        ch390_write_reg(CH390_PAR + i, mac_addr[i]);
    }
    ch390_read_reg(0);
}

/**
 * @name ch390_set_multicast
 * @brief Set multicast address hash table
 * @param multicast_addr - 8-byte length multicast address hash table array
 */
void ch390_set_multicast(uint8_t *multicast_hash)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        ch390_write_reg(CH390_MAR + i, multicast_hash[i]);
    }
    ch390_read_reg(0);
}

/**
 * @brief reflect an 8bit value.
 *        Only for "ch390_compute_hash_bit"
 */
static uint8_t reflect_8(uint8_t val)
{
	  int i;
    uint8_t resVal = 0;

    for (i = 0; i < 8; i++)
    {
        if ((val & (1 << i)) != 0)
        {
            resVal |= 1 << (7 - i);
        }
    }
    return resVal;
}

/**
 * @brief Calculate the corresponding hash bit of the MAC address.
 *        Only for "ch390_set_hash_bit"
 * @param mac - Destination address
 * @return Hash bit number
 */
static uint8_t ch390_compute_hash_bit(uint8_t *mac)
{
	  int i;
    const uint32_t poly = 0x4C11DB7;
    uint32_t crc = 0xffffffff;

    int byte_i = 0;
    for(byte_i = 0; byte_i < 6; byte_i++)
    {
        uint8_t cur_byte = reflect_8(mac[byte_i]);
        crc ^= (uint32_t)cur_byte << 24;
        for (i = 0; i < 8; i++)
        {
            if ((crc & 0x80000000) != 0)
            {
                crc = (crc << 1) ^ poly;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return (crc ^ 0xffffffff) >> 26;
}

/**
 * @brief Set MAR bit for a particular MAC address
 * @param mac - Destination address
 */
void ch390_set_hash_bit(uint8_t *mac)
{
    uint8_t bit = ch390_compute_hash_bit(mac);
    uint8_t mar = CH390_MAR + bit / 8;

    uint8_t mar_val = ch390_read_reg(mar);
    mar_val |= 1 << (bit % 8);
    ch390_write_reg(mar, mar_val);
    ch390_read_reg(0);
}

/**
 * @name ch390_get_mac
 * @brief Get mac address
 * @param mac_addr - 6-byte length mac address output
 */
void ch390_get_mac(uint8_t *mac_addr)
{
    uint8_t i;
    for (i = 0; i < 6; i++)
    {
        mac_addr[i] = ch390_read_reg(CH390_PAR + i);
    }
    ch390_read_reg(0);
}

/**
 * @name ch390_get_multicast
 * @brief Get multicast address hash table
 * @param multicast_addr - 8-byte length multicast address hash table output
 */
void ch390_get_multicast(uint8_t *multicast_hash)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        multicast_hash[i] = ch390_read_reg(CH390_MAR + i);
    }
    ch390_read_reg(0);
}

/**
 * @name ch390_get_vendor_id
 * @brief Get vendor ID
 * @return Vendor ID
 */
uint16_t ch390_get_vendor_id()
{
    uint16_t id;
    id = (ch390_read_reg(CH390_VIDL) & 0xff);
    id |= ch390_read_reg(CH390_VIDH) << 8;
    return id;
}

/**
 * @name ch390_get_product_id
 * @brief Get product ID
 * @return Product ID
 */
uint16_t ch390_get_product_id()
{
    uint16_t id;
    id = (ch390_read_reg(CH390_PIDL) & 0xff);
    id |= ch390_read_reg(CH390_PIDH) << 8;
    return id;
}

/**
 * @name ch390_get_revision
 * @brief Get chip revision
 * @return Chip revision
 */
uint8_t ch390_get_revision()
{
    return ch390_read_reg(CH390_CHIPR);
}

/**
 * @name ch390_interrupt_config
 * @brief Interrupt configuration
 * @param mask - Interrupt to be enabled, see "CH390.h" IMR_xxx
 */
void ch390_interrupt_config(uint8_t mask)
{
    ch390_write_reg(CH390_IMR, mask);
}

/**
 * @name ch390_rx_enable
 * @brief Enable or disable packet receive
 * @param op - 0: disable  1: enable
 */
void ch390_rx_enable(int op)
{
    uint8_t rcr = ch390_read_reg(CH390_RCR);

    if(op == 0)
        rcr &= ~RCR_RXEN;
    else
        rcr |= RCR_RXEN;

    ch390_write_reg(CH390_RCR, rcr);
}

/**
 * @name ch390_rx_filter_config
 * @brief Configure receive filter.
 * @param config - See "CH390.h" RCR_xxx
 */
void ch390_rx_filter_config(uint8_t config)
{
    uint8_t rcr = ch390_read_reg(CH390_RCR) & RCR_RXEN;
    ch390_write_reg(CH390_RCR, rcr | config);
}

/**
 * @name ch390_wakeup_config
 * @brief Enable or disable wakeup_function
 * @param events - Events that trigger wakeup
 *                 WCR_LINKEN   - Link status change
 *                 WCR_SAMPLEEN - Sample frame
 *                 WCR_MAGICEN  - Magic packet
 *                 0            - Disable wakeup function
 */
void ch390_wakeup_config(uint8_t events)
{
    uint8_t ncr = ch390_read_reg(CH390_NCR);
    if(events)
        ncr |= NCR_WAKEEN;
    else {
        ncr &= ~NCR_WAKEEN;
    }
    ch390_write_reg(CH390_NCR, ncr);
    ch390_write_reg(CH390_WCR, events);
}

/**
 * @name ch390_wake_notify
 * @brief Wait for Magic Packet or Sample Frame and discard all
 *        other packets.
 *        If the application needs to use Wake On LAN, call this
 *        function every time before MCU enters low power mode.
 *        An external interrupt signal is accessible on WOL pin
 *        when wake up event occurred.
 */
void ch390_wake_notify(void)
{
    uint8_t ncr = ch390_read_reg(CH390_NCR);
    ch390_write_reg(CH390_NCR, ncr ^ 0x10);
}

/**
 * @name ch390_loop_back_enable
 * @brief Enable loop back mode
 * @param op - 0: disable  1: enable
 */
void ch390_loop_back_enable(int op)
{
    uint8_t ncr = ch390_read_reg(CH390_NCR) & ~0x06;

    if(op == 1) ncr |= NCR_LBK_MAC;

    ch390_write_reg(CH390_NCR, ncr);
}

/**
 * @name ch390_get_duplex_mode
 * @brief Get current duplex mode of the internal PHY
 * @return 0: Half-duplex  1: Full-duplex
 */
int ch390_get_duplex_mode()
{
    return !!(ch390_read_reg(CH390_NCR) & NCR_FDX);
}

/**
 * @name ch390_get_phy_speed
 * @brief Get the speed of the internal PHY.
 *        Only valid after PHY linked
 * @return 0: 100Mbps  1: 10Mbps
 */
int ch390_get_phy_speed()
{
    return !!(ch390_read_reg(CH390_NSR) & NSR_SPEED);
}

/**
 * @name ch390_get_link_status
 * @brief Get link status of the internal PHY
 * @return 0: Link failed  1: Link OK
 */
int ch390_get_link_status()
{
    uint8_t nsr = ch390_read_reg(CH390_NSR);//·¢ËÍ01
    return !!(nsr & NSR_LINKST);
}

/**
 * @name ch390_sleep_control
 * @brief Enter or exit sleep mode
 * @param op - 0: Power up  1: Power down
 */
void ch390_sleep_control(int op)
{
    if(op)
    {
        ch390_write_reg(CH390_SCCR, 0x01);
    }
    else
    {
        ch390_read_reg(CH390_RSCCR);
        ch390_delay_us(100);
    }
}

#ifndef CH390_INTERFACE_16_BIT
/**
 * @name ch390_gpio_config
 * @brief Config the input/output direction of GPIO1~3
 *        In 8-bit mode, GPIO4~6 are output only
 * @param GPIOx - CH390_GPIO1 ~ CH390_GPIO3
 *        dir - 0: Input  1: Output
 */
void ch390_gpio_config(uint8_t GPIOx, uint8_t dir)
{
    uint8_t gpcr = ch390_read_reg(CH390_GPCR);
    if(dir)
    {
        gpcr |= GPIOx;
    }
    else {
        gpcr &= ~GPIOx;
    }
    ch390_write_reg(CH390_GPCR, gpcr);
}

/**
 * @name ch390_gpio_write_bit
 * @brief Sets or clears the selected gpio bit.
 *        In SPI mode, only GPIO1~3 are available
 * @param GPIOx - CH390_GPIO1 ~ CH390_GPIO6
 *        level - 0: Clear pin  1: Set pin
 */
void ch390_gpio_write_bit(uint8_t GPIOx, uint8_t level)
{
    uint8_t gpr = ch390_read_reg(CH390_GPR);
    if(level)
    {
        gpr |= GPIOx;
    }
    else {
        gpr &= ~GPIOx;
    }
    ch390_write_reg(CH390_GPR, gpr);
}

/**
 * @name ch390_gpio_read_bit
 * @brief Read gpio input, only CH390_GPIO1 ~ 3 are available
 * @param GPIOx - CH390_GPIO1 ~ CH390_GPIO3
 * @return Input pin value
 */
uint8_t ch390_gpio_read_bit(uint8_t GPIOx)
{
    uint8_t gpr = ch390_read_reg(CH390_GPR);
    return !!(gpr & GPIOx);
}
#endif

/**
 * @name ch390_int_pin_config
 * @brief Configure INT pin output type and polarity
 * @param type - INCR_TYPE_OD: Open drain output
 *               INCR_TYPE_PP: Push pull output
 *        pol - INCR_POL_L: Active low
 *              INCR_POL_H: Active high
 */
void ch390_int_pin_config(uint8_t type, uint8_t pol)
{
    ch390_write_reg(CH390_INTCR, type | pol);
}

/**
 * @name ch390_get_int_status
 * @brief Get CH390 interrupt status and clear them
 * @return Interrupt status
 */
uint8_t ch390_get_int_status()
{
    uint8_t int_status = ch390_read_reg(CH390_ISR);
    // Clear interrupt status by write 1
    ch390_write_reg(CH390_ISR, int_status);
    return int_status;
}
/**
 * @name ch390_get_int_status
 * @brief Get CH390 interrupt status and clear them
 * @return Interrupt status
 */
bool CH390Init(void)
{
    uint16_t i = 0;

    ch390_software_reset();
    /*5SµÈ´ý*/
    for(i = 0; i < 50; i++)
    {
        DELAY_MS(100);
        if(!ch390_get_link_status())
        {
            ch390_default_config();
        }
        else
        {
       //     printf("link up\r\n");
            ch390_write_reg(CH390_ISR, ISR_LNKCHG);
            return true;
        }
    }

    return false;
}

void CH390RestartEnable(void)
{
    _gCH390Restart = 1;
}

uint8_t CH390RestartProcess(void)
{
    static uint16_t sta = 0;
    static uint32_t timeout = 0;
    static uint16_t waitcount = 0;

    if(sta == 0)
    {
        if(_gCH390Restart == 1)
        {
            _gCH390Restart = 0;
            ch390_software_reset();
            sta = 1;
        }
    }
    else if(sta == 1)
    {
        timeout = TimGetSystime();
        sta = 2;
    }
    else if(sta == 2)
    {
        if(TimDifftimeFromNow(timeout) < 100)
        {
            return 0;
        }

        waitcount++;
        if(!ch390_get_link_status())
        {
            ch390_default_config();
            sta = 1;
        }
        else
        {
       //     printf("link up\r\n");
            ch390_write_reg(CH390_ISR, ISR_LNKCHG);
            sta = 0;
            return 1;
        }

        if(waitcount > 50)
        {
            waitcount = 0;
            sta = 0;
            return 2;
        }
    }
    return 0;
}

/**
 * @name ch390_ut_send_test
 * @brief
 */
void ch390_ut_send_test(void)
{
    uint8_t  packet_data[60];

    for(int i = 0;i < 6;i++)//0-5广播目标
    {
        packet_data[i] = 0xFF;
    }
    /*      MAC部分    */
    packet_data[6] = 0x11;
    packet_data[7] = 0x22;
    packet_data[8] = 0x33;
    packet_data[9] = 0x44;
    packet_data[10] = 0x55;
    packet_data[11] = 0x66;
    /*      IPV4    */
    packet_data[12] = 0x08;
    packet_data[13] = 0x00;
    /*      网络版本，如下为网络层    */
    packet_data[14] = 0x45;//版本为4,网络层共20个字节
    packet_data[15] = 0x00;//DSCP区分服务代码点
    packet_data[16] = ((20 + 8 + 16) >> 8);//长度高位，包含网络层+应用层+数据层长度
    packet_data[17] = ((20 + 8 + 16) & 0xFF);//长度低位，包含网络层+应用层+数据层长度
    packet_data[18] = 0x66;//报文ID标识,高位
    packet_data[19] = 0x88;//报文ID标识.低位
    packet_data[20] = 0x00;//分包标识.高位
    packet_data[21] = 0x00;//分包标识.高位
    packet_data[22] = 0x80;//存活时间
    packet_data[23] = 0x11;//UDP协议
    packet_data[24] = 0x00;//头部检查高位
    packet_data[25] = 0x00;//头部检查低位

    packet_data[26] = 192;//源地址
    packet_data[27] = 168;
    packet_data[28] = 1;
    packet_data[29] = 18;

    packet_data[30] = 192;//目标地址
    packet_data[31] = 168;
    packet_data[32] = 1;
    packet_data[33] = 16;
    /*      如下为应用层    */
    packet_data[34] = 0x08;//源端口2300
    packet_data[35] = 0xFC;
    packet_data[36] = 0x03;  //目标端口1000
    packet_data[37] = 0xE8;
    packet_data[38] = (8 + 16) >> 8;//应用层加数据长度
    packet_data[39] = (8 + 16) & 0xFF;
    packet_data[40] = 0x00;//校验码
    packet_data[41] = 0x00;//校验码
    /*      如下为数据层    */
    for(int i = 42; i < (42 + 16); i++)
    {
        packet_data[i] = i - 41;
    }
    ch390_send_packet(packet_data, 42 + 16);
}
```

### `device/CH390.h`

```c
/********************************** (C) COPYRIGHT *****************************
 * File Name          : CH390.h
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2024/08/20
 * Description        : CH390 Ethernet controller header file
 ******************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 ******************************************************************************/
#ifndef __CH390_H
#define __CH390_H

#include <stdint.h>
#include "F28x_Project.h"
#include <stdbool.h>
/********************************************************************
 * Interface selection
 */

#define CH390_INTERFACE_SPI     // CH390H
// #define CH390_INTERFACE_8_BIT   // CH390L 8-bit mode
// #define CH390_INTERFACE_16_BIT  // CH390L 16-bit mode

/* PHY mode definition */
enum ch390_phy_mode
{
    CH390_10MFD,   // 10M full-duplex
    CH390_100MFD,  // 100M full-duplex
    CH390_AUTO,    // Auto negotiation
};

/********************************************************************
 * Register definition
 * 
 * There are some differences between the register definitions of
 * CH390H and CH390L
 */

#ifdef CH390_INTERFACE_SPI
#define CH390_NCR       0x00
    #define NCR_WAKEEN      (1<<6) // Enable wakeup function
    #define NCR_FDX         (1<<3) // Duplex mode of the internal PHY
    #define NCR_LBK_MAC     (1<<1) // MAC loop-back
    #define NCR_RST         (1<<0) // Softwate reset
#define CH390_NSR       0x01
    #define NSR_SPEED       (1<<7) // Speed of internal PHY
    #define NSR_LINKST      (1<<6) // Link status of internal PHY
    #define NSR_WAKEST      (1<<5) // Wakeup event status
    #define NSR_TX2END      (1<<3) // Tx packet B complete status
    #define NSR_TX1END      (1<<2) // Tx packet A complete status
    #define NSR_RXOV        (1<<1) // Rx fifo overflow
    #define NSR_RXRDY       (1<<0)
#define CH390_TCR       0x02
    #define TCR_TJDIS       (1<<6) // Transmit jabber timer
    #define TCR_PAD_DIS2    (1<<4) // PAD appends for packet B
    #define TCR_CRC_DIS2    (1<<3) // CRC appends for packet B
    #define TCR_PAD_DIS1    (1<<2) // PAD appends for packet A
    #define TCR_CRC_DIS1    (1<<1) // CRC appends for packet A
    #define TCR_TXREQ       (1<<0) // Tx request
#define CH390_TSRA      0x03
#define CH390_TSRB      0x04
    #define TSR_TJTO        (1<<7) // Transmit jabber time out
    #define TSR_LC          (1<<6) // Loss of carrier
    #define TSR_NC          (1<<5) // No carrier
    #define TSR_LCOL        (1<<4) // Late collision
    #define TSR_COL         (1<<3) // Collision packet
    #define TSR_EC          (1<<2) // Excessive collision
#define CH390_RCR       0x05
    #define RCR_DEFAULT     0x00   // Default settings
    #define RCR_WTDIS       (1<<6) // Disable 2048 bytes watch dog
    #define RCR_DIS_CRC     (1<<4) // Discard CRC error packet
    #define RCR_ALL         (1<<3) // Pass all multicast
    #define RCR_RUNT        (1<<2) // Pass runt packet
    #define RCR_PRMSC       (1<<1) // Promiscuous mode
    #define RCR_RXEN        (1<<0) // Enable RX
#define CH390_RSR       0x06
    #define RSR_RF          (1<<7) // Rnt frame
    #define RSR_MF          (1<<6) // Multicast frame
    #define RSR_LCS         (1<<5) // Late collision seen
    #define RSR_RWTO        (1<<4) // Receive watchdog time-out
    #define RSR_PLE         (1<<3) // Physical layer error
    #define RSR_AE          (1<<2) // Alignment error
    #define RSR_CE          (1<<1) // CRC error
    #define RSR_FOE         (1<<0) // FIFO overflow error
#define CH390_ROCR      0x07
#define CH390_BPTR      0x08
#define CH390_FCTR      0x09
    #define FCTR_HWOT(ot)   (( ot & 0xf ) << 4)
    #define FCTR_LWOT(ot)   ( ot & 0xf )
#define CH390_FCR       0x0A
#define CH390_EPCR      0x0B
    #define EPCR_REEP       (1<<5) // Reload EEPROM
    #define EPCR_EPOS       (1<<3) // EEPROM or PHY operation select
    #define EPCR_ERPRR      (1<<2) // EEPROM or PHY read command
    #define EPCR_ERPRW      (1<<1) // EEPROM or PHY write command
    #define EPCR_ERRE       (1<<0) // EEPROM or PHY access status
#define CH390_EPAR      0x0C
#define CH390_EPDRL     0x0D
#define CH390_EPDRH     0x0E
#define CH390_WCR       0x0F
    #define WCR_LINKEN      (1<<5) // Link status change wakeup
    #define WCR_SAMPLEEN    (1<<4) // Sample frame wakeup
    #define WCR_MAGICEN     (1<<3) // Magic packet wakeup
    #define WCR_LINKST      (1<<2) // Link status change event
    #define WCR_SAMPLEST    (1<<1) // Sample frame event
    #define WCR_MAGICST     (1<<0) // Magic packet event
#define CH390_PAR       0x10
#define CH390_MAR       0x16
#define CH390_GPCR      0x1E
#define CH390_GPR       0x1F
#define CH390_TRPAL     0x22
#define CH390_TRPAH     0x23
#define CH390_RWPAL     0x24
#define CH390_RWPAH     0x25
#define CH390_VIDL      0x28
#define CH390_VIDH      0x29
#define CH390_PIDL      0x2A
#define CH390_PIDH      0x2B
#define CH390_CHIPR     0x2C
#define CH390_TCR2      0x2D
#define CH390_ATCR      0x30
#define CH390_TCSCR     0x31
    #define TCSCR_ALL         0x1F
    #define TCSCR_IPv6TCPCSE  (1<<4) // IPv6 TCP checksum generation
    #define TCSCR_IPv6UDPCSE  (1<<3) // IPv6 UDP checksum generation
    #define TCSCR_UDPCSE      (1<<2) // UDP checksum generation
    #define TCSCR_TCPCSE      (1<<1) // TCP checksum generation
    #define TCSCR_IPCSE       (1<<0) // IP checksum generation
#define CH390_RCSCSR    0x32
    #define RCSCSR_UDPS     (1<<7) // UDP checksum status
    #define RCSCSR_TCPS     (1<<6) // TCP checksum status
    #define RCSCSR_IPS      (1<<5) // IP checksum status
    #define RCSCSR_UDPP     (1<<4) // UDP packet of current received packet
    #define RCSCSR_TCPP     (1<<3) // TCP packet of current received packet
    #define RCSCSR_IPP      (1<<2) // IP packet of current received packet
    #define RCSCSR_RCSEN    (1<<1) // Receive checksum checking enable
    #define RCSCSR_DCSE     (1<<0) // Discard checksum error packet
#define CH390_MPAR      0x33
#define CH390_SBCR      0x38
#define CH390_INTCR     0x39
    #define INCR_TYPE_OD    0x02
    #define INCR_TYPE_PP    0x00
    #define INCR_POL_L      0x01
    #define INCR_POL_H      0x00
#define CH390_ALNCR     0x4A
#define CH390_SCCR      0x50
#define CH390_RSCCR     0x51
#define CH390_RLENCR    0x52
#define CH390_BCASTCR   0x53
#define CH390_INTCKCR   0x54
#define CH390_MPTRCR    0x55
#define CH390_MLEDCR    0x57
#define CH390_MRCMDX    0x70
#define CH390_MRCMDX1   0x71
#define CH390_MRCMD     0x72
#define CH390_MRRL      0x74
#define CH390_MRRH      0x75
#define CH390_MWCMDX    0x76
#define CH390_MWCMD     0x78
#define CH390_MWRL      0x7A
#define CH390_MWRH      0x7B
#define CH390_TXPLL     0x7C
#define CH390_TXPLH     0x7D
#define CH390_ISR       0x7E
    #define ISR_LNKCHG      (1<<5)  // Link status change
    #define ISR_ROO         (1<<3)  // Receive overflow counter overflow
    #define ISR_ROS         (1<<2)  // Receive overflow
    #define ISR_PT          (1<<1)  // Packet transmitted
    #define ISR_PR          (1<<0)  // Packet received
#define CH390_IMR       0x7F
    #define IMR_NONE        0x00    // Disable all interrupt
    #define IMR_ALL         0xFF    // Enable all interrupt
    #define IMR_PAR         (1<<7)  // Pointer auto-return mode
    #define IMR_LNKCHGI     (1<<5)  // Enable link status change interrupt
    #define IMR_UDRUNI      (1<<4)  // Enable transmit under-run interrupt
    #define IMR_ROOI        (1<<3)  // Enable receive overflow counter overflow interrupt
    #define IMR_ROI         (1<<2)  // Enable receive overflow interrupt
    #define IMR_PTI         (1<<1)  // Enable packet transmitted interrupt
    #define IMR_PRI         (1<<0)  // Enable packet received interrupt

// SPI commands
#define OPC_REG_W       0x80  // Register Write
#define OPC_REG_R       0x00  // Register Read
#define OPC_MEM_DMY_R   0x70  // Memory Dummy Read
#define OPC_MEM_WRITE   0xF8  // Memory Write
#define OPC_MEM_READ    0x72  // Memory Read

// GPIO
#define CH390_GPIO1     0x02
#define CH390_GPIO2     0x04
#define CH390_GPIO3     0x08

#else

#define CH390_NCR       0x00
    #define NCR_WAKEEN      (1<<6) // Enable wakeup function
    #define NCR_FDX         (1<<3) // Duplex mode of the internal PHY
    #define NCR_LBK_MAC     (1<<1) // MAC loop-back
    #define NCR_RST         (1<<0) // Softwate reset
#define CH390_NSR       0x01
    #define NSR_SPEED       (1<<7) // Speed of internal PHY
    #define NSR_LINKST      (1<<6) // Link status of internal PHY
    #define NSR_WAKEST      (1<<5) // Wakeup event status
    #define NSR_TX2END      (1<<3) // Tx packet B complete status
    #define NSR_TX1END      (1<<2) // Tx packet A complete status
    #define NSR_RXOV        (1<<1) // Rx fifo overflow
#define CH390_TCR       0x02
    #define TCR_TJDIS       (1<<6) // Transmit jabber timer
    #define TCR_PAD_DIS2    (1<<4) // PAD appends for packet B
    #define TCR_CRC_DIS2    (1<<3) // CRC appends for packet B
    #define TCR_PAD_DIS1    (1<<2) // PAD appends for packet A
    #define TCR_CRC_DIS1    (1<<1) // CRC appends for packet A
    #define TCR_TXREQ       (1<<0) // Tx request
#define CH390_TSRA      0x03
#define CH390_TSRB      0x04
    #define TSR_TJTO        (1<<7) // Transmit jabber time out
    #define TSR_LC          (1<<6) // Loss of carrier
    #define TSR_NC          (1<<5) // No carrier
    #define TSR_LCOL        (1<<4) // Late collision
    #define TSR_COL         (1<<3) // Collision packet
    #define TSR_EC          (1<<2) // Excessive collision
#define CH390_RCR       0x05
    #define RCR_DEFAULT     0x00   // Default settings
    #define RCR_WTDIS       (1<<6) // Disable 2048 bytes watch dog
    #define RCR_DIS_CRC     (1<<4) // Discard CRC error packet
    #define RCR_ALL         (1<<3) // Pass all multicast
    #define RCR_RUNT        (1<<2) // Pass runt packet
    #define RCR_PRMSC       (1<<1) // Promiscuous mode
    #define RCR_RXEN        (1<<0) // Enable RX
#define CH390_RSR       0x06
    #define RSR_RF          (1<<7) // Rnt frame
    #define RSR_MF          (1<<6) // Multicast frame
    #define RSR_LCS         (1<<5) // Late collision seen
    #define RSR_RWTO        (1<<4) // Receive watchdog time-out
    #define RSR_PLE         (1<<3) // Physical layer error
    #define RSR_AE          (1<<2) // Alignment error
    #define RSR_CE          (1<<1) // CRC error
    #define RSR_FOE         (1<<0) // FIFO overflow error
#define CH390_ROCR      0x07
#define CH390_BPTR      0x08
#define CH390_FCTR      0x09
    #define FCTR_HWOT(ot)   (( ot & 0xf ) << 4)
    #define FCTR_LWOT(ot)   ( ot & 0xf )
#define CH390_FCR       0x0A
#define CH390_EPCR      0x0B
    #define EPCR_REEP       (1<<5) // Reload EEPROM
    #define EPCR_EPOS       (1<<3) // EEPROM or PHY operation select
    #define EPCR_ERPRR      (1<<2) // EEPROM or PHY read command
    #define EPCR_ERPRW      (1<<1) // EEPROM or PHY write command
    #define EPCR_ERRE       (1<<0) // EEPROM or PHY access status
#define CH390_EPAR      0x0C
#define CH390_EPDRL     0x0D
#define CH390_EPDRH     0x0E
#define CH390_WCR       0x0F
    #define WCR_LINKEN      (1<<5) // Link status change wakeup
    #define WCR_SAMPLEEN    (1<<4) // Sample frame wakeup
    #define WCR_MAGICEN     (1<<3) // Magic packet wakeup
    #define WCR_LINKST      (1<<2) // Link status change event
    #define WCR_SAMPLEST    (1<<1) // Sample frame event
    #define WCR_MAGICST     (1<<0) // Magic packet event
#define CH390_PAR       0x10
#define CH390_MAR       0x16
#define CH390_GPCR      0x1E
#define CH390_GPR       0x1F
#define CH390_TRPAL     0x22
#define CH390_TRPAH     0x23
#define CH390_RWPAL     0x24
#define CH390_RWPAH     0x25
#define CH390_VIDL      0x28
#define CH390_VIDH      0x29
#define CH390_PIDL      0x2A
#define CH390_PIDH      0x2B
#define CH390_CHIPR     0x2C
#define CH390_TCR2      0x2D
#define CH390_ETXCSR    0x30
#define CH390_TCSCR     0x31
    #define TCSCR_ALL         0x1F
    #define TCSCR_IPv6TCPCSE  (1<<4) // IPv6 TCP checksum generation
    #define TCSCR_IPv6UDPCSE  (1<<3) // IPv6 UDP checksum generation
    #define TCSCR_UDPCSE      (1<<2) // UDP checksum generation
    #define TCSCR_TCPCSE      (1<<1) // TCP checksum generation
    #define TCSCR_IPCSE       (1<<0) // IP checksum generation
#define CH390_RCSCSR    0x32
    #define RCSCSR_UDPS     (1<<7) // UDP checksum status
    #define RCSCSR_TCPS     (1<<6) // TCP checksum status
    #define RCSCSR_IPS      (1<<5) // IP checksum status
    #define RCSCSR_UDPP     (1<<4) // UDP packet of current received packet
    #define RCSCSR_TCPP     (1<<3) // TCP packet of current received packet
    #define RCSCSR_IPP      (1<<2) // IP packet of current received packet
    #define RCSCSR_RCSEN    (1<<1) // Receive checksum checking enable
    #define RCSCSR_DCSE     (1<<0) // Discard checksum error packet
#define CH390_MPAR      0x33
#define CH390_LEDCR     0x34
#define CH390_INTCR     0x39
    #define INCR_TYPE_OD    0x02
    #define INCR_TYPE_PP    0x00
    #define INCR_POL_L      0x01
    #define INCR_POL_H      0x00
#define CH390_SCCR      0x50
#define CH390_RSCCR     0x51
#define CH390_RLENCR    0x52
#define CH390_BCASTCR   0x53
#define CH390_MPTRCR    0x55
#define CH390_MRCMDX    0xF0
#define CH390_MRCMDX1   0xF1
#define CH390_MRCMD     0xF2
#define CH390_MRRL      0xF4
#define CH390_MRRH      0xF5
#define CH390_MWCMDX    0xF6
#define CH390_MWCMD     0xF8
#define CH390_MWRL      0xFA
#define CH390_MWRH      0xFB
#define CH390_TXPLL     0xFC
#define CH390_TXPLH     0xFD
#define CH390_ISR       0xFE
    #define ISR_IOMODE      (1<<7)  // Parallel interface mode
    #define ISR_LNKCHG      (1<<5)  // Link status change
    #define ISR_UDRUN       (1<<4)  // Transmit under-run
    #define ISR_ROO         (1<<3)  // Receive overflow counter overflow
    #define ISR_ROS         (1<<2)  // Receive overflow
    #define ISR_PT          (1<<1)  // Packet transmitted
    #define ISR_PR          (1<<0)  // Packet received
#define CH390_IMR       0xFF
    #define IMR_NONE        0x00    // Disable all interrupt
    #define IMR_ALL         0xFF    // Enable all interrupt
    #define IMR_PAR         (1<<7)  // Pointer auto-return mode
    #define IMR_LNKCHGI     (1<<5)  // Enable link status change interrupt
    #define IMR_UDRUNI      (1<<4)  // Enable transmit under-run interrupt
    #define IMR_ROOI        (1<<3)  // Enable receive overflow counter overflow interrupt
    #define IMR_ROI         (1<<2)  // Enable receive overflow interrupt
    #define IMR_PTI         (1<<1)  // Enable packet transmitted interrupt
    #define IMR_PRI         (1<<0)  // Enable packet received interrupt

// GPIO
#define CH390_GPIO1     0x02
#define CH390_GPIO2     0x04
#define CH390_GPIO3     0x08
#define CH390_GPIO4     0x10
#define CH390_GPIO5     0x20
#define CH390_GPIO6     0x40
#endif

// PHY registers
#define CH390_PHY          0x40
#define CH390_PHY_BMCR     0x00
#define CH390_PHY_BMSR     0x01
#define CH390_PHY_PHYID1   0x02
#define CH390_PHY_PHYID2   0x03
#define CH390_PHY_ANAR     0x04
#define CH390_PHY_ANLPAR   0x05
#define CH390_PHY_ANER     0x06
#define CH390_PHY_PAGE_SEL 0x1F

// Packet status
#define CH390_PKT_NONE  0x00    /* No packet received */
#define CH390_PKT_RDY   0x01    /* Packet ready to receive */
#define CH390_PKT_ERR   0xFE    /* Un-stable states */
#define CH390_PKT_MAX   1536    /* Received packet max size */
#define CH390_PKT_MIN   64

/********************************************************************
 * Functions
 */

/**
 * @name ch390_receive_packet
 * @brief Receive packet
 * @param buff - Size equal to CH390_PKT_MAX
 * @param rx_status - Output abnormal status while receiving packet.
 *                    It has the same format as RSR(06h).
 * @return Packet length
 */
uint32_t ch390_receive_packet(uint8_t *buff, uint8_t *rx_status);

/**
 * @name ch390_send_packet
 * @brief Send packet
 * @param buff - Data to be sent
 * @param length - Less than 3k bytes.
 */
void ch390_send_packet(uint8_t *buff, uint16_t length);

/**
 * @name ch390_send_request
 * @brief Issue transmit request
 */
void ch390_send_request(void);

/**
 * @name ch390_drop_packet
 * @brief Drop packet in RX SRAM if don't want to read it. This function
 *        modify the memory data read pointer and skip specified length
 * @param len - Skip length, length of the current packet.
 */
void ch390_drop_packet(uint16_t len);

/**
 * @name ch390_read_phy
 * @brief Read PHY register
 * @param reg - PHY register address
 */
uint16_t ch390_read_phy(uint8_t reg);

/**
 * @name ch390_write_phy
 * @brief Write PHY register
 * @param reg - PHY register address
 * @param value - Value to be written
 */
void ch390_write_phy(uint8_t reg, uint16_t value);

/**
 * @name ch390_write_eeprom
 * @brief Write EEPROM register
 * @param reg - EEPROM register address
 * @param value - Value to be written
 */
void ch390_write_eeprom(uint8_t reg, uint16_t value);

/**
 * @name ch390_software_reset
 * @brief Software reset CH390 by NCR
 */
void ch390_software_reset(void);

/**
 * @name ch390_default_config
 * @brief Config CH390 with default options:
 *        LED mode 1;
 *        Enable transmit check sum generation;
 *        Enable RX;
 *        Enable all interrupt and PAR
 */
void ch390_default_config(void);

/**
 * @name ch390_set_phy_mode
 * @brief Set PHY mode and enable PHY.
 *        PHY mode: Auto-negotiation, 10M/100M, full-duplex/half-duplex
 * @param mode - PHY mode
 */
void ch390_set_phy_mode(enum ch390_phy_mode mode);

/**
 * @name ch390_set_mac_address
 * @brief Set mac address
 * @param mac_addr - 6-byte length mac address array
 */
void ch390_set_mac_address(uint8_t *mac_addr);

/**
 * @name ch390_set_multicast
 * @brief Set multicast address hash table
 * @param multicast_addr - 8-byte length multicast address hash table array
 */
void ch390_set_multicast(uint8_t *multicast_hash);

/**
 * @brief Set MAR bit for a particular MAC address
 * @param mac - Destination address
 */
void ch390_set_hash_bit(uint8_t *mac);

/**
 * @name ch390_get_mac
 * @brief Get mac address
 * @param mac_addr - 6 bytes mac address output
 */
void ch390_get_mac(uint8_t *mac_addr);

/**
 * @name ch390_get_multicast
 * @brief Get multicast address hash table
 * @param multicast_addr - 8-byte length multicast address hash table output
 */
void ch390_get_multicast(uint8_t *multicast_addr);

/**
 * @name ch390_get_vendor_id
 * @brief Get vendor ID
 * @return Vendor ID
 */
uint16_t ch390_get_vendor_id(void);

/**
 * @name ch390_get_product_id
 * @brief Get product ID
 * @return Product ID
 */
uint16_t ch390_get_product_id(void);

/**
 * @name ch390_get_revision
 * @brief Get chip revision
 * @return Chip revision
 */
uint8_t ch390_get_revision(void);

/**
 * @name ch390_interrupt_config
 * @brief Interrupt configuration
 * @param mask - Interrupt to be enabled, see "CH390.h" IMR_xxx
 */
void ch390_interrupt_config(uint8_t mask);

/**
 * @name ch390_rx_enable
 * @brief Enable or disable packet receive
 * @param op - 0: disable  1: enable
 */
void ch390_rx_enable(int op);

/**
 * @name ch390_rx_filter_config
 * @brief Configure receive filter.
 * @param config - See "CH390.h" RCR_xxx
 */
void ch390_rx_filter_config(uint8_t config);

/**
 * @name ch390_wakeup_config
 * @brief Enable or disable wakeup_function
 * @param events - Events that trigger wakeup,
 *                 WCR_LINKEN   - Link status change
 *                 WCR_SAMPLEEN - Sample frame
 *                 WCR_MAGICEN  - Magic packet
 *                 0            - Disable wakeup function
 */
void ch390_wakeup_config(uint8_t events);

/**
 * @name ch390_wake_notify
 * @brief Wait for Magic Packet or Sample Frame and discard all
 *        other packets.
 *        If the application needs to use Wake On LAN, call this
 *        function before MCU enters low power mode. An external
 *        interrupt signal is accessible on WOL pin when wake
 *        up event occurred.
 */
void ch390_wake_notify(void);

/**
 * @name ch390_loop_back_enable
 * @brief Enable loop back mode
 * @param op - 0: disable  1: enable
 */
void ch390_loop_back_enable(int op);

/**
 * @name ch390_get_duplex_mode
 * @brief Get current duplex mode of the internal PHY
 * @return 0: Half-duplex  1: Full-duplex
 */
int ch390_get_duplex_mode(void);

/**
 * @name ch390_get_phy_speed
 * @brief Get the speed of the internal PHY.
 *        Only valid after PHY linked
 * @return 0: 100Mbps  1: 10Mbps
 */
int ch390_get_phy_speed(void);

/**
 * @name ch390_get_link_status
 * @brief Get link status of the internal PHY
 * @return 0: Link failed  1: Link OK
 */
int ch390_get_link_status(void);

/**
 * @name ch390_sleep_control
 * @brief Enter or exit sleep mode
 * @param op - 0: Power up  1: Power down
 */
void ch390_sleep_control(int op);

#ifndef CH390_INTERFACE_16_BIT
/**
 * @name ch390_gpio_config
 * @brief Config the input/output direction of GPIO1~3
 *        Only GPIO1~3 can be defined as input, GPIO4~6 are output only
 * @param GPIOx - CH390_GPIO1 ~ CH390_GPIO3
 *        dir - 0: Input  1: Output
 */
void ch390_gpio_config(uint8_t GPIOx, uint8_t dir);

/**
 * @name ch390_gpio_write_bit
 * @brief Sets or clears the selected gpio bit.
 * @param GPIOx - CH390_GPIO1 ~ CH390_GPIO6
 *        level - 0: Clear pin  1: Set pin
 */
void ch390_gpio_write_bit(uint8_t GPIOx, uint8_t level);

/**
 * @name ch390_gpio_read_bit
 * @brief Read gpio input, only CH390_GPIO1 ~ 3 are available
 * @param GPIOx - CH390_GPIO1 ~ CH390_GPIO3
 * @return Input pin value
 */
uint8_t ch390_gpio_read_bit(uint8_t GPIOx);
#endif

/**
 * @name ch390_int_pin_config
 * @brief Configure INT pin output type and polarity
 * @param type - INCR_TYPE_OD: Open drain output
 *               INCR_TYPE_PP: Push pull output
 *        pol - INCR_POL_L: Active low
 *              INCR_POL_H: Active high
 */
void ch390_int_pin_config(uint8_t type, uint8_t pol);

/**
 * @name ch390_get_int_status
 * @brief Get CH390 interrupt status and clear them
 * @return Interrupt status
 */
uint8_t ch390_get_int_status(void);

bool CH390Init(void);
void ch390_ut_send_test(void);
void CH390RestartEnable(void);
uint8_t CH390RestartProcess(void);
#endif /* __CH390_H */
```

### `device/CH390_Interface.c`

```c
/********************************** (C) COPYRIGHT *****************************
 * File Name          : CH390_Interface.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/08/20
 * Description        : CH390 interface
 ******************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 ******************************************************************************/
#include "CH390.h"
#include "CH390_Interface.h"

#define ch390_cs(value)  GPIO_WritePin(66, value);
#define ch390_rst(value) GPIO_WritePin(67, value);

/**
 * @name ch390_get_int_pin
 * @brief Get ch390 interrupt signal
 */
uint16_t ch390_get_int_pin(void)
{
//    return GPIO_ReadPin(25);
    return GPIO_ReadPin(68);
}

/**
 * @name ch390_delay_us
 * @brief Delay a few microseconds
 */
void ch390_delay_us(uint32_t time)
{
    DELAY_US(time);
}

/**
 * @name ch390_hardware_reset
 * @brief Hardware reset CH390 by pull down RSTB pin
 */
void ch390_hardware_reset()
{
    ch390_rst(0);
    ch390_delay_us(10);
    ch390_rst(1);
    ch390_delay_us(10000);
}

static uint8_t ch390_spi_exchange_byte(uint8_t byte)
{
    uint16_t Readtemp;
    while(SpibRegs.SPIFFTX.bit.TXFFST != 0)
    {

    }
    SpibRegs.SPITXBUF = (uint8_t)(byte << 8);

    while(SpibRegs.SPISTS.bit.INT_FLAG == 0)
    {

    }
    Readtemp = SpibRegs.SPIRXBUF;

    return  (Readtemp);
}

#define ch390_spi_dummy_read() ch390_spi_exchange_byte(0)

/***************************************************************************
 * The following are the basic CH390 read/write operation functions.
 */

/**
 * @name ch390_read_reg
 * @brief Read register
 * @param reg - Target register address
 * @return Register value
 */
uint8_t ch390_read_reg(uint8_t reg)
{
    uint8_t spi_data;
    ch390_cs(0); // CS = 0
    ch390_spi_exchange_byte(reg | OPC_REG_R);
    spi_data = ch390_spi_dummy_read();
    ch390_cs(1); // CS = 1
    return spi_data;
}

/**
 * @name ch390_write_reg
 * @brief Write register
 * @param reg - Target register address
 * @param value - Value to be written
 */
void ch390_write_reg(uint8_t reg, uint8_t value)
{
    ch390_cs(0); // CS = 0
    ch390_spi_exchange_byte(reg | OPC_REG_W);
    ch390_spi_exchange_byte(value);
    ch390_cs(1); // CS = 1
    return;
}
/**
 * @name ch390_read_mem
 * @brief Read data from RX SRAM
 * @param data - Data buffer
 * @param length - Length to read
 */
void ch390_read_mem(uint8_t *data, int length)
{
    int i;
    ch390_cs(0);
    ch390_spi_exchange_byte(OPC_MEM_READ);

    for (i = 0; i < length; i++)
    {
        data[i] = ch390_spi_dummy_read();
    }
    ch390_cs(1);
}
/**
 * @name ch390_write_mem
 * @brief Write data to TX SRAM
 * @param data - Data buffer
 * @param length - Length to write
 */
void ch390_write_mem(uint8_t *data, int length)
{
    int i;
    ch390_cs(0);
    ch390_spi_exchange_byte(OPC_MEM_WRITE);
    for (i = 0; i < length; i++)
    {
        ch390_spi_exchange_byte(data[i]);
    }
    ch390_cs(1);
}
```

### `device/CH390_Interface.h`

```c
/********************************** (C) COPYRIGHT *******************************
* File Name          : CH390.h
* Author             : WCH
* Version            : V1.0
* Date               : 2024/08/20
* Description        : CH390 interface header file
*******************************************************************************/

#ifndef __CH390_INTERFACE_H
#define __CH390_INTERFACE_H

#include <stdint.h>
#include "ch390.h"
#include "F28x_Project.h"

uint16_t ch390_get_int_pin(void);
void ch390_delay_us(uint32_t time);
void ch390_hardware_reset(void);

/**
 * @name ch390_read_reg
 * @brief Read register
 * @param reg - Target register address
 * @return Register value
 */
uint8_t ch390_read_reg(uint8_t reg);

/**
 * @name ch390_write_reg
 * @brief Write register
 * @param reg - Target register address
 * @param value - Value to be written
 */
void ch390_write_reg(uint8_t reg, uint8_t value);

/**
 * @name ch390_read_mem
 * @brief Read data from RX SRAM
 * @param data - Data buffer
 * @param length - Length to read
 */
void ch390_read_mem(uint8_t *data, int length);

/**
 * @name ch390_write_mem
 * @brief Write data to TX SRAM
 * @param data - Data buffer
 * @param length - Length to write
 */
void ch390_write_mem(uint8_t *data, int length);

#endif /* __CH390_INTERFACE_H */
```

### `device/ct.c`

```c
/*
 * ct.c
 *
 *  Created on: 2025-11-17
 *      Author: Nova006348
 */

#include "ct.h"
#include "string.h"
#include <math.h>
#include <sample_update.h>
#include "ph3_calc_ifc_update.h"

 CtDataInfo_t  gCtDataInfo;
int16  _gCtPolarity[3];                        /* Default CT polarity aligned with current direction */
CtSlideWindow_t _gCtDataProcess[3];
bool ct_break_result[3] = {false,false,false};
/**
* @brief: CtParamInit()
* @param: nothing
* @return:nothing
* @note: CT data initialization
*/
static void _gCtParamInit(void)
{
    memset(&gCtDataInfo, 0, sizeof(gCtDataInfo));
    memset(&_gCtDataProcess, 0,  sizeof(_gCtDataProcess));
    _gCtPolarity[0] = 1;
    _gCtPolarity[1] = 1;
    _gCtPolarity[2] = 1;
}
/**
* @brief: CtAutoCorrentPower()
* @param: nothing
* @return:nothing
* @note: CT auto polarity detection; not suitable when remote load or parallel units are present
*/
void CtAutoCorrentPower(void)
{
    if(gstPh3CalcInfo.grid_power.a_phase_active * gstPh3CalcInfo.ct_power.a_phase_active < 0)
    {
        _gCtPolarity[0] = -1;
    }
    else
    {
        _gCtPolarity[0] = 1;
    }

    if(gstPh3CalcInfo.grid_power.b_phase_active * gstPh3CalcInfo.ct_power.b_phase_active < 0)
    {
        _gCtPolarity[1] = -1;
    }
    else
    {
        _gCtPolarity[1] = 1;
    }

    if(gstPh3CalcInfo.grid_power.c_phase_active * gstPh3CalcInfo.ct_power.c_phase_active < 0)
    {
        _gCtPolarity[2] = -1;
    }
    else
    {
        _gCtPolarity[2] = 1;
    }
}
/**
* @brief: CtReversePower()
* @param: nothing
* @return:nothing
* @note: CT polarity inversion
*/
void CtReversePower(int ct_inverse)
{
    static int en_inverse = 1;
    static bool first_run_flag = true;
    if(first_run_flag)
    {
        first_run_flag = false;
        _gCtParamInit();
    }
    if(ct_inverse)
    {
        en_inverse = -1;
    }
    else
    {
        en_inverse = 1;
    }
    gCtDataInfo.ct_power.a_phase_active = gstPh3CalcInfo.ct_power.a_phase_active * _gCtPolarity[0] * en_inverse;
    gCtDataInfo.ct_power.b_phase_active = gstPh3CalcInfo.ct_power.b_phase_active * _gCtPolarity[1] * en_inverse;
    gCtDataInfo.ct_power.c_phase_active = gstPh3CalcInfo.ct_power.c_phase_active * _gCtPolarity[2] * en_inverse;
    gCtDataInfo.ct_power.total_active = gCtDataInfo.ct_power.a_phase_active  + gCtDataInfo.ct_power.b_phase_active + gCtDataInfo.ct_power.c_phase_active;

    gCtDataInfo.ct_power.a_phase_reactive = gstPh3CalcInfo.ct_power.a_phase_reactive * _gCtPolarity[0] * en_inverse;
    gCtDataInfo.ct_power.b_phase_reactive = gstPh3CalcInfo.ct_power.b_phase_reactive * _gCtPolarity[1] * en_inverse;
    gCtDataInfo.ct_power.c_phase_reactive = gstPh3CalcInfo.ct_power.c_phase_reactive * _gCtPolarity[2] * en_inverse;
    gCtDataInfo.ct_power.total_reactive = gCtDataInfo.ct_power.a_phase_reactive  + gCtDataInfo.ct_power.b_phase_reactive + gCtDataInfo.ct_power.c_phase_reactive;

    gCtDataInfo.ct_power.a_phase_apperant = gstPh3CalcInfo.ct_power.a_phase_apperant ;
    gCtDataInfo.ct_power.b_phase_apperant = gstPh3CalcInfo.ct_power.b_phase_apperant ;
    gCtDataInfo.ct_power.c_phase_apperant = gstPh3CalcInfo.ct_power.c_phase_apperant ;

    gCtDataInfo.ct_power.total_apperant = gstPh3CalcInfo.ct_power.total_apperant;
}

/**
* @brief: _gAddWindowData()
* @param: nothing
* @return:nothing
* @note: Add sample to sliding window
*/
void _gAddWindowData(CtSlideWindow_t* window, float value)
{
    /* Remove oldest sample when window is full */
    if (window->count == WINDOW_SIZE)
    {
        float old_value = window->data[window->index];
        window->sum -= old_value;
        window->square_sum -= old_value * old_value;
    }
    else
    {
        window->count++;
    }
    /* Add new sample */
    window->data[window->index] = value;
    window->sum += value;
    window->square_sum += value * value;
    /* Advance write index */
    window->index = (window->index + 1) % WINDOW_SIZE;
}
/**
* @brief: _gCalculateVariance()
* @param: nothing
* @return:nothing
* @note: Calculate variance over current window
*/
float _gCalculateVariance(CtSlideWindow_t* window)
{
    if (window->count < 2)
    {
        return 0.0;  /* At least 2 samples required */
    }

    float mean = window->sum / window->count;
    float variance = (window->square_sum / window->count) - (mean * mean);

    /* Clamp small negative values from floating-point error */
    return variance > 0 ? variance : 0.0;
}
/**
* @brief: _gCalculateStdDev()
* @param: nothing
* @return:nothing
* @note: Calculate standard deviation over current window
*/
float _gCalculateStdDev(CtSlideWindow_t* window)     /* 0.5 used as threshold */
{
    return sqrt(_gCalculateVariance(window));
}
/**
* @brief: CtbreakJudge()
* @param: nothing
* @return:nothing
* @note: Detect CT wire-break fault
*/
bool* CtbreakJudge(void)
{
    static uint16_t cont = 0;
    float ct_std_dev[3] ;
    if(cont == 0)
    {
        _gAddWindowData(&_gCtDataProcess[0],gstAdcResult.ct_a_curr);
        ct_std_dev[0] = _gCalculateStdDev(&_gCtDataProcess[0]);

        if(ct_std_dev[0] < CT_BREAK_THRESHOLD)
        {
            ct_break_result[0] = true;
        }
        else
        {
            ct_break_result[0] = false;
        }
    }
    else if(cont == 1)
    {
        _gAddWindowData(&_gCtDataProcess[1],gstAdcResult.ct_b_curr);
        ct_std_dev[1] = _gCalculateStdDev(&_gCtDataProcess[1]);

        if(ct_std_dev[1] < CT_BREAK_THRESHOLD)
        {
            ct_break_result[1] = true;
        }
        else
        {
            ct_break_result[1] = false;
        }
    }
    else
    {
        _gAddWindowData(&_gCtDataProcess[2],gstAdcResult.ct_c_curr);
        ct_std_dev[2] = _gCalculateStdDev(&_gCtDataProcess[2]);

        if(ct_std_dev[2] < CT_BREAK_THRESHOLD)
        {
            ct_break_result[2] = true;
        }
        else
        {
            ct_break_result[2] = false;
        }
    }
    cont ++ ;
    if(cont >=3)
    {
        cont = 0;
    }

    return ct_break_result;
}
```

### `device/ct.h`

```c
/*
 * ct.h
 *
 *  Created on: 2025-11-17
 *      Author: Nova006348
 */

#ifndef APP_CT_H_
#define APP_CT_H_

#include "F28x_Project.h"


#define CT_BREAK_THRESHOLD   (0.8f)

typedef struct _ct_out_info_stru_t
{
    float   a_phase_active;
    float   b_phase_active;
    float   c_phase_active;
    float   total_active;

    float   a_phase_reactive;
    float   b_phase_reactive;
    float   c_phase_reactive;
    float   total_reactive;

    float   a_phase_apperant;
    float   b_phase_apperant;
    float   c_phase_apperant;
    float   total_apperant;

} CtOutInfo_t;

typedef struct _ct_info_stru_t
{
    CtOutInfo_t   ct_power;
} CtDataInfo_t;

#define WINDOW_SIZE 30

typedef struct {
    float data[WINDOW_SIZE];  /* Window sample buffer */
    int index;                /* Current write index */
    int count;                /* Number of samples in window */
    float sum;                /* Sum of window samples */
    float square_sum;         /* Sum of squared window samples */
} CtSlideWindow_t;

void CtAutoCorrentPower(void);   /* CT auto polarity correction; use only without remote load or parallel units */
void CtReversePower(int ct_inverse);   /* CT polarity inversion; required for all CT active/reactive power outputs */
bool* CtbreakJudge(void);   /* CT wire-break detection */

extern CtDataInfo_t  gCtDataInfo;

#endif /* APP_CT_H_ */
```

### `device/dev_flash.c`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_flash.c
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
  * </table>    32M bit flash 0~ 0x3F FFFF
  ******************************************************************************
  */


//
// Included Files
//
#include "dev_flash.h"

SpiFlash_HandleType SpiFlash;

/**
* @brief: _SpiFlashRead_Version
* @param: pSpiFlash
* @return: ��ȡFlash�е�ǰ�汾
* @note:
*/
static void _SpiFlashRead_Version(void)
{
    SpiFlashReadBytes(0x00001004,SpiFlash.updata.pcs_firmware_version,3);
}
/**
* @brief: _SpiFlashGetInfo
* @param: pSpiFlash
* @return:
* @note:
*
*/
static bool _SpiFlashGetInfo(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    SF_CS_LOW();

    c_txdata = FLASH_JEDEC_ID;
    SpiWriteData(1, &c_txdata);
    SpiReadData(3, SpiFlash.arr_id);
    SF_CS_HIGH();

    if(SpiFlash.arr_id[0])
    {
        if(0x14 == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 1;
        }
        else if(0x15 == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 2;
        }
        else if(0x16 == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 4;
        }
        else if(0x17 == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 8;
        }
        else if(0x18 == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 16;
        }
        else if(0x19 == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 32;
        }
        else if(0x20 == SpiFlash.arr_id[2] || 0x1A == SpiFlash.arr_id[2] || 0x3A == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 64;
        }
        else
        {
            SpiFlash.n_capacity_MB = 8;
        }
    }
    else
    {
        SpiFlash.n_capacity_MB = 0;
    }
    return n_ret;
}

/**
* @brief: _SpiFlashWrite32bitAddr
* @param: n_addr
* @return:
* @note:
*
*/
static int32_t _SpiFlashWrite32bitAddr (uint32_t n_addr)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    c_txdata = n_addr >> 24;
    SpiWriteData(1, &c_txdata);
    c_txdata = n_addr >> 16;
    SpiWriteData(1, &c_txdata);
    c_txdata = n_addr >> 8;
    SpiWriteData(1, &c_txdata);
    c_txdata = n_addr;
    SpiWriteData(1, &c_txdata);

    return n_ret;
}

/**
* @brief: _SpiFlashWrite24bitAddr
* @param: n_addr
* @return:
* @note:
*
*/
static int32_t _SpiFlashWrite24bitAddr (uint32_t n_addr)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    c_txdata = n_addr >> 16;
    SpiWriteData(1, &c_txdata);
    c_txdata = n_addr >> 8;
    SpiWriteData(1, &c_txdata);
    c_txdata = n_addr;
    SpiWriteData(1, &c_txdata);

    return n_ret;
}

/**
* @brief: SpiFlashReadBytes
* @param: start_addr
* @param: idata
* @param: length
* @return:
* @note:
*
*/
int32_t SpiFlashReadBytes(uint32_t start_addr, uint16_t *idata, uint16_t length)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;
    SpiFlash.n_start_addr = start_addr;

    SF_CS_LOW();
    if(SpiFlash.n_capacity_MB >= 32)
    {
        c_txdata = FLASH_LOW_READ_4BYTE;
        SpiWriteData(1, &c_txdata);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        c_txdata = FLASH_LOW_READ;
        SpiWriteData(1, &c_txdata);
        _SpiFlashWrite24bitAddr(start_addr);
    }
    SpiReadData(length, idata);
    SF_CS_HIGH();

    return n_ret;
}

/**
* @brief: _SpiFlashWriteEnable
* @param: pSpiFlash
* @return:
* @note:
*
*/
static int32_t _SpiFlashWriteEnable(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    SF_CS_LOW();
    c_txdata = FLASH_WRITE_ENABLE;
    SpiWriteData(1, &c_txdata);
    SF_CS_HIGH();

    return n_ret;
}

/**
* @brief: _SpiFlashReadStatusReg
* @param: pSpiFlash
* @return:
* @note:
*
*/
static int32_t _SpiFlashReadStatusReg(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;
    uint32_t  time_out = 0xFFFF;
    uint16_t c_txdata = 0;
    uint16_t c_rxdata = 0;

    SF_CS_LOW();
    c_txdata = FLASH_READ_STATUS_REG;
    SpiWriteData(1, &c_txdata);
//    c_txdata = DUMMY_BYTE & 0x01;

    do
    {
        SpiReadData(1, &c_rxdata);
        time_out--;
    }
    while((c_rxdata & 0x01) && (time_out != 0));

    SF_CS_HIGH();

    return n_ret;
}

/**
* @brief: SpiFlashAutoEraseSector
* @param: start_addr
* @param: length
* @return:
* @note:
*
*/
int32_t SpiFlashAutoEraseSector(uint32_t start_addr, uint16_t length)
{
    int32_t n_ret = 0;
    int32_t n_remain = 0;
    uint16_t c_txdata = 0;

    n_remain = start_addr % FLASH_ERASE_SECTOR_SIZE;

    if((n_remain == 0) ||\
            (n_remain + length > FLASH_ERASE_SECTOR_SIZE))
    {
        if(n_remain != 0)
        {
            start_addr += FLASH_ERASE_SECTOR_SIZE;
        }

        _SpiFlashWriteEnable(&SpiFlash);
        SF_CS_LOW();
        if(SpiFlash.n_capacity_MB >= 32)
        {
            c_txdata = FLASH_SECTOR_ERASE_4BYTE;
            SpiWriteData(1, &c_txdata);
            _SpiFlashWrite32bitAddr(start_addr);
        }
        else
        {
            c_txdata = FLASH_SECTOR_ERASE;
            SpiWriteData(1, &c_txdata);
            _SpiFlashWrite24bitAddr(start_addr);
        }
        SF_CS_HIGH();
        _SpiFlashReadStatusReg(&SpiFlash);
    }
    return n_ret;
}

/**
* @brief: SpiFlashEraseBlock32k
* @param: start_addr
* @return:
* @note:
*
*/
int32_t SpiFlashEraseBlock32k(uint32_t start_addr)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    _SpiFlashWriteEnable(&SpiFlash);
    SF_CS_LOW();
    c_txdata = FLASH_BLOCK_ERASE_32K;
    SpiWriteData(1, &c_txdata);
    _SpiFlashWrite24bitAddr(start_addr);
    SF_CS_HIGH();
    _SpiFlashReadStatusReg(&SpiFlash);

    return n_ret;
}

/**
* @brief: SpiFlashEraseBlock64k
* @param: start_addr
* @return:
* @note:
*
*/
int32_t SpiFlashEraseBlock64k(uint32_t start_addr)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    _SpiFlashWriteEnable(&SpiFlash);
    SF_CS_LOW();
    c_txdata = FLASH_BLOCK_ERASE_64K;
    SpiWriteData(1, &c_txdata);
    _SpiFlashWrite24bitAddr(start_addr);
    SF_CS_HIGH();
    _SpiFlashReadStatusReg(&SpiFlash);

    return n_ret;
}

/**
* @brief: SpiFlashEraseChip
* @param: void
* @return:
* @note:
*
*/
int32_t SpiFlashEraseChip(void)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    _SpiFlashWriteEnable(&SpiFlash);
    SF_CS_LOW();
    c_txdata = FLASH_CHIP_ERASE;
    SpiWriteData(1, &c_txdata);
    SF_CS_HIGH();
    _SpiFlashReadStatusReg(&SpiFlash);

    return n_ret;
}

/**
* @brief: SpiFlashPageProgram
* @param: start_addr
* @param: idata
* @param: length
* @return:
* @note:
*
*/
static int32_t SpiFlashPageProgram(uint32_t start_addr, uint16_t *idata, uint16_t length)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    _SpiFlashWriteEnable(&SpiFlash);
    SF_CS_LOW();
    if(SpiFlash.n_capacity_MB >= 32)
    {
        c_txdata = FLASH_PAGE_PROGRAM_4BYTE;
        SpiWriteData(1, &c_txdata);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        c_txdata = FLASH_PAGE_PROGRAM;
        SpiWriteData(1, &c_txdata);
        _SpiFlashWrite24bitAddr(start_addr);
    }
    SpiWriteData(length, idata);
    SF_CS_HIGH();
    _SpiFlashReadStatusReg(&SpiFlash);

    return n_ret;
}

/**
* @brief: SpiFflashWriteBytesWithoutErase
* @param: start_addr
* @param: idata
* @param: length
* @return:
* @note:
*
*/
int32_t SpiFflashWriteBytesWithoutErase(uint32_t start_addr, uint16_t *idata, uint16_t length)
{
    uint16_t num_of_page = 0;
    uint16_t num_of_single = 0;
    int32_t n_ret = 0;

    num_of_page =  length / FLASH_PAGE_SIZE;
    num_of_single = length % FLASH_PAGE_SIZE;

    while(num_of_page--)
    {
        SpiFlashPageProgram(start_addr, idata, FLASH_PAGE_SIZE);
        start_addr +=  FLASH_PAGE_SIZE;
        idata += FLASH_PAGE_SIZE;
    }
    if(num_of_single != 0)
    {
        SpiFlashPageProgram(start_addr, idata, num_of_single);
    }
    return n_ret;
}

/**
* @brief: SpiFlashWriteBytesWithoutEraseNoAligned
* @param: start_addr
* @param: idata
* @param: length
* @return:
* @note:
*
*/
int32_t SpiFlashWriteBytesWithoutEraseNoAligned(uint32_t start_addr, uint16_t *idata, uint16_t length)
{
    uint16_t num_of_start = 0, num_of_end = 0;
    uint16_t num_of_mid = 0;
    int32_t n_ret = 0;
    uint8_t mid_page_num;
    uint16_t page_num;

    if(length == 0)
    {
        return n_ret;
    }
    page_num = (start_addr % FLASH_PAGE_SIZE + length + (FLASH_PAGE_SIZE - 1)) / FLASH_PAGE_SIZE;

    if(page_num == 1)
    {
        num_of_start = length;
        num_of_end = 0;
        num_of_mid = 0;
        mid_page_num = 0;
    }
    else
    {
        num_of_start = FLASH_PAGE_SIZE - start_addr % FLASH_PAGE_SIZE;
        num_of_end = (start_addr % FLASH_PAGE_SIZE + length) % FLASH_PAGE_SIZE;
        num_of_mid = length - num_of_start - num_of_end;
        mid_page_num = num_of_mid / FLASH_PAGE_SIZE;
    }

    /*��һ��ҳ*/
    if(num_of_start > 0)
    {
        SpiFlashPageProgram(start_addr, idata, num_of_start);
        start_addr +=  num_of_start;
        idata += num_of_start;
    }

    /*�м��ҳ*/
    while(mid_page_num--)
    {
        SpiFlashPageProgram(start_addr, idata, FLASH_PAGE_SIZE);
        start_addr +=  FLASH_PAGE_SIZE;
        idata += FLASH_PAGE_SIZE;
    }
    /*���һ��ҳ*/
    if(num_of_end > 0)
    {
        SpiFlashPageProgram(start_addr, idata, num_of_end);
    }

    return n_ret;
}

/**
* @brief: SpiFlashWriteBytesWithErase
* @param: start_addr
* @param: idata
* @param: length
* @return:
* @note:
*
*/
int32_t SpiFlashWriteBytesWithErase(uint32_t start_addr, uint16_t *idata, uint16_t length)
{
    int32_t n_ret = 0;

    n_ret = SpiFlashAutoEraseSector(start_addr, length);

    /* ��Ҫд������д��flash�� */
    n_ret = SpiFlashWriteBytesWithoutEraseNoAligned(start_addr, idata, length);

    return n_ret;
}

/**
* @brief: FlashInit
* @param: void
* @note: Flash��ʼ������
*/
void FlashInit(void)
{
    _SpiFlashGetInfo(&SpiFlash);
    _SpiFlashRead_Version();
}
//
// End of file
//
```

### `device/dev_flash.h`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_flash.h
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


#ifndef __DEV_FLASH_H__
#define __DEV_FLASH_H__

#include "spi.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif
#define CS_SPI_FLASH_GPIO    61
/* Ƭѡ�����õ�ѡ��  */
#define SF_CS_LOW()		GPIO_WritePin(CS_SPI_FLASH_GPIO, 0);
/* Ƭѡ�����ø߲�ѡ�� */
#define SF_CS_HIGH()	GPIO_WritePin(CS_SPI_FLASH_GPIO, 1);
/* spi flash control cmd */
#define  FLASH_LOW_READ                 0x03
#define  FLASH_LOW_READ_4BYTE           0x13

#define  FLASH_HIGH_READ                0x0B
#define  FLASH_HIGH_READ_4BYTE          0x0C

#define  FLASH_SECTOR_ERASE             0x20
#define  FLASH_SECTOR_ERASE_4BYTE       0x21

#define  FLASH_BLOCK_ERASE_32K          0x52
#define  FLASH_CHIP_ERASE               0xC7

#define  FLASH_BLOCK_ERASE_64K          0xD8
#define  FLASH_BLOCK_ERASE_64K_4BYTE    0xDC

#define  FLASH_PAGE_PROGRAM             0x02
#define  FLASH_PAGE_PROGRAM_4BYTE       0x12

#define  FLASH_READ_STATUS_REG          0x05
#define  FLASH_WRITE_STATUS_REG         0x01
#define  FLASH_WRITE_STATUS_REG2        0x31
#define  FLASH_WRITE_ENABLE             0x06
#define  FLASH_WRITE_DISABLE            0x04
#define  FLASH_POWER_DOWN               0xB9
#define  FLASH_RELEASE_PD               0xAB
#define  FLASH_JEDEC_ID                 0x9F
#define  DUMMY_BYTE                     0xFF

#define  FLASH_PROJECT                  0xBC    //0xFC
#define  FLASH_UNPROJECT                0x00    //0x40

#define  FLASH_PAGE_SIZE            (256)
#define  FLASH_ERASE_SECTOR_SIZE    (0x1000)
#define  FLASH_ERASE_BLOCK_SIZE     (0x10000)
#define  FLASH_TEST_ADDR            (0x3FF000)

typedef struct spi_flash_updata_stru_t
{
    uint16_t        pcs_firmware_type[4];       //�����̼�����
    uint16_t        pcs_firmware_version[3];    //�����İ汾��
    uint32_t        pcs_firmware_len;           //�̼�����
    uint32_t        pcs_firmware_len_temp;      //�̼�������ʱֵ
    uint32_t        checksum;                   //32λУ��Ӻ�
    uint32_t        checksumtemp;               //32λУ��Ӻ���ʱֵ
    uint16_t        check_succeed;              //У��ɹ�
    uint16_t        finish;                     //���������ɱ�־λ
} SpiFlash_UpData_t;

typedef struct _spi_flash_handle_stru_t
{
    uint16_t             arr_id[4];
    uint16_t            n_capacity_MB;  /*�ױ���*/
    uint8_t             c_protect_state;

    uint32_t            n_start_addr;
    uint16_t            n_length;
    uint8_t             *pdata;

    SpiFlash_UpData_t   updata; //������ر�־λ
}SpiFlash_HandleType;

void SetFlash_Updata_Type(uint16_t num,uint16_t value);
void SetFlash_Updata_Version(uint16_t num,uint16_t value);
void SetFlash_Updata_Length(uint32_t value);
void SetFlash_Updata_CheckSum(uint32_t value);
void SetFlash_Updata_CheckSumTemp(uint32_t value);
uint32_t GetFlash_Updata_Length(void);
uint32_t GetFlash_Updata_CheckSumTemp(void);
uint32_t GetFlash_Updata_Len_Temp(void);
uint16_t GetFlash_Soft_Version_X(void);
uint16_t GetFlash_Soft_Version_Y(void);
uint16_t GetFlash_Soft_Version_Z(void);
void SetFlash_Updata_pcs_Len_Temp(uint32_t value);
void SetFlash_Updata_finish(uint16_t value);

int32_t SpiFlashReadBytes(uint32_t start_addr, uint16_t *idata, uint16_t length);
int32_t SpiFlashEraseChip(void);
int32_t SpiFflashWriteBytesWithoutErase(uint32_t start_addr, uint16_t *idata, uint16_t length);
int32_t SpiFlashWriteBytesWithErase(uint32_t start_addr, uint16_t *idata, uint16_t length);
int32_t SpiFlashAutoEraseSector(uint32_t start_addr, uint16_t length);

void FlashInit(void);
void Flash_1s(void);

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================
```

### `device/dev_led.c`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_led.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-1-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-1-16    <td>V1.0.0    <td>             <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "F28x_Project.h"
#include "dev_led.h"
#include "gpio.h"
#include "cpu_timers.h"

void LED_100ms(void)
{
    static uint16_t leddelay = 0;

    leddelay++;
    if(leddelay == 5)
    {
        LedGpioToggle();
        leddelay = 0;
    }
}

static LedInfo_t   _gstLedInfo[LED_SRC_NUM];


static void _LedOFF(EN_LED_SRC index)
{
    if(_gstLedInfo[index].led_hw_mode == LED_HW_GPIO)
    {
        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            SetLedGpioState(0);
        }
        else
        {
            SetLedGpioState(1);
        }
    }
    else if(_gstLedInfo[index].led_hw_mode == LED_HW_PWM)
    {
//        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
//        {
//            _gstLedInfo[index].p_timer_Handle->oc_pwm_duty = 0;
//        }
//        else
//        {
//            _gstLedInfo[index].p_timer_Handle->oc_pwm_duty = 100;
//        }
//        _gstLedInfo[index].p_timer_Handle->current_chl = _gstLedInfo[index].tim_ch;
//        TimerPWMDutySet(_gstLedInfo[index].p_timer_Handle, 100);
    }
}

static void _LedON(EN_LED_SRC index)
{
    if(_gstLedInfo[index].led_hw_mode == LED_HW_GPIO)
    {
        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            SetLedGpioState(1);
        }
        else
        {
            SetLedGpioState(0);
        }
    }
    else if(_gstLedInfo[index].led_hw_mode == LED_HW_PWM)
    {
//        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
//        {
//            _gstLedInfo[index].p_timer_Handle->oc_pwm_duty = 100;
//            
//        }
//        else
//        {
//            _gstLedInfo[index].p_timer_Handle->oc_pwm_duty = 0;
//        }
//        _gstLedInfo[index].p_timer_Handle->current_chl = _gstLedInfo[index].tim_ch;
//        TimerPWMDutySet(_gstLedInfo[index].p_timer_Handle, 100);
    }
}


static void _LedBlink(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = _gstLedInfo[index].led_status_period;
    
    if(first_flag == 1)/*?*/
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
        _LedOFF(index);
        return;
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= period / 2)
    {
        if(led_flag[index] == 0)
        {
            _LedON(index);
            led_flag[index] = 1;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= period)
    {
        timecount[index] = TimGetSystime();
        if(led_flag[index] == 1)
        {
            _LedOFF(index);
            led_flag[index] = 0;
        }
    }
}


static void _LedStandby(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = 2000;
    
    if(first_flag == 1)/*������ʼ��*/
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
        _LedOFF(index);
        return;
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= 100)
    {
        if(led_flag[index] == 0)
        {
            _LedON(index);
            led_flag[index] = 1;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 200)
    {
        if(led_flag[index] == 1)
        {
            _LedOFF(index);
            led_flag[index] = 2;
        }
    }
    
    if(TimDifftimeFromNow(timecount[index]) > period)
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
    }
}

static void _LedFailed(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = 3000;
    
    if(first_flag == 1)/*������ʼ��*/
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
        _LedOFF(index);
        return;
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= 100)
    {
        if(led_flag[index] == 0)
        {
            _LedON(index);
            led_flag[index] = 1;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 200)
    {
        if(led_flag[index] == 1)
        {
            _LedOFF(index);
            led_flag[index] = 2;
        }
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= 400)
    {
        if(led_flag[index] == 2)
        {
            _LedON(index);
            led_flag[index] = 3;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 500)
    {
        if(led_flag[index] == 3)
        {
            _LedOFF(index);
            led_flag[index] = 4;
        }
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= 700)
    {
        if(led_flag[index] == 4)
        {
            _LedON(index);
            led_flag[index] = 5;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 800)
    {
        if(led_flag[index] == 5)
        {
            _LedOFF(index);
            led_flag[index] = 6;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 1000)
    {
        if(led_flag[index] == 6)
        {
            _LedON(index);
            led_flag[index] = 7;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 1100)
    {
        if(led_flag[index] == 7)
        {
            _LedOFF(index);
            led_flag[index] = 8;
        }
    }
    
    if(TimDifftimeFromNow(timecount[index]) > period)
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
    }
}

static void _LedSucceed(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = 3000;
    
    if(first_flag == 1)/*������ʼ��*/
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
        _LedOFF(index);
        return;
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= 100)
    {
        if(led_flag[index] == 0)
        {
            _LedON(index);
            led_flag[index] = 1;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 200)
    {
        if(led_flag[index] == 1)
        {
            _LedOFF(index);
            led_flag[index] = 2;
        }
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= 400)
    {
        if(led_flag[index] == 2)
        {
            _LedON(index);
            led_flag[index] = 3;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 500)
    {
        if(led_flag[index] == 3)
        {
            _LedOFF(index);
            led_flag[index] = 4;
        }
    }
    
    
    if(TimDifftimeFromNow(timecount[index]) > period)
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
    }
}






void LedCtlOn(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_ON;
}

void LedCtlOff(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_OFF;
}

void LedCtlBlink(EN_LED_SRC src, uint32_t period)
{
    _gstLedInfo[src].led_status = LED_STATUS_BLINK;
    _gstLedInfo[src].led_status_period = period;
}


void LedCtlStandby(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_STANDBY;
}

void LedCtlSucceed(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_SUCCEED;
}

void LedCtlFailed(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_FAILED;
}




void DevLedInit(void)
{    
    _gstLedInfo[LED_SRC_SYS].led_hw_mode = LED_HW_GPIO;
    _gstLedInfo[LED_SRC_SYS].led_status = LED_STATUS_IDLE;
    _gstLedInfo[LED_SRC_SYS].led_status_period = RUN_LED_BRINK_PRIED;
    _gstLedInfo[LED_SRC_SYS].led_active_level = LED_HW_ACTIVE_LOW;

    LedCtlBlink(LED_SRC_SYS, RUN_LED_BRINK_PRIED);
}


void DevLedProcess(void)
{
    uint8_t i;
    static EN_LED_STATUS pre_status[LED_SRC_NUM];

    for(i = 0; i < LED_SRC_NUM; i++)
    {
        switch(_gstLedInfo[i].led_status)
        {
            case LED_STATUS_IDLE:
            {
                
                break;
            }
            case LED_STATUS_OFF:
            {
                _LedOFF((EN_LED_SRC)i);
                _gstLedInfo[i].led_status = LED_STATUS_IDLE;
                break;
            }
            case LED_STATUS_ON:
            {
                _LedON((EN_LED_SRC)i);
                _gstLedInfo[i].led_status = LED_STATUS_IDLE;
                break;
            }
            case LED_STATUS_BLINK:
            {
                if(pre_status[i] != LED_STATUS_BLINK)
                {
                    _LedBlink((EN_LED_SRC)i, 1);
                    pre_status[i] = LED_STATUS_BLINK;
                }
                else
                {
                    _LedBlink((EN_LED_SRC)i, 0);
                }
                break;
            }
            
            case LED_STATUS_STANDBY:
                        {
                            if(pre_status[i] != LED_STATUS_STANDBY)
                            {
                                _LedStandby((EN_LED_SRC)i, 1);
                                pre_status[i] = LED_STATUS_STANDBY;
                            }
                            else
                            {
                                _LedStandby((EN_LED_SRC)i, 0);
                            }
                            break;
                        }
                        case LED_STATUS_SUCCEED:
                        {
                            if(pre_status[i] != LED_STATUS_SUCCEED)
                            {
                                _LedSucceed((EN_LED_SRC)i, 1);
                                pre_status[i] = LED_STATUS_SUCCEED;
                            }
                            else
                            {
                                _LedSucceed((EN_LED_SRC)i, 0);
                            }
                            break;
                        }
                        case LED_STATUS_FAILED:
                        {
                            if(pre_status[i] != LED_STATUS_FAILED)
                            {
                                _LedFailed((EN_LED_SRC)i, 1);
                                pre_status[i] = LED_STATUS_FAILED;
                            }
                            else
                            {
                                _LedFailed((EN_LED_SRC)i, 0);
                            }
                            break;
                        }
            default:break;
        }
    }
}
```

### `device/dev_led.h`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_led.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-1-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-1-16    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef APP_LED_H_
#define APP_LED_H_
#include "F28x_Project.h"

#define RUN_LED_BRINK_PRIED 1000

typedef enum
{
    LED_SRC_SYS,
    LED_SRC_NUM
} EN_LED_SRC;

typedef enum
{
    LED_HW_GPIO,
    LED_HW_PWM
} EN_LED_HW_MODE;

typedef enum
{
    LED_HW_ACTIVE_HIGH,
    LED_HW_ACTIVE_LOW
} EN_LED_HW_ACTIVE_LEVEL;


typedef enum
{
    LED_STATUS_IDLE,
    LED_STATUS_OFF,
    LED_STATUS_ON,
    LED_STATUS_BLINK,
    LED_STATUS_BREATHING,
    LED_STATUS_SOS,
    LED_STATUS_STANDBY,
    LED_STATUS_FAILED,
    LED_STATUS_SUCCEED,
} EN_LED_STATUS;



typedef struct _led_info_t
{
    EN_LED_SRC              led_src;
    EN_LED_HW_MODE          led_hw_mode;
    EN_LED_HW_ACTIVE_LEVEL  led_active_level;
    EN_LED_STATUS           led_status;
    uint32_t                led_status_period;//��Ҫ��Ժ�������˸�ȳ����仯״̬����ʾһ��ѭ����Ҫ�೤ʱ��
    uint8_t                 led_brightness;//���100

//    Gpio_HandleType         *p_gpio_Handle;
//    Timer_HandleType        *p_timer_Handle;
    uint8_t                  tim_ch;//���PWM���Ƶ�LED����Ӧ��ʱ���ĸ�ͨ����GPIO���Ƶ�LED�ò���������
} LedInfo_t;

void DevLedInit(void);
void DevLedProcess(void);
void LedCtlOn(EN_LED_SRC src);
void LedCtlOff(EN_LED_SRC src);
void LedCtlBlink(EN_LED_SRC src, uint32_t period);
void LedCtlStandby(EN_LED_SRC src);
void LedCtlSucceed(EN_LED_SRC src);
void LedCtlFailed(EN_LED_SRC src);


#endif /* APP_LED_H_ */
```

### `device/dev_ntc.c`

```c
/**
 * @file dev_ntc.c
 * @brief NTC thermistor parameters and temperature conversion
 */

#include "dev_ntc.h"

#define LN(a) (_ApproLn(a))

NtcParam_t _gNtcPara = {
    .b_value = 4050.0f,
    .r25_value = 10.0f,
    .t25_kelvin = 1.0f / (25.0f + 273.15f),
    .vol_up = NTC_VOL_UP,
    .r_up = NTC_R_UP,
    .r_down = NTC_R_DOWN,
};

/**
 * @brief Approximate natural logarithm
 * @param a Input value
 * @return Approximation of ln(a)
 */
static double _ApproLn(double a)
{
    uint16_t n = 15U; /* Use first n+1 terms */
    uint16_t k = 0U;
    uint16_t nk = 0U;
    double x = 0.0;
    double x2 = 0.0;
    double y = 0.0;

    x = (a - 1.0) / (a + 1.0);
    x2 = x * x;
    nk = 2U * n + 1U;
    y = 1.0 / nk;

    for(k = n; k > 0U; k--)
    {
        nk -= 2U;
        y = 1.0 / nk + x2 * y;
    }
    return 2.0 * x * y;
}

/**
 * @brief Convert ADC voltage to temperature
 * @param voltage ADC voltage (V)
 * @return Temperature (degC)
 */
float CalculateTemp(float voltage)
{
    float rtotal = 0.0f;
    float rntc = 0.0f;
    float rtemp = 0.0f;
    float temperature = 0.0f;

    if(voltage <= 0.0f || voltage >= _gNtcPara.vol_up)
    {
        return 0.0f;
    }

    rtotal = (_gNtcPara.r_up * voltage) / (_gNtcPara.vol_up - voltage);
    rntc = (_gNtcPara.r_down * rtotal) / (_gNtcPara.r_down - rtotal);
    rtemp = LN(rntc / _gNtcPara.r25_value) / _gNtcPara.b_value + _gNtcPara.t25_kelvin;
    temperature = 1.0f / rtemp - 273.15f;

    return temperature;
}
```

### `device/dev_ntc.h`

```c
/**
 * @file dev_ntc.h
 * @brief NTC thermistor parameters and temperature conversion
 */

#ifndef DEVICE_DEV_NTC_H_
#define DEVICE_DEV_NTC_H_

#include "F28x_Project.h"

typedef struct
{
    float b_value;      /**< B constant */
    float r25_value;    /**< Resistance at 25 degC (kOhm) */
    float t25_kelvin;   /**< Kelvin temperature at 25 degC */
    float vol_up;       /**< Pull-up supply voltage */
    float r_up;         /**< Pull-up resistor (kOhm) */
    float r_down;       /**< Pull-down resistor (kOhm) */
} NtcParam_t;

#define NTC_VOL_UP   (3.3f)   /**< Pull-up voltage (V) */
#define NTC_R_UP     (10.0f)  /**< Pull-up resistor (kOhm) */
#define NTC_R_DOWN   (75.0f)  /**< Pull-down resistor (kOhm) */

static double _ApproLn(double a);
float CalculateTemp(float voltage);

#endif /* DEVICE_DEV_NTC_H_ */
```

### `device/fan.c`

```c
/**
 * @file fan.c
 * @brief Fan control, self-test, and fault detection
 */

#include <fan.h>
#include "F28x_Project.h"
#include "ntc.h"
#include "pwm.h"
#include "debug_analysis.h"
#include "gpio.h"

#pragma CODE_SECTION(_FanFaultProcess, ".TI.ramfunc");

static FanStrut_t _gFan[FAN_NUM];

/**
 * @brief Configure one fan instance
 * @param fanIndex Fan index
 * @param cap_gpio Tach feedback GPIO
 * @param in_chk_type Feedback detection type
 * @param active_level_type Active feedback level
 */
static void _FanConfig(uint8_t fanIndex, uint16_t cap_gpio, FAN_IN_CHK_TYPE in_chk_type,
                       FAN_ACTIVE_LEVEL_TYPE active_level_type)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    _gFan[fanIndex].state.current_state = FAN_STATE_INIT;
    _gFan[fanIndex].state.speed = EN_FAN_SPEED_0_PERCENT;

    _gFan[fanIndex].value.duty = 0.0f;
    _gFan[fanIndex].value.temperature = 0.0f;
    _gFan[fanIndex].value.level_cont = 0U;
    _gFan[fanIndex].value.total_level_cont = 0U;

    _gFan[fanIndex].config.self_check_time = SELF_CHECK_TIME;
    _gFan[fanIndex].config.fault_detected = 0U;
    _gFan[fanIndex].config.in_chk_type = in_chk_type;
    _gFan[fanIndex].config.speed_check_counter = 0U;
    _gFan[fanIndex].config.cap_gpio = cap_gpio;
    _gFan[fanIndex].config.active_level_type = active_level_type;
    _gFan[fanIndex].self_check_cont = 0U;
}

/**
 * @brief Initialize all fans (staggered fault-check timing to reduce ISR load)
 */
void FanInit(void)
{
    _FanConfig(0U, FAN1_CAP_GPIO, FAN_IN_FAULT_PULSE, IN_HIGH_LEVLE);
}

/**
 * @brief Update fan speed from NTC temperatures
 * @param fanIndex Fan index
 */
static void _FanCalculateSpeed(uint8_t fanIndex)
{
    float ctr_temp = 0.0f;
    float pv_temp = 0.0f;
    float inv_temp = 0.0f;

    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    ctr_temp = NTCGetFilterTemp(NTC_CHASSIS_TEMP);
    pv_temp = NTCGetFilterTemp(NTC_PV_TEMP);
    inv_temp = NTCGetFilterTemp(NTC_INV_TEMP);

    if(_gFan[fanIndex].state.speed == EN_FAN_SPEED_80_PERCENT)
    {
        if((inv_temp < 100.0f) && (pv_temp < 100.0f) && (ctr_temp < 75.0f))
        {
            _gFan[fanIndex].state.speed = EN_FAN_SPEED_50_PERCENT;
        }
    }
    else
    {
        if((inv_temp < 75.0f) && (pv_temp < 75.0f) && (ctr_temp < 60.0f))
        {
            _gFan[fanIndex].state.speed = EN_FAN_SPEED_0_PERCENT;
        }
        else if((inv_temp > 105.0f) || (pv_temp > 105.0f) || (ctr_temp > 80.0f))
        {
            _gFan[fanIndex].state.speed = EN_FAN_SPEED_80_PERCENT;
        }
        else if((inv_temp > 85.0f) || (pv_temp > 85.0f) || (ctr_temp > 70.0f))
        {
            _gFan[fanIndex].state.speed = EN_FAN_SPEED_50_PERCENT;
        }
    }
}

/**
 * @brief Map speed state to PWM duty
 * @param fanIndex Fan index
 */
static void _FanDutySet(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    switch(_gFan[fanIndex].state.speed)
    {
        case EN_FAN_SPEED_0_PERCENT:
            _gFan[fanIndex].value.duty = 0.0f;
            break;
        case EN_FAN_SPEED_20_PERCENT:
            _gFan[fanIndex].value.duty = 0.2f;
            break;
        case EN_FAN_SPEED_40_PERCENT:
            _gFan[fanIndex].value.duty = 0.4f;
            break;
        case EN_FAN_SPEED_50_PERCENT:
            _gFan[fanIndex].value.duty = 0.5f;
            break;
        case EN_FAN_SPEED_80_PERCENT:
            _gFan[fanIndex].value.duty = 0.8f;
            break;
        case EN_FAN_SPEED_100_PERCENT:
            _gFan[fanIndex].value.duty = 1.0f;
            break;
        default:
            _gFan[fanIndex].value.duty = 0.0f;
            break;
    }
}

void _gFanPwmUpdate(void)
{
    float duty_arr[FAN_NUM] = {0};
    uint8_t i;

    for(i = 0U; i < FAN_NUM; i++)
    {
        duty_arr[i] = _gFan[i].value.duty;
    }
    FanEpwmUpdata(duty_arr, 1U);
}

/**
 * @brief Leave INIT and enter self-test at 40% speed
 * @param fanIndex Fan index
 */
static void _FanStateInit(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    _gFan[fanIndex].state.current_state = FAN_STATE_SELF_TEST;
    _gFan[fanIndex].self_check_cont = 0U;
    _gFan[fanIndex].state.speed = EN_FAN_SPEED_40_PERCENT;
    _FanDutySet(fanIndex);
}

/**
 * @brief Self-test: run at fixed speed and check tach feedback
 * @param fanIndex Fan index
 */
static void _FanSelfCheck(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    _gFan[fanIndex].self_check_cont++;
    if(_gFan[fanIndex].self_check_cont >= _gFan[fanIndex].config.self_check_time)
    {
        _gFan[fanIndex].self_check_cont = 0U;
        if(_gFan[fanIndex].config.fault_detected)
        {
            _gFan[fanIndex].state.current_state = FAN_STATE_FAULT;
        }
        else
        {
            _gFan[fanIndex].state.current_state = FAN_STATE_NORMAL;
        }
        _gFan[fanIndex].state.speed = EN_FAN_SPEED_0_PERCENT;
        _FanDutySet(fanIndex);
    }
}

/**
 * @brief Normal operation: temperature-based speed control
 * @param fanIndex Fan index
 */
static void _FanStateNormal(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    _FanCalculateSpeed(fanIndex);
    _FanDutySet(fanIndex);
    if(_gFan[fanIndex].config.fault_detected)
    {
        _gFan[fanIndex].state.current_state = FAN_STATE_FAULT;
    }
}

/**
 * @brief Fault state: stop fan and schedule recovery
 * @param fanIndex Fan index
 */
static void _FanStateFault(uint8_t fanIndex)
{
    static uint32_t recovery_counter[FAN_NUM] = {0};

    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    _gFan[fanIndex].state.speed = EN_FAN_SPEED_0_PERCENT;
    _FanDutySet(fanIndex);

    recovery_counter[fanIndex]++;
    if(recovery_counter[fanIndex] >= 1U)
    {
        recovery_counter[fanIndex] = 0U;
        _gFan[fanIndex].state.current_state = FAN_STATE_RECOVERY;
    }
}

/**
 * @brief Recovery: re-enter self-test at 40% speed
 * @param fanIndex Fan index
 */
static void _FanStateRecovery(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    _gFan[fanIndex].state.current_state = FAN_STATE_SELF_TEST;
    _gFan[fanIndex].state.speed = EN_FAN_SPEED_40_PERCENT;
    _FanDutySet(fanIndex);
}

/**
 * @brief Fan state machine
 * @param fanIndex Fan index
 */
static void _FanStateProcess(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    switch(_gFan[fanIndex].state.current_state)
    {
        case FAN_STATE_INIT:
            _FanStateInit(fanIndex);
            break;
        case FAN_STATE_SELF_TEST:
            _FanSelfCheck(fanIndex);
            break;
        case FAN_STATE_NORMAL:
            _FanStateNormal(fanIndex);
            break;
        case FAN_STATE_FAULT:
            _FanStateFault(fanIndex);
            break;
        case FAN_STATE_RECOVERY:
            _FanStateRecovery(fanIndex);
            break;
        default:
            _gFan[fanIndex].state.current_state = FAN_STATE_INIT;
            break;
    }
}

/**
 * @brief Tach / level fault detection (called from ISR)
 * @param fanIndex Fan index
 */
static void _FanFaultProcess(uint8_t fanIndex)
{
    EN_SPEED_STATE current_speed;
    uint8_t current_gpio_state = 0U;

    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    current_speed = _gFan[fanIndex].state.speed;
    if(current_speed == EN_FAN_SPEED_0_PERCENT)
    {
        return;
    }

    current_gpio_state = GPIO_ReadPin(_gFan[fanIndex].config.cap_gpio);

    if(current_gpio_state == _gFan[fanIndex].config.active_level_type)
    {
        _gFan[fanIndex].value.level_cont++;
    }

    _gFan[fanIndex].value.total_level_cont++;
    _gFan[fanIndex].config.speed_check_counter++;

    if(_gFan[fanIndex].config.speed_check_counter >= (SPEED_FAULT_CHECK_TIME + fanIndex))
    {
        _gFan[fanIndex].config.speed_check_counter = 0U;
        if(_gFan[fanIndex].config.in_chk_type == FAN_IN_FAULT_LEVLE)
        {
            if(_gFan[fanIndex].value.level_cont < 0.9f * _gFan[fanIndex].value.total_level_cont)
            {
                _gFan[fanIndex].config.fault_detected = 1U;
            }
            else
            {
                _gFan[fanIndex].config.fault_detected = 0U;
            }
        }
        else
        {
            if(_gFan[fanIndex].value.level_cont < (0.3f * _gFan[fanIndex].value.total_level_cont)
               || _gFan[fanIndex].value.level_cont > (0.7f * _gFan[fanIndex].value.total_level_cont))
            {
                _gFan[fanIndex].config.fault_detected = 1U;
            }
            else
            {
                _gFan[fanIndex].config.fault_detected = 0U;
            }
        }

        _gFan[fanIndex].value.level_cont = 0U;
        _gFan[fanIndex].value.total_level_cont = 0U;
    }
}

uint8_t FanGetAllFaultState(void)
{
    uint8_t i;

    for(i = 0U; i < FAN_NUM; i++)
    {
        if(_gFan[i].config.fault_detected != 0U)
        {
            return 1U;
        }
    }
    return 0U;
}

void FanProcess(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }
    _FanStateProcess(fanIndex);
}

/**
 * @brief Copy external temperature array into fan instances
 * @param temp_array Temperature per fan
 * @param array_size Unused; kept for API compatibility
 * @return Number of fans updated
 */
uint8_t TempDataGet(float temp_array[])
{
    uint8_t i;
    uint8_t processed_count = 0U;

    if(temp_array == NULL)
    {
        return 0U;
    }

    for(i = 0U; i < FAN_NUM; i++)
    {
        _gFan[i].value.temperature = temp_array[i];
        processed_count++;
    }
    return processed_count;
}

/**
 * @brief Round-robin fan process (call from main loop)
 */
void FanProcessAll(void)
{
    static uint8_t i = 0U;

    _gFan[0].value.temperature = GetNTCMaxTemp();
    FanProcess(i);
    i++;
    if(i >= FAN_NUM)
    {
        i = 0U;
    }
    _gFanPwmUpdate();
}

/**
 * @brief Run tach fault detection for all fans (call from ISR)
 */
void FanFaultProcessAll(void)
{
    uint8_t i;

    for(i = 0U; i <= FAN_NUM; i++)
    {
        _FanFaultProcess(i);
    }
}

bool GetFanFaultState(void)
{
    uint8_t i;

    for(i = 0U; i < FAN_NUM; i++)
    {
        if(_gFan[i].config.fault_detected)
        {
            return true;
        }
    }
    return false;
}
```

### `device/fan.h`

```c
/**
 * @file fan.h
 * @brief Fan control, self-test, and fault detection
 */

#include "stdint.h"
#include "F28x_Project.h"

#ifndef MIDDLEWARE_FAN_H_
#define MIDDLEWARE_FAN_H_

/* ==================== Configuration ==================== */
#define FAN_NUM (1U)

//#define FAN1_CAP_GPIO (80U)
//#define FAN2_CAP_GPIO (81U)
//#define FAN3_CAP_GPIO (52U)
//#define FAN4_CAP_GPIO (53U)

/* Temperature thresholds (degC) */
#define TEMP_THRESHOLD_LEVEL0 (20.0f)
#define TEMP_THRESHOLD_LEVEL1 (40.0f)
#define TEMP_THRESHOLD_LEVEL2 (60.0f)
#define TEMP_THRESHOLD_LEVEL3 (70.0f)
#define TEMP_THRESHOLD_LEVEL4 (80.0f)

/* Self-test duration (100 ms units) */
#define SELF_CHECK_TIME (36U * FAN_NUM) /* 1.2 s */

/* Speed fault check interval (20 kHz ISR counts ~= 250 ms) */
#define SPEED_FAULT_CHECK_TIME ((80000U / 4U) * 0.25f)

typedef enum
{
    FAN_STATE_INIT = 0,
    FAN_STATE_SELF_TEST,
    FAN_STATE_NORMAL,
    FAN_STATE_FAULT,
    FAN_STATE_RECOVERY
} FAN_STATE;

typedef enum
{
    EN_FAN_SPEED_0_PERCENT = 0,
    EN_FAN_SPEED_20_PERCENT,
    EN_FAN_SPEED_40_PERCENT,
    EN_FAN_SPEED_50_PERCENT,
    EN_FAN_SPEED_80_PERCENT,
    EN_FAN_SPEED_100_PERCENT,
    EN_FAN_SPEED_MAX
} EN_SPEED_STATE;

typedef enum
{
    FAN_IN_FAULT_PULSE = 0, /**< Pulse (tach) detection */
    FAN_IN_FAULT_LEVLE    /**< Level detection */
} FAN_IN_CHK_TYPE;

typedef enum
{
    IN_LOW_ACTIVE = 0,
    IN_HIGH_LEVLE
} FAN_ACTIVE_LEVEL_TYPE; /**< Active feedback level */

typedef struct
{
    FAN_STATE current_state;
    EN_SPEED_STATE speed;
} FanState_t;

typedef struct
{
    float duty;
    float temperature;
    uint32_t level_cont;       /**< Active-level pulse count */
    uint32_t total_level_cont; /**< Total sample count */
} FanValue_t;

typedef struct
{
    uint16_t self_check_time;        /**< Self-test time (100 ms units) */
    uint32_t speed_check_counter;    /**< Speed monitor counter */
    uint16_t cap_gpio;               /**< Tach feedback GPIO */
    FAN_IN_CHK_TYPE in_chk_type;
    uint8_t fault_detected;
    FAN_ACTIVE_LEVEL_TYPE active_level_type;
} FanConfig_t;

typedef struct
{
    FanState_t state;
    FanValue_t value;
    FanConfig_t config;
    uint16_t self_check_cont;
} FanStrut_t;

void FanInit(void);
void FanProcessAll(void);
void FanFaultProcessAll(void);
uint8_t TempDataGet(float temp_array[]);
void FanProcess(uint8_t fanIndex);
bool GetFanFaultState(void);
uint8_t FanGetAllFaultState(void);

#endif /* MIDDLEWARE_FAN_H_ */
```

### `device/ntc.c`

```c
/**
 * @file ntc.c
 * @brief NTC temperature acquisition, filtering, and fault detection
 */

#include <ntc.h>
#include <sample_update.h>

#pragma CODE_SECTION(NTCProcess, ".TI.ramfunc");
#pragma CODE_SECTION(_DetectFault, ".TI.ramfunc");
#pragma CODE_SECTION(_ProcessChannelTempe, ".TI.ramfunc");

NtcTemp_t _gNtcTemperate;
static float _gNtcInVol[NTC_CHANNEL_NUM] = {0};
static float _gNtcFilterBuffer[NTC_CHANNEL_NUM][NTC_AVERAGE_COUNT];

static bool _SlidingFilterReady(const Mid_SlidingFilter_t* filter)
{
    return (filter->is_full != 0U) || (filter->index > 0U);
}

static void _FaultInit(NtcFaultChannel_t* fault)
{
    fault->current_fault = NTC_FAULT_NONE;
    fault->previous_fault = NTC_FAULT_NONE;
    fault->fault_active = false;
    fault->fault_count = 0U;
    fault->fault_run_cont = 0U;
}

void NTCInit(void)
{
    uint8_t i;

    for(i = 0U; i < NTC_CHANNEL_NUM; i++)
    {
        _gNtcTemperate.raw_temperature[i] = 0.0f;
        _gNtcTemperate.filtered_temperature[i] = 0.0f;
        _gNtcTemperate.valid[i] = false;
        Mid_SlidingFilter_Init(&_gNtcTemperate.filter[i], &_gNtcFilterBuffer[i][0], NTC_AVERAGE_COUNT);
        _FaultInit(&_gNtcTemperate.fault[i]);
    }

    _gNtcTemperate.current_channel = 0U;
}

static NTC_FAULT_TYPE _DetectFault(uint8_t channel, float temperature)
{
    NTC_FAULT_TYPE fault_type = NTC_FAULT_NONE;
    NtcFaultChannel_t* fault = &_gNtcTemperate.fault[channel];

    /* Open-circuit / break detection */
    if(temperature <= -45.0f || temperature >= 160.0f)
    {
        fault_type = NTC_FAULT_BREAK;
        _gNtcTemperate.filtered_temperature[channel] = temperature;
    }
    else if(fault->fault_active)
    {
        /* Use recovery threshold while fault is active */
        if(fault->current_fault == NTC_FAULT_BREAK && temperature > -40.0f && temperature < 155.0f)
        {
            fault_type = NTC_FAULT_NONE;
        }
        else
        {
            fault_type = fault->current_fault;
        }
    }

    if(fault_type != fault->current_fault)
    {
        fault->previous_fault = fault->current_fault;
        fault->current_fault = fault_type;

        if(fault_type != NTC_FAULT_NONE)
        {
            fault->fault_active = true;
            fault->fault_count++;
        }
        else
        {
            fault->fault_active = false;
        }
    }

    return fault_type;
}

static void _ProcessChannelTempe(uint8_t channel)
{
    float voltage = 0.0f;
    float raw_temperature = 0.0f;
    float filtered_temperature = 0.0f;
    Mid_SlidingFilter_t* filter = NULL;

    if(channel >= NTC_CHANNEL_NUM)
    {
        return;
    }

    filter = &_gNtcTemperate.filter[channel];
    voltage = _gNtcInVol[channel];
    raw_temperature = CalculateTemp(voltage);

    _gNtcTemperate.fault[channel].fault_run_cont++;
    _gNtcTemperate.raw_temperature[channel] =
        _gNtcTemperate.raw_temperature[channel] * 0.99f + raw_temperature * 0.01f;

    filtered_temperature = Mid_SlidingFilter_Update(filter, _gNtcTemperate.raw_temperature[channel]);

    if(_SlidingFilterReady(filter))
    {
        _gNtcTemperate.filtered_temperature[channel] = filtered_temperature;
        _gNtcTemperate.valid[channel] = true;

        if(_gNtcTemperate.fault[channel].fault_run_cont >= NTC_FAULT_JUDGE_COUNT)
        {
            _gNtcTemperate.fault[channel].fault_run_cont = 0U;
            _DetectFault(channel, filtered_temperature);
        }
    }
    else
    {
        _gNtcTemperate.valid[channel] = false;
    }
}

void NTCProcess(void)
{
    _ProcessChannelTempe(_gNtcTemperate.current_channel);

    _gNtcTemperate.current_channel++;
    if(_gNtcTemperate.current_channel >= NTC_CHANNEL_NUM)
    {
        _gNtcTemperate.current_channel = 0U;
    }
}

float NTCGetRawTemp(uint8_t channel)
{
    if(channel >= NTC_CHANNEL_NUM)
    {
        return 0.0f;
    }
    return _gNtcTemperate.raw_temperature[channel];
}

float NTCGetFilterTemp(NTC_VALUE_TYPE channel)
{
    if(channel >= NTC_CHANNEL_NUM || !_gNtcTemperate.valid[channel])
    {
        return 0.0f;
    }
    return _gNtcTemperate.filtered_temperature[channel];
}

void NTCGetInVol(uint8_t channel, float vol)
{
    if(channel >= NTC_CHANNEL_NUM)
    {
        return;
    }
    _gNtcInVol[channel] = vol;
}

bool NTCChkData(uint8_t channel)
{
    if(channel >= NTC_CHANNEL_NUM)
    {
        return false;
    }
    return _gNtcTemperate.valid[channel];
}

bool NTCFaultActiveState(uint8_t channel)
{
    if(channel >= NTC_CHANNEL_NUM)
    {
        return false;
    }
    return _gNtcTemperate.fault[channel].fault_active;
}

bool GetNTCFault(void)
{
    uint8_t i;

    for(i = 0U; i < NTC_CHANNEL_NUM; i++)
    {
        if(_gNtcTemperate.fault[i].fault_active)
        {
            return true;
        }
    }
    return false;
}

uint8_t NTCGetFaultCount(uint8_t channel)
{
    if(channel >= NTC_CHANNEL_NUM)
    {
        return 0U;
    }
    return _gNtcTemperate.fault[channel].fault_count;
}

void NTCClearFault(uint8_t channel)
{
    if(channel < NTC_CHANNEL_NUM)
    {
        _FaultInit(&_gNtcTemperate.fault[channel]);
    }
}

void NTCClearAllFault(void)
{
    uint8_t i;

    for(i = 0U; i < NTC_CHANNEL_NUM; i++)
    {
        _FaultInit(&_gNtcTemperate.fault[i]);
    }
}

float GetNTCMaxTemp(void)
{
    uint8_t i;
    float max = -50.0f;

    for(i = 0U; i < NTC_CHANNEL_NUM; i++)
    {
        if(_gNtcTemperate.filtered_temperature[i] > max)
        {
            max = _gNtcTemperate.filtered_temperature[i];
        }
    }
    return max;
}

bool NTCFaultChangeState(uint8_t channel)
{
    if(channel >= NTC_CHANNEL_NUM)
    {
        return false;
    }
    return (_gNtcTemperate.fault[channel].current_fault != _gNtcTemperate.fault[channel].previous_fault);
}
```

### `device/ntc.h`

```c
/**
 * @file ntc.h
 * @brief NTC temperature acquisition, filtering, and fault detection
 */

#ifndef _NTC_H_
#define _NTC_H_

#include "F28x_Project.h"
#include "dev_ntc.h"
#include "mid_sliding_filter.h"

/** User-configurable channel count */
#define NTC_CHANNEL_NUM         (3U)
#define NTC_AVERAGE_COUNT       (30U)   /**< Sliding-average window size */
#define NTC_FAULT_JUDGE_COUNT   (10U)   /**< Fault check interval (not every cycle) */

typedef enum
{
    NTC_FAULT_NONE = 0,
    NTC_FAULT_BREAK,
} NTC_FAULT_TYPE;

typedef struct
{
    NTC_FAULT_TYPE current_fault;
    NTC_FAULT_TYPE previous_fault;
    bool fault_active;
    uint8_t fault_count;
    uint8_t fault_run_cont; /**< Run counter; fault judge deferred until system init completes */
} NtcFaultChannel_t;

typedef struct
{
    float raw_temperature[NTC_CHANNEL_NUM];
    float filtered_temperature[NTC_CHANNEL_NUM];
    bool valid[NTC_CHANNEL_NUM];
    Mid_SlidingFilter_t filter[NTC_CHANNEL_NUM];
    uint8_t current_channel;
    NtcFaultChannel_t fault[NTC_CHANNEL_NUM];
} NtcTemp_t;

typedef enum
{
    NTC_CHASSIS_TEMP = 0,
    NTC_PV_TEMP,
    NTC_INV_TEMP
} NTC_VALUE_TYPE;

void NTCGetInVol(uint8_t channel, float vol);
void NTCInit(void);
void NTCProcess(void);

float NTCGetRawTemp(uint8_t channel);
float NTCGetFilterTemp(NTC_VALUE_TYPE channel);
float GetNTCMaxTemp(void);
bool GetNTCFault(void);

bool NTCChkData(uint8_t channel);
bool NTCFaultActiveState(uint8_t channel);
uint8_t NTCGetFaultCount(uint8_t channel);
void NTCClearFault(uint8_t channel);
void NTCClearAllFault(void);
bool NTCFaultChangeState(uint8_t channel);

#endif /* _NTC_H_ */
```

### `device/rcmu.c`

```c
/*
 * rcmu.c
 *
 *  Created on: Oct 30, 2025
 *      Author: Nova006348
 */

#include "rcmu.h"
#include "gpio.h"
#include "debug_analysis.h"
#include "mid_filter.h"

LeakDetect_t _gLeakDetector;
static Mid_Filter1OrderButter_t _gLowfilter_1order;

/* Forward declarations */
static void _LeakCurrNomalRun(float current_rms);
static void _LeakCurr60maSaltateChk(float current_avg);
static void _LeakCurr30maSaltateChk(float current_avg);
static void _LeakCurrPreChk(float rms_current);
static void _LeakCurrSelfChk(float current_rms);
static void _LeakCurrInitDelay(uint16_t cont);
static void _LeakCurr150maSaltateChk(float current_avg);

/**
 * @brief Initialize leak current detector
 */
static void _LeakCurrInit(void)
{
    memset(&_gLeakDetector, 0, sizeof(_gLeakDetector));

    _gLeakDetector.current_state = EN_LEAK_CURR_STATE_PRE_CHK;

    /* Clear history buffers */
    for (int i = 0; i < 3; i++) {
        _gLeakDetector.history_60ma[i] = 0.0f;
        _gLeakDetector.history_30ma[i] = 0.0f;
    }

    /* Reset output result */
    _gLeakDetector.result.self_test_passed = 0;
    _gLeakDetector.result.fault_flag = false;
    _gLeakDetector.result.rms_current_ma = 0.0f;
    _gLeakDetector.result.avg_rms_current_ma = 0.0f;
    _gLeakDetector.result.instant_curr_ma = 0.0f;
    _gLeakDetector.self_test_requested = false;
    _gLeakDetector.fault_cleared = false;
}

/**
 * @brief Calculate leak current RMS over one AC cycle
 * @param leak_current_ma Filtered instantaneous current (mA)
 * @param cycle_sample_count Samples per cycle
 * @param current_rms_ptr Output RMS pointer
 * @return true when one cycle RMS is updated
 */
static bool _RmsCalculate(float leak_current_ma, uint32_t cycle_sample_count, float* current_rms_ptr)
{
    static float total_sum_squares = 0.0f;
    static uint32_t sample_count = 0;
    bool cycle_completed = false;

    if (current_rms_ptr == NULL)
    {
        return false;
    }

    /* Accumulate squared samples */
    float current_square = leak_current_ma * leak_current_ma;

    total_sum_squares += current_square;
    sample_count++;

    /* Update RMS when one cycle completes */
    if (sample_count >= cycle_sample_count)
    {
        float current_rms = LEAK_CURRENT_CALCULATE_RMS(total_sum_squares, sample_count);
        *current_rms_ptr = current_rms;

        total_sum_squares = 0.0f;
        sample_count = 0;
        cycle_completed = true;
    }

    return cycle_completed;
}

/**
 * @brief Pre-check before self-test (test IO off)
 */
static void _LeakCurrPreChk(float rms_current)
{
    static float total_rms = 0.0f;
    static uint8_t cycle_count = 0;
    static uint8_t cycle_total_count = 0;
    float avg_rms = 0.0f;

    total_rms += rms_current;
    cycle_count++;
    if (cycle_count >= 3)
    {
        cycle_total_count ++;
        avg_rms = total_rms / cycle_count;
        total_rms = 0.0f;
        cycle_count = 0;
        if(avg_rms >= LEAK_CURRENT_SELF_TEST_THRESHOLD_MA)
        {
            _gLeakDetector.self_test_fault_count++;
        }
        else if(_gLeakDetector.self_test_fault_count != 0 && avg_rms < LEAK_CURRENT_SELF_TEST_THRESHOLD_MA)
        {
            _gLeakDetector.self_test_fault_count--;
        }
        if(cycle_total_count >= 4)
        {
            cycle_total_count = 0;
            if(_gLeakDetector.self_test_fault_count == 0)
            {
                /* Pre-check passed, enable self-test IO */
                SetRcmuChkState(1);
                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_SELF_CHK;
                _gLeakDetector.self_test_fault_count = 0;
            }
            else
            {
                _gLeakDetector.result.self_test_passed = -1;            /* Self-test failed */
//                _gLeakDetector.result.fault_flag = true;
//                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
                _gLeakDetector.self_test_fault_count = 0;
            }
        }
    }
}

/**
 * @brief Self-check with test IO on
 */
static void _LeakCurrSelfChk(float current_rms)
{
    static uint8_t self_test_cont = 0;
    self_test_cont++;
    if (current_rms >= LEAK_CURRENT_SELF_TEST_RANGE_MIN_MA && current_rms <= LEAK_CURRENT_SELF_TEST_RANGE_MAX_MA)
    {
        _gLeakDetector.self_test_success_count++;
    }
    else if(_gLeakDetector.self_test_success_count != 0 && (current_rms < LEAK_CURRENT_SELF_TEST_RANGE_MIN_MA || current_rms > LEAK_CURRENT_SELF_TEST_RANGE_MAX_MA))
    {
        _gLeakDetector.self_test_success_count--;
    }
    if (_gLeakDetector.self_test_success_count >= LEAK_CURRENT_SELF_TEST_FAULT_COUNT)
    {
        _gLeakDetector.result.self_test_passed = 1;            /* Self-test passed */
        _gLeakDetector.self_test_success_count = 0;
        self_test_cont = 0;
        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_RUN_DELAY;
        /* Disable self-test IO */
        SetRcmuChkState(0);
        return;
    }
    if(self_test_cont >= 7)
    {
        /* Self-test timeout */
        self_test_cont = 0;
        _gLeakDetector.self_test_success_count = 0;
//        _gLeakDetector.result.fault_flag = true;
        _gLeakDetector.result.self_test_passed = -1;
//        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
        SetRcmuChkState(0);
    }
}

/**
 * @brief Initialization delay before normal run
 * @param cont Delay count threshold
 */
static void _LeakCurrInitDelay(uint16_t cont)
{
    static uint16_t delay_test_cont = 0;
    delay_test_cont++;
    if(delay_test_cont >= cont)
    {
        delay_test_cont = 0;
        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_RUN_NOMAL;

        /* Clear step-detection history */
        for (int i = 0; i < 3; i++)
        {
            _gLeakDetector.history_60ma[i] = 0.0f;
            _gLeakDetector.history_30ma[i] = 0.0f;
        }
    }
}

/**
 * @brief Normal leak current monitoring
 * @param current_rms Cycle RMS current (mA)
 */
static void _LeakCurrNomalRun(float current_rms)
{
    static uint8_t rms_avg_cont = 0;
    static float total_rms_current = 0;
    total_rms_current += current_rms;
    rms_avg_cont++;
//    if (current_rms > LEAK_CURRENT_FAULT_THRESHOLD_THREE)
//    {
//        _gLeakDetector.result.fault_flag = true;
//        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
//        return;
//    }
    /* ~300mA level-2 fault detection */
    if (current_rms >= LEAK_CURRENT_FAULT_THRESHOLD_TWO)
    {
        _gLeakDetector.two_level_fault_count++;
    }
    else if(_gLeakDetector.two_level_fault_count != 0 && _gLeakDetector.two_level_fault_count > 0)
    {
        _gLeakDetector.two_level_fault_count--;
    }
    if (_gLeakDetector.two_level_fault_count >= LEAK_CURRENT_TWO_LEVEL_FAULT_THRESHOLD)
    {
        _gLeakDetector.result.fault_flag = true;
        _gLeakDetector.result.leak_type = 4;  /* Sudden leak type: ~300mA */
        _gLeakDetector.two_level_fault_count = 0;
        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
        return;
    }
     _LeakCurr150maSaltateChk(current_rms);
    if(rms_avg_cont >= 2)
    {
        _gLeakDetector.result.avg_rms_current_ma = total_rms_current / rms_avg_cont;
        total_rms_current = 0;
        rms_avg_cont = 0;
        _LeakCurr60maSaltateChk(_gLeakDetector.result.avg_rms_current_ma);
        _LeakCurr30maSaltateChk(_gLeakDetector.result.avg_rms_current_ma);

    }
}

/**
 * @brief 150mA sudden-change detection
 */
static void _LeakCurr150maSaltateChk(float current_avg)
{
    if (current_avg > _gLeakDetector.history_150ma[2])
    {  /* Current value > value before previous */
        float square = current_avg * current_avg;
        if (square > ((_gLeakDetector.history_150ma[2]+LEAK_CURRENT_SQUARE_DIFF_150MA_THRESHOLD)*(_gLeakDetector.history_150ma[2]+LEAK_CURRENT_SQUARE_DIFF_150MA_THRESHOLD)))
        {
            _gLeakDetector.square_diff_150ma_count++;

            if (_gLeakDetector.square_diff_150ma_count >= LEAK_CURRENT_SQUARE_DIFF_150MA_COUNT)
            {
                _gLeakDetector.result.fault_flag = true;
                _gLeakDetector.result.leak_type = 3;  /* Sudden leak type: 150mA */
                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
                _gLeakDetector.square_diff_150ma_count = 0;
                return;
            }
        }
        else
        {
            LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_150ma, current_avg);
            _gLeakDetector.square_diff_150ma_count = 0;
        }
    }
    else
    {
        LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_150ma, current_avg);
        _gLeakDetector.square_diff_150ma_count = 0;
    }
}

/**
 * @brief 60mA sudden-change detection
 */
static void _LeakCurr60maSaltateChk(float current_avg)
{
    if (current_avg > _gLeakDetector.history_60ma[2])
    {  /* Current value > value before previous */
        float square = current_avg * current_avg;
        if (square > ((_gLeakDetector.history_60ma[2]+LEAK_CURRENT_SQUARE_DIFF_60MA_THRESHOLD)*(_gLeakDetector.history_60ma[2]+LEAK_CURRENT_SQUARE_DIFF_60MA_THRESHOLD)))
        {
            _gLeakDetector.square_diff_60ma_count++;

            if (_gLeakDetector.square_diff_60ma_count >= LEAK_CURRENT_SQUARE_DIFF_60MA_COUNT)
            {
                _gLeakDetector.result.fault_flag = true;
                _gLeakDetector.result.leak_type = 2;  /* Sudden leak type: 60mA */
                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
                _gLeakDetector.square_diff_60ma_count = 0;
                return;
            }
        }
        else
        {
            LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_60ma, current_avg);
            _gLeakDetector.square_diff_60ma_count = 0;
        }
    }
    else
    {
        LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_60ma, current_avg);
        _gLeakDetector.square_diff_60ma_count = 0;
    }
}

/**
 * @brief 30mA sudden-change detection
 */
static void _LeakCurr30maSaltateChk(float current_avg)
{

    if (current_avg > _gLeakDetector.history_30ma[2])
    {
        /* Current value > value before previous */
        float square = current_avg * current_avg;
        if (square > ((_gLeakDetector.history_30ma[2]+LEAK_CURRENT_SQUARE_DIFF_30MA_THRESHOLD)*(_gLeakDetector.history_30ma[2]+LEAK_CURRENT_SQUARE_DIFF_30MA_THRESHOLD)))
        {
            _gLeakDetector.square_diff_30ma_count++;

            if (_gLeakDetector.square_diff_30ma_count >= LEAK_CURRENT_SQUARE_DIFF_30MA_COUNT)
            {
                _gLeakDetector.result.fault_flag = true;
                _gLeakDetector.result.leak_type = 1;  /* Sudden leak type: 30mA */
                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
                return;
            }
        }
        else
        {
            LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_30ma, current_avg);
            _gLeakDetector.square_diff_30ma_count = 0;
        }
    }
    else
    {  /* Current value <= value before previous */
        LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_30ma, current_avg);
        _gLeakDetector.square_diff_30ma_count = 0;
    }
}

/**
 * @brief Request leak current self-test
 */
void LeakCurrRequestSelfTest(void)
{
    _gLeakDetector.self_test_requested = true;
}

/**
 * @brief Clear fault and trigger re-test
 */
bool LeakCurrFaultClare(void)
{
    if (_gLeakDetector.current_state == EN_LEAK_CURR_STATE_FAULT)
    {
        _gLeakDetector.fault_cleared = true;
        _gLeakDetector.self_test_requested = true;
        return true;
    }
    return false;
}

/**
 * @brief Get self-test state: 0 idle, 1 pass, -1 fail
 */
int GetLeakCurrSelfTestState(void)
{
    return _gLeakDetector.result.self_test_passed;
}
bool GetLeakCurrFaultFlag(void)
{
    return  _gLeakDetector.result.fault_flag;
}
float GetLeakCurrValue(void)
{
    return  _gLeakDetector.result.rms_current_ma;
}
float GetLeakCurrFilterValue(void)
{
    return  _gLeakDetector.result.instant_curr_ma;
}

/**
 * @brief Leak current process, called from 20kHz ISR
 * @param leak_current_ma Raw leak current (mA)
 * @param samp_freq Sample frequency (Hz)
 * @param inverter_freq Inverter frequency (Hz)
 */
void LeakCurrProcess(float leak_current_ma, uint32_t samp_freq, uint32_t inverter_freq)
{
    bool     rms_value_update_flag = false;
    static bool first_run_flag = true;
    if(first_run_flag)
    {
        _LeakCurrInit();
        Mid_Filter1OrderButter_Init(&_gLowfilter_1order, LEAK_CURRENT_LPF_CUTOFF_HZ, LEAK_CURRENT_SAMPLE_FREQ_HZ);
        first_run_flag = false;
    }
    /* Handle fault clear request */
    if (_gLeakDetector.fault_cleared && _gLeakDetector.current_state == EN_LEAK_CURR_STATE_FAULT)
    {
        if(_gLeakDetector.result.self_test_passed == 1)
        {
            _gLeakDetector.current_state = EN_LEAK_CURR_STATE_RUN_DELAY;
            _gLeakDetector.result.fault_flag = false;
        }
        else
        {
            _LeakCurrInit();
        }
        _gLeakDetector.fault_cleared = false;
        return;
    }
    /* Handle self-test request */
    if (_gLeakDetector.self_test_requested && _gLeakDetector.current_state != EN_LEAK_CURR_STATE_PRE_CHK &&
        _gLeakDetector.current_state != EN_LEAK_CURR_STATE_SELF_CHK)
    {
        _LeakCurrInit();
        _gLeakDetector.self_test_requested = false;
        return;
    }

    /* 500Hz first-order Butterworth LPF */
    _gLeakDetector.result.instant_curr_ma = Mid_Filter1OrderButter_Update(&_gLowfilter_1order, leak_current_ma);
    /* Samples per AC cycle, clamp inverter freq to 30~70Hz, default 50Hz */
    _gLeakDetector.cycle_sample_cont = samp_freq / ((inverter_freq > 0) ? ((inverter_freq < 30) ? 30 : ((inverter_freq > 70) ? 70 : inverter_freq)) :50);
    /* Calculate leak current RMS */
    rms_value_update_flag = _RmsCalculate(_gLeakDetector.result.instant_curr_ma,_gLeakDetector.cycle_sample_cont,&_gLeakDetector.result.rms_current_ma);

    if (_gLeakDetector.current_state == EN_LEAK_CURR_STATE_FAULT)
    {
        return;
    }

    if(rms_value_update_flag)
    {
        switch (_gLeakDetector.current_state)
        {
            case EN_LEAK_CURR_STATE_PRE_CHK:
                _LeakCurrPreChk(_gLeakDetector.result.rms_current_ma);
                break;

            case EN_LEAK_CURR_STATE_SELF_CHK:
                _LeakCurrSelfChk(_gLeakDetector.result.rms_current_ma);
                break;

            case EN_LEAK_CURR_STATE_RUN_DELAY:
                _LeakCurrInitDelay(LEAK_CURRENT_INIT_DELAY_TIME);
                break;

            case EN_LEAK_CURR_STATE_RUN_NOMAL:
                _LeakCurrNomalRun(_gLeakDetector.result.rms_current_ma);
                break;

            default:
                break;
        }
    }
}
```

### `device/rcmu.h`

```c
/*
 * rcmu.h
 *
 *  Created on: Oct 30, 2025
 *      Author: Nova006348
 */
#include "F28x_Project.h"
#include "system.h"
#include <string.h>
#include <math.h>
#ifndef APP_RCMU_H_
#define APP_RCMU_H_


/* RMS calculation macro */
#define LEAK_CURRENT_CALCULATE_RMS(sum_squares, sample_count) \
    ((sample_count) == 0 ? 0.0f : sqrtf((sum_squares) / (sample_count)))

/* History buffer update macro */
#define LEAK_CURRENT_UPDATE_HISTORY_VALUES(history, new_value) \
    do { \
        float *_hist = (history); \
        float _new_val = (new_value); \
        if (_hist != NULL) { \
            _hist[2] = _hist[1]; \
            _hist[1] = _hist[0]; \
            _hist[0] = _new_val; \
        } \
    } while(0)



/* System configuration macros */
#define LEAK_CURRENT_SELF_TEST_THRESHOLD_MA     30.0f  /* Pre-check threshold 30mA */
#define LEAK_CURRENT_SELF_TEST_RANGE_MIN_MA     40.0f  /* Self-test range min 40mA */
#define LEAK_CURRENT_SELF_TEST_RANGE_MAX_MA     60.0f  /* Self-test range max 60mA */
#define LEAK_CURRENT_SELF_TEST_FAULT_COUNT      5U     /* Self-test success count threshold */

#define LEAK_CURRENT_INIT_DELAY_TIME          250U      /* Init delay 5s (250 * 20ms) */

//#define LEAK_CURRENT_FAULT_THRESHOLD_THREE    120.0f /* Instant fault threshold 120mA */




#define LEAK_CURRENT_FAULT_THRESHOLD_TWO                 280.0f  /* Level-2 fault threshold ~300mA */
#define LEAK_CURRENT_TWO_LEVEL_FAULT_THRESHOLD           12U     /* Level-2 fault count threshold */

#define LEAK_CURRENT_SQUARE_DIFF_150MA_THRESHOLD        148.0f   /* 150mA step threshold */
#define LEAK_CURRENT_SQUARE_DIFF_150MA_COUNT           1U     /* 150mA step count threshold */

#define LEAK_CURRENT_SQUARE_DIFF_60MA_THRESHOLD        58.0f   /* 60mA step threshold */
#define LEAK_CURRENT_SQUARE_DIFF_60MA_COUNT     3U     /* 60mA step count threshold */

/* Fault count configuration */
#define LEAK_CURRENT_SQUARE_DIFF_30MA_THRESHOLD        27.0f /* 30mA step threshold */
#define LEAK_CURRENT_SQUARE_DIFF_30MA_COUNT     6U     /* 30mA step count threshold */

#define LEAK_CURRENT_LPF_CUTOFF_HZ      500.0f   /* LPF cutoff 500Hz */
#define LEAK_CURRENT_SAMPLE_FREQ_HZ     20000.0f /* Sample rate 20kHz */

/* State machine */
typedef enum
{
    EN_LEAK_CURR_STATE_PRE_CHK = 0,   /* Pre-check with test IO off */
    EN_LEAK_CURR_STATE_SELF_CHK,         /* Self-check with test IO on */
    EN_LEAK_CURR_STATE_RUN_DELAY,             /* Post-init delay state */
    EN_LEAK_CURR_STATE_RUN_NOMAL,       /* Normal run state */
    EN_LEAK_CURR_STATE_FAULT                   /* Fault state */
} EN_LEAK_CURR_STATE;

/* Leak current detection result */
typedef struct
{
    int16 self_test_passed;        /* Self-test result: 0 idle, 1 pass, -1 fail */
    bool fault_flag;      /* Fault flag */
    uint8_t leak_type;      /* Leak current type */
    float rms_current_ma;         /* Leak current RMS (mA) */
    float instant_curr_ma;
    float avg_rms_current_ma;     /* Average leak current RMS (mA) */
} LeakResult_t;

/* Leak current detector internal state */
typedef struct
{

    /* Self-test related */
    uint8_t self_test_fault_count;     /* Pre-check fault count */
    uint8_t self_test_success_count;   /* Self-check success count */

    EN_LEAK_CURR_STATE current_state;       /* RCMU run state */
    uint32_t cycle_sample_cont;        /* Samples per AC cycle */
    /* Fault detection related */
    uint8_t two_level_fault_count;          /* ~300mA threshold counter */
    uint8_t square_diff_150ma_count;         /* 150mA step counter */
    uint8_t square_diff_60ma_count;         /* 60mA step counter */
    uint8_t square_diff_30ma_count;         /* 30mA step counter */
    /* History storage */
    float history_150ma[3];            /* 0: current, 1: previous, 2: before previous */
    float history_60ma[3];            /* 0: current, 1: previous, 2: before previous */
    float history_30ma[3];            /* 0: current, 1: previous, 2: before previous */
    /* System flags */
    bool self_test_requested;
    bool fault_cleared;
    /* Output result */
    LeakResult_t result;
} LeakDetect_t;

void LeakCurrProcess(float leak_current_ma, uint32_t samp_freq, uint32_t inverter_freq);

void LeakCurrRequestSelfTest(void);  /* Request self-test */
bool LeakCurrFaultClare(void);        /* Clear fault and restart self-test */
int GetLeakCurrSelfTestState(void);
bool GetLeakCurrFaultFlag(void);
float GetLeakCurrValue(void);
float GetLeakCurrFilterValue(void);
#endif /* APP_RCMU_H_ */
```

### `device/relay_ctrl.c`

```c
/**
 * @file relay_ctrl.c
 * @brief Relay control and self-test
 */

#include "relay_ctrl.h"
#include "gpio.h"
#include <string.h>
#include "fault.h"
#include "debug_analysis.h"

static RelayInfo_t _gRelayCtrl[EN_RELAT_CTRL_TYPE_COUNT];
static RelaySelfTestInValue_t _gRelayTestInValue;
static RelaySelfTest_t _gRelaySelfTest;

/**
 * @brief Set relay hardware state and update open flag
 * @param relay_type Relay channel
 * @param state RELAY_ON or RELAY_OFF
 */
void RelayControl(EN_RELAT_CTRL_TYPE relay_type, uint8_t state)
{
    switch(relay_type)
    {
        case EN_RELAT_CTRL_TYPE_LOAD:
            if(state == RELAY_ON)
            {
                SetLoadRlyState(1);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_LOAD].open_flag = true;
            }
            else
            {
                SetLoadRlyState(0);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_LOAD].open_flag = false;
            }
            break;

        case EN_RELAT_CTRL_TYPE_GRID:
            if(state == RELAY_ON)
            {
                SetGridRlyState(1);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_GRID].open_flag = true;
            }
            else
            {
                SetGridRlyState(0);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_GRID].open_flag = false;
            }
            break;

        case EN_RELAT_CTRL_TYPE_INV:
            if(state == RELAY_ON)
            {
                SetInvMainRlyState(1);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_INV].open_flag = true;
            }
            else
            {
                SetInvMainRlyState(0);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_INV].open_flag = false;
            }
            break;

        case EN_RELAT_CTRL_TYPE_GEN:
            if(state == RELAY_ON)
            {
                SetGenRlyState(1);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_GEN].open_flag = true;
            }
            else
            {
                SetGenRlyState(0);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_GEN].open_flag = false;
            }
            break;

        case EN_RELAT_CTRL_TYPE_N_PE:
            if(state == RELAY_ON)
            {
                SetPERlyState(1);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_N_PE].open_flag = true;
            }
            else
            {
                SetPERlyState(0);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_N_PE].open_flag = false;
            }
            break;

        case EN_RELAT_CTRL_TYPE_GEN_START:
            if(state == RELAY_ON)
            {
                SetGenStarRlyState(1);
            }
            else
            {
                SetGenStarRlyState(0);
            }
            break;

        case EN_RELAT_CTRL_TYPE_EXC:
            if(state == RELAY_ON)
            {
                SetEXCRlyState(1);
            }
            else
            {
                SetEXCRlyState(0);
            }
            break;

        default:
            break;
    }
}

static void _RelaySelfTestInit(void)
{
    _gRelaySelfTest.retest_requested = false;
    _gRelaySelfTest.test_complete = 0;
    _gRelaySelfTest.selfTestState = STATE_IDLE;
}

/** Initialize all relays to OFF and reset self-test state */
void RelayControlInit(void)
{
    memset(_gRelayCtrl, 0, sizeof(_gRelayCtrl));
    for(uint8_t i = 0; i < EN_RELAT_CTRL_TYPE_COUNT; i++)
    {
        RelayControl((EN_RELAT_CTRL_TYPE)i, RELAY_OFF);
    }
//    RelayControl(EN_RELAT_CTRL_TYPE_GEN_START, RELAY_OFF);
//    RelayControl(EN_RELAT_CTRL_TYPE_EXC, RELAY_OFF);

    _RelaySelfTestInit();
}

RelaySelfTestInValue_t* GetRelaySelfInputValue(void)
{
    return &_gRelayTestInValue;
}

/**
 * @brief Relay stick / wiring self-test state machine
 * @note Call periodically from main loop
 */
void RelaySelfTest(void)
{
    static uint32_t timeout;
    static uint16_t cont = 0;
    static bool first_run_flag = true;

    if(first_run_flag)
    {
        timeout = TimGetSystime();
        first_run_flag = false;
    }

    if(_gRelaySelfTest.retest_requested && _gRelaySelfTest.selfTestState == RELAY_TEST_FAIL)
    {
        _RelaySelfTestInit();
        first_run_flag = true;
        return;
    }

    if(_gRelaySelfTest.test_complete != 0)
    {
        return;
    }

    switch(_gRelaySelfTest.selfTestState)
    {
        case STATE_IDLE:
        {
            if(TimDifftimeFromNow(timeout) < 200)
            {
                break;
            }

            if(_gRelayCtrl[EN_RELAT_CTRL_TYPE_GRID].open_flag)
            {
                RelayControl(EN_RELAT_CTRL_TYPE_GRID, RELAY_OFF);
            }
            if(_gRelayCtrl[EN_RELAT_CTRL_TYPE_GEN].open_flag)
            {
                RelayControl(EN_RELAT_CTRL_TYPE_GEN, RELAY_OFF);
            }
            RelayControl(EN_RELAT_CTRL_TYPE_LOAD, RELAY_OFF);

            timeout = TimGetSystime();
            _gRelaySelfTest.selfTestState = STATE_CHECK_GRID;
            cont = 2;
            break;
        }

        case STATE_CHECK_GRID:
        {
            if(TimDifftimeFromNow(timeout) < 1000)
            {
                break;
            }

            if(!_gRelayCtrl[EN_RELAT_CTRL_TYPE_GRID].open_flag && !_gRelayCtrl[EN_RELAT_CTRL_TYPE_GEN].open_flag)
            {
                if((VOLTAGE_IN_RANGE(_gRelayTestInValue.grid_a_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
                   && (VOLTAGE_IN_RANGE(_gRelayTestInValue.grid_b_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
                   && (VOLTAGE_IN_RANGE(_gRelayTestInValue.grid_c_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH)))
                {
                    /* Grid present: verify other nodes are de-energized before closing grid relay */
                    if(_gRelayTestInValue.mid_a_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.mid_b_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.mid_c_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.Load_a_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.Load_b_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.Load_c_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.Inv_a_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.Inv_b_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.Inv_c_vol_rms <= GRID_VOLT_RMS_LOW)
                    {
                        cont++;
                    }
                    else
                    {
                        if(cont > 0)
                        {
                            cont--;
                        }
                    }

                    if(cont >= 5)
                    {
                        RelayControl(EN_RELAT_CTRL_TYPE_GRID, RELAY_ON);
                        _gRelaySelfTest.selfTestState = STATE_RELAY_CLOSE_TEST;
                        timeout = TimGetSystime();
                        cont = 2;
                    }
                    else if(cont == 0)
                    {
                        _gRelaySelfTest.test_complete = -1;
                        _gRelaySelfTest.selfTestState = RELAY_TEST_FAIL;
                    }
                }
                else
                {
                    _gRelaySelfTest.test_complete = 1;
                    _gRelaySelfTest.selfTestState = STATE_IDLE;
                }
            }
            else if(_gRelayCtrl[EN_RELAT_CTRL_TYPE_GRID].open_flag || _gRelayCtrl[EN_RELAT_CTRL_TYPE_GEN].open_flag)
            {
                _gRelaySelfTest.test_complete = 1;
                _gRelaySelfTest.selfTestState = STATE_IDLE;
                break;
            }
            break;
        }

        case STATE_RELAY_CLOSE_TEST:
        {
            if(_gRelayCtrl[EN_RELAT_CTRL_TYPE_GEN].open_flag)
            {
                _gRelaySelfTest.test_complete = 1;
                _gRelaySelfTest.selfTestState = STATE_IDLE;
                RelayControl(EN_RELAT_CTRL_TYPE_GRID, RELAY_OFF);
                break;
            }

            if(TimDifftimeFromNow(timeout) < 1000)
            {
                break;
            }

            /* Mid-point instant voltage should track grid; other ports must stay de-energized */
            if((fabsf(_gRelayTestInValue.mid_a_vol_instant - _gRelayTestInValue.grid_a_vol_instant) < 20.0f)
               && (fabsf(_gRelayTestInValue.mid_b_vol_instant - _gRelayTestInValue.grid_b_vol_instant) < 20.0f)
               && (fabsf(_gRelayTestInValue.mid_c_vol_instant - _gRelayTestInValue.grid_c_vol_instant) < 20.0f)
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Gen_a_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Gen_b_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Gen_c_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Load_a_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Load_b_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Load_c_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Inv_a_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Inv_b_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Inv_c_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH)))
            {
                cont++;
            }
            else
            {
                if(cont > 0)
                {
                    cont--;
                }
            }

            if(cont >= 5)
            {
                _gRelaySelfTest.test_complete = 1;
                RelayControl(EN_RELAT_CTRL_TYPE_GRID, RELAY_OFF);
                _gRelaySelfTest.selfTestState = STATE_IDLE;
                cont = 0;
            }
            else if(cont == 0)
            {
                _gRelaySelfTest.test_complete = -1;
                _gRelaySelfTest.selfTestState = RELAY_TEST_FAIL;
            }
            break;
        }

        default:
            _gRelaySelfTest.selfTestState = STATE_IDLE;
            break;
    }
}

/** @return Self-test result: 0 = in progress, 1 = pass, -1 = fail */
int16 GetRelaySelfTestResult(void)
{
    return _gRelaySelfTest.test_complete;
}

int16 GetRelayState(EN_RELAT_CTRL_TYPE relay_type)
{
    if(relay_type >= EN_RELAT_CTRL_TYPE_COUNT)
    {
        return 0;
    }
    return _gRelayCtrl[relay_type].open_flag;
}

void RelayRetestRequestSet(bool value)
{
    _gRelaySelfTest.retest_requested = value;
}
```

### `device/relay_ctrl.h`

```c
/**
 * @file relay_ctrl.h
 * @brief Relay control and self-test
 */

#ifndef APP_RELAY_CTRL_H_
#define APP_RELAY_CTRL_H_

#include "F28x_Project.h"
#include "system.h"
#include "cpu_timers.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Relay channel index */
typedef enum
{
    EN_RELAT_CTRL_TYPE_LOAD = 0,
    EN_RELAT_CTRL_TYPE_GRID,
    EN_RELAT_CTRL_TYPE_INV,
    EN_RELAT_CTRL_TYPE_GEN,
    EN_RELAT_CTRL_TYPE_N_PE,
    EN_RELAT_CTRL_TYPE_COUNT,
    EN_RELAT_CTRL_TYPE_GEN_START,
    EN_RELAT_CTRL_TYPE_EXC
} EN_RELAT_CTRL_TYPE;

#define RELAY_OFF 0
#define RELAY_ON  1

#define RELAY_MAX_RETRY_CONT 2
#define VOLTAGE_IN_RANGE(voltage, min, max) (((voltage) >= (min)) && ((voltage) <= (max)) ? 1 : 0)

typedef struct
{
    bool open_flag;
} RelayInfo_t;

/** Relay self-test state machine */
typedef enum
{
    STATE_IDLE = 0,
    STATE_CHECK_GRID,
    STATE_RELAY_CLOSE_TEST,
    RELAY_TEST_FAIL
} RelaySelfTestState;

typedef struct
{
    RelaySelfTestState selfTestState;
    int16 test_complete;
    bool retest_requested;
} RelaySelfTest_t;

/** Sampled voltages used during relay self-test */
typedef struct
{
    float mid_a_vol_rms;
    float mid_b_vol_rms;
    float mid_c_vol_rms;
    float mid_a_vol_instant;
    float mid_b_vol_instant;
    float mid_c_vol_instant;
    float grid_a_vol_rms;
    float grid_b_vol_rms;
    float grid_c_vol_rms;
    float grid_a_vol_instant;
    float grid_b_vol_instant;
    float grid_c_vol_instant;
    float Load_a_vol_rms;
    float Load_b_vol_rms;
    float Load_c_vol_rms;
    float Gen_a_vol_rms;
    float Gen_b_vol_rms;
    float Gen_c_vol_rms;
    float Inv_a_vol_rms;
    float Inv_b_vol_rms;
    float Inv_c_vol_rms;
} RelaySelfTestInValue_t;

void RelayControlInit(void);
void RelayControl(EN_RELAT_CTRL_TYPE relay_type, uint8_t state);
RelaySelfTestInValue_t* GetRelaySelfInputValue(void);
void RelaySelfTest(void);
void RelayRetestRequestSet(bool value);
int16 GetRelaySelfTestResult(void);
int16 GetRelayState(EN_RELAT_CTRL_TYPE relay_type);

#ifdef __cplusplus
}
#endif

#endif /* APP_RELAY_CTRL_H_ */
```

