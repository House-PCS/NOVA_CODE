`timescale 1ns/1ps

module RmiiCoreTb();

// 时钟参数
parameter RMII_CLK_PERIOD = 20;   // 50MHz
parameter GMII_CLK_PERIOD = 10;   // 100MHz

// 信号声明
reg         iRmiiRxClk;
reg         rRmiiRxDv[0:2];
reg  [1:0]  iRmiiRxData;
wire        oGmiiRxDv;
wire [7:0]  oGmiiRxData;
reg         iGmiiRxClk;

reg         rst_n;

reg [7:0]   rSendBuff;
reg [15:0]  rPeriodCnt;
reg [1:0]   rBitCnt;
reg [15:0]  rByteCnt;

reg [7:0]   rRevBuff;
reg [3:0]   rRevCnt;
// 实例化被测模块
RmiiRx DUT (
    .iRmiiRxClk     (iRmiiRxClk         ),
    .iRmiiRxDv      (rRmiiRxDv[1]       ),
    .iRmiiRxData    (iRmiiRxData[1:0]   ),
    .iGmiiRxClk     (iGmiiRxClk         ),
    .oGmiiRxDv      (oGmiiRxDv          ),
    .oGmiiRxData    (oGmiiRxData[7:0]   )
);

// 时钟生成
initial begin
    iRmiiRxClk = 0;
    forever #(RMII_CLK_PERIOD/2) iRmiiRxClk = ~iRmiiRxClk;
end

initial begin
    iGmiiRxClk = 0;
    forever #(GMII_CLK_PERIOD/2) iGmiiRxClk = ~iGmiiRxClk;
end

// reg [7:0] rTestData1 = 8'h55;
// reg [7:0] rTestData2 = 8'h55;
// reg [7:0] rTestData3 = 8'h55;
// reg [7:0] rTestData4 = 8'h55;
// reg [7:0] rTestData5 = 8'h55;
// reg [7:0] rTestData6 = 8'h55;
// reg [7:0] rTestData7 = 8'h55;
// reg [7:0] rTestData8 = 8'hd5;
// reg [7:0] rTestData  = 8'd00;

//周期计数，一个周期进行一次写入测试
always @(posedge iRmiiRxClk or negedge rst_n) begin
    if(!rst_n) 
        rPeriodCnt <= 16'd0;
    else begin
        if(rPeriodCnt < 16'd1000)
            rPeriodCnt <= rPeriodCnt + 16'd1;
        else
            rPeriodCnt <= 16'd0;  
    end
end  

// always @(posedge iRmiiRxClk ) begin
//     if(rPeriodCnt[1:0] == 2'b11)
//         rTestData[7:0] <= 
// end

// always @(posedge iRmiiRxClk ) begin
//     case(rPeriodCnt[1:0])
//         00: iRmiiRxData[1:0] <= 
//         01:
//         10:
//         11:
//     endcase
// end  



//设置发送使能信号
always @(posedge iRmiiRxClk or negedge rst_n) begin
    if(!rst_n) begin
        rRmiiRxDv[1] <= 1'b0;
        rRmiiRxDv[2] <= 1'b0;
    end
        
    else begin
        rRmiiRxDv[1] <= rRmiiRxDv[0];
        rRmiiRxDv[2] <= rRmiiRxDv[1];
    end
end  

//对fifo_wr_en赋值，当FIFO为空时开始写入，写满后停止写
always @(posedge iRmiiRxClk or negedge rst_n) begin
    if(!rst_n) begin
        rRmiiRxDv[0] <= 1'b0;
    end
        
    else begin
        if(rPeriodCnt == 16'd100)
            rRmiiRxDv[0] <= 1'b1;
        else if(rPeriodCnt == 16'd200)
            rRmiiRxDv[0] <= 1'b0; 
        // else if(rPeriodCnt == 16'd208)
        //     rRmiiRxDv[0] <= 1'b1;
        // else if(rPeriodCnt == 16'd308)
        //     rRmiiRxDv[0] <= 1'b0; 
    end
end  

//FIFO写入操作（设置数据的使能信号要比输入FIFO的使能信号早一帧，保证输入使能信号拉高时数据已准备好，否则会出现输入错位的情况）
always @(posedge iRmiiRxClk or negedge rst_n) begin
    if(!rst_n) begin
        iRmiiRxData[1:0] <= 2'b0;
        rSendBuff <= 8'h66;
        rBitCnt <= 2'd0;
        rByteCnt <= 16'd0;
    end
    else if(rRmiiRxDv[0] == 1'b1) begin    
        iRmiiRxData[1:0] <= rSendBuff[(rBitCnt*2) +:2];
        rBitCnt <= rBitCnt + 2'd1;
        if(rBitCnt == 2'd3) begin
            rSendBuff <= rSendBuff + 8'd1;
            rByteCnt <= rByteCnt + 16'd1;
        end
    end
    else begin
        iRmiiRxData[1:0] <= 2'b0;
        rSendBuff <= 8'h66;
        rBitCnt <= 2'd0;
        rByteCnt <= 16'd0;
    end
        
end

always @(posedge iGmiiRxClk or negedge rst_n) begin
    if(!rst_n) begin
        rRevBuff <= 8'd0;
        rRevCnt  <= 2'd0;
    end
    else if(oGmiiRxDv) begin
        rRevBuff <= oGmiiRxData[7:0];
        rRevCnt <= rRevCnt + 8'd1;
    end
    else begin
        rRevBuff <= 8'd0;
        rRevCnt <= 8'd0;
        
    end
end

// 复位信号
initial begin
    rst_n = 0;
    #100;
    rst_n = 1;
end



endmodule