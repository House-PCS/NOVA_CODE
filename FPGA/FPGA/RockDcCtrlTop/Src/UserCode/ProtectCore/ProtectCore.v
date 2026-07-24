//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   RelayCtrlCore.v
//Original Author   :   xcy Nova003107
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

module ProtectCore#(
    parameter          CLK_FREQ                 =    8'd100    ,  //MHz
    parameter          FAILURE_RECOVERY_TIME    =    10'd5        //s 10-8191
            
)
(
    //Global Clock
    input   wire                        iSysClk                     ,
    input   wire                        iRstP                       ,
    input   wire                        iSysClk1SPulse              ,
    input   wire                        iProtectEn                  ,
    input   wire                        iDmosAndCmosEn              ,
    input   wire            [9:0]       iLLCPwmDuty                 , 
    input   wire                        iWorkingEn                  ,
    input   wire                        iStateLlcEn                 ,
    input   wire                        iStateLlcOk                 ,
    input   wire                        iStateLoopEn                ,
    input   wire                        iStateLoopOk                ,
    input   wire                        iAdcSampleSequenceDone      ,
    input   wire   signed   [15:0]      iLlcBusVoltageA             , //快速采样
    input   wire   signed   [15:0]      iLlcBusVoltageB             , 
    input   wire   signed   [15:0]      iBusVoltageA                , 
    input   wire   signed   [15:0]      iBusVoltageB                , 
    input   wire   signed   [15:0]      iBusTotalVoltage            , 
    input   wire   signed   [15:0]      iBusTotalVoltagePre         , 
    input   wire   signed   [15:0]      iResonanceCurrentA          , 
    input   wire   signed   [15:0]      iResonanceCurrentB          , 
    input   wire   signed   [15:0]      iLlcBusCurrentA             , 
    input   wire   signed   [15:0]      iLlcBusCurrentB             , 
    input   wire   signed   [15:0]      iBusTotalCurrentA           , 
    input   wire   signed   [15:0]      iBusTotalCurrentB           , 
    input   wire   signed   [15:0]      iVoltageSampVref            , //慢速采样
    input   wire   signed   [15:0]      iCurrentSampVref            , 
    input   wire   signed   [15:0]      iTemp1                      ,
    input   wire   signed   [15:0]      iTemp2                      ,
    input   wire   signed   [15:0]      iTemp3                      ,
    input   wire   signed   [15:0]      iTemp4                      ,
    input   wire   signed   [15:0]      iTemp5                      ,
    input   wire   signed   [15:0]      iTemp6                      ,
    input   wire   signed   [ 7:0]      iNtcTemp1                   ,
    input   wire   signed   [ 7:0]      iNtcTemp2                   ,
    input   wire   signed   [15:0]      iDcDcPower                  ,
    input   wire                        iLlcOverCurrent             ,
    input   wire                        iLlcOverVoltage             ,
    input   wire             [1:0]      iPcsUsingId                 ,
    input   wire                        iSeriesRelayEn              ,
    input   wire                        iParallelRelayEn            ,
    input   wire                        iSelfInspectionEnd          ,

    output  wire                        oDcdcErr                    ,   
    output  wire                        oErrRecovery                ,
    output  wire            [15:0]      oDCFailureInformation1      ,
    output  wire            [15:0]      oDCFailureInformation2      ,
    output  wire            [15:0]      oDCFailureInformation3      ,
    output  wire            [15:0]      oDCDCAlarmInformation1      ,
    output  wire            [31:0]      oDbgMsg                     ,
    output  wire            [1:0]       oErrDisplay                 
);

reg [15:0]      rDCFailureInformation1=0 ;

//******************************************************************************
//LLC的电压保护阈值 0.1V 电池电压20-28V 1:11 高边电压220-308
localparam  signed  [15:0]  LLC_BUS_VOLTAGE_A_MAX       =    16'sd3360      ;//LLC高压整流后的高压限制在350.0V
localparam  signed  [15:0]  LLC_BUS_VOLTAGE_B_MAX       =    16'sd3360      ;
localparam  signed  [15:0]  LLC_BUS_VOLTAGE_A_MIN       =    16'sd1640      ;//LLC高压整流后的最低电压限制在200.0V
localparam  signed  [15:0]  LLC_BUS_VOLTAGE_B_MIN       =    16'sd1640      ;

//BuckBoost的电压保护0.1V
localparam  signed  [15:0]  BUS_VOLTAGE_A_MAX           =    16'sd5500      ;//单路高压接口限制在520.0V，串联是1040.0V
localparam  signed  [15:0]  BUS_VOLTAGE_B_MAX           =    16'sd5500      ;
localparam  signed  [15:0]  BUS_VOLTAGE_A_MIN           =    16'sd3500      ;//单路高压接口限制在250.0V，串联是500.0V
localparam  signed  [15:0]  BUS_VOLTAGE_B_MIN           =    16'sd3500      ;

localparam  signed  [15:0]  RELAY_VOLTAGE_DIFF_MAX1     =    16'sd1000;//16'sd500       ;//修改
localparam  signed  [15:0]  RELAY_VOLTAGE_DIFF_MIN1     =   -16'sd1000;//-16'sd500       ;
localparam  signed  [15:0]  RELAY_VOLTAGE_DIFF_MAX2     =    16'sd2000      ;
localparam  signed  [15:0]  RELAY_VOLTAGE_DIFF_MIN2     =   -16'sd2000      ;

wire  signed  [15:0]  BUS_TOTAL_VOLTAGE_MAX1                                ;
wire  signed  [15:0]  BUS_TOTAL_VOLTAGE_PRE_MAX1                            ;
wire  signed  [15:0]  BUS_TOTAL_VOLTAGE_MIN1                                ;
wire  signed  [15:0]  BUS_TOTAL_VOLTAGE_PRE_MIN1                            ;
assign BUS_TOTAL_VOLTAGE_MAX1     = iParallelRelayEn ? 16'sd5200 : 16'sd9000;
assign BUS_TOTAL_VOLTAGE_PRE_MAX1 = iParallelRelayEn ? 16'sd5200 : 16'sd9000;
assign BUS_TOTAL_VOLTAGE_MIN1     = iParallelRelayEn ? 16'sd4000 : 16'sd6800;
assign BUS_TOTAL_VOLTAGE_PRE_MIN1 = iParallelRelayEn ? 16'sd4000 : 16'sd6800;

wire  signed  [15:0]  BUS_TOTAL_VOLTAGE_MAX2                                ;
wire  signed  [15:0]  BUS_TOTAL_VOLTAGE_PRE_MAX2                            ;
wire  signed  [15:0]  BUS_TOTAL_VOLTAGE_MIN2                                ;
wire  signed  [15:0]  BUS_TOTAL_VOLTAGE_PRE_MIN2                            ;
assign BUS_TOTAL_VOLTAGE_MAX2     = iParallelRelayEn ? 16'sd5400 : 16'sd10000;
assign BUS_TOTAL_VOLTAGE_PRE_MAX2 = iParallelRelayEn ? 16'sd5400 : 16'sd10000;
assign BUS_TOTAL_VOLTAGE_MIN2     = iParallelRelayEn ? 16'sd3800 : 16'sd6500;
assign BUS_TOTAL_VOLTAGE_PRE_MIN2 = iParallelRelayEn ? 16'sd3800 : 16'sd6500;
localparam  signed  [15:0]  BUS_TOTAL_VOLTAGE_MIN       =    16'sd3800      ;//总电压限制在300V
localparam  signed  [15:0]  BUS_TOTAL_VOLTAGE_PRE_MIN   =    16'sd3800      ;
//******************************************************************************
//LLC电流保护阈值 1mA
localparam  signed  [15:0]  RESONANCE_CURRENT_A_MAX     =    16'sd16000     ;//谐振电流限制在±20A
localparam  signed  [15:0]  RESONANCE_CURRENT_B_MAX     =    16'sd16000     ;
localparam  signed  [15:0]  BOOST_L_BUS_CURRENT_A_MAX_1 =    16'sd10000     ;//LLC高压电流限制在±10A 1级
localparam  signed  [15:0]  BOOST_L_BUS_CURRENT_B_MAX_1 =    16'sd10000     ;
localparam  signed  [15:0]  BOOST_L_BUS_CURRENT_A_MAX_2 =    16'sd14500     ;//LLC高压电流限制在±10A 2级
localparam  signed  [15:0]  BOOST_L_BUS_CURRENT_B_MAX_2 =    16'sd14500     ;//修改
  
localparam  signed  [15:0]  RESONANCE_CURRENT_A_MIN     =   -16'sd16000     ;//20A
localparam  signed  [15:0]  RESONANCE_CURRENT_B_MIN     =   -16'sd16000     ;
localparam  signed  [15:0]  BOOST_L_BUS_CURRENT_A_MIN_1 =   -16'sd10000     ;
localparam  signed  [15:0]  BOOST_L_BUS_CURRENT_B_MIN_1 =   -16'sd10000     ;
localparam  signed  [15:0]  BOOST_L_BUS_CURRENT_A_MIN_2 =   -16'sd13500     ;//修改
localparam  signed  [15:0]  BOOST_L_BUS_CURRENT_B_MIN_2 =   -16'sd13500     ;
localparam  signed  [15:0]  BUS_TOTAL_CURRENT_A_MIN     =   -16'sd6000      ;
localparam  signed  [15:0]  BUS_TOTAL_CURRENT_B_MIN     =   -16'sd6000      ;

localparam  signed  [15:0]  BUS_TOTAL_CURRENT_A_MAX     =    16'sd7000      ;//高压侧电流限制在±7A
localparam  signed  [15:0]  BUS_TOTAL_CURRENT_B_MAX     =    16'sd7000      ;
//******************************************************************************
localparam  signed  [15:0]  CHARGE_POWER_MAX1           =   -16'sd4200      ;
localparam  signed  [15:0]  DISCHARGE_POWER_MAX1        =    16'sd4200      ;
localparam  signed  [15:0]  CHARGE_POWER_MAX2           =   -16'sd4500      ;
localparam  signed  [15:0]  DISCHARGE_POWER_MAX2        =    16'sd4500      ;
//******************************************************************************

//******************************************************************************
//温度保护阈值0.1度
localparam  signed  [15:0]  VOLTAGE_SAMP_VREF_MAX       =    16'sd4000    ;
localparam  signed  [15:0]  CURRENT_SAMP_VREF_MAX       =    16'sd4000    ;
localparam  signed  [ 7:0]  TEMP1_MAX                   =     8'sd90      ;//90℃
localparam  signed  [ 7:0]  TEMP2_MAX                   =     8'sd90      ;
localparam  signed  [ 7:0]  TEMP3_MAX                   =     8'sd90      ;
localparam  signed  [ 7:0]  TEMP4_MAX                   =     8'sd90      ;
localparam  signed  [ 7:0]  TEMP5_MAX                   =     8'sd90      ;
localparam  signed  [ 7:0]  TEMP6_MAX                   =     8'sd90      ;

localparam  signed  [15:0]  VOLTAGE_SAMP_VREF_MIN       =    16'sd1000    ;
localparam  signed  [15:0]  CURRENT_SAMP_VREF_MIN       =    16'sd1000    ;
localparam  signed  [15:0]  TEMP1_MIN                   =    -16'sd300    ;
localparam  signed  [15:0]  TEMP2_MIN                   =    -16'sd300    ;
localparam  signed  [15:0]  TEMP3_MIN                   =    -16'sd300    ;
localparam  signed  [15:0]  TEMP4_MIN                   =    -16'sd300    ;
localparam  signed  [15:0]  TEMP5_MIN                   =    -16'sd300    ;
localparam  signed  [15:0]  TEMP6_MIN                   =    -16'sd300    ;
//******************************************************************************
wire          [15:0]  wDCFailureInformation1                                ; 
reg               rLLCPwmDutyMoreThan400  = 1'b0  ;
wire              wErrRecovery                    ;
wire              wRestorableErr                  ;
wire              wAlarmRecovery                  ;
wire              wRestorableAlarm                ;
reg               rDcdcErr                = 1'b0  ;
wire              wNtcTempMaxErr1                 ;
wire              wNtcTempMaxErr2                 ;
wire              wRelayLogicErrMax               ;
wire              wRelayLogicErrMin               ;
wire              wResonanceCurrentAErrMax        ;
wire              wResonanceCurrentAErrMin        ;
wire              wResonanceCurrentBErrMax        ;
wire              wResonanceCurrentBErrMin        ;
wire              wLlcBusCurrentAErrMax1          ;
wire              wLlcBusCurrentAErrMin1          ;
wire              wLlcBusCurrentAErrMax2          ;
wire              wLlcBusCurrentAErrMin2          ;
wire              wLlcBusCurrentBErrMax1          ;
wire              wLlcBusCurrentBErrMin1          ;
wire              wLlcBusCurrentBErrMax2          ;
wire              wLlcBusCurrentBErrMin2          ;
wire              wLlcBusVoltageAOverErr          ;
wire              wLlcBusVoltageALowErr           ;
wire              wLlcBusVoltageBOverErr          ;
wire              wLlcBusVoltageBLowErr           ;
wire              wBusVoltageAOverErr             ;
wire              wBusVoltageALowErr              ;
wire              wBusVoltageBOverErr             ;
wire              wBusVoltageBLowErr              ;
wire              wBusTotalVoltageErrMax          ;
wire              wBusTotalVoltageErrMin          ;
wire              wBusTotalVoltageAlarmMax        ;
wire              wBusTotalVoltageAlarmMin        ;
wire              wBusTotalVoltageAlarmMaxClear   ;
wire              wBusTotalVoltageAlarmMinClear   ;
wire              wBusTotalVoltagePreErrMax       ;
wire              wBusTotalVoltagePreErrMin       ;
wire              wBusTotalVoltagePreDiffErrMax1  ;
wire              wBusTotalVoltageDiffErrMin1     ;
wire              wBusTotalVoltagePreDiffErrMax2  ;
wire              wBusTotalVoltageDiffErrMin2     ;
wire              wBusTotalCurrentAErrMax         ;
wire              wBusTotalCurrentAErrMin         ;
wire              wBusTotalCurrentBErrMax         ;
wire              wBusTotalCurrentBErrMin         ;
wire              wDsgOverloadAlarm               ;
wire              wChgOverloadAlarm               ;
wire              wDsgOverloadAlarmClear          ;
wire              wChgOverloadAlarmClear          ;
wire              wDsgOverloadErr                 ;
wire              wChgOverloadErr                 ;



reg               rResonanceCurrentAErr   = 1'b0  ;
reg               rResonanceCurrentBErr   = 1'b0  ;
reg               rLlcBusCurrentAErr      = 1'b0  ;
reg               rLlcBusCurrentBErr      = 1'b0  ;
reg               rBusTotalCurrentAErr    = 1'b0  ;
reg               rBusTotalCurrentBErr    = 1'b0  ;
          
reg               rLlcBusVoltageAOverErr          = 1'b0  ;
reg               rLlcBusVoltageBOverErr          = 1'b0  ;
reg               rBusVoltageAOverErr             = 1'b0  ;
reg               rBusVoltageBOverErr             = 1'b0  ;
reg               rBusTotalVoltageOverErr         = 1'b0  ;
reg               rBusTotalVoltageOverAlarm       = 1'b0  ;
reg               rBusTotalVoltagePreOverErr      = 1'b0  ;
reg               rBusTotalVoltageDiffOverErr1    = 1'b0  ;
reg               rBusTotalVoltagePreDiffOverErr1 = 1'b0  ;
reg               rBusTotalVoltageDiffOverErr2    = 1'b0  ;
reg               rBusTotalVoltagePreDiffOverErr2 = 1'b0  ;
reg     [23:0]    rBusTotalVoltageDiffOverCnt     = 24'd0 ; 
reg     [23:0]    rBusTotalVoltagePreDiffOverCnt  = 24'd0 ;
reg               rLlcBusVoltageALowErr           = 1'b0  ;
reg               rLlcBusVoltageBLowErr           = 1'b0  ;
reg               rBusVoltageALowErr              = 1'b0  ;
reg               rBusVoltageBLowErr              = 1'b0  ;
reg               rBusTotalVoltageLowErr          = 1'b0  ;
reg               rBusTotalVoltageLowAlarm        = 1'b0  ;
reg               rBusTotalVoltagePreLowErr       = 1'b0  ;
reg               rHardwareOverCurrentErr         = 1'b0  ;
reg               rHardwareOverVoltageErr         = 1'b0  ;
reg               rDsgOverloadErr                 = 1'b0  ;
reg               rChgOverloadErr                 = 1'b0  ;
reg               rDsgOverloadAlarm               = 1'b0  ;
reg               rChgOverloadAlarm               = 1'b0  ;          
wire              wCurrentErr                     ;
wire              wVoltageErr                     ;
          
wire              wBmsErr                         ;
reg               rBmsErr                     = 1'b0  ;
reg               rRelayLogicErr              = 1'b0  ;  
reg               rNtcTempErr                 = 1'b0  ;
reg     [7:0]     rHardwareOverCurrentCnt  = 8'd0;
reg     [8:0]     rHardwareOverVoltageCntL = 9'd0;
reg     [9:0]     rHardwareOverVoltageCntH = 10'd0;    
reg     [9:0]     rCntLlcBusVoltageAOver ='d0;
reg     [9:0]     rCntLlcBusVoltageBOver ='d0;
reg     [9:0]     rCntLlcBusVoltageALow ='d0;
reg     [9:0]     rCntLlcBusVoltageBLow ='d0;

assign oDbgMsg={
    3'd0,
    rBusTotalVoltageDiffOverErr2,

    rBusTotalVoltagePreDiffOverErr2,
    rDsgOverloadErr, 
    rChgOverloadErr,
    rBusTotalVoltageDiffOverErr1,

    rBusTotalVoltagePreDiffOverErr1, 
    rRelayLogicErr,
    rNtcTempErr,
    rBmsErr,

    rHardwareOverCurrentErr,
    rHardwareOverVoltageErr,
    rResonanceCurrentAErr,
    rResonanceCurrentBErr,

    rLlcBusCurrentAErr,
    rLlcBusCurrentBErr,
    rBusTotalCurrentAErr,
    rBusTotalCurrentBErr,

    rLlcBusVoltageAOverErr,
    rLlcBusVoltageBOverErr,
    rBusVoltageAOverErr,
    rBusVoltageBOverErr,
    
    rBusTotalVoltageOverErr,
    rBusTotalVoltagePreOverErr,
    rLlcBusVoltageALowErr,
    rLlcBusVoltageBLowErr,

    rBusVoltageALowErr,
    rBusVoltageBLowErr,
    rBusTotalVoltageLowErr,
    rBusTotalVoltagePreLowErr  
};

assign oDCFailureInformation1[15:0]={
    1'b0,
    rHardwareOverVoltageErr,
    rLlcBusVoltageAOverErr,
    rLlcBusVoltageBOverErr,
   
    rLlcBusVoltageALowErr,
    rLlcBusVoltageBLowErr,
    (rBusTotalVoltageDiffOverErr1||rBusTotalVoltageDiffOverErr2),
    (rBusTotalVoltagePreDiffOverErr1 || rBusTotalVoltagePreDiffOverErr2), 
   
    rBusVoltageAOverErr,
    rBusVoltageBOverErr,
    rBusTotalVoltageOverErr,
    rBusTotalVoltagePreOverErr,
   
    rBusVoltageALowErr,
    rBusVoltageBLowErr,
    rBusTotalVoltageLowErr,
    rBusTotalVoltagePreLowErr  
};

assign oDCFailureInformation2[15:0]={
    4'b0000,

    4'b0000,

    1'b0,
    rHardwareOverCurrentErr,
    rResonanceCurrentAErr,
    rResonanceCurrentBErr,

    rLlcBusCurrentAErr,
    rLlcBusCurrentBErr,
    rBusTotalCurrentAErr,
    rBusTotalCurrentBErr
};

assign oDCFailureInformation3[15:0]={
    4'b0000,

    1'b0,
    rRelayLogicErr,
    rNtcTempErr,
    rBmsErr,
    
    4'b0000,

    2'b00,
    rDsgOverloadErr, 
    rChgOverloadErr
};

assign oDCDCAlarmInformation1[15:0]={
    12'd0,

    rDsgOverloadAlarm, 
    rChgOverloadAlarm,
    rBusTotalVoltageLowAlarm,
    rBusTotalVoltageOverAlarm 
};

always @(posedge iSysClk) begin
    rLLCPwmDutyMoreThan400 <= iLLCPwmDuty[9:0] >=10'd400;
end

//******************************************************************************
//温度
ThresholdComparCore#(
    .DATA_WIDTH                         (8                        ),
    .GLITCH_FILTER_NUM                  (1                         ) 
)
NtcTempErr1(
    .iClk                               (iSysClk                   ),
    .iValue                             (iNtcTemp1                 ),
    .iMaxThreshold                      (TEMP1_MAX                 ),
    .iMinThreshold                      (0                         ),
    .iMaxCompareEn                      (iProtectEn                ),
    .iMinCompareEn                      (0                         ),
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),

    .oErrMax                            (wNtcTempMaxErr1           ),
    .oErrMin                            (                          ) 
);

ThresholdComparCore#(
    .DATA_WIDTH                         (8                        ),
    .GLITCH_FILTER_NUM                  (1                         ) 
)
NtcTempErr2(
    .iClk                               (iSysClk                   ),
    .iValue                             (iNtcTemp2                 ),
    .iMaxThreshold                      (TEMP2_MAX                 ),
    .iMinThreshold                      (0                         ),
    .iMaxCompareEn                      (iProtectEn                ),
    .iMinCompareEn                      (0                         ),
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),

    .oErrMax                            (wNtcTempMaxErr2           ),
    .oErrMin                            (                          ) 
);
//******************************************************************************

always @(posedge iSysClk) begin
    rNtcTempErr <= wNtcTempMaxErr2 || wNtcTempMaxErr1;
end
//******************************************************************************
//
always @(posedge iSysClk ) begin
    // if(wErrRecovery)
    //     rBmsErr <= 1'b0;
    // else 
    if((iDmosAndCmosEn == 0) && iProtectEn)
        rBmsErr <= 1'b1;
    else
        rBmsErr <= rBmsErr;
end
//******************************************************************************

ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (1                         ) 
)
RelayLogicErr(
    .iClk                               (iSysClk                   ),
    .iValue                             (iBusTotalVoltagePre       ),
    .iMaxThreshold                      ('sd5500                   ),
    .iMinThreshold                      ('sd5500                   ),
    .iMaxCompareEn                      (rLLCPwmDutyMoreThan400    ),
    .iMinCompareEn                      (rLLCPwmDutyMoreThan400    ),
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),

    .oErrMax                            (wRelayLogicErrMax         ),
    .oErrMin                            (wRelayLogicErrMin         ) 
);

always @(posedge iSysClk) begin
    if(iParallelRelayEn)
        rRelayLogicErr <= wRelayLogicErrMax; 
    else if(iSeriesRelayEn) 
        rRelayLogicErr <= wRelayLogicErrMin;         
    else
        rRelayLogicErr <= rRelayLogicErr;
end
//******************************************************************************
//1、谐振电流过流保护 持续时间50us 50us/18.73us ≈ 2
//在闭合阶段进行掩码不判断

reg         ResonanceCurAErrCompareEn = 1'b1;

always @(posedge iSysClk) begin
    if(rLLCPwmDutyMoreThan400)
        ResonanceCurAErrCompareEn <= 1'b1;
    else if(iSeriesRelayEn || iParallelRelayEn)
        ResonanceCurAErrCompareEn <= 1'b0;    
end

ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (5                         ) //修改
)
ResonanceCurrentAErr(
    .iClk                               (iSysClk                   ),
    .iValue                             (iResonanceCurrentA[15:0]  ),
    .iMaxThreshold                      (RESONANCE_CURRENT_A_MAX   ),
    .iMinThreshold                      (RESONANCE_CURRENT_A_MIN   ),
    .iMaxCompareEn                      (ResonanceCurAErrCompareEn ),
    .iMinCompareEn                      (ResonanceCurAErrCompareEn ),
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),

    .oErrMax                            (wResonanceCurrentAErrMax  ),
    .oErrMin                            (wResonanceCurrentAErrMin  ) 
);
always @(posedge iSysClk) begin
    rResonanceCurrentAErr <= wResonanceCurrentAErrMax || wResonanceCurrentAErrMin;
end


ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (5                         ) 
)
ResonanceCurrentBErr(
    .iClk                               (iSysClk                   ),
    .iValue                             (iResonanceCurrentB[15:0]  ),
    .iMaxThreshold                      (RESONANCE_CURRENT_B_MAX   ),
    .iMinThreshold                      (RESONANCE_CURRENT_B_MIN   ),
    .iMaxCompareEn                      (1'b1                      ),//(ResonanceCurAErrCompareEn ),
    .iMinCompareEn                      (1'b1                      ),//(ResonanceCurAErrCompareEn ),
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wResonanceCurrentBErrMax  ),
    .oErrMin                            (wResonanceCurrentBErrMin  ) 
);
always @(posedge iSysClk) begin
    rResonanceCurrentBErr <= wResonanceCurrentBErrMax || wResonanceCurrentBErrMin;
end
//******************************************************************************

//******************************************************************************
//BOOST电感电流（iLlcBusCurrent）过流保护
//由于逆变器电流是正弦，因此做10ms的故障穿越，采样周期是1us*16通道，大改在16us

reg signed [15:0] rLlcBusCurrentA;
reg signed [15:0] rLlcBusCurrentB;

always @(posedge iSysClk) begin
    rLlcBusCurrentA <= iLlcBusCurrentA;
    rLlcBusCurrentB <= iLlcBusCurrentB;
end
//一级过流故障保护 持续时间10ms  10000us/18.73us ≈ 533
ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (630                       ) //修改
)
LlcBusCurrentAErr1(
    .iClk                               (iSysClk                    ),
    .iValue                             (rLlcBusCurrentA            ),
    .iMaxThreshold                      (BOOST_L_BUS_CURRENT_A_MAX_1),
    .iMinThreshold                      (BOOST_L_BUS_CURRENT_A_MIN_1),
    .iMaxCompareEn                      (1'b1                       ),
    .iMinCompareEn                      (1'b1                       ),
    .iErrClear                          (1'b0                       ),
    .iADCTrigger                        (iAdcSampleSequenceDone     ),
               
    .oErrMax                            (wLlcBusCurrentAErrMax1     ),
    .oErrMin                            (wLlcBusCurrentAErrMin1     ) 
);
//二级过流故障保护 持续时间50us  50us/18.73us ≈ 2
ThresholdComparCore#(
    .DATA_WIDTH                         (16                         ),
    .GLITCH_FILTER_NUM                  (3                          ) 
) 
LlcBusCurrentAErr2( 
    .iClk                               (iSysClk                    ),
    .iValue                             (rLlcBusCurrentA            ),
    .iMaxThreshold                      (BOOST_L_BUS_CURRENT_A_MAX_2),
    .iMinThreshold                      (BOOST_L_BUS_CURRENT_A_MIN_2),
    .iMaxCompareEn                      (1'b1                       ),
    .iMinCompareEn                      (1'b1                       ),
    .iErrClear                          (1'b0                       ),
    .iADCTrigger                        (iAdcSampleSequenceDone     ),
            
    .oErrMax                            (wLlcBusCurrentAErrMax2     ),
    .oErrMin                            (wLlcBusCurrentAErrMin2     ) 
);

always @(posedge iSysClk) begin
    rLlcBusCurrentAErr <= wLlcBusCurrentAErrMax1 || wLlcBusCurrentAErrMin1 || wLlcBusCurrentAErrMax2 || wLlcBusCurrentAErrMin2;
end
//******************************************************************************

//******************************************************************************
//BOOST电感电流（iLlcBusCurrent）过流保护
//由于逆变器电流是正弦，因此做10ms的故障穿越，采样周期是18.73us


//一级过流故障保护  持续时间10ms  10000us/18.73us ≈ 533
ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (630                       ) //修改
)
LlcBusCurrentBErr1(
    .iClk                               (iSysClk                   ),
    .iValue                             (rLlcBusCurrentB            ),
    .iMaxThreshold                      (BOOST_L_BUS_CURRENT_B_MAX_1   ),
    .iMinThreshold                      (BOOST_L_BUS_CURRENT_B_MIN_1   ),
    .iMaxCompareEn                      (1'b1                      ),
    .iMinCompareEn                      (1'b1                      ),
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wLlcBusCurrentBErrMax1    ),
    .oErrMin                            (wLlcBusCurrentBErrMin1    ) 
);
//二级过流故障保护 持续时间50us  50us/18.73us ≈ 2
ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (3                         ) //修改
)
LlcBusCurrentBErr2(
    .iClk                               (iSysClk                   ),
    .iValue                             (rLlcBusCurrentB           ),
    .iMaxThreshold                      (BOOST_L_BUS_CURRENT_B_MAX_2   ),
    .iMinThreshold                      (BOOST_L_BUS_CURRENT_B_MIN_2   ),
    .iMaxCompareEn                      (1'b1                      ),
    .iMinCompareEn                      (1'b1                      ),
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wLlcBusCurrentBErrMax2    ),
    .oErrMin                            (wLlcBusCurrentBErrMin2    ) 
);

always @(posedge iSysClk) begin
    rLlcBusCurrentBErr <= wLlcBusCurrentBErrMax1 || wLlcBusCurrentBErrMin1 || wLlcBusCurrentBErrMax2 || wLlcBusCurrentBErrMin2;
end
//******************************************************************************

//******************************************************************************
//总输出电流 过流保护 1ms持续 630*16us=1ms

ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (630                       ) 
)
BusTotalCurrentAErr(
    .iClk                               (iSysClk                   ),
    .iValue                             (iBusTotalCurrentA         ),
    .iMaxThreshold                      (BUS_TOTAL_CURRENT_A_MAX   ),
    .iMinThreshold                      (BUS_TOTAL_CURRENT_A_MIN   ),
    .iMaxCompareEn                      (1'b1                      ),
    .iMinCompareEn                      (1'b1                      ),
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wBusTotalCurrentAErrMax   ),
    .oErrMin                            (wBusTotalCurrentAErrMin   ) 
);

ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (630                       ) 
)
BusTotalCurrentBErr(
    .iClk                               (iSysClk                   ),
    .iValue                             (iBusTotalCurrentB         ),
    .iMaxThreshold                      (BUS_TOTAL_CURRENT_B_MAX   ),
    .iMinThreshold                      (BUS_TOTAL_CURRENT_B_MIN   ),
    .iMaxCompareEn                      (1'b1                      ),
    .iMinCompareEn                      (1'b1                      ),
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wBusTotalCurrentBErrMax   ),
    .oErrMin                            (wBusTotalCurrentBErrMin   ) 
);

always @(posedge iSysClk) begin
    rBusTotalCurrentAErr  <= wBusTotalCurrentAErrMax || wBusTotalCurrentAErrMin;
    rBusTotalCurrentBErr  <= wBusTotalCurrentBErrMax || wBusTotalCurrentBErrMin;
end

//******************************************************************************

assign wCurrentErr = (rResonanceCurrentAErr || rResonanceCurrentBErr || rLlcBusCurrentAErr || rLlcBusCurrentBErr || rBusTotalCurrentAErr || rBusTotalCurrentBErr);

//******************************************************************************

//******************************************************************************
//电压保护
//******************************************************************************

//******************************************************************************
//LLC高压侧电压 过欠压保护 持续5ms 5000us/18.73us ≈ 266

ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (266                       ) 
)
LlcBusVoltageAOverErr(
    .iClk                               (iSysClk                   ),
    .iValue                             (iLlcBusVoltageA[15:0]     ),
    .iMaxThreshold                      (LLC_BUS_VOLTAGE_A_MAX     ),
    .iMinThreshold                      (LLC_BUS_VOLTAGE_A_MIN     ),
    .iMaxCompareEn                      (rLLCPwmDutyMoreThan400    ),
    .iMinCompareEn                      (rLLCPwmDutyMoreThan400    ),//&&iStateLlcOk
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wLlcBusVoltageAOverErr    ),
    .oErrMin                            (wLlcBusVoltageALowErr     ) 
);

always @(posedge iSysClk) begin
    rLlcBusVoltageAOverErr <= wLlcBusVoltageAOverErr;
    rLlcBusVoltageALowErr  <= wLlcBusVoltageALowErr ;
end


ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (266                        ) 
)
LlcBusVoltageBOverErr(
    .iClk                               (iSysClk                   ),
    .iValue                             (iLlcBusVoltageB[15:0]     ),
    .iMaxThreshold                      (LLC_BUS_VOLTAGE_B_MAX     ),
    .iMinThreshold                      (LLC_BUS_VOLTAGE_B_MIN     ),
    .iMaxCompareEn                      (rLLCPwmDutyMoreThan400    ),
    .iMinCompareEn                      (rLLCPwmDutyMoreThan400    ),//&& iStateLlcOk
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wLlcBusVoltageBOverErr    ),
    .oErrMin                            (wLlcBusVoltageBLowErr     )
);

always @(posedge iSysClk) begin
    rLlcBusVoltageBOverErr <= wLlcBusVoltageBOverErr;
    rLlcBusVoltageBLowErr  <= wLlcBusVoltageBLowErr ;
end
//******************************************************************************
//A B路Boost母线电压 过欠压保护 持续10ms 10000us/18.73us ≈ 533
ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (533                       ) 
)
BusTotalVoltageAErr(
    .iClk                               (iSysClk                   ),
    .iValue                             (iBusVoltageA[15:0]        ),
    .iMaxThreshold                      (BUS_VOLTAGE_A_MAX         ),
    .iMinThreshold                      (BUS_VOLTAGE_A_MIN         ),
    .iMaxCompareEn                      (1'b1                      ),
    .iMinCompareEn                      (rLLCPwmDutyMoreThan400    ),
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wBusVoltageAOverErr       ),
    .oErrMin                            (wBusVoltageALowErr        ) 
);
always @(posedge iSysClk) begin
    rBusVoltageAOverErr <= wBusVoltageAOverErr;
    rBusVoltageALowErr  <= wBusVoltageALowErr ;
end

ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (533                       ) 
)
BusTotalVoltageBErr(
    .iClk                               (iSysClk                   ),
    .iValue                             (iBusVoltageB[15:0]        ),
    .iMaxThreshold                      (BUS_VOLTAGE_B_MAX         ),
    .iMinThreshold                      (BUS_VOLTAGE_B_MIN         ),
    .iMaxCompareEn                      (1'b1                      ),
    .iMinCompareEn                      (rLLCPwmDutyMoreThan400    ),
    .iErrClear                          (1'b0                      ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wBusVoltageBOverErr       ),
    .oErrMin                            (wBusVoltageBLowErr        ) 
);
always @(posedge iSysClk) begin
    rBusVoltageBOverErr <= wBusVoltageBOverErr;
    rBusVoltageBLowErr  <= wBusVoltageBLowErr ;
end
//******************************************************************************
//总电压 欠压保护 持续时间10ms  10000us/18.73us ≈ 533us
ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (533                       ) 
)
BusTotalVoltageAlarm1(
    .iClk                               (iSysClk                   ),
    .iValue                             (iBusTotalVoltage[15:0]    ),
    .iMaxThreshold                      (BUS_TOTAL_VOLTAGE_MIN1    ),
    .iMinThreshold                      (BUS_TOTAL_VOLTAGE_MIN1    ),
    .iMaxCompareEn                      (1'b1                      ),
    .iMinCompareEn                      (rLLCPwmDutyMoreThan400    ),
    .iErrClear                          (wBusTotalVoltageAlarmMinClear),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wBusTotalVoltageAlarmMinClear),
    .oErrMin                            (wBusTotalVoltageAlarmMin    ) 
);
always @(posedge iSysClk) begin
    rBusTotalVoltageLowAlarm  <= wBusTotalVoltageAlarmMin;
end

//总电压 欠压保护 持续时间2s  2_000_000us/18.73us ≈ 106781
ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (106781                    ) 
)
BusTotalVoltageAlarm2(
    .iClk                               (iSysClk                   ),
    .iValue                             (iBusTotalVoltage[15:0]    ),
    .iMaxThreshold                      (BUS_TOTAL_VOLTAGE_MAX1    ),
    .iMinThreshold                      (BUS_TOTAL_VOLTAGE_MAX1    ),
    .iMaxCompareEn                      (1'b1                      ),
    .iMinCompareEn                      (rLLCPwmDutyMoreThan400    ),
    .iErrClear                          (wBusTotalVoltageAlarmMaxClear),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wBusTotalVoltageAlarmMax),
    .oErrMin                            (wBusTotalVoltageAlarmMaxClear) 
);
always @(posedge iSysClk) begin
    rBusTotalVoltageOverAlarm  <= wBusTotalVoltageAlarmMax;
end

ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (533                       ) 
)
BusTotalVoltageErr2(
    .iClk                               (iSysClk                   ),
    .iValue                             (iBusTotalVoltage[15:0]    ),
    .iMaxThreshold                      (BUS_TOTAL_VOLTAGE_MAX2    ),
    .iMinThreshold                      (BUS_TOTAL_VOLTAGE_MIN2    ),
    .iMaxCompareEn                      (1'b1                      ),
    .iMinCompareEn                      (rLLCPwmDutyMoreThan400    ),
    .iErrClear                          (wErrRecovery             ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wBusTotalVoltageErrMax    ),
    .oErrMin                            (wBusTotalVoltageErrMin    ) 
);
always @(posedge iSysClk) begin
    rBusTotalVoltageOverErr <= wBusTotalVoltageErrMax;
    rBusTotalVoltageLowErr  <= wBusTotalVoltageErrMin;
end

ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (533                       ) 
)
BusTotalVoltagePreErr2(
    .iClk                               (iSysClk                   ),
    .iValue                             (iBusTotalVoltagePre[15:0] ),
    .iMaxThreshold                      (BUS_TOTAL_VOLTAGE_PRE_MAX2),
    .iMinThreshold                      (BUS_TOTAL_VOLTAGE_PRE_MIN2),
    .iMaxCompareEn                      (1'b1                      ),
    .iMinCompareEn                      (rLLCPwmDutyMoreThan400 && iStateLoopOk    ),
    .iErrClear                          (wErrRecovery             ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wBusTotalVoltagePreErrMax ),
    .oErrMin                            (wBusTotalVoltagePreErrMin ) 
);
always @(posedge iSysClk) begin
    rBusTotalVoltagePreOverErr <= wBusTotalVoltagePreErrMax;
    rBusTotalVoltagePreLowErr  <= wBusTotalVoltagePreErrMin;
end


//******************************************************************************

//******************************************************************************
//预充电阻两端压差过大保护 5ms持续 5000/18.73us ≈ 266

ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                 (8064)// (266                       ) 
)
BusTotalVoltageDiffErr1(
    .iClk                               (iSysClk                   ),
    .iValue                             (iBusTotalVoltagePre - iBusTotalVoltage),
    .iMaxThreshold                      (RELAY_VOLTAGE_DIFF_MAX1   ),
    .iMinThreshold                      (RELAY_VOLTAGE_DIFF_MIN1   ),
    .iMaxCompareEn                      (1'b1                      ),//rLLCPwmDutyMoreThan400
    .iMinCompareEn                      (1'b1                      ),//rLLCPwmDutyMoreThan400
    .iErrClear                          (wErrRecovery             ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wBusTotalVoltagePreDiffErrMax1),
    .oErrMin                            (wBusTotalVoltageDiffErrMin1) 
);
always @(posedge iSysClk) begin
    rBusTotalVoltagePreDiffOverErr1 <= wBusTotalVoltagePreDiffErrMax1;
    rBusTotalVoltageDiffOverErr1    <= wBusTotalVoltageDiffErrMin1;
end
//******************************************************************************

//******************************************************************************
//预充电阻两端压差过大保护 50us持续 50us/18.73us ≈ 2

ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (2                         ) 
)
BusTotalVoltageDiffErr2(
    .iClk                               (iSysClk                   ),
    .iValue                             (iBusTotalVoltagePre - iBusTotalVoltage),
    .iMaxThreshold                      (RELAY_VOLTAGE_DIFF_MAX2   ),
    .iMinThreshold                      (RELAY_VOLTAGE_DIFF_MIN2   ),
    .iMaxCompareEn                      (1                         ),//rLLCPwmDutyMoreThan400
    .iMinCompareEn                      (1                         ),//rLLCPwmDutyMoreThan400
    .iErrClear                          (wErrRecovery             ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wBusTotalVoltagePreDiffErrMax2),
    .oErrMin                            (wBusTotalVoltageDiffErrMin2) 
);

always @(posedge iSysClk) begin
    rBusTotalVoltagePreDiffOverErr2 <= wBusTotalVoltagePreDiffErrMax2;
    rBusTotalVoltageDiffOverErr2    <= wBusTotalVoltageDiffErrMin2;
end
//******************************************************************************

assign wVoltageErr = (rLlcBusVoltageAOverErr || rLlcBusVoltageBOverErr || rBusVoltageAOverErr || rBusVoltageBOverErr || rBusTotalVoltageOverErr || rBusTotalVoltagePreOverErr 
                      ||rBusTotalVoltageDiffOverErr1 || rBusTotalVoltagePreDiffOverErr1 || rBusTotalVoltageDiffOverErr2 || rBusTotalVoltagePreDiffOverErr2)
                    ||(rLlcBusVoltageALowErr || rLlcBusVoltageBLowErr || rBusVoltageALowErr || rBusVoltageBLowErr || rBusTotalVoltageLowErr || rBusTotalVoltagePreLowErr);



//******************************************************************************
//******************************************************************************
//充放电过载  持续时间60s 60_000_000/18.73us ≈ 3203417
ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (3203417                   ) 
)
DsgOverloadAlarm(
    .iClk                               (iSysClk                   ),
    .iValue                             (iDcDcPower                ),
    .iMaxThreshold                      (DISCHARGE_POWER_MAX1      ),
    .iMinThreshold                      (DISCHARGE_POWER_MAX1      ),
    .iMaxCompareEn                      (1'b1                      ),
    .iMinCompareEn                      (1'b1                      ),
    .iErrClear                          (wDsgOverloadAlarmClear    ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wDsgOverloadAlarm         ),
    .oErrMin                            (wDsgOverloadAlarmClear    ) 
);

always @(posedge iSysClk) begin
    rDsgOverloadAlarm <= wDsgOverloadAlarm;
end

ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (3203417                   ) 
)
ChgOverloadAlarm(
    .iClk                               (iSysClk                   ),
    .iValue                             (iDcDcPower                ),
    .iMaxThreshold                      (CHARGE_POWER_MAX1         ),
    .iMinThreshold                      (CHARGE_POWER_MAX1         ),
    .iMaxCompareEn                      (1'b1                      ),
    .iMinCompareEn                      (1'b1                      ),
    .iErrClear                          (wChgOverloadAlarmClear    ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wChgOverloadAlarmClear    ),
    .oErrMin                            (wChgOverloadAlarm         ) 
);

always @(posedge iSysClk) begin
    rChgOverloadAlarm <= wChgOverloadAlarm;
end
//充放电过载  持续时间5s 5000000/18.73us ≈ 266952
ThresholdComparCore#(
    .DATA_WIDTH                         (16                        ),
    .GLITCH_FILTER_NUM                  (266952                    ) 
)
OverloadErr(
    .iClk                               (iSysClk                   ),
    .iValue                             (iDcDcPower                ),
    .iMaxThreshold                      (DISCHARGE_POWER_MAX2      ),
    .iMinThreshold                      (CHARGE_POWER_MAX2         ),
    .iMaxCompareEn                      (1                         ),
    .iMinCompareEn                      (1                         ),
    .iErrClear                          (wErrRecovery             ),
    .iADCTrigger                        (iAdcSampleSequenceDone    ),
              
    .oErrMax                            (wDsgOverloadErr              ),
    .oErrMin                            (wChgOverloadErr              ) 
);

always @(posedge iSysClk) begin
    rChgOverloadErr <= wChgOverloadErr;
    rDsgOverloadErr <= wDsgOverloadErr;
end
//******************************************************************************

//******************************************************************************
//硬件过流 过压信号
always @(posedge iSysClk) begin
    if(!iProtectEn) 
        rHardwareOverCurrentCnt[7:0] <= 8'b0;
    else begin
        if(iLlcOverCurrent == 1'b1)
            rHardwareOverCurrentCnt[7:0] <= rHardwareOverCurrentCnt[7:0] + 1'b1;
        else
            rHardwareOverCurrentCnt[7:0] <= 8'b0;
    end    
end  

always @(posedge iSysClk ) begin
    // if(wErrRecovery)
    //     rHardwareOverCurrentErr <= 1'b0;
    // else 
    if(rHardwareOverCurrentCnt[7:0]>=3)
        rHardwareOverCurrentErr <= 1'b1;
    else
        rHardwareOverCurrentErr <= rHardwareOverCurrentErr;
end

always @(posedge iSysClk) begin
    if(!iProtectEn)
        rHardwareOverVoltageCntL[8:0] <= 9'd0;
    else begin
        if(iLlcOverVoltage == 1'b1)
            rHardwareOverVoltageCntL[8:0] <= rHardwareOverVoltageCntL[8:0] + 1'b1;
        else
            rHardwareOverVoltageCntL[8:0] <= 9'd0;     
    end   
end

always @(posedge iSysClk) begin
    if(!iProtectEn)
        rHardwareOverVoltageCntH[9:0] <= 10'd0;
    else begin
        if(iLlcOverVoltage == 1'b1)begin
            if(rHardwareOverVoltageCntL[8:0] == 9'h1ff)  
                rHardwareOverVoltageCntH[9:0] <= rHardwareOverVoltageCntH[9:0] + 1'b1;
            else
                rHardwareOverVoltageCntH[9:0] <= rHardwareOverVoltageCntH[9:0];
        end
        else
            rHardwareOverVoltageCntH[9:0] <= 10'd0; 
    end
end

always @(posedge iSysClk ) begin
    // if(wErrRecovery)
    //     rHardwareOverVoltageErr <= 1'b0;
    // else 
    if(rHardwareOverVoltageCntH[9:0] == 10'h3ff)
        rHardwareOverVoltageErr <= 1'b1;
    else
        rHardwareOverVoltageErr <= rHardwareOverVoltageErr;
end

//******************************************************************************

always @(posedge iSysClk ) begin
    rDcdcErr <= rRelayLogicErr |wVoltageErr | wCurrentErr | rHardwareOverVoltageErr | rHardwareOverCurrentErr | rBmsErr | rNtcTempErr | rDsgOverloadErr | rChgOverloadErr;
end

assign oErrDisplay[1:0] = {(rRelayLogicErr | wVoltageErr | wCurrentErr | rHardwareOverVoltageErr | rHardwareOverCurrentErr ),rBmsErr};

assign wRestorableErr = rDsgOverloadErr | rChgOverloadErr | rBusTotalVoltageOverErr | rBusTotalVoltageLowErr | rBusTotalVoltagePreOverErr | rBusTotalVoltagePreLowErr | 
                        rBusTotalVoltagePreDiffOverErr1 | rBusTotalVoltageDiffOverErr1 | rBusTotalVoltagePreDiffOverErr2 | rBusTotalVoltageDiffOverErr2;
//DEBUG: 故障暂不恢复
//******************************************************************************
 FailureRecovery # (
    .FAILURE_RECOVERY_DELAY    (20                       ),  //故障恢复延时（秒）
    .FAILURE_RECOVERY_TIMES    (2                        ),  //周期内最大允许故障次数，其中0表示无次数限制
    .FAILURE_STATISTICAL_PERIOD(600                      )   //统计周期，单位秒                
 )ErrRecovery
 (
     //Global Clock
     .iSysClk                  (iSysClk                  ),
     .iSysClk1SPulse           (iSysClk1SPulse           ),
     .iDcdcErr                 (wRestorableErr           ),
     .oFailRecovery            (wErrRecovery             )
 );
//assign wErrRecovery = 1'b0; 
//******************************************************************************


assign oDcdcErr                     =   rDcdcErr;
assign oErrRecovery                 =   wErrRecovery;
 
endmodule



