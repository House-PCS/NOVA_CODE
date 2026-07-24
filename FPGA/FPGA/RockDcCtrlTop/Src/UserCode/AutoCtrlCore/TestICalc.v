module TestCalc #(
    parameter signed [31:0] WC        = 32'sd6280,
    parameter signed [31:0] CALC_GAIN = 32'sd1,
    parameter signed [31:0] FREQ      = 32'sd40000,
    parameter signed [31:0] CLK_FREQ  = 32'sd20000000
) (
    input                iSysClk,
    input  signed [31:0] iInput,
    output signed [31:0] oOutput
);

    reg signed [31:0] rInput1=0;
    reg signed [31:0] rInput0=0;
    reg signed [31:0] rOutput1=0;

    localparam signed [31:0] uk0 = 0;
    localparam signed [31:0] uk1 = WC;
    localparam signed [31:0] yk0 = FREQ;
    localparam signed [31:0] yk1 = WC - FREQ;
    localparam [15:0] CNT = CLK_FREQ / FREQ;

    reg        [15:0] rCnt;
    reg signed [31:0] rOutput;

    always @(posedge iSysClk) begin
        if (rCnt == CNT - 1) begin
            rCnt <= 16'd0;
        end else begin
            rCnt <= rCnt + 1;
        end
    end

    always @(posedge iSysClk) begin
        if (rCnt == 16'd0) begin
            rOutput <= (rInput0 * uk0 * CALC_GAIN + rInput1 * uk1 - rOutput1 * yk1) / FREQ;
        end else begin
            rOutput  <= rOutput;
            rOutput1 <= rOutput;
            rInput0 <= iInput;
            rInput1  <= rInput0 * CALC_GAIN;
        end
    end

    assign oOutput = rOutput / CALC_GAIN;

endmodule
