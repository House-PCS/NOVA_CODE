//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   SelfInspectionCore.v
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
module SelfInspectionCore#(
    parameter          CLK_FREQ                 =    100       //MHz            
)
(
    //Global Clock
    input               iSysClk                     ,
    input               iSysRst                     ,
    input               iCnt1Ms                     ,
    input   [15:0]      iLlcBusVoltageA             ,
    input   [15:0]      iLlcBusVoltageB             ,
    input   [15:0]      iBusVoltageA                ,
    input   [15:0]      iBusVoltageB                ,
    input   [15:0]      iBusTotalVoltage            ,
    input   [15:0]      iBusTotalVoltagePre         ,

    input   [15:0]      iVoltageSampVref            ,
    input   [15:0]      iCurrentSampVref            ,

    input   [1:0]       iPcsHardwareId              ,//00：Idel 01：SplitPhase 10:SP  11:ThreePhase
    input   [1:0]       iPcsSoftWareId              ,

    input               iAdcSampleSequenceDone      ,
    input               iSelfInspectionEn           ,
    output              oSelfInspectionEnd          ,

    output  [11:0]      oVoltageSampVrefAvera       ,
    output  [11:0]      oCurrentSampVrefAvera       ,

    output              oSeriesRelayEn              ,
    output              oParallelRelayEn            ,           
    output  [1:0]       oPcsUsingId                 //PCS硬件ID
    );


    localparam  AVERAGE_NUM                 =   10'd128  ;


    reg [9:0]   rCount                      =   10'd0    ;
    reg [2:0]   rAdcSampleSequenceDone      =   3'd0     ;
    reg [18:0]  rVoltageSampVref            =   22'd0    ;
    reg [18:0]  rCurrentSampVref            =   22'd0    ;

    reg [11:0]  rVoltageSampVrefAvera       =   12'd0    ;
    reg [11:0]  rCurrentSampVrefAvera       =   12'd0    ;
    reg         rSeriesRelayEn              =   1'b0     ;
    reg         rParallelRelayEn            =   1'b0     ;
    reg         rSelfInspectionEnd          =   1'b0     ;
    reg [1:0]   rPcsUsingId                 =   2'b00    ;

    wire        wSinglePhasePcsEn                        ;
    wire        wThreePhasePcsEn                         ;
    reg [9:0]   rRelayDelayCnt              =   10'd0    ;

assign wSinglePhasePcsEn  = (iPcsSoftWareId[1:0] == 2'b10)  ;
assign wThreePhasePcsEn   = (iPcsSoftWareId[1:0] == 2'b11)  ;


//******************************************************************************
// 

always @ (posedge iSysClk) begin
    rAdcSampleSequenceDone[2:0] <= {rAdcSampleSequenceDone[1:0],iAdcSampleSequenceDone};
end

//******************************************************************************

//******************************************************************************
//
always @ (posedge iSysClk) begin
    if(iSelfInspectionEn)begin
        if(rAdcSampleSequenceDone[2:1] == 2'b01 )begin
            if(rCount[9:0] == AVERAGE_NUM)
                rCount[9:0] <= rCount[9:0];
            else
                rCount[9:0] <= rCount[9:0] + 1'b1;   
        end
    end
    else begin
         rCount[9:0] <= 10'd0;
    end
end
//******************************************************************************


//******************************************************************************
// 
always @ (posedge iSysClk) begin
    if(iSelfInspectionEn)begin
        if(rAdcSampleSequenceDone[2:1] == 2'b01)begin
            if(rCount[9:0] == AVERAGE_NUM)
                rVoltageSampVref[18:0] <= rVoltageSampVref[18:0] ;
            else
                rVoltageSampVref[18:0] <= rVoltageSampVref[18:0] + iVoltageSampVref[11:0];
        end
    end
    else begin
        rVoltageSampVref[18:0] <= 'd0;
    end
end

always @ (posedge iSysClk) begin
    if(iSelfInspectionEn)begin
        if(rAdcSampleSequenceDone[2:1] == 2'b01)begin
            if(rCount[9:0] == AVERAGE_NUM)
                rVoltageSampVrefAvera[11:0] <= rVoltageSampVref[18:7];
        end
    end
    else begin
       rVoltageSampVrefAvera[11:0] <= 12'd0;
    end
end
//******************************************************************************

//******************************************************************************
//
always @ (posedge iSysClk) begin
    if(iSelfInspectionEn)begin
        if(rAdcSampleSequenceDone[2:1] == 2'b01)begin
            if(rCount[9:0] == AVERAGE_NUM)
                rCurrentSampVref[18:0] <= rCurrentSampVref[18:0] ;
            else
                rCurrentSampVref[18:0] <= rCurrentSampVref[18:0] + iCurrentSampVref[11:0];
        end
    end
    else begin
        rCurrentSampVref[18:0] <= 19'd0;
    end
end

always @ (posedge iSysClk) begin
    if(iSelfInspectionEn)begin
        if(rAdcSampleSequenceDone[2:1] == 2'b01)begin
            if(rCount[9:0] == AVERAGE_NUM)
                rCurrentSampVrefAvera[11:0] <= rCurrentSampVref[18:7];
        end
    end
    else begin
       rCurrentSampVrefAvera[11:0] <= 12'd0;
    end
end
//******************************************************************************

assign  oVoltageSampVrefAvera[11:0] = rVoltageSampVrefAvera[11:0] ;
assign  oCurrentSampVrefAvera[11:0] = rCurrentSampVrefAvera[11:0] ;


//******************************************************************************
always @(posedge iSysClk) begin 
    if(iSelfInspectionEn && (!rSelfInspectionEnd))begin
      if(iPcsHardwareId[1:0] != 2'b00)
        rPcsUsingId[1:0] <= iPcsHardwareId[1:0] ;
      else
        rPcsUsingId[1:0] <= iPcsSoftWareId[1:0] ;
    end 
    else
      rPcsUsingId[1:0] <= rPcsUsingId[1:0] ;       
end

always @ (posedge iSysClk) begin
    if(iSelfInspectionEn && (rCount[9:0] == AVERAGE_NUM))begin
        if(!rSelfInspectionEnd)begin  //串并联继电器确定之后退出自检逻辑
            case (rPcsUsingId[1:0])
               2'b00:begin
                    rSeriesRelayEn      <=  1'b0;
                    rParallelRelayEn    <=  1'b0;                   
               end
               2'b10: begin//第一个包 并联
                    rSeriesRelayEn      <=  1'b0;
                    rParallelRelayEn    <=  1'b1;
               end
               2'b11:begin //第一个包 串联
                    rSeriesRelayEn      <=  1'b1;
                    rParallelRelayEn    <=  1'b0;
               end
               default:begin
                    rSeriesRelayEn      <=  1'b0;
                    rParallelRelayEn    <=  1'b0;                  
               end
            endcase
        end 
        else begin
            rSeriesRelayEn      <=  rSeriesRelayEn;
            rParallelRelayEn    <=  rParallelRelayEn;
        end
    end
    else begin
        rSeriesRelayEn      <=  1'b0;
        rParallelRelayEn    <=  1'b0;
    end
end

RelayCtrlCore#(
    .CLK_FREQ                   (CLK_FREQ                       )   ,   //MHz
    .RELAY_DEAD_TIME            (1024*1                         )       //ms 10-8191             
)
(
    .iSysClk                    (iSysClk                        ),
    
    .iSeriesRelayEn             (rSeriesRelayEn                 ),
    .iParallelRelayEn           (rParallelRelayEn               ),
    .iCnt1Ms                    (iCnt1Ms                        ),
    .oSeriesRelayEn             (oSeriesRelayEn                 ),
    .oParallelRelayEn           (oParallelRelayEn               )
);

always @(posedge iSysClk) begin
    if(oSeriesRelayEn || oParallelRelayEn)begin
        if(iCnt1Ms)begin
            if(rRelayDelayCnt[9:0] == 10'd1023)
                rRelayDelayCnt[9:0] <= rRelayDelayCnt[9:0];
            else
                rRelayDelayCnt[9:0] <= rRelayDelayCnt[9:0] +  1'b1;
        end
        else
            rRelayDelayCnt[9:0] <= rRelayDelayCnt[9:0];
    end
    else
        rRelayDelayCnt[9:0] <= 10'd0;
end

always @ (posedge iSysClk) begin
    if(rRelayDelayCnt[9:0] == 10'd1023)
        rSelfInspectionEnd <= 1'b1;
    else
        rSelfInspectionEnd <= 1'b0;
end

assign oSelfInspectionEnd   = rSelfInspectionEnd;

assign oPcsUsingId[1:0] = rPcsUsingId[1:0] ;
//******************************************************************************


//******************************************************************************
//

//******************************************************************************
//



endmodule
