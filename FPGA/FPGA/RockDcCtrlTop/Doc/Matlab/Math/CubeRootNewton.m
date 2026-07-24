%%
close;
clear;


% 电流单位1mA 电压单位0.1V
% 电流单位1mA 电压单位0.1V
Current = -10000:1:10000;
Voltage = (10*nthroot(Current, 3)+4957);
Power   = Current.*Voltage/10000;  

% 计算阻抗 (dV/dI)
% 使用差分近似导数
dVoltage = diff(Voltage/10);
dCurrent = diff(Current/100);
Impedance = dVoltage ./ dCurrent;

figure;
subplot(3,1,1); % 改为3行1列
plot(Current, Voltage);
xlabel('Current (mA)');
ylabel('Voltage (0.1V)');
title('Voltage = Current^{1/3}');
grid on;

subplot(3,1,2);
plot(Current, Power);
xlabel('Current (mA)');
ylabel('Power (W)');
title('Power = Current × Voltage');
grid on;

% 添加阻抗曲线
subplot(3,1,3);
plot(Current(1:end-1), Impedance); % 注意长度比Current少1
xlabel('Current (mA)');
ylabel('Impedance (dVoltage/dCurrent)');
title('Impedance Curve');
grid on;

% 调整子图间距
sgtitle('Current-Voltage-Power-Impedance Characteristics');

% U = 800     ;%V
% %L = 100e-6 ;%H
% F = 70e3    ;%Hz
% dT =1/2/F   ;%s
% dI = 1.5    ;% 1.5A的静态峰值电流    
% % U = L*dI/dT ;
% 
% L = U/(dI/dT)
% 
% %%谐振计算
% Lr = 8e-6;
% C  =(1/F/2/pi)^2/Lr

%牛顿迭代法求解开立方
%a=10000;
% j =1;
% for a = -32767:1:32767
%     x(1)=a/2;
%     for k = 1:20
%         x(k+1) = (2*x(k) + a/(x(k)*x(k))) / 3;
%     end
%     j=j+1;
%     b(j)=fix(x(k+1));
% end
% %x(k+1)
%  figure;
% plot(b);
% save('data.txt', 'b','-ASCII');

Xi = [0,0.1,0.2,0.3,0.5,0.6,1,1.5,2,2.5,3,3.5,4,4.5,5,5.5,6,6.5,7,7.5,8]
Yu = [495.7,495.7,495.8,496.3,496.7,496.8,497.5,497.7,498.5,499.7,500.2,500.8,500.9,501.3,501.8,502.07,502.5,502.9,503.2,503.73,504.22];
Xi1 = [-3,-3.5,-4,-4.5,-5,-5.5,-6,-6.5,-7,-7.5,-8,-8.5];
Yu1 = [478.3,477.6,476.6,476.18,475.6,474.3,474.1,473.2,473.1,472.9,472.2,471.8];
figure;
%绘制Yu相对于Xi的图形
plot(Xi, Yu);
xlabel('Xi');
ylabel('Yu');
title('Yu vs Xi');





