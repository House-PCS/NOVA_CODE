//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   PidControl.v
//Original Author   :   lvyb
//Creation Date     :   2024.10.10
/*Description       :   创建Pid控制模块。

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
module PidControl
#(
    parameter   signed  [15:0]      OUT_GAIN    = 16'sd1    
)
(
    //Global Clock
    input                           iSysClk             ,
    input       signed  [15:0]      Kp                  ,
    input       signed  [15:0]      Ki                  ,
    input       signed  [15:0]      Kd                  ,
    input                           iPidOpEn            ,
    input                           iStateRst           ,
    input       signed  [31:0]      iTargetParam        ,
    input       signed  [31:0]      iFeedBackParam      ,
    input       signed  [31:0]      iPidOutMax          ,
    input       signed  [31:0]      iPidOutMin          ,   
    output      signed  [31:0]      oPidControlParam    ,

    output      signed  [31:0]      oIout               ,
    output                          oCalcOk
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//根据输出结果计算积分最大最小值
wire                        wDataOutValidMax        ;
wire                        wDataOutValidMin        ;
wire    signed  [31:0]      wIntegralMax            ;   //商
wire    signed  [31:0]      wIntegralMin            ;
wire    signed  [31:0]      wResultMax              ;   // 被除数
wire    signed  [31:0]      wResultMin              ;   // 被除数
wire    signed  [15:0]      wRealKi                ;   // 除数

reg     signed  [31:0]      rIntegralParamMax       ;
reg     signed  [31:0]      rIntegralParamMin       ;

assign wRealKi[15:0]       = Ki == 0 ? 1 : Ki;

assign wResultMax[31:0]   = $signed(iPidOutMax*OUT_GAIN);
assign wResultMin[31:0]   = $signed(iPidOutMin*OUT_GAIN);

//******************************************************************************
// 计算积分溢出值的上下限

SignedDivision #(
    .WITH1      (32                     ),  // 被除数位宽
    .WITH2      (16                     )   // 除数位宽
) U_Division_Max (
    .iSysClk    (iSysClk                ),
    .iSysRst    (0                      ),
    .iStart     (1                      ),
    .iDividend  (wResultMax             ),
    .iDivisor   (wRealKi                ),
    .oQuotient  (wIntegralMax           ),
    .oRemainder (                       ),
    .oDone      (wDataOutValidMax       ),
    .oBusy      (                       )
);

SignedDivision #(
    .WITH1(32),  // 被除数位宽
    .WITH2(16)   // 除数位宽
) U_Division_Min (
    .iSysClk   (iSysClk                 ),
    .iSysRst   (0                       ),
    .iStart    (1                       ),
    .iDividend (wResultMin              ),
    .iDivisor  (wRealKi                 ),
    .oQuotient (wIntegralMin            ),
    .oRemainder(                        ),
    .oDone     (wDataOutValidMin        ),
    .oBusy     (                        )
);

always @(posedge iSysClk ) begin
    if(wDataOutValidMax)begin
        rIntegralParamMax <= wIntegralMax ;
    end
end
always @(posedge iSysClk ) begin
    if(wDataOutValidMin)begin
        rIntegralParamMin <= wIntegralMin ;
    end
end

//******************************************************************************

//******************************************************************************
//

    reg                 [6:0]       rPidOpEnBuf         = 7'd0              ;
    reg     signed      [31:0]      rFeedBackParam      = 32'sd0             ;
    reg     signed      [31:0]      rTargetParam        = 32'sd0             ;
    reg     signed      [31:0]      rPidError           = 32'sd0             ;
    reg     signed      [31:0]      rPidErrorLast        = 32'sd0             ;
    reg     signed      [31:0]      rPidErrorSumBuf     = 32'sd0             ;
    reg     signed      [31:0]      rPidErrorSum        = 32'sd0             ;
    reg     signed      [31:0]      rPidErrorDiffBuf    = 32'sd0             ;
    reg     signed      [31:0]      rPidErrorDiff       = 32'sd0             ;
    reg     signed      [31:0]      rPidControlParamBuf = 32'sd0             ;
    reg     signed      [31:0]      rPidControlParam    = 32'sd0             ;

//******************************************************************************

//******************************************************************************
//产生不同的脉冲信号 分布在不同的节拍进行计算

assign oIout = rPidErrorSum;

always @ (posedge iSysClk) begin
    rPidOpEnBuf[6:0] <= {rPidOpEnBuf[5:0], iPidOpEn};
end

//******************************************************************************

//******************************************************************************
//求误差
always @ (posedge iSysClk) begin
    if(rPidOpEnBuf[0]) begin
        rPidError <= iTargetParam - iFeedBackParam;
    end else begin
        rPidError <= rPidError;
    end
end

//******************************************************************************
//积分
always @ (posedge iSysClk) begin
    if(!iStateRst) begin
        if(rPidOpEnBuf[1])
            rPidErrorSumBuf <= rPidErrorSum + rPidError;
        else
            rPidErrorSumBuf <= rPidErrorSumBuf;
    end else begin
        rPidErrorSumBuf <= 32'sd0;
    end
end

//******************************************************************************
//微分
always @ (posedge iSysClk) begin
    if(rPidOpEnBuf[1])
        rPidErrorDiffBuf <= rPidError - rPidErrorLast;
    else
        rPidErrorDiffBuf <= rPidErrorDiffBuf;
end

//******************************************************************************
//防止积分溢出功能
always @ (posedge iSysClk) begin
    if(rPidOpEnBuf[2]) begin
        if(rPidErrorSumBuf > rIntegralParamMax)
            rPidErrorSum <= rIntegralParamMax;
        else if(rPidErrorSumBuf < rIntegralParamMin)
            rPidErrorSum <= rIntegralParamMin;
        else
            rPidErrorSum <= rPidErrorSumBuf;
    end
    else
        rPidErrorSum <= rPidErrorSum;
end

//******************************************************************************
//可以做防止微分差值过大
always @ (posedge iSysClk) begin
    if(rPidOpEnBuf[2])
        rPidErrorDiff <= rPidErrorDiffBuf;
    else
        rPidErrorDiff <= rPidErrorDiff;
end

//******************************************************************************
//历史数值更新
always @ (posedge iSysClk) begin
    if(rPidOpEnBuf[3])
        rPidErrorLast <= rPidError;
    else
        rPidErrorLast <= rPidErrorLast;
end

//******************************************************************************

//******************************************************************************
//
wire signed [31:0] wPidResult = ((Kp * rPidError) + (Ki * rPidErrorSum) + (Kd * rPidErrorDiff));

always @ (posedge iSysClk) begin
    if(rPidOpEnBuf[4])
        rPidControlParamBuf <= wPidResult ;
    else
        rPidControlParamBuf <= rPidControlParamBuf;
end

//******************************************************************************

//******************************************************************************
//
wire                wPidControlParamDoutValid   ;
wire signed [31:0]  wPidControlParamDoutData    ;
reg  signed [31:0]  wPidControlParamDoutDataBuf ;
SignedDivision #(
    .WITH1      (32                                 ),  // 被除数位宽
    .WITH2      (16                                 )   // 除数位宽
) U_Division_Gain (
    .iSysClk    (iSysClk                            ),
    .iSysRst    (0                                  ),
    .iStart     (rPidOpEnBuf[5]                     ),
    .iDividend  (rPidControlParamBuf                ),
    .iDivisor   (OUT_GAIN                           ),
    .oQuotient  (wPidControlParamDoutData[31:0]     ),
    .oRemainder (                                   ),
    .oDone      (wPidControlParamDoutValid          ),
    .oBusy      (                                   )
);

//******************************************************************************

always @(posedge iSysClk ) begin
    if(wPidControlParamDoutValid)
       wPidControlParamDoutDataBuf <= wPidControlParamDoutData;
    else
       wPidControlParamDoutDataBuf <= wPidControlParamDoutDataBuf;
end

reg [1:0] rPidControlParamValid = 2'd0;
always @(posedge iSysClk ) begin
    rPidControlParamValid[1:0] <= {rPidControlParamValid[0], wPidControlParamDoutValid};
end

//******************************************************************************
//结果最大值限制
always @ (posedge iSysClk) begin
    if(wPidControlParamDoutDataBuf > $signed(iPidOutMax))
        rPidControlParam <= $signed(iPidOutMax) ;
    else if(wPidControlParamDoutDataBuf < $signed(iPidOutMin)) 
        rPidControlParam <= $signed(iPidOutMin) ;
    else
        rPidControlParam <= wPidControlParamDoutDataBuf;
end

//******************************************************************************
//
assign oPidControlParam = rPidControlParam      ;
assign oCalcOk          = rPidControlParamValid[1]     ;
//******************************************************************************

endmodule
