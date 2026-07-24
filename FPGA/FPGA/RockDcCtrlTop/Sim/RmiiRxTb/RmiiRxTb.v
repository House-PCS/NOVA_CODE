`timescale 1ns / 1ns
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/04/11 23:03:42
// Design Name: 
// Module Name: RmiiRxTb
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


// module RmiiRxTb ();

// `timescale 1ns/1ns

module RmiiRxTb();

reg iRmiiRxClk;
reg iGmiiRxClk;
reg iRmiiRxDv;
reg [1:0] iRmiiRxData;

wire oGmiiRxDv;
wire [7:0] oGmiiRxData;

// 实例化被测模块
RmiiRx uut (
    .iRmiiRxClk(iRmiiRxClk),
    .iRmiiRxDv(iRmiiRxDv),
    .iRmiiRxData(iRmiiRxData),
    .iGmiiRxClk(iGmiiRxClk),
    .oGmiiRxDv(oGmiiRxDv),
    .oGmiiRxData(oGmiiRxData)
);

// 生成RMII时钟（50MHz）
initial begin
    iRmiiRxClk = 0;
    forever #4 iRmiiRxClk = ~iRmiiRxClk;
end

// 生成GMII时钟（125MHz）
initial begin
    iGmiiRxClk = 0;
    forever #4 iGmiiRxClk = ~iGmiiRxClk;
end

// 测试过程
initial begin
    // 初始化输入信号
    iRmiiRxDv = 0;
    iRmiiRxData = 2'b00;
    #100; // 等待系统稳定

    // 发送测试帧：7个前导码0x55 + SFD 0xD5 + 4个数据字节
 //   send_frame();
send_Multframe();
    // 等待足够时间确保数据处理完成
    #1000;
    $finish;
end

task send_Multframe;
begin
     repeat(2) send_frame1();
      @(posedge iRmiiRxClk) ;
       #100;
      send_frame3();
           @(posedge iRmiiRxClk) ;
       #100;
      send_frame2();
      send_frame4();
      send_frame5();
           @(posedge iRmiiRxClk) ;
       #100;
     repeat(2) send_frame1();
end
endtask

// 发送一个完整帧的任务
task send_frame1();
    integer i;
    begin
        // 发送前导码（7个0x55）
        @(posedge iRmiiRxClk) ;
        repeat(7) send_byte(8'h55);

        // 发送SFD（0xD5）
         send_byte(8'hd5);
        // // 发送数据部分（0xAA, 0xBB, 0xCC, 0xDD）
        repeat(5) send_byte(8'h00);
        repeat(4) send_byte(8'h11);
        repeat(3) send_byte(8'h22);
        repeat(2) send_byte(8'h33);
        repeat(1) send_byte(8'h44);//13
        // 结束传输，拉低数据有效信号
         @(posedge iRmiiRxClk) ;
        iRmiiRxDv <= 0;
      #100;
    end
endtask

task send_frame2();
    integer i;
    begin
        // 发送前导码（7个0x55）
        @(posedge iRmiiRxClk) ;
        repeat(7) send_byte(8'h55);

        // 发送SFD（0xD5）
         send_byte(8'hd5);
        // // 发送数据部分（0xAA, 0xBB, 0xCC, 0xDD）
        repeat(10)send_byte(8'h22);
        send_byte(8'h33);
        send_byte(8'h44);
        // 结束传输，拉低数据有效信号
         @(posedge iRmiiRxClk) ;
        iRmiiRxDv <= 0;
      #100;
    end
endtask

task send_frame4();
    integer i;
    begin
        // 发送前导码（7个0x55）
        @(posedge iRmiiRxClk) ;
        repeat(7) send_byte(8'h55);

        // 发送SFD（0xD5）
         send_byte(8'hd5);
        // // 发送数据部分（0xAA, 0xBB, 0xCC, 0xDD）
        repeat(10)send_byte(8'h33);
        send_byte(8'h44);
        send_byte(8'h55);
        // 结束传输，拉低数据有效信号
         @(posedge iRmiiRxClk) ;
        iRmiiRxDv <= 0;
      #100;
    end
endtask

task send_frame5();
    integer i;
    begin
        // 发送前导码（7个0x55）
        @(posedge iRmiiRxClk) ;
        repeat(7) send_byte(8'h55);

        // 发送SFD（0xD5）
         send_byte(8'hd5);
        // // 发送数据部分（0xAA, 0xBB, 0xCC, 0xDD）
        repeat(10)send_byte(8'h44);
        send_byte(8'h55);
        send_byte(8'h66);
        send_byte(8'h77);
        // 结束传输，拉低数据有效信号
         @(posedge iRmiiRxClk) ;
        iRmiiRxDv <= 0;
      #100;
    end
endtask

task send_frame3();
    integer i;
    begin
        // 发送前导码（7个0x55）
        @(posedge iRmiiRxClk) ;
        repeat(7) send_byte(8'h55);

        // 发送SFD（0xD5）
         send_byte(8'hd5);
        // // 发送数据部分（0xAA, 0xBB, 0xCC, 0xDD）
        repeat(40)send_byte(8'h11);
        repeat(40)send_byte(8'h22);
        repeat(40)send_byte(8'h33);
        repeat(40)send_byte(8'h44);
        repeat(40)send_byte(8'h55);
        // 结束传输，拉低数据有效信号
         @(posedge iRmiiRxClk) ;
        iRmiiRxDv <= 0;
      #100;
    end
endtask
// 发送单个字节的任务（分解为4个RMII周期）
task send_byte(input [7:0] data);
    reg [1:0] d0, d1, d2, d3;
    begin
        d0 = data[1:0]; // 第1个两位：D1 D0
        d1 = data[3:2]; // 第2个两位：D3 D2
        d2 = data[5:4]; // 第3个两位：D5 D4
        d3 = data[7:6]; // 第4个两位：D7 D6
        
        // 按顺序发送四个两位数据
        @(posedge iRmiiRxClk) 
            iRmiiRxData <= d0;
            iRmiiRxDv   <= 1'b1;

        @(posedge iRmiiRxClk) iRmiiRxData <= d1;
        @(posedge iRmiiRxClk) iRmiiRxData <= d2;
        @(posedge iRmiiRxClk) iRmiiRxData <= d3;
    end
endtask

// GMII输出监控
integer byte_count = 0;
// always @(posedge iGmiiRxClk) begin
//     if (oGmiiRxDv) begin
//         $display("GMII Received Byte %0d: 0x%h", byte_count, oGmiiRxData);
        
//         // 验证前导码和SFD
//         if (byte_count < 7) begin
//             if (oGmiiRxData !== 8'h55) begin
//                 $error("前导码错误：期望0x55，实际0x%h", oGmiiRxData);
//                 $finish;
//             end
//         end else if (byte_count == 7) begin
//             if (oGmiiRxData !== 8'hD5) begin
//                 $error("SFD错误：期望0xD5，实际0x%h", oGmiiRxData);
//                 $finish;
//             end
//         end else begin
//             // 验证数据部分
//             case (byte_count - 8)
//                 0: if (oGmiiRxData !== 8'hAA) begin
//                         $error("数据0错误：期望0xAA，实际0x%h", oGmiiRxData);
//                         $finish;
//                     end
//                 1: if (oGmiiRxData !== 8'hBB) begin
//                         $error("数据1错误：期望0xBB，实际0x%h", oGmiiRxData);
//                         $finish;
//                     end
//                 2: if (oGmiiRxData !== 8'hCC) begin
//                         $error("数据2错误：期望0xCC，实际0x%h", oGmiiRxData);
//                         $finish;
//                     end
//                 3: if (oGmiiRxData !== 8'hDD) begin
//                         $error("数据3错误：期望0xDD，实际0x%h", oGmiiRxData);
//                         $finish;
//                     end
//                 4: if (oGmiiRxData !== 8'hEE) begin
//                         $error("数据3错误：期望0xEE，实际0x%h", oGmiiRxData);
//                         $finish;
//                     end
//                 default: begin
//                     $error("收到多余数据：0x%h", oGmiiRxData);
//                     $finish;
//                 end
//             endcase
//         end
//         byte_count <= byte_count + 1;
//     end
// end

endmodule
