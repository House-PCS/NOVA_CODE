clear;
clc;
close;

width = 11;
N = 1024;
t = linspace(-4, 0,N);%选取采样点
y = 2.^(1*t);

gen_coe_txt_mif(y,width,1024,'./SlowLaunchCtrlCore.coe')


 y1 = y * 2 ^ (width-1);%小数的时候打开
q = quantizer([width,0]);
hexStr = num2hex(q,y1);
plot(y1);


% fid = fopen('./CortexM0.bin', 'rb'); % 'rb' 表示以二进制只读模式打开文件
% if fid == -1
%   error('无法打开文件');
% end
% 
% data =(uint8(fread(fid, 'uint8'))) ; % 读取 100 行 50 列的单精度浮点数数据
% fclose(fid);
% 
% [m,n]     = size(data);  %得到数据长度m 
% Remainder = mod(m, 4) ;  %求取余数
% 
% itcm1 = uint8(zeros(round(m/4),1));
% itcm2 = uint8(zeros(round(m/4),1));
% itcm3 = uint8(zeros(round(m/4),1));
% itcm4 = uint8(zeros(round(m/4),1));
% 
% for i = 1:round(m/4)-1
%    itcm1(i) = uint8(data(4*(i-1)+1,1));
%    itcm2(i) = data(4*(i-1)+2,1);
%    itcm3(i) = data(4*(i-1)+3,1);
%    itcm4(i) = data(4*(i-1)+4,1);
% end
% 
% gen_coe_txt_mif(itcm1,8,round(m/4),'./itcm1.coe')
% gen_coe_txt_mif(itcm2,8,round(m/4),'./itcm2.coe')
% gen_coe_txt_mif(itcm3,8,round(m/4),'./itcm3.coe')
% gen_coe_txt_mif(itcm4,8,round(m/4),'./itcm4.coe')





% switch Remainder
%     case 0
%         itcm1 = zeros(round(m/4),1)
%         itcm2 = zeros(round(m/4),1)
%         itcm3 = zeros(round(m/4),1)
%         itcm4 = zeros(round(m/4),1)
%     case 1
%         itcm1 = zeros(round(m/4)+1,1)
%         itcm2 = zeros(round(m/4),1)
%         itcm3 = zeros(round(m/4),1)
%         itcm4 = zeros(round(m/4),1)
%     case 2
%         itcm1 = zeros(round(m/4)+1,1)
%         itcm2 = zeros(round(m/4)+1,1)
%         itcm3 = zeros(round(m/4),1)
%         itcm4 = zeros(round(m/4),1)
%     case 3
%         itcm1 = zeros(round(m/4)+1,1)
%         itcm2 = zeros(round(m/4)+1,1)
%         itcm3 = zeros(round(m/4)+1,1)
%         itcm4 = zeros(round(m/4),1)
%     otherwise
%         warning('err')
% end



% switch Remainder
%     case 1
%         itcm1(i+1) = data(4*(i)+1,1);
%     case 2
%         itcm1(i+1) = data(4*(i)+1,1);
%         itcm2(i+1) = data(4*(i)+2,1);
%     case 3
%         itcm1(i+1) = data(4*(i)+1,1);
%         itcm2(i+1) = data(4*(i)+2,1);
%         itcm3(i+1) = data(4*(i)+3,1);
% end
% u8itcm1 = dec2hex(uint8());

%plot(data);

