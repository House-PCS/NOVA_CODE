//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   RelayCtrlCore.v
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


module RelayCtrlCore#(
    parameter          CLK_FREQ                 =    100 ,  //MHz
    parameter          RELAY_DEAD_TIME          =    1024*1   //ms 10-8191
                  
)
(
    //Global Clock
    input               iSysClk                     ,

    input               iSeriesRelayEn              ,
    input               iParallelRelayEn            ,
    input               iCnt1Ms                     ,
    output              oSeriesRelayEn              ,
    output              oParallelRelayEn            

);

//******************************************************************************
// 防止出现串并联继电器同时闭合
//******************************************************************************
//
localparam  DEAD_TIME               = (RELAY_DEAD_TIME > 13'd100) ? RELAY_DEAD_TIME :  13'd100 ;

reg [12:0]  rCountRelayDeadTime     = 'd0   ;

//******************************************************************************


//******************************************************************************
//
reg [2:0] rSeriesRelayEn      ;
reg [2:0] rParallelRelayEn    ;

always @ (posedge iSysClk)  begin
    rSeriesRelayEn[2:0] <= {rSeriesRelayEn[1:0],iSeriesRelayEn};
    rParallelRelayEn[2:0] <= {rParallelRelayEn[1:0],iParallelRelayEn};
end

always @ (posedge iSysClk)  begin
    if((iParallelRelayEn & rSeriesRelayEn[2]) || ( rParallelRelayEn[2] & iSeriesRelayEn) || (^rSeriesRelayEn[2:1]) || (^rParallelRelayEn[2:1]) )begin
        rCountRelayDeadTime[12:0] <=  13'd0;
    end
    else if(iCnt1Ms)begin
        if(rCountRelayDeadTime[12:0] == (DEAD_TIME - 13'd1))
            rCountRelayDeadTime[12:0] <= rCountRelayDeadTime[12:0];
        else
            rCountRelayDeadTime[12:0] <= rCountRelayDeadTime[12:0] + 1'b1;
    end
    else
        rCountRelayDeadTime[12:0] <= rCountRelayDeadTime[12:0];
end

assign  oSeriesRelayEn      = ((rCountRelayDeadTime[12:0] == (DEAD_TIME - 13'd1)) && iSeriesRelayEn && (!iParallelRelayEn));
assign  oParallelRelayEn    = ((rCountRelayDeadTime[12:0] == (DEAD_TIME - 13'd1)) && iParallelRelayEn && (!iSeriesRelayEn));

//******************************************************************************

endmodule
