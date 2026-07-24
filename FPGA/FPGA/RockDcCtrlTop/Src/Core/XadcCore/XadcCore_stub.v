// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
// Date        : Fri Jun 26 16:32:43 2026
// Host        : 202404110226-P1 running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub -rename_top XadcCore -prefix
//               XadcCore_ XadcCore_stub.v
// Design      : XadcCore
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7a50tfgg484-2
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
module XadcCore(daddr_in, dclk_in, den_in, di_in, dwe_in, reset_in, 
  vauxp0, vauxn0, vauxp1, vauxn1, vauxp2, vauxn2, vauxp3, vauxn3, vauxp4, vauxn4, vauxp5, vauxn5, vauxp6, 
  vauxn6, vauxp7, vauxn7, vauxp8, vauxn8, vauxp9, vauxn9, vauxp10, vauxn10, vauxp11, vauxn11, vauxp12, 
  vauxn12, vauxp13, vauxn13, vauxp14, vauxn14, vauxp15, vauxn15, busy_out, channel_out, do_out, 
  drdy_out, eoc_out, eos_out, ot_out, vccaux_alarm_out, vccint_alarm_out, user_temp_alarm_out, 
  alarm_out, vp_in, vn_in)
/* synthesis syn_black_box black_box_pad_pin="daddr_in[6:0],dclk_in,den_in,di_in[15:0],dwe_in,reset_in,vauxp0,vauxn0,vauxp1,vauxn1,vauxp2,vauxn2,vauxp3,vauxn3,vauxp4,vauxn4,vauxp5,vauxn5,vauxp6,vauxn6,vauxp7,vauxn7,vauxp8,vauxn8,vauxp9,vauxn9,vauxp10,vauxn10,vauxp11,vauxn11,vauxp12,vauxn12,vauxp13,vauxn13,vauxp14,vauxn14,vauxp15,vauxn15,busy_out,channel_out[4:0],do_out[15:0],drdy_out,eoc_out,eos_out,ot_out,vccaux_alarm_out,vccint_alarm_out,user_temp_alarm_out,alarm_out,vp_in,vn_in" */;
  input [6:0]daddr_in;
  input dclk_in;
  input den_in;
  input [15:0]di_in;
  input dwe_in;
  input reset_in;
  input vauxp0;
  input vauxn0;
  input vauxp1;
  input vauxn1;
  input vauxp2;
  input vauxn2;
  input vauxp3;
  input vauxn3;
  input vauxp4;
  input vauxn4;
  input vauxp5;
  input vauxn5;
  input vauxp6;
  input vauxn6;
  input vauxp7;
  input vauxn7;
  input vauxp8;
  input vauxn8;
  input vauxp9;
  input vauxn9;
  input vauxp10;
  input vauxn10;
  input vauxp11;
  input vauxn11;
  input vauxp12;
  input vauxn12;
  input vauxp13;
  input vauxn13;
  input vauxp14;
  input vauxn14;
  input vauxp15;
  input vauxn15;
  output busy_out;
  output [4:0]channel_out;
  output [15:0]do_out;
  output drdy_out;
  output eoc_out;
  output eos_out;
  output ot_out;
  output vccaux_alarm_out;
  output vccint_alarm_out;
  output user_temp_alarm_out;
  output alarm_out;
  input vp_in;
  input vn_in;
endmodule
