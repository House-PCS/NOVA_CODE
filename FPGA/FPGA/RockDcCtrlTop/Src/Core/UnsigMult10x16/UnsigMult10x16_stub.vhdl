-- Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
-- Date        : Fri Jun 26 16:34:28 2026
-- Host        : 202404110226-P1 running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode synth_stub -rename_top UnsigMult10x16 -prefix
--               UnsigMult10x16_ UnsigMult10x16_stub.vhdl
-- Design      : UnsigMult10x16
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xc7a50tfgg484-2
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity UnsigMult10x16 is
  Port ( 
    CLK : in STD_LOGIC;
    A : in STD_LOGIC_VECTOR ( 9 downto 0 );
    B : in STD_LOGIC_VECTOR ( 15 downto 0 );
    P : out STD_LOGIC_VECTOR ( 25 downto 0 )
  );

end UnsigMult10x16;

architecture stub of UnsigMult10x16 is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "CLK,A[9:0],B[15:0],P[25:0]";
attribute x_core_info : string;
attribute x_core_info of stub : architecture is "mult_gen_v12_0_15,Vivado 2019.1";
begin
end;
