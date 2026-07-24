module DebugTest
#(
    parameter  [5:0]    PARAM_COUNT = 6'd36
)
(
    input          iSysClk,
    input          iEthTxRdEn,
    input  [PARAM_COUNT * 32 - 1:0] iBuf,
    output [  7:0] oEthTxData,
    output [ 15:0] oEthTxLen,
    output         oEthTxEn 
);

    localparam [5:0] BYTE_LENG = PARAM_COUNT + 2;

    localparam LEN = 4 * BYTE_LENG;

    reg  [31:0] cnt;
    reg  [31:0] rFrameCnt;
    reg         rEthTxEn;
    reg  [ 7:0] rEthTxData = 8'hA0;
    reg         rUdpTxReq;
    reg  [ 15:0] rDataSel = 8'b0;
    reg  [ 1:0] rEthTxRdEn;
    wire [ 7:0] wCache             [BYTE_LENG*4-1:0];
    wire        wEthTxRdEnEdgn;

    assign oEthTxLen       = LEN;
    assign oEthTxEn        = (cnt==0) && (rDataSel < LEN);
    // assign oEthTxData      = rEthTxData;

    // assign oEthTxEn        = rEthTxEn;

    assign wCache[0][7:0]  = 8'hA5;
    assign wCache[1][7:0]  = 8'h5A;
    assign wCache[2][7:0]  = 8'hFE;
    assign wCache[3][7:0]  = 8'h01;
    assign wCache[BYTE_LENG*4-4][7:0] = 8'h00;
    assign wCache[BYTE_LENG*4-3][7:0] = 8'h00;
    assign wCache[BYTE_LENG*4-2][7:0] = 8'h80;
    assign wCache[BYTE_LENG*4-1][7:0] = 8'h7F;
    assign oEthTxData[7:0] = (rDataSel >= LEN) ? 8'b0 : wCache[rDataSel][7:0];
    // assign rEthTxRdEn[1:0] = {rEthTxRdEn[0], iEthTxRdEn};
    reg [PARAM_COUNT * 32 - 1:0] rBuf;

    always @(posedge iSysClk) begin
        rEthTxRdEn[1:0] <= {rEthTxRdEn[0], iEthTxRdEn};
    end

    assign wEthTxRdEnEdge  = (rEthTxRdEn[1] == 1'b0) && (rEthTxRdEn[0] == 1'b1);

    genvar i;

    generate
        for (i = 0; i < PARAM_COUNT; i = i + 1) begin
            assign wCache[(i+1)*4][7:0]   = rBuf[i*32+:8];
            assign wCache[(i+1)*4+1][7:0] = rBuf[i*32+8+:8];
            assign wCache[(i+1)*4+2][7:0] = rBuf[i*32+16+:8];
            assign wCache[(i+1)*4+3][7:0] = rBuf[i*32+24+:8];
        end
    endgenerate

    always @(posedge iSysClk) begin
        if(rFrameCnt == 32'd100000 - 1) begin
            rBuf <= iBuf;
        end else begin
            rBuf <= rBuf;
        end
    end

    always @(posedge iSysClk) begin
        if (rFrameCnt < 32'd100000 - 1) 
            rFrameCnt <= rFrameCnt + 1;
        else
            rFrameCnt <= 0;
    end

    always @(posedge iSysClk) begin
        if (cnt < 32'd550 - 1)
            cnt <= cnt + 1;
        else 
            cnt <= 0;
    end
    always @(posedge iSysClk) begin
        if (wEthTxRdEnEdge) begin
            rDataSel <= rDataSel + 16'd1;
        end else if (rFrameCnt == 0) begin
            rDataSel <= 0;
        end else begin
            rDataSel <= rDataSel;
        end 

    end

endmodule
