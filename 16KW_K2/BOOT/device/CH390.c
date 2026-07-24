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
    uint8_t nsr = ch390_read_reg(CH390_NSR);//发送01
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
    /*5S等待*/
    for(i = 0; i < 50; i++)
    {
        DELAY_MS(100);
        if(!ch390_get_link_status())
        {
            ch390_default_config();
        }
        else
        {
            printf("link up\r\n");
            ch390_write_reg(CH390_ISR, ISR_LNKCHG);
            return true;
        }
    }

    return false;
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
