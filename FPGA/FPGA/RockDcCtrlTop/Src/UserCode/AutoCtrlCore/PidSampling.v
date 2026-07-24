//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   PidSampling.v
//Original Author   :   lvyb
//Creation Date     :   
/*Description       :

*/
//------------------------------------------------------------------------------
//Version           :   Rev 0.1
//Modifier          :   
//Modify Date       :   
/*Modify Record     :   

*/
//------------------------------------------------------------------------------
//Version           :   Rev 0.2
//Modifier          :   
//Modify Date       :   
/*Modify Record     :   

*/  
//------------------------------------------------------------------------------

`timescale 1ns/1ns
module PidSampling
#(
    parameter                       CLK_FREQ            = 100                        
)
(
    input                           iSysClk                                 ,
    input                           iAdcSampleSequenceDone                  ,
    output      [1:0]               oPidOpEn
);

//-------------------------------Code Start Here--------------------------------

reg [2:0]   rAdcSampleSequenceDoneBuf   = 3'd0  ;   
reg         rTriger                     = 1'b0  ;
reg [5:0]   rPidSlowCnt                 = 6'd0  ;

reg         rPidFastOpEn                = 1'b0  ;
wire        wPidFastOpEn                        ;

reg  [2:0]  rPidSlowOpEn                = 3'd0  ;
wire        wPidSlowOpEn                        ;


//******************************************************************************
//
always @(posedge iSysClk ) begin
    rAdcSampleSequenceDoneBuf[2:0] <= {rAdcSampleSequenceDoneBuf[1:0],iAdcSampleSequenceDone};
end

always @(posedge iSysClk ) begin
    if( rAdcSampleSequenceDoneBuf[2:1] == 2'b01)
        rTriger <= 1'b1;
    else
        rTriger <= 1'b0;
end

 always @(posedge iSysClk ) begin
    if(rTriger)
        rPidFastOpEn <= 1'b1;
    else
        rPidFastOpEn <= 1'b0;
 end

assign wPidFastOpEn = rPidFastOpEn ;
//******************************************************************************
always @(posedge iSysClk ) begin
    if(rTriger)begin
        if(rPidSlowCnt == 6'd10)
            rPidSlowCnt <= 6'd0;
        else
            rPidSlowCnt <= rPidSlowCnt + 1; 
    end
    else begin
        rPidSlowCnt <= rPidSlowCnt;
    end
end

always @(posedge iSysClk ) begin
    rPidSlowOpEn[2:0] <= {rPidSlowOpEn[1:0],(rPidSlowCnt == 6'd10)};
end

assign wPidSlowOpEn = rPidSlowOpEn[2:1] == 2'b01;
//******************************************************************************
//

assign oPidOpEn[1:0] = {wPidFastOpEn, wPidSlowOpEn};

//******************************************************************************

endmodule
