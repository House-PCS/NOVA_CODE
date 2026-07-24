module uart_send (
    input               iSysClk,
    input               iSysRst,

    input               iUartEn,
    input       [7:0]   iUartDin,
    output              oUartTxBusy,
    output  reg         oUartTxd
);

parameter CLK_FREQ = 100000000;
parameter UART_BPS = 2000000;
localparam BPS_CNT = CLK_FREQ/UART_BPS;

reg rUartEnD0=0;
reg rUartEnD1=0;
reg [15:0] rClkCnt=0;
reg [3:0] rTxCnt=0;
reg rTxFlag=0;
reg [7:0] rTxData=0;

wire wEnFlag;

assign oUartTxBusy=rTxFlag;

assign wEnFlag=(~rUartEnD1)&rUartEnD0;

always @(posedge iSysClk or posedge iSysRst) begin
    if (iSysRst) begin
        rUartEnD0<=1'b0;
        rUartEnD1<=1'b0;
    end
    else begin
       rUartEnD0<=iUartEn;
       rUartEnD1<=rUartEnD0; 
    end
end

always @(posedge iSysClk or posedge iSysRst) begin
    if (iSysRst) begin
        rTxFlag<=1'b0;
        rTxData<=8'd0;
    end 
    else if(wEnFlag) begin
        rTxFlag<=1'b1;
        rTxData<=iUartDin;
    end 
    else if((rTxCnt==4'd9)&&(rClkCnt==BPS_CNT-(BPS_CNT/16)))begin
        rTxFlag<=1'b0;
        rTxData<=8'd0;
    end
    else begin
        rTxFlag<=rTxFlag;
        rTxData<=rTxData;
    end
end

always @(posedge iSysClk or posedge iSysRst) begin
    if (iSysRst) begin
        rClkCnt<=16'd0;
    end
    else if(rTxFlag) begin
        if(rClkCnt < BPS_CNT - 1)
            rClkCnt<=rClkCnt+1'b1;
        else
            rClkCnt <= 16'd0;
    end
    else
        rClkCnt <= 16'd0;
end

always @(posedge iSysClk or posedge iSysRst) begin
    if (iSysRst) begin
        rTxCnt<=4'd0;
    end
    else if(rTxFlag) begin
        if(rClkCnt == BPS_CNT - 1)
            rTxCnt<=rTxCnt+1'b1;
        else
            rTxCnt <= rTxCnt;
    end
    else
        rTxCnt<=4'd0;
end

always @(posedge iSysClk or posedge iSysRst) begin
    if(iSysRst)
        oUartTxd<=1'b1;
    else if (rTxFlag) begin
        case (rTxCnt)
            4'd0: oUartTxd <= 1'b0; 
            4'd1: oUartTxd <= rTxData[0]; 
            4'd2: oUartTxd <= rTxData[1]; 
            4'd3: oUartTxd <= rTxData[2]; 
            4'd4: oUartTxd <= rTxData[3]; 
            4'd5: oUartTxd <= rTxData[4]; 
            4'd6: oUartTxd <= rTxData[5]; 
            4'd7: oUartTxd <= rTxData[6]; 
            4'd8: oUartTxd <= rTxData[7]; 
            4'd9: oUartTxd <= 1'b1; 
            default: ;
        endcase
    end
    else
        oUartTxd<=1'b1;
end

endmodule