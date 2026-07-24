module CurrentRamBuffer(
input                  iWrClk,
input                  iRdClk,
input                  iWrStart,
input                  iRdEn, 
input                  iAdcSampleSequenceDone,
input   signed [15:0]  iBusTotalCurrentA,
input   signed [15:0]  iBusTotalCurrentB,
output  signed [15:0]  oBusTotalCurrentA,
output  signed [15:0]  oBusTotalCurrentB   
);

reg   [12:0]   rWrAddr = 13'd0;
reg   [11:0]   rRdAddr = 12'd0;
reg   rAdcSampleSequenceDone=1'd0;
wire  [15:0]   wBusTotalCurrentA;
wire  [15:0]   wBusTotalCurrentB;
wire  WrEn;
reg signed [15:0]  rBusTotalCurrentA;
reg signed [15:0]  rBusTotalCurrentB;
always @(posedge iWrClk) begin
    rAdcSampleSequenceDone <= iAdcSampleSequenceDone;
end

assign WrEn = (!rAdcSampleSequenceDone) & iAdcSampleSequenceDone;

always @(posedge iWrClk) begin
    rBusTotalCurrentA <= iBusTotalCurrentA;
    rBusTotalCurrentB <= iBusTotalCurrentB;
end
Sdpram4096x16_4096x16_D2 BusTotalCurrentA(
    .clka  (iWrClk                    ),
    .wea   (WrEn  && (rWrAddr!=13'd4096) ),
    .addra (rWrAddr[11:0]               ),
    .dina  (rBusTotalCurrentA[15:0]    ),
    .clkb  (iRdClk                    ),
    .addrb (rRdAddr[11:0]               ),
    .doutb (wBusTotalCurrentA[15:0]    )
  );

Sdpram4096x16_4096x16_D2 BusTotalCurrentB(
    .clka  (iWrClk                    ),
    .wea   (WrEn  && (rWrAddr!=13'd4096) ),
    .addra (rWrAddr[11:0]               ),
    .dina  (rBusTotalCurrentB[15:0]    ),
    .clkb  (iRdClk                 ),
    .addrb (rRdAddr[11:0]               ),
    .doutb (wBusTotalCurrentB[15:0]        )
  );

always@(posedge iWrClk)begin
  if(iWrStart)
    rWrAddr[12:0] <= 13'd0;
  else if(WrEn)begin
    if(rWrAddr[12:0]==13'd4096)
      rWrAddr[12:0] <= rWrAddr[12:0];
    else
      rWrAddr[12:0] <= rWrAddr[12:0] + 1'b1;
  end
  else
    rWrAddr[12:0] <= rWrAddr[12:0];
end

always@(posedge iRdClk)begin
  if(iWrStart)
    rRdAddr[11:0] <= 12'd0;
  if((rWrAddr[12:0]==13'd4096) && iRdEn)begin
    if(rRdAddr[11:0]==12'd4095) 
      rRdAddr[11:0]<=rRdAddr[11:0];
    else
      rRdAddr[11:0]<=rRdAddr[11:0] + 1'b1;      
  end
end

assign oBusTotalCurrentA[15:0] = $signed(wBusTotalCurrentA[15:0]);
assign oBusTotalCurrentB[15:0] = $signed(wBusTotalCurrentB[15:0]);
endmodule