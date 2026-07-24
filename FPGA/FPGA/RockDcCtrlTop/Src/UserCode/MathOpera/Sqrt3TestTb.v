`timescale 1ns/1ps

module Sqrt3TestTb;

    parameter WIDTH = 32;
    parameter ITER_NUM = 10;

    reg                  clk;
    reg                  rst_n;
    reg                  start;
    reg  signed [WIDTH-1:0] din;
    wire signed [WIDTH-1:0] dout;
    wire                 done;

    // 实例化待测模块
    Sqrt3Test #(
        .WIDTH(WIDTH),
        .ITER_NUM(ITER_NUM)
    ) uut (
        .clk(clk),
        .rst_n(rst_n),
        .start(start),
        .din(din),
        .dout(dout),
        .done(done)
    );

    // 时钟生成
    initial clk = 0;
    always #5 clk = ~clk;

    initial begin
        rst_n = 0;
        start = 0;
        din   = 0;
        #20;
        rst_n = 1;
        #20;

        // 测试1：din = 27，期望结果为3
        din = 32'sd27;
        start = 1;
        #10;
        start = 0;
        wait(done);
        $display("din=%0d, cube_root=%0d", din, dout);

        #50;

        // 测试2：din = 64，期望结果为4
        din = 32'sd64;
        start = 1;
        #10;
        start = 0;
        wait(done);
        $display("din=%0d, cube_root=%0d", din, dout);

        #50;

        // 测试3：din = 125，期望结果为5
        din = 32'sd125;
        start = 1;
        #10;
        start = 0;
        wait(done);
        $display("din=%0d, cube_root=%0d", din, dout);

        #50;

        // 测试4：din = -27，期望结果为-3
        din = -32'sd27;
        start = 1;
        #10;
        start = 0;
        wait(done);
        $display("din=%0d, cube_root=%0d", din, dout);

        #100;
        $stop;
    end

endmodule