

%%-------------------------------------------------------------------------------------------------------------
    function [total_loss, breakdown] = calculate_mosfet_loss(varargin)
% 计算MOSFET功率损耗
% 用法1: calculate_mosfet_loss('param1', value1, 'param2', value2, ...)
% 用法2: [total_loss, breakdown] = calculate_mosfet_loss(config_struct)

    %% 参数解析
    if nargin == 1 && isstruct(varargin{1})
        config = varargin{1};
    else
        config = parse_parameters(varargin{:});
    end
    
    %% 从配置中提取参数
    Vin = config.Vin;           % 输入电压 (V)
    Vout = config.Vout;         % 输出电压 (V)
    I_load = config.I_load;     % 负载电流 (A)
    fsw = config.fsw;           % 开关频率 (Hz)
    Vgs = config.Vgs;           % 栅极驱动电压 (V)
    Tj_est = config.Tj_est;     % 估计结温 (°C)
    Ta = config.Ta;             % 环境温度 (°C)
    
    % MOSFET参数
    Rds_on = config.Rds_on;     % 导通电阻 (Ω)
    Qg = config.Qg;             % 总栅极电荷 (nC)
    Qgs = config.Qgs;           % 栅源电荷 (nC)
    Qgd = config.Qgd;           % 栅漏电荷 (nC)
    tr = config.tr;             % 上升时间 (s)
    tf = config.tf;             % 下降时间 (s)
    Coss = config.Coss;         % 输出电容 (F)
    Rth_ja = config.Rth_ja;     % 结到环境热阻 (°C/W)
    
    %% 计算基本工作参数
    % D = Vout / Vin;             % 占空比 (Buck电路)
    % Irms = I_load * sqrt(D);    % RMS电流 (简化计算)

    % 占空比计算
D = 1 - (Vin / Vout);
% RMS电流计算
Irms = I_load / (1 - D) * sqrt(1 - D); % 输入电流RMS
% 开关损耗电压
V_sw = Vout; % MOSFET关断时承受输出电压
    
    %% 1. 导通损耗计算
    % 考虑温度对Rds_on的影响 (典型: Rds_on随温度升高而增大)
    Rds_on_actual = Rds_on * (1 + 0.007 * (Tj_est - 25)); % 0.7%/°C的温度系数
    P_cond = Irms^2 * Rds_on_actual * D;
    
    %% 2. 开关损耗计算
    % 方法1: 使用开关时间计算
    E_switch_single = 0.5 * Vin * I_load * (tr + tf); % 单次开关能量 (J)
    P_sw_time = E_switch_single * fsw;
    
    % 方法2: 使用栅极电荷计算 (更精确)
    V_plateau = 4; % 典型米勒平台电压 (V)，可根据数据手册调整
    E_on = Qgd * 1e-9 * V_plateau;  % 开通能量 (J)
    E_off = Qgd * 1e-9 * (Vin - V_plateau); % 关断能量 (J)
    P_sw_charge = (E_on + E_off) * fsw;
    
    % 使用更精确的方法
    P_sw = P_sw_charge;
    
    %% 3. 输出电容损耗
    P_coss = 0.5 * Coss * Vin^2 * fsw;
    
    %% 4. 驱动损耗
    P_drive = Qg * 1e-9 * Vgs * fsw;
    
    %% 5. 总损耗计算
    P_total = P_cond + P_sw + P_coss + P_drive;
    
    %% 6. 热计算 - 迭代以获得更准确的结果
    Tj_new = Ta + P_total * Rth_ja;
    
    % 如果估计结温与实际计算相差较大，重新计算
    if abs(Tj_new - Tj_est) > 5
        fprintf('进行热迭代: Tj_est = %.1f°C, Tj_new = %.1f°C\n', Tj_est, Tj_new);
        % 更新Rds_on基于新的结温
        Rds_on_new = Rds_on * (1 + 0.007 * (Tj_new - 25));
        P_cond_new = Irms^2 * Rds_on_new * D;
        P_total_new = P_cond_new + P_sw + P_coss + P_drive;
        
        % 使用迭代后的值
        P_cond = P_cond_new;
        P_total = P_total_new;
        Tj_final = Ta + P_total_new * Rth_ja;
    else
        Tj_final = Tj_new;
    end
    
    %% 输出结果
    fprintf('\n=== MOSFET损耗分析结果 ===\n');
    fprintf('工作条件:\n');
    fprintf('  输入电压: %.1f V\n', Vin);
    fprintf('  输出电压: %.1f V\n', Vout);
    fprintf('  负载电流: %.1f A\n', I_load);
    fprintf('  开关频率: %.0f kHz\n', fsw/1e3);
    fprintf('  占空比: %.2f\n', D);
    
    fprintf('\n损耗分析:\n');
    fprintf('  导通损耗: %.3f W (%.1f%%)\n', P_cond, P_cond/P_total*100);
    fprintf('  开关损耗: %.3f W (%.1f%%)\n', P_sw, P_sw/P_total*100);
    fprintf('  Coss损耗: %.3f W (%.1f%%)\n', P_coss, P_coss/P_total*100);
    fprintf('  驱动损耗: %.3f W (%.1f%%)\n', P_drive, P_drive/P_total*100);
    fprintf('  -------------------------\n');
    fprintf('  总损耗:   %.3f W\n', P_total);
    
    fprintf('\n热分析:\n');
    fprintf('  环境温度: %.1f°C\n', Ta);
    fprintf('  结温:     %.1f°C\n', Tj_final);
    fprintf('  热裕量:   %.1f°C (假设最大结温150°C)\n', 150 - Tj_final);
    
    %% 返回结果
    total_loss = P_total;
    breakdown.cond_loss = P_cond;
    breakdown.sw_loss = P_sw;
    breakdown.coss_loss = P_coss;
    breakdown.drive_loss = P_drive;
    breakdown.junction_temp = Tj_final;
    
    %% 绘制损耗分布图
    if config.plot_results
        plot_loss_breakdown(breakdown);
    end
end

function config = parse_parameters(varargin)
% 解析输入参数
    p = inputParser;
    
    % 默认参数 - 基于典型的MOSFET应用
    addParameter(p, 'Vin', 24, @isnumeric);
    addParameter(p, 'Vout', 12, @isnumeric);
    addParameter(p, 'I_load', 5, @isnumeric);
    addParameter(p, 'fsw', 100e3, @isnumeric);
    addParameter(p, 'Vgs', 10, @isnumeric);
    addParameter(p, 'Tj_est', 80, @isnumeric);
    addParameter(p, 'Ta', 40, @isnumeric);
    
    % MOSFET参数默认值 (基于典型器件)
    addParameter(p, 'Rds_on', 0.01, @isnumeric);      % 10mΩ
    addParameter(p, 'Qg', 25, @isnumeric);            % 25nC
    addParameter(p, 'Qgs', 8, @isnumeric);            % 8nC
    addParameter(p, 'Qgd', 12, @isnumeric);           % 12nC
    addParameter(p, 'tr', 20e-9, @isnumeric);         % 20ns
    addParameter(p, 'tf', 15e-9, @isnumeric);         % 15ns
    addParameter(p, 'Coss', 300e-12, @isnumeric);     % 300pF
    addParameter(p, 'Rth_ja', 40, @isnumeric);        % 40°C/W
    
    addParameter(p, 'plot_results', true, @islogical);
    
    parse(p, varargin{:});
    config = p.Results;
end

function plot_loss_breakdown(breakdown)
% 绘制损耗分布饼图
    losses = [breakdown.cond_loss, breakdown.sw_loss, ...
              breakdown.coss_loss, breakdown.drive_loss];
    labels = {'导通损耗', '开关损耗', 'Coss损耗', '驱动损耗'};
    
    figure;
    pie(losses, labels);
    title('MOSFET损耗分布');
    
    % 添加数值标签
    legend_text = cell(1, length(labels));
    for i = 1:length(labels)
        legend_text{i} = sprintf('%s: %.3fW (%.1f%%)', ...
            labels{i}, losses(i), losses(i)/sum(losses)*100);
    end
    legend(legend_text, 'Location', 'eastoutside');
end

      