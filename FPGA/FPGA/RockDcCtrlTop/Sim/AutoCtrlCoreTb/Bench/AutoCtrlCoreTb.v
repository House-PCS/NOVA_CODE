//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AutoCtrlCoreTb.v
//Original Author   :   lvyb
//Creation Date     :   2024.10.10
/*Description       :   创建自动PID控制模块仿真。

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
module AutoCtrlCoreTb;

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    parameter                       PERIOD_SYS                  = 10     ;

//******************************************************************************

//******************************************************************************
//
    reg                             rSysClk                     = 1'b0  ;
    wire    signed  [23:0]          wTargetVoltageParam                 ;
    wire    signed  [23:0]          wFeedBackVoltageParam               ;
    wire    signed  [23:0]          wFeedBackCurrentParam               ;
    wire    signed  [23:0]          wPidControlPwmParam                 ;       //只取高14bit作为占空比使用
    wire    signed  [9:0]           wPidControlPwmDuty                  ;
    wire    signed  [23:0]          wVoltageOut                         ;
    wire    signed  [23:0]          wTargetVoltage                      ;
    reg     signed  [23:0]          rTargetVoltage              = 24'd0 ;
    wire    signed  [23:0]          wCurrentOut                         ;
    wire    signed  [23:0]          wResistanceOut                      ;
    reg     signed  [23:0]          rResistanceOut                      ;
    wire    signed  [23:0]          wVoltageIn                          ;

//******************************************************************************

//******************************************************************************
//

initial begin
    forever # (PERIOD_SYS/2)
        rSysClk = ~rSysClk;
end

//******************************************************************************

//******************************************************************************
//电阻参数和输入电压参数

assign wResistanceOut[23:0] = rResistanceOut[23:0];
assign wVoltageIn[23:0] = $signed(256);

//******************************************************************************

//******************************************************************************
//占空比和输出电压
//占空比与Vout关系为：Vout = Vin/(1 - Duty)
//等效Vout = 1024*Vin/(1024 - 1024*Duty)
assign wPidControlPwmDuty[9:0] = wPidControlPwmParam[19:10];
assign wVoltageOut[23:0] = wVoltageIn[23:0]*$signed(1024)/($signed(1024) - wPidControlPwmDuty[9:0]);

//******************************************************************************

//******************************************************************************
//输出电流
//等效1000*Vout/R，单位毫安
assign wCurrentOut[23:0] = wVoltageOut[23:0]*$signed(1000)/wResistanceOut[23:0];

//******************************************************************************

//******************************************************************************
//反馈电流参数
//等效Iout*58.7+2048
//等效(58 + 0.5)*Vout/R + 2048
assign wFeedBackCurrentParam[23:0] = ($signed(58)*wVoltageOut[23:0] + wVoltageOut[23:1])/wResistanceOut[23:0] + $signed(2048);

//******************************************************************************

//******************************************************************************
//
//assign wFeedBackVoltageParam[23:0] = (wVoltageOut[23:0]*1.5/(43 + 470*7) + 0.5)*4096; 
//等效1.843*Vout + 2048
//等效1.843*1024/(1024*Vout) + 2048 
//等效1887/(1024*Vout) + 2048
assign wFeedBackVoltageParam[23:0]  = wVoltageOut[23:0]*$signed(1887)/$signed(1024) + $signed(2048);
assign wTargetVoltageParam[23:0]    = wTargetVoltage[23:0]*$signed(1887)/$signed(1024) + $signed(2048);

assign wTargetVoltage[23:0] = rTargetVoltage[23:0];

//******************************************************************************

//******************************************************************************
//

initial begin
    rTargetVoltage[23:0] = $signed(400)     ;
    rResistanceOut[23:0] = $signed(200)     ;

    #50000000;
    rTargetVoltage[23:0] = $signed(400)     ;
    rResistanceOut[23:0] = $signed(100)     ;
    
    #50000000;
    rTargetVoltage[23:0] = $signed(500)     ;
    rResistanceOut[23:0] = $signed(300)     ;

    #50000000;
    rTargetVoltage[23:0] = $signed(400)     ;
    rResistanceOut[23:0] = $signed(100)     ;

    #50000000;
    rTargetVoltage[23:0] = $signed(300)     ;
    rResistanceOut[23:0] = $signed(200)     ;
end


//******************************************************************************

//******************************************************************************
//

AutoCtrlCore 
#(
    .CLK_FREQ                       (100                                )
)U_AutoCtrlCore 
(
    .iSysClk                        (rSysClk                            ),
    .iTargetVoltageParam            (wTargetVoltageParam[23:0]          ),
    .iFeedBackVoltageParam          (wFeedBackVoltageParam[23:0]        ),
    .iFeedBackCurrentParam          (wFeedBackCurrentParam[23:0]        ),
    .oPidControlPwmParam            (wPidControlPwmParam[23:0]          ) 
);

//******************************************************************************

endmodule
