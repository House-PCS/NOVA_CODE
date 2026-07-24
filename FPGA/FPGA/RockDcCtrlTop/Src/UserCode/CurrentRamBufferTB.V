`timescale 1ns/1ps  // 时间单位1ns，精度1ps

module CurrentRamBufferTB;

// 1. 定义仿真信号
reg         iWrClk;
reg         iRdClk;
reg         iWrStart;
reg         iRdEn;
reg         iAdcSampleSequenceDone;
reg  signed [15:0] iBusTotalCurrentA;
reg  signed [15:0] iBusTotalCurrentB;
wire signed [15:0] oBusTotalCurrentA;
wire signed [15:0] oBusTotalCurrentB;

// 2. 例化待测试模块
CurrentRamBuffer uut (
    .iWrClk                 (iWrClk),
    .iRdClk                 (iRdClk),
    .iWrStart               (iWrStart),
    .iRdEn                  (iRdEn),
    .iAdcSampleSequenceDone (iAdcSampleSequenceDone),
    .iBusTotalCurrentA      (iBusTotalCurrentA),
    .iBusTotalCurrentB      (iBusTotalCurrentB),
    .oBusTotalCurrentA      (oBusTotalCurrentA),
    .oBusTotalCurrentB      (oBusTotalCurrentB)
);

// 3. 生成异步时钟
initial begin
    iWrClk = 0;
    forever #10 iWrClk = ~iWrClk;  // 50MHz (周期20ns)
end

initial begin
    iRdClk = 0;
    forever #20 iRdClk = ~iRdClk;  // 25MHz (周期40ns)
end

// 4. 测试激励时序
initial begin
    // 初始化信号
    iWrStart               <= 1'b0;
    iRdEn                  <= 1'b0;
    iAdcSampleSequenceDone <= 1'b0;
    iBusTotalCurrentA      <= 16'sd0;
    iBusTotalCurrentB      <= 16'sd0;
    #50;  // 等待时钟稳定

    // ========== 场景1：写地址复位 ==========
    iWrStart <= 1'b1;
    #80;   // 写时钟1个周期
     iWrStart <= 1'b0;
    #20;
    $display("写地址复位后: rWrAddr = %0d", uut.rWrAddr);  // 预期0

    // ========== 场景2：写入4096个测试数据 ==========
    for (integer i = 0; i < 4096; i=i+1) begin
        // 输入递增的测试数据
        iBusTotalCurrentA <= -16'sd100;
        iBusTotalCurrentB <= i + 100;
        // 产生写使能脉冲（iAdcSampleSequenceDone）
        iAdcSampleSequenceDone <= 1'b1;
        #20;  // 写时钟1个周期
        iAdcSampleSequenceDone <= 1'b0;
        #80;  // 写时钟1个周期
    end
    $display("写入4096个数据后: rWrAddr = %0d", uut.rWrAddr);  // 预期4096

    // ========== 场景3：地址边界验证（写地址不再递增） ==========
    iBusTotalCurrentA = 16'sd9999;
    iBusTotalCurrentB = 16'sd8888;
    iAdcSampleSequenceDone = 1'b1;
    #20;
    iAdcSampleSequenceDone = 1'b0;
    $display("边界后写地址: rWrAddr = %0d", uut.rWrAddr);  // 预期仍为4096

    // ========== 场景4：读取写入的数据 ==========
    #20;
    iRdEn = 1'b1;
    for (integer j = 0; j < 4096; j=j+1) begin
        #40;  // 读时钟1个周期
    end

    // ========== 场景5：读地址边界验证（读地址到4095后不再递增） ==========
    #40;
    $display("读地址到4095后: rRdAddr = %0d", uut.rRdAddr);  // 预期4095
    #40;
    $display("继续读使能后: rRdAddr = %0d", uut.rRdAddr);    // 预期仍为4095

    // ========== 结束仿真 ==========
    #100;
        iWrStart = 1'b1;
    #80;   // 写时钟1个周期
    $finish;
end


endmodule
