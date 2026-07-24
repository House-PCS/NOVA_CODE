`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/09/03 15:30:00
// Design Name: 
// Module Name: SignedDiv100_16bitTb
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


module SignedDiv100_16bitTb();

    reg         iSysClk;
    reg         iSysRst;
    reg signed [15:0] iData;
    reg         iData_Valid;
    wire signed [15:0] oData;
    wire        oData_Valid;
    
    // 实例化被测模块
    signed_div100_16bit uut (
        .iSysClk(iSysClk),
        .iSysRst(iSysRst),
        .iData(iData),
        .iData_Valid(iData_Valid),
        .oData(oData),
        .oData_Valid(oData_Valid)
    );
    
    // 时钟生成
    always #5 iSysClk = ~iSysClk;
    
    // 测试向量
    integer i;
    real expected, actual, error;
    
    initial begin
        // 初始化
        iSysClk = 0;
        iSysRst = 1;
        iData = 0;
        iData_Valid = 0;
        
        // 复位
        #20 iSysRst = 0;
        #10;
        
        // 测试用例1：边界值测试
        $display("=== 边界值测试 ===");
        test_case(16'sd100, 1);      // 100/100 = 1
        test_case(16'sd1000, 10);    // 1000/100 = 10
        test_case(16'sd32767, 327);  // 最大值/100
        test_case(-16'sd32768, -327); // 最小值/100
        
        // 测试用例2：随机值测试
        $display("\n=== 随机值测试 ===");
        for (i = 0; i < 20; i = i + 1) begin
            test_case($random % 32768, 0);
        end
        
        // 测试用例3：连续数据流测试
        $display("\n=== 连续数据流测试 ===");
        iData_Valid = 1;
        for (i = 1; i <= 20; i = i + 1) begin
            iData = i * 100;
            #10;
        end
        iData_Valid = 0;
    //   rgba(128, 63, 63, 1);
        $display("测试完成!");
        $finish;
    end
    
    // 测试任务
    task test_case;
        input signed [15:0] test_input;
        input integer expected_result;
        begin
            iData = test_input;
            iData_Valid = 1;
            #10;
            iData_Valid = 0;
            
            // 等待结果
            #30;
            
            expected = test_input / 100.0;
            actual = oData;
            error = (actual - expected) / expected * 100;
            
            if (expected_result != 0) begin
                $display("输入: %d, 预期: %d, 实际: %d, 误差: %.2f%%", 
                        test_input, expected_result, oData, error);
            end else begin
                $display("输入: %d, 实际: %d, 浮点结果: %.2f, 误差: %.2f%%", 
                        test_input, oData, expected, error);
            end
            #10;
        end
    endtask
    
    // 监控输出
    always @(posedge iSysClk) begin
        if (oData_Valid) begin
            $display("时钟 %t: 输出有效, 数据 = %d", $time, oData);
        end
    end
    
    // 波形导出
    // initial begin
    //     $dumpfile("tb_signed_div100_16bit.vcd");
    //     $dumpvars(0, tb_signed_div100_16bit);
    // end
endmodule