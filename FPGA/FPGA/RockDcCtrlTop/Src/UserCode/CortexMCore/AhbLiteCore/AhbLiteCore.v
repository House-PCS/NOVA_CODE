//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AhbLiteCore.v
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
module AhbLiteCore
(
    //Global Clock
    input                           iSysClk                                 ,
    input                           iSysRstN                                ,

    input                           iAhbReady                               ,
    input       [31:0]              iAhbAddr                                ,
    output      [15:0]              oAhbSelMap                              ,
    input       [(16*32 - 1):0]     iAhbRdDataSlave                         ,
    input       [15:0]              iAhbReadySlave                          ,
    input       [15:0]              iAhbRespSlave                           ,
    output                          oAhbReady                               ,
    output                          oAhbResp                                ,
    output      [31:0]              oAhbRdData                   
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    wire        [15:0]              wAhbSelMap                              ;

//******************************************************************************

//******************************************************************************
//

AhbLiteDecoder U_AhbLiteDecoder 
(
    .iAhbAddr                       (iAhbAddr[31:0]                         ), 
    .oAhbSelMap                     (wAhbSelMap[15:0]                       )
);

assign oAhbSelMap[15:0] = wAhbSelMap[15:0];

//******************************************************************************

//******************************************************************************
//

AhbLiteMux U_AhbLiteMux
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (iSysRstN                               ),
    .iAhbReady                      (iAhbReady                              ),
    .iAhbSelMap                     (wAhbSelMap[15:0]                       ),
    .iAhbRdDataSlave                (iAhbRdDataSlave[(16*32 - 1):0]         ),
    .iAhbReadySlave                 (iAhbReadySlave[15:0]                   ),
    .iAhbRespSlave                  (iAhbRespSlave[15:0]                    ),
    .oAhbRdData                     (oAhbRdData                             ),
    .oAhbReady                      (oAhbReady                              ),
    .oAhbResp                       (oAhbResp                               )
);


//******************************************************************************

endmodule
