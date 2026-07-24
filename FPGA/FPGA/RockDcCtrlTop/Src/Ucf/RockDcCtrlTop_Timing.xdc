##------------------------------------------------------------------------------
##Project Name      :
##Moudle Name       :   RockDcCtrlTop_Pins.xdc
##Original Author   :   lvyb
##Creation Date     :   2024.9.1
##Description       :   创建RockDcCtrlTop时序约束。
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
#create_clock

create_clock -period 20.000 -name iSysClkIn -waveform {0.000 10.000} -add [get_ports iSysClkIn]


#create_clock -period 20.000 -name wSysClk1 -waveform {0.000 10.000} -add [get_nets -hierarchical *wSysClk1*]
#create_clock -period 10.000 -name wSysClk2 -waveform {0.000 5.000} -add [get_nets -hierarchical *wSysClk2*]
#create_clock -period 5.000 -name wSysClk3 -waveform {0.000 2.500} -add [get_nets -hierarchical *wSysClk3*]
# create_clock -period 50.000 -name wSysClk4 -waveform {0.000 25.000} -add [get_nets -hierarchical *wSysClk4*]
#create_clock -period 20.000 -name wSysClk5 -waveform {10.000 20.000} -add [get_nets -hierarchical *wSysClk5*]
# set_property C_CLK_INPUT_FREQ_HZ 300000000 [get_debug_cores dbg_hub]
# set_property C_ENABLE_CLK_DIVIDER false [get_debug_cores dbg_hub]
# set_property C_USER_SCAN_CHAIN 1 [get_debug_cores dbg_hub]
# connect_debug_port dbg_hub/clk [get_nets wSysClk4]
##------------------------------------------------------------------------------

##------------------------------------------------------------------------------

##------------------------------------------------------------------------------





