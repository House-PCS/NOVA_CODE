module Sqrt3Test #(
    parameter WIDTH = 32,
    parameter ITER_NUM = 10
)(
    input                  clk,
    input                  rst_n,
    input                  start,
    input      signed [WIDTH-1:0] din,
    output reg signed [WIDTH-1:0] dout,
    output reg             done
);

    // 状态定义
    localparam IDLE = 2'd0, CALC_DIV = 2'd1, CALC_DIV3 = 2'd2, UPDATE = 2'd3;
    reg [1:0] state;

    reg signed [WIDTH-1:0] x;           // 当前迭代值
    reg [3:0]       iter_cnt;           // 迭代计数
    reg             busy;

    // din/(x*x)除法相关信号
    reg                  div_start;
    wire                 div_done;
    wire signed [WIDTH-1:0]     div_quotient;
    wire signed [WIDTH-1:0]     div_remainder;
    reg  signed [WIDTH-1:0]     div_dividend;
    reg  signed [WIDTH-1:0]     div_divisor;

    // (2*x+din/(x*x))/3除法相关信号
    reg                  div3_start;
    wire                 div3_done;
    wire signed [WIDTH-1:0]     div3_quotient;
    wire signed [WIDTH-1:0]     div3_remainder;
    reg  signed [WIDTH-1:0]     div3_dividend;
    reg  signed [WIDTH-1:0]     div3_divisor;

    // 乘法结果
    wire signed [2*WIDTH-1:0]   x2 = x * x;

    // 实例化SignedDivision模块（din/(x*x)）
    SignedDivision #(
        .WITH1(WIDTH),
        .WITH2(WIDTH)
    ) u_div (
        .iSysClk(clk),
        .iSysRst(~rst_n),
        .iStart(div_start),
        .iDividend(div_dividend),
        .iDivisor(div_divisor),
        .oQuotient(div_quotient),
        .oRemainder(div_remainder),
        .oDone(div_done),
        .oBusy()
    );

    // 实例化SignedDivision模块（(2*x+din/(x*x))/3）
    SignedDivision #(
        .WITH1(WIDTH),
        .WITH2(WIDTH)
    ) u_div3 (
        .iSysClk(clk),
        .iSysRst(~rst_n),
        .iStart(div3_start),
        .iDividend(div3_dividend),
        .iDivisor(div3_divisor),
        .oQuotient(div3_quotient),
        .oRemainder(div3_remainder),
        .oDone(div3_done),
        .oBusy()
    );

    // 保存din/(x*x)结果
    reg signed [WIDTH-1:0] din_div_x2;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state    <= IDLE;
            x        <= 0;
            dout     <= 0;
            iter_cnt <= 0;
            done     <= 0;
            busy     <= 0;
            div_start <= 0;
            div_dividend <= 0;
            div_divisor  <= 0;
            div3_start <= 0;
            div3_dividend <= 0;
            div3_divisor  <= 0;
            din_div_x2 <= 0;
        end else begin
            case (state)
                IDLE: begin
                    done <= 0;
                    if (start && !busy) begin
                        x        <= din;
                        iter_cnt <= 0;
                        busy     <= 1;
                        div_start <= 1;
                        div_dividend <= din;
                        div_divisor  <= din; // 先给个初值，实际在CALC_DIV赋值
                        state    <= CALC_DIV;
                    end
                end
                CALC_DIV: begin
                    div_start <= 0;
                    div_dividend <= din;
                    div_divisor  <= x2[WIDTH-1:0];
                    if (div_done) begin
                        din_div_x2 <= div_quotient;
                        div3_dividend <= (2 * x + div_quotient);
                        div3_divisor  <= 3;
                        div3_start    <= 1;
                        state         <= CALC_DIV3;
                    end
                end
                CALC_DIV3: begin
                    div3_start <= 0;
                    if (div3_done) begin
                        x <= div3_quotient;
                        iter_cnt <= iter_cnt + 1;
                        if (iter_cnt + 1 < ITER_NUM) begin
                            div_start <= 1;
                            div_dividend <= din;
                            div_divisor  <= (div3_quotient * div3_quotient);
                            state <= CALC_DIV;
                        end else begin
                            dout <= div3_quotient;
                            done <= 1;
                            busy <= 0;
                            state <= IDLE;
                        end
                    end
                end
                default: state <= IDLE;
            endcase
        end
    end

endmodule