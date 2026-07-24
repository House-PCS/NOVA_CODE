`timescale 1ns / 1ps
//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   FilterCore.v
//Original Author   :   Nova003107  xcy
//Creation Date     :   2025.10
/*Description       :   滤波器处理模块

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


module FilterCore #(
    parameter CLK_FREQ                 =    8'd100    
)(
input                   iSysClk                     ,
input                   iAdcSampleSequenceDone      ,
input  signed  [31:0]   iLlcBusVoltageA             ,
input  signed  [31:0]   iLlcBusVoltageB             ,
input  signed  [31:0]   iBusTotalVoltage            ,
input  signed  [31:0]   iBusTotalVoltagePre         ,
input  signed  [31:0]   iBusVoltageA32              ,
input  signed  [31:0]   iBusVoltageB32              ,
input  signed  [31:0]   iLlcBusCurrentA32           ,
input  signed  [31:0]   iLlcBusCurrentB32           ,
input  signed  [31:0]   iBusTotalCurrentA32         ,
input  signed  [31:0]   iBusTotalCurrentB32         ,

output  signed [31:0]   oLlcBusVolFilterA           ,
output  signed [31:0]   oLlcBusVolFilterB           ,
output  signed [31:0]   oBusTotalVolFilter          ,
output  signed [31:0]   oBusTotalVolFilterPre       ,
output  signed [31:0]   oBusVolFilterA              ,
output  signed [31:0]   oBusVolFilterB              ,
output  signed [31:0]   oLlcBusCurFilterA           ,
output  signed [31:0]   oLlcBusCurFilterB           ,
output  signed [31:0]   oBusTotalCurFilterA         ,
output  signed [31:0]   oBusTotalCurFilterB         
   );

LPFtustin #(
    .TRI_MODE       (1'b0                           ),
    .WC             (32'sd3140                      ),
    .FS             (32'sd53418                     )
) U_LPFBusVolA( 
    .iSysClk        (iSysClk                        ),
    .iInput         ($signed(iLlcBusVoltageA)       ),//A 路电压[15:0]
    .iSamplePulse   (iAdcSampleSequenceDone         ),
    .oOutput        (oLlcBusVolFilterA              )  
);
LPFtustin #(
    .TRI_MODE       (1'b0                           ),
    .WC             (32'sd3140                      ),
    .FS             (32'sd53418                     )  
) U_LPFBusVolB( 
    .iSysClk        (iSysClk                        ),
    .iInput         ($signed(iLlcBusVoltageB)       ),//A 路电压[15:0]
    .iSamplePulse   (iAdcSampleSequenceDone         ),
    .oOutput        (oLlcBusVolFilterB              )  
);
LPFtustin #(
    .TRI_MODE       (1'b0                           ),
    .WC             (32'sd3140                      ),
    .FS             (32'sd53418                     )  
) U_LPFBusToTalVol( 
    .iSysClk        (iSysClk                        ),
    .iInput         ($signed(iBusTotalVoltage)      ),//A 路电压[15:0]
    .iSamplePulse   (iAdcSampleSequenceDone         ),
    .oOutput        (oBusTotalVolFilter             )  
);
LPFtustin #(
    .TRI_MODE       (1'b0                           ),
    .WC             (32'sd3140                      ),
    .FS             (32'sd53418                     )  
) U_LPFBusToTalVolPre(  
    .iSysClk        (iSysClk                        ),
    .iInput         ($signed(iBusTotalVoltagePre)   ),//A 路电压[15:0]
    .iSamplePulse   (iAdcSampleSequenceDone         ),
    .oOutput        (oBusTotalVolFilterPre          )  
);

LPFtustin #(
    .TRI_MODE       (1'b0                           ),
    .WC             (32'sd3140                      ),
    .FS             (32'sd53418                     )  
) U_LPFCoreVolA(
    .iSysClk        (iSysClk                        ),
    .iInput         (iBusVoltageA32                 ),//A 路电压[15:0]
    .iSamplePulse   (iAdcSampleSequenceDone         ),
    .oOutput        (oBusVolFilterA                 )  
);

LPFtustin #(
    .TRI_MODE       (1'b0                           ),
    .WC             (32'sd3140                      ),
    .FS             (32'sd53418                     )  
) U_LPFCoreVolB(
    .iSysClk        (iSysClk                        ),
    .iInput         (iBusVoltageB32                 ),//B 路电压[15:0]
    .iSamplePulse   (iAdcSampleSequenceDone         ),
    .oOutput        (oBusVolFilterB                 )  
);

LPFtustin #(
    .TRI_MODE       (1'b0                           ),
    .WC             (32'sd6280                      ),
    .FS             (32'sd53418                     )  
) U_LPFCoreCurA(
    .iSysClk        (iSysClk                        ),
    .iInput         (iLlcBusCurrentA32              ),
    .iSamplePulse   (iAdcSampleSequenceDone         ),
    .oOutput        (oLlcBusCurFilterA              )  
);

LPFtustin #(
    .TRI_MODE       (1'b0                           ),
    .WC             (32'sd6280                      ),
    .FS             (32'sd53418                     )  
) U_LPFCoreCurB(
    .iSysClk        (iSysClk                        ),
    .iInput         (iLlcBusCurrentB32              ),
    .iSamplePulse   (iAdcSampleSequenceDone         ),
    .oOutput        (oLlcBusCurFilterB              )  
);

LPFtustin #(
    .TRI_MODE       (1'b0                           ),
    .WC             (32'sd30                        ),
    .FS             (32'sd53418                     )  
) U_LPFCoreBusTotalCurA(
    .iSysClk        (iSysClk                        ),
    .iInput         (iBusTotalCurrentA32            ),
    .iSamplePulse   (iAdcSampleSequenceDone         ),
    .oOutput        (oBusTotalCurFilterA            )  
);

LPFtustin #(
    .TRI_MODE       (1'b0                           ),
    .WC             (32'sd30                        ),
    .FS             (32'sd53418                     )  
) U_LPFCoreBusTotalCurB(
    .iSysClk        (iSysClk                        ),
    .iInput         (iBusTotalCurrentB32            ),
    .iSamplePulse   (iAdcSampleSequenceDone         ),
    .oOutput        (oBusTotalCurFilterB            )  
);

endmodule
