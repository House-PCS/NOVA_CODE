//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   PwmTransferCore.v
//Original Author   :   lvyb
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
module PwmTransferCore
(
    //Global Clock
    input                           iSysClk                                 ,//时钟2000MHz
    input                           iSysRst                                 ,
    input                           iSscClk                                 ,
    input                           iLLCPwmEn                               ,
    input                           iBuckBoostPwmEn                         ,
    input          [9:0]            iBuckBoost1PwmDuty                      ,
    input          [9:0]            iBuckBoost2PwmDuty                      ,
    input          [9:0]            iLLC1PwmDuty                            ,
    input          [9:0]            iLLC2PwmDuty                            ,

    output                          oBuckBoost1PwmOutP                      ,
    output                          oBuckBoost1PwmOutN                      ,
    output                          oBuckBoost2PwmOutP                      ,
    output                          oBuckBoost2PwmOutN                      ,

    output                          oLLC1LVPwm1OutP                         ,
    output                          oLLC1LVPwm2OutP                         ,
    output                          oLLC1HVPwm1OutP                         ,
    output                          oLLC1HVPwm2OutP                         ,

    output                          oLLC2LVPwm1OutP                         ,
    output                          oLLC2LVPwm2OutP                         ,
    output                          oLLC2HVPwm1OutP                         ,
    output                          oLLC2HVPwm2OutP                         
    
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
// parameter

localparam  BUCKBOOST_PWM_CYCLE     =  200000/35    ;//16'd5000   ; // 200Mhz/5000 = 40Khz
localparam  BUCKBOOST_DEAD_CYCLE    =  8'd150       ; // 5ns*150 = 750ns

localparam  LLC1_PWM_CYCLE          =  200000/55    ;
localparam  LLC1_DEAD_CYCLE         =  9'd150       ; // 150*5ns = 750ns
localparam  LLC2_PWM_CYCLE          =  200000/55    ; // 200Mhz/3078 = 65Khz
localparam  LLC2_DEAD_CYCLE         =  9'd150       ; // 150*5ns = 750ns

localparam  LV_DELAY                =  8'd89        ; // 底边进行延迟 89-20 = 69
localparam  HV_DELAY                =  8'd20        ; // 

//******************************************************************************

 wire          wLLC1Pwm1P                       ;
 wire          wLLC1Pwm2P                       ;
 wire          wLLC2Pwm1P                       ;
 wire          wLLC2Pwm2P                       ;

 reg [127 :0]  rLLC1Pwm1PBuf     = 128'd0       ;
 reg [127 :0]  rLLC1Pwm2PBuf     = 128'd0       ;
 reg [127 :0]  rLLC2Pwm1PBuf     = 128'd0       ;
 reg [127 :0]  rLLC2Pwm2PBuf     = 128'd0       ;

 reg [9:0] rBuckBoost1PwmDuty;
 reg [9:0] rBuckBoost2PwmDuty;

//******************************************************************************

//******************************************************************************
//
always @(posedge iSysClk ) begin
    rBuckBoost1PwmDuty[9:0] <= iBuckBoost1PwmDuty[9:0];
    rBuckBoost2PwmDuty[9:0] <= iBuckBoost2PwmDuty[9:0];
end

//******************************************************************************


//******************************************************************************
//第一通道BuckBoost
PwmGenBuckBoost #(
     .PWM_CYCLE              (BUCKBOOST_PWM_CYCLE                    ),  //Max : 1000~65535  200KHz ~ 3052Hz
     .DEAD_CYCLE             (BUCKBOOST_DEAD_CYCLE                   ),  //Time: 0-256 *5ns
     .COUNTER_PHASE          (9'd0)//9'd180                                 )
  )U_PwmGenBuckBoost1
  (
     .iSysClk                (iSscClk                                ),
     .iSysRst                (iSysRst                                ),
     .iPwmEn                 (iBuckBoostPwmEn                        ),
     .iPwmDuty               (rBuckBoost1PwmDuty[9:0]                ),
     .oPwmPSignal            (oBuckBoost1PwmOutP                     ),
     .oPwmNSignal            (oBuckBoost1PwmOutN                     )
  );

//******************************************************************************
//第二通道BuckBoost
PwmGenBuckBoost #(
     .PWM_CYCLE              (BUCKBOOST_PWM_CYCLE                   ),  //Max : 1000~65535  200KHz ~ 3052Hz
     .DEAD_CYCLE             (BUCKBOOST_DEAD_CYCLE                  ),  //Time: 0-256 *5ns
     .COUNTER_PHASE          (9'd0                                  )
  )U_PwmGenBuckBoost2
  (
     .iSysClk                (iSscClk                               ),
     .iSysRst                (iSysRst                               ),
     .iPwmEn                 (iBuckBoostPwmEn                       ),
     .iPwmDuty               (rBuckBoost2PwmDuty[9:0]               ),
     .oPwmPSignal            (oBuckBoost2PwmOutP                    ),
     .oPwmNSignal            (oBuckBoost2PwmOutN                    )
  );

//******************************************************************************
//LLC1和LLC相位偏移90度，减小纹波电流

PwmGenLLCNew #(
    .PWM_CYCLE       (LLC1_PWM_CYCLE      ), // Max : 1000~65535  200KHz ~ 3052Hz
    .DEAD_CYCLE      (LLC1_DEAD_CYCLE     ),  // Time: 0-512 *5ns
    .COUNTER_PHASE   ( 9'd90              )
    )U_PwmGenLLC1
    (
     .iSysClk        (iSysClk             ), //200Mhz 5ns    
     .iSysRst        (iSysRst             ), //复位
     .iPWMEn         (iLLCPwmEn           ),
     .iPwmDuty       (iLLC1PwmDuty[9:0]   ), //0-3FF（0-1023） --> 0-50% 
     .oPwm13Signal   (wLLC1Pwm1P          ),
     .oPwm24Signal   (wLLC1Pwm2P          )
 
    );

PwmGenLLCNew #(
    .PWM_CYCLE       (LLC2_PWM_CYCLE      ), // Max : 1000~65535  200KHz ~ 3052Hz
    .DEAD_CYCLE      (LLC2_DEAD_CYCLE     ),  // Time: 0-512 *5ns
    .COUNTER_PHASE   ( 9'd0               )
    )U_PwmGenLLC2
    (
     .iSysClk        (iSysClk             ), //200Mhz 5ns    
     .iSysRst        (iSysRst             ), //复位
     .iPWMEn         (iLLCPwmEn           ),
     .iPwmDuty       (iLLC2PwmDuty[9:0]   ), //0-3FF（0-1023） --> 0-50% 
     .oPwm13Signal   (wLLC2Pwm1P          ),
     .oPwm24Signal   (wLLC2Pwm2P          )
 
    );


//******************************************************************************
//LLC由于高压侧驱动和低压侧驱动差异，因此需要做延时对齐，使得达到高压和低压功率开关管的驱动波形一致

always@(posedge iSysClk)begin
 rLLC1Pwm1PBuf [127 : 0] <= {rLLC1Pwm1PBuf[126 : 0] , wLLC1Pwm1P} ;
 rLLC1Pwm2PBuf [127 : 0] <= {rLLC1Pwm2PBuf[126 : 0] , wLLC1Pwm2P} ;
 rLLC2Pwm1PBuf [127 : 0] <= {rLLC2Pwm1PBuf[126 : 0] , wLLC2Pwm1P} ;
 rLLC2Pwm2PBuf [127 : 0] <= {rLLC2Pwm2PBuf[126 : 0] , wLLC2Pwm2P} ;
end 

reg rLLC1LVPwm1OutP  ;
reg rLLC1LVPwm2OutP  ;
reg rLLC2LVPwm1OutP  ;
reg rLLC2LVPwm2OutP  ;

reg rLLC1HVPwm1OutP  ;
reg rLLC1HVPwm2OutP  ;
reg rLLC2HVPwm1OutP  ;
reg rLLC2HVPwm2OutP  ;

always @(posedge iSysClk ) begin
    rLLC1LVPwm1OutP <= rLLC1Pwm1PBuf[LV_DELAY];
    rLLC1LVPwm2OutP <= rLLC1Pwm2PBuf[LV_DELAY];
    rLLC1HVPwm1OutP <= rLLC1Pwm1PBuf[HV_DELAY];
    rLLC1HVPwm2OutP <= rLLC1Pwm2PBuf[HV_DELAY];
end


always @(posedge iSysClk ) begin
    rLLC2LVPwm1OutP <= rLLC2Pwm1PBuf[LV_DELAY];
    rLLC2LVPwm2OutP <= rLLC2Pwm2PBuf[LV_DELAY];
    rLLC2HVPwm1OutP <= rLLC2Pwm1PBuf[HV_DELAY];
    rLLC2HVPwm2OutP <= rLLC2Pwm2PBuf[HV_DELAY];
end


assign oLLC1LVPwm1OutP = rLLC1LVPwm1OutP  ;
assign oLLC1LVPwm2OutP = rLLC1LVPwm2OutP  ;
assign oLLC2LVPwm1OutP = rLLC2LVPwm1OutP  ;
assign oLLC2LVPwm2OutP = rLLC2LVPwm2OutP  ;

assign oLLC1HVPwm1OutP = rLLC2HVPwm1OutP  ; //1-2调转，解决驱动板LL1和LLC2高反的问题
assign oLLC1HVPwm2OutP = rLLC2HVPwm2OutP  ;

assign oLLC2HVPwm1OutP = rLLC1HVPwm1OutP  ;
assign oLLC2HVPwm2OutP = rLLC1HVPwm2OutP  ;

//******************************************************************************


endmodule
