`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/10/29 21:55:00
// Design Name: 
// Module Name: CubeRootNewtonTb
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


module CubeRootNewtonTb();


// 参数定义
parameter WIDTH = 32;
parameter ITERATIONS = 20;

// 时钟和复位信号
reg iSysClk;
reg iSysRst;

// 输入信号
reg signed [WIDTH-1:0] iData;
reg iDataValid;

// 输出信号
wire signed [WIDTH-1:0] oData;
wire oDataValid;
wire oBusy;

// 测试用例变量
reg [31:0] test_cases [0:9];
reg signed [WIDTH-1:0] expected_results [0:9];
integer i;
integer error_count;

// 实例化被测模式
CubeRootNewton #(
    .WIDTH(WIDTH),
    .ITERATIONS(ITERATIONS)
) uut (
    .iSysClk(iSysClk),
    .iSysRst(iSysRst),
    .iData(iData),
    .iDataValid(iDataValid),
    .oData(oData),
    .oDataValid(oDataValid),
    .oBusy(oBusy)
);

// 时钟生成
always #5 iSysClk = ~iSysClk;

// 初始化测试用例
initial begin
    // 测试用例：输入 -> 期望的立方根（取整）
    test_cases[0] = 16'sd0;        expected_results[0] = 0;      // 0的立方根
    test_cases[1] = 16'sd1;        expected_results[1] = 1;      // 1的立方根
    test_cases[2] = 16'sd8;        expected_results[2] = 2;      // 8的立方根
    test_cases[3] = 16'sd27;       expected_results[3] = 3;      // 27的立方根
    test_cases[4] = 16'sd64;       expected_results[4] = 4;      // 64的立方根
    test_cases[5] = 16'sd125;      expected_results[5] = 5;      // 125的立方根
    test_cases[6] = 16'sd1000;     expected_results[6] = 10;     // 1000的立方根
    test_cases[7] = 16'sd32767;    expected_results[7] = 32;     // 32767的立方根  31.999
    test_cases[8] = -16'sd8;       expected_results[8] = -2;     // -8的立方根
    test_cases[9] = -16'sd27;      expected_results[9] = -3;     // -27的立方根
end

// 主测试流程
initial begin
    // 初始化信号
    iSysClk = 0;
    iSysRst = 1;
    iData = 0;
    iDataValid = 0;
    error_count = 0;
    
    // 复位
    #20;
    iSysRst = 0;
    #20;
    
    //$display("开始立方根牛顿迭代法测试...");
    $display("==========================================");
    
    // 执行测试用例
    for (i = 0; i < 10; i = i + 1) begin
        run_test_case(i, test_cases[i], expected_results[i]);
        #100; // 等待一段时间再执行下一个测试
    end
    
    // 测试连续输入
    $display("测试连续输入...");
    test_continuous_input();
    
    // 测试忙信??
    $display("测试忙信号...");
    test_busy_signal();
    
    // 测试总结
    $display("==========================================");
    if (error_count == 0) begin
        $display("所有测试用例通过");
    end else begin
        $display("测试完成，共 %0d 个错误", error_count);
    end
    
    #100;
    $finish;
end

// 运行单个测试用例的任务
task run_test_case;
    input integer test_num;
    input [31:0] input_val;
    input signed [WIDTH-1:0] expected;
    
    integer start_time;
    integer end_time;
    integer cycles;
    begin
        $display("测试用例 %0d: 输入 = %0d, 期望立方根 = %0d", 
                 test_num, $signed(input_val), $signed(expected));
        
        // 设置输入
        iData = input_val;
        iDataValid = 1;
        start_time = $time;
        
        // 等待有效输出
        wait(oDataValid);
        end_time = $time;
        cycles = (end_time - start_time) / 10; // 计算时钟周期
        
        // 验证结果
        if (oData !== expected) begin
            $display("错误: 期望 = %0d, 实际 = %0d", 
                     $signed(expected), $signed(oData));
            error_count = error_count + 1;
        end else begin
            $display("通过! 计算耗时 %0d 个时钟周期", cycles);
        end
        
       // iDataValid = 0;
        #20;
    end
endtask

// 测试连续输入
task test_continuous_input;
    integer j;
    begin
        $display("连续输入测试...");
        
        for (j = 0; j < 5; j = j + 1) begin
            iData = test_cases[j];
            iDataValid = 1;
            #10;
        end
        
        iDataValid = 0;
        
        // 等待所有有输出完成
        #500;
    end
endtask


// 测试忙信??
task test_busy_signal;
    begin
        $display("检查忙信号行为：");
        
        // 初始状???应该不??
        if (oBusy !== 0) begin
            $display("错误: 初始状???忙信号应为0");
            error_count = error_count + 1;
        end
        
        // 启动计算
        iData = 64;
        iDataValid = 1;
        #10;
        
        // ??查是否变??
        if (oBusy !== 1) begin
            $display("错误: ??始计算后忙信号应??1");
            error_count = error_count + 1;
        end
        
      //  iDataValid = 0;
        
        // 等待计算完成
        wait(oDataValid);
        #10;
        
        // ??查是否恢复不??
        if (oBusy !== 0) begin
            $display("错误: 计算完成后忙信号应为0");
            error_count = error_count + 1;
        end
        
        $display("忙信号测试完??");
    end
endtask

// 监视器：实时显示重要信号
always @(posedge iSysClk) begin
    if (oDataValid) begin
        $display("时间 %0t: 立方根计算结?? = %0d", $time, $signed(oData));
    end
end 

// 断言????
always @(posedge iSysClk) begin
    // ??查数据有效信号不会持续多个周??
    // if (oDataValid && $past(oDataValid)) begin
    //     $display("警告: oDataValid 持续了多个时钟周??");
    // end
    
// ??查输出数据范??
    if (oDataValid) begin
        assert (oData >= -32768 && oData <= 32767) 
        else $error("输出数据超出范围: %0d", $signed(oData));
    end
end

endmodule