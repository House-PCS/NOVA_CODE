`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/09/17 14:39:40
// Design Name: 
// Module Name: LPFTustinTb
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module LPFTustinTb;

    reg iSysClk;
    reg iRst;
    reg signed [31:0] iInput;
    wire signed [31:0] oOutput;
    reg               rAdcSampleSequenceDone=0;
    integer     fp_input,cnt;   
    reg signed [31:0] InputMem[0:49997]  ;       
    
    LPFtustin #(
        .WC(32'd6280),
        .FS(32'd53418)
        //.PRECISION(32'sd1024)
    ) dut (
        .iSysClk(iSysClk),
        //.iRst(iRst),
        .iAdcSampleSequenceDone(rAdcSampleSequenceDone),
        .iInput(iInput),
        .oOutput(oOutput)
    );


    
   
    initial begin
        iSysClk = 0;
        forever #5 iSysClk = ~iSysClk; // 100MHz
    end

//------------------------------------------------------------------
//--------------------------------------------------------------------
    integer i;

    initial begin
        
        iRst = 1;
        iInput = 0;
        rAdcSampleSequenceDone=0;
       
        #20;
        iRst = 0;
    
        iInput = 32'sd0; 
        #1000; 

        fp_input = $fopen("data1.txt", "r");
        if (!fp_input) begin
            $display("文件打开失败！！！");
            $stop;
        end
        else begin
            $display("文件打开成功！！！");   
        end       
        cnt = 0;
       // 2. 循环读取所有十进制数（自动按空格/换行分割）
       while ($fscanf(fp_input, "%d", InputMem[cnt]) == 1) begin
            $display("读取到第 %0d 个数 = %d", cnt+1, InputMem[cnt]);
            cnt=cnt+1;
        end
        begin
            for(i = 1; i < 49998; i = i + 1) begin
                //iInput = 32'sd5000 * ($sin(i * 3.14*2*100/40000) + $sin(i * 3.14*2*9000/40000)); // 鐢熸垚姝ｅ鸡锟??
                //$fdisplay(fp_input, "%d", iInput);
                iInput = InputMem[i];
                //if(i>=1)begin
                  #10;
                  rAdcSampleSequenceDone=1;
                  #10;
                  rAdcSampleSequenceDone=0;
                  #230; 
                //end
                //else begin
                //  #250;
               // end

            end
        end

        iInput = 0;
        #1000;
        iInput = 32'sd2000; 
        #250;
        iInput = 0;
        #2000;
        

    $finish;
    end
    
endmodule
