//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   RockDcCtrlTop.v
//Original Author   :   lvyb
//Creation Date     :   
/*Description       :

*/
//------------------------------------------------------------------------------
//Version           :   Rev 0.1
//Modifier          :   
//Modify Date       :   
/*Modify Record     :   

*/
//------------------------------------------------------------------------------
//Version           :   Rev 0.2
//Modifier          :   
//Modify Date       :   
/*Modify Record     :   

*/  
//------------------------------------------------------------------------------
`timescale 1ns/1ns
module RockDcCtrlTop
#(
    parameter                       RMII_CH             = 2                 
)
(
    //Global Clock
    input                           iSysClkIn                               ,
    //Rmii Port
    input       [(RMII_CH - 1):0]   iRmiiRxDv                               ,   //CRS/CRS_DV
    input       [(RMII_CH*2 - 1):0] iRmiiRxData                             ,
    output      [(RMII_CH - 1):0]   oRmiiTxEn                               ,
    output      [(RMII_CH*2 - 1):0] oRmiiTxData                             ,
    output      [(RMII_CH - 1):0]   oRmiiClk                                ,
    //Arm Interface
    output                          oArmLed                                 ,
    output      [1:0]               oEhtPhyRstN                             ,
    output                          oEthPhyMdclk                            ,
    inout                           ioEthPhyMdio                            ,
    //Uart
    input                           iArmUartRx                              ,
    output                          oArmUartTx                              ,
    //Uart2/
    input                           iDmosAndCmosEn                          ,
    //UartDebug
    output                          oArmUartDbgTx                           ,

    //SPI
    //output                          oSpiClk                                 ,
    output                          oSpiCs                                  ,
    output                          oSpiMosi                                ,
    input                           iSpiMiso                                ,
    //DDR
    inout       [15:0]              ioDdrDq                                 ,
    inout       [1:0]               ioDdrDqsN                               ,
    inout       [1:0]               ioDdrDqsP                               ,
    output      [12:0]              oDdrAddr                                ,
    output      [2:0]               oDdrBa                                  ,
    output                          oDdrRasN                                ,
    output                          oDdrCasN                                ,
    output                          oDdrWeN                                 ,
    output                          oDdrRstN                                ,
    output                          oDdrClkP                                ,
    output                          oDdrClkN                                ,
    output                          oDdrCke                                 ,
    output      [1:0]               oDdrDm                                  ,
    output                          oDdrOdt                                 , 
    //SWD
    input                           iSwdClk                                 ,
    inout                           ioSwdData                               ,

    //Pwm
    output                          oBuckBoost1PwmOutP                        , // A13
    output                          oBuckBoost1PwmOutN                        , // B15
    output                          oBuckBoost2PwmOutP                        , // B16
    output                          oBuckBoost2PwmOutN                        , // B17 

    output                          oLLC1LVPwm1OutP                           , //F15  LLC1_LV_H0L1   G1-G1
    output                          oLLC1LVPwm2OutP                           , //E17  LLC1_LV_H1L0   G1-G2
    output                          oLLC2LVPwm1OutP                           , //D16  LLC2_LV_H0L1   G1-G3
    output                          oLLC2LVPwm2OutP                           , //F14  LLC2_LV_H1L0   G1-G4

    output                          oLLC1HVPwm1OutP                           , //A16  LLC1_HV_H0L1   G3-G2
    output                          oLLC1HVPwm2OutP                           , //C15  LLC1_HV_H1L0   G3-G3
    output                          oLLC2HVPwm1OutP                           , //A15  LLC2_HV_H0L1   G1-G8
    output                          oLLC2HVPwm2OutP                           , //A14  LLC2_HV_H1L0   G1-G7

    output      [2:0]               oDrvOutEn                                 , 
    //Debug
    output                          oCaptureInputEn                           , //D14
    output                          oFanPwmOutP                               , 
    input                           iFanPwmFedBack                            , //C20

    // output                          oFanPwmOutN                             ,
    // output      [3:0]               oRelayCtrl                              ,


    //AdcSample

    input                           iVP                                     ,
    input                           iVN                                     ,
    input       [15:0]              iVauxP                                  ,
    input       [15:0]              iVauxN                                  ,
    output      [2:0]               oTempSel                                ,

    output                          oSeriesRelayEn                          ,
    output                          oParallelRelayEn                        ,
    output                          oMainRelayEn                            ,
    output                          oPrechargeRelayDis                      ,
    
    //Hardware Protection
    input                           iLlcOverCurrent                         , //T20 1---Protection 180A
    input                           iLlcOverVoltage                         , //R19 1---Protection

    //PCS ID 
    input       [1:0]               iPcsHardwareId                          , //P19  U21   默认:00  01    单相：10   三相：11

    //User Interface
    output                          oSysLed                                 ,
    output      [1:0]               oRGBOut                                 ,
    output                          oUartTxd     
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//


//******************************************************************************

//******************************************************************************
//

    wire                            wSysClk1                                ;       //50MHz
    wire                            wSysClk2                                ;       //100MHz
    wire                            wSysClk3                                ;       //200MHz
    wire                            wSysClk4                                ;       //20MHz
    wire                            wSysClk5                                ;       //50MMHz180deg
    wire                            wPllLocked                              ;
    wire                            wPllLocked1                             ;
    wire                            wPllLocked2                             ;
    wire                            wSysClkSSC200M                          ;
    wire                            wSysClkSSC50M                           ;

    wire                            wSysClk1RstP                            ;
    wire                            wSysClk2RstP                            ;
    wire                            wSysClk2RstN                            ;
    wire                            wSysClk3RstP                            ;
    wire                            wSysClk4RstP                            ;

    wire                            wSysClk1Delay1Us                        ;
    wire                            wSysClk1Delay1Ms                        ;
    wire                            wSysClk1Delay1S                         ;
    wire                            wSysClk2Delay1Us                        ;
    wire                            wSysClk2Delay1Ms                        ;
    wire                            wSysClk2Delay1S                         ;
    wire                            wSysClk3Delay1Us                        ;
    wire                            wSysClk3Delay1Ms                        ;
    wire                            wSysClk3Delay1S                         ;
    wire                            wSysClk4Delay1Us                        ;
    wire                            wSysClk4Delay1Ms                        ;
    wire                            wSysClk4Delay1S                         ;

    wire        [(RMII_CH - 1):0]   wLogicRxDv                              ;
    wire        [(RMII_CH*8 - 1):0] wLogicRxData                            ;
    wire        [(RMII_CH - 1):0]   wGmiiRxDv                               ;
    wire        [(RMII_CH*8 - 1):0] wGmiiRxData                             ;
    wire        [(RMII_CH - 1):0]   wLogicTxEn                              ;
    wire        [(RMII_CH*8 - 1):0] wLogicTxData                            ;

    wire        [(RMII_CH - 1):0]   wGmiiTxEn                               ;
    wire        [(RMII_CH*8 - 1):0] wGmiiTxData                             ;

    wire                            wEthClk                                 ;
    wire                            wEthTxDataRdEn                          ;
    wire        [7:0]               wEthRxData                              ;
    wire                            wEthRxDataVld                           ;
    wire        [7:0]               wEthTxData                              ;
    wire        [15:0]              wEthTxLen                               ;
    wire                            wEthTxDataReady                         ;

    wire        [7:0]               wBmsParamBusRdData                      ;
    wire                            wBmsParamBusClk                         ;
    wire        [31:0]              wBmsParamBusAddr                        ;

    //wire        [(2*32 - 1):0]      wFpgaParam                              ;
    wire                            wFpgaParamBusClk                        ;
    wire        [31:0]              wFpgaParamBusAddr                       ;
    wire                            wFpgaParamBusWrEn                       ;
    wire        [15:0]              wFpgaParamBusWrData                     ;
    wire        [15:0]              wFpgaParamBusRdData                     ;
    wire signed [31:0]              wBatVoltage                             ;
    wire signed [31:0]              wBatVoltageAverage                      ;
    wire        [31:0]              wBatCurrent                             ;
    wire        [15:0]              wBatOutputPower                         ;
    wire        [15:0]              wBatSoc                                 ;
    wire        [31:0]              wCurrentMaxChgCurrent                   ; 
    wire        [31:0]              wCurrentMaxDsgCurrent                   ;
    wire        [31:0]              wBmsShutdown                            ;
    wire        [15:0]              wDCOutputTotalVoltage                   ;
    wire        [15:0]              wDCHeatSinkTemp1                        ;
    wire        [15:0]              wDCHeatSinkTemp2                        ;
    wire        [15:0]              wDCFailureInformation1                  ;
    wire        [15:0]              wDCFailureInformation2                  ;
    wire        [15:0]              wDCFailureInformation3                  ;
    wire        [15:0]              wDCAlarmInformation1                    ;
    wire        [15:0]              wFan1Speed                              ;
    wire                            wFan1Err                                ;
    wire        [15:0]              wFan2Speed                              ;
    wire        [15:0]              wLlc1OutputVoltage                      ;
    wire        [15:0]              wLlc1OutputCurrent                      ;
    wire        [15:0]              wLlc2OutputVoltage                      ;
    wire        [15:0]              wLlc2OutputCurrent                      ;
    wire        [15:0]              wBuckBoost1OutputVoltage                ;
    wire        [15:0]              wBuckBoost1OutputCurrent                ;
    wire        [15:0]              wBuckBoost2OutputVoltage                ;
    wire        [15:0]              wBuckBoost2OutputCurrent                ;
    wire        [15:0]              wDcdcWorkStatus                         ;

    wire                            wArmUartTx                              ;

    wire                            wMCURebootEn                            ;

    wire                            wBoardSelf                              ;

    wire                            wSwitchOnOff                            ;
    wire                            wForwardEn                              ;
    wire         [31:0]             wBoardIp                                ;
    wire                            wDCOnOff                                ;
    wire                            wEqualEnableCharge                      ;
    wire                            wEqualEnableDischarge                   ;
    wire         [1:0]              wPcsSoftWareId                          ;
    wire                            wClearOfFault                           ;
    wire         [15:0]             wUpdataStatus                           ;
    wire         [15:0]             wEqualizationEnable                     ;
    wire         [7:0]              wLedDir                                 ;
    wire         [7:0]              wLedRValue                              ;
    wire         [7:0]              wLedGValue                              ;
    wire         [7:0]              wLedBValue                              ;
    wire         [15:0]             wLedMode                                ;
    wire         [15:0]             wLedPeriod                              ;

    wire                            wBoardIpUpdata                          ;
    wire                            wEthUdpRstN                             ;

    wire          [7:0]             wManualReboot                           ;
    wire                            wFanForceEn                             ;
    wire                            wMcuFlashBurnEn                         ;
    wire       [25:0]               wMcuRdStartAddr                         ;
    wire                            wMcuFaultBurnFinish                     ;
    wire                            wBuckBoostPwmEn                         ;
    wire                            wLLCPwmEn                               ;
    wire        [9:0]               wBuckBoost1PwmDuty                      ;
    wire        [9:0]               wBuckBoost2PwmDuty                      ;
    wire        [9:0]               wLLCPwmDuty                             ;

    wire                            wSysStable                              ;
    wire                            wEhtPhyRstN                             ;
    wire                            wDdrWrOpClk0                            ;
    wire                            wDdrWrOpClk1                            ;
    wire                            wDdrWrOpBurstBegin0                     ;
    wire        [27:0]              wDdrWrOpAddr0                           ;
    wire        [127:0]             wDdrWrOpData0                           ;
    wire        [15:0]              wDdrWrOpDataMask0                       ;

    wire                            wDdrWrOpBurstBegin1                     ;
    wire                            wDdrWrOpBurstRdy1                       ;
    wire        [   4: 0]           wDdrWrBufBaseAddr1                      ;
    wire                            wDdrWrBufSel1                           ;
    wire        [   8: 0]           wDdrWrOpLen1                            ;
    wire        [  25: 0]           wDdrWrOpAddr1                           ;
    wire        [   8: 0]           wDdrWrDataAddr1                         ;
    wire                            wDdrWrDataAddrEn1                       ;
    wire        [  15: 0]           wDdrWrOpDataMask1                       ;
    wire        [ 127: 0]           wDdrWrOpData1                           ;
    wire                            wDdrLogicClk                            ;
    wire                            wDdrRdOpClk0                             ;
    wire                            wDdrRdOpBurstBegin0                     ;
    wire        [27:0]              wDdrRdOpAddr0                           ;
    wire        [127:0]             wDdrRdOpData0                           ;
    wire                            wDdrRdOpDataVld0                        ;

    wire                            wDdrInitDone                            ;
    wire                            wDdrWrOpDone                            ;

    //ADC
    wire        [11:0]              wFpgaTemp                               ;
    wire    signed    [15:0]        wLlcBusVoltageA                         ;
    wire    signed    [15:0]        wLlcBusVoltageB                         ;
    wire    signed    [15:0]        wBusVoltageA                            ;
    wire    signed    [15:0]        wBusVoltageB                            ;
    wire    signed    [15:0]        wBusTotalVoltage                        ;
    wire    signed    [15:0]        wBusTotalVoltagePre                     ;
    wire    signed    [15:0]        wResonanceCurrentA                      ;
    wire    signed    [15:0]        wResonanceCurrentB                      ;
    wire    signed    [15:0]        wLlcBusCurrentA                         ;
    wire    signed    [15:0]        wLlcBusCurrentB                         ;
    wire    signed    [15:0]        wBusTotalCurrentA                       ;
    wire    signed    [15:0]        wBusTotalCurrentB                       ;
    wire    signed    [15:0]        wVoltageSampVref                        ;
    wire    signed    [15:0]        wCurrentSampVref                        ;
    wire    signed    [15:0]        wTemp1                                  ;
    wire    signed    [15:0]        wTemp2                                  ;
    wire    signed    [15:0]        wTemp3                                  ;
    wire    signed    [15:0]        wTemp4                                  ;
    wire    signed    [15:0]        wTemp5                                  ;
    wire    signed    [15:0]        wTemp6                                  ;
    wire    signed    [ 7:0]        wNtcTemp1                               ;
    wire    signed    [ 7:0]        wNtcTemp3                               ;
    
    wire                            wAdcSampleSequenceDone                  ;
    wire                            wErrRecovery                            ;
    //Relay
    wire              [1:0]         wPcsUsingId                             ;
    wire                            wWorkingEn                              ;
    wire                            wProtectEn                              ;
    wire                            wDcdcErr                                ;
    wire                            wMainRelayEn                            ;
    wire                            wPrechargeRelayDis                      ;
    wire        [32*36-1:0]         wAdcData                                ;
    wire        [23:0]              wPidControlPwmParamA                    ;   
    wire        [23:0]              wPidControlPwmParamB                    ;   
    wire                            wStateLlcEn                             ;
    wire                            wStateLlcOk                             ;
    wire                            wStateLoopEn                            ;

    wire                            wSelfInspectionEn                       ;
    wire                            wSelfInspectionEnd                      ;

    wire        [1:0]               wCurrentState                           ;

    wire signed [31:0]              wGiRefA                                 ;
    wire signed [31:0]              wGvIA                                   ;
    wire signed [31:0]              wGiRefB                                 ;
    wire signed [31:0]              wGvIB                                   ;
   //wire signed [31:0] wDFilterA;
   //wire signed [31:0] wDFilterB;

    wire signed [31:0]              wLoadR                                  ;
    wire signed [31:0]              wBusVolFilterA                          ;
    wire signed [31:0]              wBusVolFilterB                          ;
    wire signed [31:0]              wLlcBusCurFilterA                       ;
    wire signed [31:0]              wLlcBusCurFilterB                       ;
    wire signed [31:0]              wBusTotalCurFilterA                     ;
    wire signed [31:0]              wBusTotalCurFilterB                     ;
    wire signed [31:0]              wLLCVolilter                            ;

    wire signed [31:0]             wLlcBusVolFilterA                       ;
    wire signed [31:0]             wLlcBusVolFilterB                       ;
    wire signed [31:0]             wBusTotalVolFilter                      ;
    wire signed [31:0]             wBusTotalVolFilterPre                   ;

    wire signed [31:0]              wBatVoltageFilter                      ;
    wire signed [31:0]              wBatVoltageAverageFilter               ;

    wire signed [31:0]              wState                                  ;
    wire signed [31:0]              wLlcBusVoltageA32                       ;
    wire signed [31:0]              wLlcBusVoltageB32                       ;
    wire signed [31:0]              wBusVoltageA32                          ;
    wire signed [31:0]              wBusVoltageB32                          ;
    wire signed [31:0]              wBusTotalVoltage32                      ;
    wire signed [31:0]              wBusTotalVoltagePre32                   ;
    wire signed [31:0]              wResonanceCurrentA32                    ;
    wire signed [31:0]              wResonanceCurrentB32                    ;
    wire signed [31:0]              wLlcBusCurrentA32                       ;
    wire signed [31:0]              wLlcBusCurrentB32                       ;
    wire signed [31:0]              wBusTotalCurrentA32                     ;
    wire signed [31:0]              wBusTotalCurrentB32                     ;
    wire signed [31:0]              wProtectDbgMsg                          ;

    wire signed [31:0]              wTargetVoltageOffsetA                   ;
    wire signed [31:0]              wTargetVoltageOffsetB                   ;
    wire signed [31:0]              wActiveBalanceVoltageOffset             ;
    wire signed [31:0]              wTargetCurrentParamMax                  ;
    wire signed [31:0]              wTargetCurrentParamMin                  ;
    wire        [1:0]               wErrDisplay                             ;
    wire                            wNoEthNet                               ;
    wire        [1:0]               wSrcStable                              ;
    //SelfInspection
    wire        [1:0]               wPcsHardwareId                          ;
    wire       [11:0]               wVoltageSampVrefAvera                   ;
    wire       [11:0]               wCurrentSampVrefAvera                   ;

    //SlowLaunch
    wire                            wMainRelaySwitchOnEnd                   ;
    //DCDC Power Clac
    wire signed [15:0]              wDCOutputTotalCurrent                   ;
    wire signed [15:0]              wDCOutputTotalPower                     ;
    wire signed [31:0]              wDCOutputTotalPowerPre                  ;

    assign wLlcBusVoltageA32 = wLlcBusVoltageA;
    assign wLlcBusVoltageB32 = wLlcBusVoltageB;
    assign wBusVoltageA32 = wBusVoltageA;
    assign wBusVoltageB32 = wBusVoltageB;
    assign wBusTotalVoltage32 = wBusTotalVoltage;
    assign wBusTotalVoltagePre32 = wBusTotalVoltagePre;
    assign wResonanceCurrentA32 = wResonanceCurrentA;
    assign wResonanceCurrentB32 = wResonanceCurrentB;
    assign wLlcBusCurrentA32 = wLlcBusCurrentA;
    assign wLlcBusCurrentB32 = wLlcBusCurrentB;
    assign wBusTotalCurrentA32 = wBusTotalCurrentA;
    assign wBusTotalCurrentB32 = wBusTotalCurrentB;
    // assign wLoadR = wBusVoltageBTrans * 32'sd100000 / wBusTotalCurrentBTrans;
    assign wState[31:0] = 
    {
        1'b0,
        wSysClk2RstP,
        wCurrentState[1],
        wCurrentState[0],

        wSelfInspectionEn,
        wSelfInspectionEnd,
        wStateLlcEn, 
        wStateLlcOk,

        wWorkingEn,
        wProtectEn,
        wNoEthNet,
        1'b0,

        wDcdcErr,
        iDmosAndCmosEn,
        wBmsShutdown[3],
        wBmsShutdown[2],

        1'b0,
        1'b0,
        wEqualEnableCharge,
        wEqualEnableDischarge,

        1'b0,
        iDmosAndCmosEn,
        iLlcOverVoltage,
        iLlcOverCurrent,

        iPcsHardwareId[1],
        iPcsHardwareId[0],
        oMainRelayEn,
        oParallelRelayEn,

        oSeriesRelayEn,
        wStateLoopEn,
        wStateLlcOk,
        wStateLlcEn
    };

    assign wAdcData[32* 0    +:32]    =   wLlcBusVoltageA32     [31:0]; 
    assign wAdcData[32* 1    +:32]    =   wLlcBusVoltageB32     [31:0];
    assign wAdcData[32* 2    +:32]    =   wBusVoltageA32        [31:0];
    assign wAdcData[32* 3    +:32]    =   wBusVoltageB32        [31:0];
    assign wAdcData[32* 4    +:32]    =   wBusTotalVolFilter    [31:0];
    assign wAdcData[32* 5    +:32]    =   wBusTotalVoltagePre32 [31:0];
    assign wAdcData[32* 6    +:32]    =   wResonanceCurrentA32  [31:0];
    assign wAdcData[32* 7    +:32]    =   wResonanceCurrentB32  [31:0];
    assign wAdcData[32* 8    +:32]    =   wLlcBusCurrentA32     [31:0];
    assign wAdcData[32* 9    +:32]    =   wLlcBusCurrentB32     [31:0];
    assign wAdcData[32*10    +:32]    =   wBusTotalCurrentA32   [31:0]; 
    assign wAdcData[32*11    +:32]    =   wBusTotalCurrentB32   [31:0]; 
    assign wAdcData[32*12    +:32]    =   wBatVoltageAverage    [31:0];
    assign wAdcData[32*13    +:32]    =   wProtectDbgMsg[31:0];
    assign wAdcData[32*14    +:32]     =   {31'd0,wDcdcErr};
    assign wAdcData[32*15    +:32]     =   {31'd0,wErrRecovery};
    assign wAdcData[32*16    +:32]    =   wGiRefA[31:0];
    assign wAdcData[32*17    +:32]    =   wGvIA[31:0];
    assign wAdcData[32*18    +:32]    =   wGiRefB[31:0];
    assign wAdcData[32*19    +:32]    =   wGvIB[31:0];
    assign wAdcData[32*20    +:32]    =   {22'd0,wBuckBoost1PwmDuty[9:0]};
    assign wAdcData[32*21    +:32]    =   {22'd0,wBuckBoost2PwmDuty[9:0]};
    assign wAdcData[32*22    +:32]    =   {30'd0,wCurrentState[1:0]};
    assign wAdcData[32*23    +:32]    =   {22'd0,wLLCPwmDuty[9:0]};
    assign wAdcData[32*24    +:32]    =   wState[31:0];
    assign wAdcData[32*25    +:32]    =   wLlcBusCurFilterB[31:0];
    assign wAdcData[32*26    +:32]    =   wBusTotalCurFilterA[31:0];
    assign wAdcData[32*27    +:32]    =   wBusTotalCurFilterB[31:0];
    assign wAdcData[32*28    +:32]    =   wBatVoltage[31:0];
    assign wAdcData[32*29    +:32]    =   wBatCurrent[31:0];
    assign wAdcData[32*30    +:32]    =   {16'd0,wBatOutputPower[15:0]};
    assign wAdcData[32*31    +:32]    =   {16'd0,wBatSoc[15:0]};
    assign wAdcData[32*32    +:32]    =   wCurrentMaxChgCurrent[31:0];
    assign wAdcData[32*33    +:32]    =   wCurrentMaxDsgCurrent[31:0];
    assign wAdcData[32*34    +:32]    =   {{24{wNtcTemp1[7]}},wNtcTemp1[7:0]};
    assign wAdcData[32*35    +:32]    =   {{24{wNtcTemp3[7]}},wNtcTemp3[7:0]};


    // wire    wSlowLaunchEnd              ;
    // wire    wSlowLaunchEn               ;

//******************************************************************************

//******************************************************************************
//

SysPllCore U_SysPllCore
(
    .clk_in1                        (iSysClkIn                              ),  //50MHz
    .clk_out1                       (wSysClk1                               ),  //50MHz
    .clk_out2                       (wSysClk2                               ),  //100MHz
    .clk_out3                       (wSysClk3                               ),  //200MHz
    .clk_out4                       (wSysClk4                               ),  //20MHz
    .clk_out5                       (wSysClk5                               ),  //50MHz180Deg
    .locked                         (wPllLocked1                            ) 
);

SSCPllCore U_SSCPllCore
(
    .clk_in1                        (iSysClkIn                               ),
    .clk_out1                       (wSysClkSSC200M                          ),  // 200MHz
    .clk_out2                       (wSysClkSSC50M                           ),  // 200MHz
    .locked                         (wPllLocked2                             )  // output locked
);      

assign wPllLocked = wPllLocked2 & wPllLocked1;
//******************************************************************************

//******************************************************************************
DelayTimeCore #(
    .CLK_FRQ (50)
)
SysClk1_DelayTimeCore(
    .iSysClk    (wSysClk1        ),
    .oDelay1Us  (wSysClk1Delay1Us),
    .oDelay1Ms  (wSysClk1Delay1Ms),
    .oDelay1S   (wSysClk1Delay1S )                        
);

DelayTimeCore #(
    .CLK_FRQ (100)
)
SysClk2_DelayTimeCore(
    .iSysClk    (wSysClk2        ),
    .oDelay1Us  (wSysClk2Delay1Us),
    .oDelay1Ms  (wSysClk2Delay1Ms),
    .oDelay1S   (wSysClk2Delay1S )                        
);

DelayTimeCore #(
    .CLK_FRQ (200)
)
SysClk3_DelayTimeCore(
    .iSysClk    (wSysClk3        ),
    .oDelay1Us  (wSysClk3Delay1Us),
    .oDelay1Ms  (wSysClk3Delay1Ms),
    .oDelay1S   (wSysClk3Delay1S )                        
);

DelayTimeCore #(
    .CLK_FRQ (20)
)
SysClk4_DelayTimeCore(
    .iSysClk    (wSysClk4        ),
    .oDelay1Us  (wSysClk4Delay1Us),
    .oDelay1Ms  (wSysClk4Delay1Ms),
    .oDelay1S   (wSysClk4Delay1S )                        
);
//******************************************************************************

//******************************************************************************
FilterCore #(
 .CLK_FREQ                       (8'd100                                 )    
)U_FilterCore(
    .iSysClk                        (wSysClk2                               ),
    .iAdcSampleSequenceDone         (wAdcSampleSequenceDone                 ),
    .iLlcBusVoltageA                ($signed(wLlcBusVoltageA     )          ),
    .iLlcBusVoltageB                ($signed(wLlcBusVoltageB     )          ),
    .iBusTotalVoltage               ($signed(wBusTotalVoltage    )          ),
    .iBusTotalVoltagePre            ($signed(wBusTotalVoltagePre )          ),
    .iBusVoltageA32                 (wBusVoltageA32                         ),
    .iBusVoltageB32                 (wBusVoltageB32                         ),
    .iLlcBusCurrentA32              (wLlcBusCurrentA32                      ),
    .iLlcBusCurrentB32              (wLlcBusCurrentB32                      ),
    .iBusTotalCurrentA32            (wBusTotalCurrentA32                    ),
    .iBusTotalCurrentB32            (wBusTotalCurrentB32                    ),

    .oLlcBusVolFilterA              (wLlcBusVolFilterA                      ),
    .oLlcBusVolFilterB              (wLlcBusVolFilterB                      ),
    .oBusTotalVolFilter             (wBusTotalVolFilter                     ),
    .oBusTotalVolFilterPre          (wBusTotalVolFilterPre                  ),
    .oBusVolFilterA                 (wBusVolFilterA                         ),
    .oBusVolFilterB                 (wBusVolFilterB                         ),
    .oLlcBusCurFilterA              (wLlcBusCurFilterA                      ),
    .oLlcBusCurFilterB              (wLlcBusCurFilterB                      ),
    .oBusTotalCurFilterA            (wBusTotalCurFilterA                    ),
    .oBusTotalCurFilterB            (wBusTotalCurFilterB                    )
);

LowSpeedFilterCore #(
    .CLK_FREQ                       (8'd100                                 )        
)(
    .iSysClk                        (wSysClk2                               ),
    .iSysClk1sPulse                 (wSysClk2Delay1S                        ),
    .iBatVoltage                    (wBatVoltage                            ),
    .iBatAverageVoltage             (wBatVoltageAverage                     ),

    .oBatVoltage                    (wBatVoltageFilter                      ),
    .oBatAverageVoltage             (wBatVoltageAverageFilter               )     
);

//******************************************************************************

//******************************************************************************
//
SysStatusDisplay#(
    .CLK_FREQ                       (50                                     )
)U_SysStatusDisplay
(
    //Global Clock
    .iSysClk                        (wSysClk1                               ),
    .iNoEthNet                      (wNoEthNet                              ), 
    .iErrDisplay                    (wErrDisplay                            ),
    .oSrcStable                     (wSrcStable[1:0]                        )    
);


SysCore #(
    .CLK_FREQ1                       (50                                    ),
    .CLK_FREQ2                       (100                                   ),
    .CLK_FREQ3                       (200                                   ),
    .CLK_FREQ4                       (20                                    )
)U_SysCore 
(
    .iClk1                          (wSysClk1                               ),
    .iClk2                          (wSysClk2                               ),
    .iClk3                          (wSysClk3                               ),
    .iClk4                          (wSysClk4                               ),
    .iClk1Cnt1Us                    (wSysClk1Delay1Us                       ),
    .iClk2Cnt1Us                    (wSysClk2Delay1Us                       ),
    .iClk3Cnt1Us                    (wSysClk3Delay1Us                       ),
    .iClk4Cnt1Us                    (wSysClk4Delay1Us                       ),
    .iClk1Cnt1Ms                    (wSysClk1Delay1Ms                       ),
    .iPllLocked                     (wPllLocked                             ),
    .iSysStable                     (wSrcStable[1:0]                        ),
    .oClk1RstP                      (wSysClk1RstP                           ),
    .oClk2RstP                      (wSysClk2RstP                           ),
    .oClk2RstN                      (wSysClk2RstN                           ),
    .oClk3RstP                      (wSysClk3RstP                           ),
    .oClk4RstP                      (wSysClk4RstP                           ),
    .oSysLed                        (oSysLed                                )
);

//******************************************************************************

//******************************************************************************
//

wire                            wSpiClk                                 ;

STARTUPE2 U_STARTUPE2
(
    .GTS                            (0                                      ),
    .USRCCLKO                       (wSpiClk                                ),
    .USRCCLKTS                      (0                                      )
);

//******************************************************************************

//******************************************************************************

assign wBoardSelf = 1'b1;

RmiiDrvCore#(
    .RMII_CH                        (RMII_CH                                )
)U_RmiiDrvCore
(
    .iSysClk                        (wSysClk2                               ),  //100MHz
    .iBoardSelf                     (wBoardSelf                             ),  //1 indicates that this card is selected for packet return; 0 indicates that the data from the downstream board card is transmitted directly.
    .iForwardEn                     (wForwardEn                             ),  //1 indicates receiving instructions from the upstream and forwarding them to the downstream boards; 0 indicates receiving instructions from the upstream but not forwarding them to the downstream boards.
    .iRmiiClk                       (wSysClk1                               ),  //50MHz
    .iRmiiClk180Deg                 (wSysClk5                               ),  //50MHz180Deg
    .oRmiiClk                       (oRmiiClk[(RMII_CH - 1):0]              ),  //50Mhz@180Deg
    
    .iRmiiRxDv                      (iRmiiRxDv[(RMII_CH - 1):0]             ),  ////CRS/CRS_DV
    .iRmiiRxData                    (iRmiiRxData[(RMII_CH*2 - 1):0]         ),  //
    .oGmiiRxDv                      (wGmiiRxDv[(RMII_CH - 1):0]             ),
    .oGmiiRxData                    (wGmiiRxData[(RMII_CH*8 - 1):0]         ),
    
    .iGmiiTxEn                      (wGmiiTxEn[(RMII_CH - 1):0]             ),
    .iGmiiTxData                    (wGmiiTxData[(RMII_CH*8 - 1):0]         ),
    .oRmiiTxEn                      (oRmiiTxEn[(RMII_CH - 1):0]             ),  //
    .oRmiiTxData                    (oRmiiTxData[(RMII_CH*2 - 1):0]         )   //
);
 
//******************************************************************************

//******************************************************************************
//

assign wGmiiTxEn[1*1+:1]    = 1'd0  ;
assign wGmiiTxData[1*8+:8]  = 8'd0  ;

//******************************************************************************

//******************************************************************************
//

EthProtocalAnalysisCore U_EthProtocalAnalysisCore
(
    .sys_rst                        (wSysClk2RstP                           ),  
    .EthUdpRst                      (!wEthUdpRstN                            ),
    .eth_rxc                        (wSysClk2                               ),  //i
    .eth_rx_ctl                     (wGmiiRxDv[0*1+:1]                      ),  //i
    .eth_rxd                        (wGmiiRxData[0*8+:8]                    ),  //i
    .eth_tx_ctl                     (wGmiiTxEn[0*1+:1]                      ),  //o
    .eth_txd                        (wGmiiTxData[0*8+:8]                    ),  //o
    .eth_rst_n                      (                                       ),  //o

    .iBoardIp                       (wBoardIp[31:0]                         ),
    .iBoardIpUpdata                 (wBoardIpUpdata                         ),

    .iEthTxDataReady                (wEthTxDataReady                        ),  //
    .iEthTxDataLen                  (wEthTxLen[15:0]                        ),  //
    .iEthTxData                     (wEthTxData[7:0]                        ),  //
    .oEthTxDataRdEn                 (wEthTxDataRdEn                         ),
    .oEthClk                        (wEthClk                                ),
    .oEthRxData                     (wEthRxData[7:0]                        ),
    .oEthRxDataVld                  (wEthRxDataVld                          ) 
);

wire wUartTxEn;
wire wUartTxBusy;
wire [7:0] wUartTxData;

uart_send U_UartSend(
    .iSysClk                        (wEthClk                               ),
    .iSysRst                        (wSysClk2RstP                          ),

    .iUartEn                        (wUartTxEn                             ),
    .iUartDin                       (wUartTxData[7:0]                      ),
    .oUartTxBusy                    (wUartTxBusy                           ),
    .oUartTxd                       (oUartTxd                              )
);

DebugTest U_DebugTest(
    .iSysClk(wEthClk),
    .iEthTxRdEn(wUartTxBusy),
    .iBuf(wAdcData),
    .oEthTxEn(wUartTxEn),
    .oEthTxData(wUartTxData[7:0]),
    .oEthTxLen( )
);

//******************************************************************************


NetworkMonitorCore#(
    .CLK_FREQ                   ( 100                   )   ,  //MHz
    .NETWORK_MONITOR_TIME       ( 120                   )      //s 10-8191               
)U_NetworkMonitorCore
(
    //Global Clock
    .iSysClk                    (wSysClk2                       ),
    .iForwardEn                 (wForwardEn                     ),
    .iDataValid                 (iRmiiRxDv[(RMII_CH - 1):0]     ),
    .iCnt1S                     (wSysClk2Delay1S                ),
    .oNoEthNet                  (wNoEthNet                      )
);
//******************************************************************************
//

CortexMCore 
#(
    .CLK_FREQ                       (50                                     )
)U_CortexMCore
(
    .iSysClk                        (wSysClk1                               ),  //50MHz
    .iSysRst                        (wSysClk1RstP                                ),  

    .iEthClk                        (wEthClk                                ),  //100MHz
    .iEthRxDataVld                  (wEthRxDataVld                          ),
    .iEthRxData                     (wEthRxData[7:0]                        ),

    .oEthTxDataReady                (wEthTxDataReady                        ),
    .oEthTxLen                      (wEthTxLen[15:0]                        ),
    .iEthTxDataRdEn                 (wEthTxDataRdEn                         ),
    .oEthTxData                     (wEthTxData[7:0]                        ),

    .iBmsParamBusClk                (wBmsParamBusClk                        ),
    .iBmsParamBusAddr               (wBmsParamBusAddr[31:0]                 ),
    .oBmsParamBusRdData             (wBmsParamBusRdData[7:0]                ),

    //.iFpgaParam                     (wFpgaParam[(2*32 - 1):0]               ),
    .iFpgaParamBusClk               (wFpgaParamBusClk                       ),  //wSysClk2              100MHz
    .iFpgaParamBusAddr              (wFpgaParamBusAddr[31:0]                ),
    .iFpgaParamBusWrEn              (wFpgaParamBusWrEn                      ),
    .iFpgaParamBusWrData            (wFpgaParamBusWrData[15:0]              ),
    .oFpgaParamBusRdData            (wFpgaParamBusRdData[15:0]              ),

    .oArmLed                        (oArmLed                                ),
    .oEhtPhyRstN                    (wEhtPhyRstN                            ),
    .oEthPhyMdclk                   (oEthPhyMdclk                           ),
    .ioEthPhyMdio                   (ioEthPhyMdio                           ),
    .iArmUartRx                     (iArmUartRx                             ),
    .oArmUartTx                     (wArmUartTx                             ),

    .oMCURebootEn                   (wMCURebootEn                           ),

    .oSpiClk                        (wSpiClk                                ),
    .oSpiCs                         (oSpiCs                                 ),
    .oSpiMosi                       (oSpiMosi                               ),
    .iSpiMiso                       (iSpiMiso                               ),

    .oDdrWrOpClk                    (wDdrWrOpClk0                           ),
    .oDdrWrOpBurstBegin             (wDdrWrOpBurstBegin0                    ),
    .oDdrWrOpAddr                   (wDdrWrOpAddr0[27:0]                    ),
    .oDdrWrOpData                   (wDdrWrOpData0[127:0]                   ),
    .oDdrWrOpDataMask               (wDdrWrOpDataMask0[15:0]                ),
    .iDdrWrOpDone                   (wDdrWrOpDone                           ),
    .oDdrRdOpClk                    (wDdrRdOpClk0                            ),
    .oDdrRdOpBurstBegin             (wDdrRdOpBurstBegin0                    ),
    .oDdrRdOpAddr                   (wDdrRdOpAddr0[27:0]                    ),
    .iDdrLogicClk                   (wDdrLogicClk                           ),
    .iDdrRdOpData                   (wDdrRdOpData0[127:0]                   ),
    .iDdrRdOpDataVld                (wDdrRdOpDataVld0                       ),

    .iSwdClk                        (iSwdClk                                ),
    .ioSwdData                      (ioSwdData                              )
);


assign oArmUartTx       =   wArmUartTx;
assign oArmUartDbgTx    =   wArmUartTx;
assign oEhtPhyRstN[1:0] = {wEhtPhyRstN,wEhtPhyRstN};
//******************************************************************************

//******************************************************************************
//32'h0100_0000
BmsParamCore U_BmsParamCore
(
    .iSysClk                        (wSysClk2                               ),
    .iBmsParamBusRdData             (wBmsParamBusRdData[7:0]                ),
    .oBmsParamBusClk                (wBmsParamBusClk                        ),
    .oBmsParamBusAddr               (wBmsParamBusAddr[31:0]                 ),
    
    .oBatVoltage                    (wBatVoltage            [31:0]          ),
    .oBatVoltageAverage             (wBatVoltageAverage     [31:0]          ), 
    .oBatCurrent                    (wBatCurrent            [31:0]          ),  
    .oBatSoc                        (wBatSoc                [15:0]          ),  
    .oCurrentMaxChgCurrent          (wCurrentMaxChgCurrent  [31:0]          ),  
    .oCurrentMaxDsgCurrent          (wCurrentMaxDsgCurrent  [31:0]          ),  
    .oBmsShutdown                   (wBmsShutdown           [ 7:0]          )  
);

  //------------------------SoftReboot Start-----------------------------------  

wire         wSoftRebootEn    ;
wire         wSoftRebootMode  ;
SoftReBoot SoftReBoot_inst(
  .iClk                             (wSysClk1                               ),
  .iMCURebootEn                     (wMCURebootEn                           ), //1 indicates a restart from the APP section
  .iManualReboot                    (wManualReboot[7:0]                     )  //0x50 indicates a restart from the APP area; 0x51 indicates a restart from the BOOT area.
);
  //------------------------SoftReboot End-----------------------------------  
//32'h0200_0000
FpgaParamCore U_FpgaParamCore
(
    .iSysClk                        (wSysClk2                               ),
    //.oFpgaParam                     (wFpgaParam[(2*32 - 1):0]               ),
    .oFpgaParamBusClk               (wFpgaParamBusClk                       ),
    .oFpgaParamBusAddr              (wFpgaParamBusAddr[31:0]                ),

    .oFpgaParamBusWrEn              (wFpgaParamBusWrEn                      ),
    .oFpgaParamBusWrData            (wFpgaParamBusWrData[15:0]              ),
    .iFpgaParamBusRdData            (wFpgaParamBusRdData[15:0]              ),
    //Param fpga Wr
    .iDCOutputTotalVoltage          (wBusTotalVolFilterPre     [15:0]       ),
    .iDCOutputTotalPower            (wDCOutputTotalPower       [15:0]       ),
    .iDCHeatSinkTemp1               (wDCHeatSinkTemp1          [15:0]       ),
    .iDCHeatSinkTemp2               (wDCHeatSinkTemp2          [15:0]       ),
    .iDCFailureInformation1         (wDCFailureInformation1    [15:0]       ),
    .iDCFailureInformation2         (wDCFailureInformation2    [15:0]       ),
    .iDCFailureInformation3         (wDCFailureInformation3    [15:0]       ),
    .iDCAlarmInformation1           (wDCAlarmInformation1       [15:0]       ),
    .iFan1Status                    ({wFan1Err,2'b00,wFan1Speed[12:0]}      ),
    .iFan2Status                    (                                       ),
    .iLlc1OutputVoltage             (wLlc1OutputVoltage        [15:0]       ),
    .iLlc1OutputCurrent             (wLlc1OutputCurrent        [15:0]       ),
    .iLlc2OutputVoltage             (wLlc2OutputVoltage        [15:0]       ),
    .iLlc2OutputCurrent             (wLlc2OutputCurrent        [15:0]       ),
    .iBuckBoost1OutputVoltage       (wBuckBoost1OutputVoltage  [15:0]       ),
    .iBuckBoost1OutputCurrent       (wBuckBoost1OutputCurrent  [15:0]       ),
    .iBuckBoost2OutputVoltage       (wBuckBoost2OutputVoltage  [15:0]       ),
    .iBuckBoost2OutputCurrent       (wBuckBoost2OutputCurrent  [15:0]       ),
    .iDcdcWorkStatus                (wCurrentState             [1:0]        ),
    .iPcsHardwareId                 (iPcsHardwareId            [1:0]        ),
    .iSeriesRelayEn                 (oSeriesRelayEn                         ),
    //Param fpga Rd
    .oSwitchOnOff                   (wSwitchOnOff                           ),
    .oForwardEn                     (wForwardEn                             ),
    .oBoardIp                       (wBoardIp[31:0]                         ), 
    .oDCOnOff                       (wDCOnOff                               ),
    .oEqualEnableCharge             (wEqualEnableCharge                     ),
    .oEqualEnableDischarge          (wEqualEnableDischarge                  ),
    .oPcsSoftWareId                 (wPcsSoftWareId[1:0]                    ) ,
    .oClearOfFault                  (wClearOfFault                          ),
    .oUpdataStatus                  (wUpdataStatus             [15:0]       ),
    .oEqualizationEnable            (wEqualizationEnable       [15:0]       ),
    .oLedDir                        (wLedDir                   [7:0]        ),
    .oLedRValue                     (wLedRValue                [7:0]        ),
    .oLedGValue                     (wLedGValue                [7:0]        ),
    .oLedBValue                     (wLedBValue                [7:0]        ),
    .oLedMode                       (wLedMode                  [15:0]       ),
    .oLedPeriod                     (wLedPeriod                [15:0]       ),

    .oBoardIpUpdata                 (wBoardIpUpdata                         ), 
    .oEthUdpRstN                    (wEthUdpRstN                            ),
     //------------------------SoftReboot Start-----------------------------------   
    .oManualReboot                  (wManualReboot                          ),
     //------------------------SoftReboot End-----------------------------------  
    .oFanForceEn                    (wFanForceEn                            ),
    //Fault Recording
    .iMcuFlashBurnEn                (wMcuFlashBurnEn                        ), 
    .iMcuRdStartAddr                (wMcuRdStartAddr                        ),
    .oMcuFaultBurnFinish            (wMcuFaultBurnFinish                    )
);    

//The input signal unit is mA, and the output unit signal is 0.1A.
Division100_16bit #(
    .CLK_FREQ   (100                        )
)U_Division10_16bit(
    .iSysClk    (wSysClk2                   ),
    .iSysRst    (wSysClk2RstP               ),
    .iDataIn0   (wLlcBusCurrentA   [15:0]   ),
    .iDataIn1   (wLlcBusCurrentB   [15:0]   ),
    .iDataIn2   (wBusTotalCurFilterA[15:0]  ),
    .iDataIn3   (wBusTotalCurFilterB[15:0]  ),
    .iDataIn4   ( 16'd1000                  ),
    .iDataIn5   ( 16'd1000                  ),
    .iDataIn6   ( 16'd1000                  ),
    .iDataIn7   ( 16'd1000                  ),
    .iCnt1Us    (wSysClk2Delay1Us           ),
    .oDataOut0  (wLlc1OutputCurrent         ),
    .oDataOut1  (wLlc2OutputCurrent         ),
    .oDataOut2  (wBuckBoost1OutputCurrent   ),
    .oDataOut3  (wBuckBoost2OutputCurrent   ),
    .oDataOut4  (                           ),
    .oDataOut5  (                           ),
    .oDataOut6  (                           ),
    .oDataOut7  (                           )
);

assign wDCOutputTotalCurrent[15:0] = $signed(wBusTotalCurFilterA[15:0]) + $signed(wBusTotalCurFilterB[15:0]);
MultAxB_XilinxV0101 #
(
    .SYNTHESIS_MODE_SET (1  ), // synthrsis mode set  1: DSP 0: LUT
    .AWIDTH             (16 ), // Width of multiplier's 1st input
    .BWIDTH             (16 ), // Width of multiplier's 2nd input
    .PWIDTH             (32 ), // Output Width
    .TC                 (1  )  // 1'b0 : unsigned 1'b1 : signed 
)Multi_DCPower(
    .InClk (wSysClk2                    ), // Clock
    .InA   (wDCOutputTotalCurrent [15:0]),// Multiplier input, max data bw 25
    .InB   (wBusTotalVolFilterPre [15:0]), // Mutiplier input, max data bw 18 
    .OutP  (wDCOutputTotalPowerPre[31:0]   )// Output,max data bw 48
);

SignedDivision  #(
   . WITH1 (32    )        ,     
   . WITH2 (15    )  
)DCOutputTotalPower (
    .iSysClk     (wSysClk2                  ),
    .iSysRst     (wSysClk2RstP              ),
    .iStart      (1                         ),
    .iDividend   (wDCOutputTotalPowerPre[31:0] ),
    .iDivisor    (15'sd10000                 ),
    .oQuotient   (wDCOutputTotalPower       ),
    .oRemainder  (                          ),
    .oDone       (                          ),
    .oBusy       (                          ) 
);

//******************************************************************************

//******************************************************************************
// DDR:1Gb , x16 ,row:13 , col:10 , bank:3

DdrCtrlCore U_DdrCtrlCore
(
    .iDdrClk                            (wSysClk2                  ),////100MHz
    .iDdrRefClk                         (wSysClk3                  ),////200MHz
    .iMigRst                            (wSysClk2RstP              ),
    .iFpgaTemp                          (wFpgaTemp[11:0]           ),//
    .oDdrInitDone                       (wDdrInitDone              ),//o
    .oDdrWrOpDone                       (wDdrWrOpDone              ),//o
    .oDdrLogicClk                       (wDdrLogicClk              ),//o
    .ddr3_addr                          (oDdrAddr[12:0]            ),//o
    .ddr3_ba                            (oDdrBa[2:0]               ),//o
    .ddr3_cas_n                         (oDdrCasN                  ),//o
    .ddr3_ck_n                          (oDdrClkN                  ),//o
    .ddr3_ck_p                          (oDdrClkP                  ),//o
    .ddr3_cke                           (oDdrCke                   ),//o
    //.ddr3_cs_n                      (oDdrCsN                                ),  //o
    .ddr3_ras_n                         (oDdrRasN                  ),//o
    .ddr3_reset_n                       (oDdrRstN                  ),//o
    .ddr3_we_n                          (oDdrWeN                   ),//o
    .ddr3_dq                            (ioDdrDq                   ),//io
    .ddr3_dqs_n                         (ioDdrDqsN                 ),//io
    .ddr3_dqs_p                         (ioDdrDqsP                 ),//io
    .ddr3_dm                            (oDdrDm[1:0]               ),//o
    .ddr3_odt                           (oDdrOdt                   ),//o
    
    .wr0_addr                           (wDdrWrOpAddr0[27:0]       ),//i
    .wr0_buf_base                       (8'd0                      ),//i
    .wr0_buf_sel                        (1'd0                      ),//i
    .wr0_burst_begin                    (wDdrWrOpBurstBegin0       ),//i
    .wr0_burst_rdy                      (wDdrWrOpBurstRdy0         ),
    .wr0_clk                            (wDdrWrOpClk0              ),//i
    .wr0_len                            (9'd1                      ),//i
    .wr0_buf_addr                       (                          ),//o
    .wr0_buf_addr_en                    (                          ),//o
    .wr0_buf_data                       (wDdrWrOpData0[127:0]      ),//i
    .wr0_buf_mask                       (wDdrWrOpDataMask0[15:0]   ),//i

    .wr1_addr                           ({2'd0,wDdrWrOpAddr1[25:0]}),
    .wr1_buf_base                       ({3'd0,wDdrWrBufBaseAddr1[4:0]}),
    .wr1_buf_sel                        (wDdrWrBufSel1             ),
    .wr1_buf_data                       (wDdrWrOpData1             ),
    .wr1_buf_mask                       (wDdrWrOpDataMask1         ),
    .wr1_burst_begin                    (wDdrWrOpBurstBegin1       ),
    .wr1_burst_rdy                      (wDdrWrOpBurstRdy1         ),
    .wr1_clk                            (wDdrWrOpClk1              ),
    .wr1_len                            (wDdrWrOpLen1              ),
    .wr1_buf_addr                       (wDdrWrDataAddr1           ),
    .wr1_buf_addr_en                    (wDdrWrDataAddrEn1         ),

    .rd0_clk                            (wDdrRdOpClk0               ),//i
    .rd0_addr                           (wDdrRdOpAddr0[27:0]       ),//i
    .rd0_burst_begin                    (wDdrRdOpBurstBegin0       ),//i
    .rd0_burst_rdy                      (wDdrRdOpBurstRdy0         ),
    .rd0_len                            (9'd1                      ),//i
    .rd0_opt                            (14'd0                     ),//i
    .rd0_buf_sel                        (1'b0                      ),
    .rd0_buf_base                       (8'd0                      ),
    .rd0_data                           (wDdrRdOpData0[127:0]      ),//o
    .rd0_data_addr                      (                          ),//o
    .rd0_data_en                        (wDdrRdOpDataVld0          ),//o
    .rd0_data_opt                       (                          )//o

    // .rd1_clk                            (wDdrRdOpClk1              ),//i
    // .rd1_addr                           ({2'd0,wDdrRdOpAddr1[25:0]}),//i
    // .rd1_burst_begin                    (wDdrRdOpBurstBegin1       ),//i
    // .rd1_burst_rdy                      (wDdrRdOpBurstRdy1         ),
    // .rd1_len                            (wDdrRdOpLen1              ),
    // .rd1_opt                            (14'd0                     ),
    // .rd1_buf_sel                        (wDdrRdBufSel1             ),
    // .rd1_buf_base                       ({3'd0,wDdrRdBufBaseAddr1[4:0]}),
    // .rd1_data                           (wDdrRdOpData1             ),//o
    // .rd1_data_addr                      (wDdrRdDataAddr1           ),//o
    // .rd1_data_en                        (wDdrRdDataEn1             ),//o
    // .rd1_data_opt                       (                          ) //o
);

 FaultRecordingCore U_FaultRecordingCore(
    .iSysClk                            (wSysClk2                  ),
    .iRstP                              (!wDdrInitDone             ),
//*************************U_MonitorDataStorage******************************//  
    .iMonitorData                       (wAdcData                  ),
    .iMonitorDataVld                    ( wAdcSampleSequenceDone         ),//(wSysClk2Delay1Ms    ),
    .iDcDcErr                           (wDcdcErr                  ),  
    .iBmsShutdown                       (wBmsShutdown[7:0]         ),
    .oMcuRdStartAddr                    (wMcuRdStartAddr[25:0]     ),
    .oMcuFlashBurnEn                    (wMcuFlashBurnEn           ),
    .iMcuFaultBurnFinish                (wMcuFaultBurnFinish       ),
    //Write DDR CMD
    .oDdrWrCmdClk                       (wDdrWrOpClk1               ),
    .oDdrWrBegin                        (wDdrWrOpBurstBegin1       ),
    .iDdrWrRdy                          (wDdrWrOpBurstRdy1         ),
    .oDdrWrBufBaseAddr                  (wDdrWrBufBaseAddr1[4:0]   ),
    .oDdrWrBufSel                       (wDdrWrBufSel1             ),
    .oDdrWrLen                          (wDdrWrOpLen1[8:0]         ),
    .oDdrWrAddr                         (wDdrWrOpAddr1[25:0]       ),
    //Write DDR Data
    .iDdrWrDataClk                      (wDdrLogicClk              ),
    .iDdrWrDataAddr                     (wDdrWrDataAddr1[8:0]      ),
    .iDdrWrDataAddrEn                   (wDdrWrDataAddrEn1         ),
    .oDdrWrMask                         (wDdrWrOpDataMask1[15:0]   ),
    .oDdrWrData                         (wDdrWrOpData1[127:0]      ) 
);



//******************************************************************************
//

AdcSampleCore U_AdcSampleCore
(
    .iSysClk                        (wSysClk2                               ),
    .iSysClkDbug                    (wSysClk4                               ),
    .iSysRst                        (wSysClk2RstP                           ),
    .iVP                            (iVP                                    ),
    .iVN                            (iVN                                    ),
    .iVauxP                         (iVauxP                 [15:0]          ),
    .iVauxN                         (iVauxN                 [15:0]          ),
    .oFpgaTemp                      (wFpgaTemp              [11:0]          ),
    .oLlcBusVoltageA                (wLlcBusVoltageA        [15:0]          ),
    .oLlcBusVoltageB                (wLlcBusVoltageB        [15:0]          ),
    .oBusVoltageA                   (wBusVoltageA           [15:0]          ),
    .oBusVoltageB                   (wBusVoltageB           [15:0]          ),
    .oBusTotalVoltage               (wBusTotalVoltage       [15:0]          ),
    .oBusTotalVoltagePre            (wBusTotalVoltagePre    [15:0]          ),

    .oResonanceCurrentA             (wResonanceCurrentA     [15:0]          ),
    .oResonanceCurrentB             (wResonanceCurrentB     [15:0]          ),
    .oLlcBusCurrentA                (wLlcBusCurrentA        [15:0]          ),
    .oLlcBusCurrentB                (wLlcBusCurrentB        [15:0]          ),
    .oBusTotalCurrentA              (wBusTotalCurrentA      [15:0]          ),
    .oBusTotalCurrentB              (wBusTotalCurrentB      [15:0]          ),
    .oVoltageSampVref               (wVoltageSampVref       [15:0]          ),
    .oCurrentSampVref               (wCurrentSampVref       [15:0]          ),

    .oTemp1                         (wTemp1                 [15:0]          ),
    .oTemp2                         (wTemp2                 [15:0]          ),
    .oTemp3                         (wTemp3                 [15:0]          ),
    .oTemp4                         (wTemp4                 [15:0]          ),
    .oTemp5                         (wTemp5                 [15:0]          ),
    .oTemp6                         (wTemp6                 [15:0]          ),
    .oTempSel                       (oTempSel               [2:0]           ),
    .oAdcSampleSequenceDone         (wAdcSampleSequenceDone                 ),
    .iCurrentSampVrefAvera          (wCurrentSampVrefAvera[11:0]            )
);

NtcSampleCore U_NtcSampleCore
(
    .iSysClk                       (wSysClk2                    ),
    .iAdcSampleSequenceDone        (wAdcSampleSequenceDone      ),
    .iAdcTemp1                     (wTemp1[11:0]                ),    // 12bit ADC input
    .iAdcTemp2                     (wTemp3[11:0]                ),   // 16bit Temp output
    .oTemp1                        (wNtcTemp1                   ),
    .oTemp2                        (wNtcTemp3                   )
);
//******************************************************************************

//******************************************************************************
//

SlowLaunchCtrlCore#(
    .CLK_FREQ                  (100                         ) ,//MHz
    .SLOW_LAUNCH_TIME          (1024*10                     )  //ms               
)U_SlowLaunchCtrlCore
(
    .iSysClk                   (wSysClk2                    ),
    .iSlowLaunchEn             (wStateLlcEn                 ),
    .iCnt1Ms                   (wSysClk2Delay1Ms            ),
    .oSlowLaunchEnd            (wStateLlcOk                 ),
    .oPwmDuty                  (wLLCPwmDuty[9:0]            )  
);

//******************************************************************************

//******************************************************************************
//

assign wStateLoopEn = wStateLlcEn && (wLLCPwmDuty > 10'd70);//75
assign wLLCPwmEn        = (!wDcdcErr) && wStateLlcEn; 
assign wBuckBoostPwmEn  = wLLCPwmEn && (wLLCPwmDuty > 10'd70); 

BatteryPackActiveBalance #(
    .CLK_FREQ                       (100                                )
)U_BatteryPackActiveBalance
(
    .iSysClk                       (wSysClk2                            ),
    .iEqualizationEnable           (wEqualizationEnable                 ),
    .iBatVoltage                   (wBatVoltageFilter                   ),
    .iBatSoc                       (wBatSoc                             ),
    .iBatVoltageAverage            (wBatVoltageAverageFilter            ),
    .oActiveBalanceVoltageOffset   (wActiveBalanceVoltageOffset         ),
    .iDCOutputTotalPower           (wDCOutputTotalPower                 )
);

BatteryPowerLimit#(
    .CLK_FREQ                       (100                                ) 
)U_BatteryPowerLimit(
    .iSysClk                        (wSysClk2                           ),
    .iWorkingEn                     (wWorkingEn                         ),
    .iBatSoc                        (wBatSoc[15:0]                      ),
    .iSeriesRelayEn                 (oSeriesRelayEn                     ),
    .iBatCurrent                    (wBatCurrent                        ),
    .iChgMax                        (wCurrentMaxChgCurrent              ),
    .iDsgMax                        (wCurrentMaxDsgCurrent              ),
    .oTargetCurrentParamMax         (wTargetCurrentParamMax             ),
    .oTargetCurrentParamMin         (wTargetCurrentParamMin             )
);

VirtualResCtrlCore#(
    .CLK_FREQ                       (100                                )
)U_VirtualResCtrlCore
(
    .iSysClk                        (wSysClk2                           ),
    .iCnt1Ms                        (wSysClk2Delay1Ms                   ),
    .iSeriesRelayEn                 (oSeriesRelayEn                     ),
    .iBusTotalCurFilterA            (wBusTotalCurFilterA                ),
    .iBusTotalCurFilterB            (wBusTotalCurFilterB                ),
    .iDCOutputTotalPower            (wDCOutputTotalPower                ),
    .oTargetVoltageOffsetA          (wTargetVoltageOffsetA              ),
    .oTargetVoltageOffsetB          (wTargetVoltageOffsetB              )
);
   
AutoCtrlCore #(
    .CLK_FREQ                       (100                                )
)U_AutoCtrlCoreA 
(
    .iSysClk                        (wSysClk2                           ),
    .iLoopCtrlEn                    (wStateLoopEn                       ),
    .iAdcSampleSequenceDone         (wAdcSampleSequenceDone             ),
    .iFeedBackVoltageParam          (wBusVolFilterA                     ),
    .iFeedBackCurrentParam          (wLlcBusCurFilterA                  ),

    .iTargetCurrentParamMax         (wTargetCurrentParamMax             ),
    .iTargetCurrentParamMin         (wTargetCurrentParamMin             ),
    .iTargetVoltageOffset           (wTargetVoltageOffsetA              ),
    .iActiveBalanceVoltageOffset    (wActiveBalanceVoltageOffset        ),
    .iSeriesRelayEn                 (oSeriesRelayEn                     ),
    .oPidControlPwmParam            (wPidControlPwmParamA               ),

    .oGiRef                         (wGiRefA                            ),
    .oGiI                           (wGvIA                              )
    
);

AutoCtrlCore #(
    .CLK_FREQ                       (100                                )
)U_AutoCtrlCoreB 
(
    .iSysClk                        (wSysClk2                           ),
    .iLoopCtrlEn                    (wStateLoopEn                       ),
    .iAdcSampleSequenceDone         (wAdcSampleSequenceDone             ),
    .iFeedBackVoltageParam          (wBusVolFilterB                     ),
    .iFeedBackCurrentParam          (wLlcBusCurFilterB                  ),
    
    .iTargetCurrentParamMax         (wTargetCurrentParamMax             ),
    .iTargetCurrentParamMin         (wTargetCurrentParamMin             ),
    .iTargetVoltageOffset           (wTargetVoltageOffsetB              ),
    .iActiveBalanceVoltageOffset    (wActiveBalanceVoltageOffset        ),   
    .iSeriesRelayEn                 (oSeriesRelayEn                     ),
    .oPidControlPwmParam            (wPidControlPwmParamB               ),

    .oGiRef                         (wGiRefB                            ),
    .oGiI                           (wGvIB                              )

);

BuckBoostCtrlCore #(
    .CLK_FREQ                       (100                                )
)U_BuckBoostCtrl(
    .iSysClk                        (wSysClk2                           ),
    .iPcsHardwareIdUsing            (wPcsUsingId                        ),
    .iSlowLaunchEn                  (wStateLlcEn                        ),
    .iLLCPwmDuty                    (wLLCPwmDuty[9:0]                   ),

    .iPidControlPwmParamA           (wPidControlPwmParamA               ),
    .iPidControlPwmParamB           (wPidControlPwmParamB               ),

    .oBuckBoostPwmDutyA             (wBuckBoost1PwmDuty[9:0]            ),
    .oBuckBoostPwmDutyB             (wBuckBoost2PwmDuty[9:0]            )
);



//******************************************************************************

//******************************************************************************
PwmTransferCore U_PwmTransfer
(
    .iSysClk                            (wSysClk3                       ),//2000MHz wSysClk3 wSysClkSSC
    .iSscClk                            (wSysClk3                       ),//wSysClkSSC200M
    .iSysRst                            (wSysClk3RstP                   ),
    .iLLCPwmEn                          (wLLCPwmEn                      ),
    .iBuckBoostPwmEn                    (wBuckBoostPwmEn                ),
    .iBuckBoost1PwmDuty                 (wBuckBoost1PwmDuty[9:0]        ),//wBuckBoost1PwmDuty[9:0]
    .iBuckBoost2PwmDuty                 (wBuckBoost2PwmDuty[9:0]        ),//wBuckBoost2PwmDuty[9:0]
    .iLLC1PwmDuty                       (wLLCPwmDuty[9:0]               ),//wLLC1PwmDuty[9:0]
    .iLLC2PwmDuty                       (wLLCPwmDuty[9:0]               ),//wLLC2PwmDuty[9:0]
    .oBuckBoost1PwmOutP                 (oBuckBoost1PwmOutP             ),
    .oBuckBoost1PwmOutN                 (oBuckBoost1PwmOutN             ),
    .oBuckBoost2PwmOutP                 (oBuckBoost2PwmOutP             ),
    .oBuckBoost2PwmOutN                 (oBuckBoost2PwmOutN             ),
    .oLLC1LVPwm1OutP                    (oLLC1LVPwm1OutP                ),
    .oLLC1LVPwm2OutP                    (oLLC1LVPwm2OutP                ),
    .oLLC1HVPwm1OutP                    (oLLC1HVPwm1OutP                ),
    .oLLC1HVPwm2OutP                    (oLLC1HVPwm2OutP                ),
    .oLLC2LVPwm1OutP                    (oLLC2LVPwm1OutP                ),
    .oLLC2LVPwm2OutP                    (oLLC2LVPwm2OutP                ),
    .oLLC2HVPwm1OutP                    (oLLC2HVPwm1OutP                ),
    .oLLC2HVPwm2OutP                    (oLLC2HVPwm2OutP                ) 
    );

assign oDrvOutEn[2:0]                = {3{1'b1}}    ;


//******************************************************************************


//******************************************************************************
ProtectCore#(
    .CLK_FREQ                   (100                                    ),  //MHz
    .FAILURE_RECOVERY_TIME      (5                                      )   //s 10-8191
                  
)U_ProtectCore
(
    .iSysClk                    (wSysClk2                               ),
    .iRstP                      (wSysClk2RstP                           ),
    .iSysClk1SPulse             (wSysClk2Delay1S                        ),
    .iProtectEn                 (wProtectEn                             ),
    .iDmosAndCmosEn             (iDmosAndCmosEn                         ),
    .iLLCPwmDuty                (wLLCPwmDuty[9:0]                       ),  //LLC 0-3FF--> 0-50% 
    .iWorkingEn                 (1                                      ),
    .iStateLlcEn                (wStateLlcEn                            ),
    .iStateLlcOk                (wStateLlcOk                            ),
    .iStateLoopEn               (wStateLlcEn                            ),
    .iStateLoopOk               (wStateLlcOk                            ),
    .iAdcSampleSequenceDone     (wAdcSampleSequenceDone                 ),
    .iLlcBusVoltageA            (wLlcBusVoltageA        [15:0]          ), //wLlcBusVoltageA
    .iLlcBusVoltageB            (wLlcBusVoltageB        [15:0]          ), //wLlcBusVoltageB
    .iBusVoltageA               (wBusVoltageA           [15:0]          ), 
    .iBusVoltageB               (wBusVoltageB           [15:0]          ), 
    .iBusTotalVoltage           (wBusTotalVolFilter     [15:0]          ), //wBusTotalVoltage   
    .iBusTotalVoltagePre        (wBusTotalVolFilterPre  [15:0]          ), //wBusTotalVoltagePre
    .iResonanceCurrentA         (wResonanceCurrentA     [15:0]          ), 
    .iResonanceCurrentB         (wResonanceCurrentB     [15:0]          ), 
    .iLlcBusCurrentA            (wLlcBusCurrentA                        ),//  wLlcBusCurFilterA      [15:0]
    .iLlcBusCurrentB            (wLlcBusCurrentB                        ),//  wLlcBusCurFilterB      [15:0]
    .iBusTotalCurrentA          (wBusTotalCurrentA                      ),//  wBusTotalCurFilterA    [15:0]
    .iBusTotalCurrentB          (wBusTotalCurrentB                      ),//  wBusTotalCurFilterB    [15:0]
    .iVoltageSampVref           (wVoltageSampVref       [15:0]          ),//
    .iCurrentSampVref           (wCurrentSampVref       [15:0]          ), 
    .iTemp1                     (wTemp1                 [15:0]          ),
    .iTemp2                     (wTemp2                 [15:0]          ),
    .iTemp3                     (wTemp3                 [15:0]          ),
    .iTemp4                     (wTemp4                 [15:0]          ),
    .iTemp5                     (wTemp5                 [15:0]          ),
    .iTemp6                     (wTemp6                 [15:0]          ),
    .iNtcTemp1                  (wNtcTemp1              [ 7:0]          ),
    .iNtcTemp2                  (wNtcTemp3              [ 7:0]          ),
    .iLlcOverCurrent            (iLlcOverCurrent                        ),
    .iLlcOverVoltage            (iLlcOverVoltage                        ),
    .iSeriesRelayEn             (oSeriesRelayEn                         ),
    .iParallelRelayEn           (oParallelRelayEn                       ),
    .iSelfInspectionEnd         (wSelfInspectionEnd                     ),
    .iPcsUsingId                (wPcsUsingId                            ),
    .iDcDcPower                 (wDCOutputTotalPower    [15:0]          ),

    .oDcdcErr                   (wDcdcErr                               ),
    .oErrRecovery               (wErrRecovery                           ),
    .oDCFailureInformation1     (wDCFailureInformation1                 ),
    .oDCFailureInformation2     (wDCFailureInformation2                 ),    
    .oDCFailureInformation3     (wDCFailureInformation3                 ),
    .oDCDCAlarmInformation1     (wDCAlarmInformation1                    ), 
    .oDbgMsg                    (wProtectDbgMsg                         ),
    .oErrDisplay                (wErrDisplay                            )
);

//******************************************************************************



SelfInspectionCore#(
    .CLK_FREQ                   ( 100                           ) //MHz            
)U_SelfInspectionCore
(
    //Global Clock
    .iSysClk                    (wSysClk2                       ),
    .iSysRst                    (wSysClk2RstP                   ),
    .iCnt1Ms                    (wSysClk2Delay1Ms               ),
    .iLlcBusVoltageA            (wLlcBusVoltageA         [15:0] ),
    .iLlcBusVoltageB            (wLlcBusVoltageB         [15:0] ),
    .iBusVoltageA               (wBusVoltageA            [15:0] ),
    .iBusVoltageB               (wBusVoltageB            [15:0] ),
    .iBusTotalVoltage           (wBusTotalVoltage        [15:0] ),
    .iBusTotalVoltagePre        (wBusTotalVoltagePre     [15:0] ),
    .iVoltageSampVref           (wVoltageSampVref        [15:0] ),
    .iCurrentSampVref           (wCurrentSampVref        [15:0] ),
    .iPcsHardwareId             (iPcsHardwareId          [1:0]  ),
    .iPcsSoftWareId             (wPcsSoftWareId          [1:0]  ),
    .iAdcSampleSequenceDone     (wAdcSampleSequenceDone         ),
    .iSelfInspectionEn          (wSelfInspectionEn              ),
    .oSelfInspectionEnd         (wSelfInspectionEnd             ),
    .oVoltageSampVrefAvera      (wVoltageSampVrefAvera   [11:0] ),
    .oCurrentSampVrefAvera      (wCurrentSampVrefAvera   [11:0] ),
    .oSeriesRelayEn             (oSeriesRelayEn                 ), 
    .oParallelRelayEn           (oParallelRelayEn               ),
    .oPcsUsingId                (wPcsUsingId                    )

    );


//******************************************************************************
//
assign wMainRelayEn = wWorkingEn;

assign  oMainRelayEn        = wMainRelayEn      ;
assign  oPrechargeRelayDis  = wPrechargeRelayDis;

//******************************************************************************


//******************************************************************************
//
OperationCtrl#(
    .CLK_FREQ               (100    )       //MHz            
)U_OperationCtrl
(
    //Global Clock
    .iSysClk                (wSysClk2                           ),
    .iSysRst                (wSysClk2RstP                       ),

  
    .iDcdcErr               (wDcdcErr                           ),
    .iNoEthNet              (wNoEthNet                          ),

    .iSelfInspectionEnd     (wSelfInspectionEnd                 ), 
    .iSlowLaunchEnd         (wStateLlcOk                        ), 
    
    .iDmosAndCmosEn         (iDmosAndCmosEn                     ),
    .iBmsShutdown           (wBmsShutdown[7:0]                  ),

    .oSelfInspectionEn      (wSelfInspectionEn                  ),  
    .oSlowLaunchEn          (wStateLlcEn                        ), 
    .oWorkingEn             (wWorkingEn                         ),
    .oProtectEn             (wProtectEn                         ),
    .oCurrentState          (wCurrentState[1:0]                 )

);




assign oCaptureInputEn               = 1'b1         ;          

FanCtrlCore U_FanCtrlCore
(
    .iSysClk                            (wSysClkSSC50M             ),// SSC50MHz
    .iSysRst                            (wSysClk2RstP              ),// HIGH_ACTIVE
    .iSysClk2                           (wSysClk2                  ),//100MHz
    .iSysClk2Cnt1Ms                     (wSysClk2Delay1Ms          ),
    .iNtcTemp1                          (wNtcTemp1                 ),//1℃
    .iNtcTemp2                          (wNtcTemp3                 ),
    .iFanFreGen                         (iFanPwmFedBack            ),
    .iSelfInspectionEn                  (wSelfInspectionEn         ),
    .iFanForceEn                        (wFanForceEn               ),
    .oFanPwm                            (oFanPwmOutP               ),
    .oFanSpeed                          (wFan1Speed                 ),//rpm
    .oFanErr                            (wFan1Err                   ) //High_ACTIVE
);



LedLampStripCtrlCore#(
    .CLK_FREQ               (100                                ) ,      //MHz  
    .LED_NUM                (21                                 )          
)U_LedLampStripCtrlCore
(
    .iSysClk                (wSysClk2                           ),//wSysClkSSC50M/*
    .iCnt1Ms                (wSysClk2Delay1Ms                   ),
    .iBatSoc                (wBatSoc[15:0]                      ),
    .iNoEthNet              (wNoEthNet                          ),
    .iLedStatus             (wLedDir[7:0]                       ),
    .iLedRValue             (wLedRValue[7:0]                    ),
    .iLedGValue             (wLedGValue[7:0]                    ),
    .iLedBValue             (wLedBValue[7:0]                    ),
    .iLedMode               (wLedMode  [7:0]                    ),
    .iLedPeriod             (wLedPeriod[15:0]                   ),
    .oRGBOut                (oRGBOut[1:0]                       )
);

//******************************************************************************



//******************************************************************************
//

//******************************************************************************

endmodule
