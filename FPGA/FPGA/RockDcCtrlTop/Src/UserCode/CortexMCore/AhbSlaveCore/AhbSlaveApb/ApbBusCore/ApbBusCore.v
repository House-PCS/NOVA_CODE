//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   ApbBusCore.v
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
module ApbBusCore
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
    input       [3:0]               iAhbProt                                ,
    output                          oAhbReady                               ,
    output                          oAhbResp                                ,
    output      [31:0]              oAhbRdData                              ,
    //Apb

    output      [15:0]              oApbAddr                                ,
    output                          oApbEnable                              ,
    output                          oApbWrite                               ,
    output      [3:0]               oApbStrb                                ,
    output      [2:0]               oApbProt                                ,
    output      [31:0]              oApbWrData                              ,
    output                          oApbActive                              ,
    output      [15:0]              oApbSelMap                              ,
    input       [(16*32 - 1):0]     iApbRdDataSlave                         ,
    input       [15:0]              iApbReadySlave                          ,
    input       [15:0]              iApbSlaveError                           

);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    wire        [15:0]              wApbAddr                                ;
    wire                            wApbSel                                 ;
    wire        [15:0]              wApbSelMap                              ;
    wire        [31:0]              wApbRdData                              ;
    wire                            wApbReady                               ;
    wire                            wApbSlaveError                          ;

//******************************************************************************

//******************************************************************************
//

AhbToApb U_AhbToApb 
(
    .HCLK                           (iSysClk                                ),
    .HRESETn                        (iSysRstN                               ),
    .PCLKEN                         (1'b1                                   ),
    .HSEL                           (iAhbSel                                ),
    .HADDR                          (iAhbAddr[15:0]                         ),
    .HTRANS                         (iAhbTrans[1:0]                         ),
    .HSIZE                          (iAhbSize[2:0]                          ),
    .HPROT                          (iAhbProt[3:0]                          ),
    .HWRITE                         (iAhbWrite                              ),
    .HREADY                         (iAhbReady                              ),
    .HWDATA                         (iAhbWrData[31:0]                       ),
    .HREADYOUT                      (oAhbReady                              ),
    .HRDATA                         (oAhbRdData[31:0]                       ),
    .HRESP                          (oAhbResp                               ),

    .PADDR                          (wApbAddr[15:0]                         ),  //o
    .PENABLE                        (oApbEnable                             ),  //o
    .PWRITE                         (oApbWrite                              ),  //o
    .PSTRB                          (oApbStrb[3:0]                          ),  //o
    .PPROT                          (oApbProt[2:0]                          ),  //o
    .PWDATA                         (oApbWrData[31:0]                       ),  //o
    .PSEL                           (wApbSel                                ),  //o
    .APBACTIVE                      (oApbActive                             ),  //o
    
    .PRDATA                         (wApbRdData[31:0]                       ),  //i
    .PREADY                         (wApbReady                              ),  //i
    .PSLVERR                        (wApbSlaveError                         )   //i
);

//******************************************************************************

//******************************************************************************
//

assign oApbAddr[15:0] = wApbAddr[15:0];

//******************************************************************************

//******************************************************************************
//

ApbBusDecoder U_ApbBusDecoder 
(
    .iApbAddr                       (wApbAddr[15:0]                         ), 
    .oApbSelMap                     (wApbSelMap[15:0]                       )
);

//******************************************************************************

//******************************************************************************
//

ApbSlaveMux U_ApbSlaveMux
(
    .iApbSel                        (wApbSel                                ),
    .iApbSelMap                     (wApbSelMap[15:0]                       ),
    .oApbSelMap                     (oApbSelMap[15:0]                       ),
    .iApbRdDataSlave                (iApbRdDataSlave[(16*32 - 1):0]         ),
    .iApbReadySlave                 (iApbReadySlave[15:0]                   ),
    .iApbSlaveError                 (iApbSlaveError[15:0]                   ),
    .oApbRdData                     (wApbRdData[31:0]                       ),
    .oApbReady                      (wApbReady                              ),
    .oApbSlaveError                 (wApbSlaveError                         )
);

//******************************************************************************

endmodule
