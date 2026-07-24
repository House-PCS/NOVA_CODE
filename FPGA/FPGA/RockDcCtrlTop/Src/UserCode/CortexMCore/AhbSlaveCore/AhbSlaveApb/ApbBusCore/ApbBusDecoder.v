//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   ApbBusDecoder.v
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
module ApbBusDecoder
(
    input       [15:0]              iApbAddr                                ,
    output      [15:0]              oApbSelMap                              
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

assign oApbSelMap[0]    = (iApbAddr[15:12] == 4'd0)     ;                       //ApbTimer
assign oApbSelMap[1]    = (iApbAddr[15:12] == 4'd1)     ;                       //ApbUart
assign oApbSelMap[2]    = (iApbAddr[15:12] == 4'd2)     ;
assign oApbSelMap[3]    = (iApbAddr[15:12] == 4'd3)     ;
assign oApbSelMap[4]    = (iApbAddr[15:12] == 4'd4)     ;
assign oApbSelMap[5]    = (iApbAddr[15:12] == 4'd5)     ;
assign oApbSelMap[6]    = (iApbAddr[15:12] == 4'd6)     ;
assign oApbSelMap[7]    = (iApbAddr[15:12] == 4'd7)     ;
assign oApbSelMap[8]    = (iApbAddr[15:12] == 4'd8)     ;
assign oApbSelMap[9]    = (iApbAddr[15:12] == 4'd9)     ;
assign oApbSelMap[10]   = (iApbAddr[15:12] == 4'd10)    ;
assign oApbSelMap[11]   = (iApbAddr[15:12] == 4'd11)    ;
assign oApbSelMap[12]   = (iApbAddr[15:12] == 4'd12)    ;
assign oApbSelMap[13]   = (iApbAddr[15:12] == 4'd13)    ;
assign oApbSelMap[14]   = (iApbAddr[15:12] == 4'd14)    ;
assign oApbSelMap[15]   = (iApbAddr[15:12] == 4'd15)    ;

//******************************************************************************

endmodule
