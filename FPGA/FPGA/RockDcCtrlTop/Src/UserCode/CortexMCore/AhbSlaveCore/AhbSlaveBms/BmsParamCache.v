//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   BmsParamCache.v
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
module BmsParamCache
(
    input                           iSysClk                                 ,
    input                           iAhbCacheCs                             ,
    input       [29:0]              iAhbCacheAddr                           ,
    input       [3:0]               iAhbCacheWrEn                           ,
    input       [31:0]              iAhbCacheWrData                         ,
    output      [31:0]              oAhbCacheRdData                         ,

    input                           iParamBusClk                            ,
    input       [31:0]              iParamBusAddr                           ,
    input                           iParamBusWrEn                           ,
    input       [7:0]               iParamBusWrData                         ,
    output      [7:0]               oParamBusRdData                          
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    wire                            wAhbCacheWrEn                           ;
    wire                            wParamBusWrEn                           ;

//******************************************************************************

//******************************************************************************
//

assign wAhbCacheWrEn = ((iAhbCacheWrEn[3:0] == 4'b1111) && iAhbCacheCs);
assign wParamBusWrEn = (iParamBusWrEn && (iParamBusAddr[31:16] == 16'h0100));

//******************************************************************************

//******************************************************************************
//

DPRam_512x32_2048x8 U_DPRam_512x32_2048x8
(
    .clka                           (iSysClk                    ),
    .wea                            (wAhbCacheWrEn              ),
    .addra                          (iAhbCacheAddr[8:0]         ),
    .dina                           (iAhbCacheWrData[31:0]      ),
    .douta                          (oAhbCacheRdData[31:0]      ),

    .clkb                           (iParamBusClk               ),
    .web                            (wParamBusWrEn              ),
    .addrb                          (iParamBusAddr[10:0]        ),
    .dinb                           (iParamBusWrData[7:0]       ),
    .doutb                          (oParamBusRdData[7:0]       )
);

//******************************************************************************
endmodule
