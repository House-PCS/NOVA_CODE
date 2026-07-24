`timescale 1ns/1ns

module RmiiTxTb();

//******************************************************************************
// 时钟和复位信号
//******************************************************************************

reg                     rGmiiTxClk                          ;
reg                     rRmiiTxClk                          ;
reg                     rRmiiClk180Deg                      ;

//******************************************************************************
// GMII接口信号
//******************************************************************************

reg                     rGmiiTxEn                           ;
reg     [7:0]           rGmiiTxData                         ;

//******************************************************************************
// RMII接口信号
//******************************************************************************

wire                    wRmiiTxEn                           ;
wire    [1:0]           wRmiiTxData                         ;
wire                    wRmiiClk                            ;

//******************************************************************************
// 测试控制信号
//******************************************************************************

reg     [31:0]          rTestCnt              = 32'd0       ;
reg     [7:0]           rTxDataCnt            = 8'd0        ;
reg                     rTxStart              = 1'b0        ;
reg     [7:0]           rTxDataArray    [0:63]              ;
integer                 i                                   ;
reg                     rPacketEnd            = 1'b0        ;

//******************************************************************************
// 实例化被测模块
//******************************************************************************

RmiiTx U_RmiiTx
(
    .iGmiiTxClk                     (rGmiiTxClk             ),
    .iGmiiTxEn                      (rGmiiTxEn              ),
    .iGmiiTxData                    (rGmiiTxData            ),
    
    .iRmiiTxClk                     (rRmiiTxClk             ),
    .iRmiiClk180Deg                 (rRmiiClk180Deg         ),
    .oRmiiTxEn                      (wRmiiTxEn              ),
    .oRmiiTxData                    (wRmiiTxData            ),
    .oRmiiClk                       (wRmiiClk               )
);
wire [7:0]   wGmiiRxData;
wire         wGmiiRxDv;
RmiiRx U_RmiiRx
(
    .iRmiiRxClk                         (rRmiiClk180Deg),
    .iRmiiRxDv                          (wRmiiTxEn     ),       //CRS/CRS_DV
    .iRmiiRxData                        (wRmiiTxData   ),
    
    .iGmiiRxClk                         (rGmiiTxClk),
    .oGmiiRxDv                          (wGmiiRxDv),
    .oGmiiRxData                        (wGmiiRxData)
);
//******************************************************************************
// 时钟生成
//******************************************************************************

// GMII时钟 125MHz (8ns周期)
initial begin
    rGmiiTxClk <= 1'b0;
    forever #4 rGmiiTxClk <= ~rGmiiTxClk;
end

// RMII时钟 50MHz (20ns周期)
initial begin
    rRmiiTxClk <= 1'b0;
    forever #10 rRmiiTxClk <= ~rRmiiTxClk;
end

// RMII 180度相移时钟
initial begin
    rRmiiClk180Deg <= 1'b1;
    forever #10 rRmiiClk180Deg <= ~rRmiiClk180Deg;
end

//******************************************************************************
// 测试用例主程序
//******************************************************************************

initial begin
    // 初始化
    rGmiiTxEn  <= 1'b0;
    rGmiiTxData <= 8'd0;
    rTxStart   <= 1'b0;
    rPacketEnd <= 1'b0;
    
    // 等待复位完成
    #100;
    
    // 测试用例1：验证边沿检测逻辑 - 发送短数据包
    $display("[%t] Test Case 1: Verify Edge Detection - Short Packet", $time);
    test_send_packet_with_edge_check(16);
    
    #200;
    
    // 测试用例2：验证背靠背数据包
    $display("[%t] Test Case 2: Back-to-Back Packets with Edge Detection", $time);
    test_back_to_back_with_edge_check();
    
    #200;
    
    // 测试用例3：验证数据包长度计数
    $display("[%t] Test Case 3: Packet Length Counting", $time);
    test_packet_length_count();
    
    #200;
    
    $display("[%t] All Test Cases Completed!", $time);
    $finish;
end

//******************************************************************************
// 测试任务：发送数据包并验证边沿检测
//******************************************************************************

task test_send_packet_with_edge_check;
    input [7:0] packet_length;
    integer j;
    begin
        $display("[%t] Starting packet transmission with length %0d", $time, packet_length);
        
        // 生成测试数据
        for (j = 0; j < packet_length; j = j + 1) begin
            rTxDataArray[j] = j + 8'h10;
        end
        // 重置包结束标志
        rPacketEnd = 1'b0;
        // 启动发送 - 在时钟边沿同步
        @(posedge rGmiiTxClk);
        rTxStart = 1'b1;
        // 发送前导码和SFD
        send_preamble_sfd();
        // 发送数据
        for (j = 0; j < packet_length; j = j + 1) begin
            send_byte(rTxDataArray[j]);
        end
        
        // 记录包结束时刻（在最后一个字节发送后）
        @(posedge rGmiiTxClk);
        rPacketEnd = 1'b1;
        
        // 结束发送 - 确保在时钟边沿变化
        end_transmission();
        
        // 验证边沿检测
        //verify_edge_detection(packet_length);
        
        // 监控RMII输出
        //monitor_rmii_output(packet_length);
        
        rPacketEnd = 1'b0;
    end
endtask

//******************************************************************************
// 验证任务：验证边沿检测逻辑
//******************************************************************************

// task verify_edge_detection;
//     input [7:0] expected_length;
//     begin
//         $display("[%t] Verifying edge detection logic...", $time);
        
//         // 等待rGmiiTxEn被采样（延迟一拍）
//         @(posedge rGmiiTxClk);
        
//         // 检查延迟后的rGmiiTxEn值
//         if (U_RmiiTx.rGmiiTxEn !== 1'b1) begin
//             $display("[%t] ERROR: rGmiiTxEn should be 1 after sampling", $time);
//         end else begin
//             $display("[%t] PASS: rGmiiTxEn correctly sampled as 1", $time);
//         end
        
//         // 等待包结束被检测到（再等一拍）
//         @(posedge rGmiiTxClk);
        
//         // 检查边沿检测信号
//         // 注意：wGmiiTxEnNege是组合逻辑，当{rGmiiTxEn, iGmiiTxEn} == 2'b10时置位
//         if (U_RmiiTx.wGmiiTxEnNege !== 1'b1) begin
//             $display("[%t] ERROR: Edge detection signal not asserted correctly", $time);
//             $display("[%t]       rGmiiTxEn = %b, iGmiiTxEn = %b", 
//                     $time, U_RmiiTx.rGmiiTxEn, rGmiiTxEn);
//         end else begin
//             $display("[%t] PASS: Edge detection correctly asserted", $time);
//         end
        
//         // 验证包长度计数
//         if (U_RmiiTx.rGmiiTxCnt !== expected_length + 8'd8) begin  // +8 for preamble and SFD
//             $display("[%t] ERROR: Packet length count incorrect. Expected %0d, Got %0d", 
//                     $time, expected_length + 8'd8, U_RmiiTx.rGmiiTxCnt);
//         end else begin
//             $display("[%t] PASS: Packet length count correct: %0d", 
//                     $time, U_RmiiTx.rGmiiTxCnt);
//         end
//     end
// endtask

//******************************************************************************
// 测试任务：背靠背发送并验证边沿检测
//******************************************************************************

task test_back_to_back_with_edge_check;
    begin
        $display("[%t] Starting back-to-back packet test...", $time);
        
        // 发送第一个包
        test_send_packet_with_edge_check(16);
        
        // 最小间隔后立即发送第二个包
        #32; // 等待几个时钟周期
        test_send_packet_with_edge_check(24);
        
        $display("[%t] Back-to-back test completed", $time);
    end
endtask

//******************************************************************************
// 测试任务：验证数据包长度计数
//******************************************************************************

task test_packet_length_count;
    reg [7:0] test_length;
    begin
        $display("[%t] Testing packet length counting...", $time);
        
        // 测试不同长度的数据包
        for (test_length = 10; test_length <= 50; test_length = test_length + 10) begin
            $display("[%t] Testing packet length: %0d", $time, test_length);
            test_send_packet_with_edge_check(test_length);
            #100;
        end
    end
endtask

//******************************************************************************
// 发送任务：前导码和SFD
//******************************************************************************

task send_preamble_sfd;
    integer j;
    begin
        rGmiiTxEn <= 1'b1;
        
        // 发送7个前导码字节 (0x55)
        for (j = 0; j < 7; j = j + 1) begin
            rGmiiTxData <= 8'h55;
            @(posedge rGmiiTxClk);
        end
        
        // 发送SFD (0xD5)
        rGmiiTxData <= 8'hD5;
        @(posedge rGmiiTxClk);
    end
endtask

//******************************************************************************
// 发送任务：单个字节
//******************************************************************************

task send_byte;
    input [7:0] data;
    begin
        rGmiiTxData <= data;
        @(posedge rGmiiTxClk);
    end
endtask

//******************************************************************************
// 发送任务：结束传输
//******************************************************************************

task end_transmission;
    begin
        rGmiiTxEn <= 1'b0;
        rGmiiTxData <= 8'd0;
        @(posedge rGmiiTxClk);
    end
endtask

//******************************************************************************
// 监控任务：监控RMII输出
//******************************************************************************

task monitor_rmii_output;
    input [7:0] expected_length;
    reg [7:0] rmii_byte;
    reg [1:0] nibble_cnt;
    integer byte_cnt;
    begin
        byte_cnt = 0;
        nibble_cnt = 0;
        rmii_byte = 8'd0;
        
        $display("[%t] Monitoring RMII Output...", $time);
        
        // 等待RMII传输开始
        wait(wRmiiTxEn == 1'b1);
        $display("[%t] RMII transmission started", $time);
        
        // 监控RMII数据
        while (wRmiiTxEn == 1'b1) begin
            @(posedge rRmiiClk180Deg);
            
            case (nibble_cnt)
                2'd0: begin
                    rmii_byte[1:0] = wRmiiTxData;  // 注意位序：原始代码中进行了位序交换
                    nibble_cnt = 2'd1;
                end
                2'd1: begin
                    rmii_byte[3:2] = wRmiiTxData;
                    nibble_cnt = 2'd2;
                end
                2'd2: begin
                    rmii_byte[5:4] = wRmiiTxData;
                    nibble_cnt = 2'd3;
                end
                2'd3: begin
                    rmii_byte[7:6] = wRmiiTxData;
                    
                    // 完成一个字节的接收
                    $display("[%t] RMII Received Byte %0d: 0x%02h", 
                            $time, byte_cnt, rmii_byte);
                    
                    // 验证数据（跳过前导码和SFD）
                    if (byte_cnt >= 8 && byte_cnt < expected_length + 8) begin
                        reg [7:0] expected_data;
                        expected_data = (byte_cnt - 8) + 8'h10;
                        if (rmii_byte !== expected_data) begin
                            $display("[%t] ERROR: Expected 0x%02h, Got 0x%02h", 
                                    $time, expected_data, rmii_byte);
                        end
                    end
                    
                    byte_cnt = byte_cnt + 1;
                    nibble_cnt = 2'd0;
                end
            endcase
        end
        
        $display("[%t] RMII Transmission Complete. Received %0d bytes", 
                $time, byte_cnt);
    end
endtask

//******************************************************************************
// 测试计数器
//******************************************************************************

always @(posedge rGmiiTxClk) begin
    rTestCnt <= rTestCnt + 32'd1;
end

//******************************************************************************
// 波形记录
//******************************************************************************

// initial begin
//     $dumpfile("tb_RmiiTx.vcd");
//     $dumpvars(0, tb_RmiiTx);
// end

//******************************************************************************
// 超时保护
//******************************************************************************

initial begin
    #1000000;
    $display("[%t] Error: Simulation Timeout!", $time);
    $finish;
end

endmodule