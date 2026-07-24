`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/04/03 16:09:15
// Design Name: 
// Module Name: PwmTransferCoreTb
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


module PwmTransferCoreTb( );

    // 参数定义


    // 输入信号
    reg iSysClk;                       // 系统时钟
    reg iSysRst;

    reg         rChargeEn           ;                       
    reg [9:0]   rBuckBoost1PwmDuty  ;
    reg [9:0]   rBuckBoost2PwmDuty  ;
    reg [9:0]   rLLC1PwmDuty        ;
    reg [9:0]   rLLC2PwmDuty        ;

    wire  wBuckBoost1PwmOutP  ;
    wire  wBuckBoost1PwmOutN  ;
    wire  wBuckBoost2PwmOutP  ;
    wire  wBuckBoost2PwmOutN  ;
    wire  wLLC1LVPwm1OutP     ;
    wire  wLLC1LVPwm2OutP     ;
    wire  wLLC1HVPwm1OutP     ;
    wire  wLLC1HVPwm2OutP     ;
    wire  wLLC2LVPwm1OutP     ;
    wire  wLLC2LVPwm2OutP     ;
    wire  wLLC2HVPwm1OutP     ;
    wire  wLLC2HVPwm2OutP     ;

    // 实例化被测模块 
PwmTransferCore uut
(
    //Global Clock
    .iSysClk            (iSysClk                ),//时钟2000MHz
    .iSscClk            (iSysClk                ),
    .iSysRst            (iSysRst                ),
    .iLLCPwmEn          (1'b1                   ),
    .iBuckBoostPwmEn    (1'b1                   ),
    
    .iBuckBoost1PwmDuty (rBuckBoost1PwmDuty[9:0]),
    .iBuckBoost2PwmDuty (rBuckBoost2PwmDuty[9:0]),
    .iLLC1PwmDuty       (rLLC1PwmDuty      [9:0]),
    .iLLC2PwmDuty       (rLLC2PwmDuty      [9:0]),
    
    .oBuckBoost1PwmOutP (wBuckBoost1PwmOutP     ),
    .oBuckBoost1PwmOutN (wBuckBoost1PwmOutN     ),
    .oBuckBoost2PwmOutP (wBuckBoost2PwmOutP     ),
    .oBuckBoost2PwmOutN (wBuckBoost2PwmOutN     ),
    
    .oLLC1LVPwm1OutP    (wLLC1LVPwm1OutP        ),
    .oLLC1LVPwm2OutP    (wLLC1LVPwm2OutP        ),
    .oLLC1HVPwm1OutP    (wLLC1HVPwm1OutP        ),
    .oLLC1HVPwm2OutP    (wLLC1HVPwm2OutP        ),
    
    .oLLC2LVPwm1OutP    (wLLC2LVPwm1OutP        ),
    .oLLC2LVPwm2OutP    (wLLC2LVPwm2OutP        ),
    .oLLC2HVPwm1OutP    (wLLC2HVPwm1OutP        ),
    .oLLC2HVPwm2OutP    (wLLC2HVPwm2OutP        )
    
);


    // 时钟生成
    initial begin
        iSysClk = 0;
        forever #2.5 iSysClk = ~iSysClk; // 100MHz 时钟，周期 10ns
    end

    initial begin
        // 初始化
        #10
        iSysRst = 1'b1;
        #50
        iSysRst = 1'b0;
        rBuckBoost1PwmDuty[9:0] = 10'd64;
        rBuckBoost2PwmDuty[9:0] = 10'd64;
        rLLC1PwmDuty      [9:0] = 10'd1;
        rLLC2PwmDuty      [9:0] = 10'd1;
        #200000
        rBuckBoost1PwmDuty[9:0] = 10'd64;
        rBuckBoost2PwmDuty[9:0] = 10'd64;
        rLLC1PwmDuty      [9:0] = 10'd64;
        rLLC2PwmDuty      [9:0] = 10'd64;
        #200000
        rBuckBoost1PwmDuty[9:0] = 10'd940;
        rBuckBoost2PwmDuty[9:0] = 10'd940;
        rLLC1PwmDuty      [9:0] = 10'd940;
        rLLC2PwmDuty      [9:0] = 10'd940;
        #200000
        #50
        iSysRst = 1'b1;
         #5000
        iSysRst = 1'b0;

    end

endmodule