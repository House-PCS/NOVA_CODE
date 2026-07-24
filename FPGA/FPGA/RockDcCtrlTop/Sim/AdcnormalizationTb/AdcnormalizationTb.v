`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/03/06 11:41:03
// Design Name: 
// Module Name: AdcnormalizationTb
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


module AdcnormalizationTb();

    // Inputs
    reg         iSysClk;
    reg [16:0]  A ;
    reg  [7:0]  B ;
    wire [16:0]   C;
    reg  iReset;
    // Outputs


     Instantiate the Unit Under Test (UUT)
     Adcnormalization uut (
         .iSysClk(iSysClk),
         .iReset(iReset),
         .A(A),
         .B(B),
         .C(C),
         .oDone(oDone)
     );

    // Clock generation
    initial begin
        iSysClk = 0;
        forever #5 iSysClk = ~iSysClk; // 100 MHz clock
    end
initial begin
    iReset = 1'b1;
    #100;
    iReset = 1'b0;
    A = 17'd10000;
    B = 17'd120;
end

    // Test stimulus
    // initial begin
    //     // Initialize Inputs
    //     iPortA = 12'h000;
    //     iPortB = 16'h0000;
    //     iPortC = 15'h0000;

    //     // Wait for global reset
    //     #100;

    //     // Apply test vectors
    //     iPortA = 12'h123; // Example value for iPortA
    //     iPortB = 16'h4567; // Example value for iPortB
    //     iPortC = 15'h89AB; // Example value for iPortC
    //     #100;

    //     iPortA = 12'h456; // Another example value for iPortA
    //     iPortB = 16'h789A; // Another example value for iPortB
    //     iPortC = 15'hBCDE; // Another example value for iPortC
    //     #100;

    //     iPortA = 12'h789; // Another example value for iPortA
    //     iPortB = 16'h9ABC; // Another example value for iPortB
    //     iPortC = 15'hDEF0; // Another example value for iPortC
    //     #100;

    //     // Add more test cases as needed

    //     // Finish simulation
    //     //$stop;
    // end

// always @(posedge iSysClk ) 
// begin
//     iPortA[11:0] <= iPortA[11:0] + 1'b1;
// end

// //assign iPortA = rPortA;
//     // Monitor the outputs
//     initial begin
//         $monitor("Time: %0t, iPortA: %h, iPortB: %h, iPortC: %h, oPortP: %h",
//                  $time, iPortA, iPortB, iPortC, oPortP);
//     end

endmodule