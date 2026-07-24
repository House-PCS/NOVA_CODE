`timescale 1ns/1ps

module DebugTest_tb;

    // 参数定义
    parameter PARAM_COUNT = 8;
    parameter BUF_WIDTH = PARAM_COUNT * 32;

    // 信号定义
    reg                  iSysClk;
    wire                  iEthTxRdEn;
    reg  [BUF_WIDTH-1:0] iBuf;
    wire [7:0]           oEthTxData;
    wire [15:0]          oEthTxLen;
    wire                 oEthTxEn;
    wire oUartTxd;

    uart_send U_UartSend(
    .iSysClk                        (iSysClk                               ),

    .iUartEn                        (oEthTxEn                             ),
    .iUartDin                       (oEthTxData[7:0]                        ),
    .oUartTxBusy                    (iEthTxRdEn                         ),
    .oUartTxd                       (oUartTxd                         )
);

    // 实例化待测模块
    DebugTest #(
        .PARAM_COUNT(PARAM_COUNT)
    ) dut (
        .iSysClk(iSysClk),
        .iEthTxRdEn(iEthTxRdEn),
        .iBuf(iBuf),
        .oEthTxData(oEthTxData),
        .oEthTxLen(oEthTxLen),
        .oEthTxEn(oEthTxEn)
    );

    // 时钟生成
    initial iSysClk = 0;
    always #5 iSysClk = ~iSysClk; // 100MHz

    // 初始化输入
    integer idx;
    initial begin
        // iEthTxRdEn = 0;
        iBuf = 0;
        // 给iBuf赋初值
        for (idx = 0; idx < PARAM_COUNT; idx = idx + 1) begin
            iBuf[idx*32 +: 32] = 32'hA5A50000 + idx;
        end

        // #100;
        // // 产生iEthTxRdEn脉冲
        // repeat (PARAM_COUNT + 4) begin
        //     @(negedge iSysClk);
        //     iEthTxRdEn = 1;
        //     @(negedge iSysClk);
        //     iEthTxRdEn = 0;
        //     #20;
        // end

        // #1000;
        // $finish;
    end

    // 输出监控
    // initial begin
    //     $display("time\tEthTxEn\tEthTxData\tEthTxLen");
    //     $monitor("%0t\t%b\t%02x\t%04x", $time, oEthTxEn, oEthTxData, oEthTxLen);
    // end

endmodule