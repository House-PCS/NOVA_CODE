//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   FpgaParamCore.v
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

`include "FunctionDefine.h"
`timescale 1ns/1ns
module FpgaParamCore
(
    //Global Clock
    input                                   iSysClk                                 ,
    
    //output              [(2*32 - 1):0]      oFpgaParam                              ,
    output                                  oFpgaParamBusClk                        ,
    output              [31:0]              oFpgaParamBusAddr                       ,
    output                                  oFpgaParamBusWrEn                       ,
    output              [15:0]              oFpgaParamBusWrData                     ,

    input               [15:0]              iFpgaParamBusRdData                     ,
    //ParamWr

    input  signed       [15:0]              iDCOutputTotalVoltage                   ,
    input  signed       [15:0]              iDCOutputTotalPower                     ,

    input  signed       [15:0]              iDCHeatSinkTemp1                        ,
    input  signed       [15:0]              iDCHeatSinkTemp2                        ,
    input               [15:0]              iDCFailureInformation1                  ,
    input               [15:0]              iDCFailureInformation2                  ,
    input               [15:0]              iDCFailureInformation3                  ,
    input               [15:0]              iDCAlarmInformation1                    ,    
    input               [15:0]              iFan1Status                             ,
    input               [15:0]              iFan2Status                             ,                  
    input  signed       [15:0]              iLlc1OutputVoltage                      ,
    input  signed       [15:0]              iLlc1OutputCurrent                      ,
    input  signed       [15:0]              iLlc2OutputVoltage                      ,
    input  signed       [15:0]              iLlc2OutputCurrent                      ,
    input  signed       [15:0]              iBuckBoost1OutputVoltage                ,
    input  signed       [15:0]              iBuckBoost1OutputCurrent                ,
    input  signed       [15:0]              iBuckBoost2OutputVoltage                ,
    input  signed       [15:0]              iBuckBoost2OutputCurrent                ,
    input                [1:0]              iDcdcWorkStatus                         ,
    input                [1:0]              iPcsHardwareId                          ,
    input                                   iSeriesRelayEn                          ,
    //ParamRd
   // output                          oSysStable                                      ,
    output                          oSwitchOnOff                                    ,
    output                          oForwardEn                                      ,
    output      [31:0]              oBoardIp                                        ,
    output                          oDCOnOff                                        ,
    output                          oEqualEnableCharge                              ,
    output                          oEqualEnableDischarge                           ,
    output      [1:0]               oPcsSoftWareId                                  ,
    output                          oClearOfFault                                   ,
    output      [15:0]              oUpdataStatus                                   ,
    output      [15:0]              oEqualizationEnable                             ,

    output      [7:0]               oLedDir                                         ,
    output      [7:0]               oLedRValue                                      ,
    output      [7:0]               oLedGValue                                      ,
    output      [7:0]               oLedBValue                                      ,
    output      [15:0]              oLedMode                                        ,
    output      [15:0]              oLedPeriod                                      ,

    output                          oBoardIpUpdata                                  ,
    output                          oEthUdpRstN                                     ,
    output      [7:0]               oManualReboot                                   ,
    output                          oFanForceEn                                     ,
    //FaultRecording
    input                           iMcuFlashBurnEn                                 ,         
    input       [26:0]              iMcuRdStartAddr                                 ,
    output                          oMcuFaultBurnFinish                             
);

//3002_0000
localparam  READ_ONLY_OFFSET_ADDR          = 11'h000;
localparam  READ_WRITE_ONLY_OFFSET_ADDR    = 11'h100;

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    reg         [7:0]               rFpgaParamBusAddr     [1:0]             ;
    reg         [(2*32 - 1):0]      rFpgaParam          = 'd0               ;
    reg         [7:0]               rFpgaParamBusAddrBuf  [1:0]             ;
    reg         [15:0]              rFpgaParamBusWrData = 16'd0             ;
    reg                             rFpgaParamBusWrEn                       ;

    reg                             rSysStable          = 1'b0              ;

    reg         [15:0]              rForwardTransfer    = 16'd0             ;
    reg         [1:0]               rBoardIpUpdata      = 2'd0              ;
    wire                            wBoardIpUpdata                          ;

    reg                             rEthUdpRstN         = 1'b1              ;
    reg         [15:0]              rBoardIpUpdataBuf   = 16'd0             ;

    reg         [15:0]              rSwitchOnOff             ='d0           ;
    reg         [31:0]              rBoardIp                 ='d0           ;
    reg         [15:0]              rDCOnOff                 ='d0           ;
    reg         [15:0]              rEqualizationEnable      ='d0           ;
    reg         [15:0]              rPcsDeviceType           ='d0           ;
    reg         [15:0]              rClearOfFault            ='d0           ;
    reg         [7:0]               rFanForceEn              ='d0           ;
    reg         [15:0]              rDcdcMaxChargeVoltage    ='d0           ;
    reg         [15:0]              rDcdcMaxChargeCurrent    ='d0           ;
    reg         [15:0]              rDcdcMaxChargePower      ='d0           ;
    reg         [15:0]              rDcdcMaxDischargeVoltage ='d0           ;
    reg         [15:0]              rDcdcMaxDischargeCurrent ='d0           ;
    reg         [15:0]              rDcdcMaxDischargePower   ='d0           ;
    reg         [15:0]              rUpdataStatus            ='d0           ;
    reg         [7:0]               rLedDir                  ='d0           ;
    reg         [7:0]               rLedRValue               ='d0           ;
    reg         [7:0]               rLedGValue               ='d0           ;
    reg         [7:0]               rLedBValue               ='d0           ;
    reg         [15:0]              rLedMode                 ='d0           ;
    reg         [15:0]              rLedPeriod               ='d0           ;
    reg         [15:0]              rManualReboot            ='d0           ;
    reg                             rMcuFaultBurnFinish      =1'b0          ;
    reg  signed [15:0]              rDCOutputTotalCurrent    =16'sd0        ;
    reg  signed [15:0]              rDCOutputTotalPower      =16'sd0        ;
    wire signed [15:0]              wDCOutputTotalPower                     ;
//******************************************************************************

always @ (posedge iSysClk) begin
    rDCOutputTotalCurrent <=  iSeriesRelayEn ? (iBuckBoost1OutputCurrent + iBuckBoost2OutputCurrent) >>>1 : iBuckBoost1OutputCurrent + iBuckBoost2OutputCurrent;
end

always @(posedge iSysClk) begin
    rDCOutputTotalPower <= iSeriesRelayEn ? iDCOutputTotalPower >>>1 : iDCOutputTotalPower;
end
//******************************************************************************
//

always @ (posedge iSysClk) begin
    rFpgaParamBusAddr[0][7:0] <= rFpgaParamBusAddr[0][7:0] + 11'd1;
end

always @ (posedge iSysClk) begin
    rFpgaParamBusAddr[1][7:0] <= rFpgaParamBusAddr[0][7:0];
end

//******************************************************************************

//******************************************************************************
//

assign oFpgaParamBusClk = iSysClk;

//3200_0000
assign oFpgaParamBusAddr[31:0] = {16'h3002, 8'h00, rFpgaParamBusAddr[1][7:0]};

//******************************************************************************

//******************************************************************************
//

//assign oFpgaParam[(2*32 - 1):0] = rFpgaParam[(2*32 - 1):0];

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    rFpgaParamBusAddrBuf[0][7:0] <= rFpgaParamBusAddr[1][7:0];
    rFpgaParamBusAddrBuf[1][7:0] <= rFpgaParamBusAddrBuf[0][7:0];
end

//******************************************************************************

//******************************************************************************
always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0] == 8'h90)
        rBoardIp[31:16] <= iFpgaParamBusRdData[15:0];
    else
        rBoardIp[31:16] <= rBoardIp[31:16];
end

always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0] == 8'h91)
        rBoardIp[15:0] <= iFpgaParamBusRdData[15:0];
    else
        rBoardIp[15:0] <= rBoardIp[15:0];
end

always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0] == 8'h90)
        rBoardIpUpdata[0] <= (iFpgaParamBusRdData[15:0] != rBoardIp[31:16]);
    else if(rFpgaParamBusAddrBuf[1][7:0] == 8'h92)
        rBoardIpUpdata[0] <= 1'd0;
    else
        rBoardIpUpdata[0] <= rBoardIpUpdata[0];
end

always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0] == 8'h91)
        rBoardIpUpdata[1] <= (iFpgaParamBusRdData[15:0] != rBoardIp[15:0]);
    else if(rFpgaParamBusAddrBuf[1][7:0] == 8'h92)
        rBoardIpUpdata[1] <= 1'd0;
    else
        rBoardIpUpdata[1] <= rBoardIpUpdata[1];
end

assign oBoardIp[31:0] = rBoardIp[31:0];
assign wBoardIpUpdata = rBoardIpUpdata[0] || rBoardIpUpdata[1];
assign oBoardIpUpdata = wBoardIpUpdata;

always @ (posedge iSysClk) begin
    rBoardIpUpdataBuf[15:0] <= {rBoardIpUpdataBuf[14:0], wBoardIpUpdata};
end

always @ (posedge iSysClk) begin
    if(rBoardIpUpdataBuf[7])
        rEthUdpRstN <= 1'b0;
    else if(rBoardIpUpdataBuf[15])
        rEthUdpRstN <= 1'b1;
    else
        rEthUdpRstN <= rEthUdpRstN;
end

assign oEthUdpRstN = rEthUdpRstN;
//******************************************************************************

//******************************************************************************
always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0] == 8'h92)
        rPcsDeviceType[15:0] <= iFpgaParamBusRdData[15:0];
end

assign  oPcsSoftWareId    =   rPcsDeviceType[1:0];
//******************************************************************************

//******************************************************************************
//******************************************************************************
//32'h3002_011E -- 32'h3002_0125
always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0] == 8'h93)
        rLedDir[7:0] <= iFpgaParamBusRdData[7:0];
end

always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0] == 8'h93)
        rLedRValue[7:0] <= iFpgaParamBusRdData[15:8];
end

always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0] == 8'h94)
        rLedGValue[7:0] <= iFpgaParamBusRdData[7:0];
end

always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0]== 8'h94)
        rLedBValue[7:0] <= iFpgaParamBusRdData[15:8];
end

always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0]== 8'h95)
        rLedMode[15:0] <= iFpgaParamBusRdData[15:0];
end

always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0]== 8'h96)
        rLedPeriod[15:0] <= iFpgaParamBusRdData[15:0];
end

assign oLedDir[7:0]      = rLedDir[7:0]      ;
assign oLedRValue[7:0]   = rLedRValue[7:0]   ;
assign oLedGValue[7:0]   = rLedGValue[7:0]   ;
assign oLedBValue[7:0]   = rLedBValue[7:0]   ;
assign oLedMode  [15:0]  = rLedMode  [15:0]  ;
assign oLedPeriod[15:0]  = rLedPeriod[15:0]  ;
//******************************************************************************

//******************************************************************************
always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0]== 8'h97)
        rManualReboot[15:0] <= iFpgaParamBusRdData[15:0];
end

assign oManualReboot[7:0] = rManualReboot[7:0];
//******************************************************************************

//******************************************************************************
always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0]== 8'h98)
        rMcuFaultBurnFinish <= iFpgaParamBusRdData[0];
end

assign oMcuFaultBurnFinish = rMcuFaultBurnFinish;
//******************************************************************************

//******************************************************************************      
always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0]== 8'hA0)
        rSwitchOnOff[15:0] <= iFpgaParamBusRdData[15:0];
end

assign oSwitchOnOff = (rSwitchOnOff[0]);
//******************************************************************************

//******************************************************************************
always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0]== 8'hA1)
        rForwardTransfer[15:0] <= iFpgaParamBusRdData[15:0];
    else
        rForwardTransfer[15:0] <= rForwardTransfer[15:0];
end

assign oForwardEn = (rForwardTransfer[15:0] == 16'd1);

//******************************************************************************

//******************************************************************************
always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0]== 8'hA2)
        rDCOnOff[15:0] <= iFpgaParamBusRdData[15:0];
end

assign oDCOnOff = (rDCOnOff[0]);
//******************************************************************************

//******************************************************************************
//32'h3002_010A -- 32'h3002_010B

always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0]== 8'hA3)
        rEqualizationEnable[15:0] <= iFpgaParamBusRdData[15:0];
end

assign oEqualEnableCharge       = ($signed(rEqualizationEnable[15:0]) == 16'sd1);
assign oEqualEnableDischarge    = ($signed(rEqualizationEnable[15:0]) == -16'sd1);
assign oEqualizationEnable[15:0]        =   rEqualizationEnable[15:0]       ;
//******************************************************************************

//******************************************************************************
always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0]== 8'hA4)
        rUpdataStatus[15:0] <= iFpgaParamBusRdData[15:0];
end

assign oUpdataStatus[15:0]  = rUpdataStatus[15:0];
//******************************************************************************

//******************************************************************************
always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0]== 8'hA5)
        rClearOfFault[15:0] <= iFpgaParamBusRdData[15:0];
end

assign oClearOfFault = (rClearOfFault[15:0] == 16'h1) ;
//******************************************************************************
//******************************************************************************
always @ (posedge iSysClk) begin
    if(rFpgaParamBusAddrBuf[1][7:0]== 8'hA6)
        rFanForceEn[7:0] <= iFpgaParamBusRdData[7:0];
end
      
assign oFanForceEn = rFanForceEn[7:0] == 8'h55;
//******************************************************************************
//******************************************************************************
//3002_0020-3002_008f 参数写入寄存器中
//3002_0020-3002_008f 参数写入寄存器中
always @ (posedge iSysClk) begin
    if( (rFpgaParamBusAddr[0][7:0] >= 8'h0B) && (rFpgaParamBusAddr[0][7:0] < 8'h90) || (rFpgaParamBusAddr[0][7:0] > 8'hAF))
        rFpgaParamBusWrEn<= 1'b1;
    else
        rFpgaParamBusWrEn<= 1'b0;
end
always @ (posedge iSysClk) begin
    case(rFpgaParamBusAddr[0][7:0])
        8'h0B:    rFpgaParamBusWrData[15:0] <= 16'h0002;
        8'h0C:    rFpgaParamBusWrData[15:0] <= 16'h0000;
        8'h0D:    rFpgaParamBusWrData[15:0] <= 16'h0105;              
        `ifdef BOOTMODE
        8'h1A:    rFpgaParamBusWrData[15:0] <= 16'h0000;
        `else
        8'h1A:    rFpgaParamBusWrData[15:0] <= 16'h0001;
        `endif
        8'h20:    rFpgaParamBusWrData[15:0] <= iDCOutputTotalVoltage[15:0];
        8'h21:    rFpgaParamBusWrData[15:0] <= rDCOutputTotalCurrent[15:0];
        8'h22:    rFpgaParamBusWrData[15:0] <= rDCOutputTotalPower[15:0];
        8'h23:    rFpgaParamBusWrData[15:0] <= iLlc1OutputVoltage[15:0];
        8'h24:    rFpgaParamBusWrData[15:0] <= iLlc1OutputCurrent[15:0];
        8'h25:    rFpgaParamBusWrData[15:0] <= iLlc2OutputVoltage[15:0];
        8'h26:    rFpgaParamBusWrData[15:0] <= iLlc2OutputCurrent[15:0];
        8'h27:    rFpgaParamBusWrData[15:0] <= iBuckBoost1OutputVoltage[15:0];
        8'h28:    rFpgaParamBusWrData[15:0] <= iBuckBoost1OutputCurrent[15:0];
        8'h29:    rFpgaParamBusWrData[15:0] <= iBuckBoost2OutputVoltage[15:0];
        8'h2A:    rFpgaParamBusWrData[15:0] <= iBuckBoost2OutputCurrent[15:0];
        8'h40:    rFpgaParamBusWrData[15:0] <= iFan1Status[15:0];
        8'h44:    rFpgaParamBusWrData[15:0] <= iDCHeatSinkTemp1[15:0];
        8'h45:    rFpgaParamBusWrData[15:0] <= iDCHeatSinkTemp2[15:0];
        8'h48:    rFpgaParamBusWrData[15:0] <= {14'd0,iDcdcWorkStatus[1:0]};
        8'h49:    rFpgaParamBusWrData[15:0] <= {14'd0,iPcsHardwareId[1:0]};
        8'h60:    rFpgaParamBusWrData[15:0] <= iDCFailureInformation1[15:0];
        8'h61:    rFpgaParamBusWrData[15:0] <= iDCFailureInformation2[15:0];
        8'h62:    rFpgaParamBusWrData[15:0] <= iDCFailureInformation3[15:0];
        8'h65:    rFpgaParamBusWrData[15:0] <= iDCAlarmInformation1[15:0];
        8'h70:    rFpgaParamBusWrData[15:0] <= {14'd0,iMcuFlashBurnEn};
        8'h71:    rFpgaParamBusWrData[15:0] <= {5'd0,iMcuRdStartAddr[25:15]};
        8'h72:    rFpgaParamBusWrData[15:0] <= {iMcuRdStartAddr[14:0],1'b0};  
        default: rFpgaParamBusWrData[15:0] <=  16'h0000;
    endcase 
end


assign  oFpgaParamBusWrEn            =  rFpgaParamBusWrEn           ;
assign  oFpgaParamBusWrData [15:0]   =  rFpgaParamBusWrData [15:0]  ;

endmodule
