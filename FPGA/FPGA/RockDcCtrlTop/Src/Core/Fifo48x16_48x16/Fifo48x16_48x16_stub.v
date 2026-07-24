// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
// Date        : Fri Jun 26 16:32:53 2026
// Host        : 202404110226-P1 running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub -rename_top Fifo48x16_48x16 -prefix
//               Fifo48x16_48x16_ Fifo48x16_48x16_stub.v
// Design      : Fifo48x16_48x16
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7a50tfgg484-2
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* x_core_info = "fifo_generator_v13_2_4,Vivado 2019.1" *)
module Fifo48x16_48x16(rst, wr_clk, rd_clk, din, wr_en, rd_en, dout, full, 
  almost_full, empty, rd_data_count, wr_data_count, prog_full)
/* synthesis syn_black_box black_box_pad_pin="rst,wr_clk,rd_clk,din[47:0],wr_en,rd_en,dout[47:0],full,almost_full,empty,rd_data_count[3:0],wr_data_count[3:0],prog_full" */;
  input rst;
  input wr_clk;
  input rd_clk;
  input [47:0]din;
  input wr_en;
  input rd_en;
  output [47:0]dout;
  output full;
  output almost_full;
  output empty;
  output [3:0]rd_data_count;
  output [3:0]wr_data_count;
  output prog_full;
endmodule
