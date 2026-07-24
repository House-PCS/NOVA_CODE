`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/03/06 11:22:05
// Design Name: 
// Module Name: Adcnormalization
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





module Adcnormalization(
   input        iSysClk,      // 系统时钟
   input        iReset,       // 复位信号
   input [16:0] A,            // 被除数
   input [7:0]  B,            // 除数
   output reg [16:0] C,       // 商
   output reg   oDone          // 运算完成标志
);

reg [16:0] remainder;         // 余数寄存器
reg [16:0] quotient;          // 商寄存器
reg [7:0]  divisor;           // 除数寄存器
reg [16:0]  step;              // 迭代计数器

always @(posedge iSysClk or posedge iReset) begin
    if (iReset) begin
        // 复位初始化
        remainder <= 17'd0;
        quotient  <= 17'd0;
        divisor   <= 8'd0;
        step      <= 5'd0;
        C         <= 17'd0;
        oDone     <= 1'b0;
    end else begin
        if (step == 5'd0) begin
            // 初始化
            remainder <= A;           // 被除数存入余数寄存器
            quotient  <= 17'd0;       // 商清零
            divisor   <= B;           // 除数存入寄存器
            step      <= 5'd1;        // 进入第一步
            oDone     <= 1'b0;       // 运算未完成
        end else if (step <= 17'd1000) begin
            // 试减法迭代
            if (remainder >= divisor) begin
                remainder <= remainder - divisor; // 减去除数
                quotient  <= quotient + 1;        // 商加 1
            end else begin
                // 当前位无法再减，进入下一位
                remainder <= remainder;           // 余数保持不变
                quotient  <= quotient;            // 商保持不变
            end
            //divisor  <= divisor << 1; // 除数左移（相当于乘以 2）
            step     <= step + 1;      // 进入下一步
        end else begin
            // 计算完成
            C     <= quotient;         // 输出商
            oDone <= 1'b1;             // 设置完成标志
            step  <= 5'd0;             // 复位计数器
        end
    end
end

endmodule

// module Adcnormalization(
//      input                   iSysClk                   ,
//     // input       [11 : 0]    iPortA                    ,
//     // input       [15 : 0]    iPortB                    ,
//     // input       [14 : 0]    iPortC                    ,
//    input [7:0]      iRVal1 ,
//    input [7:0]      iGVal1 ,
//    input [7:0]      iBVal1 ,
//    input [16:0]     wAllPeriod,
//    output [31:0] oSignalPeriod,

//     // output      [27 : 0]    oPortP        

//     output [16:0] ocnt2
//     );

//     parameter                   T0H_COUNT   = 20    ;           //bit0低电平时间计数  0.4us
//     parameter                   T0L_COUNT   = 50    ;           //bit0高电平时间计数  1us
//     parameter                   T1H_COUNT   = 50    ;           //bit1低电平时间计数  1us   
//     parameter                   T1L_COUNT   = 20    ;           //bit1高电平时间计数  0.4us  
//     parameter                   RESET_COUNT = 8240  ;       //帧间隔计数   

// reg     [7:0]   rColorMaxCnt;
// reg     [31:0]  rSignalPeriod;
// wire    [31:0]  wSignalPeriodmMin;
// reg     [31:0]  rrSignalPeriod;
// assign wSignalPeriodmMin = (T0H_COUNT + T0L_COUNT) * 24 * 7 + 4000; 


// reg [7:0] rRVal1[1:0] ;
// reg [7:0] rGVal1[1:0] ;
// reg [7:0] rBVal1[1:0] ;

// wire        wFlag   ;
// reg  [7:0]  rFlag   ;
// reg         rFlag1  ;
// reg [7:0]   rFlag2  ;
// always @(posedge iSysClk ) begin
//     rRVal1[0][7:0] <= iRVal1[7:0];
//     rGVal1[0][7:0] <= iGVal1[7:0];
//     rBVal1[0][7:0] <= iBVal1[7:0];
// end
// always @(posedge iSysClk ) begin
//     rRVal1[1][7:0] <= rRVal1[0][7:0];
//     rGVal1[1][7:0] <= rGVal1[0][7:0];
//     rBVal1[1][7:0] <= rBVal1[0][7:0];
// end

// assign wFlag = (rRVal1[0]!=rRVal1[1]) || (rGVal1[0]!=rGVal1[1]) || (rBVal1[0]!=rBVal1[1]) ;

// always @(posedge iSysClk ) begin
//     rFlag[7:0] <= {rFlag[6:0],wFlag};
// end
// always @(posedge iSysClk ) begin
//     rFlag1 <= (rFlag[7:6] ==  2'b10);
// end

// always @(posedge iSysClk ) begin
//     rFlag2[7:0] <= {rFlag2[6:0],rFlag1};
// end

// always @(posedge iSysClk ) begin
//     if(rFlag2[0])
//     rColorMaxCnt <= (iRVal1 > iGVal1 ? iRVal1 : iGVal1) > iBVal1 ? (iRVal1 > iGVal1 ? iRVal1 : iGVal1) : iBVal1;
// end

// always @(posedge iSysClk ) begin
//     if(rFlag2[1])begin
//         if(rColorMaxCnt != 0)
//             rSignalPeriod <= (wAllPeriod - rColorMaxCnt);
//         else
//             rSignalPeriod <= wAllPeriod;
//     end
// end

// always @(posedge iSysClk ) begin
//     if(rFlag2[2])begin
//         if(rSignalPeriod < wSignalPeriodmMin)
//             rrSignalPeriod <= wSignalPeriodmMin;
//         else
//      rrSignalPeriod <= rSignalPeriod;
//     end

// end


// always @(posedge iSysClk ) 
// begin
//     rSignalPeriod <= ((rColorMaxCnt != 0) ? (wAllPeriod / rColorMaxCnt) : wAllPeriod) < wSignalPeriodmMin ? wSignalPeriodmMin : ((rColorMaxCnt != 0) ? (wAllPeriod / rColorMaxCnt) : wAllPeriod);
// end

//assign oSignalPeriod[31:0] = rrSignalPeriod[31:0];


    //帧间隔计数值

// Multadd_U12_S16_S15 U_Multadd_U12_S16_S15 (
//   .CLK      (iSysClk            ),          // input wire CLK
//   .CE       (1'b1               ),          // input wire CE
//   .SCLR     (1'b0               ),          // input wire SCLR
//   .A        (iPortA [11 : 0]    ),          // input wire [11 : 0] A
//   .B        (iPortB [15 : 0]    ),          // input wire [15 : 0] B
//   .C        (iPortC [14 : 0]    ),          // input wire [14 : 0] C
//   .SUBTRACT (1'b1               ),          // input wire SUBTRACT
//   .P        (oPortP[27 : 0]     ),          // output wire [27 : 0] P
//   .PCOUT    (                   )           // output wire [47 : 0] PCOUT
// );

// reg signed [27:0]rResu;
// reg signed [27:0]rrResu;
// reg signed [27:0]rrrResu;

// reg  [15:0]rrrrResu;

// always @(posedge iSysClk ) begin
//     rResu[27:0] <=  iPortA[11:0] * 16'd22220;
// end 
// always @(posedge iSysClk ) begin
//     rrResu[27:0] <=  rResu[27:0] /16'd4096 - 15'd11110;
// end 

// always @(posedge iSysClk ) begin
//     rrrrResu[15:0] <=  rResu[27:12] ;
// end 

// always @(posedge iSysClk ) begin
//     rrrResu[27:0] <=  rrrrResu[15:0] - 15'd11110;
// end

// reg         [9:0] cnt = 10'd0;
// reg signed  [9:0] cnts ;
// reg signed [15:0] cnt2 =16'd0;

// always @(posedge iSysClk ) begin
//     cnt[9:0] <=  cnt[9:0] + 1'd1;
// end 
// always @(posedge iSysClk ) begin
//     cnts[9:0] <=  cnt[9:0];
// end 

// always @(posedge iSysClk ) begin
//     cnt2 <= cnts;
// end 

// assign ocnt2 = cnt2;
//endmodule


// module Adcnormalization(
//    input        iSysClk,
//    input [16:0]     A,
//    input [7:0]      B,
//    output reg[16:0]    C

//     );

//  always @(posedge iSysClk ) begin
//      C <=  A / B;
//  end 

//  endmodule