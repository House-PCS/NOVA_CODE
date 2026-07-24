##------------------------------------------------------------------------------
##Project Name      :
##Moudle Name       :   RockDcCtrlTop_Pins.xdc
##Original Author   :   lvyb
##Creation Date     :   2024.9.1
##Description       :   创建RockDcCtrlTop管脚分配�?
##
##
##------------------------------------------------------------------------------
##Version           :   Rev 0.1
##Modifier          :
##Modify Date       :
##Modify Record     :
##
##
##------------------------------------------------------------------------------
##Version           :   Rev 0.2
##Modifier          :
##Modify Date       :
##Modify Record     :
##
##
##------------------------------------------------------------------------------

##------------------------------------------------------------------------------
#Global

set_property PACKAGE_PIN K18 [get_ports iSysClkIn]
set_property PACKAGE_PIN G21 [get_ports oSysLed]

set_property IOSTANDARD LVCMOS33 [get_ports iSysClkIn]
set_property IOSTANDARD LVCMOS33 [get_ports oSysLed]

##------------------------------------------------------------------------------

##------------------------------------------------------------------------------
#Phy Mdio

#PhyRstN
set_property PACKAGE_PIN M22 [get_ports {oEhtPhyRstN[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oEhtPhyRstN[0]}]

set_property PACKAGE_PIN M18 [get_ports {oEhtPhyRstN[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oEhtPhyRstN[1]}]

#Mdclk
set_property PACKAGE_PIN L18 [get_ports oEthPhyMdclk]
set_property IOSTANDARD LVCMOS33 [get_ports oEthPhyMdclk]

#Mdio
set_property PACKAGE_PIN N18 [get_ports ioEthPhyMdio]
set_property IOSTANDARD LVCMOS33 [get_ports ioEthPhyMdio]

##------------------------------------------------------------------------------

##------------------------------------------------------------------------------
#Arm Interface

#Led
set_property PACKAGE_PIN G22 [get_ports oArmLed]
set_property IOSTANDARD LVCMOS33 [get_ports oArmLed]
set_property PACKAGE_PIN V22 [get_ports {oRGBOut[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oRGBOut[0]}]
set_property PACKAGE_PIN T21 [get_ports {oRGBOut[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oRGBOut[1]}]

#Uart
set_property PACKAGE_PIN Y22 [get_ports iArmUartRx]
set_property IOSTANDARD LVCMOS33 [get_ports iArmUartRx]

set_property PACKAGE_PIN Y21 [get_ports oArmUartTx]
set_property IOSTANDARD LVCMOS33 [get_ports oArmUartTx]
set_property PACKAGE_PIN M2 [get_ports oArmUartDbgTx]
set_property IOSTANDARD LVCMOS33 [get_ports oArmUartDbgTx]

set_property PACKAGE_PIN J4 [get_ports oUartTxd]
set_property IOSTANDARD LVCMOS33 [get_ports oUartTxd]
set_property PULLUP true [get_ports oUartTxd]

#
set_property PACKAGE_PIN W21 [get_ports iDmosAndCmosEn]
set_property IOSTANDARD LVCMOS33 [get_ports iDmosAndCmosEn]

#SWD
# #UartRx
set_property PACKAGE_PIN G4 [get_ports iSwdClk]
set_property IOSTANDARD LVCMOS33 [get_ports iSwdClk]
# #UartTx
set_property PACKAGE_PIN G3 [get_ports ioSwdData]
set_property IOSTANDARD LVCMOS33 [get_ports ioSwdData]
set_property PULLUP true [get_ports ioSwdData]

set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets iSwdClk_IBUF]

##------------------------------------------------------------------------------

##------------------------------------------------------------------------------
#

# set_property PACKAGE_PIN E19 [get_ports oSpiClk]
# set_property IOSTANDARD LVCMOS33 [get_ports oSpiClk]

set_property PACKAGE_PIN T19 [get_ports oSpiCs]
set_property IOSTANDARD LVCMOS33 [get_ports oSpiCs]

set_property PACKAGE_PIN P22 [get_ports oSpiMosi]
set_property IOSTANDARD LVCMOS33 [get_ports oSpiMosi]

set_property PACKAGE_PIN R22 [get_ports iSpiMiso]
set_property IOSTANDARD LVCMOS33 [get_ports iSpiMiso]

#set_property BITSTREAM.CONFIG.CONFIGRATE 50 [current_design]
##------------------------------------------------------------------------------

##------------------------------------------------------------------------------
#Ethnet

set_property PACKAGE_PIN U20 [get_ports {oRmiiClk[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oRmiiClk[0]}]
set_property SLEW FAST [get_ports {oRmiiClk[0]}]

set_property PACKAGE_PIN W19 [get_ports {oRmiiClk[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oRmiiClk[1]}]
set_property SLEW FAST [get_ports {oRmiiClk[1]}]


set_property PACKAGE_PIN U17 [get_ports {oRmiiTxEn[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oRmiiTxEn[0]}]
set_property SLEW FAST [get_ports {oRmiiTxEn[0]}]

set_property PACKAGE_PIN R16 [get_ports {oRmiiTxEn[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oRmiiTxEn[1]}]
set_property SLEW FAST [get_ports {oRmiiTxEn[1]}]

set_property PACKAGE_PIN R14 [get_ports {iRmiiRxDv[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iRmiiRxDv[0]}]

set_property PACKAGE_PIN P16 [get_ports {iRmiiRxDv[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iRmiiRxDv[1]}]

set_property PACKAGE_PIN U18 [get_ports {oRmiiTxData[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oRmiiTxData[0]}]
set_property SLEW FAST [get_ports {oRmiiTxData[0]}]

set_property PACKAGE_PIN P14 [get_ports {oRmiiTxData[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oRmiiTxData[1]}]
set_property SLEW FAST [get_ports {oRmiiTxData[1]}]


set_property PACKAGE_PIN N13 [get_ports {oRmiiTxData[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oRmiiTxData[2]}]
set_property SLEW FAST [get_ports {oRmiiTxData[2]}]

set_property PACKAGE_PIN N14 [get_ports {oRmiiTxData[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oRmiiTxData[3]}]
set_property SLEW FAST [get_ports {oRmiiTxData[3]}]

set_property PACKAGE_PIN R18 [get_ports {iRmiiRxData[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iRmiiRxData[0]}]

set_property PACKAGE_PIN T18 [get_ports {iRmiiRxData[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iRmiiRxData[1]}]

set_property PACKAGE_PIN R17 [get_ports {iRmiiRxData[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iRmiiRxData[2]}]

set_property PACKAGE_PIN N15 [get_ports {iRmiiRxData[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iRmiiRxData[3]}]

##------------------------------------------------------------------------------

##------------------------------------------------------------------------------
#Protection PcsID
# T20 0---Protection
# R19 0---Protection

set_property PACKAGE_PIN T20 [get_ports iLlcOverCurrent]
set_property PACKAGE_PIN R19 [get_ports iLlcOverVoltage]
set_property PACKAGE_PIN P19 [get_ports {iPcsHardwareId[1]}]
set_property PACKAGE_PIN U21 [get_ports {iPcsHardwareId[0]}]

set_property IOSTANDARD LVCMOS33 [get_ports iLlcOverCurrent]
set_property IOSTANDARD LVCMOS33 [get_ports iLlcOverVoltage]
set_property IOSTANDARD LVCMOS33 [get_ports {iPcsHardwareId[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iPcsHardwareId[0]}]

##------------------------------------------------------------------------------


##------------------------------------------------------------------------------
#PwmOut

set_property PACKAGE_PIN F15 [get_ports oLLC1LVPwm1OutP]
set_property PACKAGE_PIN E17 [get_ports oLLC1LVPwm2OutP]
set_property PACKAGE_PIN D16 [get_ports oLLC2LVPwm1OutP]
set_property PACKAGE_PIN F14 [get_ports oLLC2LVPwm2OutP]

set_property PACKAGE_PIN A16 [get_ports oLLC1HVPwm1OutP]
set_property PACKAGE_PIN C15 [get_ports oLLC1HVPwm2OutP]
set_property PACKAGE_PIN A15 [get_ports oLLC2HVPwm1OutP]
set_property PACKAGE_PIN A14 [get_ports oLLC2HVPwm2OutP]

set_property PACKAGE_PIN A13 [get_ports oBuckBoost1PwmOutP]
set_property PACKAGE_PIN B15 [get_ports oBuckBoost1PwmOutN]
set_property PACKAGE_PIN B16 [get_ports oBuckBoost2PwmOutP]
set_property PACKAGE_PIN B17 [get_ports oBuckBoost2PwmOutN]

set_property IOSTANDARD LVCMOS33 [get_ports oBuckBoost1PwmOutP]
set_property IOSTANDARD LVCMOS33 [get_ports oBuckBoost1PwmOutN]
set_property IOSTANDARD LVCMOS33 [get_ports oBuckBoost2PwmOutP]
set_property IOSTANDARD LVCMOS33 [get_ports oBuckBoost2PwmOutN]

set_property IOSTANDARD LVCMOS33 [get_ports oLLC1LVPwm1OutP]
set_property IOSTANDARD LVCMOS33 [get_ports oLLC1LVPwm2OutP]
set_property IOSTANDARD LVCMOS33 [get_ports oLLC2LVPwm1OutP]
set_property IOSTANDARD LVCMOS33 [get_ports oLLC2LVPwm2OutP]

set_property IOSTANDARD LVCMOS33 [get_ports oLLC1HVPwm1OutP]
set_property IOSTANDARD LVCMOS33 [get_ports oLLC1HVPwm2OutP]
set_property IOSTANDARD LVCMOS33 [get_ports oLLC2HVPwm1OutP]
set_property IOSTANDARD LVCMOS33 [get_ports oLLC2HVPwm2OutP]

set_property SLEW FAST [get_ports oBuckBoost1PwmOutP]
set_property SLEW FAST [get_ports oBuckBoost1PwmOutN]
set_property SLEW FAST [get_ports oBuckBoost2PwmOutP]
set_property SLEW FAST [get_ports oBuckBoost2PwmOutN]

set_property SLEW FAST [get_ports oLLC1LVPwm1OutP]
set_property SLEW FAST [get_ports oLLC1LVPwm2OutP]
set_property SLEW FAST [get_ports oLLC2LVPwm1OutP]
set_property SLEW FAST [get_ports oLLC2LVPwm2OutP]

set_property SLEW FAST [get_ports oLLC1HVPwm1OutP]
set_property SLEW FAST [get_ports oLLC1HVPwm2OutP]
set_property SLEW FAST [get_ports oLLC2HVPwm1OutP]
set_property SLEW FAST [get_ports oLLC2HVPwm2OutP]




set_property PACKAGE_PIN C14 [get_ports {oDrvOutEn[2]}]
set_property PACKAGE_PIN D15 [get_ports {oDrvOutEn[1]}]
set_property PACKAGE_PIN E14 [get_ports {oDrvOutEn[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oDrvOutEn[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oDrvOutEn[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oDrvOutEn[2]}]

##------------------------------------------------------------------------------

##------------------------------------------------------------------------------
##RelayCtrl


#set_property PACKAGE_PIN E13 [get_ports {oPrechargeRelayDis[1]}]
#set_property PACKAGE_PIN C13 [get_ports {oMainRelayEn[1]]  #Group2_OUT_GPIO3  Relay_Ctrl_OUT1  JK1


# set_property PACKAGE_PIN B22 [get_ports  oMainRelayEn ]        # Group2_OUT_GPIO3  Relay_Ctrl_OUT1  JK1
# set_property PACKAGE_PIN B21 [get_ports  oPrechargeRelayDis ]  # Group2_OUT_GPIO4  Relay_Ctrl_OUT2  JK2
# set_property PACKAGE_PIN A20 [get_ports  oParallelRelayEn ]    # Group2_OUT_GPIO5  Relay_Ctrl_OUT3  JK3
# set_property PACKAGE_PIN F16 [get_ports  oSeriesRelayEn ]      # Group2_OUT_GPIO6  Relay_Ctrl_OUT4  JK4

set_property PACKAGE_PIN B22 [get_ports oMainRelayEn]
set_property PACKAGE_PIN B21 [get_ports oPrechargeRelayDis]
set_property PACKAGE_PIN A20 [get_ports oParallelRelayEn]
set_property PACKAGE_PIN F16 [get_ports oSeriesRelayEn]

set_property IOSTANDARD LVCMOS33 [get_ports oMainRelayEn]
set_property IOSTANDARD LVCMOS33 [get_ports oPrechargeRelayDis]
set_property IOSTANDARD LVCMOS33 [get_ports oParallelRelayEn]
set_property IOSTANDARD LVCMOS33 [get_ports oSeriesRelayEn]

##------------------------------------------------------------------------------


##------------------------------------------------------------------------------
##AdcSample

set_property IOSTANDARD LVCMOS33 [get_ports iVP]
set_property IOSTANDARD LVCMOS33 [get_ports iVN]

set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[4]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[5]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[6]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[7]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[8]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[9]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[10]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[11]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[12]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[13]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[14]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxN[15]}]

set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[4]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[5]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[6]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[7]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[8]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[9]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[10]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[11]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[12]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[13]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[14]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iVauxP[15]}]


set_property PACKAGE_PIN C13 [get_ports {oTempSel[0]}]
set_property PACKAGE_PIN E13 [get_ports {oTempSel[1]}]
set_property PACKAGE_PIN F13 [get_ports {oTempSel[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oTempSel[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oTempSel[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {oTempSel[2]}]
##------------------------------------------------------------------------------


##------------------------------------------------------------------------------
#Fan

set_property PACKAGE_PIN D22 [get_ports oFanPwmOutP]
set_property IOSTANDARD LVCMOS33 [get_ports oFanPwmOutP]

set_property PACKAGE_PIN D14 [get_ports oCaptureInputEn]
set_property PACKAGE_PIN C20 [get_ports iFanPwmFedBack]

set_property IOSTANDARD LVCMOS33 [get_ports oCaptureInputEn]
set_property IOSTANDARD LVCMOS33 [get_ports iFanPwmFedBack]

##------------------------------------------------------------------------------
#Debug
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
set_property BITSTREAM.GENERAL.CRC ENABLE [current_design]
set_property BITSTREAM.CONFIG.CONFIGFALLBACK ENABLE [current_design]
set_property BITSTREAM.CONFIG.CONFIGRATE 50 [current_design]
set_property CONFIG_MODE SPIx1 [current_design]
# set_property PACKAGE_PIN F20 [get_ports oFanPwmOutN]
# set_property IOSTANDARD LVCMOS33 [get_ports oFanPwmOutN]

# set_property PACKAGE_PIN B22 [get_ports {oRelayCtrl[0]}]
# set_property IOSTANDARD LVCMOS33 [get_ports {oRelayCtrl[0]}]

# set_property PACKAGE_PIN B21 [get_ports {oRelayCtrl[1]}]
# set_property IOSTANDARD LVCMOS33 [get_ports {oRelayCtrl[1]}]

# set_property PACKAGE_PIN A20 [get_ports {oRelayCtrl[2]}]
# set_property IOSTANDARD LVCMOS33 [get_ports {oRelayCtrl[2]}]

# set_property PACKAGE_PIN F16 [get_ports {oRelayCtrl[3]}]
# set_property IOSTANDARD LVCMOS33 [get_ports {oRelayCtrl[3]}]

#set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]

##------------------------------------------------------------------------------

# create_clock -period 20.000 -name wSysClk1 -waveform {0.000 10.000} -add [get_nets -hierarchical *wSysClk1*]
# create_clock -period 10.000 -name wSysClk2 -waveform {0.000 5.000} -add [get_nets -hierarchical *wSysClk2*]
# create_clock -period 5.000 -name wSysClk3 -waveform {0.000 2.500} -add [get_nets -hierarchical *wSysClk3*]
# create_clock -period 50.000 -name wSysClk4 -waveform {0.000 25.000} -add [get_nets -hierarchical *wSysClk4*]
# create_clock -period 20.000 -name wSysClk5 -waveform {10.000 0.000} -add [get_nets -hierarchical *wSysClk5*]


# set_property OFFCHIP_TERM NONE [get_ports oDdrCasN]
# set_property OFFCHIP_TERM NONE [get_ports oDdrCke]
# set_property OFFCHIP_TERM NONE [get_ports oDdrOdt]
# set_property OFFCHIP_TERM NONE [get_ports oDdrRasN]
# set_property OFFCHIP_TERM NONE [get_ports oDdrRstN]
# set_property OFFCHIP_TERM NONE [get_ports oDdrWeN]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[15]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[14]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[13]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[12]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[11]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[10]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[9]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[8]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[7]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[6]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[5]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[4]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[3]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[2]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[1]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDq[0]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDqsP[1]]
# set_property OFFCHIP_TERM NONE [get_ports ioDdrDqsP[0]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[12]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[11]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[10]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[9]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[8]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[7]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[6]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[5]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[4]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[3]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[2]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[1]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrAddr[0]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrBa[2]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrBa[1]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrBa[0]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrDm[1]]
# set_property OFFCHIP_TERM NONE [get_ports oDdrDm[0]]
# set_property OFFCHIP_TERM NONE [get_ports oTempSel[1]]
# set_property OFFCHIP_TERM NONE [get_ports oTempSel[0]]
# set_property DRIVE 12 [get_ports {oDrvOutEn[2]}]
# set_property DRIVE 12 [get_ports {oDrvOutEn[1]}]
# set_property DRIVE 12 [get_ports {oDrvOutEn[0]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[15]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[14]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[13]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[12]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[11]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[10]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[9]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[8]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[7]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[6]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[5]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[4]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[3]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[2]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[1]}]
# set_property IOSTANDARD SSTL15 [get_ports {ioDdrDq[0]}]
# set_property IOSTANDARD SSTL15 [get_ports oDdrRstN]
# set_property SLEW SLOW [get_ports oDdrCasN]
# set_property SLEW SLOW [get_ports oDdrCke]
# set_property SLEW SLOW [get_ports oDdrOdt]
# set_property SLEW SLOW [get_ports oDdrRstN]
# set_property SLEW FAST [get_ports oDdrClkP]
# set_property SLEW FAST [get_ports oDdrClkN]
# set_property SLEW SLOW [get_ports oDdrWeN]
# set_property SLEW SLOW [get_ports {ioDdrDq[15]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[14]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[13]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[12]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[11]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[10]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[9]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[8]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[7]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[6]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[5]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[4]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[3]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[2]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[1]}]
# set_property SLEW SLOW [get_ports {ioDdrDq[0]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[12]}]
# set_property SLEW FAST [get_ports {ioDdrDqsP[1]}]
# set_property SLEW FAST [get_ports {ioDdrDqsN[1]}]
# set_property SLEW FAST [get_ports {ioDdrDqsP[0]}]
# set_property SLEW FAST [get_ports {ioDdrDqsN[0]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[11]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[9]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[10]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[8]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[7]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[6]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[5]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[4]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[3]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[2]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[1]}]
# set_property SLEW SLOW [get_ports {oDdrAddr[0]}]
# set_property SLEW SLOW [get_ports {oDdrBa[2]}]
# set_property SLEW SLOW [get_ports {oDdrBa[1]}]
# set_property SLEW SLOW [get_ports {oDdrBa[0]}]
# set_property SLEW SLOW [get_ports {oDdrDm[1]}]
# set_property SLEW SLOW [get_ports {oDdrDm[0]}]
# set_property SLEW SLOW [get_ports oDdrRasN]

