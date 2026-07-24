function [result]=gen_coe_txt_mif(data_in,width,depth,path)
% type = extractAfter(path,'.');
[~, ~, ext] = fileparts(path);
% data_in = data_in * 2 ^ (width-1);%小数的时候打开
% data_in = data_in ;%整数
q = quantizer([width,0]);
%hexStr = num2hex(q,data_in);
hexStr = dec2hex(data_in);
x = strings(depth,1);
for addr1 = 1:depth
    x(addr1) = hexStr(addr1,:);
end

fid=fopen(path,'wt');  %打开或者新建文件，存放位置和文件名任意
switch(ext)
    case '.coe'
        fprintf(fid,'memory_initialization_radix=16;\n');
        fprintf(fid,'memory_initialization_vector=');

        for addr2 = 1:depth-1
             fprintf(fid,'\n %s,',x(addr2));%开始写数据了
        end
        fprintf(fid,'\n %s;',x(depth));
        fclose(fid);
    case '.txt'
        for addr2 = 1:depth
             fprintf(fid,'%s\n',x(addr2));%开始写数据了
        end
        fclose(fid);
    case '.mif'
        fprintf(fid,'DEPTH=%d;\n',depth);
        fprintf(fid,'WIDTH=%d;\n',width);
        fprintf(fid,'ADDRESS_RADIX = HEX;\n');
        fprintf(fid,'DATA_RADIX = HEX;\n');
        fprintf(fid,'CONTENT BEGIN');
        for addr2 = 1:depth-1
             fprintf(fid,'\n %x:%s,',addr2-1,x(addr2));%开始写数据了
        end
        fprintf(fid,'\n %x:%s;',depth-1,x(depth));
        fprintf(fid,'\n END;');
        fclose(fid);
    otherwise
        disp(type_error');
end
result = 1;