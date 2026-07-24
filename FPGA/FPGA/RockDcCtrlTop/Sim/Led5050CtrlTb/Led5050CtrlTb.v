`timescale 1ns / 1ns
module Led5050CtrlTb;

    //-------------------------------Code Start Here--------------------------------


    parameter PERIOD_SYS = 50;
    parameter LED_COUNT = 16;

    reg         rSysClk = 1'b0;
    reg         rLedOut;
    reg  [23:0] rData           [LED_COUNT-1:0];
    reg  [ 4:0] i;

    wire [23:0] wData;
    wire        wLedOutFinished;
    wire  wLedBusy;

    assign wData[23:0] = rData[i][23:0];

    initial begin
        for (i = 0; i < LED_COUNT; i = i + 1) begin
            rData[i] <= i;
        end
        rLedOut <= 1'b0;
        forever
        #(PERIOD_SYS / 2) begin
            rSysClk = ~rSysClk;
        end
    end

    always @(posedge rSysClk) begin
        if (wLedOutFinished == 1'b1) begin

            if (i >= 15) begin
                i <= 0;
            end else begin
                i <= i + 1;
            end

        end
        else if(wLedBusy == 0)begin
          i <= 0;
        end

    end

    Led5050CtrlCore #(
        .LED_COUNT(LED_COUNT)
    ) U_Led5050CtrlCore (
        .iSysClk   (rSysClk),
        .iRGBData  (wData),
        .oLedFinish(wLedOutFinished),
        .oLedBusy(wLedBusy)
    );

    //******************************************************************************







endmodule
