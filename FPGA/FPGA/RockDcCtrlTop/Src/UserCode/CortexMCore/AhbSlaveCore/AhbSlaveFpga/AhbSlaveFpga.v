//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AhbSlaveFpga.v
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
module AhbSlaveFpga
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
    //CmdBus
   // input       [(2*32 - 1):0]      iFpgaParam                              ,
    input                           iParamBusClk                            ,
    input       [31:0]              iParamBusAddr                           ,
    input                           iParamBusWrEn                           ,
    input       [15:0]              iParamBusWrData                         ,
    output      [15:0]              oParamBusRdData                          
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    wire        [31:0]              wAhbCacheAddr                           ;
    wire        [3:0]               wAhbCacheWrEn                           ;
    wire        [31:0]              wAhbCacheWrData                         ;
    wire                            wAhbCacheCs                             ;
    wire        [31:0]              wAhbCacheRdData                         ;

//******************************************************************************

//******************************************************************************
//

AhbToFpgaCache U_AhbToFpgaCache
(
    .HCLK                           (iSysClk                                ),
    .HRESETn                        (iSysRstN                               ),
    .HSEL                           (iAhbSel                                ),
    .HREADY                         (iAhbReady                              ),
    .HTRANS                         (iAhbTrans[1:0]                         ),
    .HSIZE                          (iAhbSize[2:0]                          ),
    .HWRITE                         (iAhbWrite                              ),
    .HADDR                          (iAhbAddr[31:0]                         ),
    .HWDATA                         (iAhbWrData[31:0]                       ),
    .HREADYOUT                      (oAhbReady                              ),
    .HRESP                          (oAhbResp                               ),
    .HRDATA                         (oAhbRdData[31:0]                       ),
    
    .iCacheRdData                   (wAhbCacheRdData[31:0]                  ),
    .oCacheAddr                     (wAhbCacheAddr[29:0]                    ),
    .oCacheWrEn                     (wAhbCacheWrEn[3:0]                     ),
    .oCacheWrData                   (wAhbCacheWrData[31:0]                  ),
    .oCacheCs                       (wAhbCacheCs                            )
);

//******************************************************************************

//******************************************************************************
//

FpgaParamCache U_FpgaParamCache
(
    .iSysClk                        (iSysClk                                ),
    .iAhbCacheCs                    (wAhbCacheCs                            ),
    .iAhbCacheAddr                  (wAhbCacheAddr[29:0]                    ),
    .iAhbCacheWrEn                  (wAhbCacheWrEn[3:0]                     ),
    .iAhbCacheWrData                (wAhbCacheWrData[31:0]                  ),
    .oAhbCacheRdData                (wAhbCacheRdData[31:0]                  ),
   // .iFpgaParam                     (iFpgaParam[(2*32 - 1):0]               ),
    .iParamBusClk                   (iParamBusClk                           ),
    .iParamBusAddr                  (iParamBusAddr[31:0]                    ),
    .iParamBusWrEn                  (iParamBusWrEn                          ),
    .iParamBusWrData                (iParamBusWrData[15:0]                  ),
    .oParamBusRdData                (oParamBusRdData[15:0]                  )
);


//******************************************************************************
endmodule
