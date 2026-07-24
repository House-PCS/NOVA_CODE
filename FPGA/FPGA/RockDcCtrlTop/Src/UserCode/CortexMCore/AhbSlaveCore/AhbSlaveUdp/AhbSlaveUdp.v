//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AhbSlaveUdp.v
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
module AhbSlaveUdp
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
    //UdpPort
    input                           iEthClk                                 ,
    input                           iEthRxDataVld                           ,
    input       [7:0]               iEthRxData                              ,
    input                           iEthRxIrqClr                            ,
    output                          oEthRxIrq                               ,

    input                           iEthTxDataRdEn                          ,
    output      [7:0]               oEthTxData                               
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
//Addr 0x3000_0000 --> 0x3000_07ff  2048KB  RxBuf
//Addr 0x3000_1000 --> 0x3000_07ff  2048KB  TxBuf

AhbToUdpCache U_AhbToUdpCache
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

UdpDataCache U_UdpDataCache
(
    .iSysClk                        (iSysClk                                ),
    .iAhbCacheCs                    (wAhbCacheCs                            ),
    .iAhbCacheAddr                  (wAhbCacheAddr[29:0]                    ),
    .iAhbCacheWrEn                  (wAhbCacheWrEn[3:0]                     ),
    .iAhbCacheWrData                (wAhbCacheWrData[31:0]                  ),
    .oAhbCacheRdData                (wAhbCacheRdData[31:0]                  ),
    .iEthClk                        (iEthClk                                ),
    .iEthRxDataVld                  (iEthRxDataVld                          ),
    .iEthRxData                     (iEthRxData[7:0]                        ),
    .iEthTxDataRdEn                 (iEthTxDataRdEn                         ),
    .oEthTxData                     (oEthTxData[7:0]                        ),

    .iEthRxIrqClr                   (iEthRxIrqClr                           ),
    .oEthRxIrq                      (oEthRxIrq                              )          

);

//******************************************************************************

endmodule
