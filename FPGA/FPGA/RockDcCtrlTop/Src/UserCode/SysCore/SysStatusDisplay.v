//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   SysLed.v
//Original Author   :   xcy 003107
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
`timescale 1ns / 1ps

module SysStatusDisplay
#(
    parameter       CLK_FREQ    = 100
)
(
    //Global Clock
    input               iSysClk         ,
    input               iNoEthNet       , 
    input    [1:0]      iErrDisplay     ,
    output   [1:0]      oSrcStable          
);

//******************************************************************************
//
reg [2:0]       rStatus         ;
reg [1:0]       rSrcStable      ;   
//******************************************************************************


//******************************************************************************
//


always @(posedge iSysClk ) begin
    rStatus[2:0] <= {iErrDisplay[1:0],iNoEthNet};
end

always @(posedge iSysClk ) begin
    case (rStatus[2:0])
        3'b000:     rSrcStable  <=  2'd0; //正常运行  正常闪烁
        3'b001:     rSrcStable  <=  2'd1; //没有网络  两次闪烁
        3'b010:     rSrcStable  <=  2'd3; //BMS异常   快速闪烁
        3'b100:     rSrcStable  <=  2'd2; //DCDC异常  三次闪烁
        default: rSrcStable  <=  rSrcStable;
    endcase
end

assign oSrcStable = rSrcStable;
//******************************************************************************

endmodule
