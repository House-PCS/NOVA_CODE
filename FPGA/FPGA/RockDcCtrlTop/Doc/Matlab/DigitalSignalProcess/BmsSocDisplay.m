%%
%--------------------------------------------------------------------------%
% 时间： 2025.08.27
% 作者： 许成洋
% 目的： 多个电池包SOC显示计算
% 主要问题：直接平均值的方法会导致SOC达不到100%，或者当最后一个达到100%的时候，部分会过压
%           按最大值和最小值均不合适
%--------------------------------------------------------------------------%
clc;
clear all;
close all;
%% 参数定义
a       = 0;
b       = 1;
BatNum  = 20;
SOC         = sort(a+(b-a).*rand(50,BatNum)); %10节电芯 SOC从0-1

%%

SOC_average = mean(SOC,2);      %多个电池包平均值
SOC_min     = min(SOC,[],2);    %多个电池包最小值
SOC_max     = max(SOC,[],2);    %多个电池包最大值

SOC_Real1    = SOC_max.*SOC_max + (1-SOC_max).*SOC_average;
SOC_Real2    = (1-SOC_min).*SOC_min + (SOC_min).*SOC_average;
SOC_Real     = SOC_max.*SOC_Real1 + (1-SOC_max).*SOC_Real2;

figure(1)
subplot(3,1,1); plot(SOC); 
subplot(3,1,2); plot(SOC_average); 
subplot(3,1,3); plot([SOC_min,SOC_max]); 

figure(2)
 plot(SOC_Real1,'*- r');hold on
 plot(SOC_Real2,'*- g');  hold on
plot(SOC);  hold on
plot(SOC_Real,'*- b');
title('多块电池包的SOC值和显示值');


