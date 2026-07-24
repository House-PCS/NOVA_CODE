//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   ApbSlaveMux.v
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
module ApbSlaveMux
(
    input                           iApbSel                                 ,
    input       [15:0]              iApbSelMap                              ,
    output      [15:0]              oApbSelMap                              ,
    //APB Slave
    input       [(16*32 - 1):0]     iApbRdDataSlave                         ,
    input       [15:0]              iApbReadySlave                          ,
    input       [15:0]              iApbSlaveError                          ,

    //APB Master
    output      [31:0]              oApbRdData                              ,
    output                          oApbReady                               ,
    output                          oApbSlaveError                          
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    wire        [15:0]              wApbSelMap                              ;

//******************************************************************************

//******************************************************************************
//

assign wApbSelMap[15:0] = {32{iApbSel}} & iApbSelMap[15:0];

assign oApbSelMap[15:0] = wApbSelMap[15:0] ;

//******************************************************************************

//******************************************************************************
//

assign oApbRdData[31:0] =   ({32{wApbSelMap[0]}} & iApbRdDataSlave[0*32+:32]) |
                            ({32{wApbSelMap[1]}} & iApbRdDataSlave[1*32+:32]) |
                            ({32{wApbSelMap[2]}} & iApbRdDataSlave[2*32+:32]) |
                            ({32{wApbSelMap[3]}} & iApbRdDataSlave[3*32+:32]) |
                            ({32{wApbSelMap[4]}} & iApbRdDataSlave[4*32+:32]) |
                            ({32{wApbSelMap[5]}} & iApbRdDataSlave[5*32+:32]) |
                            ({32{wApbSelMap[6]}} & iApbRdDataSlave[6*32+:32]) |
                            ({32{wApbSelMap[7]}} & iApbRdDataSlave[7*32+:32]) |
                            ({32{wApbSelMap[8]}} & iApbRdDataSlave[8*32+:32]) |
                            ({32{wApbSelMap[9]}} & iApbRdDataSlave[9*32+:32]) |
                            ({32{wApbSelMap[10]}} & iApbRdDataSlave[10*32+:32]) |
                            ({32{wApbSelMap[11]}} & iApbRdDataSlave[11*32+:32]) |
                            ({32{wApbSelMap[12]}} & iApbRdDataSlave[12*32+:32]) |
                            ({32{wApbSelMap[13]}} & iApbRdDataSlave[13*32+:32]) |
                            ({32{wApbSelMap[14]}} & iApbRdDataSlave[14*32+:32]) |
                            ({32{wApbSelMap[15]}} & iApbRdDataSlave[15*32+:32]) ;

//******************************************************************************

//******************************************************************************
//

assign oApbReady =  (!iApbSel) || (| (iApbSelMap[15:0] & iApbReadySlave[15:0]));

//******************************************************************************

//******************************************************************************
//

assign oApbSlaveError = (| (wApbSelMap[15:0] & iApbSlaveError[15:0]));

//******************************************************************************

endmodule
