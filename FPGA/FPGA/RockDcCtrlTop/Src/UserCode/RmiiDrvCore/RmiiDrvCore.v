//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   RmiiDrvCore.v
//Original Author   :   lvyb
//Creation Date     :   2024.09.12
/*Description       :   Rmii驱动模块。

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
module RmiiDrvCore
#(
    parameter                       RMII_CH             = 2     
)
(
    input                           iSysClk                             ,
    input                           iBoardSelf                          ,       //1表征选择本卡进行回包；0表示透传下游板卡数据包
    input                           iForwardEn                          ,       //1表示接收上游下发指令并转发下游板卡；0表示接收上游指令，不转发给下游板卡
    input                           iRmiiClk                            ,       //50MHz
    input                           iRmiiClk180Deg                      ,       //50MHz180Deg
    output  [(RMII_CH - 1):0]       oRmiiClk                            ,       //50Mhz@180Deg

    input   [(RMII_CH - 1):0]       iRmiiRxDv                           ,       //CRS/CRS_DV
    input   [(RMII_CH*2 - 1):0]     iRmiiRxData                         ,
    output  [(RMII_CH - 1):0]       oGmiiRxDv                           ,
    output  [(RMII_CH*8 - 1):0]     oGmiiRxData                         ,

    input   [(RMII_CH - 1):0]       iGmiiTxEn                           ,
    input   [(RMII_CH*8 - 1):0]     iGmiiTxData                         ,
    output  [(RMII_CH - 1):0]       oRmiiTxEn                           ,
    output  [(RMII_CH*2 - 1):0]     oRmiiTxData                         
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    wire    [(RMII_CH - 1):0]       wRmiiTxEn                           ;
    wire    [(RMII_CH*2 - 1):0]     wRmiiTxData                         ;

//******************************************************************************

//******************************************************************************
//
/*
genvar i;

generate
    for(i = 0; i < RMII_CH; i = i + 1) begin : U_RmiiRx

        RmiiRx U_RmiiRx
        (
            .iRmiiRxClk                     (iRmiiClk                           ),
            .iRmiiRxDv                      (iRmiiRxDv[i*1+:1]                  ),      //CRS/CRS_DV
            .iRmiiRxData                    (iRmiiRxData[i*2+:2]                ),
            .iGmiiRxClk                     (iSysClk                            ),
            .oGmiiRxDv                      (oGmiiRxDv[i*1+:1]                  ),
            .oGmiiRxData                    (oGmiiRxData[i*8+:8]                ) 
        );

   end
endgenerate
*/

// ila_0 your_instance_name (
// 	.clk    (iSysClk    ), // input wire clk
// 	.probe0 ({oGmiiRxDv[0*1+:1],oGmiiRxData[0*8+:8],iRmiiRxDv[0*1+:1],iRmiiRxData[0*2+:2],wRmiiTxEn[0*1+:1],wRmiiTxData[0*2+:2],iGmiiTxEn[0*1+:1],iGmiiTxData[0*8+:8]})     // input wire [31:0] probe0
// );

RmiiRx U0_RmiiRx
(
    .iRmiiRxClk                     (iRmiiClk                           ),
    .iRmiiRxDv                      (iRmiiRxDv[0*1+:1]                  ),      //CRS/CRS_DV
    .iRmiiRxData                    (iRmiiRxData[0*2+:2]                ),
    .iGmiiRxClk                     (iSysClk                            ),
    .oGmiiRxDv                      (oGmiiRxDv[0*1+:1]                  ),
    .oGmiiRxData                    (oGmiiRxData[0*8+:8]                ) 
);

RmiiRx U1_RmiiRx
(
    .iRmiiRxClk                     (iRmiiClk                           ),
    .iRmiiRxDv                      (iRmiiRxDv[1*1+:1]                  ),      //CRS/CRS_DV
    .iRmiiRxData                    (iRmiiRxData[1*2+:2]                ),
    .iGmiiRxClk                     (iSysClk                            ),
    .oGmiiRxDv                      (oGmiiRxDv[1*1+:1]                  ),      
    .oGmiiRxData                    (oGmiiRxData[1*8+:8]                )       
);

//******************************************************************************

//******************************************************************************
//

RmiiTx U0_RmiiTx
(
    .iGmiiTxClk                     (iSysClk                            ),
    .iGmiiTxEn                      (iGmiiTxEn[0*1+:1]                  ),
    .iGmiiTxData                    (iGmiiTxData[0*8+:8]                ),
    .iRmiiTxClk                     (iRmiiClk                           ),
    .iRmiiClk180Deg                 (iRmiiClk180Deg                     ),
    .oRmiiTxEn                      (wRmiiTxEn[0*1+:1]                  ),
    .oRmiiTxData                    (wRmiiTxData[0*2+:2]                ),
    .oRmiiClk                       (oRmiiClk[0*1+:1]                   ) 
);

RmiiTx U1_RmiiTx
(
    .iGmiiTxClk                     (iSysClk                            ),
    .iGmiiTxEn                      (iGmiiTxEn[1*1+:1]                  ),         
    .iGmiiTxData                    (iGmiiTxData[1*8+:8]                ),         
    .iRmiiTxClk                     (iRmiiClk                           ),
    .iRmiiClk180Deg                 (iRmiiClk180Deg                     ),
    .oRmiiTxEn                      (                                   ),      //wRmiiTxEn[1*1+:1]  
    .oRmiiTxData                    (                                   ),      //wRmiiTxData[1*2+:2]
    .oRmiiClk                       (oRmiiClk[1*1+:1]                   ) 
);


//******************************************************************************

//******************************************************************************
//

reg     [(RMII_CH - 1):0]       rRmiiRxDv   [1:0]  ;     //CRS/CRS_DV
reg     [(RMII_CH*2 - 1):0]     rRmiiRxData [1:0]  ;


//******************************************************************************
//下行数据
//先用上升沿采集
//再用下降沿输出
always @ (posedge iRmiiClk) begin
    rRmiiRxDv[0][0*1+:1] <= iRmiiRxDv[0*1+:1] ;
end

always @ (posedge iRmiiClk) begin
    rRmiiRxData[0][0*2+:2] <= iRmiiRxData[0*2+:2] ;
end
//
always @ (posedge iRmiiClk180Deg) begin
    rRmiiRxDv[1][0*1+:1] <= rRmiiRxDv[0][0*1+:1] ;
end

always @ (posedge iRmiiClk180Deg) begin
    rRmiiRxData[1][0*2+:2] <= rRmiiRxData[0][0*2+:2] ;
end

//******************************************************************************
//
//******************************************************************************
//上行数据
//先用上升沿采集
//再用下降沿输出
always @ (posedge iRmiiClk) begin
    rRmiiRxDv[0][1*1+:1] <= iRmiiRxDv[1*1+:1] ;
end

always @ (posedge iRmiiClk) begin
    rRmiiRxData[0][1*2+:2] <= iRmiiRxData[1*2+:2] ;
end
//
always @ (posedge iRmiiClk180Deg) begin
    rRmiiRxDv[1][1*1+:1] <= rRmiiRxDv[0][1*1+:1] ;
end

always @ (posedge iRmiiClk180Deg) begin
    rRmiiRxData[1][1*2+:2] <= rRmiiRxData[0][1*2+:2] ;
end

//******************************************************************************

//******************************************************************************
//


assign oRmiiTxEn[1*1+:1]                = iForwardEn ? rRmiiRxDv[1][0*1+:1] : 1'd0;
assign oRmiiTxData[1*2+:2]              = iForwardEn ? rRmiiRxData[1][0*2+:2] : 2'd0;

assign oRmiiTxEn[0*1+:1]                = wRmiiTxEn[0*1+:1] ? 1'b1 : rRmiiRxDv[1][1*1+:1];
assign oRmiiTxData[0*2+:2]              = wRmiiTxEn[0*1+:1] ? wRmiiTxData[0*2+:2] : rRmiiRxData[1][1*2+:2];


// assign oRmiiTxEn[1*1+:1]                = iForwardEn ? iRmiiRxDv[0*1+:1] : 1'd0;
// assign oRmiiTxData[1*2+:2]              = iForwardEn ? iRmiiRxData[0*2+:2] : 2'd0;

// assign oRmiiTxEn[0*1+:1]                = wRmiiTxEn[0*1+:1] ? 1'b1 : iRmiiRxDv[1*1+:1];
// assign oRmiiTxData[0*2+:2]              = wRmiiTxEn[0*1+:1] ? wRmiiTxData[0*2+:2] : iRmiiRxData[1*2+:2];

//******************************************************************************

endmodule
