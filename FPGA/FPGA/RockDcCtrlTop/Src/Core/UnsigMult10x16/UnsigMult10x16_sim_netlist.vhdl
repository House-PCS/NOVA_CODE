-- Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
-- Date        : Fri Jun 26 16:34:28 2026
-- Host        : 202404110226-P1 running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode funcsim -rename_top UnsigMult10x16 -prefix
--               UnsigMult10x16_ UnsigMult10x16_sim_netlist.vhdl
-- Design      : UnsigMult10x16
-- Purpose     : This VHDL netlist is a functional simulation representation of the design and should not be modified or
--               synthesized. This netlist cannot be used for SDF annotated simulation.
-- Device      : xc7a50tfgg484-2
-- --------------------------------------------------------------------------------
`protect begin_protected
`protect version = 1
`protect encrypt_agent = "XILINX"
`protect encrypt_agent_info = "Xilinx Encryption Tool 2019.1"
`protect key_keyowner="Cadence Design Systems.", key_keyname="cds_rsa_key", key_method="rsa"
`protect encoding = (enctype="BASE64", line_length=76, bytes=64)
`protect key_block
KGg++J83s0yJ7o2/XMVLkRRTRjS0oC9h86tQjl1+xE1m53Uwmm0+K41skiYHo3Urr6lMQ4q2jL5Y
R/1NOu1WGg==

`protect key_keyowner="Synopsys", key_keyname="SNPS-VCS-RSA-2", key_method="rsa"
`protect encoding = (enctype="BASE64", line_length=76, bytes=128)
`protect key_block
jCBx8aLaNWpgdwu0tsffQfmLNKET4Uy44Upxw9AlkO9Ma9Y+tqZHrHroYhGJUxa/dyJZ7Z0HDJ1t
hUhVV6SjuhVMs1NLM1MVw9F3MTSW7MB/qx7j0WAj62FJgoxsCtt6g392p1JAAosX8yACeLKiQ0KF
mnMpugzqSRDI445k7So=

`protect key_keyowner="Aldec", key_keyname="ALDEC15_001", key_method="rsa"
`protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`protect key_block
zdO8kU0uCj5Mggk0oLUcYcllNQJVD7vxIj25evesPPwBvXuv6EUsbKmUaCAlFUyG0YQ0mxWxXmzV
V/dRqKxqZ1ZI8+mX4IFaTJSCcYctMZsCl+2EWvQQHakV4QzWuCyca1phNacrRJfur8Ssc/Mhbez3
GLQCRrSfyBYyi3u9J+SAJRcJapyB1syXXhclDtup6m1z2C5S+NX/ql6kVXkcd9P+C5ordunfutgU
6uco8UymF/9QFYiBCWlTkHAgd7DH3dCI1E72N2H/KpX0/0xFBk++NCVuNucOwd9h4/hAyr4L+SI0
6Dzmn6kaBO4lnMAj5P58GIeWO/EtqrPeWg4UJw==

`protect key_keyowner="ATRENTA", key_keyname="ATR-SG-2015-RSA-3", key_method="rsa"
`protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`protect key_block
FdbUT4bIXyyFULrG0eEn0kqX6tjVoWssNb1FURO5jvyN5IkvkkDKCSLsd4J+2RE35ttJ20+4IZm2
p3H/UGCxkuCYtlZzovVpVf93DlhFUM2iSGd/L3evdLLL8VYETZTScGFdFXqiqe4ggXPHQCSEPD+e
PmMIJTGQka0DD3H+w+9t5Po/+M8b4r1y70l3Py7aYMeCEsZ/yHRmk8szsOjUbwvFEJk8SPXrEERg
EYMIrbryPHXq5E2fCL7hTgHa+bzIdFQOc2/8wn8YMVTmIJCZLBZDXvGSSm16cifWzXKHbPSly8js
RAoD2yYva4rr9cUy8jEyEpUcPGnaJXBDnB7lsQ==

`protect key_keyowner="Xilinx", key_keyname="xilinxt_2019_02", key_method="rsa"
`protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`protect key_block
eGYl/A3vBqVYodgklvBXVlduDkQKDOe941//b/7D71XaDbW1Cqv7m5eqy+I7bUTyBfnKRV6WeTtg
K2eZlSMADPLNGmIEawb1T81kHA95L4SgxCaMDbzt0t5pO+IQTca0KxjvPFPjj860AZ/Y4IJCgD9Z
vZNfcSeez7bqGB9kVNzxh40hdeBm7XY8a+5R/yPufF2S8KSSaiPSvYwD8yXOBzVoRhqA9q5PWKTd
u6qoeWMnQ1r/hIDsge5oDE06b6+zC7odC460K8KIOtKzeCrfWezkynmD7wBR1fdIwh9FGe2Uq4lO
ZbT2QFx8Ga5NQIwIIZZci/uL4Tw/7+CPKEoddw==

`protect key_keyowner="Mentor Graphics Corporation", key_keyname="MGC-VELOCE-RSA", key_method="rsa"
`protect encoding = (enctype="BASE64", line_length=76, bytes=128)
`protect key_block
k1GN+kT7KgRIHJs5Cw+hQb7EZrReCsvXgXeCjz4o0RyqpPm8XlxoPCNX4kR8BSaVxBTPm8qGrOj8
IkQcLP4XpLGNjMzOE8knGvgjraCBhhY/bboSihIYbJYXuKW0k/ErxcqbMup3dsmp8N5M+ZYpiEuF
88HraBjchDshDh5xlcY=

`protect key_keyowner="Mentor Graphics Corporation", key_keyname="MGC-VERIF-SIM-RSA-2", key_method="rsa"
`protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`protect key_block
jzBUDUoUQBD0tzi9B/VXNwpoyjUIKBzxkVyikkxc/QHKpaIlgud+eCQD6psG9RUWZouQN8CQmJEY
0K5qgvfm7GxXMbjLUwnVBRg4Uzfc4OTySfJMu1k9/qGISvYwf4r0rzMMp9aPgp+ElEwTGx3z9N0A
vWNdEjCI2mqdxmP3Q9AYUPTudILppELRMP4SJijczuRIhtAKpxFjTP2gL8zQE0aq1kkWRZfaHW1t
wV7tZ/jCUxkX8uj8DL6Bei6oBC1nTm/FjPhi+htKla8XNUEftaqUre2/0Sxhsxl/FTAzaex9fCj4
AMt2l6o0FpW5JlLhGnTYhWm/bgsyGCPBg6lSjQ==

`protect key_keyowner="Mentor Graphics Corporation", key_keyname="MGC-PREC-RSA", key_method="rsa"
`protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`protect key_block
GtHFyhBhrQ8BLXw1Lw0GG9ZvN6bHa7Zbqyyy0yv1s3uAA4pNGg1UFY1Ra1+0qE7iM1ukc9hmQuwa
NL5xNsaz2M5jOGym3YVCHs+2Lq0L1z8ZCRPtj8/+smsMMnd6O27S7inNcDXGbUdsM3ZitP8ERNBX
uVHmxARDp4pFWiBuaqXfgcbUTqlduFjgnTbx4ndiLZ9CWWfQEQ/7bZVRqINGmQI3qBF0JAFRvAuK
yaiRkQO4U1ml9T7tMb57CcXJeolo2QJMNCz9wNptUh901Kg+Vzb73Xod5kJ5vrQvbgBVI9W0yD96
VPvUrcGlQYYyl/bS78kbnF7i+9So83OLC2/K9w==

`protect key_keyowner="Synplicity", key_keyname="SYNP15_1", key_method="rsa"
`protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`protect key_block
ncCuN4Vb68WISYL5qZ3eNVN815cS1qnJqEZ0u5q7ZsQg+63G6zwXYrnQ5R0t2hLysuultPOkvx7F
sBvsWxHq3p5MpQ38UY6KRE8rrqCYDsiVoHs1qjgTowDMbSena0srIA1Ff4H16AG2P95/QAwYd4a+
B1d3gDYh43yCl+25SJkacF5XwFZB4sSO+JzZW6RTsAbVdmMvIUYsYExb00WYgt+VCJ9DYmT8zGZM
Qt56O74vJM0wv22dd3JJVKBwt6KOiHoVhRFTDLn7+iLAuvtYqPxdT6t/7zPDrD0HMcggljH5h+mc
BT906qJCDBdpktfgc06ZshDIsJcf7fBn22+G3Q==

`protect data_method = "AES128-CBC"
`protect encoding = (enctype = "BASE64", line_length = 76, bytes = 10144)
`protect data_block
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
`protect end_protected
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity UnsigMult10x16_mult_gen_v12_0_15 is
  port (
    CLK : in STD_LOGIC;
    A : in STD_LOGIC_VECTOR ( 9 downto 0 );
    B : in STD_LOGIC_VECTOR ( 15 downto 0 );
    CE : in STD_LOGIC;
    SCLR : in STD_LOGIC;
    ZERO_DETECT : out STD_LOGIC_VECTOR ( 1 downto 0 );
    P : out STD_LOGIC_VECTOR ( 25 downto 0 );
    PCASC : out STD_LOGIC_VECTOR ( 47 downto 0 )
  );
  attribute C_A_TYPE : integer;
  attribute C_A_TYPE of UnsigMult10x16_mult_gen_v12_0_15 : entity is 1;
  attribute C_A_WIDTH : integer;
  attribute C_A_WIDTH of UnsigMult10x16_mult_gen_v12_0_15 : entity is 10;
  attribute C_B_TYPE : integer;
  attribute C_B_TYPE of UnsigMult10x16_mult_gen_v12_0_15 : entity is 1;
  attribute C_B_VALUE : string;
  attribute C_B_VALUE of UnsigMult10x16_mult_gen_v12_0_15 : entity is "10000001";
  attribute C_B_WIDTH : integer;
  attribute C_B_WIDTH of UnsigMult10x16_mult_gen_v12_0_15 : entity is 16;
  attribute C_CCM_IMP : integer;
  attribute C_CCM_IMP of UnsigMult10x16_mult_gen_v12_0_15 : entity is 0;
  attribute C_CE_OVERRIDES_SCLR : integer;
  attribute C_CE_OVERRIDES_SCLR of UnsigMult10x16_mult_gen_v12_0_15 : entity is 0;
  attribute C_HAS_CE : integer;
  attribute C_HAS_CE of UnsigMult10x16_mult_gen_v12_0_15 : entity is 0;
  attribute C_HAS_SCLR : integer;
  attribute C_HAS_SCLR of UnsigMult10x16_mult_gen_v12_0_15 : entity is 0;
  attribute C_HAS_ZERO_DETECT : integer;
  attribute C_HAS_ZERO_DETECT of UnsigMult10x16_mult_gen_v12_0_15 : entity is 0;
  attribute C_LATENCY : integer;
  attribute C_LATENCY of UnsigMult10x16_mult_gen_v12_0_15 : entity is 2;
  attribute C_MODEL_TYPE : integer;
  attribute C_MODEL_TYPE of UnsigMult10x16_mult_gen_v12_0_15 : entity is 0;
  attribute C_MULT_TYPE : integer;
  attribute C_MULT_TYPE of UnsigMult10x16_mult_gen_v12_0_15 : entity is 1;
  attribute C_OPTIMIZE_GOAL : integer;
  attribute C_OPTIMIZE_GOAL of UnsigMult10x16_mult_gen_v12_0_15 : entity is 1;
  attribute C_OUT_HIGH : integer;
  attribute C_OUT_HIGH of UnsigMult10x16_mult_gen_v12_0_15 : entity is 25;
  attribute C_OUT_LOW : integer;
  attribute C_OUT_LOW of UnsigMult10x16_mult_gen_v12_0_15 : entity is 0;
  attribute C_ROUND_OUTPUT : integer;
  attribute C_ROUND_OUTPUT of UnsigMult10x16_mult_gen_v12_0_15 : entity is 0;
  attribute C_ROUND_PT : integer;
  attribute C_ROUND_PT of UnsigMult10x16_mult_gen_v12_0_15 : entity is 0;
  attribute C_VERBOSITY : integer;
  attribute C_VERBOSITY of UnsigMult10x16_mult_gen_v12_0_15 : entity is 0;
  attribute C_XDEVICEFAMILY : string;
  attribute C_XDEVICEFAMILY of UnsigMult10x16_mult_gen_v12_0_15 : entity is "artix7";
  attribute downgradeipidentifiedwarnings : string;
  attribute downgradeipidentifiedwarnings of UnsigMult10x16_mult_gen_v12_0_15 : entity is "yes";
end UnsigMult10x16_mult_gen_v12_0_15;

architecture STRUCTURE of UnsigMult10x16_mult_gen_v12_0_15 is
  signal \<const0>\ : STD_LOGIC;
  signal NLW_i_mult_PCASC_UNCONNECTED : STD_LOGIC_VECTOR ( 47 downto 0 );
  signal NLW_i_mult_ZERO_DETECT_UNCONNECTED : STD_LOGIC_VECTOR ( 1 downto 0 );
  attribute C_A_TYPE of i_mult : label is 1;
  attribute C_A_WIDTH of i_mult : label is 10;
  attribute C_B_TYPE of i_mult : label is 1;
  attribute C_B_VALUE of i_mult : label is "10000001";
  attribute C_B_WIDTH of i_mult : label is 16;
  attribute C_CCM_IMP of i_mult : label is 0;
  attribute C_CE_OVERRIDES_SCLR of i_mult : label is 0;
  attribute C_HAS_CE of i_mult : label is 0;
  attribute C_HAS_SCLR of i_mult : label is 0;
  attribute C_HAS_ZERO_DETECT of i_mult : label is 0;
  attribute C_LATENCY of i_mult : label is 2;
  attribute C_MODEL_TYPE of i_mult : label is 0;
  attribute C_MULT_TYPE of i_mult : label is 1;
  attribute C_OPTIMIZE_GOAL of i_mult : label is 1;
  attribute C_OUT_HIGH of i_mult : label is 25;
  attribute C_OUT_LOW of i_mult : label is 0;
  attribute C_ROUND_OUTPUT of i_mult : label is 0;
  attribute C_ROUND_PT of i_mult : label is 0;
  attribute C_VERBOSITY of i_mult : label is 0;
  attribute C_XDEVICEFAMILY of i_mult : label is "artix7";
  attribute downgradeipidentifiedwarnings of i_mult : label is "yes";
begin
  PCASC(47) <= \<const0>\;
  PCASC(46) <= \<const0>\;
  PCASC(45) <= \<const0>\;
  PCASC(44) <= \<const0>\;
  PCASC(43) <= \<const0>\;
  PCASC(42) <= \<const0>\;
  PCASC(41) <= \<const0>\;
  PCASC(40) <= \<const0>\;
  PCASC(39) <= \<const0>\;
  PCASC(38) <= \<const0>\;
  PCASC(37) <= \<const0>\;
  PCASC(36) <= \<const0>\;
  PCASC(35) <= \<const0>\;
  PCASC(34) <= \<const0>\;
  PCASC(33) <= \<const0>\;
  PCASC(32) <= \<const0>\;
  PCASC(31) <= \<const0>\;
  PCASC(30) <= \<const0>\;
  PCASC(29) <= \<const0>\;
  PCASC(28) <= \<const0>\;
  PCASC(27) <= \<const0>\;
  PCASC(26) <= \<const0>\;
  PCASC(25) <= \<const0>\;
  PCASC(24) <= \<const0>\;
  PCASC(23) <= \<const0>\;
  PCASC(22) <= \<const0>\;
  PCASC(21) <= \<const0>\;
  PCASC(20) <= \<const0>\;
  PCASC(19) <= \<const0>\;
  PCASC(18) <= \<const0>\;
  PCASC(17) <= \<const0>\;
  PCASC(16) <= \<const0>\;
  PCASC(15) <= \<const0>\;
  PCASC(14) <= \<const0>\;
  PCASC(13) <= \<const0>\;
  PCASC(12) <= \<const0>\;
  PCASC(11) <= \<const0>\;
  PCASC(10) <= \<const0>\;
  PCASC(9) <= \<const0>\;
  PCASC(8) <= \<const0>\;
  PCASC(7) <= \<const0>\;
  PCASC(6) <= \<const0>\;
  PCASC(5) <= \<const0>\;
  PCASC(4) <= \<const0>\;
  PCASC(3) <= \<const0>\;
  PCASC(2) <= \<const0>\;
  PCASC(1) <= \<const0>\;
  PCASC(0) <= \<const0>\;
  ZERO_DETECT(1) <= \<const0>\;
  ZERO_DETECT(0) <= \<const0>\;
GND: unisim.vcomponents.GND
     port map (
      G => \<const0>\
    );
i_mult: entity work.UnsigMult10x16_mult_gen_v12_0_15_viv
     port map (
      A(9 downto 0) => A(9 downto 0),
      B(15 downto 0) => B(15 downto 0),
      CE => '0',
      CLK => CLK,
      P(25 downto 0) => P(25 downto 0),
      PCASC(47 downto 0) => NLW_i_mult_PCASC_UNCONNECTED(47 downto 0),
      SCLR => '0',
      ZERO_DETECT(1 downto 0) => NLW_i_mult_ZERO_DETECT_UNCONNECTED(1 downto 0)
    );
end STRUCTURE;
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity UnsigMult10x16 is
  port (
    CLK : in STD_LOGIC;
    A : in STD_LOGIC_VECTOR ( 9 downto 0 );
    B : in STD_LOGIC_VECTOR ( 15 downto 0 );
    P : out STD_LOGIC_VECTOR ( 25 downto 0 )
  );
  attribute NotValidForBitStream : boolean;
  attribute NotValidForBitStream of UnsigMult10x16 : entity is true;
  attribute CHECK_LICENSE_TYPE : string;
  attribute CHECK_LICENSE_TYPE of UnsigMult10x16 : entity is "UnsigMult10x16,mult_gen_v12_0_15,{}";
  attribute downgradeipidentifiedwarnings : string;
  attribute downgradeipidentifiedwarnings of UnsigMult10x16 : entity is "yes";
  attribute x_core_info : string;
  attribute x_core_info of UnsigMult10x16 : entity is "mult_gen_v12_0_15,Vivado 2019.1";
end UnsigMult10x16;

architecture STRUCTURE of UnsigMult10x16 is
  signal NLW_U0_PCASC_UNCONNECTED : STD_LOGIC_VECTOR ( 47 downto 0 );
  signal NLW_U0_ZERO_DETECT_UNCONNECTED : STD_LOGIC_VECTOR ( 1 downto 0 );
  attribute C_A_TYPE : integer;
  attribute C_A_TYPE of U0 : label is 1;
  attribute C_A_WIDTH : integer;
  attribute C_A_WIDTH of U0 : label is 10;
  attribute C_B_TYPE : integer;
  attribute C_B_TYPE of U0 : label is 1;
  attribute C_B_VALUE : string;
  attribute C_B_VALUE of U0 : label is "10000001";
  attribute C_B_WIDTH : integer;
  attribute C_B_WIDTH of U0 : label is 16;
  attribute C_CCM_IMP : integer;
  attribute C_CCM_IMP of U0 : label is 0;
  attribute C_CE_OVERRIDES_SCLR : integer;
  attribute C_CE_OVERRIDES_SCLR of U0 : label is 0;
  attribute C_HAS_CE : integer;
  attribute C_HAS_CE of U0 : label is 0;
  attribute C_HAS_SCLR : integer;
  attribute C_HAS_SCLR of U0 : label is 0;
  attribute C_HAS_ZERO_DETECT : integer;
  attribute C_HAS_ZERO_DETECT of U0 : label is 0;
  attribute C_LATENCY : integer;
  attribute C_LATENCY of U0 : label is 2;
  attribute C_MODEL_TYPE : integer;
  attribute C_MODEL_TYPE of U0 : label is 0;
  attribute C_MULT_TYPE : integer;
  attribute C_MULT_TYPE of U0 : label is 1;
  attribute C_OPTIMIZE_GOAL : integer;
  attribute C_OPTIMIZE_GOAL of U0 : label is 1;
  attribute C_OUT_HIGH : integer;
  attribute C_OUT_HIGH of U0 : label is 25;
  attribute C_OUT_LOW : integer;
  attribute C_OUT_LOW of U0 : label is 0;
  attribute C_ROUND_OUTPUT : integer;
  attribute C_ROUND_OUTPUT of U0 : label is 0;
  attribute C_ROUND_PT : integer;
  attribute C_ROUND_PT of U0 : label is 0;
  attribute C_VERBOSITY : integer;
  attribute C_VERBOSITY of U0 : label is 0;
  attribute C_XDEVICEFAMILY : string;
  attribute C_XDEVICEFAMILY of U0 : label is "artix7";
  attribute downgradeipidentifiedwarnings of U0 : label is "yes";
  attribute x_interface_info : string;
  attribute x_interface_info of CLK : signal is "xilinx.com:signal:clock:1.0 clk_intf CLK";
  attribute x_interface_parameter : string;
  attribute x_interface_parameter of CLK : signal is "XIL_INTERFACENAME clk_intf, ASSOCIATED_BUSIF p_intf:b_intf:a_intf, ASSOCIATED_RESET sclr, ASSOCIATED_CLKEN ce, FREQ_HZ 10000000, PHASE 0.000, INSERT_VIP 0";
  attribute x_interface_info of A : signal is "xilinx.com:signal:data:1.0 a_intf DATA";
  attribute x_interface_parameter of A : signal is "XIL_INTERFACENAME a_intf, LAYERED_METADATA undef";
  attribute x_interface_info of B : signal is "xilinx.com:signal:data:1.0 b_intf DATA";
  attribute x_interface_parameter of B : signal is "XIL_INTERFACENAME b_intf, LAYERED_METADATA undef";
  attribute x_interface_info of P : signal is "xilinx.com:signal:data:1.0 p_intf DATA";
  attribute x_interface_parameter of P : signal is "XIL_INTERFACENAME p_intf, LAYERED_METADATA undef";
begin
U0: entity work.UnsigMult10x16_mult_gen_v12_0_15
     port map (
      A(9 downto 0) => A(9 downto 0),
      B(15 downto 0) => B(15 downto 0),
      CE => '1',
      CLK => CLK,
      P(25 downto 0) => P(25 downto 0),
      PCASC(47 downto 0) => NLW_U0_PCASC_UNCONNECTED(47 downto 0),
      SCLR => '0',
      ZERO_DETECT(1 downto 0) => NLW_U0_ZERO_DETECT_UNCONNECTED(1 downto 0)
    );
end STRUCTURE;
