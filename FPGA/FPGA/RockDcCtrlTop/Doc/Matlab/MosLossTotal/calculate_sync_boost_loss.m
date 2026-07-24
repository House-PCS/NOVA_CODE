
%%

function [total_loss, loss_breakdown] = calculate_sync_boost_loss(varargin)
% 计算同步Boost变换器上下MOSFET的功率损耗
% 用法: [total_loss, loss_breakdown] = calculate_sync_boost_loss('param1', value1, ...)

    %% 参数解析
    if nargin == 1 && isstruct(varargin{1})
        config = varargin{1};
    else
        config = parse_sync_boost_parameters(varargin{:});
    end
    
    %% 从配置中提取参数
    Vin         = config.Vin        ;   % 输入电压 (V)
    Vout        = config.Vout       ;   % 输出电压 (V)
    I_out       = config.I_out      ;   % 输出电流 (A)
    fsw         = config.fsw        ;   % 开关频率 (Hz)
    L           = config.L          ;   % 电感值 (H)
    RL          = config.RL         ;   % 电感等效串联电阻 (Ω)
    
    % 控制MOSFET参数 (下管 - 主动开关)
    Rds_on_ctrl = config.Rds_on_ctrl;   % 控制管导通电阻 (Ω)
    Qg_ctrl     = config.Qg_ctrl    ;   % 控制管栅极电荷 (nC)
    Qgd_ctrl    = config.Qgd_ctrl   ;   % 控制管栅漏电荷 (nC)
    tr_ctrl     = config.tr_ctrl    ;   % 控制管上升时间 (s)
    tf_ctrl     = config.tf_ctrl    ;   % 控制管下降时间 (s)
    Coss_ctrl   = config.Coss_ctrl  ;   % 控制管输出电容 (F)
    
    % 同步MOSFET参数 (上管 - 同步整流)
    Rds_on_sync = config.Rds_on_sync;   % 同步管导通电阻 (Ω)
    Qg_sync     = config.Qg_sync    ;   % 同步管栅极电荷 (nC)
    Qgd_sync    = config.Qgd_sync   ;   % 同步管栅漏电荷 (nC)
    tr_sync     = config.tr_sync    ;   % 同步管上升时间 (s)
    tf_sync     = config.tf_sync    ;   % 同步管下降时间 (s)
    Coss_sync   = config.Coss_sync  ;   % 同步管输出电容 (F)
    
    % 通用参数
    Vgs         = config.Vgs        ;   % 栅极驱动电压 (V)
    Tj_est      = config.Tj_est     ;   % 估计结温 (°C)
    Ta          = config.Ta         ;   % 环境温度 (°C)
    Rth_ja      = config.Rth_ja     ;   % 结到环境热阻 (°C/W)
    
    %% 计算同步Boost基本工作参数
    D = 1 - (Vin / Vout);       % 占空比 (控制管导通占空比)
    I_in = I_out / (1 - D);     % 输入平均电流 (A)
    
    % 电感电流纹波
    delta_IL = (Vin * D) / (fsw * L);  % 电感电流纹波峰值 (A)
    IL_avg = I_in;                     % 电感平均电流 = 输入平均电流 (A)
    IL_rms = sqrt(IL_avg^2 + (delta_IL^2 / 12)); % 电感电流RMS值
    
    fprintf('\n=== 同步Boost工作参数 ===\n');
    fprintf('占空比 (控制管): %.3f\n', D);
    fprintf('输入平均电流: %.3f A\n', I_in);
    fprintf('电感平均电流: %.3f A\n', IL_avg);
    fprintf('电感电流纹波: ±%.3f A\n', delta_IL/2);
    fprintf('电感电流RMS: %.3f A\n', IL_rms);
    
    %% 控制MOSFET (下管) 损耗计算
    fprintf('\n=== 控制MOSFET (下管) 损耗计算 ===\n');
    
    % 1. 导通损耗--------------------------------------------------------------
    % 控制管只在(D)时间内导通，电流为电感电流
    Rds_on_ctrl_actual = Rds_on_ctrl * (1 + 0.007 * (Tj_est - 25));
    P_cond_ctrl = IL_rms^2 * Rds_on_ctrl_actual * D;
    fprintf('控制管导通损耗: %.3f W\n', P_cond_ctrl);
    
    % 2. 开关损耗---------------------------------------------------------------
    % 控制管开关时承受的电压为输出电压Vout
    V_sw_ctrl = Vout;
    
    % 使用栅极电荷法计算开关损耗
    V_plateau = 4; % 典型米勒平台电压 (V)
  E_on_ctrl = Qgd_ctrl * 1e-9 * V_plateau;        % 开通能量
  E_off_ctrl = Qgd_ctrl * 1e-9 * (V_sw_ctrl - V_plateau); % 关断能量
  P_sw_ctrl = (E_on_ctrl + E_off_ctrl) * fsw;
   % E_on_ctrl = 1/2 * V_sw_ctrl * IL_rms * tr_ctrl ;        % 开通能量
   % E_off_ctrl = 1/2 * V_sw_ctrl * IL_rms * tf_ctrl;  % 关断能量
   % P_sw_ctrl = (E_on_ctrl + E_off_ctrl) * fsw;
    fprintf('控制管开关损耗: %.3f W\n', P_sw_ctrl);
    
    % 3. 输出电容损耗
    P_coss_ctrl = 0.5 * Coss_ctrl * V_sw_ctrl^2 * fsw;
    fprintf('控制管Coss损耗: %.3f W\n', P_coss_ctrl);
    
    % 4. 驱动损耗
    P_drive_ctrl = Qg_ctrl * 1e-9 * Vgs * fsw;
    fprintf('控制管驱动损耗: %.3f W\n', P_drive_ctrl);
    
    P_total_ctrl = P_cond_ctrl + P_sw_ctrl + P_coss_ctrl + P_drive_ctrl;
    fprintf('控制管总损耗: %.3f W\n', P_total_ctrl);
    
    %% 同步MOSFET (上管) 损耗计算
    fprintf('\n=== 同步MOSFET (上管) 损耗计算 ===\n');
    
    % 1. 导通损耗
    % 同步管在(1-D)时间内导通，电流为电感电流
    Rds_on_sync_actual = Rds_on_sync * (1 + 0.007 * (Tj_est - 25));
    P_cond_sync = IL_rms^2 * Rds_on_sync_actual * (1 - D);
    fprintf('同步管导通损耗: %.3f W\n', P_cond_sync);
    
    % 2. 开关损耗
    % 同步管开关损耗通常较小，因为体二极管先导通
    % 但反向恢复会产生额外损耗
    
    % a) 体二极管反向恢复损耗
    Qrr = config.Qrr;           % 反向恢复电荷 (nC)
    if Qrr > 0
        P_rr_sync = 0.5 * Qrr * 1e-9 * Vout * fsw;
    else
        % 估算反向恢复损耗
        P_rr_sync = 0.25 * (Qgd_sync * 1e-9 * Vout * fsw); % 约为控制管的25%
    end
    
    % b) 常规开关损耗 (较小)
    V_sw_sync = Vout; % 同步管承受输出电压
    E_sw_sync = 0.25 * (Qgd_sync * 1e-9 * V_sw_sync); % 同步管开关能量较小
    P_sw_sync = E_sw_sync * fsw + P_rr_sync;
    fprintf('同步管开关损耗: %.3f W (含反向恢复)\n', P_sw_sync);
    
    % 3. 输出电容损耗
    P_coss_sync = 0.5 * Coss_sync * V_sw_sync^2 * fsw;
    fprintf('同步管Coss损耗: %.3f W\n', P_coss_sync);
    
    % 4. 驱动损耗
    P_drive_sync = Qg_sync * 1e-9 * Vgs * fsw;
    fprintf('同步管驱动损耗: %.3f W\n', P_drive_sync);
    
    P_total_sync = P_cond_sync + P_sw_sync + P_coss_sync + P_drive_sync;
    fprintf('同步管总损耗: %.3f W\n', P_total_sync);
    
    %% 电感损耗
    P_inductor = IL_rms^2 * RL;
    fprintf('\n电感铜损: %.3f W\n', P_inductor);
    
    %% 总损耗和效率计算
    P_total = P_total_ctrl + P_total_sync + P_inductor;
    P_out = Vout * I_out;
    P_in = P_out + P_total;
    efficiency = P_out / P_in * 100;
    
    fprintf('\n=== 总损耗和效率 ===\n');
    fprintf('输出功率: %.3f W\n', P_out);
    fprintf('总损耗: %.3f W\n', P_total);
    fprintf('输入功率: %.3f W\n', P_in);
    fprintf('计算效率: %.2f%%\n', efficiency);
    
    %% 热计算
    Tj_ctrl = Ta + P_total_ctrl * Rth_ja;
    Tj_sync = Ta + P_total_sync * Rth_ja;
    
    fprintf('\n=== 热分析 ===\n');
    fprintf('环境温度: %.1f°C\n', Ta);
    fprintf('控制管结温: %.1f°C\n', Tj_ctrl);
    fprintf('同步管结温: %.1f°C\n', Tj_sync);
    
    %% 返回结果
    total_loss = P_total;
    
    loss_breakdown.control = struct(...
        'cond_loss', P_cond_ctrl, ...
        'sw_loss', P_sw_ctrl, ...
        'coss_loss', P_coss_ctrl, ...
        'drive_loss', P_drive_ctrl, ...
        'total_loss', P_total_ctrl, ...
        'junction_temp', Tj_ctrl);
    
    loss_breakdown.sync = struct(...
        'cond_loss', P_cond_sync, ...
        'sw_loss', P_sw_sync, ...
        'coss_loss', P_coss_sync, ...
        'drive_loss', P_drive_sync, ...
        'total_loss', P_total_sync, ...
        'junction_temp', Tj_sync);
    
    loss_breakdown.inductor = P_inductor;
    loss_breakdown.efficiency = efficiency;
    loss_breakdown.operating_params = struct(...
        'duty_cycle', D, ...
        'I_in_avg', I_in, ...
        'IL_rms', IL_rms, ...
        'delta_IL', delta_IL);
    
    %% 绘制结果
    if config.plot_results
        plot_sync_boost_results(loss_breakdown, config);
    end
end

function config = parse_sync_boost_parameters(varargin)
% 解析同步Boost参数
    p = inputParser;
    
    % 基本工作参数
    addParameter(p, 'Vin', 12, @isnumeric);
    addParameter(p, 'Vout', 24, @isnumeric);
    addParameter(p, 'I_out', 5, @isnumeric);
    addParameter(p, 'fsw', 100e3, @isnumeric);
    addParameter(p, 'L', 10e-6, @isnumeric);
    addParameter(p, 'RL', 0.005, @isnumeric); % 电感DCR
    
    % 控制MOSFET参数 (下管)
    addParameter(p, 'Rds_on_ctrl', 0.005, @isnumeric);   % 5mΩ
    addParameter(p, 'Qg_ctrl', 25, @isnumeric);          % 25nC
    addParameter(p, 'Qgd_ctrl', 12, @isnumeric);         % 12nC
    addParameter(p, 'tr_ctrl', 20e-9, @isnumeric);       % 20ns
    addParameter(p, 'tf_ctrl', 15e-9, @isnumeric);       % 15ns
    addParameter(p, 'Coss_ctrl', 300e-12, @isnumeric);   % 300pF
    
    % 同步MOSFET参数 (上管)
    addParameter(p, 'Rds_on_sync', 0.003, @isnumeric);   % 3mΩ (通常更小)
    addParameter(p, 'Qg_sync', 20, @isnumeric);          % 20nC
    addParameter(p, 'Qgd_sync', 10, @isnumeric);         % 10nC
    addParameter(p, 'tr_sync', 15e-9, @isnumeric);       % 15ns
    addParameter(p, 'tf_sync', 12e-9, @isnumeric);       % 12ns
    addParameter(p, 'Coss_sync', 400e-12, @isnumeric);   % 400pF
    addParameter(p, 'Qrr', 15, @isnumeric);              % 反向恢复电荷
    
    % 通用参数
    addParameter(p, 'Vgs', 10, @isnumeric);
    addParameter(p, 'Tj_est', 80, @isnumeric);
    addParameter(p, 'Ta', 40, @isnumeric);
    addParameter(p, 'Rth_ja', 40, @isnumeric);
    addParameter(p, 'plot_results', true, @islogical);
    
    parse(p, varargin{:});
    config = p.Results;
end

function plot_sync_boost_results(loss_breakdown, config)
% 绘制同步Boost损耗分析结果
    figure('Position', [100, 100, 1200, 800]);
    
    % 子图1: 总损耗分布
    subplot(2,3,1);
    losses_total = [loss_breakdown.control.total_loss, ...
                   loss_breakdown.sync.total_loss, ...
                   loss_breakdown.inductor];
    labels_total = {'控制管', '同步管', '电感'};
    pie(losses_total, labels_total);
    title('总损耗分布');
    
    % 子图2: 控制管损耗细分
    subplot(2,3,2);
    losses_ctrl = [loss_breakdown.control.cond_loss, ...
                  loss_breakdown.control.sw_loss, ...
                  loss_breakdown.control.coss_loss, ...
                  loss_breakdown.control.drive_loss];
    labels_ctrl = {'导通', '开关', 'Coss', '驱动'};
    pie(losses_ctrl, labels_ctrl);
    title('控制管损耗细分');
    
    % 子图3: 同步管损耗细分
    subplot(2,3,3);
    losses_sync = [loss_breakdown.sync.cond_loss, ...
                  loss_breakdown.sync.sw_loss, ...
                  loss_breakdown.sync.coss_loss, ...
                  loss_breakdown.sync.drive_loss];
    labels_sync = {'导通', '开关', 'Coss', '驱动'};
    pie(losses_sync, labels_sync);
    title('同步管损耗细分');
    
    % 子图4: 损耗对比柱状图
    subplot(2,3,4);
    categories = {'导通损耗', '开关损耗', 'Coss损耗', '驱动损耗'};
    ctrl_data = [loss_breakdown.control.cond_loss, loss_breakdown.control.sw_loss, ...
                loss_breakdown.control.coss_loss, loss_breakdown.control.drive_loss];
    sync_data = [loss_breakdown.sync.cond_loss, loss_breakdown.sync.sw_loss, ...
                loss_breakdown.sync.coss_loss, loss_breakdown.sync.drive_loss];
    
    bar_data = [ctrl_data; sync_data]';
    bar(bar_data);
    set(gca, 'XTickLabel', categories);
    legend('控制管', '同步管', 'Location', 'northwest');
    ylabel('损耗 (W)');
    title('损耗分量对比');
    grid on;
    
    % 子图5: 热分析
    subplot(2,3,5);
    temps = [loss_breakdown.control.junction_temp, loss_breakdown.sync.junction_temp, config.Ta];
    bar(temps);
    ylabel('温度 (°C)');
    set(gca, 'XTickLabel', {'控制管结温', '同步管结温', '环境温度'});
    title('热分析');
    grid on;
    
    % 添加温度线
    hold on;
    plot(xlim, [150, 150], 'r--', 'LineWidth', 2, 'DisplayName', '最大结温150°C');
    legend;
    
    % 子图6: 工作参数显示
    subplot(2,3,6);
    text(0.1, 0.9, sprintf('效率: %.2f%%', loss_breakdown.efficiency), 'FontSize', 12);
    text(0.1, 0.7, sprintf('占空比: %.3f', loss_breakdown.operating_params.duty_cycle), 'FontSize', 12);
    text(0.1, 0.5, sprintf('输入电流: %.2fA', loss_breakdown.operating_params.I_in_avg), 'FontSize', 12);
    text(0.1, 0.3, sprintf('电感电流RMS: %.2fA', loss_breakdown.operating_params.IL_rms), 'FontSize', 12);
    text(0.1, 0.1, sprintf('电流纹波: ±%.2fA', loss_breakdown.operating_params.delta_IL/2), 'FontSize', 12);
    axis off;
    title('工作参数');
end




