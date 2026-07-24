//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   VirtualResistanceCore.v
//Original Author   :   xcy Nova003107
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

module VirtualResistanceCore #(
    parameter               CLK_FREQ                =   100
   )(
    input                   iSysClk                         , 
    input                   iSeriesRelayEn                  ,  
    input   signed [31:0]   iBusCurrent                     ,
    input   signed [15:0]   iDCOutputTotalPower             ,
    output  signed [31:0]   oTargetVoltageOffset            
    );

    //-------------------------------Code Start Here--------------------------------
    localparam  P_MAX        = 16'sd4000 ;
    //******************************************************************************
    //  
    wire                    wDivisionCalculateDone     ;
    wire                    wSqrt3CalculateDone        ;
    reg           [15:0]    rDCOutputTotalPowerAbs          = 16'd0     ;
    reg  signed   [15:0]    rMaxPowerDiffDCOutputTotalPower = 16'sd0    ;
    wire signed   [31:0]    wTargetVoltageOffsetLin    ;
    reg  signed   [31:0]    rTargetVoltageOffsetLin    ;
    wire signed   [31:0]    wTargetVoltageOffsetCube   ;
    reg  signed   [31:0]    rTargetVoltageOffsetCube   ;
    reg  signed   [31:0]    rTargetVoltageOffsetRealPre;
    reg  signed   [31:0]    rTargetVoltageOffsetPre    ;
    wire signed   [31:0]    wTargetVoltageOffsetReal   ;
    reg  signed   [31:0]    rTargetVoltageOffsetReal   ;
    wire                    wTargetVoltageOffsetDone   ;
    //******************************************************************************

    //******************************************************************************
    //虚拟阻抗
    SignedDivision #(   //线性虚拟阻抗 5R 
        .WITH1      (32                                     ),  // 被除数位宽
        .WITH2      (16                                     )   // 除数位宽
    ) U_Division_VirRef (
        .iSysClk    (iSysClk                                ),
        .iSysRst    (1'b0                                   ),
        .iStart     (1'b1                                   ),
        .iDividend  ($signed(iBusCurrent[31:0])             ),
        .iDivisor   (16'sd17                                ), //5R 的阻抗
        .oQuotient  (wTargetVoltageOffsetLin[31:0]          ),
        .oRemainder (                                       ),
        .oDone      (wDivisionCalculateDone                 )
    );

    always @(posedge iSysClk ) begin
        if(wDivisionCalculateDone)
            rTargetVoltageOffsetLin  <= wTargetVoltageOffsetLin ;
        else
            rTargetVoltageOffsetLin  <= rTargetVoltageOffsetLin ;
    end
    //******************************************************************************

    //******************************************************************************
    CubeRootNewton#(
        .WIDTH          (32                                ),
        .ITERATIONS     (20                                )
    )U_CubeRoot_VirRef       
    (          
        .iSysClk        (iSysClk                           ),
        .iSysRst        (1'b0                              ),
        .iData          ($signed(iBusCurrent)              ),    //-32767 ~ 32767
        .iDataValid     (1'b1                              ),

        .oData          (wTargetVoltageOffsetCube[31:0]    ),
        .oDataValid     (wSqrt3CalculateDone               ),
        .oBusy	        (                                  )
    );

   always @(posedge iSysClk ) begin
        if(wSqrt3CalculateDone)
            rTargetVoltageOffsetCube <= $signed(wTargetVoltageOffsetCube) * 8'sd10;
        else
            rTargetVoltageOffsetCube <= rTargetVoltageOffsetCube;
    end
    //******************************************************************************
     //功率因子 功率过大的时候 均衡就占比很小
     always @(posedge iSysClk) begin
         if(iDCOutputTotalPower[15] == 1'b1)
             rDCOutputTotalPowerAbs[15:0] <= ~iDCOutputTotalPower[15:0] + 1'b1;
         else
             rDCOutputTotalPowerAbs[15:0] <= iDCOutputTotalPower[15:0];
     end
     
     
     always @(posedge iSysClk) begin
         if(rDCOutputTotalPowerAbs > P_MAX)
             rMaxPowerDiffDCOutputTotalPower[15:0] <= 16'd0;
         else
             rMaxPowerDiffDCOutputTotalPower[15:0] <= $signed(P_MAX-rDCOutputTotalPowerAbs[15:0]);
     end


     always@(posedge iSysClk)begin
            rTargetVoltageOffsetPre <= (($signed(rTargetVoltageOffsetCube)*$signed(rMaxPowerDiffDCOutputTotalPower)) 
                                       + ($signed(rTargetVoltageOffsetLin)*$signed(rDCOutputTotalPowerAbs)))>>>12;
     end

    always@(posedge iSysClk)begin
        rTargetVoltageOffsetRealPre <= iSeriesRelayEn ? rTargetVoltageOffsetPre >>>1 : rTargetVoltageOffsetPre;
    end
//   SignedDivision #(   //线性虚拟阻抗 5R 
//         .WITH1      (32                                     ),  // 被除数位宽
//         .WITH2      (16                                     )   // 除数位宽
//     ) U_Division_TargetVoltageOffset (
//         .iSysClk    (iSysClk                                ),
//         .iSysRst    (1'b0                                   ),
//         .iStart     (1'b1                                   ),
//         .iDividend  ($signed(rTargetVoltageOffsetRealPre[31:0])  ),
//         .iDivisor   (P_MAX                                   ), //5R 的阻抗
//         .oQuotient  (wTargetVoltageOffsetReal[31:0]          ),
//         .oRemainder (                                        ),
//         .oDone      (wTargetVoltageOffsetDone                )
//     );
    //限制在±15V
    // always @(posedge iSysClk) begin
    //     if(wTargetVoltageOffsetDone)begin
    //         if(wTargetVoltageOffsetReal < -32'sd300)
    //           rTargetVoltageOffsetReal <= -32'sd300;
    //         else if(wTargetVoltageOffsetReal > 32'sd300)
    //           rTargetVoltageOffsetReal <= 32'sd300;
    //         else
    //           rTargetVoltageOffsetReal <= wTargetVoltageOffsetReal;
    //     end
    //     else
    //         rTargetVoltageOffsetReal <= rTargetVoltageOffsetReal;
    // end
    always @(posedge iSysClk) begin
        if(rTargetVoltageOffsetRealPre < -32'sd300)
          rTargetVoltageOffsetReal <= -32'sd300;
        else if(rTargetVoltageOffsetRealPre > 32'sd300)
          rTargetVoltageOffsetReal <= 32'sd300;
        else
          rTargetVoltageOffsetReal <= rTargetVoltageOffsetRealPre;
    end


 assign oTargetVoltageOffset = $signed(rTargetVoltageOffsetReal) ;
    //******************************************************************************

endmodule
