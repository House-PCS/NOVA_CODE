//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   SysLed.v
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
module SysLed
#(
    parameter       CLK_FREQ    = 100
)
(
    //Global Clock
    input           iSysClk         ,
    input           iSysRst         ,
    input           iCnt1Ms         ,
    input   [1:0]   iSrcStable      ,
    //User Interface
    output          oSysLed         
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

// 内部参数定义
localparam  COUNT_100MS         =   10'd99      ;
localparam  COUNT_1S            =   10'd1000    ;  

// 状态定义
localparam  STATE_PATTERN0      = 2'b00;    //正常运行  正常闪烁
localparam  STATE_PATTERN1      = 2'b01;    //没有网络  两次闪烁
localparam  STATE_PATTERN2      = 2'b10;    //DCDC异常  三次闪烁
localparam  STATE_PATTERN3      = 2'b11;    //BMS异常   快速闪烁

reg [9:0]   rCount          =   10'd0   ;
reg [1:0]   rPatternStep    =   2'd0    ;
reg         rLed            =   1'b0    ;

//******************************************************************************
//
//    reg     [1:0]   rSysRstBuf      = 2'd0  ;
    reg     [1:0]   rSrcStableBuf   = 2'd0  ;

//******************************************************************************

//******************************************************************************
//

// always @ (posedge iSysClk) begin
//     rSysRstBuf[1:0] <= {rSysRstBuf[0], iSysRst};
// end

always @ (posedge iSysClk) begin
    rSrcStableBuf[1:0] <= iSrcStable[1:0];
end

//******************************************************************************

//******************************************************************************

`ifndef BOOTMODE
always @(posedge iSysClk ) begin
     if(iCnt1Ms) begin
        rCount <= rCount + 1;
        case (rSrcStableBuf[1:0])

            STATE_PATTERN0: begin
                if (rCount < 5*COUNT_100MS)         rLed <= 1'b1;
                else if (rCount < 10*COUNT_100MS)   rLed <= 1'b0;
                else begin
                    rCount <= 0;
                end
            end
            
            STATE_PATTERN1: begin // 短-短-长模式
                case (rPatternStep)
                    0: begin 
                        if (rCount < COUNT_100MS)      rLed <= 1'b1;
                        else if (rCount < 2*COUNT_100MS) rLed <= 1'b0;
                        else begin
                            rCount <= 0;
                            rPatternStep <= 1;
                        end
                    end
                    1: begin 
                        if (rCount < 1*COUNT_100MS)     rLed <= 1'b1;
                        else if (rCount < 2*COUNT_100MS) rLed <= 1'b0;
                        else begin
                            rCount <= 0;
                            rPatternStep <= 2;
                        end
                    end
                    2: begin // 长闪
                        if (rCount < 6*COUNT_100MS) rLed <= 1'b0;
                        else begin
                            rCount <= 0;
                            rPatternStep <= 0;
                        end
                    end
                endcase
            end
            
            STATE_PATTERN2: begin // 长-短模式
                case (rPatternStep)
                    0: begin // 长闪
                        if (rCount < 1*COUNT_100MS)      rLed <= 1'b1;
                        else if (rCount < 2*COUNT_100MS) rLed <= 1'b0;
                        else begin
                            rCount <= 0 ;
                            rPatternStep <= 1;
                        end
                    end
                    1: begin // 短闪
                        if (rCount < 1*COUNT_100MS)      rLed <= 1'b1;
                        else if (rCount < 2*COUNT_100MS) rLed <= 1'b0;
                        else begin
                            rCount <= 0;
                            rPatternStep <= 2;
                        end
                    end
                    2: begin
                        if (rCount < 1*COUNT_100MS)      rLed <= 1'b1;
                        else if (rCount < 7*COUNT_100MS) rLed <= 1'b0;
                        else begin
                            rCount <= 0;
                            rPatternStep <= 0;
                        end
                    end
                endcase
            end
            STATE_PATTERN3: begin // 双短闪+长暂停  --__--__----
                if (rCount < COUNT_100MS)         rLed <= 1'b1;
                else if (rCount < 2*COUNT_100MS)   rLed <= 1'b0;
                else begin
                    rCount <= 0;
                end
            end
            default: begin
                rLed <= 1'b0;
                rCount <= 0;
                rPatternStep <= 0;
            end
        endcase
    end else begin
        rCount <= rCount ;
     end
end
`else
always @(posedge iSysClk ) begin
  if(iCnt1Ms) begin
    if(rCount == COUNT_1S)
      rCount <= 'd0;
    else
      rCount <= rCount + 1;
  end
  else
    rCount <= rCount;
end
  
always@(posedge iSysClk)begin
  if(iCnt1Ms && (rCount == COUNT_1S))
    rLed <= ~rLed;
  else
    rLed <= rLed;    
end
`endif

assign oSysLed = ~rLed;

endmodule

//-------------------------------Code Start Here--------------------------------
/*
//******************************************************************************
//
    localparam  COUNT_1US           = CLK_FREQ  ;


    `ifdef SIMULATION_EN
        localparam  COUNT_1MS           = 10'd10    ;
        localparam  FAST_TOGGLE         = 16'd20    ;//快速反转半周期
        localparam  SLOW_TOGGLE         = 16'd80    ;//慢速反转半周期
    `else
        localparam  COUNT_1MS           = 10'd1000  ;
        localparam  FAST_TOGGLE         = 16'd125   ;//快速反转半周期
        localparam  SLOW_TOGGLE         = 16'd1000  ;//慢速反转半周期
    `endif



//******************************************************************************

//******************************************************************************
//
    reg     [1:0]   rSysRstBuf      = 2'd0  ;
    reg     [1:0]   rSrcStableBuf   = 2'd0  ;
    reg     [7:0]   rCountUs        = 8'd0  ;
    wire            wCountUs                ;
    reg     [9:0]   rCountMs        = 10'd0 ;
    wire            wCountMs                ;

    reg     [15:0]  rFastToggleCnt  = 16'd0 ;
    reg     [15:0]  rSlowToggleCnt  = 16'd0 ;
    reg             rFastToggle     = 1'b0  ;
    reg             rSlowToggle     = 1'b0  ;
    reg             rSysLed         = 1'b0  ;

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    rSysRstBuf[1:0] <= {rSysRstBuf[0], iSysRst};
end

always @ (posedge iSysClk) begin
    rSrcStableBuf[1:0] <= {rSrcStableBuf[0], iSrcStable};
end

//******************************************************************************

//******************************************************************************
//
always @ (posedge iSysClk) begin
    if(rCountUs[7:0] == (COUNT_1US - 8'd1))
        rCountUs[7:0] <= 8'd0;
    else
        rCountUs[7:0] <= rCountUs[7:0] + 8'd1;
end
    
assign wCountUs = (rCountUs[7:0] == (COUNT_1US - 8'd1));


//******************************************************************************

//******************************************************************************
//
always @ (posedge iSysClk) begin
    if(wCountUs) begin
        if(rCountMs[9:0] == (COUNT_1MS - 10'd1))
            rCountMs[9:0] <= 10'd0;
        else
            rCountMs[9:0] <= rCountMs[9:0] + 10'd1;
    end
    else
        rCountMs[9:0] <= rCountMs[9:0];
end

assign wCountMs = (wCountUs && (rCountMs[9:0] == (COUNT_1MS - 10'd1)));


//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(wCountMs) begin
        if((rFastToggleCnt[15:0] + 16'd1) == FAST_TOGGLE)
            rFastToggleCnt[15:0] <= 16'd0;
        else
            rFastToggleCnt[15:0] <= rFastToggleCnt[15:0] + 16'd1;
    end
    else
        rFastToggleCnt[15:0] <= rFastToggleCnt[15:0];
end


always @ (posedge iSysClk) begin
    if(wCountMs) begin
        if((rSlowToggleCnt[15:0] + 16'd1) == SLOW_TOGGLE)
            rSlowToggleCnt[15:0] <= 16'd0;
        else
            rSlowToggleCnt[15:0] <= rSlowToggleCnt[15:0] + 16'd1;
    end
    else
        rSlowToggleCnt[15:0] <= rSlowToggleCnt[15:0];
end


//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(wCountMs && ((rFastToggleCnt[15:0] + 16'd1) == FAST_TOGGLE))
        rFastToggle <= ~rFastToggle;
    else
        rFastToggle <= rFastToggle;
end

always @ (posedge iSysClk) begin
    if(wCountMs && ((rSlowToggleCnt[15:0] + 16'd1) == SLOW_TOGGLE))
        rSlowToggle <= ~rSlowToggle;
    else
        rSlowToggle <= rSlowToggle;
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(rSysRstBuf[1])
        rSysLed <= 1'b0;
    else if(rSrcStableBuf[1])
        rSysLed <= rFastToggle;
    else
        rSysLed <= rSlowToggle;
end

//******************************************************************************

//******************************************************************************
//

assign oSysLed = rSysLed;

//******************************************************************************

endmodule*/


