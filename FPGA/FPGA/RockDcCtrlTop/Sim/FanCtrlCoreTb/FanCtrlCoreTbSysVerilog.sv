`timescale 1ns / 1ns
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/10/29 21:55:00
// Design Name: 
// Module Name: CubeRootNewtonTb
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


module FanCtrlCoreTb();

// 时钟和复位信号
    reg                                 SysClk50M                 ='d0;
    reg                                 SysClk100M                ='d0;
    reg                                 SysClk1000M               ='d0;
    reg                                 iSysRst                   ='d1;

// 输入信号
    reg     signed     [  15: 0]        iNtcTemp1                 ='sd0;
    reg     signed     [  15: 0]        iNtcTemp2                 ='sd0;
    reg                                 iFanFreGen                ='d0;
    reg                                 iSelfInspectionEn         ='d0;

// 输出信号
    wire    signed     [   7: 0]        oFanSpeed                   ;
    wire                                oFanPwm                     ;
    wire                                oFanErr                     ;

    localparam                          MAX_SPEED                   = 7500                 ;

// 实例化被测模式
FanCtrlCore FanCtrlCore(
    .iSysClk                            (SysClk50M                 ),// 时钟输入，例如50MHz
    .iSysRst                            (iSysRst                   ),// 低有效复位
    .iSysClk2                           (SysClk100M                ),
    .iNtcTemp1                          (iNtcTemp1                 ),//iSysClk2时钟域
    .iNtcTemp2                          (iNtcTemp2                 ),//iSysClk2时钟域
    .iFanFreGen                         (iFanFreGen                ),
    .iSelfInspectionEn                  (iSelfInspectionEn         ),//系统自检使能
    .iFanForceEn                        (                          ),

    .oFanPwm                            (oFanPwm                   ),// PWM输出
    .oFanSpeed                          (oFanSpeed                 ),
    .oFanErr                            (oFanErr                   ) 

);

// 时钟生成
always #10 SysClk50M = ~SysClk50M;
always #5  SysClk100M = ~SysClk100M;
always #0.5  SysClk1000M = ~SysClk1000M;
assign FanSpeedRpm = oFanSpeed * 60 ;

// 主测试流程
initial begin
    // 初始化信号
    SysClk50M = 0;
    SysClk100M=0;
    SysClk1000M=0;
    iSysRst = 1;
    iNtcTemp1 = -16'sd0;
    iNtcTemp1 = -16'sd0;
    PwmDuty = 'd0;
    // 复位
    #20;
    iSysRst = 0;
    #200;
    iSelfInspectionEn = 1'b1;
    #20;
    iSelfInspectionEn = 1'b0;
    #1_000_000;
    
    iNtcTemp1 = 16'sd400;
    iNtcTemp2 = 16'sd480;
    $display("变温度1,iNtcTemp1 = 40℃,iNtcTemp2 = 48℃");
    #10_000_000;
    $display("[Phase %0d] PWM占空比", PwmDuty);
    $display("[Phase %0d] 风扇转速", FanSpeedRpm);
    
    iNtcTemp1 = 16'sd430;
    iNtcTemp2 = 16'sd510;
    $display("变温度2,iNtcTemp1 = 43℃,iNtcTemp2 = 51℃");
    #10_000_000;
    $display("[Phase %0d] PWM占空比", PwmDuty);
    $display("[Phase %0d] 风扇转速", FanSpeedRpm);
    
    iNtcTemp1 = 16'sd490;
    iNtcTemp2 = 16'sd300;
    $display("变温度3,iNtcTemp1 = 49℃,iNtcTemp2 = 30℃");
    #10_000_000;
    $display("[Phase %0d] PWM占空比", PwmDuty);
    $display("[Phase %0d] 风扇转速", FanSpeedRpm); 
    
    iNtcTemp1 = 16'sd400;
    iNtcTemp2 = 16'sd479;
    $display("变温度4,iNtcTemp1 = 40℃,iNtcTemp2 = 479℃");
    #10_000_000;
    $display("[Phase %0d] PWM占空比", PwmDuty);
    $display("[Phase %0d] 风扇转速", FanSpeedRpm);
    
    iNtcTemp1 = 16'sd590;
    iNtcTemp2 = 16'sd480;
    $display("变温度5,iNtcTemp1 = 59℃,iNtcTemp2 = 59℃");
    #10_000_000;
    $display("[Phase %0d] PWM占空比", PwmDuty);
    $display("[Phase %0d] 风扇转速", FanSpeedRpm);
    
    iNtcTemp1 = 16'sd550;
    iNtcTemp2 = 16'sd480;
    $display("变温度6,iNtcTemp1 = 48℃,iNtcTemp2 = 55℃");
    #10_000_000;
    $display("[Phase %0d] PWM占空比", PwmDuty);
    $display("[Phase %0d] 风扇转速", FanSpeedRpm);

    iNtcTemp1 = 16'sd520;
    iNtcTemp2 = -16'sd480;
    $display("变温度7,iNtcTemp1 = 52℃,iNtcTemp2 = -48℃");
    #10_000_000;
    $display("[Phase %0d] PWM占空比", PwmDuty);
    $display("[Phase %0d] 风扇转速", FanSpeedRpm);

    iNtcTemp1 = 16'sd510;
    iNtcTemp2 = -16'sd400;
    $display("变温度8,iNtcTemp1 = 51℃,iNtcTemp2 = -40℃");
    #10_000_000;
    $display("[Phase %0d] PWM占空比", PwmDuty);
    $display("[Phase %0d] 风扇转速", FanSpeedRpm);

    iNtcTemp1 = 16'sd490;
    iNtcTemp2 = -16'sd400;
    $display("变温度9,iNtcTemp1 = 49℃,iNtcTemp2 = -40℃");
    #10_000_000;
    $display("[Phase %0d] PWM占空比", PwmDuty);
    $display("[Phase %0d] 风扇转速", FanSpeedRpm);

    iNtcTemp1 = 16'sd470;
    iNtcTemp2 = -16'sd400;
    $display("变温度10,iNtcTemp1 = 47℃,iNtcTemp2 = -40℃");
    #10_000_000;
    $display("[Phase %0d] PWM占空比", PwmDuty);
    $display("[Phase %0d] 风扇转速", FanSpeedRpm);

    $finish;
end

reg  [32:0]  PwmHighCnt =32'd0;
reg          rFanPwm = 1'b0;
reg  [7:0]   PwmDuty ='d0;
//PWM占空比检测
always @(posedge SysClk50M) begin
  if(!oFanPwm)
    PwmHighCnt <=  'd0;
  else if(PwmHighCnt == 2000)
    PwmHighCnt <=  PwmHighCnt;
  else
    PwmHighCnt <=  PwmHighCnt + 'd1;       
end

always @(posedge SysClk50M) begin
    rFanPwm <= oFanPwm;
end
always @(posedge SysClk50M) begin
 PwmDuty <= ( ((!oFanPwm) &rFanPwm) || (PwmHighCnt == 2000) ) ? 50_000_000/25_000/PwmHighCnt : PwmDuty;
end
//******TACH生成
reg  [63:0]  TACHFreqPeriod ;
reg  [63:0]  rTACHFreqPeriod ;
reg  [63:0]  TACHPreiodCnt =64'd0;
always @(*) begin
    case(PwmDuty)
       0: TACHFreqPeriod <= 0;
       1: TACHFreqPeriod <= 10_000_000/(MAX_SPEED/60 * 2);//1_000_000_000/(MAX_SPEED/60 * 2);   //减小100倍，提升仿真速率
       2: TACHFreqPeriod <= 10_000_000/(MAX_SPEED/2/60 * 2);
       4: TACHFreqPeriod <= 10_000_000/(MAX_SPEED/4/60 * 2);
       default:TACHFreqPeriod <= 64'd0;
    endcase
end


always @(posedge SysClk1000M) begin
    if(TACHFreqPeriod == 0)
       iFanFreGen <= 1'b0;
    else if(TACHPreiodCnt== TACHFreqPeriod/2 || TACHPreiodCnt== TACHFreqPeriod )
        iFanFreGen <= ~iFanFreGen;     
end
//assign iFanFreGen = TACHFreq == 0 ? 0 :( (TACHPreiodCnt== 1/TACHFreq) ? ~iSelfInspectionEn :iSelfInspectionEn );

always @(posedge SysClk50M) begin
    rTACHFreqPeriod <= TACHFreqPeriod;
end

always @(posedge SysClk1000M)begin
   if(rTACHFreqPeriod!= TACHFreqPeriod)
     TACHPreiodCnt <= 64'd0;
    else begin
      if(TACHPreiodCnt == TACHFreqPeriod)
        TACHPreiodCnt <= 64'd0;
      else
        TACHPreiodCnt <= TACHPreiodCnt + 1'd1;
    end
end

endmodule