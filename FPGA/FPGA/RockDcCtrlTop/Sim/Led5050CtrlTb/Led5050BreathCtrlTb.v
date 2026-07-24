`timescale 1ns / 1ns
module Led5050BreathCtrlTb;

    //-------------------------------Code Start Here--------------------------------


    parameter PERIOD_SYS = 50;
    parameter LED_COUNT = 4;

    parameter PERIOD = 32'd20000000;
    parameter R_DELTA_PERIOD = 24'd200000;
    parameter G_DELTA_PERIOD = 24'd133333;
    parameter B_DELTA_PERIOD = 24'd100000;

    reg         rSysClk = 1'b0;

    initial begin
        forever
        #(PERIOD_SYS / 2) begin
            rSysClk = ~rSysClk;
        end
    end

    Led5050BreathCtrl U_Led5050BreathCtrl (
        .iSysClk      (rSysClk),
        .iPeriod      (PERIOD),
        .iRDeltaPeriod(R_DELTA_PERIOD),
        .iGDeltaPeriod(G_DELTA_PERIOD),
        .iBDeltaPeriod(B_DELTA_PERIOD)
    );

    //******************************************************************************







endmodule
