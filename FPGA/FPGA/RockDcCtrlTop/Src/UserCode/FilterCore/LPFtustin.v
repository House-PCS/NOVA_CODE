`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/09/16 17:07:31
// Design Name: 
// Module Name: LPFzoh
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

//  sys = wn/(S+wn) = 6280/(S+6280) 
//  双线性变换法 (Tustin/Bilinear Transform)
//  s = (2/T) * (z-1)/(z+1)
//  G(z) = wn / [(2/T)*(z-1)/(z+1) + wn] 
//       = (wn*T*(z+1)) / (2(z-1) + wn*T(z+1))
//  y[n] = [(2 - wn*T)/(2 + wn*T)] * y[n-1] + [(wn*T)/(2 + wn*T)] * (x[n] + x[n-1])
//  y[n] = [(2 - wn/Fs)/(2 + wn/Fs)] * y[n-1] + [(wn/Fs)/(2 + wn/Fs)] * (x[n] + x[n-1])

module LPFtustin #(
    parameter                   TRI_MODE    = 1'b0            ,
    parameter   signed [31:0]   WC          = 32'sd6280       ,
    parameter   signed [31:0]   FS          = 32'sd53418      ,//40000
    parameter   signed [31:0]   CALC_GAIN   = 32'sd1          ,
    parameter   signed [31:0]   CLK_FREQ    = 32'sd100000000  
) (
    input                       iSysClk                     ,
    input       signed [31:0]   iInput                      ,
    input                       iSamplePulse      ,
    output      signed [31:0]   oOutput                     
);

//-------------------------------Code Start Here--------------------------------
//计算采样计数
localparam      CNT             = CLK_FREQ/FS                                       ;
localparam      SHIFT           = 23                                                ;
// 计算系数
localparam real WN_DIV_FS_REAL  = $itor(WC) / $itor(FS)                             ;
localparam real A1_COEFF_REAL   = (2.0 - WN_DIV_FS_REAL) / (2.0 + WN_DIV_FS_REAL)   ;// (2-WC/FS)/(2+WC/FS) 
localparam real B0_COEFF_REAL   = WN_DIV_FS_REAL / (2.0 + WN_DIV_FS_REAL)           ;// (WC/FS)  /(2+WC/FS)

// 将系数转换为定点数
localparam signed [31:0] A1_COEFF = A1_COEFF_REAL * 32'sd8388608;//(1 <<SHIFT);
localparam signed [31:0] B0_COEFF = B0_COEFF_REAL * 32'sd8388608;//(1 <<SHIFT);

// 内部寄存器
reg signed [31:0] rXn1        = 32'sd0;   // 存储 x[n-1]
reg signed [31:0] rYn1        = 32'sd0;   // 存储 y[n-1]
reg signed [31:0] rYn         = 32'sd0; 
reg signed [31:0] rXn         = 32'sd0; 
reg signed [63:0] rProduct1   = 64'sd0;   // 中间乘积1: A1_COEFF * y_n1
reg signed [63:0] rProduct2   = 64'sd0;   // 中间乘积2: B0_COEFF * (x_n + x_n1)
reg signed [63:0] rSum        = 64'sd0;   // 总和: product1 + product2
reg signed [63:0] rSumPre     = 64'sd0;   // 总和: product1 + product2

// localparam  A1   = 8192 * (2-WC/FS)/(2+WC/FS)  ;
// localparam  B0   = 8192 * (WC/FS)  /(2+WC/FS)  ;


reg        [15:0] rCnt            =     16'd0   ;
wire              wSampling                     ;
reg        [5:0]  rSampling       =     6'd0    ;

//******************************************************************************

always @(posedge iSysClk ) begin
    if (rCnt == CNT - 1) begin
        rCnt <= 16'd0   ;
    end else begin
        rCnt <= rCnt + 1;
    end
end

assign wSampling = (rCnt == 16'd0);

always @(posedge iSysClk ) begin
    if(TRI_MODE==1'b1)
        rSampling[5:0] <= {rSampling[4:0],wSampling}; 
    else
        rSampling[5:0] <= {rSampling[4:0],iSamplePulse};       
end

//******************************************************************************


//******************************************************************************
//滤波器核心  rYn <= A1 *rYn1 + B0 * (rXn + rXn1);

always @(posedge iSysClk ) begin
    if(rSampling[0])begin
      rProduct1 <= A1_COEFF * rYn1 ;
      rProduct2 <= B0_COEFF * (rXn + rXn1);
    end
end

always @(posedge iSysClk ) begin
    if(rSampling[2])
        rSumPre <= rProduct1 + rProduct2;
    else
        rSumPre <= rSumPre ;
end

always @(posedge iSysClk ) begin
    if(rSampling[3])begin
        if(rSumPre[63]==1'b1)
            rSum <= rSumPre - 64'sd4194304;
        else
            rSum <= rSumPre + 64'sd4194304;
    end
    else
        rSum <= rSum ;
end

always @(posedge iSysClk ) begin
    if(rSampling[4])begin
       if(rSum[63] && (|rSum[22:0]))
            rYn <= (rSum>>>SHIFT)+ 64'sd1;
       else
            rYn <= rSum>>>SHIFT;
    end
    else
        rYn <= rYn ;
end

assign oOutput = rYn ;
//******************************************************************************


//******************************************************************************
// 状态更新
always @(posedge iSysClk ) begin
    if(rSampling[5])begin
        rYn1 <= rYn     ;
        rXn1 <= rXn     ;
        rXn  <= iInput  ;
    end
    else begin
        rYn1 <= rYn1    ;
        rXn1 <= rXn1    ;
        rXn  <= rXn     ;
    end
end
//******************************************************************************                                        
endmodule


