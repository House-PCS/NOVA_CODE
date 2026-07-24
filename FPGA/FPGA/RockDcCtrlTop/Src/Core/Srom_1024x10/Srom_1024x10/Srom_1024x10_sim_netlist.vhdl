-- Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
-- Date        : Fri Jun 26 18:50:31 2026
-- Host        : 202404110226-P1 running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode funcsim -rename_top Srom_1024x10 -prefix
--               Srom_1024x10_ Srom_1024x10_sim_netlist.vhdl
-- Design      : Srom_1024x10
-- Purpose     : This VHDL netlist is a functional simulation representation of the design and should not be modified or
--               synthesized. This netlist cannot be used for SDF annotated simulation.
-- Device      : xc7a50tfgg484-2
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity Srom_1024x10_blk_mem_gen_prim_wrapper_init is
  port (
    douta : out STD_LOGIC_VECTOR ( 9 downto 0 );
    clka : in STD_LOGIC;
    ena : in STD_LOGIC;
    addra : in STD_LOGIC_VECTOR ( 9 downto 0 )
  );
end Srom_1024x10_blk_mem_gen_prim_wrapper_init;

architecture STRUCTURE of Srom_1024x10_blk_mem_gen_prim_wrapper_init is
  signal \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_0\ : STD_LOGIC;
  signal \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_1\ : STD_LOGIC;
  signal \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_10\ : STD_LOGIC;
  signal \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_2\ : STD_LOGIC;
  signal \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_32\ : STD_LOGIC;
  signal \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_33\ : STD_LOGIC;
  signal \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_8\ : STD_LOGIC;
  signal \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_9\ : STD_LOGIC;
  signal \NLW_DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_DOBDO_UNCONNECTED\ : STD_LOGIC_VECTOR ( 15 downto 0 );
  signal \NLW_DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_DOPBDOP_UNCONNECTED\ : STD_LOGIC_VECTOR ( 1 downto 0 );
  attribute box_type : string;
  attribute box_type of \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram\ : label is "PRIMITIVE";
begin
\DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram\: unisim.vcomponents.RAMB18E1
    generic map(
      DOA_REG => 1,
      DOB_REG => 0,
      INITP_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_04 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_05 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_06 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_07 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_00 => X"0202020202020202020102010201020102010201020002000200020002000200",
      INIT_01 => X"0205020502050205020402040204020402040203020302030203020302030202",
      INIT_02 => X"0208020802080208020702070207020702070206020602060206020602050205",
      INIT_03 => X"020B020B020B020B020B020A020A020A020A020A020902090209020902090208",
      INIT_04 => X"020F020F020E020E020E020E020E020D020D020D020D020C020C020C020C020C",
      INIT_05 => X"0212021202120212021102110211021102110210021002100210020F020F020F",
      INIT_06 => X"0216021602150215021502150215021402140214021402130213021302130213",
      INIT_07 => X"021A021A02190219021902190218021802180218021702170217021702160216",
      INIT_08 => X"021E021E021D021D021D021D021C021C021C021C021B021B021B021B021A021A",
      INIT_09 => X"0302030203010301030103010300030003000300021F021F021F021F021E021E",
      INIT_0A => X"0306030603060306030503050305030403040304030403030303030303030302",
      INIT_0B => X"030B030B030A030A030A03090309030903090308030803080307030703070307",
      INIT_0C => X"0310030F030F030F030E030E030E030E030D030D030D030C030C030C030B030B",
      INIT_0D => X"0315031403140314031303130313031203120312031103110311031103100310",
      INIT_0E => X"031A031903190319031903180318031803170317031703160316031603150315",
      INIT_0F => X"031F031F031F031E031E031E031D031D031D031C031C031B031B031B031A031A",
      INIT_10 => X"0405040504040404040304030403040204020402040104010401040004000400",
      INIT_11 => X"040B040A040A040A040904090409040804080407040704070406040604060405",
      INIT_12 => X"0411041104100410040F040F040F040E040E040D040D040D040C040C040C040B",
      INIT_13 => X"0417041704170416041604150415041504140414041304130413041204120411",
      INIT_14 => X"041E041E041D041D041C041C041C041B041B041A041A041A0419041904180418",
      INIT_15 => X"0505050505040504050305030503050205020501050105000500041F041F041F",
      INIT_16 => X"050D050C050C050B050B050A050A050905090508050805070507050705060506",
      INIT_17 => X"0514051405130513051205120511051105100510050F050F050E050E050D050D",
      INIT_18 => X"051C051C051B051B051A051A0519051905180518051705170516051605150515",
      INIT_19 => X"0605060406040603060206020601060106000600051F051F051E051E051D051D",
      INIT_1A => X"060D060D060C060C060B060B060A060906090608060806070607060606060605",
      INIT_1B => X"0616061606150615061406140613061206120611061106100610060F060E060E",
      INIT_1C => X"0700061F061F061E061E061D061C061C061B061B061A06190619061806180617",
      INIT_1D => X"070A070907090708070707070706070607050704070407030702070207010701",
      INIT_1E => X"07140714071307120712071107100710070F070E070E070D070C070C070B070B",
      INIT_1F => X"071F071E071E071D071C071C071B071A071A0719071807180717071607160715",
      INIT_20 => X"080A080A08090808080808070806080508050804080308030802080108010800",
      INIT_21 => X"0816081608150814081308130812081108100810080F080E080D080D080C080B",
      INIT_22 => X"09030902090109000900081F081E081D081C081C081B081A0819081908180817",
      INIT_23 => X"0910090F090E090D090C090B090B090A09090908090709070906090509040903",
      INIT_24 => X"091D091C091B091A091A09190918091709160915091509140913091209110910",
      INIT_25 => X"0A0B0A0A0A090A080A080A070A060A050A040A030A020A010A010A00091F091E",
      INIT_26 => X"0A1A0A190A180A170A160A150A140A130A120A110A110A100A0F0A0E0A0D0A0C",
      INIT_27 => X"0B090B080B070B060B050B040B030B020B010B000B000A1F0A1E0A1D0A1C0A1B",
      INIT_28 => X"0B190B180B170B160B150B140B130B120B110B100B0F0B0E0B0D0B0C0B0B0B0A",
      INIT_29 => X"0C0A0C090C080C070C060C050C040C030C010C000B1F0B1E0B1D0B1C0B1B0B1A",
      INIT_2A => X"0C1B0C1A0C190C180C170C160C150C140C130C110C100C0F0C0E0C0D0C0C0C0B",
      INIT_2B => X"0D0E0D0D0D0B0D0A0D090D080D070D060D040D030D020D010D000C1F0C1E0C1D",
      INIT_2C => X"0E010E000D1E0D1D0D1C0D1B0D190D180D170D160D150D140D120D110D100D0F",
      INIT_2D => X"0E150E130E120E110E100E0E0E0D0E0C0E0B0E090E080E070E060E040E030E02",
      INIT_2E => X"0F090F080F070F050F040F030F020F000E1F0E1E0E1C0E1B0E1A0E180E170E16",
      INIT_2F => X"0F1F0F1E0F1C0F1B0F1A0F180F170F160F140F130F110F100F0F0F0D0F0C0F0B",
      INIT_30 => X"10161014101310121010100F100D100C100A1009100810061005100310021001",
      INIT_31 => X"110E110C110A1109110711061105110311021100101F101D101C101A10191017",
      INIT_32 => X"12061205120312021200111E111D111B111A111811171115111411121111110F",
      INIT_33 => X"1300121E121D121B121A121812161215121312111210120E120D120B12091208",
      INIT_34 => X"131B131913181316131413121311130F130D130C130A13081307130513031302",
      INIT_35 => X"14171415141414121410140E140C140B140914071405140414021400131E131D",
      INIT_36 => X"151515131511150F150D150B150915081506150415021500141E141D141B1419",
      INIT_37 => X"16131611160F160D160C160A16081606160416021600151E151C151A15181516",
      INIT_38 => X"17131711170F170D170B17091707170517031701161F161D161B161916171615",
      INIT_39 => X"181518131811180F180C180A18081806180418021800171E171C171A17181715",
      INIT_3A => X"1918191619131911190F190D190B19081906190419021900181E181B18191817",
      INIT_3B => X"1A1C1A1A1A181A151A131A111A0F1A0C1A0A1A081A051A031A01191F191C191A",
      INIT_3C => X"1C031C001B1E1B1B1B191B161B141B121B0F1B0D1B0B1B081B061B031B011A1F",
      INIT_3D => X"1D0A1D081D051D031D001C1E1C1B1C191C161C141C111C0F1C0C1C0A1C071C05",
      INIT_3E => X"1E141E111E0F1E0C1E091E071E041E021D1F1D1C1D1A1D171D151D121D101D0D",
      INIT_3F => X"1F1F1F1D1F1A1F171F141F121F0F1F0C1F0A1F071F041F011E1F1E1C1E191E17",
      INIT_A => X"00000",
      INIT_B => X"00000",
      INIT_FILE => "NONE",
      IS_CLKARDCLK_INVERTED => '0',
      IS_CLKBWRCLK_INVERTED => '0',
      IS_ENARDEN_INVERTED => '0',
      IS_ENBWREN_INVERTED => '0',
      IS_RSTRAMARSTRAM_INVERTED => '0',
      IS_RSTRAMB_INVERTED => '0',
      IS_RSTREGARSTREG_INVERTED => '0',
      IS_RSTREGB_INVERTED => '0',
      RAM_MODE => "TDP",
      RDADDR_COLLISION_HWCONFIG => "PERFORMANCE",
      READ_WIDTH_A => 18,
      READ_WIDTH_B => 18,
      RSTREG_PRIORITY_A => "REGCE",
      RSTREG_PRIORITY_B => "REGCE",
      SIM_COLLISION_CHECK => "ALL",
      SIM_DEVICE => "7SERIES",
      SRVAL_A => X"00000",
      SRVAL_B => X"00000",
      WRITE_MODE_A => "WRITE_FIRST",
      WRITE_MODE_B => "WRITE_FIRST",
      WRITE_WIDTH_A => 18,
      WRITE_WIDTH_B => 18
    )
        port map (
      ADDRARDADDR(13 downto 4) => addra(9 downto 0),
      ADDRARDADDR(3 downto 0) => B"0000",
      ADDRBWRADDR(13 downto 0) => B"00000000000000",
      CLKARDCLK => clka,
      CLKBWRCLK => clka,
      DIADI(15 downto 0) => B"0000000000000000",
      DIBDI(15 downto 0) => B"0000000000000000",
      DIPADIP(1 downto 0) => B"00",
      DIPBDIP(1 downto 0) => B"00",
      DOADO(15) => \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_0\,
      DOADO(14) => \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_1\,
      DOADO(13) => \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_2\,
      DOADO(12 downto 8) => douta(9 downto 5),
      DOADO(7) => \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_8\,
      DOADO(6) => \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_9\,
      DOADO(5) => \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_10\,
      DOADO(4 downto 0) => douta(4 downto 0),
      DOBDO(15 downto 0) => \NLW_DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_DOBDO_UNCONNECTED\(15 downto 0),
      DOPADOP(1) => \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_32\,
      DOPADOP(0) => \DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_n_33\,
      DOPBDOP(1 downto 0) => \NLW_DEVICE_7SERIES.NO_BMM_INFO.SP.SIMPLE_PRIM18.ram_DOPBDOP_UNCONNECTED\(1 downto 0),
      ENARDEN => ena,
      ENBWREN => '0',
      REGCEAREGCE => ena,
      REGCEB => '0',
      RSTRAMARSTRAM => '0',
      RSTRAMB => '0',
      RSTREGARSTREG => '0',
      RSTREGB => '0',
      WEA(1 downto 0) => B"00",
      WEBWE(3 downto 0) => B"0000"
    );
end STRUCTURE;
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity Srom_1024x10_blk_mem_gen_prim_width is
  port (
    douta : out STD_LOGIC_VECTOR ( 9 downto 0 );
    clka : in STD_LOGIC;
    ena : in STD_LOGIC;
    addra : in STD_LOGIC_VECTOR ( 9 downto 0 )
  );
end Srom_1024x10_blk_mem_gen_prim_width;

architecture STRUCTURE of Srom_1024x10_blk_mem_gen_prim_width is
begin
\prim_init.ram\: entity work.Srom_1024x10_blk_mem_gen_prim_wrapper_init
     port map (
      addra(9 downto 0) => addra(9 downto 0),
      clka => clka,
      douta(9 downto 0) => douta(9 downto 0),
      ena => ena
    );
end STRUCTURE;
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity Srom_1024x10_blk_mem_gen_generic_cstr is
  port (
    douta : out STD_LOGIC_VECTOR ( 9 downto 0 );
    clka : in STD_LOGIC;
    ena : in STD_LOGIC;
    addra : in STD_LOGIC_VECTOR ( 9 downto 0 )
  );
end Srom_1024x10_blk_mem_gen_generic_cstr;

architecture STRUCTURE of Srom_1024x10_blk_mem_gen_generic_cstr is
begin
\ramloop[0].ram.r\: entity work.Srom_1024x10_blk_mem_gen_prim_width
     port map (
      addra(9 downto 0) => addra(9 downto 0),
      clka => clka,
      douta(9 downto 0) => douta(9 downto 0),
      ena => ena
    );
end STRUCTURE;
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity Srom_1024x10_blk_mem_gen_top is
  port (
    douta : out STD_LOGIC_VECTOR ( 9 downto 0 );
    clka : in STD_LOGIC;
    ena : in STD_LOGIC;
    addra : in STD_LOGIC_VECTOR ( 9 downto 0 )
  );
end Srom_1024x10_blk_mem_gen_top;

architecture STRUCTURE of Srom_1024x10_blk_mem_gen_top is
begin
\valid.cstr\: entity work.Srom_1024x10_blk_mem_gen_generic_cstr
     port map (
      addra(9 downto 0) => addra(9 downto 0),
      clka => clka,
      douta(9 downto 0) => douta(9 downto 0),
      ena => ena
    );
end STRUCTURE;
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity Srom_1024x10_blk_mem_gen_v8_4_3_synth is
  port (
    douta : out STD_LOGIC_VECTOR ( 9 downto 0 );
    clka : in STD_LOGIC;
    ena : in STD_LOGIC;
    addra : in STD_LOGIC_VECTOR ( 9 downto 0 )
  );
end Srom_1024x10_blk_mem_gen_v8_4_3_synth;

architecture STRUCTURE of Srom_1024x10_blk_mem_gen_v8_4_3_synth is
begin
\gnbram.gnativebmg.native_blk_mem_gen\: entity work.Srom_1024x10_blk_mem_gen_top
     port map (
      addra(9 downto 0) => addra(9 downto 0),
      clka => clka,
      douta(9 downto 0) => douta(9 downto 0),
      ena => ena
    );
end STRUCTURE;
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity Srom_1024x10_blk_mem_gen_v8_4_3 is
  port (
    clka : in STD_LOGIC;
    rsta : in STD_LOGIC;
    ena : in STD_LOGIC;
    regcea : in STD_LOGIC;
    wea : in STD_LOGIC_VECTOR ( 0 to 0 );
    addra : in STD_LOGIC_VECTOR ( 9 downto 0 );
    dina : in STD_LOGIC_VECTOR ( 9 downto 0 );
    douta : out STD_LOGIC_VECTOR ( 9 downto 0 );
    clkb : in STD_LOGIC;
    rstb : in STD_LOGIC;
    enb : in STD_LOGIC;
    regceb : in STD_LOGIC;
    web : in STD_LOGIC_VECTOR ( 0 to 0 );
    addrb : in STD_LOGIC_VECTOR ( 9 downto 0 );
    dinb : in STD_LOGIC_VECTOR ( 9 downto 0 );
    doutb : out STD_LOGIC_VECTOR ( 9 downto 0 );
    injectsbiterr : in STD_LOGIC;
    injectdbiterr : in STD_LOGIC;
    eccpipece : in STD_LOGIC;
    sbiterr : out STD_LOGIC;
    dbiterr : out STD_LOGIC;
    rdaddrecc : out STD_LOGIC_VECTOR ( 9 downto 0 );
    sleep : in STD_LOGIC;
    deepsleep : in STD_LOGIC;
    shutdown : in STD_LOGIC;
    rsta_busy : out STD_LOGIC;
    rstb_busy : out STD_LOGIC;
    s_aclk : in STD_LOGIC;
    s_aresetn : in STD_LOGIC;
    s_axi_awid : in STD_LOGIC_VECTOR ( 3 downto 0 );
    s_axi_awaddr : in STD_LOGIC_VECTOR ( 31 downto 0 );
    s_axi_awlen : in STD_LOGIC_VECTOR ( 7 downto 0 );
    s_axi_awsize : in STD_LOGIC_VECTOR ( 2 downto 0 );
    s_axi_awburst : in STD_LOGIC_VECTOR ( 1 downto 0 );
    s_axi_awvalid : in STD_LOGIC;
    s_axi_awready : out STD_LOGIC;
    s_axi_wdata : in STD_LOGIC_VECTOR ( 9 downto 0 );
    s_axi_wstrb : in STD_LOGIC_VECTOR ( 0 to 0 );
    s_axi_wlast : in STD_LOGIC;
    s_axi_wvalid : in STD_LOGIC;
    s_axi_wready : out STD_LOGIC;
    s_axi_bid : out STD_LOGIC_VECTOR ( 3 downto 0 );
    s_axi_bresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
    s_axi_bvalid : out STD_LOGIC;
    s_axi_bready : in STD_LOGIC;
    s_axi_arid : in STD_LOGIC_VECTOR ( 3 downto 0 );
    s_axi_araddr : in STD_LOGIC_VECTOR ( 31 downto 0 );
    s_axi_arlen : in STD_LOGIC_VECTOR ( 7 downto 0 );
    s_axi_arsize : in STD_LOGIC_VECTOR ( 2 downto 0 );
    s_axi_arburst : in STD_LOGIC_VECTOR ( 1 downto 0 );
    s_axi_arvalid : in STD_LOGIC;
    s_axi_arready : out STD_LOGIC;
    s_axi_rid : out STD_LOGIC_VECTOR ( 3 downto 0 );
    s_axi_rdata : out STD_LOGIC_VECTOR ( 9 downto 0 );
    s_axi_rresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
    s_axi_rlast : out STD_LOGIC;
    s_axi_rvalid : out STD_LOGIC;
    s_axi_rready : in STD_LOGIC;
    s_axi_injectsbiterr : in STD_LOGIC;
    s_axi_injectdbiterr : in STD_LOGIC;
    s_axi_sbiterr : out STD_LOGIC;
    s_axi_dbiterr : out STD_LOGIC;
    s_axi_rdaddrecc : out STD_LOGIC_VECTOR ( 9 downto 0 )
  );
  attribute C_ADDRA_WIDTH : integer;
  attribute C_ADDRA_WIDTH of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 10;
  attribute C_ADDRB_WIDTH : integer;
  attribute C_ADDRB_WIDTH of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 10;
  attribute C_ALGORITHM : integer;
  attribute C_ALGORITHM of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1;
  attribute C_AXI_ID_WIDTH : integer;
  attribute C_AXI_ID_WIDTH of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 4;
  attribute C_AXI_SLAVE_TYPE : integer;
  attribute C_AXI_SLAVE_TYPE of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_AXI_TYPE : integer;
  attribute C_AXI_TYPE of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1;
  attribute C_BYTE_SIZE : integer;
  attribute C_BYTE_SIZE of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 9;
  attribute C_COMMON_CLK : integer;
  attribute C_COMMON_CLK of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_COUNT_18K_BRAM : string;
  attribute C_COUNT_18K_BRAM of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "1";
  attribute C_COUNT_36K_BRAM : string;
  attribute C_COUNT_36K_BRAM of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "0";
  attribute C_CTRL_ECC_ALGO : string;
  attribute C_CTRL_ECC_ALGO of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "NONE";
  attribute C_DEFAULT_DATA : string;
  attribute C_DEFAULT_DATA of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "0";
  attribute C_DISABLE_WARN_BHV_COLL : integer;
  attribute C_DISABLE_WARN_BHV_COLL of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_DISABLE_WARN_BHV_RANGE : integer;
  attribute C_DISABLE_WARN_BHV_RANGE of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_ELABORATION_DIR : string;
  attribute C_ELABORATION_DIR of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "./";
  attribute C_ENABLE_32BIT_ADDRESS : integer;
  attribute C_ENABLE_32BIT_ADDRESS of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_EN_DEEPSLEEP_PIN : integer;
  attribute C_EN_DEEPSLEEP_PIN of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_EN_ECC_PIPE : integer;
  attribute C_EN_ECC_PIPE of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_EN_RDADDRA_CHG : integer;
  attribute C_EN_RDADDRA_CHG of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_EN_RDADDRB_CHG : integer;
  attribute C_EN_RDADDRB_CHG of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_EN_SAFETY_CKT : integer;
  attribute C_EN_SAFETY_CKT of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_EN_SHUTDOWN_PIN : integer;
  attribute C_EN_SHUTDOWN_PIN of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_EN_SLEEP_PIN : integer;
  attribute C_EN_SLEEP_PIN of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_EST_POWER_SUMMARY : string;
  attribute C_EST_POWER_SUMMARY of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "Estimated Power for IP     :     1.229999 mW";
  attribute C_FAMILY : string;
  attribute C_FAMILY of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "artix7";
  attribute C_HAS_AXI_ID : integer;
  attribute C_HAS_AXI_ID of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_HAS_ENA : integer;
  attribute C_HAS_ENA of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1;
  attribute C_HAS_ENB : integer;
  attribute C_HAS_ENB of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_HAS_INJECTERR : integer;
  attribute C_HAS_INJECTERR of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_HAS_MEM_OUTPUT_REGS_A : integer;
  attribute C_HAS_MEM_OUTPUT_REGS_A of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1;
  attribute C_HAS_MEM_OUTPUT_REGS_B : integer;
  attribute C_HAS_MEM_OUTPUT_REGS_B of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_HAS_MUX_OUTPUT_REGS_A : integer;
  attribute C_HAS_MUX_OUTPUT_REGS_A of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_HAS_MUX_OUTPUT_REGS_B : integer;
  attribute C_HAS_MUX_OUTPUT_REGS_B of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_HAS_REGCEA : integer;
  attribute C_HAS_REGCEA of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_HAS_REGCEB : integer;
  attribute C_HAS_REGCEB of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_HAS_RSTA : integer;
  attribute C_HAS_RSTA of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_HAS_RSTB : integer;
  attribute C_HAS_RSTB of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_HAS_SOFTECC_INPUT_REGS_A : integer;
  attribute C_HAS_SOFTECC_INPUT_REGS_A of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_HAS_SOFTECC_OUTPUT_REGS_B : integer;
  attribute C_HAS_SOFTECC_OUTPUT_REGS_B of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_INITA_VAL : string;
  attribute C_INITA_VAL of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "0";
  attribute C_INITB_VAL : string;
  attribute C_INITB_VAL of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "0";
  attribute C_INIT_FILE : string;
  attribute C_INIT_FILE of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "Srom_1024x10.mem";
  attribute C_INIT_FILE_NAME : string;
  attribute C_INIT_FILE_NAME of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "Srom_1024x10.mif";
  attribute C_INTERFACE_TYPE : integer;
  attribute C_INTERFACE_TYPE of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_LOAD_INIT_FILE : integer;
  attribute C_LOAD_INIT_FILE of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1;
  attribute C_MEM_TYPE : integer;
  attribute C_MEM_TYPE of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 3;
  attribute C_MUX_PIPELINE_STAGES : integer;
  attribute C_MUX_PIPELINE_STAGES of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_PRIM_TYPE : integer;
  attribute C_PRIM_TYPE of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1;
  attribute C_READ_DEPTH_A : integer;
  attribute C_READ_DEPTH_A of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1024;
  attribute C_READ_DEPTH_B : integer;
  attribute C_READ_DEPTH_B of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1024;
  attribute C_READ_LATENCY_A : integer;
  attribute C_READ_LATENCY_A of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1;
  attribute C_READ_LATENCY_B : integer;
  attribute C_READ_LATENCY_B of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1;
  attribute C_READ_WIDTH_A : integer;
  attribute C_READ_WIDTH_A of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 10;
  attribute C_READ_WIDTH_B : integer;
  attribute C_READ_WIDTH_B of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 10;
  attribute C_RSTRAM_A : integer;
  attribute C_RSTRAM_A of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_RSTRAM_B : integer;
  attribute C_RSTRAM_B of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_RST_PRIORITY_A : string;
  attribute C_RST_PRIORITY_A of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "CE";
  attribute C_RST_PRIORITY_B : string;
  attribute C_RST_PRIORITY_B of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "CE";
  attribute C_SIM_COLLISION_CHECK : string;
  attribute C_SIM_COLLISION_CHECK of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "ALL";
  attribute C_USE_BRAM_BLOCK : integer;
  attribute C_USE_BRAM_BLOCK of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_USE_BYTE_WEA : integer;
  attribute C_USE_BYTE_WEA of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_USE_BYTE_WEB : integer;
  attribute C_USE_BYTE_WEB of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_USE_DEFAULT_DATA : integer;
  attribute C_USE_DEFAULT_DATA of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_USE_ECC : integer;
  attribute C_USE_ECC of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_USE_SOFTECC : integer;
  attribute C_USE_SOFTECC of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_USE_URAM : integer;
  attribute C_USE_URAM of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 0;
  attribute C_WEA_WIDTH : integer;
  attribute C_WEA_WIDTH of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1;
  attribute C_WEB_WIDTH : integer;
  attribute C_WEB_WIDTH of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1;
  attribute C_WRITE_DEPTH_A : integer;
  attribute C_WRITE_DEPTH_A of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1024;
  attribute C_WRITE_DEPTH_B : integer;
  attribute C_WRITE_DEPTH_B of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 1024;
  attribute C_WRITE_MODE_A : string;
  attribute C_WRITE_MODE_A of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "WRITE_FIRST";
  attribute C_WRITE_MODE_B : string;
  attribute C_WRITE_MODE_B of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "WRITE_FIRST";
  attribute C_WRITE_WIDTH_A : integer;
  attribute C_WRITE_WIDTH_A of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 10;
  attribute C_WRITE_WIDTH_B : integer;
  attribute C_WRITE_WIDTH_B of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is 10;
  attribute C_XDEVICEFAMILY : string;
  attribute C_XDEVICEFAMILY of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "artix7";
  attribute downgradeipidentifiedwarnings : string;
  attribute downgradeipidentifiedwarnings of Srom_1024x10_blk_mem_gen_v8_4_3 : entity is "yes";
end Srom_1024x10_blk_mem_gen_v8_4_3;

architecture STRUCTURE of Srom_1024x10_blk_mem_gen_v8_4_3 is
  signal \<const0>\ : STD_LOGIC;
begin
  dbiterr <= \<const0>\;
  doutb(9) <= \<const0>\;
  doutb(8) <= \<const0>\;
  doutb(7) <= \<const0>\;
  doutb(6) <= \<const0>\;
  doutb(5) <= \<const0>\;
  doutb(4) <= \<const0>\;
  doutb(3) <= \<const0>\;
  doutb(2) <= \<const0>\;
  doutb(1) <= \<const0>\;
  doutb(0) <= \<const0>\;
  rdaddrecc(9) <= \<const0>\;
  rdaddrecc(8) <= \<const0>\;
  rdaddrecc(7) <= \<const0>\;
  rdaddrecc(6) <= \<const0>\;
  rdaddrecc(5) <= \<const0>\;
  rdaddrecc(4) <= \<const0>\;
  rdaddrecc(3) <= \<const0>\;
  rdaddrecc(2) <= \<const0>\;
  rdaddrecc(1) <= \<const0>\;
  rdaddrecc(0) <= \<const0>\;
  rsta_busy <= \<const0>\;
  rstb_busy <= \<const0>\;
  s_axi_arready <= \<const0>\;
  s_axi_awready <= \<const0>\;
  s_axi_bid(3) <= \<const0>\;
  s_axi_bid(2) <= \<const0>\;
  s_axi_bid(1) <= \<const0>\;
  s_axi_bid(0) <= \<const0>\;
  s_axi_bresp(1) <= \<const0>\;
  s_axi_bresp(0) <= \<const0>\;
  s_axi_bvalid <= \<const0>\;
  s_axi_dbiterr <= \<const0>\;
  s_axi_rdaddrecc(9) <= \<const0>\;
  s_axi_rdaddrecc(8) <= \<const0>\;
  s_axi_rdaddrecc(7) <= \<const0>\;
  s_axi_rdaddrecc(6) <= \<const0>\;
  s_axi_rdaddrecc(5) <= \<const0>\;
  s_axi_rdaddrecc(4) <= \<const0>\;
  s_axi_rdaddrecc(3) <= \<const0>\;
  s_axi_rdaddrecc(2) <= \<const0>\;
  s_axi_rdaddrecc(1) <= \<const0>\;
  s_axi_rdaddrecc(0) <= \<const0>\;
  s_axi_rdata(9) <= \<const0>\;
  s_axi_rdata(8) <= \<const0>\;
  s_axi_rdata(7) <= \<const0>\;
  s_axi_rdata(6) <= \<const0>\;
  s_axi_rdata(5) <= \<const0>\;
  s_axi_rdata(4) <= \<const0>\;
  s_axi_rdata(3) <= \<const0>\;
  s_axi_rdata(2) <= \<const0>\;
  s_axi_rdata(1) <= \<const0>\;
  s_axi_rdata(0) <= \<const0>\;
  s_axi_rid(3) <= \<const0>\;
  s_axi_rid(2) <= \<const0>\;
  s_axi_rid(1) <= \<const0>\;
  s_axi_rid(0) <= \<const0>\;
  s_axi_rlast <= \<const0>\;
  s_axi_rresp(1) <= \<const0>\;
  s_axi_rresp(0) <= \<const0>\;
  s_axi_rvalid <= \<const0>\;
  s_axi_sbiterr <= \<const0>\;
  s_axi_wready <= \<const0>\;
  sbiterr <= \<const0>\;
GND: unisim.vcomponents.GND
     port map (
      G => \<const0>\
    );
inst_blk_mem_gen: entity work.Srom_1024x10_blk_mem_gen_v8_4_3_synth
     port map (
      addra(9 downto 0) => addra(9 downto 0),
      clka => clka,
      douta(9 downto 0) => douta(9 downto 0),
      ena => ena
    );
end STRUCTURE;
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity Srom_1024x10 is
  port (
    clka : in STD_LOGIC;
    ena : in STD_LOGIC;
    addra : in STD_LOGIC_VECTOR ( 9 downto 0 );
    douta : out STD_LOGIC_VECTOR ( 9 downto 0 )
  );
  attribute NotValidForBitStream : boolean;
  attribute NotValidForBitStream of Srom_1024x10 : entity is true;
  attribute CHECK_LICENSE_TYPE : string;
  attribute CHECK_LICENSE_TYPE of Srom_1024x10 : entity is "Srom_1024x10,blk_mem_gen_v8_4_3,{}";
  attribute downgradeipidentifiedwarnings : string;
  attribute downgradeipidentifiedwarnings of Srom_1024x10 : entity is "yes";
  attribute x_core_info : string;
  attribute x_core_info of Srom_1024x10 : entity is "blk_mem_gen_v8_4_3,Vivado 2019.1";
end Srom_1024x10;

architecture STRUCTURE of Srom_1024x10 is
  signal NLW_U0_dbiterr_UNCONNECTED : STD_LOGIC;
  signal NLW_U0_rsta_busy_UNCONNECTED : STD_LOGIC;
  signal NLW_U0_rstb_busy_UNCONNECTED : STD_LOGIC;
  signal NLW_U0_s_axi_arready_UNCONNECTED : STD_LOGIC;
  signal NLW_U0_s_axi_awready_UNCONNECTED : STD_LOGIC;
  signal NLW_U0_s_axi_bvalid_UNCONNECTED : STD_LOGIC;
  signal NLW_U0_s_axi_dbiterr_UNCONNECTED : STD_LOGIC;
  signal NLW_U0_s_axi_rlast_UNCONNECTED : STD_LOGIC;
  signal NLW_U0_s_axi_rvalid_UNCONNECTED : STD_LOGIC;
  signal NLW_U0_s_axi_sbiterr_UNCONNECTED : STD_LOGIC;
  signal NLW_U0_s_axi_wready_UNCONNECTED : STD_LOGIC;
  signal NLW_U0_sbiterr_UNCONNECTED : STD_LOGIC;
  signal NLW_U0_doutb_UNCONNECTED : STD_LOGIC_VECTOR ( 9 downto 0 );
  signal NLW_U0_rdaddrecc_UNCONNECTED : STD_LOGIC_VECTOR ( 9 downto 0 );
  signal NLW_U0_s_axi_bid_UNCONNECTED : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal NLW_U0_s_axi_bresp_UNCONNECTED : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal NLW_U0_s_axi_rdaddrecc_UNCONNECTED : STD_LOGIC_VECTOR ( 9 downto 0 );
  signal NLW_U0_s_axi_rdata_UNCONNECTED : STD_LOGIC_VECTOR ( 9 downto 0 );
  signal NLW_U0_s_axi_rid_UNCONNECTED : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal NLW_U0_s_axi_rresp_UNCONNECTED : STD_LOGIC_VECTOR ( 1 downto 0 );
  attribute C_ADDRA_WIDTH : integer;
  attribute C_ADDRA_WIDTH of U0 : label is 10;
  attribute C_ADDRB_WIDTH : integer;
  attribute C_ADDRB_WIDTH of U0 : label is 10;
  attribute C_ALGORITHM : integer;
  attribute C_ALGORITHM of U0 : label is 1;
  attribute C_AXI_ID_WIDTH : integer;
  attribute C_AXI_ID_WIDTH of U0 : label is 4;
  attribute C_AXI_SLAVE_TYPE : integer;
  attribute C_AXI_SLAVE_TYPE of U0 : label is 0;
  attribute C_AXI_TYPE : integer;
  attribute C_AXI_TYPE of U0 : label is 1;
  attribute C_BYTE_SIZE : integer;
  attribute C_BYTE_SIZE of U0 : label is 9;
  attribute C_COMMON_CLK : integer;
  attribute C_COMMON_CLK of U0 : label is 0;
  attribute C_COUNT_18K_BRAM : string;
  attribute C_COUNT_18K_BRAM of U0 : label is "1";
  attribute C_COUNT_36K_BRAM : string;
  attribute C_COUNT_36K_BRAM of U0 : label is "0";
  attribute C_CTRL_ECC_ALGO : string;
  attribute C_CTRL_ECC_ALGO of U0 : label is "NONE";
  attribute C_DEFAULT_DATA : string;
  attribute C_DEFAULT_DATA of U0 : label is "0";
  attribute C_DISABLE_WARN_BHV_COLL : integer;
  attribute C_DISABLE_WARN_BHV_COLL of U0 : label is 0;
  attribute C_DISABLE_WARN_BHV_RANGE : integer;
  attribute C_DISABLE_WARN_BHV_RANGE of U0 : label is 0;
  attribute C_ELABORATION_DIR : string;
  attribute C_ELABORATION_DIR of U0 : label is "./";
  attribute C_ENABLE_32BIT_ADDRESS : integer;
  attribute C_ENABLE_32BIT_ADDRESS of U0 : label is 0;
  attribute C_EN_DEEPSLEEP_PIN : integer;
  attribute C_EN_DEEPSLEEP_PIN of U0 : label is 0;
  attribute C_EN_ECC_PIPE : integer;
  attribute C_EN_ECC_PIPE of U0 : label is 0;
  attribute C_EN_RDADDRA_CHG : integer;
  attribute C_EN_RDADDRA_CHG of U0 : label is 0;
  attribute C_EN_RDADDRB_CHG : integer;
  attribute C_EN_RDADDRB_CHG of U0 : label is 0;
  attribute C_EN_SAFETY_CKT : integer;
  attribute C_EN_SAFETY_CKT of U0 : label is 0;
  attribute C_EN_SHUTDOWN_PIN : integer;
  attribute C_EN_SHUTDOWN_PIN of U0 : label is 0;
  attribute C_EN_SLEEP_PIN : integer;
  attribute C_EN_SLEEP_PIN of U0 : label is 0;
  attribute C_EST_POWER_SUMMARY : string;
  attribute C_EST_POWER_SUMMARY of U0 : label is "Estimated Power for IP     :     1.229999 mW";
  attribute C_FAMILY : string;
  attribute C_FAMILY of U0 : label is "artix7";
  attribute C_HAS_AXI_ID : integer;
  attribute C_HAS_AXI_ID of U0 : label is 0;
  attribute C_HAS_ENA : integer;
  attribute C_HAS_ENA of U0 : label is 1;
  attribute C_HAS_ENB : integer;
  attribute C_HAS_ENB of U0 : label is 0;
  attribute C_HAS_INJECTERR : integer;
  attribute C_HAS_INJECTERR of U0 : label is 0;
  attribute C_HAS_MEM_OUTPUT_REGS_A : integer;
  attribute C_HAS_MEM_OUTPUT_REGS_A of U0 : label is 1;
  attribute C_HAS_MEM_OUTPUT_REGS_B : integer;
  attribute C_HAS_MEM_OUTPUT_REGS_B of U0 : label is 0;
  attribute C_HAS_MUX_OUTPUT_REGS_A : integer;
  attribute C_HAS_MUX_OUTPUT_REGS_A of U0 : label is 0;
  attribute C_HAS_MUX_OUTPUT_REGS_B : integer;
  attribute C_HAS_MUX_OUTPUT_REGS_B of U0 : label is 0;
  attribute C_HAS_REGCEA : integer;
  attribute C_HAS_REGCEA of U0 : label is 0;
  attribute C_HAS_REGCEB : integer;
  attribute C_HAS_REGCEB of U0 : label is 0;
  attribute C_HAS_RSTA : integer;
  attribute C_HAS_RSTA of U0 : label is 0;
  attribute C_HAS_RSTB : integer;
  attribute C_HAS_RSTB of U0 : label is 0;
  attribute C_HAS_SOFTECC_INPUT_REGS_A : integer;
  attribute C_HAS_SOFTECC_INPUT_REGS_A of U0 : label is 0;
  attribute C_HAS_SOFTECC_OUTPUT_REGS_B : integer;
  attribute C_HAS_SOFTECC_OUTPUT_REGS_B of U0 : label is 0;
  attribute C_INITA_VAL : string;
  attribute C_INITA_VAL of U0 : label is "0";
  attribute C_INITB_VAL : string;
  attribute C_INITB_VAL of U0 : label is "0";
  attribute C_INIT_FILE : string;
  attribute C_INIT_FILE of U0 : label is "Srom_1024x10.mem";
  attribute C_INIT_FILE_NAME : string;
  attribute C_INIT_FILE_NAME of U0 : label is "Srom_1024x10.mif";
  attribute C_INTERFACE_TYPE : integer;
  attribute C_INTERFACE_TYPE of U0 : label is 0;
  attribute C_LOAD_INIT_FILE : integer;
  attribute C_LOAD_INIT_FILE of U0 : label is 1;
  attribute C_MEM_TYPE : integer;
  attribute C_MEM_TYPE of U0 : label is 3;
  attribute C_MUX_PIPELINE_STAGES : integer;
  attribute C_MUX_PIPELINE_STAGES of U0 : label is 0;
  attribute C_PRIM_TYPE : integer;
  attribute C_PRIM_TYPE of U0 : label is 1;
  attribute C_READ_DEPTH_A : integer;
  attribute C_READ_DEPTH_A of U0 : label is 1024;
  attribute C_READ_DEPTH_B : integer;
  attribute C_READ_DEPTH_B of U0 : label is 1024;
  attribute C_READ_LATENCY_A : integer;
  attribute C_READ_LATENCY_A of U0 : label is 1;
  attribute C_READ_LATENCY_B : integer;
  attribute C_READ_LATENCY_B of U0 : label is 1;
  attribute C_READ_WIDTH_A : integer;
  attribute C_READ_WIDTH_A of U0 : label is 10;
  attribute C_READ_WIDTH_B : integer;
  attribute C_READ_WIDTH_B of U0 : label is 10;
  attribute C_RSTRAM_A : integer;
  attribute C_RSTRAM_A of U0 : label is 0;
  attribute C_RSTRAM_B : integer;
  attribute C_RSTRAM_B of U0 : label is 0;
  attribute C_RST_PRIORITY_A : string;
  attribute C_RST_PRIORITY_A of U0 : label is "CE";
  attribute C_RST_PRIORITY_B : string;
  attribute C_RST_PRIORITY_B of U0 : label is "CE";
  attribute C_SIM_COLLISION_CHECK : string;
  attribute C_SIM_COLLISION_CHECK of U0 : label is "ALL";
  attribute C_USE_BRAM_BLOCK : integer;
  attribute C_USE_BRAM_BLOCK of U0 : label is 0;
  attribute C_USE_BYTE_WEA : integer;
  attribute C_USE_BYTE_WEA of U0 : label is 0;
  attribute C_USE_BYTE_WEB : integer;
  attribute C_USE_BYTE_WEB of U0 : label is 0;
  attribute C_USE_DEFAULT_DATA : integer;
  attribute C_USE_DEFAULT_DATA of U0 : label is 0;
  attribute C_USE_ECC : integer;
  attribute C_USE_ECC of U0 : label is 0;
  attribute C_USE_SOFTECC : integer;
  attribute C_USE_SOFTECC of U0 : label is 0;
  attribute C_USE_URAM : integer;
  attribute C_USE_URAM of U0 : label is 0;
  attribute C_WEA_WIDTH : integer;
  attribute C_WEA_WIDTH of U0 : label is 1;
  attribute C_WEB_WIDTH : integer;
  attribute C_WEB_WIDTH of U0 : label is 1;
  attribute C_WRITE_DEPTH_A : integer;
  attribute C_WRITE_DEPTH_A of U0 : label is 1024;
  attribute C_WRITE_DEPTH_B : integer;
  attribute C_WRITE_DEPTH_B of U0 : label is 1024;
  attribute C_WRITE_MODE_A : string;
  attribute C_WRITE_MODE_A of U0 : label is "WRITE_FIRST";
  attribute C_WRITE_MODE_B : string;
  attribute C_WRITE_MODE_B of U0 : label is "WRITE_FIRST";
  attribute C_WRITE_WIDTH_A : integer;
  attribute C_WRITE_WIDTH_A of U0 : label is 10;
  attribute C_WRITE_WIDTH_B : integer;
  attribute C_WRITE_WIDTH_B of U0 : label is 10;
  attribute C_XDEVICEFAMILY : string;
  attribute C_XDEVICEFAMILY of U0 : label is "artix7";
  attribute downgradeipidentifiedwarnings of U0 : label is "yes";
  attribute x_interface_info : string;
  attribute x_interface_info of clka : signal is "xilinx.com:interface:bram:1.0 BRAM_PORTA CLK";
  attribute x_interface_parameter : string;
  attribute x_interface_parameter of clka : signal is "XIL_INTERFACENAME BRAM_PORTA, MEM_SIZE 8192, MEM_WIDTH 32, MEM_ECC NONE, MASTER_TYPE OTHER, READ_LATENCY 1";
  attribute x_interface_info of ena : signal is "xilinx.com:interface:bram:1.0 BRAM_PORTA EN";
  attribute x_interface_info of addra : signal is "xilinx.com:interface:bram:1.0 BRAM_PORTA ADDR";
  attribute x_interface_info of douta : signal is "xilinx.com:interface:bram:1.0 BRAM_PORTA DOUT";
begin
U0: entity work.Srom_1024x10_blk_mem_gen_v8_4_3
     port map (
      addra(9 downto 0) => addra(9 downto 0),
      addrb(9 downto 0) => B"0000000000",
      clka => clka,
      clkb => '0',
      dbiterr => NLW_U0_dbiterr_UNCONNECTED,
      deepsleep => '0',
      dina(9 downto 0) => B"0000000000",
      dinb(9 downto 0) => B"0000000000",
      douta(9 downto 0) => douta(9 downto 0),
      doutb(9 downto 0) => NLW_U0_doutb_UNCONNECTED(9 downto 0),
      eccpipece => '0',
      ena => ena,
      enb => '0',
      injectdbiterr => '0',
      injectsbiterr => '0',
      rdaddrecc(9 downto 0) => NLW_U0_rdaddrecc_UNCONNECTED(9 downto 0),
      regcea => '0',
      regceb => '0',
      rsta => '0',
      rsta_busy => NLW_U0_rsta_busy_UNCONNECTED,
      rstb => '0',
      rstb_busy => NLW_U0_rstb_busy_UNCONNECTED,
      s_aclk => '0',
      s_aresetn => '0',
      s_axi_araddr(31 downto 0) => B"00000000000000000000000000000000",
      s_axi_arburst(1 downto 0) => B"00",
      s_axi_arid(3 downto 0) => B"0000",
      s_axi_arlen(7 downto 0) => B"00000000",
      s_axi_arready => NLW_U0_s_axi_arready_UNCONNECTED,
      s_axi_arsize(2 downto 0) => B"000",
      s_axi_arvalid => '0',
      s_axi_awaddr(31 downto 0) => B"00000000000000000000000000000000",
      s_axi_awburst(1 downto 0) => B"00",
      s_axi_awid(3 downto 0) => B"0000",
      s_axi_awlen(7 downto 0) => B"00000000",
      s_axi_awready => NLW_U0_s_axi_awready_UNCONNECTED,
      s_axi_awsize(2 downto 0) => B"000",
      s_axi_awvalid => '0',
      s_axi_bid(3 downto 0) => NLW_U0_s_axi_bid_UNCONNECTED(3 downto 0),
      s_axi_bready => '0',
      s_axi_bresp(1 downto 0) => NLW_U0_s_axi_bresp_UNCONNECTED(1 downto 0),
      s_axi_bvalid => NLW_U0_s_axi_bvalid_UNCONNECTED,
      s_axi_dbiterr => NLW_U0_s_axi_dbiterr_UNCONNECTED,
      s_axi_injectdbiterr => '0',
      s_axi_injectsbiterr => '0',
      s_axi_rdaddrecc(9 downto 0) => NLW_U0_s_axi_rdaddrecc_UNCONNECTED(9 downto 0),
      s_axi_rdata(9 downto 0) => NLW_U0_s_axi_rdata_UNCONNECTED(9 downto 0),
      s_axi_rid(3 downto 0) => NLW_U0_s_axi_rid_UNCONNECTED(3 downto 0),
      s_axi_rlast => NLW_U0_s_axi_rlast_UNCONNECTED,
      s_axi_rready => '0',
      s_axi_rresp(1 downto 0) => NLW_U0_s_axi_rresp_UNCONNECTED(1 downto 0),
      s_axi_rvalid => NLW_U0_s_axi_rvalid_UNCONNECTED,
      s_axi_sbiterr => NLW_U0_s_axi_sbiterr_UNCONNECTED,
      s_axi_wdata(9 downto 0) => B"0000000000",
      s_axi_wlast => '0',
      s_axi_wready => NLW_U0_s_axi_wready_UNCONNECTED,
      s_axi_wstrb(0) => '0',
      s_axi_wvalid => '0',
      sbiterr => NLW_U0_sbiterr_UNCONNECTED,
      shutdown => '0',
      sleep => '0',
      wea(0) => '0',
      web(0) => '0'
    );
end STRUCTURE;
