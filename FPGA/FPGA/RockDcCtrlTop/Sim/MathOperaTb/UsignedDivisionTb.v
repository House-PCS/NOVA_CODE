`timescale 1ns / 1ns
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/03/16 22:48:38
// Design Name: 
// Module Name: UsignedDivisionTb
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


module UsignedDivisionTb();
localparam  WITH1 = 24;
localparam  WITH2 = 16;
    reg clk;
    reg rst;
    reg start;
    reg signed [WITH1-1:0] dividend;
    reg signed [WITH2-1:0] divisor;
    wire signed [WITH1-1:0] quotient;
    wire signed [WITH2-1:0] remainder;
    wire done;
    wire busy;

SignedDivision #(
    .WITH1      (    WITH1         ),     // 被除数位宽
    .WITH2      (    WITH2          )      // 除数位宽
) u1(
    .iSysClk   (clk             )       ,   // 时钟
    .iSysRst   (rst             )       ,   // 异步复位，低有效
    .iStart    (start           )       ,   // 开始信号，高有效
    .iDividend (dividend        )       ,   // 被除数
    .iDivisor  (divisor         )       ,   // 除数
    .oQuotient (quotient        )       ,   // 商
    .oRemainder(remainder       )       ,   // 余数
    .oDone     (done            )       ,   // 完成信号，高有效
    .oBusy     (busy            )
);

    // 时钟生成
    always #5 clk = ~clk;

    initial begin
        // 初始化
        clk = 0;
        rst = 1;
        start = 0;
        #20;
        rst = 0;

        // 测试1: 1000000 / 256 = 3906 (余64)
        dividend = -1000000;
        divisor = -256;
        start = 1;
        #10 start = 0;
        wait(done);
        $display("Test1: %d / %d = %d (Remainder %d)", dividend, divisor, quotient, remainder);
#100
        // 测试2: 123456 / 789 = 156 (余372)
        dividend = -123456;
        divisor = 789;
        start = 1;
        #10 start = 0;
        wait(done);
        $display("Test2: %d / %d = %d (Remainder %d)", dividend, divisor, quotient, remainder);
#100
        // 测试3: 32767 / 1024 = 31 (余1023)
        dividend = 32767;
        divisor = 1024;
        start = 1;
        #10 start = 0;
        wait(done);
        $display("Test3: %d / %d = %d (Remainder %d)", dividend, divisor, quotient, remainder);

        $stop;
    end
endmodule