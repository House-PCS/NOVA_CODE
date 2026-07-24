module FanCtrlCore(
    input                               iSysClk                    ,// 时钟输入，例如50MHz
    input                               iSysRst                    ,// 低有效复位
    input                               iSysClk2                   ,
    input                               iSysClk2Cnt1Ms             ,
    input        signed  [   7: 0]      iNtcTemp1                  ,//iSysClk2时钟域
    input        signed  [   7: 0]      iNtcTemp2                  ,//iSysClk2时钟域
    input                               iFanFreGen                 ,
    input                               iSelfInspectionEn          ,//系统自检使能
    input                               iFanForceEn                ,

    output                              oFanPwm                    ,// PWM输出
    output               [  12: 0]      oFanSpeed                  ,
    output                              oFanErr                    

);
/***********************************状态机**********************************************/
    localparam                          IDLE                        = 2'b00                ;
    localparam                          SELFINSPECT                 = 2'b01                ;
    localparam                          DISWORKING                  = 2'b10                ;
    localparam                          WORKING                     = 2'b11                ;

    localparam                          PWM_PERIOD                  = 50_000_000 / 25_000  ;
    reg                [   8: 0]        rCnt500Ms                 ='d0;
    wire                                wCnt500Ms                   ;
    reg                [   1: 0]        rCnt2S=2'b00                ;
    wire                                wDelay2S                    ;
    reg                [   1: 0]        CurrentState              =2'b00;
    reg                [   1: 0]        rCurrentState             =2'b00;
    reg                [   1: 0]        NextState                   ;
    reg                                 rSelfInspectEnd           =1'b0;
    reg     signed     [   7: 0]        RealNtcTemp               =8'sd0;
    reg                                 TempGreaterThan55         ='d0;
    reg                                 TempGreaterThan50='d0       ;
    reg                                 TempLessThan53            ='d0;
    reg                                 TempLessThan48            ='d1;
    wire                                wFanWorkingEn               ;
    wire                                wFanWorkingDis              ;
    reg                [   1: 0]        rPwmDutySel               =2'b00;
    reg                                 FanSpeedChange            =1'b0;
    reg                [   2: 0]        rFanSpeedChange           =2'b00;
    wire                                wFanSpeedChangeExtende      ;
    reg                [   2: 0]        FanSpeedChangeExtendeSysClk=2'b00  ;
    reg                [   1: 0]        PwmDutySelSysClk          ='d0;
    reg                [   1: 0]        PwmDutySel                ='d0;
    reg                [  10: 0]        PwmHigh                     ;
    reg                [  10: 0]        PwmPeriodCnt              ='d0;
    reg                                 rFanPwm                   ='d0;
    reg                [   2: 0]        rFanFreGen                =3'd0;
    reg                [   7: 0]        StableTime                =8'hff;
    reg                [   1: 0]        rStableFanFreGen          =2'b00;
    reg                [   7: 0]        CntFanFreGen=             'd0;
    reg                [   7: 0]        rFanSpeed                 ='d0;
    reg                                 rFanErr                   =1'b0;

    wire                                wFanSpeedUpData             ;
    reg                                 rFanSpeedUpData           ='d0;
    reg                                 rFanForceEn               ='d0;

// 500ms计数器
always @ (posedge iSysClk2) begin
  if(CurrentState!= rCurrentState)//状态跳变时
    rCnt500Ms[8:0] <= 9'd0;
  else if(iSysClk2Cnt1Ms)begin
    if(rCnt500Ms[8:0] == 500) //if(Cnt500Ms[8:0] == 8'd500) 
      rCnt500Ms[8:0] <= 'd0;
    else
      rCnt500Ms[8:0] <= rCnt500Ms[8:0] + 1;
  end
  else
    rCnt500Ms[8:0] <= rCnt500Ms[8:0];
end  

assign wCnt500Ms = (rCnt500Ms[8:0] == 500 && iSysClk2Cnt1Ms);
//2S计数器
always @ (posedge iSysClk2) begin
  if(CurrentState!= rCurrentState || (iFanForceEn!=rFanForceEn))//状态跳变时,清0 ，从头开始记
    rCnt2S[1:0] <= 2'b00;
  else if(wCnt500Ms)begin
    if(rCnt2S[1:0] == 2'b11)
      rCnt2S[1:0]<= 2'b11;
    else
      rCnt2S[1:0] <= rCnt2S[1:0] + 1;
  end
  else
    rCnt2S[1:0] <= rCnt2S[1:0];
end 

assign  wDelay2S = (rCnt2S[1:0]== 2'b11) ; //所有状态Delay2S等待转速稳定后再进行风扇状态检测，否则检测的不准

always@(posedge iSysClk2)begin
  rCurrentState <= CurrentState;
end

always@(posedge iSysClk2 or posedge iSysRst)begin
  if(iSysRst)
    CurrentState <= IDLE;
  else
    CurrentState <= NextState;
end

always@(*)begin
  if(iSysRst)
    NextState <= IDLE;
  else begin
  case(CurrentState)
    IDLE: begin
      if(iSelfInspectionEn)
        NextState <= SELFINSPECT;
      else
        NextState <= IDLE;
    end
    SELFINSPECT: begin   
      if(rSelfInspectEnd)   
        NextState <= DISWORKING;
      else
        NextState <= SELFINSPECT;
    end
    DISWORKING: begin
      if(wFanWorkingEn)
        NextState <= WORKING;
      else
        NextState <= DISWORKING;
    end
    WORKING:begin
      if(wFanWorkingDis)
        NextState <= DISWORKING;
      else
        NextState <= WORKING;
    end
  endcase
  end
end
//****************2S 自检完成标记************************
always @(posedge iSysClk2) begin
    if(wCnt500Ms&&(rCurrentState == SELFINSPECT)&&wDelay2S)
      rSelfInspectEnd <= 1'b1;
    else
      rSelfInspectEnd <= 1'b0;     
end

//**************温度阈值判断 ************
always @(posedge iSysClk2) begin
  if(iNtcTemp1 > iNtcTemp2)
    RealNtcTemp[7:0] <= iNtcTemp1;
  else
    RealNtcTemp[7:0] <= iNtcTemp2;
end

always @(posedge iSysClk2) begin
  TempGreaterThan55 <= RealNtcTemp > 'sd55;
  TempGreaterThan50 <= RealNtcTemp > 'sd50;
  TempLessThan53    <= RealNtcTemp < 'sd53;
  TempLessThan48    <= RealNtcTemp < 'sd48;  
end
assign wFanWorkingEn  = TempGreaterThan50; //大于50℃风扇开始工作
assign wFanWorkingDis = TempLessThan48; //从大于50℃ -> 小于48℃时，风扇停止工作
//**************转速控制****************
always @(posedge iSysClk2) begin
  if(CurrentState == SELFINSPECT)
    PwmDutySel[1:0] <= 2'b1 ; //自检状态时，输出占空比25%，MaxSpeed/4
  else if(CurrentState == WORKING)begin
    if(TempGreaterThan55)
      PwmDutySel[1:0] <= 2'b11; //温度大于55℃时，占空比100%，MaxSpee
    else if(TempLessThan53)
      PwmDutySel[1:0] <= 2'b10; //温度小于53℃时，占空比50%, MaxSpee/2   
  end
  else
    PwmDutySel[1:0] <= 2'b0 ; //占空比0%,风扇不转    
end
//----------PwmDutySel做跨时钟域处理-------------------------
always @(posedge iSysClk2) begin
 rPwmDutySel[1:0] <=  PwmDutySel[1:0];
end

always @(posedge iSysClk2) begin
  if(rPwmDutySel[1:0]!= PwmDutySel[1:0])
    FanSpeedChange <= 1'b1;
  else
    FanSpeedChange <= 1'b0;    
end

always @(posedge iSysClk2) begin
  rFanSpeedChange[2:0] <= {rFanSpeedChange[1:0],FanSpeedChange};  
end

assign wFanSpeedChangeExtende = |rFanSpeedChange[2:0];
  
always @(posedge iSysClk) begin
  FanSpeedChangeExtendeSysClk[2:0] <= {FanSpeedChangeExtendeSysClk[1:0],wFanSpeedChangeExtende};
end

always @(posedge iSysClk) begin
  if(FanSpeedChangeExtendeSysClk[2:1]==2'b01)
    PwmDutySelSysClk[1:0] <=  PwmDutySel[1:0];
  else
    PwmDutySelSysClk[1:0] <=  PwmDutySelSysClk[1:0];    
end

always @(*) begin
  case(PwmDutySelSysClk[1:0])
    1:PwmHigh[10:0] <= PWM_PERIOD >> 2;
    2:PwmHigh[10:0] <= PWM_PERIOD >> 1;
    3:PwmHigh[10:0] <= PWM_PERIOD ;
    default:PwmHigh[10:0] <= 0;
  endcase
end
//-----------------生成25KHZ，占空比可变的PWM波------------------------
always @(posedge iSysClk) begin
  if (CurrentState == IDLE || CurrentState == DISWORKING)
    PwmPeriodCnt[10:0] <= 12'd0;
  else if (PwmPeriodCnt[10:0] == PWM_PERIOD - 1)
    PwmPeriodCnt[10:0] <= 11'd0;
  else
    PwmPeriodCnt[10:0] <= PwmPeriodCnt[10:0] + 1'b1;
end

always @(posedge iSysClk or posedge iSysRst) begin
  if (iSysRst)
    rFanPwm <= 1'b0;
  else
    rFanPwm <= (PwmPeriodCnt[10:0] < PwmHigh[10:0]) ? 1'b1 : 1'b0;
end

assign oFanPwm = iFanForceEn ? 1'b1 : rFanPwm;
//**********************************************************************
//********************风扇状态检测****************************************
//----------------------输出做消抖处理-----------------------------------
always@(posedge iSysClk2)begin
  rFanFreGen[2:0] <= {rFanFreGen[1:0],iFanFreGen};
end
    
always@(posedge iSysClk2)begin
  if(!rFanFreGen[2])
    StableTime[7:0] <= 8'hff;
  else if(StableTime[7:0] == 8'd0)
    StableTime[7:0] <= StableTime[7:0];
  else
    StableTime[7:0] <= StableTime[7:0] - 'd1;
end

always @(posedge iSysClk2) begin
  rStableFanFreGen[1:0] <= {rStableFanFreGen[0],StableTime[7:0] == 8'd0};
end
//------------以500Ms为单位转速统计,风扇每转动一圈产生两个上升沿-------------
always@(posedge iSysClk2)begin
  if(CurrentState!= rCurrentState)
    CntFanFreGen[7:0] <= 'd0;
  else if(wCnt500Ms)
    CntFanFreGen[7:0] <= 'd0;
  else if(rStableFanFreGen[1:0] == 2'b01)begin
    CntFanFreGen[7:0] <= CntFanFreGen[7:0] + 1'd1;        
  end
end

assign wFanSpeedUpData = wCnt500Ms && wDelay2S;
always@(posedge iSysClk2)begin
  if(wFanSpeedUpData)
    rFanSpeed[7:0] <= CntFanFreGen[7:0];
  else
    rFanSpeed[7:0] <= rFanSpeed[7:0] ;  
end
//-------------------------转速处理成rmp---------------------------------
assign oFanSpeed[12:0] = iFanForceEn ? 'd6750 : rFanSpeed[7:0] << 6;
//-------------------------风扇状态标记生成-------------------------------
always @(posedge iSysClk2) begin
  rFanSpeedUpData <= wFanSpeedUpData;
end

always @(posedge iSysClk2) begin
  rFanForceEn <= iFanForceEn;
end
always @(posedge iSysClk2) begin
  if(rFanForceEn && rFanSpeedUpData)
    rFanErr <= rFanSpeed[7:0] < 'd70; 
  else if(rCurrentState == IDLE)
    rFanErr <= 1'b1;
  else if(rCurrentState == SELFINSPECT & rFanSpeedUpData)
    rFanErr <= rFanSpeed[7:0] < 'd10; 
  else if(rCurrentState == WORKING && rFanSpeedUpData)
    rFanErr <= rFanSpeed[7:0] < 'd50; //正常工作时转速小于3000rpm
end

assign oFanErr = rFanErr;

endmodule