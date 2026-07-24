`timescale 1ns / 1ps
//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   VirtualResCtrlCore.v
//Original Author   :   xcy Nova003107
//Creation Date     :   2026.1.8
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


module VirtualResCtrlCore #(
    parameter               CLK_FREQ                =   100
)
(

    input                   iSysClk                         ,
    input                   iCnt1Ms                         ,
    input                   iSeriesRelayEn                  ,
    input   signed  [31:0]  iBusTotalCurFilterA             ,
    input   signed  [31:0]  iBusTotalCurFilterB             ,
    input   signed  [15:0]  iDCOutputTotalPower             ,
    output  signed  [31:0]  oTargetVoltageOffsetA           ,
    output  signed  [31:0]  oTargetVoltageOffsetB           
);

    localparam  S_COUNT         = 1000                      ;
    reg [10:0]  rCnt1S          = 'd0                       ;
    wire        wCountStep                                  ;


    // reg signed      [31:0] rBusTotalCurFilterA              ;
    // reg signed      [31:0] rBusTotalCurFilterB              ;

    reg signed      [31:0] rTargetVoltageOffsetA            ;
    reg signed      [31:0] rTargetVoltageOffsetB            ;

    wire signed     [31:0] wTargetVoltageOffsetA            ;
    wire signed     [31:0] wTargetVoltageOffsetB            ;

//******************************************************************************

//******************************************************************************
//1s计数器

always @ (posedge iSysClk) begin
    if(iCnt1Ms) begin
        if(rCnt1S[10:0] == (S_COUNT - 11'd1))
            rCnt1S[10:0] <= 11'd0;
        else
            rCnt1S[10:0] <= rCnt1S[10:0] + 11'd1;
    end
    else
        rCnt1S[10:0] <= rCnt1S[10:0];
end
assign wCountStep = (iCnt1Ms && (rCnt1S[10:0] == (S_COUNT - 11'd1)));

//******************************************************************************


VirtualResistanceCore #(
   .CLK_FREQ                        (CLK_FREQ                           )
)U_VirtualResistanceCoreA
(
    .iSysClk                        (iSysClk                            ),
    .iSeriesRelayEn                 (iSeriesRelayEn                     ),
    .iBusCurrent                    (iBusTotalCurFilterA                ),
    .iDCOutputTotalPower            (iDCOutputTotalPower                ),
    .oTargetVoltageOffset           (wTargetVoltageOffsetA              )
);

VirtualResistanceCore #(
   .CLK_FREQ                        (CLK_FREQ                           )
)U_VirtualResistanceB
(
    .iSysClk                        (iSysClk                            ),  
    .iSeriesRelayEn                 (iSeriesRelayEn                     ), 
    .iBusCurrent                    (iBusTotalCurFilterB                ),
    .iDCOutputTotalPower            (iDCOutputTotalPower                ),
    .oTargetVoltageOffset           (wTargetVoltageOffsetB              )
);

//******************************************************************************
//
always @ (posedge iSysClk) begin
    //if(wCountStep) begin
        rTargetVoltageOffsetA <= wTargetVoltageOffsetA;
        rTargetVoltageOffsetB <= wTargetVoltageOffsetB;
   // end
end

assign oTargetVoltageOffsetA = rTargetVoltageOffsetA ;
assign oTargetVoltageOffsetB = rTargetVoltageOffsetB ;

//******************************************************************************


endmodule
