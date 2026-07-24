//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   UdpDataCache.v
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
module UdpDataCache
(
    input                           iSysClk                                 ,
    input                           iAhbCacheCs                             ,
    input       [29:0]              iAhbCacheAddr                           ,
    input       [3:0]               iAhbCacheWrEn                           ,
    input       [31:0]              iAhbCacheWrData                         ,
    output      [31:0]              oAhbCacheRdData                         ,

    input                           iEthClk                                 ,
    input                           iEthRxDataVld                           ,
    input       [7:0]               iEthRxData                              ,
    input                           iEthTxDataRdEn                          ,
    output      [7:0]               oEthTxData                              ,
    input                           iEthRxIrqClr                            ,
    output                          oEthRxIrq                                
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    reg         [10:0]              rEthRxWrAddr        = 11'd0             ;    
    wire                            wAhbCacheWrEn                           ;    
    reg         [10:0]              rEthTxRdAddr        = 11'd0             ;
    reg         [2:0]               rEthRxDataVldBuf    = 3'd0              ;
    wire                            wEthRxDataVldNege                       ;
    reg                             rEthRxIrq           = 1'd0              ;

//******************************************************************************

//******************************************************************************
//

always @ (posedge iEthClk) begin
    if(iEthRxDataVld)
        rEthRxWrAddr[10:0] <= rEthRxWrAddr[10:0] + 10'd1;
    else
        rEthRxWrAddr[10:0] <= 11'd0;
end

//******************************************************************************

//******************************************************************************
//Addr 0x3000_0000 --> 0x3000_07ff  2048KB  RxCache

SDPRam_2048x8_512x32 U_EthRxDataCache
(
    .clka                           (iEthClk                                ),
    .wea                            (iEthRxDataVld                          ),
    .addra                          (rEthRxWrAddr[10:0]                     ),
    .dina                           (iEthRxData[7:0]                        ),
    
    .clkb                           (iSysClk                                ),
    .addrb                          (iAhbCacheAddr[8:0]                     ),
    .doutb                          (oAhbCacheRdData[31:0]                  )
);

//******************************************************************************

//******************************************************************************
//

assign wAhbCacheWrEn = ((iAhbCacheWrEn[3:0] == 4'b1111) && iAhbCacheCs && (iAhbCacheAddr[13:10] == 4'h1));

always @ (posedge iEthClk) begin
    if(iEthTxDataRdEn)
        rEthTxRdAddr[10:0] <= rEthTxRdAddr[10:0] + 11'd1;
    else
        rEthTxRdAddr[10:0] <= 11'd0;
end

//******************************************************************************

//******************************************************************************
//Addr 0x3000_1000 --> 0x3000_07ff  2048KB  TxCache

SDPRam_512x32_2048x8 U_EthTxDataCache
(
    .clka                           (iSysClk                                ),
    .wea                            (wAhbCacheWrEn                          ),
    .addra                          (iAhbCacheAddr[8:0]                     ),
    .dina                           (iAhbCacheWrData[31:0]                  ),
    
    .clkb                           (iEthClk                                ),
    .addrb                          (rEthTxRdAddr[10:0]                     ),
    .doutb                          (oEthTxData[7:0]                        )
);

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    rEthRxDataVldBuf[2:0] <= {rEthRxDataVldBuf[1:0], iEthRxDataVld};
end

assign wEthRxDataVldNege = {rEthRxDataVldBuf[2:1] == 2'b10};

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(iEthRxIrqClr)
        rEthRxIrq <= 1'b0;
    else if(wEthRxDataVldNege)
        rEthRxIrq <= 1'b1;
    else
        rEthRxIrq <= rEthRxIrq;
end

//******************************************************************************

//******************************************************************************
//

assign oEthRxIrq = rEthRxIrq                ;

//******************************************************************************

endmodule
