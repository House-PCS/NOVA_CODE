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
module PwmGenLLCNew #(
    parameter  [15:0] PWM_CYCLE       = 200000/55,  // Max : 1000~65535  200KHz ~ 3052Hz
    parameter  [8:0]  DEAD_CYCLE      = 9'd300   ,  // Time: 0-255 *5ns
    parameter  [8:0]  COUNTER_PHASE   = 9'd90       // 0 90 180 270 
    )
    (
     input  wire       iSysClk       , //200Mhz 5ns    
     input  wire       iSysRst       , //复位
     input  wire       iPWMEn        ,
     input  wire [9:0] iPwmDuty      , //0-3FF（0-1023） --> 0-50% 
     output wire       oPwm13Signal  ,
     output wire       oPwm24Signal  
 
    );

localparam  [8:0] L_DEAD_CYCLE  = DEAD_CYCLE ;//>= 8'd10 ?  DEAD_CYCLE : 8'd10;


//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

wire [25:0]       wPulseWidth                 ;

reg  [15:0]       rCycleCnt          = 16'd0   ;
reg  [15:0]       rPhaseCnt         = 16'd0   ;
reg  [15:0]       rRealPulseWidth1  = 16'd0   ;
reg  [15:0]       rRealPulseWidth2  = 16'd0   ;
reg  [25:0]       rPulseWidth       = 34'd0   ;
reg               rPwm1Signal       = 1'b0    ;
reg               rPwm2Signal       = 1'b0    ;
reg  [25:0]       rPhasePoint       = 26'd0   ;
reg               rPwm13Signal      = 1'b0    ;
reg               rPwm24Signal      = 1'b0    ;
reg               rSysRst           = 1'b1    ;


//******************************************************************************

// reg [15:0]PWM_CYCLE = 200000/55;

// //******************************************************************************
// // 线性调频 每个PWMs更改一次频率
// //******************************************************************************
// //
// reg flag1 = 0 ;
// reg [5:0] SSCCnt = 6'd0;
// wire wSSCen;
// localparam [15:0] Freqmax = 200000/55.1;//3633
// localparam [15:0] Freqmin = 200000/54.9;//3639

// // always @(posedge iSysClk ) begin
// //   if(rCycleCnt[15:0] == 16'd0)begin
// //     SSCCnt[5:0] <= SSCCnt[5:0] + 1'b1;
// //   end
// // end

// assign wSSCen = (rCycleCnt[15:0] == 16'd0) ;//& (SSCCnt[5:0] == 6'd0);

// always @ (posedge iSysClk) begin
// if(wSSCen)begin
//   if(PWM_CYCLE > Freqmin)
//     flag1 <= 1'b0;
//   else if(PWM_CYCLE < Freqmax)
//     flag1 <= 1'b1;
//   else 
//     flag1 <= flag1;
//   end
// end

// always @(posedge iSysClk ) begin
// if(wSSCen)begin
//   if(flag1 == 1'b0)
//     PWM_CYCLE <= PWM_CYCLE - 10'd1;
//   else if(flag1 == 1'b1)
//     PWM_CYCLE <= PWM_CYCLE + 10'd1;
//   else
//     PWM_CYCLE <= PWM_CYCLE;
// end
// end

//******************************************************************************

UnsigMult10x16 U1 (
 .CLK (iSysClk          ),       // input wire CLK
 .A   (iPwmDuty[9:0]    ),   // input wire [9 : 0] A
 .B   (PWM_CYCLE[15:0]  ),      // input wire [23 : 0] B
 .P   (wPulseWidth[25:0])      // output wire [33 : 0] P
);

//******************************************************************************
//锯齿计数器延迟90度
always @(posedge iSysClk or posedge iSysRst) begin
  if(iSysRst)begin
    rPhaseCnt[15:0] <= 16'd0;
    rSysRst <= 1'b1;
  end
  else if(COUNTER_PHASE == 9'd90)begin
    if(rPhaseCnt[15:0] == PWM_CYCLE[15:2])begin
      rPhaseCnt[15:0] <= rPhaseCnt[15:0] ;
      rSysRst <= 1'b0;
    end
    else begin
      rPhaseCnt[15:0] <= rPhaseCnt[15:0] + 1'b1;
      rSysRst <= rSysRst;
    end
  end
  else if(COUNTER_PHASE == 9'd180)begin
    if(rPhaseCnt[15:0] == PWM_CYCLE[15:1])begin
      rPhaseCnt[15:0] <= rPhaseCnt[15:0] ;
      rSysRst <= 1'b0;
    end
    else begin
      rPhaseCnt[15:0] <= rPhaseCnt[15:0] + 1'b1;
      rSysRst <= rSysRst;
    end
  end
  else begin
    rPhaseCnt[15:0] <= 16'd0;
    rSysRst <= 1'b0;    
  end
end

//******************************************************************************


//******************************************************************************
//锯齿波计数器产生周期计数
always @(posedge iSysClk ) begin
  if(rSysRst | (rCycleCnt[15:0] == PWM_CYCLE[15:0] - 16'd1))
    rCycleCnt[15:0] <= 24'd0;
  else
    rCycleCnt[15:0] <= rCycleCnt[15:0] + 1'b1;
end

//******************************************************************************
//产生脉冲宽度
always @(posedge iSysClk ) begin
  if((rCycleCnt[15:0] == PWM_CYCLE[15:0] - 16'd1)) begin   //在计数器为0的时刻进行装载参数
    if(wPulseWidth[25:11] < L_DEAD_CYCLE[8:0]+10'd300)         //限制占空比最小为死区的二倍
      rPulseWidth[25:11] <= {7'd0,L_DEAD_CYCLE[8:0]+10'd300}; //最小为160ns的开通时间
    // else if(wPulseWidth[25:11] > (PWM_CYCLE[15:1]-(L_DEAD_CYCLE[7:0]<<1)))
    //   rPulseWidth[25:11] <= PWM_CYCLE[15:1]-(L_DEAD_CYCLE[7:0]<<1);  //为了占空比50%
    else
      rPulseWidth[25:11] <= wPulseWidth[25:11];  
  end
end
//******************************************************************************

always @(posedge iSysClk ) begin
  rRealPulseWidth1[15:0] <= {1'b0, rPulseWidth[25:11]} ;//提前进行分频
  rRealPulseWidth2[15:0] <= { PWM_CYCLE[15:1] + rPulseWidth[25:11]};
end    



always @(posedge iSysClk ) begin
  if(rSysRst)
    rPwm1Signal <= 1'b0;
  else if((rCycleCnt[15:0] < rRealPulseWidth1[15:0]) && (rCycleCnt[15:0] > {7'd0,L_DEAD_CYCLE[8:0]}))
    rPwm1Signal <= 1'b1 ;
  else 
    rPwm1Signal <= 1'b0 ;
end

always @(posedge iSysClk ) begin
    if(rSysRst)
      rPwm2Signal <= 1'b0;
    else if(rCycleCnt[15:0] < rRealPulseWidth2[15:0] && rCycleCnt[15:0]> {1'b0,PWM_CYCLE[15:1]+L_DEAD_CYCLE[8:0]}) //计数器最大值的中点开始置位
      rPwm2Signal <= 1'b1 ;
    else 
      rPwm2Signal <= 1'b0 ;
end

always @(posedge iSysClk ) begin
  if(iPWMEn == 1'b1)begin
    rPwm13Signal <=  rPwm1Signal;
    rPwm24Signal <=  rPwm2Signal;
  end
  else begin
    rPwm13Signal <= 1'b0 ;
    rPwm24Signal <= 1'b0 ;
  end
end

assign oPwm13Signal = rPwm13Signal ;
assign oPwm24Signal = rPwm24Signal ;


endmodule





//  reg [15:0]PWM_CYCLE = 200000/55;

// //******************************************************************************
// // Debug 线性调频 2s更改一次频率
// //******************************************************************************
// //
// localparam COUNT_1US = 8'd200;
// localparam COUNT_1MS = 10'd1000;
// localparam COUNT_2S =  11'd300;
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

// reg flag1 = 0 ;

// localparam [15:0] Freqmax = 200000/61;
// localparam [15:0] Freqmin = 200000/53;

// always @ (posedge iSysClk) begin
// if(wCounts)begin
//   if(PWM_CYCLE > Freqmin)
//     flag1 <= 1'b0;
//   else if(PWM_CYCLE < Freqmax)
//     flag1 <= 1'b1;
//   else 
//     flag1 <= flag1;
//   end
// end

// always @(posedge iSysClk ) begin
// if(wCounts)begin
//   if(flag1 == 1'b0)
//     PWM_CYCLE <= PWM_CYCLE - 10'd10;
//   else if(flag1 == 1'b1)
//     PWM_CYCLE <= PWM_CYCLE + 10'd10;
//   else
//     PWM_CYCLE <= PWM_CYCLE;
// end
// end