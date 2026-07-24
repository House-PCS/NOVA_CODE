#ifndef __DEV_PHY_H_
#define __DEV_PHY_H_

#include "mdio.h"




bool ConfigRtl8201(uint8_t phy_addr);
bool DevPhy8201Init(void);
static void _ConfigRtl8201Led(uint8_t phy_addr);
static void _SmiSetReg(uint8_t phyAddr, uint8_t regAddr, uint16_t regVal);
static uint16_t _SmiReadReg(uint8_t phyAddr, uint8_t regAddr);


#endif


