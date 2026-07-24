`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/09/05 09:48:01
// Design Name: 
// Module Name: Division100Tb
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


module Division100Tb();


reg clk;
reg rst;
reg signed [15:0] data_in [0:7];
wire signed [15:0] data_out [0:7];

// 实例化被测模块
Division100_16bit #(
    .CLK_FREQ(100)
) uut (
    .iSysClk(clk),
    .iSysRst(rst),
    .iDataIn0(data_in[0]),
    .iDataIn1(data_in[1]),
    .iDataIn2(data_in[2]),
    .iDataIn3(data_in[3]),
    .iDataIn4(data_in[4]),
    .iDataIn5(data_in[5]),
    .iDataIn6(data_in[6]),
    .iDataIn7(data_in[7]),
    .oDataOut0(data_out[0]),
    .oDataOut1(data_out[1]),
    .oDataOut2(data_out[2]),
    .oDataOut3(data_out[3]),
    .oDataOut4(data_out[4]),
    .oDataOut5(data_out[5]),
    .oDataOut6(data_out[6]),
    .oDataOut7(data_out[7])
);

// 时钟生成
initial begin
    clk = 0;
    forever #5 clk = ~clk;
end

// 测试用例
initial begin
    // 初始化
    rst = 1;
    data_in[0] = 16'd0;
    data_in[1] = 16'd0;
    data_in[2] = 16'd0;
    data_in[3] = 16'd0;
    data_in[4] = 16'd0;
    data_in[5] = 16'd0;
    data_in[6] = 16'd0;
    data_in[7] = 16'd0;
    
    // 复位
    #20 rst = 0;
    
    // 测试用例1: 正数除法
    #100;
    data_in[0] = 16'd1000;
    data_in[1] = 16'd2000;
    data_in[2] = 16'd3000;
    data_in[3] = 16'd4000;
    data_in[4] = 16'd5000;
    data_in[5] = 16'd6000;
    data_in[6] = 16'd7000;
    data_in[7] = 16'd8000;
    
    // 等待计算完成
    #2000;
    
    // 检查结果 (1000/100=10, 2000/100=20, 等等)
    $display("Test 1 - Positive Numbers:");
    $display("Input: %d, Output: %d, Expected: %d", data_in[0], data_out[0], 10);
    $display("Input: %d, Output: %d, Expected: %d", data_in[1], data_out[1], 20);
    $display("Input: %d, Output: %d, Expected: %d", data_in[2], data_out[2], 30);
    $display("Input: %d, Output: %d, Expected: %d", data_in[3], data_out[3], 40);
    $display("Input: %d, Output: %d, Expected: %d", data_in[4], data_out[4], 50);
    $display("Input: %d, Output: %d, Expected: %d", data_in[5], data_out[5], 60);
    $display("Input: %d, Output: %d, Expected: %d", data_in[6], data_out[6], 70);
    $display("Input: %d, Output: %d, Expected: %d", data_in[7], data_out[7], 80);
    
    // 测试用例2: 负数除法
    #100;
    data_in[0] = -16'd1000;
    data_in[1] = -16'd2000;
    data_in[2] = -16'd3000;
    data_in[3] = -16'd4000;
    data_in[4] = -16'd5000;
    data_in[5] = -16'd6000;
    data_in[6] = -16'd7000;
    data_in[7] = -16'd8000;
    
    // 等待计算完成
    #2000;
    
    $display("\nTest 2 - Negative Numbers:");
    $display("Input: %d, Output: %d, Expected: %d", data_in[0], data_out[0], -10);
    $display("Input: %d, Output: %d, Expected: %d", data_in[1], data_out[1], -20);
    $display("Input: %d, Output: %d, Expected: %d", data_in[2], data_out[2], -30);
    $display("Input: %d, Output: %d, Expected: %d", data_in[3], data_out[3], -40);
    $display("Input: %d, Output: %d, Expected: %d", data_in[4], data_out[4], -50);
    $display("Input: %d, Output: %d, Expected: %d", data_in[5], data_out[5], -60);
    $display("Input: %d, Output: %d, Expected: %d", data_in[6], data_out[6], -70);
    $display("Input: %d, Output: %d, Expected: %d", data_in[7], data_out[7], -80);
    
    // 测试用例3: 边界值测试
    #100;
    data_in[0] = 16'd32767;  // 最大正数
    data_in[1] = -16'd32768; // 最小负数
    data_in[2] = 16'd0;      // 零
    data_in[3] = 16'd1;      // 最小正数
    data_in[4] = -16'd1;     // 最大负数
    data_in[5] = 16'd99;     // 小于100
    data_in[6] = 16'd100;    // 等于100
    data_in[7] = 16'd101;    // 大于100
    
    // 等待计算完成
    #2000;
    
    $display("\nTest 3 - Boundary Values:");
    $display("Input: %d, Output: %d", data_in[0], data_out[0]);
    $display("Input: %d, Output: %d", data_in[1], data_out[1]);
    $display("Input: %d, Output: %d", data_in[2], data_out[2]);
    $display("Input: %d, Output: %d", data_in[3], data_out[3]);
    $display("Input: %d, Output: %d", data_in[4], data_out[4]);
    $display("Input: %d, Output: %d", data_in[5], data_out[5]);
    $display("Input: %d, Output: %d", data_in[6], data_out[6]);
    $display("Input: %d, Output: %d", data_in[7], data_out[7]);
    
    $display("\nTest completed!");
    $finish;
end

// 波形输出
// initial begin
//     $dumpfile("division10_16bit.vcd");
//     $dumpvars(0, tb_Division10_16bit);
// end

endmodule
