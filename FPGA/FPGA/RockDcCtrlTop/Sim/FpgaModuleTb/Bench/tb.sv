`timescale 1ns/1ps
////////////////////////////////////////////////////////////////////////////////////////////////////
// Company        : NOVASTAR
// Author         : LiSong
// Create Date    : 2021-08-28
// Module Name    : tb
// Project Name   : 
// EDA Tool       : 
// Target Devices : 
// Description    : 
// Revision       : Initial v1.0
// Comment        : 
////////////////////////////////////////////////////////////////////////////////////////////////////
// Modify by      : 
// Date           : 
// Revision       : 
// Comment        : 
////////////////////////////////////////////////////////////////////////////////////////////////////

module  tb;

localparam GT_DRP_CLK_PERIOD    = 40   ; //50M
localparam GT_REF_CLK_PERIOD    = 9.704; //103.125M
localparam PHY_CLK_PERIOD       = 8    ; //125M

localparam PORT_NOP             = 2'd0 ;//port data unvalid
localparam PORT_POS             = 2'd1 ;//port posedge
localparam PORT_NEG             = 2'd2 ;//port negedge
localparam PORT_VLD             = 2'd3 ;//port valid
localparam PORT_NUM             = 4'd5 ;//port number
localparam DATA_HEAD            = 2'd1 ;//data header for 64/66
localparam PARA_HEAD            = 2'd2 ;//parameter header for 64/66
localparam GT_WIDTH             = 8'd32;//gt data width

localparam GAP_LEN              =15;
localparam SFD_LEN              =8;
localparam DAT_LEN              =66;
localparam PKG_NUM              =50;

localparam RX_PATH0             ="./DataFile/DutRxData0.txt";
localparam RX_PATH1             ="./DataFile/DutRxData1.txt";
localparam RX_PATH2             ="./DataFile/DutRxData2.txt";
localparam RX_PATH3             ="./DataFile/DutRxData3.txt";
localparam RX_PATH4             ="./DataFile/DutRxData4.txt";

localparam TX_PATH0             ="./DataFile/DutTxData0.txt";
localparam TX_PATH1             ="./DataFile/DutTxData1.txt";
localparam TX_PATH2             ="./DataFile/DutTxData2.txt";
localparam TX_PATH3             ="./DataFile/DutTxData3.txt";
localparam TX_PATH4             ="./DataFile/DutTxData4.txt";

////////////////////////////////////////////////////////////////////////////////////////////////////
//logic
////////////////////////////////////////////////////////////////////////////////////////////////////

logic      [PORT_NUM-1:0]      InGmiiTxClk         ;
logic      [PORT_NUM-1:0]      InGmiiTxEn          ;
logic      [PORT_NUM*8-1:0]    InGmiiTxData        ;
logic      [PORT_NUM-1:0]      InGmiiTxLink        ;
logic      [PORT_NUM-1:0]      InGmiiRxClk         ;
logic      [PORT_NUM-1:0]      OutGmiiRxEn         ;
logic      [PORT_NUM*8-1:0]    OutGmiiRxData       ;
logic      [PORT_NUM-1:0]      OutGmiiRxLink       ;
logic                          InGtTxClk           ;
logic                          InGtTxRstDone       ;
logic      [6:0]               OutGtTxSeq          ;
logic      [1:0]               OutGtTxHead         ;
logic      [GT_WIDTH-1:0]      OutGtTxData         ;
logic                          InGtRxClk           ;
logic                          InGtRxRstDone       ;
logic                          InGtRxHeadVld       ;
logic      [1:0]               InGtRxHead          ;
logic                          InGtRxDataVld       ;
logic      [GT_WIDTH-1:0]      InGtRxData          ;
logic                          OutGtPkgVld         ;
logic                          OutGtRxGearBoxSlip  ;

logic                          GtDrpClk            ;
logic                          GtRefClkP           ;
logic                          GtRefClkN           ;
logic                          PhyClk              ;
logic                          GtTxP               ;
logic                          GtTxN               ;
logic                          GtRxP               ;
logic                          GtRxN               ;

initial begin
    GtDrpClk = 0;
    GtRefClkP= 0;
    GtRefClkN= 1;
    PhyClk   = 0;
end

always #(GT_DRP_CLK_PERIOD/2)   GtDrpClk= ~GtDrpClk;
always #(GT_REF_CLK_PERIOD/2)   GtRefClkP= ~GtRefClkP;
always #(GT_REF_CLK_PERIOD/2)   GtRefClkN= ~GtRefClkN;
always #(PHY_CLK_PERIOD/2)      PhyClk  = ~PhyClk;

assign GtRxP = GtTxP;
assign GtRxN = GtTxN;

integer FileRx [PORT_NUM-1:0];
integer FileTx [PORT_NUM-1:0];

initial begin
    FileRx[0]=$fopen(RX_PATH0,"w");
    FileRx[1]=$fopen(RX_PATH1,"w");
    FileRx[2]=$fopen(RX_PATH2,"w");
    FileRx[3]=$fopen(RX_PATH3,"w");
    FileRx[4]=$fopen(RX_PATH4,"w");

    FileTx[0]=$fopen(TX_PATH0,"w");
    FileTx[1]=$fopen(TX_PATH1,"w");
    FileTx[2]=$fopen(TX_PATH2,"w");
    FileTx[3]=$fopen(TX_PATH3,"w");
    FileTx[4]=$fopen(TX_PATH4,"w");
end

////////////////////////////////////////////////////////////////////////////////////////////////////
//generate data
////////////////////////////////////////////////////////////////////////////////////////////////////

logic    [15:0]  RgmiiCnt    ;
logic            HeadVld     ;
logic    [7:0]   Head        ;
logic            DataVld     ;
logic    [7:0]   Data        ;
logic            rHeadVld    ;
logic            rDataVld    ;
logic    [7:0]   rData       ;
logic            GmiiVld     ;
logic    [7:0]   GmiiData    ;
logic           rOutGmiiRxEn ;
logic   [15:0]  PkgNum       ;
logic           FileWrDone   ;

logic LocalRst;
initial begin
    LocalRst=1;
    #990;
    LocalRst=0;
end


always @(posedge PhyClk)
    if(LocalRst)
        RgmiiCnt <= 'd0;
    else if(RgmiiCnt == (GAP_LEN+SFD_LEN+DAT_LEN)-1)
        RgmiiCnt <= 'd0;
    else
        RgmiiCnt <= RgmiiCnt+'d1;

always @(posedge PhyClk)
    if(RgmiiCnt == GAP_LEN-1)
        HeadVld<='d1;
    else if(RgmiiCnt == (GAP_LEN+SFD_LEN)-1)
        HeadVld<='d0;
    else
        HeadVld<=HeadVld;

always @(posedge PhyClk)
    if(RgmiiCnt == (GAP_LEN+SFD_LEN)-1)
        DataVld<='d1;
    else if(RgmiiCnt == (GAP_LEN+SFD_LEN+DAT_LEN)-1)
        DataVld<='d0;
    else
        DataVld<=DataVld;

always @(posedge PhyClk)
    if(RgmiiCnt == (GAP_LEN+SFD_LEN)-1)
        Head<='hD5;
    else
        Head<='h55;

always @(posedge PhyClk)
    if(DataVld)begin
        Data[3:0]<=Data[3:0]+'h2;
        Data[7:4]<=Data[7:4]+'h2;
    end
    else begin
        Data[3:0]<='h0;
        Data[7:4]<='h1;
    end

always @(posedge PhyClk)begin
    rHeadVld<=HeadVld;
    rDataVld<=DataVld;
    rData   <=Data   ;
end

always @(posedge PhyClk)begin
    GmiiVld <=rHeadVld | rDataVld;
    GmiiData<=rHeadVld ? Head  :
              rDataVld ? rData : 'h0;
end

assign InGmiiTxClk  ={PORT_NUM{PhyClk}};
assign InGmiiTxEn   ={PORT_NUM{GmiiVld}};
assign InGmiiTxData ={PORT_NUM{GmiiData}};
assign InGmiiTxLink ={PORT_NUM{1'b1}};

assign InGmiiRxClk  = {PORT_NUM{PhyClk}};

////////////////////////////////////////////////////////////////////////////////////////////////////
//write data
////////////////////////////////////////////////////////////////////////////////////////////////////

genvar i;
generate
    for(i=0;i<PORT_NUM;i=i+1)begin

//write tx data
always @(posedge PhyClk)
    if(FileWrDone)
        $fclose(FileTx[i]);
    else if(PkgNum>10)begin
        if(InGmiiTxEn[i])
            $fwrite(FileTx[i],"%h\n",InGmiiTxData[i*8+:8]);
    end

//write rx data
always @(posedge PhyClk)
    if(FileWrDone)
        $fclose(FileRx[i]);
    else if(PkgNum>10)begin
        if(OutGmiiRxEn[i])
            $fwrite(FileRx[i],"%h\n",OutGmiiRxData[i*8+:8]);
    end


    end
endgenerate

logic           ChkRxEn0;
logic           ChkRxEn1;
logic           ChkRxEn2;
logic           ChkRxEn3;
logic           ChkRxEn4;
logic   [7:0]   ChkRxData0;
logic   [7:0]   ChkRxData1;
logic   [7:0]   ChkRxData2;
logic   [7:0]   ChkRxData3;
logic   [7:0]   ChkRxData4;

assign ChkRxEn0 = OutGmiiRxEn[0];
assign ChkRxEn1 = OutGmiiRxEn[1];
assign ChkRxEn2 = OutGmiiRxEn[2];
assign ChkRxEn3 = OutGmiiRxEn[3];
assign ChkRxEn4 = OutGmiiRxEn[4];

assign ChkRxData0 = OutGmiiRxData[0*8+:8];
assign ChkRxData1 = OutGmiiRxData[1*8+:8];
assign ChkRxData2 = OutGmiiRxData[2*8+:8];
assign ChkRxData3 = OutGmiiRxData[3*8+:8];
assign ChkRxData4 = OutGmiiRxData[4*8+:8];

logic           ChkTxEn0;
logic           ChkTxEn1;
logic           ChkTxEn2;
logic           ChkTxEn3;
logic           ChkTxEn4;
logic   [7:0]   ChkTxData0;
logic   [7:0]   ChkTxData1;
logic   [7:0]   ChkTxData2;
logic   [7:0]   ChkTxData3;
logic   [7:0]   ChkTxData4;

assign ChkTxEn0 = InGmiiTxEn[0];
assign ChkTxEn1 = InGmiiTxEn[1];
assign ChkTxEn2 = InGmiiTxEn[2];
assign ChkTxEn3 = InGmiiTxEn[3];
assign ChkTxEn4 = InGmiiTxEn[4];

assign ChkTxData0 = InGmiiTxData[0*8+:8];
assign ChkTxData1 = InGmiiTxData[1*8+:8];
assign ChkTxData2 = InGmiiTxData[2*8+:8];
assign ChkTxData3 = InGmiiTxData[3*8+:8];
assign ChkTxData4 = InGmiiTxData[4*8+:8];

////////////////////////////////////////////////////////////////////////////////////////////////////
//check
////////////////////////////////////////////////////////////////////////////////////////////////////

always @(posedge PhyClk)
    rOutGmiiRxEn<=OutGmiiRxEn[0];

always @(posedge PhyClk)
    if((!InGtRxRstDone) | (!InGtTxRstDone))
        PkgNum<='d0;
    else if(!rOutGmiiRxEn & OutGmiiRxEn[0])
        PkgNum <= PkgNum+'d1;

initial begin
    FileWrDone=0;
    #1000;
    wait(InGtRxRstDone);
    $display("//=============================");
    $display("//>>gt0 rx reset done!");
    wait(InGtTxRstDone);
    $display("//>>gt0 tx reset done!");
    $display("//=============================");
    wait(PkgNum == PKG_NUM+1);
    FileWrDone=1;
    #1000;
    $display("//=============================");
    $display("//>>rx package number is %d ",PkgNum);
    $display("//=============================");
    $stop;
end

////////////////////////////////////////////////////////////////////////////////////////////////////

FiberCvrt64B66BTop #(
    .PORT_NOP                       ( PORT_NOP                      ),
    .PORT_POS                       ( PORT_POS                      ),
    .PORT_NEG                       ( PORT_NEG                      ),
    .PORT_VLD                       ( PORT_VLD                      ),
    .PORT_NUM                       ( PORT_NUM                      ),
    .DATA_HEAD                      ( DATA_HEAD                     ),
    .PARA_HEAD                      ( PARA_HEAD                     ),
    .GT_WIDTH                       ( GT_WIDTH                      )
)
FiberCvrt64B66BTopInst(
    .InGmiiTxClk                    ( InGmiiTxClk                   ),
    .InGmiiTxEn                     ( InGmiiTxEn                    ),
    .InGmiiTxData                   ( InGmiiTxData                  ),
    .InGmiiTxLink                   ( InGmiiTxLink                  ),
    .InGmiiRxClk                    ( InGmiiRxClk                   ),
    .OutGmiiRxEn                    ( OutGmiiRxEn                   ),
    .OutGmiiRxData                  ( OutGmiiRxData                 ),
    .OutGmiiRxLink                  ( OutGmiiRxLink                 ),
    .InGtTxClk                      ( InGtTxClk                     ),
    .InGtTxRstDone                  ( InGtTxRstDone                 ),
    .OutGtTxSeq                     ( OutGtTxSeq                    ),
    .OutGtTxHead                    ( OutGtTxHead                   ),
    .OutGtTxData                    ( OutGtTxData                   ),
    .InGtRxClk                      ( InGtRxClk                     ),
    .InGtRxRstDone                  ( InGtRxRstDone                 ),
    .InGtRxHeadVld                  ( InGtRxHeadVld                 ),
    .InGtRxHead                     ( InGtRxHead                    ),
    .InGtRxDataVld                  ( InGtRxDataVld                 ),
    .InGtRxData                     ( InGtRxData                    ),
    .OutGtPkgVld                    ( OutGtPkgVld                   ),
    .OutGtRxGearBoxSlip             ( OutGtRxGearBoxSlip            )
);

logic GtRst;
initial begin
    GtRst = 1;
    #10000;
    GtRst = 0;
end

Hsst5GCore Hsst5GCoreInst
(
    .InSysClk          (GtDrpClk           ),   //25Mhz
    .InSysRst          (GtRst              ),
    .InGtRefClkP       (GtRefClkP          ), //103.0125Mhz
    .InGtRefClkN       (GtRefClkN          ),
    .InGtRxP           ({4{GtRxP}}              ),
    .InGtRxN           ({4{GtRxN}}              ),   
    .OutGtTxP          (GtTxP              ),
    .OutGtTxN          (GtTxN              ),
    .OutGtTxClk        (InGtTxClk          ), //161M
    .OutGtTxRstDone    (InGtTxRstDone      ), 
    .InGtTxHead        ({4{1'b0,OutGtTxHead  }} ),
    .InGtTxSeq         ({4{OutGtTxSeq        }} ),
    .InGtTxData        ({4{OutGtTxData       }} ),
    .InGtRxGearBoxSlip ({4{OutGtRxGearBoxSlip}} ),
    .OutGtRxClk        (InGtRxClk          ),  //161M 
    .OutGtRxRstDone    (InGtRxRstDone      ),      
    .OutGtRxData       (InGtRxData         ),
    .OutGtRxDataVld    (InGtRxDataVld      ),
    .OutGtRxHead       (InGtRxHead         ),
    .OutGtRxHeadVld    (InGtRxHeadVld      )
    
);

endmodule
