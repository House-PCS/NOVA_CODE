// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
// Date        : Fri Jun 26 16:33:51 2026
// Host        : 202404110226-P1 running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub -rename_top SDPRam_2048x8_512x32 -prefix
//               SDPRam_2048x8_512x32_ SDPRam_2048x8_512x32_stub.v
// Design      : SDPRam_2048x8_512x32
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7a50tfgg484-2
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* x_core_info = "blk_mem_gen_v8_4_3,Vivado 2019.1" *)
module SDPRam_2048x8_512x32(clka, wea, addra, dina, clkb, addrb, doutb)
/* synthesis syn_black_box black_box_pad_pin="clka,wea[0:0],addra[10:0],dina[7:0],clkb,addrb[8:0],doutb[31:0]" */;
  input clka;
  input [0:0]wea;
  input [10:0]addra;
  input [7:0]dina;
  input clkb;
  input [8:0]addrb;
  output [31:0]doutb;
endmodule
