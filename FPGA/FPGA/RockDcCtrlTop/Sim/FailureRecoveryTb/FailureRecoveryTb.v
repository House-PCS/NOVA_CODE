`timescale 1ns / 1ns
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/03/10 22:21:23
// Design Name: 
// Module Name: FailureRecoveryTb
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


module FailureRecoveryTb();

    parameter CLK_FREQ = 2;       
    parameter FAILURE_RECOVERY_TIME = 2; 

    reg iSysClk;                       
    reg iDcdcErr;                      
    reg   En;
  
    wire oFailRecovery;               

  
    FailureRecovery #(
        .CLK_FREQ(CLK_FREQ),
        .FAILURE_RECOVERY_TIME(2),
        .ERR_TIMES            (3),           
        .COUNT_RECOVERY_PERIOD (15)       
    ) uut (
        .iSysClk(iSysClk),
        .iDcdcErr(iDcdcErr),
        .oFailRecovery(oFailRecovery)
    );


    initial begin
        iSysClk = 0;
        forever #5 iSysClk = ~iSysClk; 
    end

   
    initial begin
        iDcdcErr = 0;
       #100; 
        iDcdcErr = 1;
        #25_000_000;
        if(oFailRecovery)begin
           iDcdcErr = 0;
           $display("Test 1 Passed");
        end
        else begin
           $display("Test 1 Failed");
        end
        #20_000_000;
        iDcdcErr = 1;
        #25_000_000;
        if(oFailRecovery)begin
           iDcdcErr = 0;
           $display("Test 2 Passed");
        end
        else begin
           $display("Test 2 Failed");
        end
        #20_000_000; 
        iDcdcErr = 1;
        #25_000_000;
        if(oFailRecovery)begin
           iDcdcErr = 0;
           $display("Test 3 Passed");
        end
        else begin
           $display("Test 3 Failed");
        end
        #20_000_000;   
        iDcdcErr = 1;
        #25_000_000;
        if(oFailRecovery)begin
           iDcdcErr = 0;
           $display("Test 4 Passed");
        end
        else begin
           $display("Test 4 Failed");
        end
        #20_000_000;  
    end

endmodule
