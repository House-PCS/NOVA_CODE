module MonitorDataRead (
    input                               iSysClk                    ,
    input                               iRstP                      ,
    input                               iDdrMonitorDataVld         ,

    input                               iDdrCmdClk                 ,
    output                              oDdrRdBegin                ,
    input                               iDdrRdRdy                  , 
    output               [   4: 0]      oDdrRdBufBaseAddr          ,     
    output                              oDdrRdBufSel               , 
    output               [   8: 0]      oDdrRdLen                  ,
    output               [  25: 0]      oDdrRdAddr                 ,

    input                               iDdrDataClk                ,
    input                [   8: 0]      iDdrRdDataAddr             ,
    input                               iDdrRdDataEn               ,
    input                [ 127: 0]      oDdrRdData                 
);

    reg                                 rDdrRdBegin                ;
    reg                  [  15: 0]      RdCnt='d0                  ;
    reg                  [  22: 0]      rDdrRdAddr='d0             ;
    wire                                wDdrMonitorDataVldPose     ;
    reg                                 rDdrMonitorDataVld=1'b0    ;

always@(posedge iDdrCmdClk)
    rDdrMonitorDataVld <= iDdrMonitorDataVld;

assign  wDdrMonitorDataVldPose = iDdrMonitorDataVld & (~rDdrMonitorDataVld);

always@(posedge iDdrCmdClk)begin
    if(rDdrRdBegin & iDdrRdRdy)
        rDdrRdBegin <= 1'b0;
    else if(wDdrMonitorDataVldPose)
        rDdrRdBegin <= 1'b1;
end

always@(posedge iDdrDataClk)begin
    if(iDdrRdDataEn & (iDdrRdDataAddr[8:0]=='d0))
        RdCnt[15:0] <= RdCnt[15:0] + 1'b1;
end

always @(posedge iDdrCmdClk)begin
    if(iRstP)
        rDdrRdAddr[22:0] <= 24'd0;
    else if(rDdrRdBegin & iDdrRdRdy)begin
        if(rDdrRdAddr[22:0]== 'd720)
            rDdrRdAddr[22:0]<= 24'd0;
        else
            rDdrRdAddr[22:0]<= rDdrRdAddr[22:0] + 'd72;
    end
    else
        rDdrRdAddr[22:0]<= rDdrRdAddr[22:0];
    
end

assign oDdrRdBegin = rDdrRdBegin;
assign oDdrRdLen = 'd9;
assign oDdrRdBufSel = 1'b0;
assign oDdrRdBufBaseAddr[4:0] = 'd0;
//assign oDdrRdAddr[25:0] = {3'b001,rDdrRdAddr[22:0]};
assign oDdrRdAddr[25:0] = {3'b001,23'd0};
endmodule