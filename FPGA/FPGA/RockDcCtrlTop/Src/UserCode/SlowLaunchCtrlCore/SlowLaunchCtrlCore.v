//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   SlowLaunchCtrlCore.v
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

`timescale 1ns/1ns

module SlowLaunchCtrlCore#(
    parameter          CLK_FREQ                 =    100 ,  //MHz
    parameter          SLOW_LAUNCH_TIME         =    1024*6   //ms               
)
(
    //Global Clock
    input              iSysClk                    ,
    input              iSlowLaunchEn              ,
    input              iCnt1Ms                    ,
    output             oSlowLaunchEnd             ,
    output     [9:0]   oPwmDuty                    //LLC 0-3FF（0-1023） --> 0-50% 
);

//******************************************************************************
// 缓慢启动，LLC占空比慢慢变高
//******************************************************************************

localparam SLOW_LAUNCH_STEP        = SLOW_LAUNCH_TIME/1024 ;

reg [10:0]  rCountStep             = 'd0  ;
reg [9:0]   rSlowLaunchRomAddr     = 'd0  ;

reg         rSlowLaunchEnd         = 'd0  ;
reg [9:0]   rPwmDuty               = 'd0  ;



wire        wCountStep      ;
wire        wSlowLaunchEnd  ;
wire [9:0]  wPwmDuty        ;

//******************************************************************************

//******************************************************************************
//ms计数器

always @ (posedge iSysClk) begin
    if(iCnt1Ms) begin
        if(rCountStep[10:0] == (SLOW_LAUNCH_STEP - 11'd1))
            rCountStep[10:0] <= 11'd0;
        else
            rCountStep[10:0] <= rCountStep[10:0] + 11'd1;
    end
    else
        rCountStep[10:0] <= rCountStep[10:0];
end
assign wCountStep = (iCnt1Ms && (rCountStep[10:0] == (SLOW_LAUNCH_STEP - 11'd1)));

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(iSlowLaunchEn == 1'b0)
        rSlowLaunchRomAddr[9:0] <= 10'd0;
    else begin
        if(wCountStep) begin
            if(rSlowLaunchRomAddr[9:0] == 10'h3FF)
                rSlowLaunchRomAddr[9:0] <= 10'h3FF;
            else
                rSlowLaunchRomAddr[9:0] <= rSlowLaunchRomAddr[9:0] + 10'd1;
        end
        else
            rSlowLaunchRomAddr[9:0] <= rSlowLaunchRomAddr[9:0];
    end
end

assign wSlowLaunchEnd = (rSlowLaunchRomAddr[9:0] == 10'h3FF);
//******************************************************************************

Srom_1024x10 U_SlowLaunchRom (
  .clka (iSysClk                    ),   // input wire clka
  .ena  (1                          ),   // input wire ena iSlowLaunchEn
  .addra(rSlowLaunchRomAddr[9:0]    ),   // input wire [9 : 0] addra
  .douta(wPwmDuty[9:0]              )    // output wire [9 : 0] douta
);


always @ (posedge iSysClk) begin
    rSlowLaunchEnd <=  wSlowLaunchEnd;
    rPwmDuty[9:0]  <=  wPwmDuty[9:0] ;
end

assign oSlowLaunchEnd = rSlowLaunchEnd;
assign oPwmDuty[9:0]  = rPwmDuty[9:0];

endmodule