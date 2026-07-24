`timescale 1ns / 1ns
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/03/10 22:48:58
// Design Name: 
// Module Name: RelayCtrlCoreTb
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


module RelayCtrlCoreTb();


    // 参数定义
    parameter CLK_FREQ = 2;          // 时钟频率 100MHz
    parameter RELAY_DEAD_TIME = 1024;  // 死区时间 1024ms

    // 输入信号
    reg iSysClk;                       // 系统时钟
    reg iSeriesRelayEn;                // 串联继电器使能
    reg iParallelRelayEn;              // 并联继电器使能
    reg iMainRelayEn;                  // 主继电器使能
    reg iPrechargeRelayDis;            // 预充电继电器禁用

    // 输出信号
    wire oSeriesRelayEn;               // 串联继电器输出使能
    wire oParallelRelayEn;             // 并联继电器输出使能
    wire [1:0] oMainRelayEn;           // 主继电器输出使能
    wire [1:0] oPrechargeRelayDis;     // 预充电继电器输出禁用

    // 实例化被测模块
    RelayCtrlCore #(
        .CLK_FREQ(CLK_FREQ),
        .RELAY_DEAD_TIME(RELAY_DEAD_TIME)
    ) uut (
        .iSysClk(iSysClk),
        .iSeriesRelayEn(iSeriesRelayEn),
        .iParallelRelayEn(iParallelRelayEn),
        .iMainRelayEn(iMainRelayEn),
        .iPrechargeRelayDis(iPrechargeRelayDis),
        .oSeriesRelayEn(oSeriesRelayEn),
        .oParallelRelayEn(oParallelRelayEn),
        .oMainRelayEn(oMainRelayEn),
        .oPrechargeRelayDis(oPrechargeRelayDis)
    );

    // 时钟生成
    initial begin
        iSysClk = 0;
        forever #5 iSysClk = ~iSysClk; // 100MHz 时钟，周期 10ns
    end

    // 测试逻辑
    initial begin
        // 初始化
        iSeriesRelayEn = 0;
        iParallelRelayEn = 0;
        iMainRelayEn = 0;
        iPrechargeRelayDis = 0;
        #100; // 等待系统稳定

        // 测试 1: 串联继电器使能
        $display("Test 1: Series Relay Enable");
        iSeriesRelayEn = 1;   // 使能串联继电器
        iParallelRelayEn = 0; // 禁用并联继电器
        #(RELAY_DEAD_TIME * 1_100_000/(100/CLK_FREQ)); // 等待死区时间
        if (oSeriesRelayEn == 1)
            $display("Test 1 Passed: oSeriesRelayEn is asserted after dead time");
        else
            $display("Test 1 Failed: oSeriesRelayEn is not asserted");

        // 测试 2: 并联继电器使能
        $display("Test 2: Parallel Relay Enable");
        
        iParallelRelayEn = 1; // 使能并联继电器
        #10;
        iSeriesRelayEn = 0;   // 禁用串联继电器
        #(RELAY_DEAD_TIME * 1_100_000/(100/CLK_FREQ)); // 等待死区时间
        if (oParallelRelayEn == 1)
            $display("Test 2 Passed: oParallelRelayEn is asserted after dead time");
        else
            $display("Test 2 Failed: oParallelRelayEn is not asserted");

        // 测试 3: 串联关闭并联使能
        $display("Test 3: Parallel Relay Enable");
        iSeriesRelayEn = 0;  
        iParallelRelayEn = 0;
        #100;
        iSeriesRelayEn = 1;   // 使能串联继电器
        #200;
        iParallelRelayEn = 1; // 使能并联继电器
        #(RELAY_DEAD_TIME * 1_100_000/(100/CLK_FREQ)); // 等待死区时间
        if (oParallelRelayEn == 0 && oSeriesRelayEn == 0)
            $display("Test 3 Passed: oParallelRelayEn and oSeriesRelayEn is not asserted after dead time");
        else
            $display("Test 3 Failed: oParallelRelayEn or oSeriesRelayEn is asserted");


        // 测试 3: 主继电器使能
        $display("Test 4: Main Relay Enable");
        iMainRelayEn = 1; // 使能主继电器
        #100; // 等待一段时间
        if (oMainRelayEn == 2'b11)
            $display("Test 4 Passed: oMainRelayEn is asserted");
        else
            $display("Test 4 Failed: oMainRelayEn is not asserted");

        // 测试 4: 预充电继电器禁用
        $display("Test 5: Precharge Relay Disable");
        iPrechargeRelayDis = 1; // 禁用预充电继电器
        #100; // 等待一段时间
        if (oPrechargeRelayDis == 2'b11)
            $display("Test 5 Passed: oPrechargeRelayDis is asserted");
        else
            $display("Test 5 Failed: oPrechargeRelayDis is not asserted");

        // 结束测试
        $display("All tests completed");
        $finish;
    end

endmodule