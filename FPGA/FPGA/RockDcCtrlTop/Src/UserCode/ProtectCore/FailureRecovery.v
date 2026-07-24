//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   FailureRecovery.v
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

module FailureRecovery#(
    parameter          FAILURE_RECOVERY_DELAY        =    5      ,  //故障恢复延时（秒）
    parameter          FAILURE_RECOVERY_TIMES        =    5      ,  //周期内最大允许故障次数
    parameter          FAILURE_STATISTICAL_PERIOD    =    255       //统计周期，单位秒  
)
(
    //Global Clock
    input   wire                iSysClk                     ,
    input   wire                iDcdcErr                    ,
    input   wire                iSysClk1SPulse                      ,
    output  wire                oFailRecovery               
);
function integer clogb2 ( input integer depth ) ;
begin
    for ( clogb2 = 0 ; depth > 0 ; clogb2 = clogb2 + 1 ) begin 
        depth = depth >> 1 ;
    end
end
endfunction

localparam RECOVERY_DELAY_WIDE        = clogb2(FAILURE_RECOVERY_DELAY);
localparam RECOVERY_TIMES_WIDE        = clogb2(FAILURE_RECOVERY_TIMES);
localparam STATISTICAL_PERIOD_WIDE    = clogb2(FAILURE_STATISTICAL_PERIOD);
//******************************************************************************
    reg                  [    RECOVERY_DELAY_WIDE-1: 0]   rCntRecoveryDelay        ='d0;
    reg                                                   rCntStatisticalPeriodEn  ='d0;
    reg                  [STATISTICAL_PERIOD_WIDE-1: 0]   rCntStatisticalPeriod    ='d0;
    reg                  [                        1: 0]   rDcdcErr                 ='d0;
    reg                  [    RECOVERY_TIMES_WIDE-1: 0]   rCntRecoveryTimes        ='d0;
    reg                                                   rRecoveryDisEn           ='d0;
    reg                                                   rFailRecovery            ='d0;
//******************************************************************************
always @(posedge iSysClk) begin
    rDcdcErr[1:0] <= {rDcdcErr[0],iDcdcErr};
end
//============================================================================
// 故障延时恢复计数器（检测到故障后开始计时）
//============================================================================
always @ (posedge iSysClk)  begin
    if(rDcdcErr[1:0]==2'b01)begin
        rCntRecoveryDelay <=  'd0;
    end
    else if(iDcdcErr)begin
        if(iSysClk1SPulse )begin
          if(rCntRecoveryDelay == (FAILURE_RECOVERY_DELAY))
              rCntRecoveryDelay <= rCntRecoveryDelay;
          else
              rCntRecoveryDelay <= rCntRecoveryDelay + 1'b1;
        end
        else
          rCntRecoveryDelay <= rCntRecoveryDelay;
    end
    else
        rCntRecoveryDelay <= 'd0;
end
//============================================================================
// 恢复输出脉冲生成（延时到达后输出单周期高脉冲）
//============================================================================
always @(posedge iSysClk) begin
    if(rRecoveryDisEn && (FAILURE_RECOVERY_TIMES != 0))
      rFailRecovery <= 1'b0;
    else if((rCntRecoveryDelay == (FAILURE_RECOVERY_DELAY-1)))
      rFailRecovery <= 1'b1;
    else
      rFailRecovery <= 1'b0;
end

assign  oFailRecovery = rFailRecovery;
//============================================================================
// 故障次数统计 + 超限锁定
//============================================================================
always@(posedge iSysClk) begin
    if((rCntStatisticalPeriod == FAILURE_STATISTICAL_PERIOD) && iSysClk1SPulse)
      rCntStatisticalPeriodEn <= 1'b0;
    else if(rDcdcErr[1:0]==2'b01)
      rCntStatisticalPeriodEn <= 1'b1;
    else
      rCntStatisticalPeriodEn <= rCntStatisticalPeriodEn;
end

always @(posedge iSysClk) begin
    if(rCntStatisticalPeriodEn)begin
      if(iSysClk1SPulse)
        rCntStatisticalPeriod <= rCntStatisticalPeriod + 1'b1;
      else
        rCntStatisticalPeriod <= rCntStatisticalPeriod;
    end
    else
      rCntStatisticalPeriod <= 'd0;
end 

always @(posedge iSysClk) begin
    if(iSysClk1SPulse && (rCntStatisticalPeriod == FAILURE_STATISTICAL_PERIOD))
      rCntRecoveryTimes <= 'd0; 
    else if(rDcdcErr[1:0]==2'b01)begin
      if(rCntRecoveryTimes == FAILURE_RECOVERY_TIMES)
        rCntRecoveryTimes <= FAILURE_RECOVERY_TIMES;
      else
        rCntRecoveryTimes <= rCntRecoveryTimes + 1'b1;  
    end   
end

always @(posedge iSysClk) begin
    if(rCntRecoveryTimes >= FAILURE_RECOVERY_TIMES && rDcdcErr[1:0]==2'b01)
      rRecoveryDisEn <= 1'b1;
    else
      rRecoveryDisEn <= rRecoveryDisEn;      
end
endmodule



