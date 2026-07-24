module signed_div100_16bit (
    input                       iSysClk     ,
    input                       iSysRst     ,
    input   signed [15:0]       iData       ,
    input                       iData_Valid ,
    output  signed [15:0]       oData       ,
    output                      oData_Valid 
);

// 1/100 = (2^-7 + 2^-9 + 2^-12);
// 1/100 = (2^-6 - 2^-7 + 2^-8 - 2^-9 + 2^-10 - 2^-11 - 2^-12 - 2^-13 + 2^-14 + 2^-15)
// 3级流水线
reg signed [15:0] rDataStage   = 16'sd0;

reg         [3:0]  rData_Valid   = 4'b0      ;
reg signed [15:0] rData         = 16'sd0    ;
reg signed [15:0] rData1        = 16'sd0    ;
reg signed [15:0] rData2        = 16'sd0    ;
reg signed [15:0] rData3        = 16'sd0    ;

reg signed [15:0] rShift6       = 16'sd0    ;
reg signed [15:0] rShift7       = 16'sd0    ;
reg signed [15:0] rShift8       = 16'sd0    ;
reg signed [15:0] rShift9       = 16'sd0    ;
reg signed [15:0] rShift10      = 16'sd0    ;
reg signed [15:0] rShift11      = 16'sd0    ;
reg signed [15:0] rShift12      = 16'sd0    ;
reg signed [15:0] rShift13      = 16'sd0    ;
reg signed [15:0] rShift14      = 16'sd0    ;
reg signed [15:0] rShift15      = 16'sd0    ;

always @(posedge iSysClk ) begin
    rData_Valid[3:0] <= {rData_Valid[2:0],iData_Valid};
end

always @(posedge iSysClk ) begin
    if(iData_Valid)
        rDataStage <= iData;
    else
        rDataStage <= rDataStage;
end

always @(posedge iSysClk ) begin//保留符号的右移动>>>
    if(rData_Valid[0])begin
        rShift6     <= rDataStage>>>6   ;
        rShift7     <= rDataStage>>>7   ;
        rShift8     <= rDataStage>>>8   ;
        rShift9     <= rDataStage>>>9   ;
        rShift10    <= rDataStage>>>10  ;
        rShift11    <= rDataStage>>>11  ;
        rShift12    <= rDataStage>>>12  ;
        rShift13    <= rDataStage>>>13  ;
        rShift14    <= rDataStage>>>14  ;
    end
end

//(2^-6 - 2^-7 + 2^-8 - 2^-9 + 2^-10 - 2^-11 - 2^-12 - 2^-13 + 2^-14 + 2^-15)

always @(posedge iSysClk ) begin
    if(rData_Valid[1])begin
        rData1 <= rShift6 - rShift7 + rShift8 ;
        rData2 <=  - rShift9 + rShift10 - rShift11 ;
        rData3 <= - rShift12 - rShift13 + rShift14 + rShift15;
    end
    else begin
        rData1 <=  rData1 ;
        rData2 <=  rData2 ;
        rData3 <=  rData3 ;
    end
end

always @(posedge iSysClk ) begin
    if(rData_Valid[2])
       rData <= rData1 + rData2 + rData3;
    else
       rData <= rData ;
end

assign oData_Valid  = rData_Valid[3];
assign oData        = rData         ;

endmodule