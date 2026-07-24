//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   OperationCtrl.v
//Original Author   :   xcy 003107
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
module OperationCtrl#(
    parameter          CLK_FREQ                 =    100       //MHz            
)
(
    //Global Clock
    input               iSysClk                     ,
    input               iSysRst                     ,
            
    input               iDcdcErr                    ,
    input               iNoEthNet                   ,

    input               iSelfInspectionEnd          ,
    input               iSlowLaunchEnd              , 
    
    input               iDmosAndCmosEn              ,
    input       [7:0]   iBmsShutdown                ,

    output              oSelfInspectionEn           ,
    output              oSlowLaunchEn               , 
    output              oWorkingEn                  ,//使能电压电流双闭环
    output              oProtectEn                  ,
    output      [7:0]   oCurrentState             
);
 // 状态定义
localparam                          IDLE                      = 2'b00;
localparam                          SELF_INSPECTION           = 2'b01;
localparam                          SLOW_LAUNCH               = 2'b10;
localparam                          WORKING                   = 2'b11;
reg                [   8: 0]        rDelayL                   ='d0;
reg                [   9: 0]        rDelayM                   ='d0;
reg                [   9: 0]        rDelayH                   ='d0;
reg                                 rStartDelayComp           ='d0;
reg                [   1: 0]        rCurrentState             =2'd0;
reg                [   1: 0]        rNextState                =2'd0;
reg                                 rSlowLaunchEn             =1'b0;
reg                                 rSelfInspectionEn         =1'b0;
reg                                 rWorkingEn                =1'b0;
reg                                 rProtectEn                =1'b0;
wire                                wGotoIdle                   ;

always @(posedge iSysClk) begin
    if(rDelayL[8:0] == 9'h1ff)
      rDelayL[8:0] <= 9'h000;
    else
      rDelayL[8:0] <= rDelayL[8:0] + 1'b1;      
end

always @(posedge iSysClk or posedge iSysRst) begin
    if(iSysRst)
      rDelayM[9:0] <= 10'h000;
    else if(wGotoIdle)
      rDelayM[9:0] <= 10'h000;    
    else if(rDelayL[8:0] == 9'h1ff)begin
      if(rDelayM[9:0] == 10'h3ff)
        rDelayM[9:0] <= rDelayM[9:0];
      else
        rDelayM[9:0] <= rDelayM[9:0] + 1'b1;
    end     
end

always @(posedge iSysClk or posedge iSysRst) begin
    if(wGotoIdle || iSysRst)
      rDelayH[9:0] <= 10'h000;
    else if(wGotoIdle)
      rDelayH[9:0] <= 10'h000;
    else if(rDelayL[8:0] == 9'h1ff && rDelayM[9:0] == 10'h3ff)begin
      if(rDelayH[9:0] == 10'h3ff)
        rDelayH[9:0] <= rDelayH[9:0];
      else
        rDelayH[9:0] <= rDelayH[9:0] + 1'b1;
    end     
end

always @(posedge iSysClk or posedge iSysRst) begin
    if( iSysRst)
      rStartDelayComp <= 1'b0;  
    else if(wGotoIdle || iSysRst)
      rStartDelayComp <= 1'b0;     
    else if(rDelayM[9:0] == 10'h3ff && rDelayH[9:0] == 10'h3ff)
      rStartDelayComp <= 1'b1;    
end

always @(posedge iSysClk or posedge iSysRst) begin
    if (iSysRst)
        rCurrentState <= IDLE;
    else
        rCurrentState <= rNextState;
end

assign oCurrentState = rCurrentState;

assign wGotoIdle = (iBmsShutdown[3:2] != 2'd0) ||iDcdcErr || iNoEthNet || (!iDmosAndCmosEn);

always @ (*) begin
    if(wGotoIdle)
      rNextState = IDLE;
    else begin
      case (rCurrentState)
          IDLE: begin
              if(rStartDelayComp && iDmosAndCmosEn)
                  rNextState = SELF_INSPECTION;
              else
                  rNextState = IDLE;
          end
          SELF_INSPECTION: begin
              if(iSelfInspectionEnd)
                  rNextState = SLOW_LAUNCH;
              else
                  rNextState = SELF_INSPECTION;
          end
          SLOW_LAUNCH: begin
              if(iSlowLaunchEnd)
                  rNextState = WORKING;
              else
                  rNextState = SLOW_LAUNCH;
          end 
          WORKING:begin
                rNextState = WORKING;
          end
          default:    rNextState = IDLE;
      endcase
    end
end

always @(posedge iSysClk)begin
    case (rCurrentState)
        IDLE:begin
            rProtectEn          <=  1'b0 ;
            rSelfInspectionEn   <=  1'b0 ;
            rSlowLaunchEn       <=  1'b0 ;
            rWorkingEn          <=  1'b0 ;
        end
        SELF_INSPECTION:begin
            rProtectEn          <=  1'b1 ;
            rSelfInspectionEn   <=  1'b1 ;
            rSlowLaunchEn       <=  1'b0 ;
            rWorkingEn          <=  1'b0 ;
        end 
        SLOW_LAUNCH:begin
            rProtectEn          <=  1'b1 ;
            rSelfInspectionEn   <=  1'b1 ;
            rSlowLaunchEn       <=  1'b1 ;
            rWorkingEn          <=  1'b0 ;
        end 
        WORKING:begin
            rProtectEn          <=  1'b1 ;
            rSelfInspectionEn   <=  1'b1 ;
            rSlowLaunchEn       <=  1'b1 ;
            rWorkingEn          <=  1'b1 ;
        end
        default:begin
            rProtectEn          <=  rProtectEn        ;
            rSelfInspectionEn   <=  rSelfInspectionEn ;
            rSlowLaunchEn       <=  rSlowLaunchEn     ;
            rWorkingEn          <=  rWorkingEn        ;
        end  
    endcase
end

assign  oSlowLaunchEn       =   rSlowLaunchEn      ;
assign  oSelfInspectionEn   =   rSelfInspectionEn  ;
assign  oWorkingEn          =   rWorkingEn         ;
assign  oProtectEn          =   rProtectEn         ;  

endmodule