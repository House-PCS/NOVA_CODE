//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   SignedDivision.v
//Original Author   :   xucy   Nova003107 
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

`timescale 1ns/1ps

module SignedDivision  #(
    parameter                   WITH1 = 16        ,     // 被除数位宽
    parameter                   WITH2 = 8               // 除数位宽
) (
    input                       iSysClk         ,
    input                       iSysRst         ,
    input                       iStart          ,
    input   signed [WITH1-1:0]  iDividend       ,
    input   signed [WITH2-1:0]  iDivisor        ,
    output  signed [WITH1-1:0]  oQuotient       ,
    output  signed [WITH2-1:0]  oRemainder      ,
    output                      oDone           ,
    output                      oBusy           
);

//-------------------------------Code Start Here--------------------------------

localparam CNT_WITCH    = $clog2(WITH1);

localparam IDLE         = 2'b00; // 状态定义
localparam BUSY         = 2'b01;

//******************************************************************************
//
reg         [1:0]                       rState                  ;
reg                                     rDone                   ;
reg signed  [WITH1-1:0]                 rQuotient               ;
reg signed  [WITH2-1:0]                 rRemainder              ;
reg                                     rDividendSign           ;
reg                                     rDivisorSign            ;
//reg         [WITH1-1:0]                 rDividend               ;
reg         [WITH2-1:0]                 rDivisor                ;
reg         [WITH1+WITH2-1:0]           rTempDividendShifted    ;
reg         [WITH1-1:0]                 rQuotientBuf            ;
reg         [CNT_WITCH:0]               rCycleCnt               ;
wire        [WITH1+WITH2-1:0]           wTempDividendShifted    ;
wire        [WITH2-1:0]                 wShiftedHigh            ;         
wire        [WITH2-1:0]                 wNewHigh                ;         
wire        [WITH1-1:0]                 wQuotientNext           ;         


//******************************************************************************
// assign   wTempDividendShifted[WITH1+WITH2-1:0] = rTempDividendShifted << 1;
// assign   wShiftedHigh        [WITH2-1:0]           = wTempDividendShifted[WITH1+WITH2-1:WITH1];
// assign   wNewHigh            [WITH2-1:0]           = (wShiftedHigh >= rDivisor) ? (wShiftedHigh - rDivisor) : wShiftedHigh;
// assign   wQuotientNext       [WITH1-1:0]           = (wShiftedHigh >= rDivisor) ? ((rQuotientBuf << 1) | 1'b1) : (rQuotientBuf << 1);
//******************************************************************************

assign  wTempDividendShifted    [WITH1+WITH2-1:0]   = rTempDividendShifted[WITH1+WITH2-1:0] << 1    ; // 左移后的被除数
assign  wShiftedHigh            [WITH2-1:0]         = wTempDividendShifted[WITH1+WITH2-1:WITH1]     ; // 高位部分
assign  wNewHigh                [WITH2-1:0]         = (wShiftedHigh[WITH2-1:0] >= rDivisor[WITH2-1:0]) ? (wShiftedHigh[WITH2-1:0] - rDivisor[WITH2-1:0]) : wShiftedHigh[WITH2-1:0]; // 新余数
assign  wQuotientNext           [WITH1-1:0]         = (wShiftedHigh[WITH2-1:0] >= rDivisor[WITH2-1:0]) ? ((rQuotientBuf[WITH1-1:0] << 1) | 1'b1) : (rQuotientBuf[WITH1-1:0] << 1'b1); // 新商


//******************************************************************************
//
always @(posedge iSysClk) begin
    if (iSysRst) begin
        rState                                  <= IDLE ;
        rDone                                   <= 0    ;
        rTempDividendShifted[WITH1+WITH2-1:0]   <= 0    ;
        rQuotientBuf[WITH1-1:0]                 <= 0    ;
        rDivisor[WITH2-1:0]                     <= 0    ;
       // rDividend[WITH1-1:0]                    <= 0    ;
        rCycleCnt[CNT_WITCH:0]                  <= 0    ;
        rQuotient[WITH1-1:0]                    <= 0    ;
        rRemainder[WITH2-1:0]                   <= 0    ;
    end else begin
        case (rState)
            IDLE: begin
                rDone <= 0;
                if (iStart) begin
                    rDividendSign                           <= iDividend[WITH1-1];
                    rDivisorSign                            <= iDivisor[WITH2-1];
                   // rDividend[WITH1-1:0]                    <= ($signed(iDividend[WITH1-1:0]) < 0) ? -iDividend[WITH1-1:0] : iDividend[WITH1-1:0];
                    rDivisor[WITH2-1:0]                     <= ($signed(iDivisor[WITH2-1:0]) < 0) ? -iDivisor[WITH2-1:0] : iDivisor[WITH2-1:0];
                    rTempDividendShifted[WITH1+WITH2-1:0]   <= {{WITH2{1'b0}}, ($signed(iDividend[WITH1-1:0]) < 0) ? -iDividend[WITH1-1:0] : iDividend[WITH1-1:0]};
                    rQuotientBuf[WITH1-1:0]                 <= 0;
                    rCycleCnt[CNT_WITCH:0]                  <= 0;
                    rState                                  <= BUSY;
                end
            end
            
            BUSY: begin
                rTempDividendShifted[WITH1+WITH2-1:0]   <= {wNewHigh[WITH2-1:0], wTempDividendShifted[WITH1-1:0]};
                rQuotientBuf[WITH1-1:0]                 <= wQuotientNext[WITH1-1:0] ;
                rCycleCnt[CNT_WITCH:0]                  <= rCycleCnt[CNT_WITCH:0] + 1; 
                if (rCycleCnt[CNT_WITCH:0] == WITH1 - 1) begin
                                   
                    if (rDividendSign ^ rDivisorSign) begin // 调整商符号
                        rQuotient[WITH1-1:0] <= -wQuotientNext[WITH1-1:0];
                    end else begin
                        rQuotient[WITH1-1:0] <= wQuotientNext[WITH1-1:0];
                    end
                   
                    if (rDividendSign) begin // 调整余数符号
                        rRemainder[WITH2-1:0] <= -wNewHigh[WITH2-1:0];
                    end else begin
                        rRemainder[WITH2-1:0] <= wNewHigh[WITH2-1:0];//;
                    end
                    rDone <= 1;
                    rState <= IDLE;
                end
            end
        endcase
    end
end
//******************************************************************************
//

//******************************************************************************
//
assign  oQuotient  [WITH1-1:0]  = rQuotient [WITH1-1:0]         ;
assign  oRemainder [WITH2-1:0]  = rRemainder[WITH2-1:0]         ;
assign  oDone                   = rDone                         ;
assign  oBusy                   = (rState == BUSY) | rDone      ;
//******************************************************************************

endmodule


