clear all;
clc ;
close all;

% 1>定义电路参数：如输入电压、输出电压、输出电流、开关频率、占空比、环境温度等。
% 2>定义MOS管参数：如Rds(on)（在特定结温和Vgs下）、开关时间（tr, tf）或开关能量（Eon, Eoff）、栅极电荷Qg、热阻等。
% 3>计算导通损耗：P_cond = Irms^2 * Rds(on) * D
% 4>计算开关损耗：P_sw = (Eon + Eoff) * f_sw 或者使用近似公式 P_sw = 0.5 * Vds * Id * (tr+tf) * f_sw
% 5>计算驱动损耗：P_drive = Qg * Vgs * f_sw
% 6>计算总损耗：P_total = P_cond + P_sw + P_drive
% 7>计算结温：Tj = Ta + P_total * Rth_ja（其中Rth_ja是结到环境的热阻）

%% 方法2：使用配置结构体
%电路参数
config.Vin      = 256;       % 输入电压(V)
config.Vout     = 350;       % 输出电压(V)
config.I_out    = 2000/350;  % 输出电流(A)
config.fsw      = 80e3;      % 开关频率(Hz)
config.L        = 520e-6;    % 电感量(H)
config.RL       = 45e-3;     % 电感直流电阻DCR

%器件环境参数
config.Vgs          =   18 ;    % 栅极驱动电压 (V)
config.Tj_est       =   175;    % 估计结温 (°C)
config.Ta           =   25 ;    % 环境温度 (°C)
config.Rth_ja       =   35 ;    % (°C/W)
config.plot_results =   true;

% MOSFET参数 (基于具体器件：IPT65R040CM8)
config.Rds_on_ctrl  = 40e-3;    % 导通电阻 (Ω)
config.Qg_ctrl      = 80;       % 总栅极电荷 (nC)
config.Qgd_ctrl     = 25;       % 栅源电荷 (nC)
config.Coss_ctrl    = 44e-12;   % 输出电容 (F)
config.tr_ctrl      = 12e-9;   % 上升时间 (s)-----和外界有关
config.tf_ctrl      = 6e-9;   % 下降时间 (s)-----和外界有关
config.Qrr = 0.73e3;             % 反向恢复电荷(nC)

config.Rds_on_sync  = 40e-3;    % 导通电阻 (Ω)
config.Qg_sync      = 80;       % 总栅极电荷 (nC)
config.Qgd_sync     = 25;       % 栅源电荷 (nC)
config.Coss_sync    = 44e-12;   % 输出电容 (F)
config.tr_sync      = 6.6e-9;   % 上升时间 (s)-----和外界有关
config.tf_sync      = 5.8e-9;   % 下降时间 (s)-----和外界有关

[total_loss, results] = calculate_sync_boost_loss(config);



%% 分析不同开关频率下的损耗特性
%frequency_sweep();
%efficiency_vs_load(config);