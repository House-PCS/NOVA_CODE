//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   PidControl.v
//Original Author   :   djh
//Creation Date     :   2025.4
/*Description       :   创建低通滤波器模块。

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

module LPFCore #(
    parameter   signed [31:0]   WC        = 32'sd6280       ,
    parameter   signed [31:0]   CALC_GAIN = 32'sd1          ,
    parameter   signed [31:0]   FREQ      = 32'sd40000      ,
    parameter   signed [31:0]   CLK_FREQ  = 32'sd20000000
) (
    input                       iSysClk                     ,
    input       signed [31:0]   iInput                      ,
    output      signed [31:0]   oOutput                     
);

    localparam signed [47:0] uk0 = 0                ;
    localparam signed [47:0] uk1 = WC               ;
    localparam signed [47:0] yk0 = FREQ             ;
    localparam signed [47:0] yk1 = WC - FREQ        ;
    localparam        [15:0] CNT = CLK_FREQ / FREQ  ;

    reg signed [47:0] rInput1   =   0   ;
    reg signed [47:0] rInput0   =   0   ;
    reg signed [47:0] rOutput1  =   0   ;
    reg        [15:0] rCnt              ;
    reg signed [47:0] rOutput           ;

    always @(posedge iSysClk) begin
        if (rCnt == CNT - 1) begin
            rCnt <= 16'd0   ;
        end else begin
            rCnt <= rCnt + 1;
        end
    end
    
    wire [47:0] wOutWithCalc        ;
    wire [47:0] wOutputWithGain     ;
    wire        wOutputWithGainDone ;
    wire        wOutputDivDone      ;

    reg  [2:0]rOutputDivDone = 3'd0 ;

//***********************************************************************************
reg signed [47:0]sig1               ;
reg signed [47:0]sig2               ;
reg signed [47:0]sig3               ;
reg signed [47:0]rOutWithCalc       ;

//assign wOutWithCalc = (rInput0 * uk0 * CALC_GAIN + rInput1 * uk1 - rOutput1 * yk1);

    always @(posedge iSysClk ) begin
        rOutputDivDone[2:0] <= {rOutputDivDone[1:0],wOutputDivDone};
    end

    always @(posedge iSysClk ) begin
        if(rOutputDivDone[0])
            sig1 <= rInput0 * uk0 * CALC_GAIN;
        else
            sig1 <= sig1 ;
    end
    always @(posedge iSysClk ) begin
        if(rOutputDivDone[0])
            sig3 <= rOutput1 * yk1;
        else
            sig3 <= sig3 ;
    end
    always @(posedge iSysClk ) begin
        if(rOutputDivDone[0])
            sig2 <= rInput1 * uk1;
        else
            sig2 <= sig2 ;
    end
 
    always @(posedge iSysClk ) begin
        if(rOutputDivDone[2])
            rOutWithCalc <= sig1 + sig2 - sig3;
        else 
            rOutWithCalc <= rOutWithCalc ;
    end

assign wOutWithCalc = rOutWithCalc;

//************************************************************************************
    

    wire [47:0] wOutput;

    SignedDivision #(
        .WITH1          (48                     ),  // 被除数位宽
        .WITH2          (32                     )   // 除数位宽
    ) U_OutputWithGainDiv (
        .iSysClk        (iSysClk                ),
        .iSysRst        (0                      ),
        .iStart         (rCnt == 16'd0          ),
        .iDividend      (wOutWithCalc           ),
        .iDivisor       (FREQ                   ),
        .oQuotient      (wOutputWithGain        ),
        .oRemainder     (                       ),
        .oDone          (wOutputWithGainDone    ),
        .oBusy          (                       )
    );

    SignedDivision #(
        .WITH1          (48                     ),  // 被除数位宽
        .WITH2          (32                     )   // 除数位宽
    ) U_OutputDiv (
        .iSysClk        (iSysClk                ),
        .iSysRst        (0                      ),
        .iStart         (wOutputWithGainDone    ),
        .iDividend      (wOutputWithGain        ),
        .iDivisor       (CALC_GAIN              ),
        .oQuotient      (oOutput                ),
        .oRemainder     (                       ),
        .oDone          (wOutputDivDone         ),
        .oBusy          (                       )
    );

    always @(posedge iSysClk) begin
        if (wOutputWithGainDone) begin
            rOutput <= wOutputWithGain;
        end else begin
            rOutput     <= rOutput;
            rOutput1    <= rOutput;
            rInput0     <= iInput;
            rInput1     <= rInput0 * CALC_GAIN;
        end
    end

    // assign oOutput = rOutput / CALC_GAIN;

endmodule
