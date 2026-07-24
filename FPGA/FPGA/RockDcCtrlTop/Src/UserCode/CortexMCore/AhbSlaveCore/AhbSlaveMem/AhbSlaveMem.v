//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AhbSlaveMem.v
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
module AhbSlaveMem
#(
    parameter                       MEM_INIT            = "ON"              ,
    parameter                       A_WIDTH             = 10                 
)
(
    //Global Clock
    input                           iSysClk                                 ,
    //Slave Sel
    input                           iAhbSel                                 ,
    //Ahb
    input                           iAhbWrite                               ,
    input       [31:0]              iAhbAddr                                ,
    input                           iAhbReady                               ,
    input       [31:0]              iAhbWrData                              ,
    input       [1:0]               iAhbTrans                               ,
    input       [2:0]               iAhbSize                                ,
    output                          oAhbReady                               ,
    output                          oAhbResp                                ,
    output      [31:0]              oAhbRdData                               
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    reg                             rAhbSel             = 1'd0              ;
    reg                             rAhbWrite           = 1'd0              ;
    reg         [31:0]              rAhbAddr            = 32'd0             ;
    reg         [1:0]               rAhbTrans           = 2'd0              ;
    reg         [2:0]               rAhbSize            = 3'd0              ;
    // reg         [31:0]              rAhbRdData          = 32'd0             ;
reg         [31:0]              rAhbMem         [0:(2**(A_WIDTH - 2) - 1)];

//******************************************************************************

//******************************************************************************
//

// generate 
//     if(MEM_INIT == "ON") begin

initial begin
    (* RAM_STYLE = "block" *) $readmemh("code.hex", rAhbMem);
end

//     end
// endgenerate

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(iAhbReady)
        rAhbSel <= iAhbSel;
    else
        rAhbSel <= rAhbSel;
end

always @ (posedge iSysClk) begin
    if(iAhbReady)
        rAhbWrite <= iAhbWrite;
    else
        rAhbWrite <= rAhbWrite;
end

always @ (posedge iSysClk) begin
    if(iAhbReady)
        rAhbAddr[31:0] <= iAhbAddr[31:0];
    else
        rAhbAddr[31:0] <= rAhbAddr[31:0];
end

always @ (posedge iSysClk) begin
    if(iAhbReady)
        rAhbTrans[1:0] <= iAhbTrans[1:0];
    else
        rAhbTrans[1:0] <= rAhbTrans[1:0];
end


always @ (posedge iSysClk) begin
    if(iAhbReady)
        rAhbSize[2:0] <= iAhbSize[2:0];
    else
        rAhbSize[2:0] <= rAhbSize[2:0];
end

//******************************************************************************

//******************************************************************************
//

assign wTxByteMode = (rAhbSize[1:0] == 2'b00);
assign wTxHalfMode = (rAhbSize[1:0] == 2'b01);
assign wTxWordMode = (rAhbSize[1:0] == 2'b10);

assign wTxByte00 = wTxByteMode && (rAhbAddr[1:0] == 2'b00);
assign wTxByte01 = wTxByteMode && (rAhbAddr[1:0] == 2'b01);
assign wTxByte10 = wTxByteMode && (rAhbAddr[1:0] == 2'b10);
assign wTxByte11 = wTxByteMode && (rAhbAddr[1:0] == 2'b11);

assign wTxHalf00 = wTxHalfMode && (rAhbAddr[1:0] == 2'b01);
assign wTxHalf10 = wTxHalfMode && (rAhbAddr[1:0] == 2'b10);

assign wTxWord00 = wTxWordMode;

assign wDataByte0 = wTxWord00 || wTxHalf00 || wTxByte00;
assign wDataByte1 = wTxWord00 || wTxHalf00 || wTxByte01;
assign wDataByte2 = wTxWord00 || wTxHalf10 || wTxByte10;
assign wDataByte3 = wTxWord00 || wTxHalf10 || wTxByte11;


//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(rAhbSel && rAhbWrite && rAhbTrans[1]) begin
        if(wDataByte0)
            rAhbMem[rAhbAddr[A_WIDTH:2]][7:0] <= iAhbWrData[7:0];
        else if(wDataByte1)
            rAhbMem[rAhbAddr[A_WIDTH:2]][15:8] <= iAhbWrData[15:8];
        else if(wDataByte2)
            rAhbMem[rAhbAddr[A_WIDTH:2]][23:16] <= iAhbWrData[23:16];
        else if(wDataByte3)
            rAhbMem[rAhbAddr[A_WIDTH:2]][31:24] <= iAhbWrData[31:24];
    end
    else
        rAhbMem[rAhbAddr[A_WIDTH:2]][31:0] <= rAhbMem[rAhbAddr[A_WIDTH:2]][31:0];
end


//******************************************************************************

//******************************************************************************
//

// always @ (posedge iSysClk) begin
//     if(iAhbSel && (!iAhbWrite) && iAhbTrans[1])
//         rAhbRdData[31:0] <= rAhbMem[iAhbAddr[A_WIDTH:2]][31:0];
//     else
//         rAhbRdData[31:0] <= rAhbRdData[31:0];
// end

//******************************************************************************

//******************************************************************************
//

assign oAhbReady = 1'b1;
assign oAhbResp  = 1'b0;
// assign oAhbRdData[31:0] = rAhbRdData[31:0];
assign oAhbRdData[31:0] = rAhbMem[rAhbAddr[A_WIDTH:2]][31:0];

//******************************************************************************

endmodule
