//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AutoCtrlCore.v
//Original Author   :   lvyb
//Creation Date     :   2024.10.10
/*Description       :   创建自动控制模块。

*/

`timescale 1ns / 1ns

module AutoCtrlCore #(
    parameter CLK_FREQ = 100
) (
    //Global Clock
    input                   iSysClk                     ,
    input                   iLoopCtrlEn                 ,
    input                   iAdcSampleSequenceDone      ,
    input  signed [31:0]    iFeedBackVoltageParam       ,
    input  signed [31:0]    iFeedBackCurrentParam       ,
    input  signed [31:0]    iTargetCurrentParamMax      ,
    input  signed [31:0]    iTargetCurrentParamMin      ,
    input  signed [31:0]    iTargetVoltageOffset        ,
    input  signed [31:0]    iActiveBalanceVoltageOffset , 
    //input  signed [31:0]    iZeroPowerCtrlVoltageOffset ,

    input                   iSeriesRelayEn              ,

    output signed [31:0]    oPidControlPwmParam         ,

    output signed [31:0]    oGiRef                      ,
    output signed [31:0]    oGiI
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
localparam VOL_REF_SERIES           = 32'sd4000     ;
localparam VOL_REF_PARALLEL         = 32'sd4700     ;
//******************************************************************************

//******************************************************************************
//
wire        [1:0]   wPidOpEn                        ;
reg  signed [31:0]  rTargetVoltageParam     = 0     ; 

reg                 rVoltageOuterLoopTrigger        ;  //电压外环计算触发信号
reg                 rCurrentInnerLoopTrigger        ;  //电流内环计算触发信号
reg                 rLoopIntegralReset              ;  //积分重置信号

wire signed [31:0]  wTargetCurrentParam             ;//电压环输出是电流环给定
reg  signed [31:0]  rTargetCurrentParam     = 32'sd0;
wire signed [31:0]  wTargetPwmParam                 ; //电流环输出是PWM给定
reg  signed [31:0]  rTargetPwmParam         = 32'sd0;          

wire                wVoltageOutLoopCalcOk           ;
wire                wCurrentOutLoopCalcOk           ;

wire signed [31:0]  wGiI                            ;  //测试信号

//******************************************************************************
//PID参数
reg   signed[15:0]  VolKp       ;
reg   signed[15:0]  VolKi       ;
wire  signed[15:0]  VolKd       ;
reg   signed[15:0]  CurKp       ;
reg   signed[15:0]  CurKi       ;
wire  signed[15:0]  CurKd       ;

always @(posedge iSysClk) begin
    VolKp <= iSeriesRelayEn ? 16'sd512  : 16'sd1024     ; 
    VolKi <= iSeriesRelayEn ? 16'sd64    : 16'sd32      ;
    CurKp <= iSeriesRelayEn ? 16'sd64    : 16'sd128     ;
    CurKi <= iSeriesRelayEn ? 16'sd1     : 16'sd2       ;
end
assign VolKd = 16'sd0;
assign CurKd = 16'sd0;
//******************************************************************************
//输出电压值 = 参考电压 + 主动均衡电压 + 虚拟阻抗偏置电压
wire   signed  [31:0]      wVolRef;     
assign wVolRef = iSeriesRelayEn ? VOL_REF_SERIES : VOL_REF_PARALLEL;

always @(posedge iSysClk ) begin
    rTargetVoltageParam <= wVolRef + iActiveBalanceVoltageOffset - iTargetVoltageOffset;/*-iZeroPowerCtrlVoltageOffset*/
end

//******************************************************************************

//******************************************************************************
//Pid内环和外环计算频率设定
PidSampling #(
    .CLK_FREQ                   (CLK_FREQ                   )     //MHz
) U_PidSampling (
    .iSysClk                    (iSysClk                    ),
    .iAdcSampleSequenceDone     (iAdcSampleSequenceDone     ),
    .oPidOpEn                   (wPidOpEn[1:0]              )
);

always @(posedge iSysClk ) begin
    if(iLoopCtrlEn)begin
        rVoltageOuterLoopTrigger <= wPidOpEn[0] ;
        rCurrentInnerLoopTrigger <= wPidOpEn[1] ;
    end else begin
        rVoltageOuterLoopTrigger <= 1'b0 ;
        rCurrentInnerLoopTrigger <= 1'b0 ;
    end
end

always @(posedge iSysClk ) begin
    if(iLoopCtrlEn)begin
        rLoopIntegralReset <= 1'b0;
    end else begin
        rLoopIntegralReset <= 1'b1;
    end
end
//******************************************************************************

//******************************************************************************
//电压外环


PidControl #(
    .OUT_GAIN           ($signed(128)                   )
) U0_PidControl (
    .iSysClk            (iSysClk                        ),
    .Kp                 (VolKp                          ),
    .Ki                 (VolKi                          ),
    .Kd                 (VolKd                          ),
    .iPidOpEn           (rVoltageOuterLoopTrigger       ),
    .iStateRst          (rLoopIntegralReset             ),
    .iTargetParam       (rTargetVoltageParam            ),
    .iFeedBackParam     (iFeedBackVoltageParam          ),
    .iPidOutMax         (iTargetCurrentParamMax         ),
    .iPidOutMin         (iTargetCurrentParamMin         ),
    .oPidControlParam   (wTargetCurrentParam            ),
    .oIout              (wGiI                           ),
    .oCalcOk            (wVoltageOutLoopCalcOk          )
);

always @(posedge iSysClk ) begin
    if(wVoltageOutLoopCalcOk)
        rTargetCurrentParam <= wTargetCurrentParam;
    else
        rTargetCurrentParam <= rTargetCurrentParam;
end
//******************************************************************************


//******************************************************************************
//电流内环wPidControlPwmParam

PidControl #(
    .OUT_GAIN           ($signed(8192)                  )
) U1_PidControl (
    .iSysClk            (iSysClk                        ),
    .Kp                 (CurKp                          ),
    .Ki                 (CurKi                          ),
    .Kd                 (CurKd                          ),
    .iPidOpEn           (rCurrentInnerLoopTrigger       ),
    .iStateRst          (rLoopIntegralReset             ),
    .iTargetParam       (rTargetCurrentParam            ),  //$signed({10'd0, wTargetCurrentParam[31:10]})  
    .iFeedBackParam     (iFeedBackCurrentParam          ),
    .iPidOutMax         ($signed(900)                   ),
    .iPidOutMin         ($signed(100)                   ),
    .oPidControlParam   (wTargetPwmParam                ),
    .oIout              (                               ),
    .oCalcOk            (wCurrentOutLoopCalcOk          )
);

always @(posedge iSysClk ) begin
    if(wCurrentOutLoopCalcOk)
        rTargetPwmParam <= wTargetPwmParam;
    else
        rTargetPwmParam <= rTargetPwmParam;
end
//******************************************************************************
assign oPidControlPwmParam = rTargetPwmParam;  // 内环输出经 CalcOk 采样保持

//******************************************************************************

assign oGiRef       = rTargetCurrentParam    ; // 内环电流给定（外环采样保持）
assign oGiI         = rTargetVoltageParam    ; //测试信号
//******************************************************************************


endmodule
