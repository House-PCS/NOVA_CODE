//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   SysCore.v
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
module SysCore
#(
    parameter                       CLK_FREQ1    = 50,
    parameter                       CLK_FREQ2    = 100,
    parameter                       CLK_FREQ3    = 200,
    parameter                       CLK_FREQ4    = 20      
)
(
    input                           iClk1               ,
    input                           iClk2               ,
    input                           iClk3               ,
    input                           iClk4               ,
    input                           iClk1Cnt1Us         ,
    input                           iClk2Cnt1Us         ,
    input                           iClk3Cnt1Us         ,
    input                           iClk4Cnt1Us         ,
    input                           iClk1Cnt1Ms         ,
    input                           iPllLocked          ,
    input          [1:0]            iSysStable          ,
    output                          oClk1RstP           ,
    output                          oClk2RstP           ,
    output                          oClk2RstN           ,
    output                          oClk3RstP           ,
    output                          oClk4RstP           ,
    output                          oSysLed
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
    wire                                wClk1RstP                   ;
    wire                                wClk2RstP                   ;
    wire                                wClk2RstN                   ;
    wire                                wClk3RstP                   ;
    wire                                wClk4RstP                   ;
//******************************************************************************

//******************************************************************************
//
SysRst
#(
    .CLK_FREQ                       (CLK_FREQ1          )
)U_RstP_Clk1 
(
    .iClk                           (iClk1              ),
    .iPllLocked                     (iPllLocked         ),
    .iCnt1Us                        (iClk1Cnt1Us        ),
    .oRstP                          (wClk1RstP          ),
    .oRstN                          (                   )
);

assign oClk1RstP = wClk1RstP;

SysRst
#(
    .CLK_FREQ                       (CLK_FREQ2          )
)U_RstP_Clk2 
(
    .iClk                           (iClk2              ),
    .iPllLocked                     (iPllLocked         ),
    .iCnt1Us                        (iClk2Cnt1Us        ),
    .oRstP                          (wClk2RstP          ),
    .oRstN                          (wClk2RstN          )
);


assign oClk2RstP = wClk2RstP;
assign oClk2RstN = wClk2RstN;
SysRst
#(
    .CLK_FREQ                       (CLK_FREQ3          )
)U_RstP_Clk3 
(
    .iClk                           (iClk3              ),
    .iPllLocked                     (iPllLocked         ),
    .iCnt1Us                        (iClk3Cnt1Us        ),
    .oRstP                          (wClk3RstP          ),
    .oRstN                          (                   )
);

assign oClk3RstP = wClk3RstP;

SysRst
#(
    .CLK_FREQ                       (CLK_FREQ4          )
)U_RstP_Clk4 
(
    .iClk                           (iClk4              ),
    .iPllLocked                     (iPllLocked         ),
    .iCnt1Us                        (iClk4Cnt1Us        ),
    .oRstP                          (wClk4RstP          ),
    .oRstN                          (                   )
);

assign oClk4RstP = wClk4RstP;
//******************************************************************************

//******************************************************************************
//

SysLed 
#(
    .CLK_FREQ                       (CLK_FREQ1          )
)U_SysLed 
(
    .iSysClk                        (iClk1              ),
    .iSysRst                        (wClk1RstP          ),
    .iCnt1Ms                        (iClk1Cnt1Ms        ),
    .iSrcStable                     (iSysStable         ),
    .oSysLed                        (oSysLed            )
);

//******************************************************************************

endmodule
