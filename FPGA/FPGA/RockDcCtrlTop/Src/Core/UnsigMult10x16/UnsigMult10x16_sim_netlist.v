// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
// Date        : Fri Jun 26 16:34:28 2026
// Host        : 202404110226-P1 running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode funcsim -rename_top UnsigMult10x16 -prefix
//               UnsigMult10x16_ UnsigMult10x16_sim_netlist.v
// Design      : UnsigMult10x16
// Purpose     : This verilog netlist is a functional simulation representation of the design and should not be modified
//               or synthesized. This netlist cannot be used for SDF annotated simulation.
// Device      : xc7a50tfgg484-2
// --------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

(* CHECK_LICENSE_TYPE = "UnsigMult10x16,mult_gen_v12_0_15,{}" *) (* downgradeipidentifiedwarnings = "yes" *) (* x_core_info = "mult_gen_v12_0_15,Vivado 2019.1" *) 
(* NotValidForBitStream *)
module UnsigMult10x16
   (CLK,
    A,
    B,
    P);
  (* x_interface_info = "xilinx.com:signal:clock:1.0 clk_intf CLK" *) (* x_interface_parameter = "XIL_INTERFACENAME clk_intf, ASSOCIATED_BUSIF p_intf:b_intf:a_intf, ASSOCIATED_RESET sclr, ASSOCIATED_CLKEN ce, FREQ_HZ 10000000, PHASE 0.000, INSERT_VIP 0" *) input CLK;
  (* x_interface_info = "xilinx.com:signal:data:1.0 a_intf DATA" *) (* x_interface_parameter = "XIL_INTERFACENAME a_intf, LAYERED_METADATA undef" *) input [9:0]A;
  (* x_interface_info = "xilinx.com:signal:data:1.0 b_intf DATA" *) (* x_interface_parameter = "XIL_INTERFACENAME b_intf, LAYERED_METADATA undef" *) input [15:0]B;
  (* x_interface_info = "xilinx.com:signal:data:1.0 p_intf DATA" *) (* x_interface_parameter = "XIL_INTERFACENAME p_intf, LAYERED_METADATA undef" *) output [25:0]P;

  wire [9:0]A;
  wire [15:0]B;
  wire CLK;
  wire [25:0]P;
  wire [47:0]NLW_U0_PCASC_UNCONNECTED;
  wire [1:0]NLW_U0_ZERO_DETECT_UNCONNECTED;

  (* C_A_TYPE = "1" *) 
  (* C_A_WIDTH = "10" *) 
  (* C_B_TYPE = "1" *) 
  (* C_B_VALUE = "10000001" *) 
  (* C_B_WIDTH = "16" *) 
  (* C_CCM_IMP = "0" *) 
  (* C_CE_OVERRIDES_SCLR = "0" *) 
  (* C_HAS_CE = "0" *) 
  (* C_HAS_SCLR = "0" *) 
  (* C_HAS_ZERO_DETECT = "0" *) 
  (* C_LATENCY = "2" *) 
  (* C_MODEL_TYPE = "0" *) 
  (* C_MULT_TYPE = "1" *) 
  (* C_OPTIMIZE_GOAL = "1" *) 
  (* C_OUT_HIGH = "25" *) 
  (* C_OUT_LOW = "0" *) 
  (* C_ROUND_OUTPUT = "0" *) 
  (* C_ROUND_PT = "0" *) 
  (* C_VERBOSITY = "0" *) 
  (* C_XDEVICEFAMILY = "artix7" *) 
  (* downgradeipidentifiedwarnings = "yes" *) 
  UnsigMult10x16_mult_gen_v12_0_15 U0
       (.A(A),
        .B(B),
        .CE(1'b1),
        .CLK(CLK),
        .P(P),
        .PCASC(NLW_U0_PCASC_UNCONNECTED[47:0]),
        .SCLR(1'b0),
        .ZERO_DETECT(NLW_U0_ZERO_DETECT_UNCONNECTED[1:0]));
endmodule

(* C_A_TYPE = "1" *) (* C_A_WIDTH = "10" *) (* C_B_TYPE = "1" *) 
(* C_B_VALUE = "10000001" *) (* C_B_WIDTH = "16" *) (* C_CCM_IMP = "0" *) 
(* C_CE_OVERRIDES_SCLR = "0" *) (* C_HAS_CE = "0" *) (* C_HAS_SCLR = "0" *) 
(* C_HAS_ZERO_DETECT = "0" *) (* C_LATENCY = "2" *) (* C_MODEL_TYPE = "0" *) 
(* C_MULT_TYPE = "1" *) (* C_OPTIMIZE_GOAL = "1" *) (* C_OUT_HIGH = "25" *) 
(* C_OUT_LOW = "0" *) (* C_ROUND_OUTPUT = "0" *) (* C_ROUND_PT = "0" *) 
(* C_VERBOSITY = "0" *) (* C_XDEVICEFAMILY = "artix7" *) (* downgradeipidentifiedwarnings = "yes" *) 
module UnsigMult10x16_mult_gen_v12_0_15
   (CLK,
    A,
    B,
    CE,
    SCLR,
    ZERO_DETECT,
    P,
    PCASC);
  input CLK;
  input [9:0]A;
  input [15:0]B;
  input CE;
  input SCLR;
  output [1:0]ZERO_DETECT;
  output [25:0]P;
  output [47:0]PCASC;

  wire \<const0> ;
  wire [9:0]A;
  wire [15:0]B;
  wire CLK;
  wire [25:0]P;
  wire [47:0]NLW_i_mult_PCASC_UNCONNECTED;
  wire [1:0]NLW_i_mult_ZERO_DETECT_UNCONNECTED;

  assign PCASC[47] = \<const0> ;
  assign PCASC[46] = \<const0> ;
  assign PCASC[45] = \<const0> ;
  assign PCASC[44] = \<const0> ;
  assign PCASC[43] = \<const0> ;
  assign PCASC[42] = \<const0> ;
  assign PCASC[41] = \<const0> ;
  assign PCASC[40] = \<const0> ;
  assign PCASC[39] = \<const0> ;
  assign PCASC[38] = \<const0> ;
  assign PCASC[37] = \<const0> ;
  assign PCASC[36] = \<const0> ;
  assign PCASC[35] = \<const0> ;
  assign PCASC[34] = \<const0> ;
  assign PCASC[33] = \<const0> ;
  assign PCASC[32] = \<const0> ;
  assign PCASC[31] = \<const0> ;
  assign PCASC[30] = \<const0> ;
  assign PCASC[29] = \<const0> ;
  assign PCASC[28] = \<const0> ;
  assign PCASC[27] = \<const0> ;
  assign PCASC[26] = \<const0> ;
  assign PCASC[25] = \<const0> ;
  assign PCASC[24] = \<const0> ;
  assign PCASC[23] = \<const0> ;
  assign PCASC[22] = \<const0> ;
  assign PCASC[21] = \<const0> ;
  assign PCASC[20] = \<const0> ;
  assign PCASC[19] = \<const0> ;
  assign PCASC[18] = \<const0> ;
  assign PCASC[17] = \<const0> ;
  assign PCASC[16] = \<const0> ;
  assign PCASC[15] = \<const0> ;
  assign PCASC[14] = \<const0> ;
  assign PCASC[13] = \<const0> ;
  assign PCASC[12] = \<const0> ;
  assign PCASC[11] = \<const0> ;
  assign PCASC[10] = \<const0> ;
  assign PCASC[9] = \<const0> ;
  assign PCASC[8] = \<const0> ;
  assign PCASC[7] = \<const0> ;
  assign PCASC[6] = \<const0> ;
  assign PCASC[5] = \<const0> ;
  assign PCASC[4] = \<const0> ;
  assign PCASC[3] = \<const0> ;
  assign PCASC[2] = \<const0> ;
  assign PCASC[1] = \<const0> ;
  assign PCASC[0] = \<const0> ;
  assign ZERO_DETECT[1] = \<const0> ;
  assign ZERO_DETECT[0] = \<const0> ;
  GND GND
       (.G(\<const0> ));
  (* C_A_TYPE = "1" *) 
  (* C_A_WIDTH = "10" *) 
  (* C_B_TYPE = "1" *) 
  (* C_B_VALUE = "10000001" *) 
  (* C_B_WIDTH = "16" *) 
  (* C_CCM_IMP = "0" *) 
  (* C_CE_OVERRIDES_SCLR = "0" *) 
  (* C_HAS_CE = "0" *) 
  (* C_HAS_SCLR = "0" *) 
  (* C_HAS_ZERO_DETECT = "0" *) 
  (* C_LATENCY = "2" *) 
  (* C_MODEL_TYPE = "0" *) 
  (* C_MULT_TYPE = "1" *) 
  (* C_OPTIMIZE_GOAL = "1" *) 
  (* C_OUT_HIGH = "25" *) 
  (* C_OUT_LOW = "0" *) 
  (* C_ROUND_OUTPUT = "0" *) 
  (* C_ROUND_PT = "0" *) 
  (* C_VERBOSITY = "0" *) 
  (* C_XDEVICEFAMILY = "artix7" *) 
  (* downgradeipidentifiedwarnings = "yes" *) 
  UnsigMult10x16_mult_gen_v12_0_15_viv i_mult
       (.A(A),
        .B(B),
        .CE(1'b0),
        .CLK(CLK),
        .P(P),
        .PCASC(NLW_i_mult_PCASC_UNCONNECTED[47:0]),
        .SCLR(1'b0),
        .ZERO_DETECT(NLW_i_mult_ZERO_DETECT_UNCONNECTED[1:0]));
endmodule
`pragma protect begin_protected
`pragma protect version = 1
`pragma protect encrypt_agent = "XILINX"
`pragma protect encrypt_agent_info = "Xilinx Encryption Tool 2019.1"
`pragma protect key_keyowner="Cadence Design Systems.", key_keyname="cds_rsa_key", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=64)
`pragma protect key_block
KGg++J83s0yJ7o2/XMVLkRRTRjS0oC9h86tQjl1+xE1m53Uwmm0+K41skiYHo3Urr6lMQ4q2jL5Y
R/1NOu1WGg==

`pragma protect key_keyowner="Synopsys", key_keyname="SNPS-VCS-RSA-2", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=128)
`pragma protect key_block
jCBx8aLaNWpgdwu0tsffQfmLNKET4Uy44Upxw9AlkO9Ma9Y+tqZHrHroYhGJUxa/dyJZ7Z0HDJ1t
hUhVV6SjuhVMs1NLM1MVw9F3MTSW7MB/qx7j0WAj62FJgoxsCtt6g392p1JAAosX8yACeLKiQ0KF
mnMpugzqSRDI445k7So=

`pragma protect key_keyowner="Aldec", key_keyname="ALDEC15_001", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
zdO8kU0uCj5Mggk0oLUcYcllNQJVD7vxIj25evesPPwBvXuv6EUsbKmUaCAlFUyG0YQ0mxWxXmzV
V/dRqKxqZ1ZI8+mX4IFaTJSCcYctMZsCl+2EWvQQHakV4QzWuCyca1phNacrRJfur8Ssc/Mhbez3
GLQCRrSfyBYyi3u9J+SAJRcJapyB1syXXhclDtup6m1z2C5S+NX/ql6kVXkcd9P+C5ordunfutgU
6uco8UymF/9QFYiBCWlTkHAgd7DH3dCI1E72N2H/KpX0/0xFBk++NCVuNucOwd9h4/hAyr4L+SI0
6Dzmn6kaBO4lnMAj5P58GIeWO/EtqrPeWg4UJw==

`pragma protect key_keyowner="ATRENTA", key_keyname="ATR-SG-2015-RSA-3", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
FdbUT4bIXyyFULrG0eEn0kqX6tjVoWssNb1FURO5jvyN5IkvkkDKCSLsd4J+2RE35ttJ20+4IZm2
p3H/UGCxkuCYtlZzovVpVf93DlhFUM2iSGd/L3evdLLL8VYETZTScGFdFXqiqe4ggXPHQCSEPD+e
PmMIJTGQka0DD3H+w+9t5Po/+M8b4r1y70l3Py7aYMeCEsZ/yHRmk8szsOjUbwvFEJk8SPXrEERg
EYMIrbryPHXq5E2fCL7hTgHa+bzIdFQOc2/8wn8YMVTmIJCZLBZDXvGSSm16cifWzXKHbPSly8js
RAoD2yYva4rr9cUy8jEyEpUcPGnaJXBDnB7lsQ==

`pragma protect key_keyowner="Xilinx", key_keyname="xilinxt_2019_02", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
eGYl/A3vBqVYodgklvBXVlduDkQKDOe941//b/7D71XaDbW1Cqv7m5eqy+I7bUTyBfnKRV6WeTtg
K2eZlSMADPLNGmIEawb1T81kHA95L4SgxCaMDbzt0t5pO+IQTca0KxjvPFPjj860AZ/Y4IJCgD9Z
vZNfcSeez7bqGB9kVNzxh40hdeBm7XY8a+5R/yPufF2S8KSSaiPSvYwD8yXOBzVoRhqA9q5PWKTd
u6qoeWMnQ1r/hIDsge5oDE06b6+zC7odC460K8KIOtKzeCrfWezkynmD7wBR1fdIwh9FGe2Uq4lO
ZbT2QFx8Ga5NQIwIIZZci/uL4Tw/7+CPKEoddw==

`pragma protect key_keyowner="Mentor Graphics Corporation", key_keyname="MGC-VELOCE-RSA", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=128)
`pragma protect key_block
k1GN+kT7KgRIHJs5Cw+hQb7EZrReCsvXgXeCjz4o0RyqpPm8XlxoPCNX4kR8BSaVxBTPm8qGrOj8
IkQcLP4XpLGNjMzOE8knGvgjraCBhhY/bboSihIYbJYXuKW0k/ErxcqbMup3dsmp8N5M+ZYpiEuF
88HraBjchDshDh5xlcY=

`pragma protect key_keyowner="Mentor Graphics Corporation", key_keyname="MGC-VERIF-SIM-RSA-2", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
jzBUDUoUQBD0tzi9B/VXNwpoyjUIKBzxkVyikkxc/QHKpaIlgud+eCQD6psG9RUWZouQN8CQmJEY
0K5qgvfm7GxXMbjLUwnVBRg4Uzfc4OTySfJMu1k9/qGISvYwf4r0rzMMp9aPgp+ElEwTGx3z9N0A
vWNdEjCI2mqdxmP3Q9AYUPTudILppELRMP4SJijczuRIhtAKpxFjTP2gL8zQE0aq1kkWRZfaHW1t
wV7tZ/jCUxkX8uj8DL6Bei6oBC1nTm/FjPhi+htKla8XNUEftaqUre2/0Sxhsxl/FTAzaex9fCj4
AMt2l6o0FpW5JlLhGnTYhWm/bgsyGCPBg6lSjQ==

`pragma protect key_keyowner="Mentor Graphics Corporation", key_keyname="MGC-PREC-RSA", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
e2udVAfhks6SGNX2VUROMYme4RfIYq3Le026fqAhL5NnHUehYjYt0StGJaQNoi+gWOX+I6sfozbM
HOqOjJjokqnvvdnOzNZwKjREMU8Y31B09TYX4YtCMSk8Fwm2Hv9nAB4v8SNCLQAyxWPdJop5X9ly
hO7S2nCjzWrb2NVxmdxNO3fRuEokcH8kv4raMG8By7PHHjhirWL3murMLdmBOpchMURDmAQZC8UE
oMW1QVUL0ij5MsMX3a0U+NVkcyIIEnBuslOhqTGnXKNxQPaHkUdLcK7u9+KQWyAA5Jbn4TDzsbEU
QRdYxe24OG2xDHA5tRm3RziqyOAH4JSnAgLZkQ==

`pragma protect key_keyowner="Synplicity", key_keyname="SYNP15_1", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
rqQTmh3K6mYOxoNYcYxMey147mr/IlxBkzQHlmrjrW0ZebAGwjMYY/FrftP6+xR19aEqywxwCceX
VbX6PiSqkcKs5NAR8HbSLKZ6dIOIcqlq42ybaESvY8QsW0P7amFRBndYnOKbgCSzCDB0rSRTS4eT
dcYMrU/F0fxxAtd3Anbcq9mCcuah0176XAflFPYmqJt4y7zQ55eoO7zR5yl7M9yerBN0qXdgjYLT
QaHusDvLm6XtYaD7B3VhiNRc6uTkhJtyZel+S+mfZNxpT9AeD7p8u4E/3D70GLF5m8Ya1AsuUW3v
jX/MTO00f5D6Nlh/YiyXIn1Q9TPZ2snbnQUh1A==

`pragma protect data_method = "AES128-CBC"
`pragma protect encoding = (enctype = "BASE64", line_length = 76, bytes = 7200)
`pragma protect data_block
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
`pragma protect end_protected
`ifndef GLBL
`define GLBL
`timescale  1 ps / 1 ps

module glbl ();

    parameter ROC_WIDTH = 100000;
    parameter TOC_WIDTH = 0;

//--------   STARTUP Globals --------------
    wire GSR;
    wire GTS;
    wire GWE;
    wire PRLD;
    tri1 p_up_tmp;
    tri (weak1, strong0) PLL_LOCKG = p_up_tmp;

    wire PROGB_GLBL;
    wire CCLKO_GLBL;
    wire FCSBO_GLBL;
    wire [3:0] DO_GLBL;
    wire [3:0] DI_GLBL;
   
    reg GSR_int;
    reg GTS_int;
    reg PRLD_int;

//--------   JTAG Globals --------------
    wire JTAG_TDO_GLBL;
    wire JTAG_TCK_GLBL;
    wire JTAG_TDI_GLBL;
    wire JTAG_TMS_GLBL;
    wire JTAG_TRST_GLBL;

    reg JTAG_CAPTURE_GLBL;
    reg JTAG_RESET_GLBL;
    reg JTAG_SHIFT_GLBL;
    reg JTAG_UPDATE_GLBL;
    reg JTAG_RUNTEST_GLBL;

    reg JTAG_SEL1_GLBL = 0;
    reg JTAG_SEL2_GLBL = 0 ;
    reg JTAG_SEL3_GLBL = 0;
    reg JTAG_SEL4_GLBL = 0;

    reg JTAG_USER_TDO1_GLBL = 1'bz;
    reg JTAG_USER_TDO2_GLBL = 1'bz;
    reg JTAG_USER_TDO3_GLBL = 1'bz;
    reg JTAG_USER_TDO4_GLBL = 1'bz;

    assign (strong1, weak0) GSR = GSR_int;
    assign (strong1, weak0) GTS = GTS_int;
    assign (weak1, weak0) PRLD = PRLD_int;

    initial begin
	GSR_int = 1'b1;
	PRLD_int = 1'b1;
	#(ROC_WIDTH)
	GSR_int = 1'b0;
	PRLD_int = 1'b0;
    end

    initial begin
	GTS_int = 1'b1;
	#(TOC_WIDTH)
	GTS_int = 1'b0;
    end

endmodule
`endif
