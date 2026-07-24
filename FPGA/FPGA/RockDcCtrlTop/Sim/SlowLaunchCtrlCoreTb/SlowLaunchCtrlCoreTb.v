`timescale 1ns / 1ns
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/03/10 17:30:57
// Design Name: 
// Module Name: SlowLaunchCtrlCoreTb
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


module SlowLaunchCtrlCoreTb();

reg iSysClk;
reg iSlowLaunchEn;
wire oSlowLaunchEnd;
wire [9:0] oPwmDuty;

// Instantiate the Unit Under Test (UUT) with modified parameters for faster simulation
SlowLaunchCtrlCore #(
    .CLK_FREQ(10),          // 100MHz clock (10ns period)
    .SLOW_LAUNCH_TIME(1024*2)  // Total launch time = 1024ms (1ms per step)
) uut (
    .iSysClk(iSysClk),
    .iSlowLaunchEn(iSlowLaunchEn),
    .oSlowLaunchEnd(oSlowLaunchEnd),
    .oPwmDuty(oPwmDuty)
);

// Generate 100MHz clock signal
initial begin
    iSysClk = 0;
    forever #5 iSysClk = ~iSysClk; // 10ns period
end

// Test sequence
initial begin
    // Initialize inputs
    iSlowLaunchEn = 0;
    #1000; // Wait 100ns for initial stability

    // Activate slow launch
    iSlowLaunchEn = 1;
    
    $display("[%t] Slow launch enabled", $time);

    // Wait for completion
    wait(oSlowLaunchEnd == 1);

    // #4000000; // Additional observation after completion
    //    iSlowLaunchEn = 0;
    // #10000; // Wait 100ns for initial stability

    // // Activate slow launch
    //    iSlowLaunchEn = 1;

    // Verify final PWM duty cycle
    if (oPwmDuty === 10'h3FF)
        $display("[%t] Test PASSED: oPwmDuty reached 1023", $time);
    else
        $display("[%t] Test FAILED: oPwmDuty = %d (expected 1023)", $time, oPwmDuty);
    
  //  $finish;
end

// Monitor key signals
initial begin
    $monitor("Time=%t ns | PWM Duty=%4d | Launch End=%b",
             $time, oPwmDuty, oSlowLaunchEnd);
end

// Simulate ROM (output equals address)
// module Srom_1024x10 (
//   input clka,
//   input ena,
//   input [9:0] addra,
//   output [9:0] douta
// );
//   reg [9:0] mem [0:1023];
//   initial begin
//     for (integer i=0; i<1024; i=i+1)
//       mem[i] = i;
//   end
//   assign douta = ena ? mem[addra] : 0;
// endmodule

endmodule
