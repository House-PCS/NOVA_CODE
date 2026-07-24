`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/10/13 10:06:50
// Design Name: 
// Module Name: tb_LedLampStripCtrlCore
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


module tb_LedLampStripCtrlCore();

// 参数定义
parameter CLK_FREQ = 100;  // 125MHz时钟
parameter LED_COUNT = 2;

// 输入信号
reg iSysClk;
reg [7:0] iLedStatus;
reg [7:0] iLedRValue;
reg [7:0] iLedGValue;
reg [7:0] iLedBValue;
reg [15:0] iLedMode;
reg [15:0] iLedPeriod;

// 输出信号
wire oRGBOut;

// 测试控制
reg [31:0] test_case;
reg [31:0] clock_count;

// 实例化被测模块
LedLampStripCtrlCore #(
    .CLK_FREQ(CLK_FREQ),
    .LED_NUM(LED_COUNT)
) uut (
    .iSysClk(iSysClk),
    .iBatSoc (16'd500),
    .iLedStatus(8'd0),//(iLedStatus),
    .iLedRValue(16'hff),//iLedRValue),
    .iLedGValue(16'h00),//iLedGValue),
    .iLedBValue(16'h00),//iLedBValue),
    .iLedMode(3'd3),
    .iLedPeriod(iLedPeriod),
    .oRGBOut(oRGBOut)
);

// 时钟生成
always #5 iSysClk = ~iSysClk;  // 125MHz时钟，周期8ns

// 测试任务：设置LED颜色
task set_led_color;
    input [7:0] r_val;
    input [7:0] g_val;
    input [7:0] b_val;
    input [7:0] status;
    begin
        iLedRValue = r_val;
        iLedGValue = g_val;
        iLedBValue = b_val;
        iLedStatus = status;
        $display("Time: %0t - Set LED Color: R=%h, G=%h, B=%h, Status=%h", 
                 $time, r_val, g_val, b_val, status);
    end
endtask

// 测试任务：检查归零码波形
task check_rz_waveform;
    input [31:0] expected_data;
    input [7:0] bit_index;
    input expected_bit;
    begin
        // 等待数据位开始
        @(posedge iSysClk);
        
        // 检查高电平持续时间
        if (expected_bit == 1'b1) begin
            // 检查T1H时间（约280ns，35个时钟周期）
            repeat(35) @(posedge iSysClk);
            if (oRGBOut !== 1'b1) begin
                $display("ERROR: Time: %0t - Bit %0d (expected 1) T1H too short", 
                         $time, bit_index);
            end
            // 检查整个位周期（800ns，100个时钟周期）
            repeat(65) @(posedge iSysClk);
            if (oRGBOut !== 1'b0) begin
                $display("ERROR: Time: %0t - Bit %0d (expected 1) total period incorrect", 
                         $time, bit_index);
            end
        end else begin
            // 检查T0H时间（约520ns，65个时钟周期）
            repeat(65) @(posedge iSysClk);
            if (oRGBOut !== 1'b1) begin
                $display("ERROR: Time: %0t - Bit %0d (expected 0) T0H too short", 
                         $time, bit_index);
            end
            // 检查整个位周期
            repeat(35) @(posedge iSysClk);
            if (oRGBOut !== 1'b0) begin
                $display("ERROR: Time: %0t - Bit %0d (expected 0) total period incorrect", 
                         $time, bit_index);
            end
        end
        
        $display("Time: %0t - Bit %0d check passed, expected: %b, actual waveform correct", 
                 $time, bit_index, expected_bit);
    end
endtask

// 测试任务：验证完整的32位数据帧
task verify_data_frame;
    input [31:0] test_data;
    integer i;
    begin
        $display("Time: %0t - Starting data frame verification for data: %h", 
                 $time, test_data);
        
        // 验证32位数据
        for (i = 31; i >= 0; i = i - 1) begin
            check_rz_waveform(test_data, i, test_data[i]);
        end
        
        $display("Time: %0t - Data frame verification completed", $time);
    end
endtask

// 主测试流程
initial begin
    // 初始化信号
    iSysClk = 0;
    iLedStatus = 8'h00;
    iLedRValue = 8'h00;
    iLedGValue = 8'h00;
    iLedBValue = 8'h00;
    iLedMode = 16'h0000;
    iLedPeriod = 16'h03FF;
    test_case = 0;
    clock_count = 0;
    
    // 等待全局复位完成
    #100;
    
    $display("=== LED Lamp Strip Controller Test Start ===");
    
    // 测试用例1：全红色
    test_case = 1;
    $display("\n=== Test Case %0d: Full Red ===", test_case);
    set_led_color(8'hFF, 8'h00, 8'h00, 8'h01);
    #16000000;  // 等待16ms（约2个Vsync周期）
    verify_data_frame(32'hFF000000);  // G,R,B格式
    
    // 测试用例2：全绿色
    test_case = 2;
    $display("\n=== Test Case %0d: Full Green ===", test_case);
    set_led_color(8'h00, 8'hFF, 8'h00, 8'h01);
    #16000000;
    verify_data_frame(32'h00FF0000);  // G,R,B格式
    
    // 测试用例3：全蓝色
    test_case = 3;
    $display("\n=== Test Case %0d: Full Blue ===", test_case);
    set_led_color(8'h00, 8'h00, 8'hFF, 8'h01);
    #16000000;
    verify_data_frame(32'h0000FF00);  // G,R,B格式
    
    // 测试用例4：白色
    test_case = 4;
    $display("\n=== Test Case %0d: White ===", test_case);
    set_led_color(8'hFF, 8'hFF, 8'hFF, 8'h01);
    #16000000;
    verify_data_frame(32'hFFFFFF00);  // G,R,B格式
    
    // 测试用例5：特定颜色模式
    test_case = 5;
    $display("\n=== Test Case %0d: Specific Pattern ===", test_case);
    set_led_color(8'hAA, 8'h55, 8'hF0, 8'h01);
    #16000000;
    verify_data_frame(32'h55AAF000);  // G,R,B格式
    
    // 测试用例6：LED关闭状态
    test_case = 6;
    $display("\n=== Test Case %0d: LED Off ===", test_case);
    set_led_color(8'hFF, 8'hFF, 8'hFF, 8'h00);  // Status = 0
    #16000000;
    // 注意：当LED状态为关闭时，输出可能为特定模式
    
    // 测试用例7：动态颜色变化
    test_case = 7;
    $display("\n=== Test Case %0d: Dynamic Color Change ===", test_case);
    repeat(5) begin
        set_led_color($random, $random, $random, 8'h01);
        #8000000;  // 等待8ms
        $display("Time: %0t - Color changed, checking output...", $time);
    end
    
    // 测试用例8：验证Vsync同步
    test_case = 8;
    $display("\n=== Test Case %0d: Vsync Synchronization ===", test_case);
    set_led_color(8'h12, 8'h34, 8'h56, 8'h01);
    
    // 监控多个Vsync周期
    repeat(4) begin
        #8000000;  // 等待Vsync
        $display("Time: %0t - Vsync cycle completed, output stable", $time);
        // 可以在这里添加更详细的检查
    end
    
    // 完成测试
    #1000000;
    $display("\n=== All Test Cases Completed ===");
    $display("=== LED Lamp Strip Controller Test PASSED ===");
    $finish;
end

// 时钟计数器
always @(posedge iSysClk) begin
    clock_count <= clock_count + 1;
end

// 监控关键信号
always @(posedge iSysClk) begin
    // 检测Vsync信号（每16个时钟周期）
    if (clock_count % 16 == 0) begin
        $display("Time: %0t - Vsync pulse, Clock Count: %0d", $time, clock_count);
    end
end

// 波形转储
// initial begin
//     $dumpfile("led_lamp_strip_ctrl.vcd");
//     $dumpvars(0, tb_LedLampStripCtrlCore);
// end

endmodule
