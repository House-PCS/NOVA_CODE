`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/03/05 22:26:00
// Design Name: 
// Module Name: SelfInspectionCoreTb
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


module SelfInspectionCoreTb();

    // Parameters
    parameter CLK_FREQ = 100; // MHz

    // Inputs
    reg iSysClk;
    reg iSysRst;

    reg [11:0] iLlcBusVoltageA;
    reg [11:0] iLlcBusVoltageB;
    reg [11:0] iBusVoltageA;
    reg [11:0] iBusVoltageB;
    reg [11:0] iBusTotalVoltage;
    reg [11:0] iBusTotalVoltagePre;

    reg [11:0] iVoltageSampVref;
    reg [11:0] iCurrentSampVref;

    reg [1:0] iPcsHardwareId;
    reg iSinglePhasePcsEn;
    reg iThreePhasePcsEn;
    reg [1:0] iPcsSoftWareId;

    wire iAdcSampleSequenceDone;
    reg iSelfInspectionEn;

    // Outputs
    wire [11:0] oVoltageSampVrefAvera;
    wire [11:0] oCurrentSampVrefAvera;
    wire oSeriesRelayEn;
    wire oParallelRelayEn;
    wire oSelfInspectionEnd;
    
    // Instantiate the Unit Under Test (UUT)
    SelfInspectionCore #(
        .CLK_FREQ(CLK_FREQ)
    ) uut (
        .iSysClk(iSysClk),
        .iSysRst(iSysRst),

        .iLlcBusVoltageA(iLlcBusVoltageA),
        .iLlcBusVoltageB(iLlcBusVoltageB),
        .iBusVoltageA(iBusVoltageA),
        .iBusVoltageB(iBusVoltageB),
        .iBusTotalVoltage(iBusTotalVoltage),
        .iBusTotalVoltagePre(iBusTotalVoltagePre),

        .iVoltageSampVref(iVoltageSampVref),
        .iCurrentSampVref(iCurrentSampVref),

        .iPcsHardwareId(iPcsHardwareId),
        .iSinglePhasePcsEn(iSinglePhasePcsEn),
        .iThreePhasePcsEn(iThreePhasePcsEn),
        .iPcsSoftWareId(iPcsSoftWareId),

        .iAdcSampleSequenceDone(iAdcSampleSequenceDone),
        .iSelfInspectionEn(iSelfInspectionEn),
        .oSelfInspectionEnd(oSelfInspectionEnd),

        .oVoltageSampVrefAvera(oVoltageSampVrefAvera),
        .oCurrentSampVrefAvera(oCurrentSampVrefAvera),

        .oSeriesRelayEn(oSeriesRelayEn),
        .oParallelRelayEn(oParallelRelayEn)
    );

    // Clock generation
    initial begin
        iSysClk = 0;
        forever #5 iSysClk = ~iSysClk; // 100 MHz clock
    end

    // Test sequence
    initial begin
        // Initialize Inputs
        iSysRst = 1;
        iLlcBusVoltageA = 12'h000;
        iLlcBusVoltageB = 12'h000;
        iBusVoltageA = 12'h000;
        iBusVoltageB = 12'h000;
        iBusTotalVoltage = 12'h000;
        iBusTotalVoltagePre = 12'h000;

        iVoltageSampVref = 12'h000;
        iCurrentSampVref = 12'h000;

        iPcsHardwareId = 2'b00;
        iSinglePhasePcsEn = 0;
        iThreePhasePcsEn = 0;
        iPcsSoftWareId = 2'b00;

        iSelfInspectionEn = 0;

        // Wait for global reset
        #100;
        iSysRst = 0;

        // Apply test vectors
        #100;
        iLlcBusVoltageA = 12'h123;
        iLlcBusVoltageB = 12'h456;
        iBusVoltageA = 12'h789;
        iBusVoltageB = 12'hABC;
        iBusTotalVoltage = 12'hDEF;
        iBusTotalVoltagePre = 12'hFED;

        iVoltageSampVref = 12'h111;
        iCurrentSampVref = 12'h222;

        iPcsHardwareId = 2'b01;
        iSinglePhasePcsEn = 1;
        iThreePhasePcsEn = 0;
        iPcsSoftWareId = 2'b10;

        #1000;
        iSelfInspectionEn = 1;

        // Wait for inspection to complete
        #1000;

        // End simulation
//        $finish;
    end
reg [5:0] cnt = 6'd0;

always @(posedge iSysClk ) begin
    cnt[5:0] <= cnt[5:0] + 1'b1;
end

assign iAdcSampleSequenceDone = (cnt[5:0]==6'b111111);

    // Monitor outputs
    initial begin
        $monitor("Time: %0t | oVoltageSampVrefAvera: %h | oCurrentSampVrefAvera: %h | oSeriesRelayEn: %b | oParallelRelayEn: %b | oSelfInspectionEnd: %b",
                 $time, oVoltageSampVrefAvera, oCurrentSampVrefAvera, oSeriesRelayEn, oParallelRelayEn, oSelfInspectionEnd);
    end

endmodule
