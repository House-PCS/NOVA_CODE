//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AhbLiteMux.v
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
module AhbLiteMux
(
    //Global Clock
    input                           iSysClk                                 ,
    input                           iSysRstN                                ,
    input                           iAhbReady                               ,
    input       [15:0]              iAhbSelMap                              ,
    input       [(16*32 - 1):0]     iAhbRdDataSlave                         ,
    input       [15:0]              iAhbReadySlave                          ,
    input       [15:0]              iAhbRespSlave                           ,

    output      [31:0]              oAhbRdData                              ,
    output                          oAhbReady                               ,
    output                          oAhbResp                                                                
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    reg         [15:0]              rAhbSelMap          = 16'd0             ;

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk or negedge iSysRstN) begin
    if(!iSysRstN)
        rAhbSelMap[15:0] <= 16'd0;
    else if(iAhbReady)
        rAhbSelMap[15:0] <= iAhbSelMap[15:0];
    else
        rAhbSelMap[15:0] <= rAhbSelMap[15:0];
end

//******************************************************************************

//******************************************************************************
//

assign oAhbReady =  ((rAhbSelMap[0] == 1'd0) || (iAhbReadySlave[0] == 1'd1)) &
                    ((rAhbSelMap[1] == 1'd0) || (iAhbReadySlave[1] == 1'd1)) &
                    ((rAhbSelMap[2] == 1'd0) || (iAhbReadySlave[2] == 1'd1)) &
                    ((rAhbSelMap[3] == 1'd0) || (iAhbReadySlave[3] == 1'd1)) &
                    ((rAhbSelMap[4] == 1'd0) || (iAhbReadySlave[4] == 1'd1)) &
                    ((rAhbSelMap[5] == 1'd0) || (iAhbReadySlave[5] == 1'd1)) &
                    ((rAhbSelMap[6] == 1'd0) || (iAhbReadySlave[6] == 1'd1)) &
                    ((rAhbSelMap[7] == 1'd0) || (iAhbReadySlave[7] == 1'd1)) &
                    ((rAhbSelMap[8] == 1'd0) || (iAhbReadySlave[8] == 1'd1)) &
                    ((rAhbSelMap[9] == 1'd0) || (iAhbReadySlave[9] == 1'd1)) &
                    ((rAhbSelMap[15] == 1'd0) || (iAhbReadySlave[15] == 1'd1));


//******************************************************************************

//******************************************************************************
//

assign oAhbRdData[31:0] =   ({32{rAhbSelMap[0]}} & iAhbRdDataSlave[0*32+:32]) |
                            ({32{rAhbSelMap[1]}} & iAhbRdDataSlave[1*32+:32]) |
                            ({32{rAhbSelMap[2]}} & iAhbRdDataSlave[2*32+:32]) |
                            ({32{rAhbSelMap[3]}} & iAhbRdDataSlave[3*32+:32]) |
                            ({32{rAhbSelMap[4]}} & iAhbRdDataSlave[4*32+:32]) |
                            ({32{rAhbSelMap[5]}} & iAhbRdDataSlave[5*32+:32]) |
                            ({32{rAhbSelMap[6]}} & iAhbRdDataSlave[6*32+:32]) |
                            ({32{rAhbSelMap[7]}} & iAhbRdDataSlave[7*32+:32]) |
                            ({32{rAhbSelMap[8]}} & iAhbRdDataSlave[8*32+:32]) |
                            ({32{rAhbSelMap[9]}} & iAhbRdDataSlave[9*32+:32]) |
                            ({32{rAhbSelMap[15]}} & iAhbRdDataSlave[15*32+:32]);

//******************************************************************************

//******************************************************************************
//

assign oAhbResp =   ((rAhbSelMap[0] == 1'd1) && (iAhbRespSlave[0] == 1'd1)) |
                    ((rAhbSelMap[1] == 1'd1) && (iAhbRespSlave[1] == 1'd1)) |
                    ((rAhbSelMap[2] == 1'd1) && (iAhbRespSlave[2] == 1'd1)) |
                    ((rAhbSelMap[3] == 1'd1) && (iAhbRespSlave[3] == 1'd1)) |
                    ((rAhbSelMap[4] == 1'd1) && (iAhbRespSlave[4] == 1'd1)) |
                    ((rAhbSelMap[5] == 1'd1) && (iAhbRespSlave[5] == 1'd1)) |
                    ((rAhbSelMap[6] == 1'd1) && (iAhbRespSlave[6] == 1'd1)) |
                    ((rAhbSelMap[7] == 1'd1) && (iAhbRespSlave[7] == 1'd1)) |
                    ((rAhbSelMap[8] == 1'd1) && (iAhbRespSlave[8] == 1'd1)) |
                    ((rAhbSelMap[9] == 1'd1) && (iAhbRespSlave[9] == 1'd1)) |
                    ((rAhbSelMap[15] == 1'd1) && (iAhbRespSlave[15] == 1'd1));

//******************************************************************************

endmodule
