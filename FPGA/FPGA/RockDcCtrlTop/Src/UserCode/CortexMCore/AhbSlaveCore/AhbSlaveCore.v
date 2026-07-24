//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AhbSlaveCore.v
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
module AhbSlaveCore
#(
    parameter                       CLK_FREQ            = 50        
)
(
    //Global Clock
    input                           iSysClk                                 ,
    input                           iSysRstN                                ,
    
    input       [31:0]              iAhbAddr                                ,
    input                           iAhbWrite                               ,
    input                           iAhbReady                               ,
    input       [31:0]              iAhbWrData                              ,
    input       [1:0]               iAhbTrans                               ,
    input       [2:0]               iAhbSize                                ,
    input       [3:0]               iAhbProt                                ,
    input       [2:0]               iAhbBurst                               ,
    input       [15:0]              iAhbSelMap                              ,
    output      [15:0]              oAhbReadySlave                          ,
    output      [15:0]              oAhbRespSlave                           ,
    output      [(16*32 - 1):0]     oAhbRdDataSlave                         , 
    //User Interface
    //UdpPort
    input                           iEthClk                                 ,
    input                           iEthRxDataVld                           ,
    input       [7:0]               iEthRxData                              ,
    output                          oEthTxDataReady                         ,
    output      [15:0]              oEthTxLen                               ,
    input                           iEthTxDataRdEn                          ,
    output      [7:0]               oEthTxData                              ,
    //BmsParamBus
    input                           iBmsParamBusClk                         ,
    input       [31:0]              iBmsParamBusAddr                        ,
    output      [7:0]               oBmsParamBusRdData                      ,
    //FpgaParamBus
    //input       [(2*32 - 1):0]      iFpgaParam                              ,
    input                           iFpgaParamBusClk                        ,
    input       [31:0]              iFpgaParamBusAddr                       ,
    input                           iFpgaParamBusWrEn                       ,
    input       [15:0]              iFpgaParamBusWrData                     ,
    output      [15:0]              oFpgaParamBusRdData                     ,
    //ArmIrq
    output      [31:0]              oArmIrq                                 ,          
    //ArmLed
    output                          oArmLed                                 ,
    output                          oEhtPhyRstN                             ,
    output                          oEthPhyMdclk                            ,
    inout                           ioEthPhyMdio                            ,
    //ArmUart
    input                           iArmUartRx                              ,
    output                          oArmUartTx                              ,
    //Spi
    output                          oSpiClk                                 ,
    output                          oSpiCs                                  ,
    output                          oSpiMosi                                ,
    input                           iSpiMiso                                ,
    //SoftReoot
    output                          oMCURebootEn                            ,
    //DDR Write Port
    output                          oDdrWrOpClk                             ,
    output                          oDdrWrOpBurstBegin                      ,
    output      [27:0]              oDdrWrOpAddr                            ,
    output      [127:0]             oDdrWrOpData                            ,
    output      [15:0]              oDdrWrOpDataMask                        ,
    input                           iDdrWrOpDone                            ,
    //DDR Read Port
    output                          oDdrRdOpClk                             ,
    output                          oDdrRdOpBurstBegin                      ,
    output      [27:0]              oDdrRdOpAddr                            ,
    input                           iDdrLogicClk                            ,
    input       [127:0]             iDdrRdOpData                            ,
    input                           iDdrRdOpDataVld                          
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    localparam                      A_WIDTH             = 12                ;

//******************************************************************************

//******************************************************************************
//
    wire                            wArmTimerIrq                            ;
    wire                            wArmUartIrq                             ;
    wire                            wEthRxIrq                               ;
    wire                            wEthRxIrqClr                            ;

//******************************************************************************

//******************************************************************************
//

assign oArmIrq[31:0] = {29'd0, wEthRxIrq, wArmUartIrq, wArmTimerIrq};

//******************************************************************************

//******************************************************************************
//32'h0000_0000  Rom+Ram = 32KB

AhbSlaveSram 
#(
    .A_WIDTH                        (13                                     )
)U_AhbSlaveSram                     
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (iSysRstN                               ),
    .iAhbSel                        (iAhbSelMap[0]                          ),
    .iAhbWrite                      (iAhbWrite                              ),
    .iAhbAddr                       (iAhbAddr[31:0]                         ),
    .iAhbReady                      (iAhbReady                              ),
    .iAhbWrData                     (iAhbWrData[31:0]                       ),
    .iAhbTrans                      (iAhbTrans[1:0]                         ),
    .iAhbSize                       (iAhbSize[2:0]                          ),
    .oAhbReady                      (oAhbReadySlave[0]                      ),
    .oAhbResp                       (oAhbRespSlave[0]                       ),
    .oAhbRdData                     (oAhbRdDataSlave[0*32+:32]              )
);


/*
AhbSlaveMem 
#(
    .MEM_INIT                       ("ON"                                   ),
    .A_WIDTH                        (15                                     )
)U_AhbSlaveMem 
(
    .iSysClk                        (iSysClk                                ),
    .iAhbSel                        (iAhbSelMap[0]                          ),
    .iAhbWrite                      (iAhbWrite                              ),
    .iAhbAddr                       (iAhbAddr[31:0]                         ),
    .iAhbReady                      (iAhbReady                              ),
    .iAhbWrData                     (iAhbWrData[31:0]                       ),
    .iAhbTrans                      (iAhbTrans[1:0]                         ),
    .iAhbSize                       (iAhbSize[2:0]                          ),
    .oAhbReady                      (oAhbReadySlave[0]                      ),
    .oAhbResp                       (oAhbRespSlave[0]                       ),
    .oAhbRdData                     (oAhbRdDataSlave[0*32+:32]              ) 
);
*/
/*
AHB2MEM 
#(
    .MEMWIDTH       (15)
) inst_AHB2MEM 
(
    .HSEL                           (iAhbSelMap[0]                          ),
    .HCLK                           (iSysClk                                ),
    .HRESETn                        (iSysRstN                               ),
    .HREADY                         (iAhbReady                              ),
    .HADDR                          (iAhbAddr[31:0]                         ),
    .HTRANS                         (iAhbTrans[1:0]                         ),
    .HWRITE                         (iAhbWrite                              ),
    .HSIZE                          (iAhbSize[2:0]                          ),
    .HWDATA                         (iAhbWrData[31:0]                       ),
    .HREADYOUT                      (oAhbReadySlave[0]                      ),
    .HRDATA                         (oAhbRdDataSlave[0*32+:32]              ),
    .LED                            (                                       )
);

assign oAhbRespSlave[0] = 1'b0;
*/
//******************************************************************************

//******************************************************************************
//32'h2000_0000 DDR 2Gb = 256MB

AhbSlaveDdr U_AhbSlaveDdr
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (iSysRstN                               ),
    .iAhbSel                        (iAhbSelMap[1]                          ),
    .iAhbWrite                      (iAhbWrite                              ),
    .iAhbAddr                       (iAhbAddr[31:0]                         ),
    .iAhbReady                      (iAhbReady                              ),
    .iAhbWrData                     (iAhbWrData[31:0]                       ),
    .iAhbTrans                      (iAhbTrans[1:0]                         ),
    .iAhbSize                       (iAhbSize[2:0]                          ),
    .oAhbReady                      (oAhbReadySlave[1]                      ),
    .oAhbResp                       (oAhbRespSlave[1]                       ),
    .oAhbRdData                     (oAhbRdDataSlave[1*32+:32]              ),

    .oDdrWrOpClk                    (oDdrWrOpClk                            ),
    .oDdrWrOpBurstBegin             (oDdrWrOpBurstBegin                     ),
    .oDdrWrOpAddr                   (oDdrWrOpAddr[27:0]                     ),
    .oDdrWrOpData                   (oDdrWrOpData[127:0]                    ),
    .oDdrWrOpDataMask               (oDdrWrOpDataMask[15:0]                 ),
    .iDdrWrOpDone                   (iDdrWrOpDone                           ),
    .oDdrRdOpClk                    (oDdrRdOpClk                            ),
    .oDdrRdOpBurstBegin             (oDdrRdOpBurstBegin                     ),
    .oDdrRdOpAddr                   (oDdrRdOpAddr[27:0]                     ),
    .iDdrLogicClk                   (iDdrLogicClk                           ),
    .iDdrRdOpData                   (iDdrRdOpData[127:0]                    ),
    .iDdrRdOpDataVld                (iDdrRdOpDataVld                        ) 
);

//******************************************************************************

//******************************************************************************
//32'h3000_0000     UdpDataRxCache
//32'h3000_1000     UdpDataTxCache

AhbSlaveUdp U_AhbSlaveUdp
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (iSysRstN                               ),
    .iAhbSel                        (iAhbSelMap[2]                          ),
    .iAhbWrite                      (iAhbWrite                              ),
    .iAhbAddr                       (iAhbAddr[31:0]                         ),
    .iAhbReady                      (iAhbReady                              ),
    .iAhbWrData                     (iAhbWrData[31:0]                       ),
    .iAhbTrans                      (iAhbTrans[1:0]                         ),
    .iAhbSize                       (iAhbSize[2:0]                          ),
    .oAhbReady                      (oAhbReadySlave[2]                      ),
    .oAhbResp                       (oAhbRespSlave[2]                       ),
    .oAhbRdData                     (oAhbRdDataSlave[2*32+:32]              ),
    .iEthClk                        (iEthClk                                ),
    .iEthRxDataVld                  (iEthRxDataVld                          ),
    .iEthRxData                     (iEthRxData[7:0]                        ),
    .iEthRxIrqClr                   (wEthRxIrqClr                           ),
    .oEthRxIrq                      (wEthRxIrq                              ),
    .iEthTxDataRdEn                 (iEthTxDataRdEn                         ),
    .oEthTxData                     (oEthTxData[7:0]                        )
);

//******************************************************************************

//******************************************************************************
//
//32'h3001_0000     BmsParamCache

AhbSlaveBms U_AhbSlaveBms
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (iSysRstN                               ),
    .iAhbSel                        (iAhbSelMap[3]                          ),
    .iAhbWrite                      (iAhbWrite                              ),
    .iAhbAddr                       (iAhbAddr[31:0]                         ),
    .iAhbReady                      (iAhbReady                              ),
    .iAhbWrData                     (iAhbWrData[31:0]                       ),
    .iAhbTrans                      (iAhbTrans[1:0]                         ),
    .iAhbSize                       (iAhbSize[2:0]                          ),
    .oAhbReady                      (oAhbReadySlave[3]                      ),
    .oAhbResp                       (oAhbRespSlave[3]                       ),
    .oAhbRdData                     (oAhbRdDataSlave[3*32+:32]              ),
    .iParamBusClk                   (iBmsParamBusClk                        ),
    .iParamBusAddr                  (iBmsParamBusAddr[31:0]                 ),
    .iParamBusWrEn                  (1'd0                                   ),
    .iParamBusWrData                (8'd0                                   ),
    .oParamBusRdData                (oBmsParamBusRdData[7:0]                )
);

//32'h3002_0000     FpgaParamCache

AhbSlaveFpga U_AhbSlaveFpga
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (iSysRstN                               ),
    .iAhbSel                        (iAhbSelMap[4]                          ),
    .iAhbWrite                      (iAhbWrite                              ),
    .iAhbAddr                       (iAhbAddr[31:0]                         ),
    .iAhbReady                      (iAhbReady                              ),
    .iAhbWrData                     (iAhbWrData[31:0]                       ),
    .iAhbTrans                      (iAhbTrans[1:0]                         ),
    .iAhbSize                       (iAhbSize[2:0]                          ),
    .oAhbReady                      (oAhbReadySlave[4]                      ),
    .oAhbResp                       (oAhbRespSlave[4]                       ),
    .oAhbRdData                     (oAhbRdDataSlave[4*32+:32]              ),
    //.iFpgaParam                     (iFpgaParam[(2*32 - 1):0]               ),
    .iParamBusClk                   (iFpgaParamBusClk                       ),
    .iParamBusAddr                  (iFpgaParamBusAddr[31:0]                ),
    .iParamBusWrEn                  (iFpgaParamBusWrEn                      ),
    .iParamBusWrData                (iFpgaParamBusWrData[15:0]               ),
    .oParamBusRdData                (oFpgaParamBusRdData[15:0]               )
);

//******************************************************************************

//******************************************************************************
//32'h4000_0000

AhbSlaveGpio U_AhbSlaveGpio
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (iSysRstN                               ),
    .iAhbSel                        (iAhbSelMap[5]                          ),
    .iAhbWrite                      (iAhbWrite                              ),
    .iAhbAddr                       (iAhbAddr[31:0]                         ),
    .iAhbReady                      (iAhbReady                              ),
    .iAhbWrData                     (iAhbWrData[31:0]                       ),
    .iAhbTrans                      (iAhbTrans[1:0]                         ),
    .iAhbSize                       (iAhbSize[2:0]                          ),
    .oAhbReady                      (oAhbReadySlave[5]                      ),
    .oAhbResp                       (oAhbRespSlave[5]                       ),
    .oAhbRdData                     (oAhbRdDataSlave[5*32+:32]              ),
    .oArmLed                        (oArmLed                                ),
    .oEthTxLen                      (oEthTxLen[15:0]                        ),
    .oEthTxDataReady                (oEthTxDataReady                        ),
    .oEthRxIrqClr                   (wEthRxIrqClr                           ),
    .oEhtPhyRstN                    (oEhtPhyRstN                            ),
    .oEthPhyMdclk                   (oEthPhyMdclk                           ),
    .ioEthPhyMdio                   (ioEthPhyMdio                           ),
    .oMCURebootEn                   (oMCURebootEn                             )
);

//******************************************************************************

//******************************************************************************
//32'h4100_0000
//32'h4100_1000
//32'h4100_2000

AhbSlaveApb U_AhbSlaveApb
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (iSysRstN                               ),
    .iAhbSel                        (iAhbSelMap[6]                          ),
    .iAhbWrite                      (iAhbWrite                              ),
    .iAhbAddr                       (iAhbAddr[31:0]                         ),
    .iAhbReady                      (iAhbReady                              ),
    .iAhbWrData                     (iAhbWrData[31:0]                       ),
    .iAhbTrans                      (iAhbTrans[1:0]                         ),
    .iAhbSize                       (iAhbSize[2:0]                          ),
    .iAhbProt                       (iAhbProt[3:0]                          ),
    .oAhbReady                      (oAhbReadySlave[6]                      ),
    .oAhbResp                       (oAhbRespSlave[6]                       ),
    .oAhbRdData                     (oAhbRdDataSlave[6*32+:32]              ),
    .oArmTimerIrq                   (wArmTimerIrq                           ),
    .iArmUartRx                     (iArmUartRx                             ),
    .oArmUartTx                     (oArmUartTx                             ),
    .oArmUartIrq                    (wArmUartIrq                            ),
    .oSpiClk                        (oSpiClk                                ),
    .oSpiCs                         (oSpiCs                                 ),
    .oSpiMosi                       (oSpiMosi                               ),
    .iSpiMiso                       (iSpiMiso                               )
);

//******************************************************************************

//******************************************************************************
//

AhbSlaveDefault U_AhbSlaveDefault
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (iSysRstN                               ),
    .iAhbSel                        (iAhbSelMap[15]                         ),
    .iAhbReady                      (iAhbReady                              ),
    .iAhbTrans                      (iAhbTrans[1:0]                         ),
    .oAhbReady                      (oAhbReadySlave[15]                     ),
    .oAhbResp                       (oAhbRespSlave[15]                      ),
    .oAhbRdData                     (oAhbRdDataSlave[15*32+:32]             )
);

//******************************************************************************

endmodule
