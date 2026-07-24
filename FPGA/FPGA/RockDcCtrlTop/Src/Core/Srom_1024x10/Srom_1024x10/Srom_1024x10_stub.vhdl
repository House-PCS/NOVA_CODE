-- Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
-- Date        : Fri Jun 26 18:50:31 2026
-- Host        : 202404110226-P1 running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode synth_stub -rename_top Srom_1024x10 -prefix
--               Srom_1024x10_ Srom_1024x10_stub.vhdl
-- Design      : Srom_1024x10
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xc7a50tfgg484-2
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity Srom_1024x10 is
  Port ( 
    clka : in STD_LOGIC;
    ena : in STD_LOGIC;
    addra : in STD_LOGIC_VECTOR ( 9 downto 0 );
    douta : out STD_LOGIC_VECTOR ( 9 downto 0 )
  );

end Srom_1024x10;

architecture stub of Srom_1024x10 is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "clka,ena,addra[9:0],douta[9:0]";
attribute x_core_info : string;
attribute x_core_info of stub : architecture is "blk_mem_gen_v8_4_3,Vivado 2019.1";
begin
end;
