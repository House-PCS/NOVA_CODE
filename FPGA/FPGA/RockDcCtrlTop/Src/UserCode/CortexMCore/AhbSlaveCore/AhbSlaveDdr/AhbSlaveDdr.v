//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AhbSlaveDdr.v
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
module AhbSlaveDdr
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
    //DDR Write Port
    output                          oDdrWrOpClk                             ,
    output                          oDdrWrOpBurstBegin                      ,
    output      [27:0]              oDdrWrOpAddr                            ,
    output      [127:0]             oDdrWrOpData                            ,
    output      [15:0]              oDdrWrOpDataMask                        ,
    input                           iDdrWrOpDone                            ,
    //DDR Read Port
    output                          oDdrRdOpClk                             ,
    output                          oDdrRdOpBurstBegin                      ,
    output      [27:0]              oDdrRdOpAddr                            ,
    input                           iDdrLogicClk                            ,
    input       [127:0]             iDdrRdOpData                            ,
    input                           iDdrRdOpDataVld                          
);




//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    reg                             rAhbSel             = 1'd0              ;
    reg                             rAhbWrite           = 1'd0              ;
    reg         [31:0]              rAhbAddr            = 32'd0             ;
    reg         [1:0]               rAhbTrans           = 2'd0              ;
   // reg         [2:0]               rAhbSize            = 3'd0              ;

    reg         [31:0]              rAhbRdData          = 32'd0             ;
    
    reg         [127:0]             rDdrWrOpData        = 128'd0            ;
    reg         [127:0]             rDdrWrOpDataBuf     = 128'd0            ;
    reg         [15:0]              rDdrWrOpDataMask    = 16'hffff          ;
    reg         [15:0]              rDdrWrOpDataMaskBuf = 16'h0000          ; 
    reg         [27:0]              rDdrWrOpAddr        = 28'd0             ;
    reg         [27:0]              rDdrWrOpAddrBuf     = 28'd0             ;
    reg                             rDdrWrOpEn          = 1'b0              ;
    reg         [1:0]               rDdrWrOpEnBuf       = 2'd0              ;
    wire                            wDdrWrOpEnPose                          ;
    reg                             rAhbReadyDdrWrOp    = 1'b1              ;
    reg         [4:0]               rDdrWrOpDone        = 5'd0              ;

    reg         [27:0]              rDdrRdOpAddr        = 28'd0             ;
    reg                             rDdrRdOpEn          = 1'd0              ;
    reg         [1:0]               rDdrRdOpEnBuf       = 2'd0              ;
    wire                            wDdrRdOpEnPose                          ;
    reg                             rAhbReadyDdrRdOp    = 1'd1              ;   //默认可以读取,保证ahb一开始可以操作
    reg         [127:0]             rDdrRdOpData        = 128'd0            ;
    reg         [4:0]               rDdrRdOpDataVld     = 5'd0              ;


    wire                            wDdrWrOpDone                            ;
    wire                            wDdrRdOpDataVld                         ;
    wire                            wAhbWrite                               ;   
    wire                            wAhbRead                                ; 
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

//always @ (posedge iSysClk or negedge iSysRstN) begin
//    if(!iSysRstN)
//        rAhbSize[2:0] <= 3'd0;
//    else if(iAhbReady)
//        rAhbSize[2:0] <= iAhbSize[2:0];
//    else
//        rAhbSize[2:0] <= rAhbSize[2:0];
//end

assign  wAhbWrite = rAhbSel && rAhbWrite && rAhbTrans[1];   
assign  wAhbRead  = rAhbSel && (!rAhbWrite) && rAhbTrans[1];

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(wAhbWrite && (rAhbAddr[3:0] == 4'h0))
        rDdrWrOpData[0*32+:32] <= iAhbWrData[31:0];
    else
        rDdrWrOpData[0*32+:32] <= rDdrWrOpData[0*32+:32];
end

always @ (posedge iSysClk) begin
    if(wAhbWrite && (rAhbAddr[3:0] == 4'h4))
        rDdrWrOpData[1*32+:32] <= iAhbWrData[31:0];
    else
        rDdrWrOpData[1*32+:32] <= rDdrWrOpData[1*32+:32];
end

always @ (posedge iSysClk) begin
    if(wAhbWrite && (rAhbAddr[3:0] == 4'h8))
        rDdrWrOpData[2*32+:32] <= iAhbWrData[31:0];
    else
        rDdrWrOpData[2*32+:32] <= rDdrWrOpData[2*32+:32];
end

always @ (posedge iSysClk) begin
    if(wAhbWrite && (rAhbAddr[3:0] == 4'hC))
        rDdrWrOpData[3*32+:32] <= iAhbWrData[31:0];
    else
        rDdrWrOpData[3*32+:32] <= rDdrWrOpData[3*32+:32];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(rDdrWrOpEn)
        rDdrWrOpDataBuf[127:0] <= rDdrWrOpData[127:0];
    else
        rDdrWrOpDataBuf[127:0] <= rDdrWrOpDataBuf[127:0];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(wAhbWrite && (rAhbAddr[3:0] == 4'h0))
        rDdrWrOpDataMask[0*4+:4] <= 4'h0;
    else if(wDdrWrOpEnPose)
        rDdrWrOpDataMask[0*4+:4] <= 4'hf;
    else
        rDdrWrOpDataMask[0*4+:4] <= rDdrWrOpDataMask[0*4+:4];
end

always @ (posedge iSysClk) begin
    if(wAhbWrite && (rAhbAddr[3:0] == 4'h4))
        rDdrWrOpDataMask[1*4+:4] <= 4'h0;
    else if(wDdrWrOpEnPose)
        rDdrWrOpDataMask[1*4+:4] <= 4'hf;
    else
        rDdrWrOpDataMask[1*4+:4] <= rDdrWrOpDataMask[1*4+:4];
end

always @ (posedge iSysClk) begin
    if(wAhbWrite && (rAhbAddr[3:0] == 4'h8))
        rDdrWrOpDataMask[2*4+:4] <= 4'h0;
    else if(wDdrWrOpEnPose)
        rDdrWrOpDataMask[2*4+:4] <= 4'hf;
    else
        rDdrWrOpDataMask[2*4+:4] <= rDdrWrOpDataMask[2*4+:4];
end

always @ (posedge iSysClk) begin
    if(wAhbWrite && (rAhbAddr[3:0] == 4'hC))
        rDdrWrOpDataMask[3*4+:4] <= 4'h0;
    else if(wDdrWrOpEnPose)
        rDdrWrOpDataMask[3*4+:4] <= 4'hf;
    else
        rDdrWrOpDataMask[3*4+:4] <= rDdrWrOpDataMask[3*4+:4];
end

//******************************************************************************

//******************************************************************************
//
always @ (posedge iSysClk) begin
    if(rDdrWrOpEn)
        rDdrWrOpDataMaskBuf[15:0] <= rDdrWrOpDataMask[15:0];
    else
        rDdrWrOpDataMaskBuf[15:0] <= rDdrWrOpDataMaskBuf[15:0];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(wAhbWrite && (rAhbAddr[3:0] == 4'hC))
        rDdrWrOpAddr[27:0] <= {1'd0, rAhbAddr[27:1]};
    else
        rDdrWrOpAddr[27:0] <= rDdrWrOpAddr[27:0];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(rDdrWrOpEn)
        rDdrWrOpAddrBuf[27:0] <= rDdrWrOpAddr[27:0];
    else
        rDdrWrOpAddrBuf[27:0] <= rDdrWrOpAddrBuf[27:0];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(wAhbWrite && (rAhbAddr[3:0] == 4'hC))
        rDdrWrOpEn <= 1'b1;
    else
        rDdrWrOpEn <= 1'b0;
end

always @ (posedge iSysClk) begin
    rDdrWrOpEnBuf[1:0] <= {rDdrWrOpEnBuf[0], rDdrWrOpEn};
end

assign wDdrWrOpEnPose = (rDdrWrOpEnBuf[1:0] == 2'b01);

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk ) begin
    if(wDdrWrOpDone)
        rAhbReadyDdrWrOp <= 1'b1;
    else if(wAhbWrite && (rAhbAddr[3:0] == 4'hC))
        rAhbReadyDdrWrOp <= 1'b0;
    else
        rAhbReadyDdrWrOp <= rAhbReadyDdrWrOp;
end

//******************************************************************************

//******************************************************************************
//

assign oDdrWrOpClk                  = iSysClk                               ;
assign oDdrWrOpBurstBegin           = wDdrWrOpEnPose                        ;
assign oDdrWrOpAddr[27:0]           = rDdrWrOpAddrBuf[27:0]                 ;
assign oDdrWrOpData[127:0]          = rDdrWrOpDataBuf[127:0]                ;
assign oDdrWrOpDataMask[15:0]       = rDdrWrOpDataMaskBuf[15:0]             ;

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(wAhbRead)
        rDdrRdOpAddr[27:0] <= {1'd0, rAhbAddr[27:1]};
    else
        rDdrRdOpAddr[27:0] <= rDdrRdOpAddr[27:0];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    //if(wAhbRead && (rAhbAddr[27:4] != rDdrRdOpAddr[26:3]))
    if(wAhbRead)
        rDdrRdOpEn <= 1'b1;
    else
        rDdrRdOpEn <= 1'b0;
end

always @ (posedge iSysClk) begin
    rDdrRdOpEnBuf[1:0] <= {rDdrRdOpEnBuf[0], rDdrRdOpEn};
end

assign wDdrRdOpEnPose = (rDdrRdOpEnBuf[1:0] == 2'b01);

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk ) begin
    if(wDdrRdOpDataVld)
        rAhbReadyDdrRdOp <= 1'b1;
    else if(iAhbSel && (!iAhbWrite) && iAhbTrans[1])
        rAhbReadyDdrRdOp <= 1'b0;
    else
        rAhbReadyDdrRdOp <= rAhbReadyDdrRdOp;
end

//******************************************************************************

//******************************************************************************
//

assign oDdrRdOpClk                  = iSysClk                               ;
assign oDdrRdOpBurstBegin           = wDdrRdOpEnPose                        ;
assign oDdrRdOpAddr[27:0]           = rDdrRdOpAddr[27:0]                    ;

//******************************************************************************

//******************************************************************************
//跨时钟处理

always @ (posedge iDdrLogicClk) begin
    rDdrWrOpDone[4:0] <= {rDdrWrOpDone[3:0],iDdrWrOpDone};
end

assign wDdrWrOpDone = |rDdrWrOpDone[4:0];
//******************************************************************************

//******************************************************************************
//

always @ (posedge iDdrLogicClk) begin
    if(iDdrRdOpDataVld)
        rDdrRdOpData[127:0] <= iDdrRdOpData[127:0];
    else
        rDdrRdOpData[127:0] <= rDdrRdOpData[127:0];
end

always @ (posedge iDdrLogicClk) begin
    rDdrRdOpDataVld[4:0] <= {rDdrRdOpDataVld[3:0],iDdrRdOpDataVld};//跨时钟处理
end

assign wDdrRdOpDataVld = |rDdrRdOpDataVld[4:0];

//******************************************************************************

//******************************************************************************
//
reg [127:0] rDdrRdOpDataBuf;
always @ (posedge iSysClk) begin
    if(wDdrRdOpDataVld)
        rDdrRdOpDataBuf[127:0] <= rDdrRdOpData[127:0];
    else
        rDdrRdOpDataBuf[127:0] <= rDdrRdOpDataBuf[127:0];
end

//******************************************************************************



//******************************************************************************
//

always @ (*) begin
    case(rAhbAddr[3:0])
        4'h0    :   rAhbRdData[31:0] = rDdrRdOpDataBuf[0*32+:32];
        4'h4    :   rAhbRdData[31:0] = rDdrRdOpDataBuf[1*32+:32];
        4'h8    :   rAhbRdData[31:0] = rDdrRdOpDataBuf[2*32+:32];
        4'hC    :   rAhbRdData[31:0] = rDdrRdOpDataBuf[3*32+:32];
        default :   rAhbRdData[31:0] = 32'd0;
    endcase
end

//******************************************************************************

//******************************************************************************
//

assign oAhbReady        = rAhbReadyDdrRdOp && rAhbReadyDdrWrOp              ;
assign oAhbResp         = 1'b0                                              ;
assign oAhbRdData[31:0] = rAhbRdData[31:0]                                  ;


// ila_1 your_instance_name_1 (
// 	.clk(iDdrLogicClk), // input wire clk

// 	.probe0     ({wDdrWrOpDone,wDdrRdOpDataVld,rAhbReadyDdrRdOp,rAhbReadyDdrWrOp,iAhbReady,oAhbReady,iAhbWrite,iAhbTrans[1:0],iAhbSize[2:0],iDdrRdOpDataVld,iDdrWrOpDone,wAhbWrite,wAhbRead,iAhbSel,wDdrRdOpDataVld} ), 
// 	.probe1     (iAhbAddr[31:0]             ),
//     .probe2     (iAhbWrData[31:0]           ),
//     .probe3     (oAhbRdData[31:0]           )  
// );



//******************************************************************************

endmodule
