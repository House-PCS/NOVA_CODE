//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AhbSlaveSram.v
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
module AhbSlaveSram
#(
    parameter                       A_WIDTH             = 12                    //单个4KB，总共16KB
)
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
    output      [31:0]              oAhbRdData                               
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    wire        [31:0]              wAhbSramAddr                            ;
    wire        [3:0]               wAhbSramWrEn                            ;
    wire        [31:0]              wAhbSramWrData                          ;
    wire                            wAhbSramCs                              ;
    wire        [31:0]              wAhbSramRdData                          ;

//******************************************************************************

//******************************************************************************
//

AhbToSram U_AhbToSram
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
    
    .SRAMRDATA                      (wAhbSramRdData[31:0]                   ),
    .SRAMADDR                       (wAhbSramAddr[29:0]                     ),
    .SRAMWEN                        (wAhbSramWrEn[3:0]                      ),
    .SRAMWDATA                      (wAhbSramWrData[31:0]                   ),
    .SRAMCS                         (wAhbSramCs                             )
);

//******************************************************************************

//******************************************************************************
//

SramMem 
#(
    .A_WIDTH                        (A_WIDTH                                )
)U_SramMem
(
    .iSysClk                        (iSysClk                                ),
    .iFpgaSramCs                    (wAhbSramCs                             ),
    .iFpgaSramWrEn                  (wAhbSramWrEn[3:0]                      ),
    .iFpgaSramAddr                  (wAhbSramAddr[29:0]                     ),
    .iFpgaSramWrData                (wAhbSramWrData[31:0]                   ),
    .oFpgaSramRdData                (wAhbSramRdData[31:0]                   )
);


//******************************************************************************

endmodule
