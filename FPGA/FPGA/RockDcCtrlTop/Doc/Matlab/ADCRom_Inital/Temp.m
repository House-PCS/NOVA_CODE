clear;
% T25=298.15;
% B=3450;
% R25=10000;
% Rt=43.44 * 1000;
% Temp1=1/(1/T25 + (1/B) * log(Rt/R25)) - 273.15;
% 
% temp = 8159 + (-25997)*(adc_in-434)/4096
% Temp1 = 4000;
% adc_in1 = (Temp1 - 8159)*4096/(-25997)+434;
% adc_in2 = (Temp1 - 9907)*4096/(-49529)+289;
% VTemp1 = (adc_in1/4096)*(6.8/1.6)
% VTemp2 = (adc_in2/4096)*(6.8/1.6)

% clear; clc; close all;
% Rt1= 70000
% A_IN1=3.3/(10000+ ((9400*Rt1)/(9400+Rt1)))*((9400*Rt1)/(9400+Rt1))/9400*8400
% Vin1=A_IN1*1.6/6.8
% adc1=Vin1*4096
% 
% adc=1287
% Vout=adc/4096;
% A_IN = Vout*6.8/1.6;
%A_IN=3.3/(10000+ ((9400*Rt)/(9400+Rt)))*((9400*Rt)/(9400+Rt))/9400*8400
% 定义方程：f(Rt) = A_IN_calc - A_IN_given = 0
%f = @(Rt) 3.3 / (10000 + (9400*Rt)/(9400+Rt)) * ((9400*Rt)/(9400+Rt)) / 9400 * 8400 - A_IN;
% 用 fzero 求解，初始猜测值可以取 10000 欧姆（10k）
%Rt_solution = fzero(f, 100);
%Rt=(2350000*A_IN)/(693-485*A_IN)
% 
% 
% 预分配数组
% 清空环境
clear; clc; close all;

% 预分配数组
adc_vals = (0:4095)';
Rt_solutions = zeros(length(adc_vals), 1);
prev_Rt = 10000;  % 初始猜测 10kΩ

% 限制边界 (欧姆)
Rt_min = 500;
Rt_max = 240000;

% 循环求解
for i = 1:length(adc_vals)
    adc = adc_vals(i);

    Vout = adc / 4096;               % ADC 输出比例
    A_IN = Vout * 6.8 / 1.6;         % 实际输入电压

    % 定义方程
    %f = @(Rt) 3.3 / (10000 + (9400*Rt)/(9400+Rt)) * ((9400*Rt)/(9400+Rt)) / 9400 * 8400 - A_IN;
    Rt=(2350000*A_IN)/(693-485*A_IN);
    if Rt > Rt_max
       Rt = Rt_max;
    elseif Rt < Rt_min
        Rt = Rt_min;
     end
        Rt_solutions(i) = Rt;
        prev_Rt = Rt;   % 更新初
end

% 将结果转换为千欧姆，保留两位小数
Rt_kOhm = Rt_solutions / 1000;
Rt_kOhm_rounded = round(Rt_kOhm, 2);

% 创建表格
T = table(adc_vals, Rt_kOhm_rounded, ...
    'VariableNames', {'ADC_Value', 'Rt_kOhm'});

% 存储为 Excel 文件
filename = 'Rt_result.xlsx';
writetable(T, filename);

fprintf('结果已保存至 %s\n', filename);

% 可选：显示前10行
disp('前10行数据：');
disp(T(1:10, :));


% 1. 读取温度-阻值表 (temp_resistance.csv)
T_R_table = readtable('temp_resistance.csv');
valid = ~isnan(T_R_table.Resistance);
T_valid = T_R_table.Temperature_C_(valid)+2;
R_valid = T_R_table.Resistance(valid);

% 2. 预处理去重并排序
data = sortrows([R_valid, T_valid], 1);
R_sorted = data(:,1);
T_sorted = data(:,2);
[~, ia] = unique(R_sorted, 'stable');
R_unique = R_sorted(ia);
T_unique = T_sorted(ia);

if ~issorted(R_unique)
    error('去重后阻值未排序');
end

%% 3. 读取 ADC-阻值表 (Rt_result.xlsx)
ADC_R_table = readtable('Rt_result.xlsx');
ADC_vals = ADC_R_table.ADC_Value;
Rt_kOhm = ADC_R_table.Rt_kOhm;

%% 4. 线性插值得到温度（初始）
T_result = zeros(size(Rt_kOhm));
for i = 1:length(Rt_kOhm)
    r = Rt_kOhm(i);
    %if r < min(R_unique) || r > max(R_unique)
    if r < 0.52
        T_result(i) = 125;
        % warning('ADC=%d 阻值 %.3f kΩ 超出范围', ADC_vals(i), r);
    else
        T_interp = interp1(R_unique, T_unique, r, 'linear');
        T_result(i) = round(T_interp);
    end
end

%% 5. 应用修正：ADC > 1348 时强制温度 = -40，电阻 = 224
idx = ADC_vals > 1348;
T_result(idx) = -40;
Rt_kOhm(idx) = 224;   % 覆盖电阻值

%% 6. 输出 CSV（三列）
output_table = table(ADC_vals, T_result, Rt_kOhm, ...
    'VariableNames', {'ADC_Value', 'Temperature_C', 'Resistance_kOhm'});
writetable(output_table, 'ADC_Temperature.csv');

disp('已生成 ADC_Temperature.csv，前10行：');
disp(output_table(1:10, :));
fprintf('共 %d 行，其中 ADC > 1348 的行数：%d\n', height(output_table), sum(idx));
%生成.coe文件
T = readtable('ADC_Temperature.csv');
ADC = T.ADC_Value;          % 地址，应为 0~4095 连续
Temp3 = T.Temperature_C;     % 温度（已整数化）

% 检查地址是否连续且从0开始
if ~isequal(ADC(:), (0:4095)')
    error('ADC 值不是从 0 到 4095 连续排列');
end

% 温度偏移处理：-40 -> 0, 125 -> 165
Temp_offset = Temp3 + 40;
% 确保数据在 0~255 范围内
if any(Temp_offset < 0) || any(Temp_offset > 255)
    error('偏移后温度超出 8-bit 无符号范围 (0~255)');
end

% 转换为十六进制字符串（2 位大写）
data_hex = dec2hex(Temp_offset, 2);

% 写入 .coe 文件
% 读取原始 CSV
T = readtable('ADC_Temperature.csv');
ADC = T.ADC_Value;
Temp3 = T.Temperature_C;   % 已经是有符号整数

% 确保地址连续从0到4095
if ~isequal(ADC(:), (0:4095)')
    error('ADC 值不是连续 0~4095');
end

% 合并：每两个连续地址取温度平均
new_addr = 0:2047;
new_temp = zeros(1, 2048);
for i = 1:2048
    t1 = Temp3(2*i - 1);
    t2 = Temp3(2*i);
    % 平均值四舍五入
    avg = round((t1 + t2) / 2);
    % 限制在 -128~127 范围内（理论上不会超出）
    if avg < -128, avg = -128; end
    if avg > 127, avg = 127; end
    new_temp(i) = avg;
end

% 转换为补码十六进制
data_hex = dec2hex(mod(new_temp, 256), 2);

% 写入 .coe 文件
fid = fopen('rom_init_11bit.coe', 'w');
fprintf(fid, 'memory_initialization_radix=16;\n');
fprintf(fid, 'memory_initialization_vector=\n');

num_per_line = 16;
for i = 1:length(data_hex)
    fprintf(fid, '%s', data_hex(i,:));
    if i < length(data_hex)
        if mod(i, num_per_line) == 0
            fprintf(fid, ',\n');
        else
            fprintf(fid, ',');
        end
    else
        fprintf(fid, ';');
    end
end
fclose(fid);

fprintf('已生成 rom_init_11bit.coe，地址 0~2047，数据为合并后的温度（平均）。\n');