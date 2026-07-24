`timescale 1ns / 1ps
module DelayTimeCore#(
    parameter  CLK_FRQ = 100
)
(
    input                               iSysClk    ,
    output                              oDelay1Us  ,
    output                              oDelay1Ms  ,
    output                              oDelay1S                           
);
function integer clogb2 ( input integer depth ) ;
begin
    for ( clogb2 = 0 ; depth > 0 ; clogb2 = clogb2 + 1 ) begin 
        depth = depth >> 1 ;
    end
end
endfunction
//------------------------------------------------------------------
localparam COUNT_1US = CLK_FRQ;
localparam COUNT_1MS = 1000;
localparam COUNT_1S  = 1000;
localparam CNT_BW = clogb2(COUNT_1US);
reg     [CNT_BW-1:0]    rCntUs ='d0;
reg                     rDelay1Us ='d0;;
reg     [9:0]           rCntMs ='d0;;
reg                     rDelay1Ms ='d0;;
reg     [9:0]           rCntS ='d0;;
reg                     rDelay1S ='d0;
wire                    wDelay1UsPre;
wire                    wDelay1MsPre;
always @(posedge iSysClk) begin
    if(rCntUs == (COUNT_1US - 'd1))
        rCntUs <= 'd0;
    else
        rCntUs <= rCntUs + 'd1;
end
    
always @(posedge iSysClk) begin
    if(wDelay1UsPre)
        rDelay1Us <= 1'b1;
    else
        rDelay1Us <= 1'b0;        
end

assign wDelay1UsPre =  rCntUs == (COUNT_1US - 'd2);
assign oDelay1Us = rDelay1Us;  

always @(posedge iSysClk) begin
    if(rDelay1Us)begin
        if(rCntMs == (COUNT_1MS - 'd1))
            rCntMs <= 'd0;
        else
            rCntMs <= rCntMs + 'd1;
    end
    else begin
        rCntMs <= rCntMs;
    end
end

always @(posedge iSysClk) begin
    if(rCntMs == (COUNT_1MS - 'd1) && wDelay1UsPre)
        rDelay1Ms <= 1'b1;
    else
        rDelay1Ms <= 1'b0;        
end

assign wDelay1MsPre = rCntMs == (COUNT_1MS - 'd1) && wDelay1UsPre;
assign oDelay1Ms = rDelay1Ms;

always @(posedge iSysClk) begin
    if(rDelay1Ms)begin
        if(rCntS == (COUNT_1S - 'd1))
            rCntS <= 'd0;
        else
            rCntS <= rCntS + 'd1;
    end
    else begin
        rCntS <= rCntS;
    end
end

always @(posedge iSysClk) begin
    if(rCntS == (COUNT_1S - 'd1) && wDelay1MsPre)
        rDelay1S <= 1'b1;
    else
        rDelay1S <= 1'b0;        
end

assign oDelay1S = rDelay1S;
                                                            
endmodule