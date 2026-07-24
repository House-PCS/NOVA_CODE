function [A1_COEFF, B0_COEFF, CNT, A1_real, B0_real] = LPFtustin_coeffs(WC, FS, CLK_FREQ)
%LPFTUSTIN_COEFFS 与 LPFtustin.v 中 localparam 计算方式一致（定点 ×8192）。
%
%   WC, FS, CLK_FREQ 为标量，与 Verilog 中 32'sd 语义一致（按有符号 32 位整数传入即可）。
%
%   CNT = CLK_FREQ / FS  （整数除法，与 Verilog 一致）
%   A1_COEFF = fix(A1_COEFF_REAL * 8192)
%   B0_COEFF = fix(B0_COEFF_REAL * 8192)

    WC = int32(WC);
    FS = int32(FS);
    CLK_FREQ = int32(CLK_FREQ);

    CNT = idivide(CLK_FREQ, FS); %#ok<NASGU>

    wn_fs = double(WC) / double(FS);
    A1_real = (2.0 - wn_fs) / (2.0 + wn_fs);
    B0_real = wn_fs / (2.0 + wn_fs);

    A1_COEFF = int32(fix(A1_real * 8192));
    B0_COEFF = int32(fix(B0_real * 8192));
end
