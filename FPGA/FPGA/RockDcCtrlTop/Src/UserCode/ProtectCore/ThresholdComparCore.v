module ThresholdComparCore#(
    parameter                           DATA_WIDTH                  = 16                   ,
    parameter                           GLITCH_FILTER_NUM           = 1                    //The minimum value is 1.
)
(
    input                               iClk                       ,
    input  signed [DATA_WIDTH-1: 0]     iMaxThreshold              ,
    input  signed [DATA_WIDTH-1: 0]     iMinThreshold              ,
    input  signed [DATA_WIDTH-1: 0]     iValue                     ,
    input                               iMaxCompareEn              ,
    input                               iMinCompareEn              ,
    input                               iErrClear                  ,
    input                               iADCTrigger                ,

    output                              oErrMax                    ,
    output                              oErrMin                    
);
function integer clogb2 ( input integer depth ) ;
begin
    for ( clogb2 = 0 ; depth > 0 ; clogb2 = clogb2 + 1 ) begin 
        depth = depth >> 1 ;
    end
end
endfunction
    
 localparam                          NUM_BW                      = clogb2(GLITCH_FILTER_NUM)          ;

 reg                  [NUM_BW-1: 0]  rCntMaxErr                ='d0;
 reg                  [NUM_BW-1: 0]  rCntMinErr                ='d0;
 reg                                 rErrMax                   =1'b0;
 reg                                 rErrMin                   =1'b0;

always @(posedge iClk) begin
    if(iErrClear)
        rCntMaxErr <= 'd0;
    else if(iMaxCompareEn && iADCTrigger)begin
        if($signed(iValue) > iMaxThreshold)begin 
            if(rCntMaxErr == GLITCH_FILTER_NUM)
                rCntMaxErr <= rCntMaxErr;
            else
                rCntMaxErr <= rCntMaxErr + 1'b1;
        end
        else begin
            rCntMaxErr <= 0;
        end   
    end
    else begin
        rCntMaxErr <= rCntMaxErr;
    end      
end

always @(posedge iClk) begin
    if(iErrClear)
        rErrMax <= 1'b0;          
    else if( rCntMaxErr == GLITCH_FILTER_NUM )
        rErrMax <= 1'b1;       
end

always @(posedge iClk) begin
    if(iErrClear)
        rCntMinErr <= 'd0;
    else if(iMinCompareEn && iADCTrigger)begin
        if($signed(iValue) < iMinThreshold)begin 
            if(rCntMinErr == GLITCH_FILTER_NUM)
                rCntMinErr <= rCntMinErr;
            else
                rCntMinErr <= rCntMinErr + 1'b1;
        end
        else begin
            rCntMinErr <= 0;
        end   
    end
    else begin
        rCntMinErr <= rCntMinErr;
    end      
end

always @(posedge iClk) begin
    if(iErrClear)
        rErrMin <= 1'b0; 
    else if( rCntMinErr == GLITCH_FILTER_NUM )
        rErrMin <= 1'b1;  
end

assign oErrMax = rErrMax;
assign oErrMin = rErrMin;

endmodule
