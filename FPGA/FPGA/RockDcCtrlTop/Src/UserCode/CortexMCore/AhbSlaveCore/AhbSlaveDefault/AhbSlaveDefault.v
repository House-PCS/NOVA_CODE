//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AhbSlaveDefault.v
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
module AhbSlaveDefault
(
    //Global Clock
    input                           iSysClk                                 ,
    input                           iSysRstN                                ,
    input                           iAhbSel                                 ,
    input                           iAhbReady                               ,
    input       [1:0]               iAhbTrans                               ,
    output                          oAhbReady                               ,
    output                          oAhbResp                                ,          
    output      [31:0]              oAhbRdData                              
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    wire                            wTransReq                               ;
    wire        [1:0]               wNextState                              ;
    reg         [1:0]               rRespState                              ;

//******************************************************************************

//******************************************************************************
//

assign wTransReq = iAhbSel && iAhbTrans[1] && iAhbReady;

assign wNextState[1:0] = {wTransReq | (~rRespState[0]), (~wTransReq)};

always @ (posedge iSysClk or negedge iSysRstN) begin
    if(!iSysRstN)
        rRespState[1:0] <= 2'b01;
    else
        rRespState[1:0] <= wNextState[1:0];
end

//******************************************************************************

//******************************************************************************
//

assign oAhbReady = rRespState[0];
assign oAhbResp  = rRespState[1];
assign oAhbRdData[31:0] = 32'd0;

//******************************************************************************

endmodule
