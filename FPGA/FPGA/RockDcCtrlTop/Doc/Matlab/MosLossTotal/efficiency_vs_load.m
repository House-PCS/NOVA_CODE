
%% 辅助函数：扫描负载电流对效率的影响
function efficiency_vs_load(base_config)
    fprintf('\n=== 负载电流扫描分析 ===\n');
    
    % 基础配置
    base_config.Vin = 12;
    base_config.Vout = 24;
    base_config.fsw = 100e3;
    base_config.L = 10e-6;
    base_config.RL = 0.005;
    base_config.plot_results = false;
    
    % MOSFET参数
    base_config.Rds_on_ctrl = 0.005;
    base_config.Rds_on_sync = 0.003;
    base_config.Qg_ctrl = 25;
    base_config.Qg_sync = 20;
    base_config.Qgd_ctrl = 12;
    base_config.Qgd_sync = 10;
    base_config.Coss_ctrl = 300e-12;
    base_config.Coss_sync = 400e-12;
    base_config.Qrr = 15;
    
    % 扫描负载电流
    I_out_range = [0.5, 1, 2, 3, 4, 5, 6, 7, 8];
    efficiencies = zeros(size(I_out_range));
    ctrl_losses = zeros(size(I_out_range));
    sync_losses = zeros(size(I_out_range));
    
    fprintf('负载电流(A)\t效率(%%)\t控制管损耗(W)\t同步管损耗(W)\n');
    fprintf('--------------------------------------------------------\n');
    
    for i = 1:length(I_out_range)
        config = base_config;
        config.I_out = I_out_range(i);
        
        [~, results] = calculate_sync_boost_loss(config);
        
        efficiencies(i) = results.efficiency;
        ctrl_losses(i) = results.control.total_loss;
        sync_losses(i) = results.sync.total_loss;
        
        fprintf('%.1f\t\t%.2f\t\t%.3f\t\t%.3f\n', ...
            I_out_range(i), efficiencies(i), ctrl_losses(i), sync_losses(i));
    end
    
    % 绘制效率曲线
    figure;
    subplot(2,1,1);
    plot(I_out_range, efficiencies, 'b-o', 'LineWidth', 2);
    xlabel('输出电流 (A)');
    ylabel('效率 (%)');
    title('同步Boost效率 vs 负载电流');
    grid on;
    
    subplot(2,1,2);
    plot(I_out_range, ctrl_losses, 'r-s', 'LineWidth', 2, 'DisplayName', '控制管损耗');
    hold on;
    plot(I_out_range, sync_losses, 'g-^', 'LineWidth', 2, 'DisplayName', '同步管损耗');
    xlabel('输出电流 (A)');
    ylabel('损耗 (W)');
    title('MOSFET损耗 vs 负载电流');
    legend;
    grid on;
end