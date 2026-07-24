function y = LPFCore_sim(x, WC, FREQ, CALC_GAIN)
%LPFCORE_SIM MATLAB model converted from LPFCore.v
%   y = LPFCore_sim(x, WC, FREQ, CALC_GAIN)
%
%   Inputs:
%     x         : input sequence (vector, integer-like values recommended)
%     WC        : same as Verilog parameter WC (default 6280)
%     FREQ      : same as Verilog parameter FREQ (default 40000)
%     CALC_GAIN : same as Verilog parameter CALC_GAIN (default 1)
%
%   This model follows the core arithmetic path in LPFCore.v:
%     wOutWithCalc = rInput1 * WC - rOutput1 * (WC - FREQ)
%     wOutputWithGain = trunc_toward_zero(wOutWithCalc / FREQ)
%     y = trunc_toward_zero(wOutputWithGain / CALC_GAIN)
%
%   with state mapping:
%     rInput1  ~= CALC_GAIN * x(k-1)
%     rOutput1 ~= CALC_GAIN * y(k-1)

    if nargin < 2 || isempty(WC), WC = int64(6280); end
    if nargin < 3 || isempty(FREQ), FREQ = int64(40000); end
    if nargin < 4 || isempty(CALC_GAIN), CALC_GAIN = int64(1); end

    x = x(:);
    N = numel(x);
    y = zeros(N, 1, 'int64');

    WC = int64(WC);
    FREQ = int64(FREQ);
    CALC_GAIN = int64(CALC_GAIN);
    yk1 = WC - FREQ;

    rInput0 = int64(0);
    rInput1 = int64(0);
    rOutput = int64(0);
    rOutput1 = int64(0);

    for k = 1:N
        % Core math path from LPFCore.v
        wOutWithCalc = rInput1 * WC - rOutput1 * yk1;
        wOutputWithGain = idivide(wOutWithCalc, FREQ, 'fix');
        outk = idivide(wOutputWithGain, CALC_GAIN, 'fix');
        y(k) = outk;

        % State update corresponding to LPFCore.v non-busy branch
        rOutput = wOutputWithGain;
        rOutput1 = rOutput;
        rInput0 = int64(x(k));
        rInput1 = rInput0 * CALC_GAIN;
    end

    % Keep interface close to RTL output width usage
    y = int32(y);
end

