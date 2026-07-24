module MonitorDataStorage(
    input                               iSysClk                    ,
    input                               iRstP                      ,
    //监控数据
    input               [32*36-1:0]     iMonitorData               ,
    input                               iMonitorDataVld            ,
    input                               iAdcSampleSequenceDone     ,
    input                               iDcDcErr                   ,
    //写DDR命令
    output                              oDdrCmdClk                 ,
    output                              oDdrWrBegin                ,
    input                               iDdrWrRdy                  , 
    output               [   6: 0]      oDdrWrBufBaseAddr          ,     
    output                              oDdrWrBufSel               , 
    output               [   8: 0]      oDdrWrLen                  ,
    output               [  25: 0]      oDdrWrAddr                 ,
    //写DDR数据
    input                               iDdrDataClk                ,
    input                [   8: 0]      iDdrWrDataAddr             ,
    input                               iDdrWrDataAddrEn           ,
    output               [  15: 0]      oDdrWrMask                 ,
    output               [ 127: 0]      oDdrWrData                 ,
    //软核通信
    input                [   7: 0]      iBmsShutdown               ,
    output               [  25: 0]      oMcuRdStartAddr            ,
    output                              oMcuFlashBurnEn            ,
    input                               iMcuFaultBurnFinish    

);
    reg                  [   6: 0]      rMonitorDataWrRamAddr='d0  ;
    reg                                 rDdrWrBegin='d0            ;
    reg                  [   4: 0]      rDdrWrBufBaseAddr='d0      ;
    reg                  [  22: 0]      rDdrWrAddr='d0             ;
    reg                  [   1: 0]      rDcDcErrAfterStorageCnt=2'b00  ;
    reg                                 rMonitorDataVld='d0         ;
    wire                                wMonitorDataVldPose         ;
    reg                                 rDataStorageRamWrEnPre   ='d0;
    reg                                 rDataStorageRamWrEn      ='d0;
    reg                  [   5: 0]      rDataStorageRamWrCnt     ='d0;
    reg                  [  31: 0]      rMonitorData             ='d0;
    reg                                 rDcDcErr                 =1'b0;
    wire                                wDcDcErrPose               ;
    reg                                 rMcuFaultBurnFinish      =1'b0;
    reg                                 rDcDcErrEn               =1'b0;
    wire                                wMcuFaultBurnFinishPose    ;

always@(posedge iSysClk)begin
    rMonitorDataVld <= iMonitorDataVld;
end

assign wMonitorDataVldPose = (!rMonitorDataVld) & iMonitorDataVld;

always @(posedge oDdrCmdClk) begin
    if(iRstP)
        rMonitorDataWrRamAddr[6:0] <= 7'd0;
    else if(rDataStorageRamWrEn)begin
        if(rMonitorDataWrRamAddr[6:0] == 7'd71)
            rMonitorDataWrRamAddr[6:0] <= 7'd0;
        else
            rMonitorDataWrRamAddr[6:0] <= rMonitorDataWrRamAddr[6:0] + 1'b1;            
    end
    else
        rMonitorDataWrRamAddr[6:0] <= rMonitorDataWrRamAddr[6:0];
end

always@(posedge iSysClk)begin
    if(rDataStorageRamWrCnt[5:0] == 'd35)  
        rDataStorageRamWrEnPre <= 1'b0;
    else if(wMonitorDataVldPose)
        rDataStorageRamWrEnPre <= 1'b1;
    else 
        rDataStorageRamWrEnPre <= rDataStorageRamWrEnPre;        
end

always@(posedge iSysClk)begin
    rDataStorageRamWrEn <= rDataStorageRamWrEnPre;
end

always@(posedge iSysClk)begin
    if(rDataStorageRamWrEnPre)
        rDataStorageRamWrCnt[5:0] <= rDataStorageRamWrCnt[5:0] + 1'b1;
    else
        rDataStorageRamWrCnt[5:0] <= 'd0;        
end

always@(posedge iSysClk)begin
    rMonitorData[31:0] <= iMonitorData[32*rDataStorageRamWrCnt+:32];
end


always @(posedge oDdrCmdClk) begin
    if(rDcDcErrAfterStorageCnt[1:0] == 2'b11)
        rDdrWrBegin <= 1'b0;         
    else if(rDdrWrBegin & iDdrWrRdy)
        rDdrWrBegin <= 1'b0;        
    else if(rDataStorageRamWrEn & ( (rMonitorDataWrRamAddr[6:0] == 'd35) || (rMonitorDataWrRamAddr[6:0] == 'd71) ) )
        rDdrWrBegin <= 1'b1;
    else 
        rDdrWrBegin <= rDdrWrBegin;
end                                        


always @(posedge oDdrCmdClk) begin
    if(rDataStorageRamWrEn & (rMonitorDataWrRamAddr[6:0] == 'd35))
        rDdrWrBufBaseAddr[4:0] <= 5'd0;
    else if(rDataStorageRamWrEn & (rMonitorDataWrRamAddr[6:0] == 'd71))
        rDdrWrBufBaseAddr[4:0] <= 5'd9;
    else
        rDdrWrBufBaseAddr[4:0] <= rDdrWrBufBaseAddr[4:0];        
end
// DDR:1Gb , x16 ,row:13 , col:10 , bank:3
always @(posedge oDdrCmdClk) begin
    if(iRstP)
        rDdrWrAddr[22:0] <= 23'd0;        
    else if(rDdrWrBegin & iDdrWrRdy)begin
        if(rDdrWrAddr[22:0]== 'd294840)
            rDdrWrAddr[22:0]<= 23'd0; 
        else
            rDdrWrAddr[22:0]<= rDdrWrAddr[22:0] + 'd72;    
    end
    else
        rDdrWrAddr[22:0]<= rDdrWrAddr[22:0];   
end

assign oDdrCmdClk = iSysClk;
assign oDdrWrBegin = rDdrWrBegin;
assign oDdrWrLen[8:0] = 9'd9;
assign oDdrWrBufSel   = 1'b0;
assign oDdrWrBufBaseAddr[4:0] = rDdrWrBufBaseAddr[4:0];
assign oDdrWrAddr[25:0] = {3'h001,rDdrWrAddr[22:0]};
assign oDdrWrMask[15:0] = 16'h0000;

Sdpram72x32_18x128_D2 MonitorDataStorageRam(
    .clka                               (iSysClk                   ),
    .wea                                (rDataStorageRamWrEn       ),
    .addra                              (rMonitorDataWrRamAddr[6:0]),
    .dina                               (rMonitorData[31:0]        ),
    .clkb                               (iDdrDataClk               ),
    .addrb                              (iDdrWrDataAddr[6:0]       ),
    .doutb                              (oDdrWrData                ) 
);
always@(posedge oDdrCmdClk)begin
    rDcDcErr <= iDcDcErr;
end
assign wDcDcErrPose = (!rDcDcErr) & iDcDcErr;

always@(posedge oDdrCmdClk)begin
    rMcuFaultBurnFinish <= iMcuFaultBurnFinish;
end
assign wMcuFaultBurnFinishPose = (!rMcuFaultBurnFinish) & iMcuFaultBurnFinish;

always@(posedge oDdrCmdClk)begin
    if(wMcuFaultBurnFinishPose)
        rDcDcErrEn <= 1'b0;
    else if(wDcDcErrPose)
        rDcDcErrEn <= 1'b1; 
    else
        rDcDcErrEn <= rDcDcErrEn;          
end

always @(posedge oDdrCmdClk)begin
    if(rDcDcErrEn && (iBmsShutdown[1:0] != 2'd2 || iBmsShutdown[3:2] != 2'd2) )begin
        if(rDdrWrBegin & iDdrWrRdy)begin
            if(rDcDcErrAfterStorageCnt[1:0] == 2'b11)
                rDcDcErrAfterStorageCnt[1:0] <= rDcDcErrAfterStorageCnt[1:0];
            else
                rDcDcErrAfterStorageCnt[1:0] <= rDcDcErrAfterStorageCnt[1:0] + 1'b1;
        end
        else
            rDcDcErrAfterStorageCnt[1:0] <= rDcDcErrAfterStorageCnt[1:0]; 
    end
    else
        rDcDcErrAfterStorageCnt[1:0] <= 2'd0;
end

assign oMcuRdStartAddr[25:0]  = oDdrWrAddr[25:0];
                                  
assign oMcuFlashBurnEn = (rDcDcErrAfterStorageCnt[1:0] == 2'b11);

endmodule