//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   ApbSlaveCore.v
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
module ApbSlaveCore
(
    //Global Clock
    input                           iSysClk                                 ,
    input                           iSysRstN                                ,
    input       [15:0]              iApbSelMap                              ,
    input       [15:0]              iApbAddr                                ,
    input                           iApbEnable                              ,
    input                           iApbWrite                               ,
    input       [31:0]              iApbWrData                              ,
    output      [(16*32 - 1):0]     oApbRdDataSlave                         ,
    output      [15:0]              oApbReadySlave                          ,
    output      [15:0]              oApbSlaveError                          ,
    //ApbSlave
    //Timer
    output                          oArmTimerIrq                            ,
    //Uart
    input                           iArmUartRx                              ,
    output                          oArmUartTx                              ,
    output                          oArmUartIrq                             ,
    //Spi
    output                          oSpiClk                                 ,
    output                          oSpiCs                                  ,
    output                          oSpiMosi                                ,
    input                           iSpiMiso                                 
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

ApbSlaveTimer U_ApbSlaveTimer
(
    .PCLK                           (iSysClk                                ),
    .PCLKG                          (iSysClk                                ),
    .PRESETn                        (iSysRstN                               ),
    .PSEL                           (iApbSelMap[0]                          ),
    .PADDR                          (iApbAddr[11:2]                         ),
    .PENABLE                        (iApbEnable                             ),
    .PWRITE                         (iApbWrite                              ),
    .PWDATA                         (iApbWrData[31:0]                       ),
    .ECOREVNUM                      (4'h0                                   ),
    .PRDATA                         (oApbRdDataSlave[0*32+:32]              ),
    .PREADY                         (oApbReadySlave[0]                      ),
    .PSLVERR                        (oApbSlaveError[0]                      ),
    .EXTIN                          (1'b0                                   ),
    .TIMERINT                       (oArmTimerIrq                           )
);

//******************************************************************************

//******************************************************************************
//

ApbSlaveUart U_ApbSlaveUart
(
    .PCLK                           (iSysClk                                ),
    .PCLKG                          (iSysClk                                ),
    .PRESETn                        (iSysRstN                               ),
    .PSEL                           (iApbSelMap[1]                          ),
    .PADDR                          (iApbAddr[11:2]                         ),
    .PENABLE                        (iApbEnable                             ),
    .PWRITE                         (iApbWrite                              ),
    .PWDATA                         (iApbWrData[31:0]                       ),
    .ECOREVNUM                      (4'h0                                   ),
    .PRDATA                         (oApbRdDataSlave[1*32+:32]              ),
    .PREADY                         (oApbReadySlave[1]                      ),
    .PSLVERR                        (oApbSlaveError[1]                      ),
    .RXD                            (iArmUartRx                             ),
    .TXD                            (oArmUartTx                             ),
    .TXEN                           (                                       ),
    .BAUDTICK                       (                                       ),
    .TXINT                          (                                       ),
    .RXINT                          (                                       ),
    .TXOVRINT                       (                                       ),
    .RXOVRINT                       (                                       ),
    .UARTINT                        (oArmUartIrq                            )
);

//******************************************************************************

//******************************************************************************
//

ApbSlaveSpi U_ApbSlaveSpi
(
    .PCLK                           (iSysClk                                ),
    .PCLKG                          (iSysClk                                ),
    .PRESETn                        (iSysRstN                               ),
    .PSEL                           (iApbSelMap[2]                          ),
    .PADDR                          (iApbAddr[11:2]                         ),
    .PENABLE                        (iApbEnable                             ),
    .PWRITE                         (iApbWrite                              ),
    .PWDATA                         (iApbWrData[31:0]                       ),
    .PRDATA                         (oApbRdDataSlave[2*32+:32]              ),
    .PREADY                         (oApbReadySlave[2]                      ),
    .PSLVERR                        (oApbSlaveError[2]                      ),
    .SPI_Clk                        (oSpiClk                                ),
    .SPI_CS                         (oSpiCs                                 ),
    .SPI_MOSI                       (oSpiMosi                               ),
    .SPI_MISO                       (iSpiMiso                               )
);

//******************************************************************************

endmodule
