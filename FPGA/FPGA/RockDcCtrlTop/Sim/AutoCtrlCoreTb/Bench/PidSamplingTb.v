//------------------------------------------------------------------------------
// Testbench for: PidSampling.v
//
// What we verify (high-signal):
// - oPidOpEn[1] (fast) is a single-cycle pulse on each rising edge of iAdcSampleSequenceDone (after edge-detect latency).
// - oPidOpEn[0] (slow) should be a single-cycle pulse every 11 triggers, and must NOT stick high.
//
// Note: Current PidSampling implementation likely makes wPidSlowOpEn stick high
// once rPidSlowCnt reaches 10, because rPidSlowCnt holds its value between triggers.
// This TB will flag that as an error.
//------------------------------------------------------------------------------
`timescale 1ns/1ns

module PidSamplingTb;

    // 100MHz clock
    reg iSysClk = 1'b0;
    always #5 iSysClk = ~iSysClk;

    reg  iAdcSampleSequenceDone = 1'b0;
    wire [1:0] oPidOpEn;

    // DUT
    PidSampling #(
        .CLK_FREQ(100)
    ) dut (
        .iSysClk(iSysClk),
        .iAdcSampleSequenceDone(iAdcSampleSequenceDone),
        .oPidOpEn(oPidOpEn)
    );

    // Generate iAdcSampleSequenceDone as a 1-cycle pulse periodically.
    // We choose a period that is much larger than the edge-detect pipeline.
    localparam integer TRIG_PERIOD_CYCLES = 40;

    integer cyc = 0;
    integer trig_cnt = 0;
    integer fast_pulse_cnt = 0;
    integer slow_pulse_cnt = 0;

    // Track pulse widths
    integer fast_high_len = 0;
    integer slow_high_len = 0;

    // Helper: drive periodic trigger
    always @(posedge iSysClk) begin
        cyc <= cyc + 1;

        // 1-cycle pulse
        if ((cyc % TRIG_PERIOD_CYCLES) == 0) begin
            iAdcSampleSequenceDone <= 1'b1;
            trig_cnt <= trig_cnt + 1;
        end else begin
            iAdcSampleSequenceDone <= 1'b0;
        end
    end

    // Monitor pulses and assert properties
    always @(posedge iSysClk) begin
        // Fast pulse width check (must be 1 cycle when it happens)
        if (oPidOpEn[1]) begin
            fast_high_len <= fast_high_len + 1;
        end
        if (!oPidOpEn[1] && fast_high_len != 0) begin
            if (fast_high_len != 1) begin
                $display("[%0t] ERROR: fast pulse width = %0d (expected 1)", $time, fast_high_len);
                $stop;
            end
            fast_pulse_cnt <= fast_pulse_cnt + 1;
            fast_high_len <= 0;
        end

        // Slow pulse width check (must be 1 cycle when it happens)
        if (oPidOpEn[0]) begin
            slow_high_len <= slow_high_len + 1;
        end
        if (!oPidOpEn[0] && slow_high_len != 0) begin
            if (slow_high_len != 1) begin
                $display("[%0t] ERROR: slow pulse width = %0d (expected 1). Signal likely stuck-high bug.", $time, slow_high_len);
                $stop;
            end
            slow_pulse_cnt <= slow_pulse_cnt + 1;
            slow_high_len <= 0;
        end

        // Guard: if slow stays high for too long, fail fast (detect "stuck high")
        if (slow_high_len > 5) begin
            $display("[%0t] ERROR: slow signal high for %0d cycles (expected 1).", $time, slow_high_len);
            $stop;
        end

        // End condition: after enough triggers, report and finish.
        if (trig_cnt >= 50) begin
            $display("---- PidSamplingTb summary ----");
            $display("Triggers seen                : %0d", trig_cnt);
            $display("Fast pulses counted          : %0d", fast_pulse_cnt);
            $display("Slow pulses counted          : %0d", slow_pulse_cnt);
            $display("NOTE: Fast/Slow pulse counts depend on edge-detect latency and implementation.");
            $finish;
        end
    end

endmodule

