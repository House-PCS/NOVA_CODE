//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   GeneralParamCore.v
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
module GeneralParamCore
(
    //Global Clock
    input                           iEthRamBusClk               ,
    input       [31:0]              iEthRamBusOpAddr            ,
    input                           iEthRamBusWrEn              ,
    input       [7:0]               iEthRamBusWrData            ,
    output      [7:0]               oEthRamBusRdData            ,

    input                           iSysClk                     ,
    output                          oBoardSelf                  ,
    output                          oForwardEn                  ,
    output      [3:0]               oLogicPriority              ,
    output                          oPwmRstN                    ,
    output                          oPwmEn                      ,
    output      [9:0]               oPwmDuty                    ,
    output      [2:0]               oDrvOutEn                   ,
    output                          oSysStable                   
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    localparam  PARAM_GEN_ADDR      =   32'h0000_0000           ;

//******************************************************************************

//******************************************************************************
//
    wire                            wEthBusOpRamEn              ;
    wire    [11:0]                  wEthBusOpRamAddr            ;
    wire                            wEthBusWrDataEn             ;
    wire    [7:0]                   wEthBusWrData               ;
    wire    [7:0]                   wEthRamBusRdData            ;
    reg     [11:0]                  rRamAddr            = 12'd0 ;
    reg     [11:0]                  rRamAddrBuf0        = 12'd0 ;
    reg     [11:0]                  rRamAddrBuf1        = 12'd0 ;
    wire    [7:0]                   wRamData                    ;
    reg                             rBoardSelf          = 1'd0  ;    
    reg                             rForwardEn          = 1'd0  ;
    reg     [3:0]                   rLogicPriority      = 4'd0  ;
    reg                             rPwmRstN            = 1'd0  ;
    reg                             rPwmPwmEn           = 1'd0  ;
    reg     [9:0]                   rPwmDuty            = 10'd0 ;
    reg     [2:0]                   rDrvOutEn           = 3'd0  ;
    reg                             wSysStable          = 1'd0  ;

//******************************************************************************

//******************************************************************************
//

assign wEthBusOpRamEn               = (iEthRamBusOpAddr[31:12] == PARAM_GEN_ADDR[31:12]);
assign wEthBusOpRamAddr[11:0]       = iEthRamBusOpAddr[11:0];
assign wEthBusWrDataEn              = iEthRamBusWrEn && wEthBusOpRamEn;
assign wEthBusWrData[7:0]           = iEthRamBusWrData[7:0];
assign oEthRamBusRdData[7:0]        = wEthRamBusRdData[7:0];

//******************************************************************************

//******************************************************************************
//

DPRam_8x2048_8x2048 U_GeneralParamRam
(
    .clka                           (iEthRamBusClk              ), 
    .wea                            (wEthBusWrDataEn            ), 
    .addra                          (wEthBusOpRamAddr[11:0]     ), 
    .dina                           (wEthBusWrData[7:0]         ), 
    .douta                          (wEthRamBusRdData[7:0]      ), 

    .clkb                           (iSysClk                    ),
    .web                            (1'b0                       ),
    .addrb                          (rRamAddr[11:0]             ),
    .dinb                           (8'h00                      ),
    .doutb                          (wRamData[7:0]              )
);

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    rRamAddr[11:0] <= rRamAddr[11:0] + 12'd1;
end

always @ (posedge iSysClk) begin
    rRamAddrBuf0[11:0] <= rRamAddr[11:0]      ;
    rRamAddrBuf1[11:0] <= rRamAddrBuf0[11:0]  ;
end

//******************************************************************************

//******************************************************************************
//32'h0200_0001

always @ (posedge iSysClk) begin
    if(rRamAddrBuf1[11:0] == 12'h001)
        rBoardSelf <= (wRamData[7:0] == 8'h00);
    else
        rBoardSelf <= rBoardSelf;
end

//******************************************************************************

//******************************************************************************
//32'h0200_0002

always @ (posedge iSysClk) begin
    if(rRamAddrBuf1[11:0] == 12'h002)
        rForwardEn <= (wRamData[7:0] == 8'h01);
    else
        rForwardEn <= rForwardEn;
end

//******************************************************************************

//******************************************************************************
//32'h0200_0003

always @ (posedge iSysClk) begin
    if(rRamAddrBuf1[11:0] == 12'h003)
        rLogicPriority[3:0] <= wRamData[3:0];
    else
        rLogicPriority[3:0] <= rLogicPriority[3:0];
end

//******************************************************************************

//******************************************************************************
//

assign oBoardSelf = rBoardSelf;
assign oForwardEn = rForwardEn;
assign oLogicPriority[3:0] = rLogicPriority[3:0];

//******************************************************************************

//******************************************************************************
//32'h0200_000a

always @ (posedge iSysClk) begin
    if(rRamAddrBuf1[11:0] == 12'h00a)
        rPwmRstN <= (wRamData[7:0] == 8'h00);
    else
        rPwmRstN <= rPwmRstN;
end

//******************************************************************************

//******************************************************************************
//32'h0200_000b

always @ (posedge iSysClk) begin
    if(rRamAddrBuf1[11:0] == 12'h00b)
        rPwmPwmEn <= (wRamData[7:0] == 8'h01);
    else
        rPwmPwmEn <= rPwmPwmEn;
end

//******************************************************************************

//******************************************************************************
//32'h0200_000c - 32'h0200_000d

always @ (posedge iSysClk) begin
    if(rRamAddrBuf1[11:0] == 12'h00c)
        rPwmDuty[7:0] <= wRamData[7:0];
    else
        rPwmDuty[7:0] <= rPwmDuty[7:0];
end

always @ (posedge iSysClk) begin
    if(rRamAddrBuf1[11:0] == 12'h00d)
        rPwmDuty[9:8] <= wRamData[1:0];
    else
        rPwmDuty[9:8] <= rPwmDuty[9:8];
end

//******************************************************************************

//******************************************************************************
//

assign oPwmRstN                     = rPwmRstN              ;
assign oPwmEn                       = rPwmPwmEn             ;
assign oPwmDuty[9:0]                = rPwmDuty[9:0]         ;

//******************************************************************************

//******************************************************************************
//32'h0200_000e

always @ (posedge iSysClk) begin
    if(rRamAddrBuf1[11:0] == 12'h00e)
        rDrvOutEn[2:0] <= wRamData[2:0];
    else
        rDrvOutEn[2:0] <= rDrvOutEn[2:0];
end

//******************************************************************************

//******************************************************************************
//

assign oDrvOutEn[2:0]               = rDrvOutEn[2:0]        ;

//******************************************************************************

//******************************************************************************
//32'h0200_000f

always @ (posedge iSysClk) begin
    if(rRamAddrBuf1[11:0] == 12'h00f)
        wSysStable <= (wRamData[7:0] == 8'h01);
    else
        wSysStable <= wSysStable;
end

//******************************************************************************

//******************************************************************************
//

assign oSysStable                   = wSysStable;

//******************************************************************************


endmodule
