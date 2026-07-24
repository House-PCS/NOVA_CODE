//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   FpgaParamCache.v
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
module FpgaParamCache
(
    input                           iSysClk                                 ,
    input                           iAhbCacheCs                             ,
    input       [29:0]              iAhbCacheAddr                           ,
    input       [3:0]               iAhbCacheWrEn                           ,
    input       [31:0]              iAhbCacheWrData                         ,
    output      [31:0]              oAhbCacheRdData                         ,

    //input       [(2*32 - 1):0]      iFpgaParam                              ,

    input                           iParamBusClk                            ,
    input       [31:0]              iParamBusAddr                           ,
    input                           iParamBusWrEn                           ,
    input       [15:0]              iParamBusWrData                         ,
    output      [15:0]              oParamBusRdData                          

);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    wire                            wAhbCacheWrEn                           ;
    wire                            wParamBusWrEn                           ;
    wire        [31:0]              wAhbCacheRdData                         ;
    //reg         [(2*32 - 1):0]      rFpgaParamBuf       [1:0]               ;
    reg         [8:0]               rAhbCacheAddrBuf    = 9'd0              ;
    reg         [31:0]              rAhbCacheRdData     = 32'd0             ;

//******************************************************************************

//******************************************************************************
//

assign wAhbCacheWrEn = ((iAhbCacheWrEn[3:0] == 4'b1111) && iAhbCacheCs);
assign wParamBusWrEn = (iParamBusWrEn && (iParamBusAddr[31:16] == 16'h3002));

//******************************************************************************

//******************************************************************************
//

//always @ (posedge iSysClk) begin
//    rFpgaParamBuf[0][(2*32 - 1):0] <= iFpgaParam[(2*32 - 1):0];
//    rFpgaParamBuf[1][(2*32 - 1):0] <= rFpgaParamBuf[0][(2*32 - 1):0];
//end

//******************************************************************************

//******************************************************************************
//
/*
always @ (posedge iSysClk) begin
    rAhbCacheAddrBuf[8:0] <= iAhbCacheAddr[8:0];
end

always @ (*) begin
    case(rAhbCacheAddrBuf[8:0])
        9'h000  :   rAhbCacheRdData[31:0] = rFpgaParamBuf[1][0*32+:32];
        9'h001  :   rAhbCacheRdData[31:0] = rFpgaParamBuf[1][1*32+:32];
        default :   rAhbCacheRdData[31:0] = wAhbCacheRdData[31:0];
    endcase
end
*/
//******************************************************************************

//******************************************************************************
//

assign oAhbCacheRdData[31:0] = wAhbCacheRdData[31:0];

//******************************************************************************

//******************************************************************************
//
DPRam_512x32_Dly1_1024x16_Dly2  U_FpgaParaRam
(
    .clka                           (iSysClk                                ),
    .wea                            (wAhbCacheWrEn                          ),
    .addra                          (iAhbCacheAddr[8:0]                     ),
    .dina                           (iAhbCacheWrData[31:0]                  ),
    .douta                          (wAhbCacheRdData[31:0]                  ),

    .clkb                           (iParamBusClk                           ),
    .web                            (wParamBusWrEn                          ),
    .addrb                          (iParamBusAddr[9:0]                     ),
    .dinb                           (iParamBusWrData[15:0]                  ),
    .doutb                          (oParamBusRdData[15:0]                  )

);

//******************************************************************************

endmodule
