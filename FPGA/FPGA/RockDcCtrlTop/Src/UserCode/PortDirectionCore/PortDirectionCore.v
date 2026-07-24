//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   PortDirectionCore.v
//Original Author   :   lvyb
//Creation Date     :   2024.09.19
/*Description       :   创建网口优先级切换模块。

*/
//------------------------------------------------------------------------------
//Version           :   Rev 0.1
//Modifier          :   lvyb
//Modify Date       :   2024.09.24
/*Modify Record     :   1、修改端口，修改为物理端口和逻辑端口；2、修改为Rmii接口的优先级判断。

*/
//------------------------------------------------------------------------------
//Version           :   Rev 0.2
//Modifier          :   
//Modify Date       :   
/*Modify Record     :   

*/  
//------------------------------------------------------------------------------

`timescale 1ns/1ns
module PortDirectionCore
#(
    parameter                       RMII_CH = 2
)
(
    //Global Clock
    input                           iSysClk         ,
    //GmiiPort
    input   [(RMII_CH - 1):0]       iPhysicalRxDv   ,
    input   [(RMII_CH*2 - 1):0]     iPhysicalRxData ,
    output  [(RMII_CH - 1):0]       oPhysicalTxEn   ,
    output  [(RMII_CH*2 - 1):0]     oPhysicalTxData ,
    //LogicPort
    input   [(RMII_CH - 1):0]       iLogicTxEn      ,
    input   [(RMII_CH*2 - 1):0]     iLogicTxData    ,
    output  [(RMII_CH - 1):0]       oLogicRxDv      ,
    output  [(RMII_CH*2 - 1):0]     oLogicRxData    ,
    //OtherPort
    input   [(RMII_CH*2 - 1):0]     iLogicPriority  
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    reg     [(RMII_CH*2 - 1):0]     rLogicPriority      = 'd0   ;
    wire                            wCompareResult              ;
    reg     [1:0]                   rCompareResultBuf   = 2'd0  ;
    reg     [26:0]                  rDirectionWrongCnt  = 27'd0 ;
    reg                             rPortDirection      = 1'd0  ;
    reg     [(RMII_CH - 1):0]       rPhysicalRxDvBuf    = 'd0   ;
    reg     [(RMII_CH*2 - 1):0]     rPhysicalRxDataBuf  = 'd0   ;
    reg     [(RMII_CH - 1):0]       rLogicRxDv          = 'd0   ;
    reg     [(RMII_CH*2 - 1):0]     rLogicRxData        = 'd0   ;
    reg     [(RMII_CH - 1):0]       rPhysicalTxEn       = 'd0   ;
    reg     [(RMII_CH*2 - 1):0]     rPhysicalTxData     = 'd0   ;

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    rLogicPriority[(RMII_CH*2 - 1):0] <= iLogicPriority[(RMII_CH*2 - 1):0];
end

assign wCompareResult = (rLogicPriority[1*2+:2] > rLogicPriority[0*2+:2]);

always @ (posedge iSysClk) begin
    rCompareResultBuf[1:0] <= {rCompareResultBuf[0], wCompareResult};
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if((rCompareResultBuf[1:0] != 2'b11) || (rDirectionWrongCnt[26:25] == 2'b11))
        rDirectionWrongCnt[26:0] <= 27'd0;
    else
        rDirectionWrongCnt[26:0] <= rDirectionWrongCnt[26:0] + 27'd1;
end

always @ (posedge iSysClk) begin
    if((rCompareResultBuf[1:0] == 2'b01) || (rDirectionWrongCnt[26:25] == 2'b11))
        rPortDirection <= !rPortDirection;
    else
        rPortDirection <= rPortDirection;  
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    rPhysicalRxDvBuf[(RMII_CH - 1):0] <= iPhysicalRxDv[(RMII_CH - 1):0];
end

always @ (posedge iSysClk) begin
    rPhysicalRxDataBuf[(RMII_CH*2 - 1):0] <= iPhysicalRxData[(RMII_CH*2 - 1):0];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(rPortDirection)
        rLogicRxDv[(RMII_CH - 1):0] <= {rPhysicalRxDvBuf[0*1+:1], rPhysicalRxDvBuf[1*1+:1]};
    else
        rLogicRxDv[(RMII_CH - 1):0] <= rPhysicalRxDvBuf[(RMII_CH - 1):0];
end

always @ (posedge iSysClk) begin
    if(rPortDirection)
        rLogicRxData[(RMII_CH*2 - 1):0] <= {rPhysicalRxDataBuf[0*2+:2], rPhysicalRxDataBuf[1*2+:2]};
    else
        rLogicRxData[(RMII_CH*2 - 1):0] <= rPhysicalRxDataBuf[(RMII_CH*2 - 1):0];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(rPortDirection)
        rPhysicalTxEn[(RMII_CH - 1):0] <= {iLogicTxEn[0*1+:1], iLogicTxEn[1*1+:1]};
    else
        rPhysicalTxEn[(RMII_CH - 1):0] <= iLogicTxEn[(RMII_CH - 1):0];
end

always @ (posedge iSysClk) begin
    if(rPortDirection)
        rPhysicalTxData[(RMII_CH*2 - 1):0] <= {iLogicTxData[0*2+:2], iLogicTxData[1*2+:2]};
    else
        rPhysicalTxData[(RMII_CH*2 - 1):0] <= iLogicTxData[(RMII_CH*2 - 1):0];
end

//******************************************************************************

//******************************************************************************
//

assign oLogicRxDv[(RMII_CH - 1):0] = rLogicRxDv[(RMII_CH - 1):0];
assign oLogicRxData[(RMII_CH*2 - 1):0] = rLogicRxData[(RMII_CH*2 - 1):0];

//******************************************************************************

//******************************************************************************
//

assign oPhysicalTxEn[(RMII_CH - 1):0] = rPhysicalTxEn[(RMII_CH - 1):0];
assign oPhysicalTxData[(RMII_CH*2 - 1):0] = rPhysicalTxData[(RMII_CH*2 - 1):0];

//******************************************************************************

endmodule
