`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/03/11 21:10:42
// Design Name: 
// Module Name: ProtectCoreTb
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


module ProtectCoreTb();


// 输入信号
reg                     iSysClk;
reg                     iSysRst;
reg                     iWorkingEn;
reg  signed     [15:0]  iLlcBusVoltageA;
reg  signed     [15:0]  iLlcBusVoltageB;
reg  signed     [15:0]  iBusVoltageA;
reg  signed     [15:0]  iBusVoltageB;
reg  signed     [15:0]  iBusTotalVoltage;
reg  signed     [15:0]  iBusTotalVoltagePre;
reg  signed     [15:0]  iResonanceCurrentA;
reg  signed     [15:0]  iResonanceCurrentB;
reg  signed     [15:0]  iLlcBusCurrentA;
reg  signed     [15:0]  iLlcBusCurrentB;
reg  signed     [15:0]  iBusTotalCurrentA;
reg  signed     [15:0]  iBusTotalCurrentB;
reg  signed     [15:0]  iVoltageSampVref;
reg  signed     [15:0]  iCurrentSampVref;
reg  signed     [15:0]  iTemp1;
reg  signed     [15:0]  iTemp2;
reg  signed     [15:0]  iTemp3;
reg  signed     [15:0]  iTemp4;
reg  signed     [15:0]  iTemp5;
reg  signed     [15:0]  iTemp6;

// 输出信号
wire        oDcdcErr;
wire        oFailRecovery;
wire [15:0] oDCFailureInformation1;

// 时钟生成
initial begin
    iSysClk = 0;
    forever #5 iSysClk = ~iSysClk; // 100MHz时钟
end

// 测试激励
initial begin
    // 初始化
    iSysRst = 1;
    iWorkingEn = 0;
    iLlcBusVoltageA = 0;
    iLlcBusVoltageB = 0;
    iBusVoltageA = 0;
    iBusVoltageB = 0;
    iBusTotalVoltage = 0;
    iBusTotalVoltagePre = 0;
    iResonanceCurrentA = 0;
    iResonanceCurrentB = 0;
    iLlcBusCurrentA = 0;
    iLlcBusCurrentB = 0;
    iBusTotalCurrentA = 0;
    iBusTotalCurrentB = 0;
    iVoltageSampVref = 0;
    iCurrentSampVref = 0;
    iTemp1 = 0;
    iTemp2 = 0;
    iTemp3 = 0;
    iTemp4 = 0;
    iTemp5 = 0;
    iTemp6 = 0;
    #20;
    iSysRst = 0;
    #20;
    iWorkingEn = 1;

    // 场景1：正常值
    $display("Scenario 1: Normal values");
    iLlcBusVoltageA = 16'd3000; // 在范围内
    iLlcBusVoltageB = 16'd3000;
    iBusVoltageA = 16'd4000;
    iBusVoltageB = 16'd4000;
    iBusTotalVoltage = 16'd5000;
    iBusTotalVoltagePre = 16'd5000;
    iResonanceCurrentA = 16'sd10000; // 有符号数
    iResonanceCurrentB = -16'sd10000;
    iLlcBusCurrentA = 16'sd5000;
    iLlcBusCurrentB = -16'sd5000;
    iBusTotalCurrentA = 16'sd3000;
    iBusTotalCurrentB = -16'sd3000;
    iVoltageSampVref = 16'd2000;
    iCurrentSampVref = 16'd2000;
    iTemp1 = 16'd2000;
    iTemp2 = 16'd2000;
    iTemp3 = 16'd2000;
    iTemp4 = 16'd2000;
    iTemp5 = 16'd2000;
    iTemp6 = 16'd2000;
    #40;

    // 场景2：电压超限
    $display("Scenario 2: The voltage exceeds the threshold");
    iLlcBusVoltageA = 16'd4000; // 超过最大值3500
    iBusVoltageA = 16'd6000;    // 超过最大值5200
    iBusTotalVoltage = 16'd11000; // 超过最大值10000
    #35;

    // 场景3：电流超限
    $display("Scenario 3: The current exceeds the upper limit");
    //iResonanceCurrentA = 16'sd25000; // 超过最大值20000
    iResonanceCurrentB = -16'sd25000; // 低于最小值-20000
    //iLlcBusCurrentA = 16'sd12000;    // 超过最大值10000
    iLlcBusCurrentB = -16'sd12000;   // 低于最小值-10000
    #40;

    // 场景4：温度超限
    $display("Scenario 4: The temperature exceeds the limit");
    iTemp1 = 16'd5000; // 超过最大值4000
    iTemp2 = 16'd5000;
    #40;

    // 场景5：输入过载（过压且过流）
    $display("Scenario 5: Input overload");
    iBusTotalVoltagePre = 16'd11000; // 过压
    iBusTotalCurrentA = 16'sd8000;   // 过流
    #40;

    // 场景6：输出过载（过压且过流）
    $display("Scenario 6: Output overload");
    iBusTotalVoltage = 16'd11000;    // 过压
    iBusTotalCurrentB = 16'sd8000;   // 过流
    #40;

    // 场景7：恢复测试
    $display("Scenario 7: Recovery test");
    iLlcBusVoltageA = 16'd3000; // 恢复正常值
    iBusVoltageA = 16'd4000;
    iBusTotalVoltage = 16'd5000;
    iResonanceCurrentA = 16'sd10000;
    iResonanceCurrentB = -16'sd10000;
    iLlcBusCurrentA = 16'sd5000;
    iLlcBusCurrentB = -16'sd5000;
    iBusTotalCurrentA = 16'sd3000;
    iBusTotalCurrentB = -16'sd3000;
    iTemp1 = 16'd2000;
    iTemp2 = 16'd2000;
    #40;

    $finish;
end

// 实例化被测模块
ProtectCore uut (
    .iSysClk(iSysClk),
    .iSysRst(iSysRst),
    .iWorkingEn(iWorkingEn),
    .iLlcBusVoltageA(iLlcBusVoltageA),
    .iLlcBusVoltageB(iLlcBusVoltageB),
    .iBusVoltageA(iBusVoltageA),
    .iBusVoltageB(iBusVoltageB),
    .iBusTotalVoltage(iBusTotalVoltage),
    .iBusTotalVoltagePre(iBusTotalVoltagePre),
    .iResonanceCurrentA(iResonanceCurrentA),
    .iResonanceCurrentB(iResonanceCurrentB),
    .iLlcBusCurrentA(iLlcBusCurrentA),
    .iLlcBusCurrentB(iLlcBusCurrentB),
    .iBusTotalCurrentA(iBusTotalCurrentA),
    .iBusTotalCurrentB(iBusTotalCurrentB),
    .iVoltageSampVref(iVoltageSampVref),
    .iCurrentSampVref(iCurrentSampVref),
    .iTemp1(iTemp1),
    .iTemp2(iTemp2),
    .iTemp3(iTemp3),
    .iTemp4(iTemp4),
    .iTemp5(iTemp5),
    .iTemp6(iTemp6),
    .oDcdcErr(oDcdcErr),
    .oFailRecovery(oFailRecovery),
    .oDCFailureInformation1(oDCFailureInformation1)
);

// 监视输出
always @(posedge iSysClk) begin
    $display("Time=%t | Err=%b | FailRecovery=%b | Info=%h", 
             $time, oDcdcErr, oFailRecovery, oDCFailureInformation1);
end

endmodule