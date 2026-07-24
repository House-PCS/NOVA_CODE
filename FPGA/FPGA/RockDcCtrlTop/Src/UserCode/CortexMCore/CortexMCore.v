//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   CortexMCore.v
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
module CortexMCore
#(
    parameter                       CLK_FREQ            = 50        
)
(
    //Global Clock
    input                           iSysClk                                 ,
    input                           iSysRst                                 ,
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
    //ArmPort
    output                          oArmLed                                 ,
    output                          oEhtPhyRstN                             ,
    output                          oEthPhyMdclk                            ,
    inout                           ioEthPhyMdio                            ,
   //SoftReoot
    output                          oMCURebootEn                            ,

    input                           iArmUartRx                              ,
    output                          oArmUartTx                              ,
    //Spi
    output                          oSpiClk                                 ,
    output                          oSpiCs                                  ,
    output                          oSpiMosi                                ,
    input                           iSpiMiso                                ,
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
    input                           iDdrRdOpDataVld                         ,
    //SWD
    input                           iSwdClk                                 ,
    inout                           ioSwdData                            
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    wire                            wSysRstN                                ;
    wire        [31:0]              wArmIrq                                 ;

    wire                            wAhbWrite                               ;
    wire        [31:0]              wAhbAddr                                ;
    wire        [31:0]              wAhbWrData                              ;
    wire        [1:0]               wAhbTrans                               ;
    wire        [2:0]               wAhbSize                                ;
    wire        [3:0]               wAhbProt                                ;
    wire        [2:0]               wAhbBurst                               ;
    wire                            wAhbReady                               ;
    wire                            wAhbResp                                ;
    wire        [31:0]              wAhbRdData                              ;

    wire        [(16*32 - 1):0]     wAhbRdDataSlave                         ;
    wire        [15:0]              wAhbReadySlave                          ;
    wire        [15:0]              wAhbRespSlave                           ;
    wire        [15:0]              wAhbSelMap                              ;

    wire                            wSwdClk                                 ;
    wire                            wSwdIn                                  ;
    wire                            wSwdOut                                 ;
    wire                            wSwdOe                                  ;


    wire                            wArmSoftRstN                            ;
    wire                            wArmRstReq                              ;
    wire                            wDbgPwrUp                               ;
    wire                            wHalTed                                 ;
    wire                            wDbgPwrUpReq                            ;
    wire                            wDbgPwrUpAck                            ;


//******************************************************************************

//******************************************************************************
//

CortexRstCore U_CortexRstCore
(
    .iSysClk                        (iSysClk                                ),
    .iSysRst                        (iSysRst                                ),
    .iArmRstReq                     (wArmRstReq                             ),
    .oArmSoftRstN                   (wArmSoftRstN                           ),
    .oSysRstN                       (wSysRstN                               )
);

//******************************************************************************

//******************************************************************************
//

CortexM0Integration U_CortexM0Integration
(
    .FCLK                           (iSysClk                                ),  //i
    .SCLK                           (iSysClk                                ),  //i
    .HCLK                           (iSysClk                                ),  //i 总线时钟
    .DCLK                           (iSysClk                                ),  //i
    
    .PORESETn                       (wSysRstN                               ),  //i
    .DBGRESETn                      (wSysRstN                               ),  //i
    .HRESETn                        (wArmSoftRstN                           ),  //i 总线复位
    //AHB Data Port
    .HWRITE                         (wAhbWrite                              ),  //o 传输方向 1写 0读
    .HADDR                          (wAhbAddr[31:0]                         ),  //o 地址总线 32位系统地址总线
    .HREADY                         (wAhbReady                              ),  //i 传送完成   控制HRDATA的读出数据周期， 高:Slave输出传输结束 低:Slave需延长传输周期
    .HRDATA                         (wAhbRdData[31:0]                       ),  //i 读数据总线  读数据总线，Slave到Master
    .HWDATA                         (wAhbWrData[31:0]                       ),  //o 写数据总线  写数据总线，Masterl到Slave
    //AHB Control Port
    .HTRANS                         (wAhbTrans[1:0]                         ),  //o 传送类型   当前传输类型 00:IDLE 01:BUSY 10:NONSEQ 11:SEQ
    .HSIZE                          (wAhbSize[2:0]                          ),  //o 传送带宽   每一个ransfer传输的数据大小，以字节为单位，最高支持1024位
    .HBURST                         (wAhbBurst[2:0]                         ),  //o 批量传送   burst类型，支持4、8、16 burst,incrementing/wwrapping
    .HMASTLOCK                      (                                       ),  //o
    .HPROT                          (wAhbProt[3:0]                          ),  //o 保护控制  保护控制信号，需要slave带保护功能，一般不用  
    .HRESP                          (wAhbResp                               ),  //i 传送响应    Slave发给Master的总线传输状态OKAY,ERROR,RETRY,SPLIT
    .HMASTER                        (                                       ),  //o
    
    .CODENSEQ                       (                                       ),  //o
    .CODEHINTDE                     (                                       ),  //o
    .SPECHTRANS                     (                                       ),  //o
    //SWD Debug
    .SWCLKTCK                       (wSwdClk                                ),  //i swd_clk_i
    .SWDITMS                        (wSwdIn                                 ),  //i swd_dat_i
    .SWDO                           (wSwdOut                                ),  //o swd_dat_o
    .SWDOEN                         (wSwdOe                                 ),  //o swd_dat_oe
    
    .TDI                            (1'b0                                   ),  //i
    .nTRST                          (1'b0                                   ),  //i
    .TDO                            (                                       ),  //o
    .nTDOEN                         (                                       ),  //o
    
    .DBGRESTART                     (1'b0                                   ),  //i
    .EDBGRQ                         (1'b0                                   ),  //i
    
    .DBGRESTARTED                   (                                       ),  //o
    .HALTED                         (wHalTed                                ),  //o
    //CM1 Sideband Signals
    .NMI                            (1'b0                                   ),  //i
    .IRQ                            (wArmIrq[31:0]                          ),  //i
    
    .TXEV                           (                                       ),  //o
    .RXEV                           (1'b0                                   ),  //i
    .LOCKUP                         (                                       ),  //o
    .SYSRESETREQ                    (wArmRstReq                             ),  //o
    
    .STCALIB                        ({26{1'b0}}                             ),  //i
    .STCLKEN                        (1'b0                                   ),  //i
    .IRQLATENCY                     (8'h00                                  ),  //i
    .ECOREVNUM                      ({28{1'b0}}                             ),  //i  // [27:20] to DAP, [19:0] to core
    
    .GATEHCLK                       (                                       ),  //o
    .SLEEPING                       (                                       ),  //o
    .SLEEPDEEP                      (                                       ),  //o
    .WAKEUP                         (                                       ),  //o
    .WICSENSE                       (                                       ),  //o
    .SLEEPHOLDACKn                  (                                       ),  //o
    .WICENACK                       (                                       ),  //o
    
    .SLEEPHOLDREQn                  (1'b1                                   ),  //i
    .WICENREQ                       (1'b0                                   ),  //i
    
    .CDBGPWRUPREQ                   (wDbgPwrUpReq                           ),  //o
    .CDBGPWRUPACK                   (wDbgPwrUpAck                           ),  //i
    
    .SE                             (1'b0                                   ),  //i
    .RSTBYPASS                      (1'b0                                   )   //i
);

assign wDbgPwrUpAck = wDbgPwrUpReq;

//******************************************************************************

//******************************************************************************
//

CortexDbgCore U_CortexDbgCore
(
    .iSwdClk                        (iSwdClk                                ),
    .ioSwdData                      (ioSwdData                              ),
    .oSwdClk                        (wSwdClk                                ),
    .oSwdIn                         (wSwdIn                                 ),
    .iSwdOut                        (wSwdOut                                ),
    .iSwdOe                         (wSwdOe                                 ) 
);

//******************************************************************************

//******************************************************************************
//

AhbLiteCore U_AhbLiteCore
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (wArmSoftRstN                           ),
    .iAhbReady                      (wAhbReady                              ),
    .iAhbAddr                       (wAhbAddr[31:0]                         ),
    .oAhbSelMap                     (wAhbSelMap[15:0]                       ),
    .iAhbRdDataSlave                (wAhbRdDataSlave[(16*32 - 1):0]         ),
    .iAhbReadySlave                 (wAhbReadySlave[15:0]                   ),
    .iAhbRespSlave                  (wAhbRespSlave[15:0]                    ),
    .oAhbReady                      (wAhbReady                              ),
    .oAhbResp                       (wAhbResp                               ),
    .oAhbRdData                     (wAhbRdData[31:0]                       )
);

//******************************************************************************

//******************************************************************************
//

AhbSlaveCore 
#(
    .CLK_FREQ                       (CLK_FREQ                               )
)U_AhbSlaveCore
(
    .iSysClk                        (iSysClk                                ),
    .iSysRstN                       (wArmSoftRstN                           ),
    .iAhbAddr                       (wAhbAddr[31:0]                         ),
    .iAhbWrite                      (wAhbWrite                              ),
    .iAhbReady                      (wAhbReady                              ),
    .iAhbWrData                     (wAhbWrData[31:0]                       ),
    .iAhbTrans                      (wAhbTrans[1:0]                         ),
    .iAhbSize                       (wAhbSize[2:0]                          ),
    .iAhbProt                       (wAhbProt[3:0]                          ),
    .iAhbBurst                      (wAhbBurst[2:0]                         ),
    .iAhbSelMap                     (wAhbSelMap[15:0]                       ),
    .oAhbReadySlave                 (wAhbReadySlave[15:0]                   ),
    .oAhbRespSlave                  (wAhbRespSlave[15:0]                    ),
    .oAhbRdDataSlave                (wAhbRdDataSlave[(16*32 - 1):0]         ),

    .iEthClk                        (iEthClk                                ),
    .iEthRxDataVld                  (iEthRxDataVld                          ),
    .iEthRxData                     (iEthRxData[7:0]                        ),
    .oEthTxDataReady                (oEthTxDataReady                        ),
    .oEthTxLen                      (oEthTxLen[15:0]                        ),
    .iEthTxDataRdEn                 (iEthTxDataRdEn                         ),
    .oEthTxData                     (oEthTxData[7:0]                        ),
    
    .iBmsParamBusClk                (iBmsParamBusClk                        ),
    .iBmsParamBusAddr               (iBmsParamBusAddr[31:0]                 ),
    .oBmsParamBusRdData             (oBmsParamBusRdData[7:0]                ),
    
    //.iFpgaParam                     (iFpgaParam[(2*32 - 1):0]               ),
    .iFpgaParamBusClk               (iFpgaParamBusClk                       ),    
    .iFpgaParamBusAddr              (iFpgaParamBusAddr[31:0]                ),    
    .iFpgaParamBusWrEn              (iFpgaParamBusWrEn                      ),
    .iFpgaParamBusWrData            (iFpgaParamBusWrData[15:0]              ),
    .oFpgaParamBusRdData            (oFpgaParamBusRdData[15:0]              ),

    .oArmIrq                        (wArmIrq[31:0]                          ),
    .oArmLed                        (oArmLed                                ),

    .oEhtPhyRstN                    (oEhtPhyRstN                            ),
    .oEthPhyMdclk                   (oEthPhyMdclk                           ),
    .ioEthPhyMdio                   (ioEthPhyMdio                           ),

    .oMCURebootEn                   (oMCURebootEn                             ),

    .iArmUartRx                     (iArmUartRx                             ),
    .oArmUartTx                     (oArmUartTx                             ),

    .oSpiClk                        (oSpiClk                                ),
    .oSpiCs                         (oSpiCs                                 ),
    .oSpiMosi                       (oSpiMosi                               ),
    .iSpiMiso                       (iSpiMiso                               ),

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

endmodule
