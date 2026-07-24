function [oOutput, dbg] = LPFtustin_sim(iInput, iAdcSampleSequenceDone, varargin)
%LPFTUSTIN_SIM  与 LPFtustin.v 逐时钟周期行为一致（含定点系数与流水线）。
%
%   对应关系（与模块一致）：
%     y[n] 的递推系数由 WC、FS、CLK_FREQ 按 Tustin 公式得到后 ×8192 定点；
%     rProduct1 = A1_COEFF * rYn1, rProduct2 = B0_COEFF * (rXn + rXn1)  （64 位有符号乘）；
%     rSum = rProduct1 + rProduct2；
%     rYn <= rSum >>> 13 （算术右移），再截断为 int32；
%     rSampling 为 6 位移位寄存器：TRI_MODE=0 时每拍移入 iAdcSampleSequenceDone。
%
%   输入：
%     iInput                 - int32 列向量，长度 N（每时钟一个样值）
%     iAdcSampleSequenceDone - 与 iInput 等长的逻辑或 0/1 向量（TRI_MODE=0 时使用）
%
%   可选 Name-Value：
%     'WC'       (默认 6280)
%     'FS'       (默认 53418)
%     'CLK_FREQ' (默认 100000000)
%     'TRI_MODE' (默认 false，与 FilterCore 中 LPFtustin 一致)
%
%   输出：
%     oOutput - int32 列向量，每时钟的 oOutput（与 oOutput = rYn 一致）
%     dbg     - 结构体，可选调试用内部状态序列
%
%   示例：
%     N = 10000;
%     x = int32(1000*sin(linspace(0,2*pi,N))');
%     adc = false(N,1); adc(1:100:end) = true;
%     y = LPFtustin_sim(x, adc, 'WC', int32(3140));

    p = inputParser;
    addParameter(p, 'WC', int32(6280), @(v) isnumeric(v) && isscalar(v));
    addParameter(p, 'FS', int32(53418), @(v) isnumeric(v) && isscalar(v));
    addParameter(p, 'CLK_FREQ', int32(100000000), @(v) isnumeric(v) && isscalar(v));
    addParameter(p, 'TRI_MODE', false, @(v) islogical(v) || isnumeric(v));
    parse(p, varargin{:});

    WC = int32(p.Results.WC);
    FS = int32(p.Results.FS);
    CLK_FREQ = int32(p.Results.CLK_FREQ);
    tri = logical(p.Results.TRI_MODE);

    iInput = iInput(:);
    iAdcSampleSequenceDone = iAdcSampleSequenceDone(:);
    assert(numel(iInput) == numel(iAdcSampleSequenceDone), ...
        'iInput 与 iAdcSampleSequenceDone 长度须相同。');

    N = numel(iInput);
    iInput = int32(iInput);

    [A1_COEFF, B0_COEFF, CNT] = LPFtustin_coeffs(WC, FS, CLK_FREQ);

    oOutput = zeros(N, 1, 'int32');

    rCnt = uint32(0);
    rSampling = uint8(0);

    rXn1 = int32(0);
    rYn1 = int32(0);
    rYn = int32(0);
    rXn = int32(0);
    rProduct1 = int64(0);
    rProduct2 = int64(0);
    rSum = int64(0);

    for k = 1:N
        adc_bit = logical(iAdcSampleSequenceDone(k));

        rCnt_old = rCnt;
        rSampling_old = rSampling;

        s0 = bitget(rSampling_old, 1) == 1;
        s2 = bitget(rSampling_old, 3) == 1;
        s3 = bitget(rSampling_old, 4) == 1;
        s4 = bitget(rSampling_old, 5) == 1;

        if s0
            rProduct1 = int64(A1_COEFF) * int64(rYn1);
            rProduct2 = int64(B0_COEFF) * int64(int64(rXn) + int64(rXn1));
        end

        if s2
            rSum = rProduct1 + rProduct2;
        end

        if s3
            rYn = int32(bitshift(int64(rSum), -13));
        end

        if s4
            rYn1 = rYn;
            rXn1 = rXn;
            rXn = iInput(k);
        end

        wSampling = (rCnt_old == 0);
        if tri
            shift_in = wSampling;
        else
            shift_in = adc_bit;
        end

        % 与 Verilog 一致：{rSampling[4:0], iAdcSampleSequenceDone} — 左移，新位进 [0]
        rSampling = uint8(bitand(int32(bitshift(int32(rSampling_old), 1)) + int32(shift_in), int32(63)));
        if rCnt_old == uint32(CNT - 1)
            rCnt = uint32(0);
        else
            rCnt = rCnt_old + uint32(1);
        end

        oOutput(k) = rYn;
    end

    if nargout > 1
        dbg = struct();
        dbg.A1_COEFF = A1_COEFF;
        dbg.B0_COEFF = B0_COEFF;
        dbg.CNT = CNT;
        dbg.WC = WC;
        dbg.FS = FS;
        dbg.CLK_FREQ = CLK_FREQ;
    end
end
