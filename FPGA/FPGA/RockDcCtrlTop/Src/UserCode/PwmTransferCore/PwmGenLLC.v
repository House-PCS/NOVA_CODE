//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   PwmGenLLC.v
//Original Author   :   Xcy Nova003107
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
module PwmGenLLC #(
    parameter  [15:0] PWM_CYCLE    = 16'd1000,     // Max : 1000~65535  200KHz ~ 3052Hz
    parameter  [7:0]  DEAD_CYCLE   = 8'd100        // Time: 0-255 *5ns
    )
    (
     input  wire       iSysClk       , //200Mhz 5ns    
     input  wire       iSysRst       , //复位
     input  wire       iPWMEn        ,
     input  wire       iClk200M      , //200Mhz 5ns
     input  wire [9:0] iPwmDuty      , //0-3FF（0-1023） --> 0-50% 
     input  wire [9:0] iPwmPhase     , //0-3FF（0-1023） --> 0-180° 
     output wire       oPwm1PSignal  ,
     output wire       oPwm1NSignal  ,
     output wire       oPwm2PSignal  ,
     output wire       oPwm2NSignal  
    );

localparam  [7:0] L_DEAD_CYCLE  = DEAD_CYCLE >= 1 ?  DEAD_CYCLE - 1'b1 : 1'b1;

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
reg [L_DEAD_CYCLE : 0] rPwm1PSignalBuf    ;
reg [L_DEAD_CYCLE : 0] rPwm2PSignalBuf    ;
reg [L_DEAD_CYCLE : 0] rPwm1NSignalBuf    ;
reg [L_DEAD_CYCLE : 0] rPwm2NSignalBuf    ;


wire [25:0]       wPulseWidth                 ;
wire [25:0]       wPhasePoint                 ;

reg  [15:0]       CycleCnt          = 16'd0   ;
reg  [15:0]       rRealPulseWidth1  = 16'd0   ;
reg  [15:0]       rRealPulseWidth2  = 16'd0   ;
reg  [25:0]       rPulseWidth       = 34'd0   ;
reg               rPwm1Signal       = 1'b0    ;
reg               rPwm2Signal       = 1'b0    ;
reg  [25:0]       rPhasePoint       = 26'd0   ;
reg               rPwm1PSignal      = 1'b0    ;
reg               rPwm1NSignal      = 1'b0    ;
reg               rPwm2PSignal      = 1'b0    ;
reg               rPwm2NSignal      = 1'b0    ;


wire   wPwm1PSignal ;
wire   wPwm2PSignal ;
wire   wPwm1NSignal ;
wire   wPwm2NSignal ;
//******************************************************************************

assign  wPwm1PSignal =   rPwm1Signal   ;
assign  wPwm2PSignal =   rPwm2Signal   ;
assign  wPwm1NSignal = (!rPwm1Signal)  ;
assign  wPwm2NSignal = (!rPwm2Signal)  ;



//  reg [15:0]PWM_CYCLE = 16'd5263;

// //******************************************************************************
// // Debug 线性调频 2s更改一次频率
// //******************************************************************************
// //
// localparam COUNT_1US = 8'd200;
// localparam COUNT_1MS = 10'd1000;
// localparam COUNT_2S =  11'd100;
// reg [7:0]   rCountUs = 'd0;
// reg [9:0]   rCountMs = 'd0;
// reg [10:0]  rCounts  = 'd0;
// wire  wCountUs ;
// wire  wCountMs ;
// wire  wCounts  ;

// always @ (posedge iSysClk) begin
//     if(rCountUs[7:0] == (COUNT_1US - 8'd1))
//         rCountUs[7:0] <= 8'd0;
//     else
//         rCountUs[7:0] <= rCountUs[7:0] + 8'd1;
// end
    
// assign wCountUs = (rCountUs[7:0] == (COUNT_1US - 8'd1));

// //******************************************************************************
// //
// always @ (posedge iSysClk) begin
//     if(wCountUs) begin
//         if(rCountMs[9:0] == (COUNT_1MS - 10'd1))
//             rCountMs[9:0] <= 10'd0;
//         else
//             rCountMs[9:0] <= rCountMs[9:0] + 10'd1;
//     end
//     else
//         rCountMs[9:0] <= rCountMs[9:0];
// end
// assign wCountMs = (wCountUs && (rCountMs[9:0] == (COUNT_1MS - 10'd1)));


// always @ (posedge iSysClk) begin
//     if(wCountMs) begin
//         if(rCounts[10:0] == (COUNT_2S - 11'd1))
//             rCounts[10:0] <= 11'd0;
//         else
//             rCounts[10:0] <= rCounts[10:0] + 11'd1;
//     end
//     else
//         rCounts[10:0] <= rCounts[10:0];
// end
// assign wCounts = (wCountMs && (rCounts[10:0] == (COUNT_2S - 11'd1)));

// always @ (posedge iSysClk) begin
// if(wCounts)begin
//   if(PWM_CYCLE>= 16'd5000)
//     PWM_CYCLE <=  16'd2666;
//   else
//     PWM_CYCLE <= PWM_CYCLE + 10'd10;
//   end
// end
// //******************************************************************************


UnsigMult10x16 U1 (
 .CLK (iSysClk          ),       // input wire CLK
 .A   (iPwmDuty[9:0]    ),   // input wire [9 : 0] A
 .B   (PWM_CYCLE[15:0]  ),      // input wire [23 : 0] B
 .P   (wPulseWidth[25:0])      // output wire [33 : 0] P
);
UnsigMult10x16 U2 (
 .CLK (iSysClk          ),       // input wire CLK
 .A   (iPwmPhase[9:0]   ),  // input wire [9 : 0] A
 .B   (PWM_CYCLE[15:0]  ),      // input wire [23 : 0] B
 .P   (wPhasePoint[25:0])      // output wire [33 : 0] P
);
//******************************************************************************
//锯齿波计数器产生周期计数
always @(posedge iSysClk ) begin
  if(iSysRst | (CycleCnt[15:0] == PWM_CYCLE[15:0]))
    CycleCnt[15:0] <= 24'd0;
  else
    CycleCnt[15:0] <= CycleCnt[15:0] + 1'b1;
end

//******************************************************************************
//产生脉冲宽度
always @(posedge iSysClk ) begin
  if((CycleCnt[15:0]+1'b1 == PWM_CYCLE[15:0])) begin   //在计数器为0的时刻进行装载参数
    if(wPulseWidth[25:11] < L_DEAD_CYCLE[7:0]<<1)      //限制占空比最小为死区的二倍
      rPulseWidth[25:11] <= {8'd0,L_DEAD_CYCLE[7:0],1'b0};
    else if(wPulseWidth[25:11] > (PWM_CYCLE[15:1]-(L_DEAD_CYCLE[7:0]<<1)))
      rPulseWidth[25:11] <= PWM_CYCLE[15:1]/*-(L_DEAD_CYCLE[7:0]<<1)*/;  //为了占空比50%
    else
      rPulseWidth[25:11] <= wPulseWidth[25:11];  
  end
end
//******************************************************************************
//移相
always @(posedge iSysClk ) begin
  if(((CycleCnt[15:0]+1'b1) == PWM_CYCLE[15:0])) begin //在计数器为0的时刻进行装载参数
    rPhasePoint[25:11] <= wPhasePoint[25:11];
  end
end

always @(posedge iSysClk ) begin
  rRealPulseWidth1[15:0] <= {1'b0, rPulseWidth[25:11]} ;//提前进行分频
  //rRealPulseWidth2[15:0] <= {1'b0, PWM_CYCLE[15:1]+PWM_CYCLE[0:0] + rPulseWidth[25:11]};
  rRealPulseWidth2[15:0] <= { rPhasePoint[25:11] + rPhasePoint[11] + rPulseWidth[25:11]};
end    



always @(posedge iSysClk ) begin
  if(iSysRst)
    rPwm1Signal <= 1'b0;
  else if(CycleCnt[15:0] < rRealPulseWidth1[15:0])
    rPwm1Signal <= 1'b1 ;
  else 
    rPwm1Signal <= 1'b0 ;
end

always @(posedge iSysClk ) begin
    if(iSysRst)
      rPwm2Signal <= 1'b0;
    else if(CycleCnt[15:0] < rRealPulseWidth2[15:0] && CycleCnt[15:0]>= {1'b0,rPhasePoint[25:11]})//相位起始点开始置位
    //else if(CycleCnt[15:0] < rRealPulseWidth2[15:0] && CycleCnt[15:0]>= {1'b0,PWM_CYCLE[15:1]}) //计数器最大值的中点开始置位
      rPwm2Signal <= 1'b1 ;
    else 
      rPwm2Signal <= 1'b0 ;
end

always @(posedge iSysClk ) begin
  rPwm1PSignalBuf[L_DEAD_CYCLE : 0] <= {rPwm1PSignalBuf[L_DEAD_CYCLE - 1'b1 : 0] , wPwm1PSignal} ;
  rPwm2PSignalBuf[L_DEAD_CYCLE : 0] <= {rPwm2PSignalBuf[L_DEAD_CYCLE - 1'b1 : 0] , wPwm2PSignal} ;
  rPwm1NSignalBuf[L_DEAD_CYCLE : 0] <= {rPwm1NSignalBuf[L_DEAD_CYCLE - 1'b1 : 0] , wPwm1NSignal} ;
  rPwm2NSignalBuf[L_DEAD_CYCLE : 0] <= {rPwm2NSignalBuf[L_DEAD_CYCLE - 1'b1 : 0] , wPwm2NSignal} ;
end

always @(posedge iSysClk ) begin
  if(iPWMEn == 1'b1)begin
    rPwm1PSignal <=  rPwm1PSignalBuf[L_DEAD_CYCLE] & wPwm1PSignal;
    rPwm2PSignal <=  rPwm2PSignalBuf[L_DEAD_CYCLE] & wPwm2PSignal;
    rPwm1NSignal <=  rPwm1NSignalBuf[L_DEAD_CYCLE] & wPwm1NSignal;
    rPwm2NSignal <=  rPwm2NSignalBuf[L_DEAD_CYCLE] & wPwm2NSignal;
  end
  else begin
    rPwm1PSignal <= 1'b0 ;
    rPwm2PSignal <= 1'b0 ;
    rPwm1NSignal <= 1'b0 ;
    rPwm2NSignal <= 1'b0 ;
  end
end

assign oPwm1PSignal = rPwm1PSignal ;
assign oPwm1NSignal = rPwm1NSignal ;
assign oPwm2PSignal = rPwm2PSignal ;
assign oPwm2NSignal = rPwm2NSignal ;

endmodule
