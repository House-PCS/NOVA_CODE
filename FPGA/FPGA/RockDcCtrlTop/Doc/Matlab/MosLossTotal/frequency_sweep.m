%% 辅助函数：扫描开关频率对损耗的影响
function frequency_sweep()
% 分析开关频率对损耗的影响
    fprintf('\n=== 开关频率扫描分析 ===\n');
    
    % 基础配置
    base_config.Vin = 24;
    base_config.Vout = 12;
    base_config.I_load = 5;
    base_config.Vgs = 10;
    base_config.Ta = 40;
    
    % MOSFET参数 (Infineon IPP320N20N3)
    base_config.Rds_on = 0.0032;    % 3.2mΩ
    base_config.Qg = 110;           % 110nC
    base_config.Qgs = 35;           % 35nC
    base_config.Qgd = 52;           % 52nC
    base_config.tr = 25e-9;         % 25ns
    base_config.tf = 20e-9;         % 20ns
    base_config.Coss = 680e-12;     % 680pF
    base_config.Rth_ja = 40;        % 40°C/W
    base_config.plot_results = false;
    
    % 频率扫描范围
    fsw_range = [50e3, 100e3, 200e3, 300e3, 500e3, 1e6];
    
    % 存储结果
    results = zeros(length(fsw_range), 5);
    
    fprintf('频率(kHz)\t导通损耗\t开关损耗\t总损耗\t结温(°C)\n');
    fprintf('---------------------------------------------------\n');
    
    for i = 1:length(fsw_range)
        config = base_config;
        config.fsw = fsw_range(i);
        config.Tj_est = 80;
        
        [total_loss, breakdown] = calculate_mosfet_loss(config);
        
        results(i, :) = [fsw_range(i)/1e3, breakdown.cond_loss, ...
                        breakdown.sw_loss, total_loss, breakdown.junction_temp];
        
        fprintf('%.0f\t\t%.3f\t\t%.3f\t\t%.3f\t%.1f\n', results(i, :));
    end
    
    % 绘制频率扫描图
    figure;
    subplot(2,1,1);
    plot(results(:,1), results(:,2), 'b-o', 'LineWidth', 2, 'DisplayName', '导通损耗');
    hold on;
    plot(results(:,1), results(:,3), 'r-s', 'LineWidth', 2, 'DisplayName', '开关损耗');
    plot(results(:,1), results(:,4), 'k-^', 'LineWidth', 2, 'DisplayName', '总损耗');
    xlabel('开关频率 (kHz)');
    ylabel('损耗 (W)');
    title('开关频率对损耗的影响');
    legend;
    grid on;
    
    subplot(2,1,2);
    plot(results(:,1), results(:,5), 'g-d', 'LineWidth', 2);
    xlabel('开关频率 (kHz)');
    ylabel('结温 (°C)');
    title('开关频率对结温的影响');
    grid on;
end   