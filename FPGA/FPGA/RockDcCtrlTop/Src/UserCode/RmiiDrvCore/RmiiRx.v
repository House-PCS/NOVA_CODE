//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   RmiiRx.v
//Original Author   :   lvyb
//Creation Date     :   2024.09.12
/*Description       :   Rmii接收接口模块。

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
module RmiiRx
(
    input                           iRmiiRxClk                          ,
    input                           iRmiiRxDv                           ,       //CRS/CRS_DV
    input   [1:0]                   iRmiiRxData                         ,
    
    input                           iGmiiRxClk                          ,
    output                          oGmiiRxDv                           ,
    output  [7:0]                   oGmiiRxData                      
);
//-------------------------------Code Start Here--------------------------------
///******************************************************************************
// Internal Signals
//******************************************************************************
reg  [2:0]              rRmiiRxDvBuf        = 3'd0;
reg  [11:0]             rRmiiRxCnt          = 12'd0;    // Reduced from 14 to 2 bits
reg  [9:0]              rRmiiRxCnt1         = 10'd0;
reg  [9:0]              rRmiiRxCnt2         = 10'd0;
reg  [7:0]              rRmii2GmiiRxData    = 8'd0;
reg                     rWrEn               = 1'b0;
reg                     rGmiiRxDv           = 1'b0;
reg                     rGmiiRxDv1          = 1'b0;
reg                     rGmiiRxDv2          = 1'b0;
reg  [7:0]              rGmiiRdData         = 8'd0;
reg  [9:0]              rGmiiRxNum          = 10'h3ff;
reg                     rRdEn               = 1'b0;

wire                    wRmiiRxDvNege       ;
wire                    wRmiiRxDvPoge       ; 
wire [7:0]              wWrData             ;
wire [7:0]              wRdData             ;
reg                     WrBaseAddr = 1'b0   ;
reg                     RdBaseAddr = 1'b0   ; 
reg  [12:0]             WrAddr = 13'h0000   ;
reg  [10:0]             RdAddr = 11'h000    ;
reg  [1:0]              RmiiRxDvNegeDly      = 1'b0;
reg  [1:0]              RamCacheCnt          = 2'd0;
wire                    CacheCntPlus               ;
wire                    CacheCntReduce             ;   
reg                     CacheValid           = 1'd0;
reg  [1:0]              CacheValidDly        = 2'd0;
reg  [1:0]              RdEnDly              = 2'd0;
reg                     CacheCntReduceExtend = 1'd0;
reg  [2:0]              CacheCntReduceDly    = 3'd0;
reg  [3:0]              CacheCntReducePreDly = 4'd0;
//******************************************************************************
// RMII Clock Domain: Input Synchronization
//******************************************************************************
always @(posedge iRmiiRxClk) begin
    rRmiiRxDvBuf[2:0]   <= {rRmiiRxDvBuf[1:0], iRmiiRxDv};
end

assign wRmiiRxDvNege = ({rRmiiRxDvBuf[0],iRmiiRxDv} == 2'b10);
assign wRmiiRxDvPoge = (rRmiiRxDvBuf[1:0] == 2'b01);

//******************************************************************************
// RMII Clock Domain: 2-bit to 8-bit Conversion
//******************************************************************************

//******************************************************************************
// Clock Domain Crossing: Async FIFO
//******************************************************************************
Sdpram8192x2_2048x8_D2 U_RmiiRxData(
    .clka  (iRmiiRxClk          ),
    .wea   (iRmiiRxDv           ),
    .addra (WrAddr[12:0]        ),
    .dina  (iRmiiRxData[1:0]    ),
    .clkb  (iGmiiRxClk          ),
    .addrb (RdAddr[10:0]        ),
    .doutb (wRdData[7:0]        )
  );
always @(posedge iRmiiRxClk) begin
     RmiiRxDvNegeDly <= {RmiiRxDvNegeDly[0],wRmiiRxDvNege};
end

always @(posedge iRmiiRxClk) begin
    if(wRmiiRxDvNege && (RamCacheCnt[1:0] != 2'd2))//(rRmiiRxDvNegeDly[0] && (RamCacheCnt[1:0] != 2'd2))
        WrBaseAddr <= ~WrBaseAddr;
    else
        WrBaseAddr <= WrBaseAddr;
end

always @(posedge iRmiiRxClk) begin
    if(RmiiRxDvNegeDly[0])//(rRmiiRxDvNegeDly[1])
        WrAddr[12:0] <= {WrBaseAddr,12'h000};
    else if(iRmiiRxDv)
        WrAddr[12:0] <= {WrAddr[12],WrAddr[11:0] + 1'b1};
end

always @(posedge iRmiiRxClk) begin
    if(wRmiiRxDvNege)begin
        if(WrBaseAddr)begin
            rRmiiRxCnt1 <= rRmiiRxCnt1;
            rRmiiRxCnt2 <= WrAddr[11:2];
        end   
        else begin
            rRmiiRxCnt1 <= WrAddr[11:2];
            rRmiiRxCnt2 <= rRmiiRxCnt2;
        end
    end
    else begin
        rRmiiRxCnt1 <= rRmiiRxCnt1;
        rRmiiRxCnt2 <= rRmiiRxCnt2;    
    end
end

always @(posedge iRmiiRxClk) begin
    if(CacheCntReduce)begin
        if(wRmiiRxDvPoge && (RamCacheCnt[1:0] == 2'd2))
            RamCacheCnt[1:0] <= 2'd0;
        else if(CacheCntPlus )
             RamCacheCnt[1:0] <= RamCacheCnt[1:0];
        else
             RamCacheCnt[1:0] <= RamCacheCnt[1:0] - 1'b1;
    end
    else if(wRmiiRxDvPoge && (RamCacheCnt[1:0] == 2'd2))
        RamCacheCnt[1:0] <= 2'd1;
    else if(CacheCntPlus)
        RamCacheCnt[1:0] <= RamCacheCnt[1:0] + 1'b1;
end

always @(posedge iRmiiRxClk) begin
    CacheCntReduceDly[2:0] <= {CacheCntReduceDly[1:0],CacheCntReduceExtend};
end

assign CacheCntReduce = CacheCntReduceDly[2:1]==2'b01;
assign CacheCntPlus = wRmiiRxDvNege;

always @(posedge iRmiiRxClk) begin
    CacheValid <= (RamCacheCnt[1:0] != 2'd0);
end
//******************************************************************************
// GMII Clock Domain: Output Logic
//******************************************************************************
// Input sync for GMII domain
always @(posedge iGmiiRxClk) begin
    if(RdEnDly[1:0]==2'b01)begin
        if (RdBaseAddr)
            rGmiiRxNum[9:0]  <= rRmiiRxCnt2;
        else
            rGmiiRxNum[9:0]  <= rRmiiRxCnt1;
    end
    else
        rGmiiRxNum[9:0]  <= rGmiiRxNum[9:0];
end

// Read control logic - improved with registered output
always @(posedge iGmiiRxClk) begin
    CacheValidDly[1:0] <= {CacheValidDly[0],CacheValid};
end

always @(posedge iGmiiRxClk) begin
    if (RdAddr[9:0] == rGmiiRxNum[9:0]-1'b1)
        rRdEn <= 1'b0;
    else if (CacheValidDly[1])
        rRdEn <= 1'b1;
end

always @(posedge iGmiiRxClk) begin
    RdEnDly[1:0] <= {RdEnDly[0],rRdEn};
end

//---------------------------
//这个需要根据读写Ram的时钟确定，如果读大于写，则就不需要Half计数了，直接用读使能上升沿即可
// wire    [8:0]   wGmiiRxNumHalf        ;
// reg             RdHalf          = 1'b0;
// reg     [1:0]   RdHalfDly       = 8'd0;
// assign  wGmiiRxNumHalf[8:0] = (rGmiiRxNum[9:0] >>1);
// always @(posedge iGmiiRxClk) begin
//     if (RdAddr[9:0] == wGmiiRxNumHalf[8:0])
//         RdHalf <= 1'b1;
//     else
//         RdHalf <= 1'b0;
// end

// always @(posedge iRmiiRxClk) begin
//     RdHalfDly[1:0] <= {RdHalfDly[0],RdHalf};
// end

// always @(posedge iRmiiRxClk) begin
//     CacheCntReducePre <= (RdHalfDly[1:0] == 2'b01);
// end

// always @(posedge iGmiiRxClk) begin
//     CacheCntReduceExtend <= CacheCntReducePre||(RdHalfDly[1:0]==2'b01);
// end
always @(posedge iGmiiRxClk) begin
    CacheCntReducePreDly[3:0] = {CacheCntReducePreDly[2:0],RdEnDly[1:0] == 2'b01};    
end

always @(posedge iGmiiRxClk) begin
    CacheCntReduceExtend <= |CacheCntReducePreDly[3:0];
end
//----------------------------------
always @(posedge iGmiiRxClk) begin
    if(RdEnDly[1:0] == 2'b01)
        RdBaseAddr <= ~RdBaseAddr;
    else
        RdBaseAddr <= RdBaseAddr;
end

always @(posedge iGmiiRxClk) begin
    if(rRdEn)
        RdAddr[10:0] <= {RdAddr[10],RdAddr[9:0] + 1'b1};
    else
        RdAddr[10:0] <= {RdBaseAddr,10'h000};
end
// GMII output registers
always @(posedge iGmiiRxClk) begin
    rGmiiRxDv  <= rRdEn;
    rGmiiRxDv1 <= rGmiiRxDv;
    rGmiiRxDv2 <= rGmiiRxDv1;
    rGmiiRdData <=wRdData[7:0];
end

//******************************************************************************
// Output Assignments
//******************************************************************************
assign oGmiiRxDv   = rGmiiRxDv2;
assign oGmiiRxData = rGmiiRdData;

endmodule
