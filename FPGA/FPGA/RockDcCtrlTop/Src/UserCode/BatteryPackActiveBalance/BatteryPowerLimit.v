//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   BatteryPackPowerLimit.v
//Original Author   :   xcy Nova003107
//Creation Date     :   
/*Description       :   电池充放电功率限制
                        0.输入信号是电池电流，输出信号是控制环路电流
                        1.电池包 iChgMax iDsgMax 单位是0.1A/bit，DCDC采样单位是1mA/bit，数学上相当于放大了100倍
                        2.变压器匝比是11:1，相当于电流缩小了11倍，由于是两路LLC相当于缩小了22倍
                        3.整体放大倍数是 G = 100/22 = 4.45倍

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

module BatteryPowerLimit#(
    parameter CLK_FREQ =    100 
)(
    input                     iSysClk                   ,
    input                     iWorkingEn                ,
    input            [15:0]   iBatSoc                   ,
    input                     iSeriesRelayEn            ,
    input  signed    [31:0]   iBatCurrent               ,
    input  signed    [31:0]   iChgMax                   ,
    input  signed    [31:0]   iDsgMax                   ,
    output signed    [31:0]   oTargetCurrentParamMax    ,
    output signed    [31:0]   oTargetCurrentParamMin    
    //output signed    [31:0]   oZeroPowerCtrlVoltageOffset
);

localparam  CHG_MAX             = -32'sd9000    ;
localparam  CHG_MIN             = -32'sd300     ;
localparam  SOFT_START_CHG_MIN  = -32'sd500     ;

localparam  DSG_MAX             = 32'sd9000     ;
localparam  DSG_MIN             = 32'sd1500     ;
localparam  SOFT_START_DSG_MIN  = 32'sd500      ;

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
// parameter
reg signed [31:0]   rChgMax             ;//电感电流
reg signed [31:0]   rDsgMax             ;//电感电流

reg signed [31:0]   rGiRefMax           ;
reg signed [31:0]   rGiRefMin           ;

reg signed [31:0]   rZeroPowerCtrlVoltageOffset      ;

//******************************************************************************


//******************************************************************************
//电池充放电最大电流限制
always @(posedge iSysClk) begin //充电倍率5
    if(iChgMax >= 0 && iChgMax < 32'sd10000) begin
        rChgMax <= (iChgMax *5) * (-32'sd1) ;
    end else begin
        rChgMax <= rChgMax;
    end
end
always @(posedge iSysClk) begin  //放电倍率需要提升到5.5
    if(iDsgMax >= 0 && iDsgMax < 32'sd10000) begin
        rDsgMax <= (iDsgMax *5) + (iDsgMax >>> 1);
    end else begin
        rDsgMax <= rDsgMax;
    end
end
//******************************************************************************

//******************************************************************************
//8A 充电电流限制 1A启动电流限制和最小电流限制
//SOC 1000的时候 系统充满，此时需要限制充电电流为0（只能放）
always @(posedge iSysClk ) begin
    if(iWorkingEn)begin
        // if(iBatSoc >= 16'd1000)begin
        //     rGiRefMin <= 32'd0;
        // end else begin
            if(rChgMax < CHG_MAX)
                rGiRefMin <= CHG_MAX;
            else if(rChgMax > CHG_MIN)
                rGiRefMin <= CHG_MIN;
            else
                rGiRefMin <= rChgMax;
        // end
    end
    else begin
        rGiRefMin <= SOFT_START_CHG_MIN;
    end
end
//******************************************************************************


//******************************************************************************
//8A 放电电流限制 1A启动电流限制和最小电流限制
//SOC 0的时候 系统放空，此时需要限制放电电流为0（只能充）
always @(posedge iSysClk ) begin
    if(iWorkingEn)begin
        // if(iBatSoc == 16'd0)begin
        //     rGiRefMax <= 32'd0;
        // end else begin
            if(rDsgMax > DSG_MAX)
                rGiRefMax <= DSG_MAX;
            else if(rDsgMax < DSG_MIN)
                rGiRefMax <= DSG_MIN;
            else
                rGiRefMax <= rDsgMax;
        // end
    end 
    else begin
        rGiRefMax <= SOFT_START_DSG_MIN  ;
    end    
end
//******************************************************************************

assign  oTargetCurrentParamMax = rGiRefMax ;
assign  oTargetCurrentParamMin = rGiRefMin ;
//******************************************************************************


//******************************************************************************
//BMS 反馈的充电电流为-，单位是0.1A
//当充满电的时候，充电Max=0，如果还有充电电流，就进行抬升电压，

always @(posedge iSysClk ) begin
    if(iChgMax == 32'sd0)begin
        if(iBatCurrent <= -32'sd100)
            rZeroPowerCtrlVoltageOffset <= -32'sd100;
        else
            rZeroPowerCtrlVoltageOffset <= iBatCurrent;
    end else begin
        rZeroPowerCtrlVoltageOffset <= 32'sd0;
    end
end

//assign  oZeroPowerCtrlVoltageOffset = 32'sd0;//rZeroPowerCtrlVoltageOffset;
//******************************************************************************
endmodule
