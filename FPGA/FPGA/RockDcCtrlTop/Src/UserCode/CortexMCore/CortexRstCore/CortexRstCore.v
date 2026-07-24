//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   CortexRstCore.v
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
module CortexRstCore
(
    //Global Clock
    input                           iSysClk                     ,
    input                           iSysRst                     ,
    //Arm Rst Req
    input                           iArmRstReq                  ,
    output                          oArmSoftRstN                ,
    output                          oSysRstN                     
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    reg                             rArmSoftRstN        = 1'd0  ;

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk or posedge iSysRst)begin
    if(iSysRst)
        rArmSoftRstN <= 1'b0;
    else if(iArmRstReq)
        rArmSoftRstN <= 1'b0;
    else
        rArmSoftRstN <= 1'b1;
end

//******************************************************************************

//******************************************************************************
//

assign oArmSoftRstN = rArmSoftRstN;
assign oSysRstN = (!iSysRst);

//******************************************************************************

endmodule
