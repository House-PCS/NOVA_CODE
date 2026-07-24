`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/08/06 19:37:17
// Design Name: 
// Module Name: BuckBoostSlowLaunchCtrlCore
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module BuckBoostCtrlCore #(
             CLK_FREQ            =   100
)(
    input                   iSysClk                 ,
    input           [1:0]   iPcsHardwareIdUsing     ,
    input                   iSlowLaunchEn           ,
    input           [9:0]   iLLCPwmDuty             ,

    input   signed  [31:0]  iPidControlPwmParamA    ,
    input   signed  [31:0]  iPidControlPwmParamB    ,

    output          [9:0]   oBuckBoostPwmDutyA      ,
    output          [9:0]   oBuckBoostPwmDutyB      

);


//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
reg     [9:0]   rStartUpPwmDuty         = 10'd0     ;
wire    [9:0]   wBuckBoostPwmMax                    ;

reg     [9:0]   rBuckBoostPwmDutyA       = 10'd0     ;
reg     [9:0]   rPidControlPwmParamA     = 10'd0     ;

reg     [9:0]   rBuckBoostPwmDutyB       = 10'd0     ;
reg     [9:0]   rPidControlPwmParamB     = 10'd0     ;

assign wBuckBoostPwmMax = iSlowLaunchEn ? 10'd900 : 10'd500;

//******************************************************************************
// 互补滤波启动 借助LLC的缓慢启动 
// RealPWM = [PWMBuckBoost*x + rStartUpPwmDuty*(1-x)] 
// x = from 0 to 1
always @(posedge iSysClk ) begin
    if(iPcsHardwareIdUsing == 2'b00)
        rStartUpPwmDuty <= 10'd256;
    else
        rStartUpPwmDuty <= 10'd0;
end

always @(posedge iSysClk ) begin
    if(iPidControlPwmParamA > wBuckBoostPwmMax)
        rPidControlPwmParamA <= wBuckBoostPwmMax;
    else
        rPidControlPwmParamA <= iPidControlPwmParamA;
end

always @(posedge iSysClk ) begin
    if(iPidControlPwmParamB > wBuckBoostPwmMax)
        rPidControlPwmParamB <= wBuckBoostPwmMax;
    else
        rPidControlPwmParamB <= iPidControlPwmParamB;
end

always @(posedge iSysClk) begin
   //rBuckBoostPwmDutyA <= ((rPidControlPwmParamA * iLLCPwmDuty + rStartUpPwmDuty * (1023 - iLLCPwmDuty))>>10);
   //rBuckBoostPwmDutyB <= ((rPidControlPwmParamB * iLLCPwmDuty + rStartUpPwmDuty * (1023 - iLLCPwmDuty))>>10);
   rBuckBoostPwmDutyA <= rPidControlPwmParamA;
   rBuckBoostPwmDutyB <= rPidControlPwmParamB;
end


assign oBuckBoostPwmDutyA = rBuckBoostPwmDutyA;
assign oBuckBoostPwmDutyB = rBuckBoostPwmDutyB;

//******************************************************************************

endmodule
