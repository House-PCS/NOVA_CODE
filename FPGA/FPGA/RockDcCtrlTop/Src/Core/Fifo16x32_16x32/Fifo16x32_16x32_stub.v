// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
// Date        : Fri Jun 26 16:32:53 2026
// Host        : 202404110226-P1 running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub -rename_top Fifo16x32_16x32 -prefix
//               Fifo16x32_16x32_ Fifo16x32_16x32_stub.v
// Design      : Fifo16x32_16x32
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7a50tfgg484-2
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* x_core_info = "fifo_generator_v13_2_4,Vivado 2019.1" *)
module Fifo16x32_16x32(clk, srst, din, wr_en, rd_en, dout, full, almost_full, 
  empty, data_count)
/* synthesis syn_black_box black_box_pad_pin="clk,srst,din[15:0],wr_en,rd_en,dout[15:0],full,almost_full,empty,data_count[5:0]" */;
  input clk;
  input srst;
  input [15:0]din;
  input wr_en;
  input rd_en;
  output [15:0]dout;
  output full;
  output almost_full;
  output empty;
  output [5:0]data_count;
endmodule
