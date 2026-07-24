`timescale 1ns / 1ps
//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   FilterCore.v
//Original Author   :   Nova003107  xcy
//Creation Date     :   2026.06
/*Description       :   滤波器处理模块 1Hz采样，滤

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


module LowSpeedFilterCore #(
    parameter CLK_FREQ                 =    8'd100    
)(
  input                   iSysClk                     ,
  input                   iSysClk1sPulse              ,
  input  signed  [31:0]   iBatVoltage                 ,
  input  signed  [31:0]   iBatAverageVoltage          ,
  
  output signed  [31:0]   oBatVoltage                 ,
  output signed  [31:0]   oBatAverageVoltage          
);


LPFtustin #(
    .TRI_MODE       (1'b0                           ), //
    .WC             (32'sd1                         ), // 0.16Hz的滤波
    .FS             (32'sd1                         )  // 1Hz采样

) U_LPFBatVol( 
    .iSysClk        (iSysClk                        ),
    .iInput         ($signed(iBatVoltage)           ),//A 路电压[15:0]
    .iSamplePulse   (iSysClk1sPulse                 ),
    .oOutput        (oBatVoltage                    )  
);


LPFtustin #(
    .TRI_MODE       (1'b0                           ),
    .WC             (32'sd1                         ),
    .FS             (32'sd1                         )  
) U_LPFBatAverageVol( 
    .iSysClk        (iSysClk                        ),
    .iInput         ($signed(iBatAverageVoltage)    ),//A 路电压[15:0]
    .iSamplePulse   (iSysClk1sPulse                 ),
    .oOutput        (oBatAverageVoltage             )  
);

endmodule
