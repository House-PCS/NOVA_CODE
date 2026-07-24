//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   UsignedDivision.v
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

`timescale 1ns/1ns

module UsignedDivision #(
    parameter WITH1 = 16,     // 被除数位宽
    parameter WITH2 = 8       // 除数位宽
) (
    input                   iSysClk                 ,   // 时钟
    input                   iSysRst                 ,   // 异步复位，低有效
    input                   iStart                  ,   // 开始信号，高有效
    input   [WITH1-1:0]     iDividend               ,   // 被除数
    input   [WITH2-1:0]     iDivisor                ,   // 除数
    output  [WITH1-1:0]     oQuotient               ,   // 商
    output  [WITH2-1:0]     oRemainder              ,   // 余数
    output                  oDone                   ,   // 完成信号，高有效
    output                  oBusy
);

//-------------------------------Code Start Here--------------------------------


localparam IDLE         = 1'b0;      // 空闲状态
localparam BUSY         = 1'b1;      // 计算状态
localparam CNT_WITCH    = $clog2(WITH1);
//******************************************************************************
//

    reg                         rState                  = 'b0;  // 当前状态
    reg                         rDone                   = 'b0;
    reg     [WITH1-1:0]         rQuotient               = 'b0;  // 商
    reg     [WITH2-1:0]         rRemainder              = 'b0;  // 余数

    reg     [WITH1+WITH2-1:0]   rTempDividendShifted    ='b0 ; // 扩展被除数（余数+被除数）
    reg     [WITH1-1:0]         rQuotientBuf            ='b0 ; // 商寄存器
    reg     [WITH2-1:0]         rDivisor                ='b0 ; // 锁存的除数
    reg     [WITH1-1:0]         rDividend               ='b0 ; // 锁存的被除数
    reg     [CNT_WITCH:0]       rCycleCnt               ='b0 ; // 周期计数器（足够位宽）

    
    wire    [WITH1+WITH2-1:0]   wTempDividendShifted    ;// 组合逻辑计算中间值
    wire    [WITH2-1:0]         wShiftedHigh            ;
    wire    [WITH2-1:0]         wNewHigh                ;
    wire    [WITH1-1:0]         wQuotientNext           ;
//******************************************************************************

//******************************************************************************
//

assign  wTempDividendShifted    [WITH1+WITH2-1:0]   = rTempDividendShifted[WITH1+WITH2-1:0] << 1    ; // 左移后的被除数
assign  wShiftedHigh            [WITH2-1:0]         = wTempDividendShifted[WITH1+WITH2-1:WITH1]     ; // 高位部分
assign  wNewHigh                [WITH2-1:0]         = (wShiftedHigh[WITH2-1:0] >= rDivisor[WITH2-1:0]) ? (wShiftedHigh[WITH2-1:0] - rDivisor[WITH2-1:0]) : wShiftedHigh[WITH2-1:0]; // 新余数
assign  wQuotientNext           [WITH1-1:0]         = (wShiftedHigh[WITH2-1:0] >= rDivisor[WITH2-1:0]) ? ((rQuotientBuf[WITH1-1:0] << 1) | 1'b1) : (rQuotientBuf[WITH1-1:0] << 1'b1); // 新商

//******************************************************************************



//******************************************************************************
//
always @(posedge iSysClk ) begin
    if (iSysRst) begin
        rState                                  <= IDLE ;
        rDone                                   <= 0    ;
        rTempDividendShifted[WITH1+WITH2-1:0]   <= 0    ;
        rQuotientBuf[WITH1-1:0]                 <= 0    ;
        rDivisor[WITH2-1:0]                     <= 0    ;
        rDividend[WITH1-1:0]                    <= 0    ;
        rCycleCnt[CNT_WITCH:0]                  <= 0    ;
        rQuotient[WITH1-1:0]                    <= 0    ;
        rRemainder[WITH2-1:0]                   <= 0    ;
    end else begin
        case (rState)
            IDLE: begin
                rDone <= 0;
                if (iStart) begin // 锁存输入并初始化
                    rDividend[WITH1-1:0]                    <= iDividend[WITH1-1:0]                     ;
                    rDivisor [WITH2-1:0]                    <= iDivisor [WITH2-1:0]                     ;
                    rTempDividendShifted[WITH1+WITH2-1:0]   <= {{WITH2{1'b0}}, iDividend[WITH1-1:0]}    ; // 初始余数为0
                    rQuotientBuf[WITH1-1:0]                 <= 'd0                                      ; // 商初始为0
                    rCycleCnt[CNT_WITCH:0]                  <= 'd0                                      ; // 重置计数器
                    rState                                  <= BUSY                                     ;               
                end
            end
            BUSY: begin // 更新余数和商
                rTempDividendShifted[WITH1+WITH2-1:0]   <= {wNewHigh[WITH2-1:0], wTempDividendShifted[WITH1-1:0]};
                rQuotientBuf[WITH1-1:0]                 <= wQuotientNext[WITH1-1:0] ;
                rCycleCnt[CNT_WITCH:0]                  <= rCycleCnt[CNT_WITCH:0] + 1;       // 计数器递增
                if (rCycleCnt[CNT_WITCH:0] == WITH1-1'b1) begin          
                    rQuotient [WITH1-1:0]  <= wQuotientNext[WITH1-1:0];   // 输出最终商
                    rRemainder[WITH2-1:0]  <= wNewHigh[WITH2-1:0];       // 输出最终余数
                    rState                 <= IDLE;               
                    rDone                  <= 1'b1;  
                end
            end
        endcase
    end
end
//******************************************************************************

//******************************************************************************
//
assign  oQuotient  [WITH1-1:0]  = rQuotient [WITH1-1:0]         ;
assign  oRemainder [WITH2-1:0]  = rRemainder[WITH2-1:0]         ;
assign  oDone                   = rDone                         ;
assign  oBusy                   = (rState == BUSY)              ;
//******************************************************************************

endmodule