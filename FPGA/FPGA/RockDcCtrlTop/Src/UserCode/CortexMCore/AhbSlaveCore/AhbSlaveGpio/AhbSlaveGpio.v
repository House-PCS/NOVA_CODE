//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AhbSlaveGpio.v
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
module AhbSlaveGpio
(
    //Global Clock
    input                           iSysClk                                 ,
    input                           iSysRstN                                ,
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
    output      [31:0]              oAhbRdData                              ,
    //Output
    output                          oArmLed                                 ,
    output      [15:0]              oEthTxLen                               ,
    output                          oEthTxDataReady                         ,
    output                          oEthRxIrqClr                            ,
    output                          oEhtPhyRstN                             ,
    output                          oMCURebootEn                            ,//软重启使能：Active_High
    output                          oEthPhyMdclk                            ,
    inout                           ioEthPhyMdio                              
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    reg                             rAhbSel             = 1'd0              ;
    reg                             rAhbWrite           = 1'd0              ;
    reg         [31:0]              rAhbAddr            = 32'd0             ;
    reg         [1:0]               rAhbTrans           = 2'd0              ;
    reg         [2:0]               rAhbSize            = 3'd0              ;
    reg         [31:0]              rAhbRdData          = 32'd0             ;

    reg                             rArmLed             = 1'd0              ;
    reg         [31:0]              rEthTxLen           = 32'd0             ;
    reg                             rEthTxDataReady     = 1'd0              ;
    reg                             rEthRxIrqClr        = 1'd0              ;
    reg                             rEhtPhyRstN         = 1'd0              ;
    reg                             rEthPhyMdclk        = 1'd0              ;
    reg                             rEthPhyMdio         = 1'd0              ;
    reg                             rMCURebootEn        = 1'd0              ;
    // reg                             rEhtPhyMdioCtrl     = 1'd0              ;
    wire                            wEthPhyMdio                             ;


//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk or negedge iSysRstN) begin
    if(!iSysRstN)
        rAhbSel <= 1'b0;
    else if(iAhbReady)
        rAhbSel <= iAhbSel;
    else
        rAhbSel <= rAhbSel;
end

always @ (posedge iSysClk or negedge iSysRstN) begin
    if(!iSysRstN)
        rAhbWrite <= 1'b0;
    else if(iAhbReady)
        rAhbWrite <= iAhbWrite;
    else
        rAhbWrite <= rAhbWrite;
end

always @ (posedge iSysClk or negedge iSysRstN) begin
    if(!iSysRstN)
        rAhbAddr[31:0] <= 32'd0;
    else if(iAhbReady)
        rAhbAddr[31:0] <= iAhbAddr[31:0];
    else
        rAhbAddr[31:0] <= rAhbAddr[31:0];
end

always @ (posedge iSysClk or negedge iSysRstN) begin
    if(!iSysRstN)
        rAhbTrans[1:0] <= 2'd0;
    else if(iAhbReady)
        rAhbTrans[1:0] <= iAhbTrans[1:0];
    else
        rAhbTrans[1:0] <= rAhbTrans[1:0];
end

always @ (posedge iSysClk or negedge iSysRstN) begin
    if(!iSysRstN)
        rAhbSize[2:0] <= 3'd0;
    else if(iAhbReady)
        rAhbSize[2:0] <= iAhbSize[2:0];
    else
        rAhbSize[2:0] <= rAhbSize[2:0];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(rAhbSel && rAhbWrite && rAhbTrans[1] && (rAhbAddr[15:0] == 16'h0000))
        rArmLed <= iAhbWrData[0];
    else
        rArmLed <= rArmLed;
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(rAhbSel && rAhbWrite && rAhbTrans[1] && (rAhbAddr[15:0] == 16'h0010))
        rEthTxLen[31:0] <= iAhbWrData[31:0];
    else
        rEthTxLen[31:0] <= rEthTxLen[31:0];
end

always @ (posedge iSysClk) begin
    if(rAhbSel && rAhbWrite && rAhbTrans[1] && (rAhbAddr[15:0] == 16'h0014))
        rEthTxDataReady <= (iAhbWrData[31:0] == 32'd1);
    else
        rEthTxDataReady <= 1'd0;
end

always @ (posedge iSysClk) begin
    if(rAhbSel && rAhbWrite && rAhbTrans[1] && (rAhbAddr[15:0] == 16'h0018))
        rEthRxIrqClr <= (iAhbWrData[31:0] == 32'd1);
    else
        rEthRxIrqClr <= 1'd0;
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(rAhbSel && rAhbWrite && rAhbTrans[1] && (rAhbAddr[15:0] == 16'h0020))
        rEhtPhyRstN <= iAhbWrData[0];
    else
        rEhtPhyRstN <= rEhtPhyRstN;
end

always @ (posedge iSysClk) begin
    if(rAhbSel && rAhbWrite && rAhbTrans[1] && (rAhbAddr[15:0] == 16'h0024))
        rEthPhyMdclk <= iAhbWrData[0];
    else
        rEthPhyMdclk <= rEthPhyMdclk;
end

always @ (posedge iSysClk) begin
    if(rAhbSel && rAhbWrite && rAhbTrans[1] && (rAhbAddr[15:0] == 16'h0028))
        rEthPhyMdio <= iAhbWrData[0];
    else
        rEthPhyMdio <= rEthPhyMdio;
end

//******************************************************************************
//************************软重启功能 Start***************************************
always @ (posedge iSysClk) begin
    if(rAhbSel && rAhbWrite && rAhbTrans[1] && (rAhbAddr[15:0] == 16'h0030))
        rMCURebootEn <= iAhbWrData[0];
    else
        rMCURebootEn <= rMCURebootEn;
end

assign oMCURebootEn = rMCURebootEn;
//*************************软重启功能 End********************************************
always @ (*) begin
    case(rAhbAddr[15:0])
        16'h0000    :   rAhbRdData[31:0] = {31'd0, rArmLed};
        16'h0010    :   rAhbRdData[31:0] = rEthTxLen[31:0];
        16'h0014    :   rAhbRdData[31:0] = {31'd0, rEthTxDataReady};
        16'h0018    :   rAhbRdData[31:0] = {31'd0, 1'b1};
        16'h0020    :   rAhbRdData[31:0] = {31'd0, rEhtPhyRstN};
        16'h0024    :   rAhbRdData[31:0] = {31'd0, rEthPhyMdclk};
        16'h0028    :   rAhbRdData[31:0] = {31'd0, wEthPhyMdio};
        default     :   rAhbRdData[31:0] = 32'd0;
    endcase
end

//******************************************************************************

//******************************************************************************
//

assign oAhbReady = 1'b1;
assign oAhbResp = 1'b0;
assign oAhbRdData[31:0] = rAhbRdData[31:0];

//******************************************************************************

//******************************************************************************
//

assign oArmLed = rArmLed;
assign oEthTxLen[15:0] = rEthTxLen[15:0];
assign oEthTxDataReady = rEthTxDataReady;
assign oEthRxIrqClr = rEthRxIrqClr;

assign oEhtPhyRstN = rEhtPhyRstN;
assign oEthPhyMdclk = rEthPhyMdclk;

assign ioEthPhyMdio = rEthPhyMdio ? 1'bz : 1'b0;
assign wEthPhyMdio = ioEthPhyMdio;

//******************************************************************************

endmodule
