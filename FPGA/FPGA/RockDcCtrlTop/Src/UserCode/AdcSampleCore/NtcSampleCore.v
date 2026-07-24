module NtcSampleCore(
    input                     iSysClk                ,
    input                     iAdcSampleSequenceDone ,
    input             [11:0]  iAdcTemp1              ,   // 12位ADC输入
    input             [11:0]  iAdcTemp2              ,   // 12位ADC输入
    output    signed  [ 7:0]  oTemp1                 ,   // 8位有符号温度输出
    output    signed  [ 7:0]  oTemp2                    // 8位有符号温度输出
);
    reg                                 rAdcTempSel              ='d0;
    reg                  [   2: 0]      rAdcTempSelDly           ='d0;
    reg                  [  10: 0]      rRdRamAddr               ='d0;
    wire                 [   7: 0]      wNtcTemp                   ;
    reg     signed       [   7: 0]      rTemp1                   ='sd0;
    reg     signed       [   7: 0]      rTemp2                   ='sd0;
always@(posedge iSysClk)begin
    if(iAdcSampleSequenceDone)
        rAdcTempSel <= ~rAdcTempSel;
    else
        rAdcTempSel <= rAdcTempSel;       
end

always@(posedge iSysClk)begin
    rAdcTempSelDly[2:0] <= {rAdcTempSelDly[1:0],rAdcTempSel};
end
always@(posedge iSysClk)begin
    if(rAdcTempSel)
        rRdRamAddr[10:0] <= iAdcTemp1[11:1];
    else
        rRdRamAddr[10:0] <= iAdcTemp2[11:1];        
end

Srom_2048x8_D2 NtcTemp(
     .clka    (iSysClk          ),
     .addra   (rRdRamAddr[10:0] ),
     .douta   (wNtcTemp[7:0]    )
);

always@(posedge iSysClk)begin
    if(rAdcTempSelDly[2] == 1'b1)
        rTemp1 <= wNtcTemp;
    else
        rTemp1 <= rTemp1;        
end

always@(posedge iSysClk)begin
    if(rAdcTempSelDly[2] == 1'b0)
        rTemp2 <= wNtcTemp;
    else
        rTemp2 <= rTemp2;        
end


assign oTemp1 = rTemp1;
assign oTemp2 = rTemp2;
endmodule