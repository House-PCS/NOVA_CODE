//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   BmsParamCore.v
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
module BmsParamCore
(
    //Global Clock
    input                               iSysClk                 ,
    //BmsParamBus
    input           [7:0]               iBmsParamBusRdData      ,
    output                              oBmsParamBusClk         ,
    output          [31:0]              oBmsParamBusAddr        ,
    //User Interface
    output  signed  [31:0]              oBatVoltage             ,
    output  signed  [31:0]              oBatVoltageAverage      ,
    output  signed  [31:0]              oBatCurrent             ,
    output  signed  [15:0]              oBatSoc                 ,
    output  signed  [31:0]              oCurrentMaxChgCurrent   ,
    output  signed  [31:0]              oCurrentMaxDsgCurrent   , 
    output  signed  [ 7:0]              oBmsShutdown    
                            
);


//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    reg         [10:0]              rBmsParamBusAddr            = 11'd0     ;
    reg         [10:0]              rBmsParamBusAddrBuf [1:0]               ;   
    reg         [7:0]               rBmsParam                   = 8'd0      ;

    reg signed  [15:0]              rBatVoltageBuf                          ; 
    reg signed  [15:0]              rBatVoltageAverageBuf                   ;
    reg signed  [15:0]              rBatCurrentBuf                          ; 
   // reg signed  [15:0]              rBatOutputPowerBuf                      ; 
    reg signed  [15:0]              rBatSocBuf                              ;
    reg signed  [15:0]              rCurrentMaxChgCurrentBuf                ;
    reg signed  [15:0]              rCurrentMaxDsgCurrentBuf                ;
    reg signed  [ 7:0]              rBmsShutdownBuf                         ;


    reg signed  [31:0]              rBatVoltage                             ; 
    reg signed  [31:0]              rBatVoltageAverage                      ;
    reg signed  [31:0]              rBatCurrent                             ; 
//    reg signed  [15:0]              rBatOutputPower                         ; 
    reg signed  [15:0]              rBatSoc                                 ;
    reg signed  [31:0]              rCurrentMaxChgCurrent                   ;
    reg signed  [31:0]              rCurrentMaxDsgCurrent                   ;
    reg signed  [ 7:0]              rBmsShutdown                            ;


//******************************************************************************




//******************************************************************************
//

always @ (posedge iSysClk) begin
    rBmsParamBusAddr[10:0] <= rBmsParamBusAddr[10:0] + 11'd1;
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    rBmsParamBusAddrBuf[0][10:0] <= rBmsParamBusAddr[10:0];
    rBmsParamBusAddrBuf[1][10:0] <= rBmsParamBusAddrBuf[0][10:0];
end


//******************************************************************************

//******************************************************************************
//

assign oBmsParamBusClk = iSysClk;
assign oBmsParamBusAddr[31:0] = {16'h0100, 5'd0, rBmsParamBusAddr[10:0]};

//******************************************************************************



//******************************************************************************
//32'h0100_0001

always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h022<<1))
        rBatVoltageBuf [7:0] <= iBmsParamBusRdData[7:0];
    else
        rBatVoltageBuf [7:0] <= rBatVoltageBuf [7:0];
end

always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h022<<1)+1'd1)
        rBatVoltageBuf [15:8] <= iBmsParamBusRdData[7:0];
    else
        rBatVoltageBuf [15:8] <= rBatVoltageBuf [15:8];
end

always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h0f0<<1))
        rBatVoltageAverageBuf [7:0] <= iBmsParamBusRdData[7:0];
    else
        rBatVoltageAverageBuf [7:0] <= rBatVoltageAverageBuf [7:0];
end

always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h0f0<<1)+1'd1)
        rBatVoltageAverageBuf [15:8] <= iBmsParamBusRdData[7:0];
    else
        rBatVoltageAverageBuf [15:8] <= rBatVoltageAverageBuf [15:8];
end
//******************************************************************************

//******************************************************************************
//
always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h023<<1))
        rBatCurrentBuf[7:0] <= iBmsParamBusRdData[7:0];
    else
        rBatCurrentBuf[7:0] <= rBatCurrentBuf[7:0];
end

always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h023<<1)+1'd1)
        rBatCurrentBuf[15:8] <= iBmsParamBusRdData[7:0];
    else
        rBatCurrentBuf[15:8] <= rBatCurrentBuf[15:8];
end

//******************************************************************************

//******************************************************************************
//
always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h025<<1))
        rBatSocBuf[7:0] <= iBmsParamBusRdData[7:0];
    else
        rBatSocBuf[7:0] <= rBatSocBuf[7:0];
end

always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h025<<1)+1'd1)
        rBatSocBuf[15:8] <= iBmsParamBusRdData[7:0];
    else
        rBatSocBuf[15:8] <= rBatSocBuf[15:8];
end
//******************************************************************************


//******************************************************************************
//
always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h028<<1))
        rCurrentMaxChgCurrentBuf[7:0] <= iBmsParamBusRdData[7:0];
    else
        rCurrentMaxChgCurrentBuf[7:0] <= rCurrentMaxChgCurrentBuf[7:0];
end

always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h028<<1)+1'd1)
        rCurrentMaxChgCurrentBuf[15:8] <= iBmsParamBusRdData[7:0];
    else
        rCurrentMaxChgCurrentBuf[15:8] <= rCurrentMaxChgCurrentBuf[15:8];
end
//******************************************************************************

//******************************************************************************
//
always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h029<<1))
        rCurrentMaxDsgCurrentBuf[7:0] <= iBmsParamBusRdData[7:0];
    else
        rCurrentMaxDsgCurrentBuf[7:0] <= rCurrentMaxDsgCurrentBuf[7:0];
end

always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h029<<1)+1'd1)
        rCurrentMaxDsgCurrentBuf[15:8] <= iBmsParamBusRdData[7:0];
    else
        rCurrentMaxDsgCurrentBuf[15:8] <= rCurrentMaxDsgCurrentBuf[15:8];
end

always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h0EC<<1))//BMS
        rBmsShutdownBuf[7:0] <= iBmsParamBusRdData[7:0];
    else
        rBmsShutdownBuf[7:0] <= rBmsShutdownBuf[7:0];
end
//******************************************************************************


//******************************************************************************
// $signed  ;
always @ (posedge iSysClk) begin
    if(rBmsParamBusAddrBuf[1][10:0] == (11'h7FF)) begin
        rBatVoltage           <= $signed(rBatVoltageBuf             );
        rBatVoltageAverage    <= $signed(rBatVoltageAverageBuf      ); 
        rBatCurrent           <= $signed(rBatCurrentBuf             );
        rBatSoc               <= $signed(rBatSocBuf                 );
        rCurrentMaxChgCurrent <= $signed(rCurrentMaxChgCurrentBuf   );
        rCurrentMaxDsgCurrent <= $signed(rCurrentMaxDsgCurrentBuf   );
        rBmsShutdown          <=        (rBmsShutdownBuf            );
        end
    else begin
        rBatVoltage           <= rBatVoltage           ;
        rBatVoltageAverage    <= rBatVoltageAverage    ;
        rBatCurrent           <= rBatCurrent           ;
        rBatSoc               <= rBatSoc               ;
        rCurrentMaxChgCurrent <= rCurrentMaxChgCurrent ;
        rCurrentMaxDsgCurrent <= rCurrentMaxDsgCurrent ;
        rBmsShutdown          <= rBmsShutdown          ;
    end
end

//******************************************************************************

//******************************************************************************
//
assign    oBatVoltage           = rBatVoltage           ;
assign    oBatVoltageAverage    = rBatVoltageAverage    ;
assign    oBatCurrent           = rBatCurrent           ;
assign    oBatSoc               = rBatSoc               ;
assign    oCurrentMaxChgCurrent = rCurrentMaxChgCurrent ;
assign    oCurrentMaxDsgCurrent = rCurrentMaxDsgCurrent ;
assign    oBmsShutdown          = rBmsShutdown          ;

//******************************************************************************
endmodule
