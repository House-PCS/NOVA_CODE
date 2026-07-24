//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   SoftReBoot.v
//Original Author   :   Gouxuhui
//Creation Date     :   
/*Description       :   

*/
//------------------------------------------------------------------------------
//Version           :   Rev 0.1
//Modifier          :   
//Modify Date       :   
/*Modify Record     :   

*/
//------------------------------------------------------------------------------
//Version           :   Rev 0.2
//Modifier          :   
//Modify Date       :   
/*Modify Record     :   

*/  
`timescale 1ns / 1ps
//------------------------------------------------------------------------------
module SoftReBootTB();

// 时钟参数
parameter RMII_CLK_PERIOD = 20;   // 50MHz

reg     ReBootEn = 'd0;
reg     ReBootMode='d0;
reg     Clk='d0;

initial begin
    Clk = 0;
    forever #(RMII_CLK_PERIOD/2) Clk = ~Clk;
end

initial begin
  ReBootEn = 1'b0;
  ReBootMode = 1'b0;
  #100;
  ReBootEn = 1'b1;
  ReBootMode = 1'b0;
  #40;
  ReBootEn = 1'b0; 
  #1000;
  ReBootEn = 1'b1;
  ReBootMode = 1'b1;
end

SoftReBoot SoftReBoot(
  .iClk         (Clk),
  .iReBootEn    (ReBootEn), // From MCU:Enable ;  0:Disable
  .iReBootMode  (ReBootMode)  // From MCU:0:APP  ;  1:Golden
);


endmodule