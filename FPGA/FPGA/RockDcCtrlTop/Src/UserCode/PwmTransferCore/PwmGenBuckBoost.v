//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   PwmGenBuckBoost.v
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
//Modifier          :   修改模块名称
//Modify Date       :   
/*Modify Record     :   

*/  
//------------------------------------------------------------------------------

`timescale 1ns/1ns
module PwmGenBuckBoost
#(
    parameter                       PWM_CYCLE           = 16'd1000          ,   //PWM计数周期，时钟5ns，PWM周期为5ns*1000 = 5us
    parameter                       DEAD_CYCLE          = 8'd60             ,   //死区时间（不能为0），时钟5ns，死区时间为5ns*50 = 250ns，
    parameter                       COUNTER_PHASE       = 9'd180            
)
(
    //Global Clock
    input                           iSysClk                                 ,   //默认100MHz频率
    input                           iSysRst                                 ,
    input                           iPwmEn                                  ,
    input       [9:0]               iPwmDuty                                ,   //PWM占空比，范围0-1023
    //Pwm
    output                          oPwmPSignal                             ,
    output                          oPwmNSignal
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    wire        [25:0]              wPwmHigh                                ;
    reg         [25:0]              rPwmHigh            = 26'd0             ;
    reg         [15:0]              rPwmCnt             = 16'd0             ;
    reg         [15:0]              rPhaseCnt           = 16'd0             ;
    reg         [15:0]              rPwmHighBuf         = 16'd0             ;
    reg                             rPwmSignal          = 1'd0              ;
    wire                            wPwmPSignal                             ;
    wire                            wPwmNSignal                             ;    
    reg         [DEAD_CYCLE:0]      rPwmPSignalBuf      = 'd0               ;
    reg         [DEAD_CYCLE:0]      rPwmNSignalBuf      = 'd0               ;    
    reg                             rPwmPSignal         = 1'd0              ;
    reg                             rPwmNSignal         = 1'd0              ;
    reg                             rSysRst             = 1'b1              ;

//******************************************************************************

//******************************************************************************
//

//assign wPwmHigh[25:0] = iPwmDuty[9:0] * PWM_CYCLE;
UnsigMult10x16 U1 (
 .CLK (iSysClk          ),       // input wire CLK
 .A   (iPwmDuty[9:0]    ),   // input wire [9 : 0] A
 .B   (PWM_CYCLE[15:0]  ),      // input wire [23 : 0] B
 .P   (wPwmHigh[25:0]   )      // output wire [33 : 0] P
);

//******************************************************************************

//******************************************************************************
//锯齿计数器延迟90度180度
always @(posedge iSysClk or posedge iSysRst ) begin
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
always @ (posedge iSysClk) begin
    if(rSysRst | (rPwmCnt[15:0]  == PWM_CYCLE[15:0] - 16'd1))
        rPwmCnt[15:0] <= 16'd0;
    else
        rPwmCnt[15:0] <= rPwmCnt[15:0] + 16'd1;
end

//******************************************************************************

//******************************************************************************
//产生脉冲宽度

always @ (posedge iSysClk) begin
    if((rPwmCnt[15:0] == PWM_CYCLE - 16'd1) ) begin
        if(wPwmHigh[25:10] < {7'd0, DEAD_CYCLE[7:0], 1'd0})
            rPwmHigh[25:10] <= {7'd0, DEAD_CYCLE[7:0], 1'd1};
        else if(wPwmHigh[25:10] > (PWM_CYCLE[15:0] - {7'd0, DEAD_CYCLE[7:0], 1'd0}))
            rPwmHigh[25:10] <= (PWM_CYCLE[15:0] - {7'd0, DEAD_CYCLE[7:0], 1'd0});
        else
            rPwmHigh[25:10] <= wPwmHigh[25:10];
    end
end

always @ (posedge iSysClk) begin
    rPwmHighBuf[15:0] <= {rPwmHigh[25:10]};
end

//******************************************************************************



//******************************************************************************
//

always @ (posedge iSysClk ) begin
    if(rSysRst)
        rPwmSignal <= 1'b0;
    else if((rPwmCnt[15:0] + 16'd1) == PWM_CYCLE[15:0])
        rPwmSignal <= 1'b1;
    else if((rPwmCnt[15:0] + 16'd1) == rPwmHighBuf[15:0])
        rPwmSignal <= 1'b0;
    else
        rPwmSignal <= rPwmSignal;
end

assign wPwmPSignal = rPwmSignal     ;
assign wPwmNSignal = (!rPwmSignal)  ;

//******************************************************************************

//******************************************************************************
//死区时间控制

always @ (posedge iSysClk) begin
    rPwmPSignalBuf[DEAD_CYCLE:0] <= {rPwmPSignalBuf[(DEAD_CYCLE - 1):0], wPwmPSignal};
end

always @ (posedge iSysClk) begin
    rPwmNSignalBuf[DEAD_CYCLE:0] <= {rPwmNSignalBuf[(DEAD_CYCLE - 1):0], wPwmNSignal};
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(rSysRst)
        rPwmPSignal <= 1'b0;
    else if(iPwmEn)
        rPwmPSignal <= rPwmPSignalBuf[DEAD_CYCLE] & wPwmPSignal;
    else
        rPwmPSignal <= 1'b0;
end

always @ (posedge iSysClk) begin
    if(rSysRst)
        rPwmNSignal <= 1'b0;
    else if(iPwmEn)
        rPwmNSignal <= rPwmNSignalBuf[DEAD_CYCLE] & wPwmNSignal;
    else
        rPwmNSignal <= 1'b0;
end

//******************************************************************************

//******************************************************************************
//

assign oPwmPSignal = rPwmNSignal;
assign oPwmNSignal = rPwmPSignal;

//******************************************************************************

endmodule
