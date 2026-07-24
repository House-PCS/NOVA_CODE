
#include "dev_phy.h"





bool DevPhy8201Init(void)
{
//    SimSmiBspInit();
    SmiRstLow();
    //DelayMs(200);
    
    Delay(200000);

    SmiRstHigh();
    //SoftDelayms(200);

    Delay(200000);

    if (ConfigRtl8201(0x00) == false)
    {
        return false;
    }
        
    return ConfigRtl8201(0x01);
}

/**
* PHY LED 模式配置
**/
static void _ConfigRtl8201Led(uint8_t phy_addr)
{
    uint16_t regtmp = 0xFFFF;
    _SmiSetReg(phy_addr, 31, 7);
//    regtmp = _SmiReadReg(phy_addr, 17);

    regtmp = 0x0038;
    _SmiSetReg(phy_addr, 17, regtmp);
    regtmp = _SmiReadReg(phy_addr, 19);
    regtmp |= 0x0008;
//    regtmp &= 0xFFCF;
    _SmiSetReg(phy_addr, 19, regtmp);
    _SmiSetReg(phy_addr, 31, 0);
}

/**
  * @brief  : 配置为100M双工模式
  * @param  : *phy :phy device
  * @return : true success,false fail
  * @note   : None
  */
bool ConfigRtl8201(uint8_t phy_addr)
{
    uint16_t regtmp = 0;

    _SmiSetReg(phy_addr, 0, 0xA100);    /*强制百兆模式*/
    //DelayMs(100);
    Delay(200000);

    _SmiSetReg(phy_addr, 0, 0x2100);    /*强制百兆模式*/
    _SmiSetReg(phy_addr, 31, 7);
    _SmiSetReg(phy_addr, 16, 0x1FFE);
    _SmiSetReg(phy_addr, 31, 0);
    _ConfigRtl8201Led(phy_addr); 
	
    //DelayMs(20);

     Delay(200);

    /*寄存器回读 检测是否配置好*/
    regtmp = _SmiReadReg(phy_addr, 0);
    if(regtmp != 0xFFFF)
    {
        return true;
    }
    return false;
}

 /**
 * @brief 读取PHY设备的寄存器值
 * 
 * @param phyAddr PHY设备地址
 * @param regAddr 寄存器地址
 * @return uint16_t 寄存器值
 */
#if 0
static uint16_t _SmiReadReg(uint8_t phyAddr, uint8_t regAddr)
{
    uint32_t temp = 0;
    uint8_t data[2] = {0};
    
    // 切换MDIO为推挽输出模式
//    SmiSetMdioOutput(&gstSimSmiHandle);

    // 前导码：32bit 1b    
    temp = 0xFFFFFFFF;
    gstSimSmiHandle.pTxData = (uint8_t*)&temp;
    gstSimSmiHandle.tx_bit_size = 32;
    SmiWriteBits(&gstSimSmiHandle);

    // 帧起始：2bit 01b
    temp = 0x01 << 6;
    gstSimSmiHandle.pTxData = (uint8_t*)&temp;
    gstSimSmiHandle.tx_bit_size = 2;
    SmiWriteBits(&gstSimSmiHandle);

    // 操作：2bit 10b（读取）
    temp = 0x02 << 6;
    gstSimSmiHandle.pTxData = (uint8_t*)&temp;
    gstSimSmiHandle.tx_bit_size = 2;
    SmiWriteBits(&gstSimSmiHandle);

    // PHY地址（5bit）+ 寄存器地址（5bit）    
    data[0] = (phyAddr << 3);
    data[0] |= ((regAddr & 0x1C) >> 2);
    data[1] = ((regAddr & 0x3) << 6);

    gstSimSmiHandle.pTxData = data;
    gstSimSmiHandle.tx_bit_size = 10;
    SmiWriteBits(&gstSimSmiHandle);

    // 操作：2bit 10b（读取）
    temp = 0x02 << 6;
    gstSimSmiHandle.pTxData = (uint8_t*)&temp;
    gstSimSmiHandle.tx_bit_size = 2;
    SmiWriteBits(&gstSimSmiHandle);
    
    // 读取数据，16bit
    gstSimSmiHandle.pRxData = data;
    gstSimSmiHandle.rx_bit_size = 16;
    SmiReadBits(&gstSimSmiHandle);

    SmiIdleState(&gstSimSmiHandle);

    return (data[0] << 8) | (data[1]);
}
#else
static uint16_t _SmiReadReg(uint8_t phyAddr, uint8_t regAddr)
{
    uint32_t temp = 0;
    uint8_t data[2] = {0};
    
    /*切换MDIO为推挽输出模式**********************************************************/
    MdioSet2Output();

    // 前导码：32bit 1b    
    temp = 0xFFFFFFFF;
//    gstSimSmiHandle.pTxData = (uint8_t*)&temp;
//    gstSimSmiHandle.tx_bit_size = 32;
    SmiWriteBits((uint8_t*)&temp, 32);

    // 帧起始：2bit 01b
    temp = 0x01 << 6;
//    gstSimSmiHandle.pTxData = (uint8_t*)&temp;
//    gstSimSmiHandle.tx_bit_size = 2;
    SmiWriteBits((uint8_t*)&temp, 2);

    // 操作：2bit 10b（读取）
    temp = 0x02 << 6;
//    gstSimSmiHandle.pTxData = (uint8_t*)&temp;
//    gstSimSmiHandle.tx_bit_size = 2;
    SmiWriteBits((uint8_t*)&temp, 2);

    // PHY地址（5bit）+ 寄存器地址（5bit）    
    data[0] = (phyAddr << 3);
    data[0] |= ((regAddr & 0x1C) >> 2);
    data[1] = ((regAddr & 0x3) << 6);

//    gstSimSmiHandle.pTxData = data;
//    gstSimSmiHandle.tx_bit_size = 10;
    SmiWriteBits((uint8_t*)&data, 10);

//    // 操作：2bit 10b（读取）
//    temp = 0x02 << 6;
//    gstSimSmiHandle.pTxData = (uint8_t*)&temp;
//    gstSimSmiHandle.tx_bit_size = 2;
//    SmiWriteBits(&gstSimSmiHandle);
    
    if(SmiSendTa())
    {
        // 读取数据，16bit
//        gstSimSmiHandle.pRxData = data;
//        gstSimSmiHandle.rx_bit_size = 16;
        SmiReadBits((uint8_t*)&data, 16);
    }
    else
    {
        *data = 0xFF;
        *(data + 1) = 0xFF;
    }

    SmiIdleState();

    return (data[1] << 8) | (data[0]);
}
#endif
/**
 * @brief 设置PHY设备的寄存器值
 * 
 * @param phyAddr PHY设备地址
 * @param regAddr 寄存器地址
 * @param regVal 寄存器值
 */
static void _SmiSetReg(uint8_t phyAddr, uint8_t regAddr, uint16_t regVal)
{
    uint32_t temp = 0;
    uint8_t data[2] = {0};
    
//    // 切换MDIO为推挽输出模式
//    SmiSetMdioOutput(&gstSimSmiHandle);

    // 前导码：32bit 1b    
    temp = 0xFFFFFFFF;
//    gstSimSmiHandle.pTxData = (uint8_t*)&temp;
//    gstSimSmiHandle.tx_bit_size = 32;
    SmiWriteBits((uint8_t*)&temp, 32);

    // 帧起始：2bit 01b
    temp = 0x01 << 6;
//    gstSimSmiHandle.pTxData = (uint8_t*)&temp;
//    gstSimSmiHandle.tx_bit_size = 2;
    SmiWriteBits((uint8_t*)&temp, 2);

    // 操作：2bit 01b
    temp = 0x01 << 6;
//    gstSimSmiHandle.pTxData = (uint8_t*)&temp;
//    gstSimSmiHandle.tx_bit_size = 2;
    SmiWriteBits((uint8_t*)&temp, 2);

    // PHY地址（5bit）+ 寄存器地址（5bit）    
    data[0] = (phyAddr << 3);
    data[0] |= ((regAddr & 0x1C) >> 2);
    data[1] = ((regAddr & 0x3) << 6);

//    gstSimSmiHandle.pTxData = data;
//    gstSimSmiHandle.tx_bit_size = 10;
    SmiWriteBits((uint8_t*)&data, 10);

    // 设置MDIO的TA状态，2bit，10b
    temp = 0x02 << 6;
//    gstSimSmiHandle.pTxData = (uint8_t*)&temp;
//    gstSimSmiHandle.tx_bit_size = 2;
    SmiWriteBits((uint8_t*)&temp, 2);

    // 设置，16bit
    data[0] = regVal >> 8;
    data[1] = regVal & 0xFF;
//    gstSimSmiHandle.pTxData = data;
//    gstSimSmiHandle.tx_bit_size = 16;
    SmiWriteBits((uint8_t*)&data, 16);

    //SmiIdleState((uint8_t*)&data, 16);
    SmiIdleState();
//    // 切换MDIO为上拉输入模式，驱动MDIO为高阻态
//    SmiSetMdioInput(&gstSimSmiHandle);
}
