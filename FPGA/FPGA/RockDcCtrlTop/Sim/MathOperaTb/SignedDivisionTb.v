`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/03/16 15:13:50
// Design Name: 
// Module Name: SignedDivisionTb
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


module SignedDivisionTb( );

// 参数定义
parameter WITH1 = 16;
parameter WITH2 = 8;

// 时钟和复位信号
reg iSysClk;
reg iSysRst;

// 输入信号
reg iStart;
reg signed [WITH1-1:0] iDividend;
reg signed [WITH2-1:0] iDivisor;

// 输出信号
wire signed [WITH1-1:0] oQuotient;
wire signed [WITH2-1:0] oRemainder;
wire oDone;
wire oBusy;

// 测试变量
integer test_count;
integer error_count;
integer i;

// 实例化被测模块
SignedDivision #(
    .WITH1(WITH1),
    .WITH2(WITH2)
) uut (
    .iSysClk(iSysClk),
    .iSysRst(iSysRst),
    .iStart(iStart),
    .iDividend(iDividend),
    .iDivisor(iDivisor),
    .oQuotient(oQuotient),
    .oRemainder(oRemainder),
    .oDone(oDone),
    .oBusy(oBusy)
);

// 时钟生成
always #5 iSysClk = ~iSysClk;

// 主测试流程
initial begin
    // 初始化
    iSysClk = 0;
    iSysRst = 1;
    iStart = 0;
    iDividend = 0;
    iDivisor = 0;
    test_count = 0;
    error_count = 0;
    
    // 复位
    #20;
    iSysRst = 0;
    #20;
    
    $display("==========================================");
    $display("开始带符号除法器测试");
    $display("参数: WITH1 = %0d, WITH2 = %0d", WITH1, WITH2);
    $display("==========================================");
    
    // 测试用例1: 正数除以正数
    $display("\n测试用例1: 正数除以正数");
    run_division_test(100, 7, 14, 2, "100 / 7 = 14 ... 2");
    run_division_test(255, 5, 51, 0, "255 / 5 = 51 ... 0");
    run_division_test(127, 3, 42, 1, "127 / 3 = 42 ... 1");
    
    // 测试用例2: 正数除以负数
    $display("\n测试用例2: 正数除以负数");
    run_division_test(100, -7, -14, 2, "100 / -7 = -14 ... 2");
    run_division_test(50, -6, -8, 2, "50 / -6 = -8 ... 2");
    
    // 测试用例3: 负数除以正数
    $display("\n测试用例3: 负数除以正数");
    run_division_test(-100, 7, -14, -2, "-100 / 7 = -14 ... -2");
    run_division_test(-50, 6, -8, -2, "-50 / 6 = -8 ... -2");
    
    // 测试用例4: 负数除以负数
    $display("\n测试用例4: 负数除以负数");
    run_division_test(-100, -7, 14, -2, "-100 / -7 = 14 ... -2");
    run_division_test(-50, -6, 8, -2, "-50 / -6 = 8 ... -2");
    
    // 测试用例5: 边界值测试
    $display("\n测试用例5: 边界值测试");
    run_division_test(32767, 1, 32767, 0, "32767 / 1 = 32767");
    run_division_test(-32768, 1, -32768, 0, "-32768 / 1 = -32768");
    run_division_test(1, 1, 1, 0, "1 / 1 = 1");
    run_division_test(0, 5, 0, 0, "0 / 5 = 0");
    
    // 测试用例6: 除数为1的特殊情况
    $display("\n测试用例6: 除数为1的情况");
    run_division_test(123, 1, 123, 0, "123 / 1 = 123");
    run_division_test(-456, 1, -456, 0, "-456 / 1 = -456");
    
    // 测试用例7: 被除数小于除数
    $display("\n测试用例7: 被除数小于除数");
    run_division_test(5, 10, 0, 5, "5 / 10 = 0 ... 5");
    run_division_test(-5, 10, 0, -5, "-5 / 10 = 0 ... -5");
    
    // 测试用例8: 连续除法测试
    $display("\n测试用例8: 连续除法测试");
    test_continuous_division();
    
    // 测试用例9: 控制信号测试
    $display("\n测试用例9: 控制信号测试");
    test_control_signals();
    
    // 测试总结
    $display("\n==========================================");
    $display("测试完成");
    $display("总测试用例: %0d", test_count);
    $display("错误数量: %0d", error_count);
    if (error_count == 0) begin
        $display("所有测试用例通过！");
    end else begin
        $display("存在错误，需要调试！");
    end
    $display("==========================================");
    
    #100;
    $finish;
end

// 执行单个除法测试的任务
task run_division_test;
    input signed [WITH1-1:0] dividend;
    input signed [WITH2-1:0] divisor;
    input signed [WITH1-1:0] expected_quotient;
    input signed [WITH2-1:0] expected_remainder;
    input [100:0] test_name;
    
    integer start_time;
    integer cycles;
    begin
        test_count = test_count + 1;
        $display("测试: %s", test_name);
        
        // 设置输入
        iDividend = dividend;
        iDivisor = divisor;
        iStart = 1;
        start_time = $time;
        
        #10; // 保持一个时钟周期
        iStart = 0;
        
        // 等待计算完成
        wait(oDone);
        cycles = ($time - start_time) / 10;
        
        // 验证结果
        if (oQuotient !== expected_quotient || oRemainder !== expected_remainder) begin
            $display("  错误!");
            $display("  期望: 商 = %0d, 余数 = %0d", expected_quotient, expected_remainder);
            $display("  实际: 商 = %0d, 余数 = %0d", oQuotient, oRemainder);
            error_count = error_count + 1;
        end else begin
            $display("  通过! 耗时 %0d 个时钟周期", cycles);
        end
        
        // 等待几个周期再执行下一个测试
        #50;
    end
endtask

// 连续除法测试
task test_continuous_division;
    integer i;
    begin
        $display("执行连续除法测试...");
        
        for (i = 0; i < 5; i = i + 1) begin
            iDividend = 100 + i * 20;
            iDivisor = 3 + i;
            iStart = 1;
            #10;
            iStart = 0;
            
            // 等待计算开始
            wait(oBusy);
            // 等待计算完成
            wait(oDone);
            
            $display("  %0d / %0d = %0d ... %0d", 
                     iDividend, iDivisor, oQuotient, oRemainder);
            
            #20;
        end
    end
endtask

// 控制信号测试
task test_control_signals;
    begin
        $display("测试控制信号...");
        
        // 测试初始状态
        if (oBusy !== 0) begin
            $display("  错误: 初始状态 oBusy 应为 0");
            error_count = error_count + 1;
        end
        
        if (oDone !== 0) begin
            $display("  错误: 初始状态 oDone 应为 0");
            error_count = error_count + 1;
        end
        
        // 启动计算
        iDividend = 100;
        iDivisor = 7;
        iStart = 1;
        #10;
        iStart = 0;
        
        // 检查忙信号
        #5; // 等待信号更新
        if (oBusy !== 1) begin
            $display("  错误: 计算开始后 oBusy 应为 1");
            error_count = error_count + 1;
        end
        
        // 等待计算完成
        wait(oDone);
        
        // 检查完成后的状态
        #5;
        if (oBusy !== 1) begin
            $display("  错误: oDone 有效时 oBusy 也应为 1");
            error_count = error_count + 1;
        end
        
        #20; // 等待几个周期
        
        if (oBusy !== 0) begin
            $display("  错误: 计算完成后 oBusy 应恢复为 0");
            error_count = error_count + 1;
        end
        
        if (oDone !== 0) begin
            $display("  错误: oDone 脉冲后应恢复为 0");
            error_count = error_count + 1;
        end
        
        $display("  控制信号测试完成");
    end
endtask

// 监视器：实时显示重要信号
always @(posedge iSysClk) begin
    if (oDone) begin
        $display("时间 %0t: 除法完成: %0d / %0d = %0d ... %0d", 
                 $time, iDividend, iDivisor, oQuotient, oRemainder);
    end
end

// 断言检查
always @(posedge iSysClk) begin
    // 检查数据一致性：被除数 = 商 * 除数 + 余数
    if (oDone && iDivisor != 0) begin
        if (iDividend !== oQuotient * iDivisor + oRemainder) begin
            $display("警告: 数据一致性检查失败");
            $display("  %0d != %0d * %0d + %0d", 
                     iDividend, oQuotient, iDivisor, oRemainder);
        end
    end
    
    // 检查余数范围
    if (oDone && iDivisor != 0) begin
        if (oRemainder >= iDivisor || oRemainder < 0 && -oRemainder >= -iDivisor) begin
            $display("警告: 余数超出有效范围");
        end
    end
end

// 波形保存（用于调试）
// initial begin
//     $dumpfile("tb_SignedDivision.vcd");
//     $dumpvars(0, tb_SignedDivision);
// end

endmodule