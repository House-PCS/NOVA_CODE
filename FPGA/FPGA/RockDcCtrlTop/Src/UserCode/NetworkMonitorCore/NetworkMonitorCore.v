//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   NetworkMonitorCore.v
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

module NetworkMonitorCore#(
    parameter          CLK_FREQ                 =    100    ,  //MHz
    parameter          NETWORK_MONITOR_TIME     =    5         //s 10-8191               
)
(
    //Global Clock
    input   wire                iSysClk                     ,
    input   wire                iForwardEn                  ,
    input   wire    [1:0]       iDataValid                  ,
    input   wire                iCnt1S                      ,
    output  wire                oNoEthNet               
);

//******************************************************************************
reg [9:0]   rCountDataValidDeadTime     = 10'd0 ;

wire        wDataValid;

assign wDataValid = iDataValid[1] | iDataValid[0];

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk)  begin
    if(wDataValid)begin
        rCountDataValidDeadTime[9:0] <=  10'd0;
    end
    else if(iCnt1S && iForwardEn)begin
        if(rCountDataValidDeadTime[9:0] == (NETWORK_MONITOR_TIME - 10'd1))
            rCountDataValidDeadTime[9:0] <= rCountDataValidDeadTime[9:0];
        else
            rCountDataValidDeadTime[9:0] <= rCountDataValidDeadTime[9:0] + 1'b1;
    end
    else
        rCountDataValidDeadTime[9:0] <= rCountDataValidDeadTime[9:0];

end


assign  oNoEthNet = (rCountDataValidDeadTime[9:0] == (NETWORK_MONITOR_TIME - 10'd1));
 
endmodule
