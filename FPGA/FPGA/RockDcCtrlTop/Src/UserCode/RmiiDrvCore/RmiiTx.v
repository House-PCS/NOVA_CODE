//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   RmiiTx.v
//Original Author   :   lvyb
//Creation Date     :   2024.09.12
/*Description       :   Rmii发送接口模块。

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
module RmiiTx
(
    input                           iGmiiTxClk                          ,
    input                           iGmiiTxEn                           ,
    input   [7:0]                   iGmiiTxData                         ,
    
    input                           iRmiiTxClk                          ,
    input                           iRmiiClk180Deg                      ,
    output                          oRmiiTxEn                           ,
    output  [1:0]                   oRmiiTxData                         ,
    output                          oRmiiClk                             
);

//-------------------------------Code Start Here--------------------------------
    wire               [   7: 0]        wGmiiTxData                 ;
    reg                [   1: 0]        GmiiTxEnDly               =2'd0;
    wire                                wGmiiTxEnNege               ;
    wire                                wGmiiTxEnPose               ;
    reg                [   1: 0]        GmiiTxEnNegeDly           =2'd0;
    reg                [  10: 0]        WrAddr                    =11'd0;
    reg                                 WrBaseAddr                =1'b0;
    reg                [   9: 0]        rGmiiTxCnt1               =10'd0;
    reg                [   9: 0]        rGmiiTxCnt2               =10'd0;
         
    reg                [   1: 0]        RamCacheCnt               =2'd0;
    reg                [   2: 0]        CacheCntReduceDly=3'd0      ;
    wire                                CacheCntReduce              ;
    wire                                CacheCntPlus                ;
    reg                                 CacheValid=1'b0             ;
    reg                [  11: 0]        rGmiiTxNum                =12'hFFF;
         
         
    reg                [   1: 0]        CacheValidDly             =2'd0;
    reg                                 rRdEn                     =1'b0;
    reg                [   1: 0]        RdEnDly                   =2'd0;
    reg                                 CacheCntReducePre         =1'b0;
    reg                                 CacheCntReduceExtend      =1'b0;
    reg                                 RdBaseAddr=1'b0             ;
    reg                [  12: 0]        RdAddr=13'd0                ;
    wire               [   1: 0]        wRmiiRdData                 ;
    reg                [   1: 0]        rRdEnDly                  =2'd0;
//******************************************************************************
ODDR 
#(
    .DDR_CLK_EDGE                   ("OPPOSITE_EDGE"                    ),      // "OPPOSITE_EDGE" or "SAME_EDGE" 
    .INIT                           (1'b0                               ),      // Initial value of Q: 1'b0 or 1'b1
    .SRTYPE                         ("SYNC"                             )       // Set/Reset type: "SYNC" or "ASYNC" 
)U_RmiiClk 
(
    .Q                              (oRmiiClk                           ),      // 1-bit DDR output
    .C                              (iRmiiTxClk                         ),      // 1-bit clock input
    .CE                             (1'b1                               ),      // 1-bit clock enable input
    .D1                             (1'b1                               ),      // 1-bit data input (positive edge)
    .D2                             (1'b0                               ),      // 1-bit data input (negative edge)
    .R                              (1'b0                               ),      // 1-bit reset
    .S                              (1'b0                               )       // 1-bit 
);
//******************************************************************************
//******************************************************************************
//assign wGmiiTxData[7:0] = {iGmiiTxData[1:0], iGmiiTxData[3:2], iGmiiTxData[5:4], iGmiiTxData[7:6]};

always @(posedge iGmiiTxClk) begin
    GmiiTxEnDly[1:0] <= {GmiiTxEnDly[0],iGmiiTxEn};
end

assign wGmiiTxEnNege = (GmiiTxEnDly[1:0] == 2'b10);
assign wGmiiTxEnPose = (GmiiTxEnDly[1:0] == 2'b01);

always @(posedge iGmiiTxClk) begin
    GmiiTxEnNegeDly[1:0] <= {GmiiTxEnNegeDly[0],wGmiiTxEnNege};
end

always @(posedge iGmiiTxClk) begin
    if(GmiiTxEnNegeDly[1])
        WrAddr[10:0] <= {WrBaseAddr,10'd0};
    else if(iGmiiTxEn)
        WrAddr[10:0] <= {WrAddr[10],WrAddr[9:0]+1'b1};
end

always @(posedge iGmiiTxClk) begin
    if(GmiiTxEnNegeDly[0]  && (RamCacheCnt[1:0]!=2'd2))
        WrBaseAddr <= ~WrBaseAddr;
    else
        WrBaseAddr <= WrBaseAddr;        
end

always @(posedge iGmiiTxClk) begin
    if(GmiiTxEnNegeDly[0])begin
        if(WrBaseAddr)begin
            rGmiiTxCnt1[9:0] <= rGmiiTxCnt1[9:0];
            rGmiiTxCnt2[9:0] <= WrAddr[9:0];         
        end   
        else begin
            rGmiiTxCnt1[9:0] <= WrAddr[9:0];
            rGmiiTxCnt2[9:0] <= rGmiiTxCnt2[9:0];  
        end
    end
    else begin
        rGmiiTxCnt1[9:0] <= rGmiiTxCnt1[9:0];
        rGmiiTxCnt2[9:0] <= rGmiiTxCnt2[9:0];    
    end
end

always @(posedge iGmiiTxClk) begin
    if(CacheCntReduce)begin
        if(wGmiiTxEnPose && (RamCacheCnt[1:0] == 2'd2))
            RamCacheCnt[1:0] <= 2'd0;
        else if(CacheCntPlus )
             RamCacheCnt[1:0] <= RamCacheCnt[1:0];
        else
             RamCacheCnt[1:0] <= RamCacheCnt[1:0] - 1'b1;
    end
    else if(wGmiiTxEnPose && (RamCacheCnt[1:0] == 2'd2))
        RamCacheCnt[1:0] <= 2'd1;
    else if(CacheCntPlus)
        RamCacheCnt[1:0] <= RamCacheCnt[1:0] + 1'b1;
end

always @(posedge iGmiiTxClk) begin
    CacheCntReduceDly[2:0] <= {CacheCntReduceDly[1:0],CacheCntReduceExtend};
end

assign CacheCntReduce = CacheCntReduceDly[2:1]==2'b01;
assign CacheCntPlus   = wGmiiTxEnNege;

always @(posedge iGmiiTxClk) begin
    CacheValid <= (RamCacheCnt[1:0] != 2'd0);
end

Sdpram2048x8_8192x2_D2 U_RmiiTxData(
    .clka  (iGmiiTxClk          ),
    .wea   (iGmiiTxEn           ),
    .addra (WrAddr[10:0]        ),
    .dina  (iGmiiTxData[7:0]    ),
    .clkb  (iRmiiClk180Deg      ),
    .addrb (RdAddr[12:0]        ),
    .doutb (wRmiiRdData[1:0]    )
  );
//******************************************************************************
// Input sync for GMII domain
always @(posedge iRmiiClk180Deg) begin
    if(RdEnDly[1:0]==2'b01)begin
        if (RdBaseAddr)
            rGmiiTxNum[11:0]  <= rGmiiTxCnt2 << 2;
        else
            rGmiiTxNum[11:0]  <= rGmiiTxCnt1 << 2;
    end
    else
        rGmiiTxNum[11:0]  <= rGmiiTxNum[11:0];
end

// Read control logic - improved with registered output
always @(posedge iRmiiClk180Deg) begin
    CacheValidDly[1:0] <= {CacheValidDly[0],CacheValid};
end

always @(posedge iRmiiClk180Deg) begin
    if(RdAddr[11:0] == rGmiiTxNum[11:0]-1'b1)
        rRdEn <= 1'b0;
    else if (CacheValidDly[1] && InterpacketGapOk)
        rRdEn <= 1'b1;
end

reg   [7:0]     InterpacketGapCnt = 'd0;
reg             InterpacketGapOk    = 'd0;
always @(posedge iRmiiClk180Deg) begin
    if(rRdEn)begin
        InterpacketGapCnt <= 'd0;
        InterpacketGapOk  <= 1'b0; 
    end
    else begin
        if(InterpacketGapCnt == 'd48)begin
            InterpacketGapCnt <= InterpacketGapCnt;
            InterpacketGapOk    <= 1'b1;
        end
        else begin
            InterpacketGapCnt <= InterpacketGapCnt + 1'b1;
            InterpacketGapOk    <= InterpacketGapOk;
        end
    end
end

always @(posedge iRmiiClk180Deg) begin
    RdEnDly[1:0] <= {RdEnDly[0],rRdEn};
end

//这个需要根据读写Ram的时钟确定，如果读大于写，则就不需要Half计数了，直接用读使能上升沿即可
reg             RdFinishPre          = 1'b0;
reg     [1:0]   RdFinishPreDly       = 8'd0;
reg     [2:0]   CacheCntReducePreDly = 2'd0;
always @(posedge iRmiiClk180Deg) begin
    if (RdAddr[11:0] == rGmiiTxNum[11:0]-'d16)
        RdFinishPre <= 1'b1;
    else
        RdFinishPre <= 1'b0;
end

always @(posedge iRmiiClk180Deg) begin
    RdFinishPreDly[1:0] <= {RdFinishPreDly[0],RdFinishPre};
end

always @(posedge iRmiiClk180Deg) begin
    CacheCntReducePreDly[2:0] <={CacheCntReducePreDly[1:0],(RdFinishPreDly[1:0] == 2'b01)};
end

always @(posedge iRmiiClk180Deg) begin
    CacheCntReduceExtend <= |CacheCntReducePreDly[2:0];
end

//----------------------------------
always @(posedge iRmiiClk180Deg) begin
    if(RdEnDly[1:0] == 2'b01)
        RdBaseAddr <= ~RdBaseAddr;
    else
        RdBaseAddr <= RdBaseAddr;
end

always @(posedge iRmiiClk180Deg) begin
    if(rRdEn)
        RdAddr[12:0] <= {RdAddr[12],RdAddr[11:0] + 1'b1};
    else
        RdAddr[12:0] <= {RdBaseAddr,12'h000};
end
//******************************************************************************

//******************************************************************************
always @(posedge iRmiiClk180Deg) begin
    rRdEnDly[1:0] <= {rRdEnDly[0],rRdEn};
end

assign oRmiiTxEn = rRdEnDly[1];
assign oRmiiTxData[1:0] = wRmiiRdData[1:0];

//******************************************************************************

endmodule
