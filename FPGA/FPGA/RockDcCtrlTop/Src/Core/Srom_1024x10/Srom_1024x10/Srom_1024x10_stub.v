// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
// Date        : Fri Jun 26 18:50:31 2026
// Host        : 202404110226-P1 running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub -rename_top Srom_1024x10 -prefix
//               Srom_1024x10_ Srom_1024x10_stub.v
// Design      : Srom_1024x10
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7a50tfgg484-2
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* x_core_info = "blk_mem_gen_v8_4_3,Vivado 2019.1" *)
module Srom_1024x10(clka, ena, addra, douta)
/* synthesis syn_black_box black_box_pad_pin="clka,ena,addra[9:0],douta[9:0]" */;
  input clka;
  input ena;
  input [9:0]addra;
  output [9:0]douta;
endmodule
