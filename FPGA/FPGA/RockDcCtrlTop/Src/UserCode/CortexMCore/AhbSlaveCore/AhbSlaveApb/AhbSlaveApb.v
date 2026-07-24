//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AhbSlaveApb.v
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
module AhbSlaveApb
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
    //ApbSlave
    output                          oArmTimerIrq                            ,
    input                           iArmUartRx                              ,
    output                          oArmUartTx                              , 
    output                          oArmUartIrq                             ,
    output                          oSpiClk                                 ,
    output                          oSpiCs                                  ,
    output                          oSpiMosi                                ,
    input                           iSpiMiso                                 
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    wire        [15:0]              wApbAddr                                ;
    wire                            wApbEnable                              ;
    wire                            wApbWrite                               ;
    wire        [3:0]               wApbStrb                                ;
    wire        [2:0]               wApbProt                                ;
    wire        [31:0]              wApbWrData                              ;
    wire        [15:0]              wApbSelMap                              ;
    wire        [(16*32 - 1):0]     wApbRdDataSlave                         ;
    wire        [15:0]              wApbReadySlave                          ;
    wire        [15:0]              wApbSlaveError                          ;

//******************************************************************************

//******************************************************************************
//

ApbBusCore U_ApbBusCore
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (iSysRstN                               ),
    .iAhbSel                        (iAhbSel                                ),
    .iAhbWrite                      (iAhbWrite                              ),
    .iAhbAddr                       (iAhbAddr[31:0]                         ),
    .iAhbReady                      (iAhbReady                              ),
    .iAhbWrData                     (iAhbWrData[31:0]                       ),
    .iAhbTrans                      (iAhbTrans[1:0]                         ),
    .iAhbSize                       (iAhbSize[2:0]                          ),
    .iAhbProt                       (iAhbProt[3:0]                          ),
    .oAhbReady                      (oAhbReady                              ),
    .oAhbResp                       (oAhbResp                               ),
    .oAhbRdData                     (oAhbRdData[31:0]                       ),

    .oApbAddr                       (wApbAddr[15:0]                         ),
    .oApbEnable                     (wApbEnable                             ),
    .oApbWrite                      (wApbWrite                              ),
    .oApbStrb                       (wApbStrb[3:0]                          ),
    .oApbProt                       (wApbProt[2:0]                          ),
    .oApbWrData                     (wApbWrData[31:0]                       ),
    .oApbActive                     (wApbActive                             ),
    .oApbSelMap                     (wApbSelMap[15:0]                       ),
    .iApbRdDataSlave                (wApbRdDataSlave[(16*32 - 1):0]         ),
    .iApbReadySlave                 (wApbReadySlave[15:0]                   ),
    .iApbSlaveError                 (wApbSlaveError[15:0]                   ) 
);

//******************************************************************************

//******************************************************************************
//

ApbSlaveCore U_ApbSlaveCore
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (iSysRstN                               ),
    .iApbSelMap                     (wApbSelMap[15:0]                       ),
    .iApbAddr                       (wApbAddr[15:0]                         ),
    .iApbEnable                     (wApbEnable                             ),
    .iApbWrite                      (wApbWrite                              ),
    .iApbWrData                     (wApbWrData[31:0]                       ),
    .oApbRdDataSlave                (wApbRdDataSlave[(16*32 - 1):0]         ),
    .oApbReadySlave                 (wApbReadySlave[15:0]                   ),
    .oApbSlaveError                 (wApbSlaveError[15:0]                   ),
    .oArmTimerIrq                   (oArmTimerIrq                           ),
    .iArmUartRx                     (iArmUartRx                             ),
    .oArmUartTx                     (oArmUartTx                             ),
    .oArmUartIrq                    (oArmUartIrq                            ),
    .oSpiClk                        (oSpiClk                                ),
    .oSpiCs                         (oSpiCs                                 ),
    .oSpiMosi                       (oSpiMosi                               ),
    .iSpiMiso                       (iSpiMiso                               )
);

//******************************************************************************

endmodule
