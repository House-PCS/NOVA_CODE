//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   Division10.v
//Original Author   :   xucy   Nova003107 
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

`timescale 1ns/1ps

module Division100_16bit #(
    parameter       CLK_FREQ    = 100
)(
    input                       iSysClk         ,
    input                       iSysRst         ,
    input                       iCnt1Us         ,
    input signed [15:0]  iDataIn0        ,
    input signed [15:0]  iDataIn1        ,
    input signed [15:0]  iDataIn2        ,
    input signed [15:0]  iDataIn3        ,
    input signed [15:0]  iDataIn4        ,
    input signed [15:0]  iDataIn5        ,
    input signed [15:0]  iDataIn6        ,
    input signed [15:0]  iDataIn7        ,
    
    

    output signed [15:0]  oDataOut0       ,
    output signed [15:0]  oDataOut1       ,
    output signed [15:0]  oDataOut2       ,
    output signed [15:0]  oDataOut3       ,
    output signed [15:0]  oDataOut4       ,
    output signed [15:0]  oDataOut5       ,
    output signed [15:0]  oDataOut6       ,
    output signed [15:0]  oDataOut7       

);
//-------------------------------Code Start Here--------------------------------

reg [7:0] rCalculEn = 8'd0;
wire wCalculEn;
reg [3:0] i = 4'd0;
reg [3:0] j = 4'd0;

reg [15:0] rData;
wire [15:0] wDataOut;
reg rDataInValid = 1'b0;
wire wDataOutValid;
wire wDataInValid;

reg signed [15:0]  rDataIn  [0:7];
reg signed [15:0]  rDataOut [0:7];
reg signed [15:0] rDataOut0, rDataOut1, rDataOut2, rDataOut3;
reg signed [15:0] rDataOut4, rDataOut5, rDataOut6, rDataOut7;



//******************************************************************************
// 1us更新一次数据

always @(posedge iSysClk ) begin
    if(iCnt1Us)begin
        rDataIn[0] <= iDataIn0 ;
        rDataIn[1] <= iDataIn1 ;
        rDataIn[2] <= iDataIn2 ;
        rDataIn[3] <= iDataIn3 ;
        rDataIn[4] <= iDataIn4 ;
        rDataIn[5] <= iDataIn5 ;
        rDataIn[6] <= iDataIn6 ;
        rDataIn[7] <= iDataIn7 ;
    end
end

//******************************************************************************

//******************************************************************************
//脉冲展宽
always @(posedge iSysClk ) begin
    rCalculEn[7:0] <= {rCalculEn[6:0],iCnt1Us};
end

assign wCalculEn = | rCalculEn[7:0];
//******************************************************************************
//
always @(posedge iSysClk ) begin
    if(wCalculEn)begin
        rDataInValid <= 1'b1;
        rData   <= rDataIn[i];
        i <= i + 1'b1;
    end else begin
        rDataInValid <= 1'b0;
        i <= 4'b0;
    end
end

assign wDataInValid = rDataInValid;
//******************************************************************************
always @(posedge iSysClk ) begin
    if(wDataOutValid)begin
       rDataOut[j] <= wDataOut;
       j <= j + 1'b1;
    end else begin
       j <= 4'b0;
    end
end
//******************************************************************************

//******************************************************************************
//

always @(posedge iSysClk ) begin
    if(iCnt1Us)begin
        rDataOut0 <= rDataOut[0] ;
        rDataOut1 <= rDataOut[1] ;
        rDataOut2 <= rDataOut[2] ;
        rDataOut3 <= rDataOut[3] ;
        rDataOut4 <= rDataOut[4] ;
        rDataOut5 <= rDataOut[5] ;
        rDataOut6 <= rDataOut[6] ;
        rDataOut7 <= rDataOut[7] ;
    end
end


assign oDataOut0 = rDataOut0 ;
assign oDataOut1 = rDataOut1 ;
assign oDataOut2 = rDataOut2 ;
assign oDataOut3 = rDataOut3 ;
assign oDataOut4 = rDataOut4 ;
assign oDataOut5 = rDataOut5 ;
assign oDataOut6 = rDataOut6 ;
assign oDataOut7 = rDataOut7 ;
//******************************************************************************



signed_div100_16bit U_signed_div100_16bit(
    .iSysClk        (iSysClk        ),
    .iSysRst        (iSysRst        ),
    .iData          (rData          ),
    .iData_Valid    (wDataInValid   ),
    .oData          (wDataOut       ),
    .oData_Valid    (wDataOutValid  )
);


//******************************************************************************


endmodule