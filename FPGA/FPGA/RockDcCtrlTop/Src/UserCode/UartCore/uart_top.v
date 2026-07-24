module uart_top (
    input sys_clk,
    input sys_rst_n,

    output clk_test,

    input   iUartRxd,
    output  oUartTxd,

    input   iVP,
    input   iVN,

    input   iVauxP,
    input   iVauxN
);

reg [7:0] data;

reg rEnSend;

wire out_clk;

reg [15:0] rAdcData;

reg [31:0] sin0=32'd0;
reg [31:0] sin1=32'd0;
reg [31:0] cos0=32'd1<<16;
reg [31:0] cos1=32'd1<<16;

assign clk_test=sys_clk;

period_clk u0_flow_led(
    .sys_rst_n  (sys_rst_n),
    .clk_input  (sys_clk),
    .clk_out    (out_clk),
    .period     (32'd10000000)
);

period_clk u0_flow_calc(
    .sys_rst_n  (sys_rst_n),
    .clk_input  (sys_clk),
    .clk_out    (calc_clk),
    .period     (32'd1000)
);

always @(posedge calc_clk) begin
    // sin0 <= sin1 + ((32'd10289*cos1)>>16);
    // cos0 <= cos1 - ((32'd10289*sin1)>>16);
    // sin1 <= sin0;
    // cos1 <= cos0;
    // if(sin0>32'd65536)
    sin0[30:16] <= 15'h12;
    sin0[31:31] <= 1'b1;
    sin0[15:0] <= 16'd7864;
end

uart_send u0_uart_send(
    .iClk       (sys_clk),
    .iRstN      (sys_rst_n),

    .iUartEn    (rEnSend),
    .iUartDin   (data),
    .oUartTxd   (oUartTxd)
);
reg [4:0] rSendAddr;
always @(posedge out_clk) begin
    // data[7:0] <= rAdcData[7:0]|8'hA0;
    case (rSendAddr)
        // 5'd0: data <= 8'hAA;
        // 5'd1: data <= 8'hBB;
        // 5'd2: data <= 8'h06;
        // 5'd3: data <= 8'h00;
        // 5'd4: data <= sin0[7:0];
        // 5'd5: data <= sin0[15:8];
        // 5'd6: data <= sin0[23:16];
        // 5'd7: data <= sin0[31:24];
        // 5'd8: data <= (sin0[31:24])+(sin0[23:16])+(sin0[15:8])+(sin0[7:0]);
        5'd0: data <= 8'h00;
        5'd1: data <= 8'h01;
        5'd2: data <= 8'h02;
        5'd3: data <= 8'h03;
        5'd4: data <= 8'h04;
        5'd5: data <= 8'h05;
        5'd6: data <= 8'h06;
        5'd7: data <= 8'h07;
        5'd8: data <= 8'h08;
        default: ;
    endcase
    rEnSend <= ~rEnSend;
end

always @(negedge out_clk) begin
    // data[7:0] <= rAdcData[7:0]|8'hA0;
    if(rSendAddr>=5'd8)
        rSendAddr <= 5'd0;
    else
        rSendAddr <= rSendAddr + 5'd1;
end

wire [6:0] wDrpAddr;
wire [4:0] wAdcChannel;
wire wDrpEn;
wire wAdcConvertDone;
wire [15:0] wDrpDataOut;
wire wDrpDataOutRdy;
wire wAdcSampleDone;

assign wDrpAddr[6:0] = {2'd0,wAdcChannel[4:0]};
assign wDrpEn=wAdcConvertDone;

ila_0 u0_ila (
	.clk        (sys_clk), // input wire clk

	.probe0     ({wAdcChannel[4:0],wDrpDataOut[15:0],sys_rst_n}) // input wire [31:0] probe0
);

xadc_wiz_0 u0_xadc(
    .reset_in       (sys_rst_n),
    .dclk_in        (sys_clk),
    .di_in          (16'd0),
    .dwe_in         (1'd0),
    .daddr_in       (wDrpAddr[6:0]),
    .den_in         (wDrpEn),
    .do_out         (wDrpDataOut[15:0]),
    .drdy_out       (wDrpDataOutRdy),

    .channel_out    (wAdcChannel[4:0]),
    .eoc_out        (wAdcConvertDone),

    // .vp_in          (iVP),
    // .vn_in          (iVN),
    // .vauxp0         (iVauxP),
    // .vauxn0         (iVauxN),
    .eos_out        (wAdcSampleDone)
);

always @(posedge sys_clk) begin
    if((wDrpDataOutRdy==1'b1)&&(wAdcChannel==5'd0))
        rAdcData <= wDrpDataOut;
    else
        rAdcData <= rAdcData;
end
    
endmodule