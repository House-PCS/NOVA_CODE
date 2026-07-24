clear;
clc;
close;





width = 11;
N = 1024;
t = linspace(63, 4096,1024);%选取采样点

y = 1-1./(1+t/1024);

%y = 2.^(1*t);
plot(t,y);

gen_coe_txt_mif(y,width,1024,'./PwmLlinearizationCore.coe')

y1 =  y * 2 ^ (width-1); %小数的时候打开

q = quantizer([width,0]);
figure;
hexStr = num2hex(q,y1);
plot(y1);
