//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   BatteryPackActiveBalance.v
//Original Author   :   xcy Nova003107
//Creation Date     :   
/*Description       :   电池包之间的主动均衡，受到监控卡的指令
                        0、能根据自身电池电压（BMS上传0.01V）和平均电压（0.01V）的差值自动调整输出口的电压
                        1、能根据监控的指令（SOC超过5%）监控会发送均衡充电还是均衡放电

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

module BatteryPackActiveBalance #(
    parameter CLK_FREQ = 100  
    )(
    input                       iSysClk                         ,
    input   signed   [15:0]     iEqualizationEnable             ,
    input            [31:0]     iBatVoltage                     ,
    input            [31:0]     iBatVoltageAverage              ,              
    input   signed   [15:0]     iDCOutputTotalPower             ,
    input            [15:0]     iBatSoc                         , 
`ifdef SIM         
    output  signed   [31:0]     oBalanceVoltageRealPre          ,
`endif
    output  signed   [31:0]     oActiveBalanceVoltageOffset    
    );
//-------------------------------Code Start Here--------------------------------
localparam  P_MAX        = 16'sd4400 ;
localparam  BAT_VOLTAGE  = 16'sd2560 ; //3.2V *8 *100  单位0.01V
//******************************************************************************
// 
reg  signed  [14:0]  rActiveBalanceVoltageOffset     = 15'sd0    ;
reg  signed  [14:0]  rBatAutoBalanceVoltageOffset    = 15'sd0    ;
reg  signed  [14:0]  rBatAutoBalanceAvrVoltageOffset = 15'sd0    ;
reg          [15:0]  rDCOutputTotalPowerAbs          = 16'd0     ;
reg  signed  [15:0]  rMaxPowerDiffDCOutputTotalPower = 16'sd0    ;
reg  signed  [15:0]  rBalanceVoltageOffsetSum        = 16'sd0    ;
reg  signed  [31:0]  rBalanceVoltageRealPre          = 32'sd0    ;
wire signed  [31:0]  wBalanceVoltageReal                         ;
wire                 wDivisionCalculateDone                      ;
reg  signed  [31:0]  rActiveBalanceVoltageOffsetReal = 32'sd0    ;
//******************************************************************************

//******************************************************************************
//主动电池包之间指令均衡，监控下发指令进行电压抬升或者降低 5V 10V
always @(posedge iSysClk ) begin
    case($signed(iEqualizationEnable[15:0])) 
        16'sd1  :   rActiveBalanceVoltageOffset <=  -15'sd100       ; //均衡充电时，电压参考值减5V
        16'sd2  :   rActiveBalanceVoltageOffset <=  -15'sd200       ;
        -16'sd1 :   rActiveBalanceVoltageOffset <=  +15'sd100       ; //均衡放电时，电压参考值加5V
        -16'sd2 :   rActiveBalanceVoltageOffset <=  +15'sd200       ;  
        default :   rActiveBalanceVoltageOffset <=   15'sd0         ; //正常电压
    endcase
end
//******************************************************************************


    //assign oActiveBalanceVoltageOffset = $signed (rActiveBalanceVoltageOffset);

//******************************************************************************
//均衡电压偏置 = ( (最大功率-实际功率）/最大功率 ) * （监控卡均衡电压偏置 + 电池包自动电压偏置）  
//            = (最大功率-实际功率） * （监控卡均衡电压偏置 + 电池包自动电压偏置）/最大功率      备注：(为了避免大功率充电时还叠加了均衡而导致BMS过流，分子的最大功率取3Kw；
//                                                                                                为了提高均衡效率，分母的最大功率取2048)
//            = (3000-实际功率） * （监控卡均衡电压偏置 + 电池包自动电压偏置）/2048
//             = (3000-实际功率） * （监控卡均衡电压偏置 + 电池包自动电压偏置）>>>11

//******************************************************************************
//电池内阻引起的电压 100A - 0.25V
//4000--0.5; 4000 - 4V 
//放电为正
// reg signed  [15:0] rBatResVoltage = 16'sd0 ;

// always @(posedge iSysClk ) begin
//     rBatResVoltage <= iDCOutputTotalPower >>> 10;
// end

//******************************************************************************
//电池电压单位是0.01V DCDC单位是0.1V
//如果电芯电压相差0.1V，DCDC高压侧要相差1V
always @(posedge iSysClk) begin
    if(iBatVoltage == 32'sd0)begin
        rBatAutoBalanceVoltageOffset <= 15'sd0;   
    end
    else begin
        rBatAutoBalanceVoltageOffset <= ($signed(iBatVoltage - BAT_VOLTAGE));//15'sd0;//
    end
end

//电池电压单位是0.01V DCDC单位是0.1V
//如果电芯电压相差0.1V，DCDC高压侧要相差1V
always @(posedge iSysClk) begin
    if((iBatVoltageAverage == 32'sd0) || (iBatSoc < 'd990))begin
        rBatAutoBalanceAvrVoltageOffset <= 15'sd0;
    end
    else begin
        rBatAutoBalanceAvrVoltageOffset <= ($signed(iBatVoltage - iBatVoltageAverage))<<<2;//15'sd0;
    end
end
//******************************************************************************

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

//******************************************************************************

always @(posedge iSysClk) begin
    rBalanceVoltageOffsetSum[15:0] = $signed(rBatAutoBalanceVoltageOffset + rActiveBalanceVoltageOffset + rBatAutoBalanceAvrVoltageOffset);
end

always @(posedge iSysClk) begin
    rBalanceVoltageRealPre[31:0] <= ($signed(rMaxPowerDiffDCOutputTotalPower)*$signed(rBalanceVoltageOffsetSum));//>>>12;
end

 
SignedDivision #(   //线性虚拟阻抗 5R 
    .WITH1      (32                                     ),  // 被除数位宽
    .WITH2      (16                                     )   // 除数位宽
) U_Division_BalanceVoltage (
    .iSysClk    (iSysClk                                ),
    .iSysRst    (1'b0                                   ),
    .iStart     (1'b1                                   ),
    .iDividend  ($signed(rBalanceVoltageRealPre[31:0])  ),
    .iDivisor   (P_MAX                                  ), //5R 的阻抗
    .oQuotient  (wBalanceVoltageReal[31:0]              ),
    .oRemainder (                                       ),
    .oDone      (wDivisionCalculateDone                 )
);
//******************************************************************************


//******************************************************************************
//均衡电压限制 限制在±15V
always @(posedge iSysClk) begin
    if(wDivisionCalculateDone)begin
        if(wBalanceVoltageReal < -32'sd400)
          rActiveBalanceVoltageOffsetReal <= -32'sd400;
        else if(wBalanceVoltageReal > 32'sd400)
          rActiveBalanceVoltageOffsetReal <= 32'sd400;
        else
          rActiveBalanceVoltageOffsetReal <= wBalanceVoltageReal;
    end
    else  
        rActiveBalanceVoltageOffsetReal <= rActiveBalanceVoltageOffsetReal; 
end
//******************************************************************************

assign  oActiveBalanceVoltageOffset = rActiveBalanceVoltageOffsetReal;

// `ifdef SIM
//    assign  oBalanceVoltageRealPre = rBalanceVoltageRealPre;
// `endif
endmodule









