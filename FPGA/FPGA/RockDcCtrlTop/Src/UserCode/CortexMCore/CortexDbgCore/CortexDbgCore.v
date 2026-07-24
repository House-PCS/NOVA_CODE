//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   CortexDbgCore.v
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
module CortexDbgCore
(
    //SWD Debug
    input                           iSwdClk                     ,
    inout                           ioSwdData                   ,
    //SWD
    output                          oSwdClk                     ,
    output                          oSwdIn                      ,
    input                           iSwdOut                     ,
    input                           iSwdOe                       
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

assign oSwdClk = iSwdClk;
assign oSwdIn = ioSwdData;
assign ioSwdData = iSwdOe ? iSwdOut : 1'bz;

//******************************************************************************

endmodule
