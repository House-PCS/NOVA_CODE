//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   EthCmdAnalysisCore.v
//Original Author   :   lvyb
//Creation Date     :   2024.09.23
/*Description       :   Gmii接口解析后转Ram接口

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
module EthCmdAnalysisCore
(
    //Global Clock
    input                           iSysClk                     ,
    
    input                           iEthRxDataEn                ,               //UDP协议接收到的数据使能
    input       [7:0]               iEthRxData                  ,               //UDP协议接收到的数据
    input                           iEthTxDataRdEn              ,               //UDP协议申请读取缓存使能
    output      [7:0]               oEthTxData                  ,               //UDP协议发送数据模块读取缓存的数据，用来发送UDP数据

    output      [31:0]              oEthRamBusOpAddr            ,               //以太网Ram总线操作地址，读写都采用这个地址
    output                          oEthRamBusWrEn              ,               //以太网Ram总线写使能，1为写使能，0为读使能
    output      [7:0]               oEthRamBusWrData            ,               //以太网Ram总线写数据,写入到内部协议地址空间

    input       [7:0]               iEthRamBusRdData            ,               //以太网Ram总线读取内部寄存器到缓存中的数据
    output                          oEthRamBusRdDone            ,               //以太网Ram总线读取内部寄存器到缓存结束标志
    output      [15:0]              oEthRamBusRdDataLen                         //以太网Ram总线读取内部寄存器到缓存中的长度
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    reg     [3:0]                   rEthRxDataEnCnt     = 4'd0      ;
    reg                             rEthRamBusCmdWr     = 1'd0      ;
    reg                             rEthRamBusCmdRd     = 1'd0      ;
    reg     [31:0]                  rEthRamBusAddr      = 32'd0     ;
    reg     [15:0]                  rEthRamBusLen       = 16'd0     ;
    reg                             rEthRamBusWrEn      = 1'd0      ;
    reg     [15:0]                  rEthRamBusWrCnt     = 16'd0     ;
    reg                             rEthRamBusRdEn      = 1'd0      ;
    reg     [15:0]                  rEthRamBusRdCnt     = 16'd0     ;
    reg     [31:0]                  rEthRamBusOpAddr    = 32'd0     ;
    reg     [2:0]                   rEthRamBusRdEnBuf   = 3'd0      ;
    wire                            wEthRamBusRdEnNege              ;
    reg     [10:0]                  rRamWrAddr          = 11'd0     ;    
    wire                            wRamWrDataEn                    ;
    wire    [10:0]                  wRamWrAddr                      ;
    wire    [7:0]                   wRamWrData                      ;    
    reg     [10:0]                  rRamRdAddr          = 11'd0     ;    
    wire    [10:0]                  wRamRdAddr                      ;
    wire    [7:0]                   wRamRdData                      ;

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(iEthRxDataEn) begin
        if(rEthRxDataEnCnt[3:0] == 4'd7)
            rEthRxDataEnCnt[3:0] <= rEthRxDataEnCnt[3:0];
        else
            rEthRxDataEnCnt[3:0] <= rEthRxDataEnCnt[3:0] + 4'd1;
    end
    else
        rEthRxDataEnCnt[3:0] <= 4'd0;
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(iEthRxDataEn && (rEthRxDataEnCnt[3:0] == 4'd0))
        rEthRamBusCmdWr <= (iEthRxData[7:0] == 8'h45);
    else
        rEthRamBusCmdWr <= rEthRamBusCmdWr;
end


always @ (posedge iSysClk) begin
    if(iEthRxDataEn && (rEthRxDataEnCnt[3:0] == 4'd0))
        rEthRamBusCmdRd <= (iEthRxData[7:0] == 8'h43);
    else
        rEthRamBusCmdRd <= rEthRamBusCmdRd;
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(iEthRxDataEn) begin
        case(rEthRxDataEnCnt[3:0])
            4'd1    :   rEthRamBusAddr[7:0]     <= iEthRxData[7:0];
            4'd2    :   rEthRamBusAddr[15:8]    <= iEthRxData[7:0];
            4'd3    :   rEthRamBusAddr[23:16]   <= iEthRxData[7:0];
            4'd4    :   rEthRamBusAddr[31:24]   <= iEthRxData[7:0];
            default :   rEthRamBusAddr[31:0]    <= rEthRamBusAddr[31:0];
        endcase
    end
    else
        rEthRamBusAddr[31:0] <= 32'd0;                                          //此处需要确保地址参数默认为0，否则会导致通信异常
end

always @ (posedge iSysClk) begin
    if(iEthRxDataEn) begin
        case(rEthRxDataEnCnt[3:0])
            4'd5    :   rEthRamBusLen[7:0]      <= iEthRxData[7:0];
            4'd6    :   rEthRamBusLen[15:8]     <= iEthRxData[7:0];
            default :   rEthRamBusLen[15:0]     <= rEthRamBusLen[15:0];
        endcase            
    end
    else
        rEthRamBusLen[15:0] <= rEthRamBusLen[15:0];
end

//******************************************************************************

//******************************************************************************
//

assign oEthRamBusRdDataLen[15:0]    = rEthRamBusLen[15:0];

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(iEthRxDataEn && (rEthRxDataEnCnt[3:0] == 4'd6))
        rEthRamBusWrEn <= rEthRamBusCmdWr;
    else if((rEthRamBusWrCnt[15:0] + 16'd1) == rEthRamBusLen[15:0])
        rEthRamBusWrEn <= 1'd0;
    else
        rEthRamBusWrEn <= rEthRamBusWrEn;
end

always @ (posedge iSysClk) begin
    if(rEthRamBusWrEn)
        rEthRamBusWrCnt[15:0] <= rEthRamBusWrCnt[15:0] + 16'd1;
    else
        rEthRamBusWrCnt[15:0] <= 16'd0;
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(iEthRxDataEn && (rEthRxDataEnCnt[3:0] == 4'd6))
        rEthRamBusRdEn <= rEthRamBusCmdRd;
    else if((rEthRamBusRdCnt[15:0] + 16'd1) == rEthRamBusLen[15:0])
        rEthRamBusRdEn <= 1'd0;
    else
        rEthRamBusRdEn <= rEthRamBusRdEn;
end

always @ (posedge iSysClk) begin
    if(rEthRamBusRdEn)
        rEthRamBusRdCnt[15:0] <= rEthRamBusRdCnt[15:0] + 16'd1;
    else
        rEthRamBusRdCnt[15:0] <= 16'd0;
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(iEthRxDataEn && (rEthRxDataEnCnt[3:0] == 4'd6))
        rEthRamBusOpAddr[31:0] <= rEthRamBusAddr[31:0];
    else if(rEthRamBusWrEn || rEthRamBusRdEn)
        rEthRamBusOpAddr[31:0] <= rEthRamBusOpAddr[31:0] + 32'd1;
    else
        rEthRamBusOpAddr[31:0] <= rEthRamBusOpAddr[31:0];
end

//******************************************************************************

//******************************************************************************
//

assign oEthRamBusOpAddr[31:0]       = rEthRamBusOpAddr[31:0]        ;
assign oEthRamBusWrEn               = rEthRamBusWrEn                ;
assign oEthRamBusWrData[7:0]        = iEthRxData[7:0]               ;

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    rEthRamBusRdEnBuf[2:0] <= {rEthRamBusRdEnBuf[1:0], rEthRamBusRdEn};
end

//******************************************************************************

//******************************************************************************
//

assign wEthRamBusRdEnNege           = (rEthRamBusRdEnBuf[2:1] == 2'b10) ;
assign oEthRamBusRdDone             = wEthRamBusRdEnNege                ;

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(rEthRamBusRdEnBuf[2])
        rRamWrAddr[10:0] <= rRamWrAddr[10:0] + 11'd1;
    else
        rRamWrAddr[10:0] <= 11'd0;
end

//******************************************************************************

//******************************************************************************
//

assign wRamWrDataEn                 = rEthRamBusRdEnBuf[2]          ;
assign wRamWrAddr[10:0]             = rRamWrAddr[10:0]              ;
assign wRamWrData[7:0]              = iEthRamBusRdData[7:0]         ;

//******************************************************************************

//******************************************************************************
//

SDPRam_8x2048_8x2048 U_EthRdDataCache
(
    .clka                           (iSysClk                    ),
    .wea                            (wRamWrDataEn               ),
    .addra                          (wRamWrAddr[10:0]           ),
    .dina                           (wRamWrData[7:0]            ),
    
    .clkb                           (iSysClk                    ),
    .addrb                          (wRamRdAddr[10:0]           ),
    .doutb                          (wRamRdData[7:0]            )
);

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(iEthTxDataRdEn)
        rRamRdAddr[10:0] <= rRamRdAddr[10:0] + 11'd1;
    else
        rRamRdAddr[10:0] <= 11'd0;
end

//******************************************************************************

//******************************************************************************
//

assign wRamRdAddr[10:0]             = rRamRdAddr[10:0]  ;
assign oEthTxData[7:0]              = wRamRdData[7:0]   ;

//******************************************************************************

endmodule
