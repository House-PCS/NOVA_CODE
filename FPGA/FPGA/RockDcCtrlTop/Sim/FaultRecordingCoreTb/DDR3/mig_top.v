//*****************************************************************************


// (c) Copyright 2009 - 2013 Xilinx, Inc. All rights reserved.


//


// This file contains confidential and proprietary information


// of Xilinx, Inc. and is protected under U.S. and


// international copyright and other intellectual property


// laws.


//


// DISCLAIMER


// This disclaimer is not a license and does not grant any


// rights to the materials distributed herewith. Except as


// otherwise provided in a valid license issued to you by


// Xilinx, and to the maximum extent permitted by applicable


// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND


// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES


// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING


// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-


// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and


// (2) Xilinx shall not be liable (whether in contract or tort,


// including negligence, or under any other theory of


// liability) for any loss or damage of any kind or nature


// related to, arising under or in connection with these


// materials, including for any direct, or any indirect,


// special, incidental, or consequential loss or damage


// (including loss of data, profits, goodwill, or any type of


// loss or damage suffered as a result of any action brought


// by a third party) even if such damage or loss was


// reasonably foreseeable or Xilinx had been advised of the


// possibility of the same.


//


// CRITICAL APPLICATIONS


// Xilinx products are not designed or intended to be fail-


// safe, or for use in any application requiring fail-safe


// performance, such as life-support or safety devices or


// systems, Class III medical devices, nuclear facilities,


// applications related to the deployment of airbags, or any


// other applications that could lead to death, personal


// injury, or severe property or environmental damage


// (individually and collectively, "Critical


// Applications"). Customer assumes the sole risk and


// liability of any use of Xilinx products in Critical


// Applications, subject only to applicable laws and


// regulations governing limitations on product liability.


//


// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS


// PART OF THIS FILE AT ALL TIMES.


//


//*****************************************************************************


//   ____  ____


//  /   /\/   /


// /___/  \  /    Vendor             : Xilinx


// \   \   \/     Version            : 4.2


//  \   \         Application        : MIG


//  /   /         Filename           : example_top.v


// /___/   /\     Date Last Modified : $Date: 2011/06/02 08:35:03 $


// \   \  /  \    Date Created       : Tue Sept 21 2010


//  \___\/\___\


//


// Device           : 7 Series


// Design Name      : DDR3 SDRAM


// Purpose          :


//   Top-level  module. This module serves as an example,


//   and allows the user to synthesize a self-contained design,


//   which they can be used to test their hardware.


//   In addition to the memory controller, the module instantiates:


//     1. Synthesizable testbench - used to model user's backend logic


//        and generate different traffic patterns


// Reference        :


// Revision History :


//*****************************************************************************





//`define SKIP_CALIB


`timescale 1ps/1ps





module mig_top #


  (





   //***************************************************************************


   // Traffic Gen related parameters


   //***************************************************************************


   parameter PORT_MODE             = "BI_MODE",


   parameter DATA_MODE             = 4'b0010,


   parameter TST_MEM_INSTR_MODE    = "R_W_INSTR_MODE",


   parameter EYE_TEST              = "FALSE",


                                     // set EYE_TEST = "TRUE" to probe memory


                                     // signals. Traffic Generator will only


                                     // write to one single location and no


                                     // read transactions will be generated.


   parameter DATA_PATTERN          = "DGEN_ALL",


                                      // For small devices, choose one only.


                                      // For large device, choose "DGEN_ALL"


                                      // "DGEN_HAMMER", "DGEN_WALKING1",


                                      // "DGEN_WALKING0","DGEN_ADDR","


                                      // "DGEN_NEIGHBOR","DGEN_PRBS","DGEN_ALL"


   parameter CMD_PATTERN           = "CGEN_ALL",


                                      // "CGEN_PRBS","CGEN_FIXED","CGEN_BRAM",


                                      // "CGEN_SEQUENTIAL", "CGEN_ALL"


   parameter CMD_WDT               = 'h3FF,


   parameter WR_WDT                = 'h1FFF,


   parameter RD_WDT                = 'h3FF,


   parameter SEL_VICTIM_LINE       = 0,


   parameter BEGIN_ADDRESS         = 32'h00000000,


   parameter END_ADDRESS           = 32'h00ffffff,


   parameter PRBS_EADDR_MASK_POS   = 32'hff000000,





   //***************************************************************************


   // The following parameters refer to width of various ports


   //***************************************************************************


   parameter CK_WIDTH              = 1,


                                     // # of CK/CK# outputs to memory.


   parameter nCS_PER_RANK          = 1,


                                     // # of unique CS outputs per rank for phy


   parameter CKE_WIDTH             = 1,


                                     // # of CKE outputs to memory.


   parameter DM_WIDTH              = 2,


                                     // # of DM (data mask)


   parameter ODT_WIDTH             = 1,


                                     // # of ODT outputs to memory.


   parameter BANK_WIDTH            = 3,


                                     // # of memory Bank Address bits.


   parameter COL_WIDTH             = 10,


                                     // # of memory Column Address bits.


   parameter CS_WIDTH              = 1,


                                     // # of unique CS outputs to memory.


   parameter DQ_WIDTH              = 16,


                                     // # of DQ (data)


   parameter DQS_WIDTH             = 2,


   parameter DQS_CNT_WIDTH         = 1,


                                     // = ceil(log2(DQS_WIDTH))


   parameter DRAM_WIDTH            = 8,


                                     // # of DQ per DQS


   parameter ECC                   = "OFF",


   parameter ECC_TEST              = "OFF",


   //parameter nBANK_MACHS           = 4,


   parameter nBANK_MACHS           = 4,


   parameter RANKS                 = 1,


                                     // # of Ranks.


   parameter ROW_WIDTH             = 13,


                                     // # of memory Row Address bits.


   parameter ADDR_WIDTH            = 27,


                                     // # = RANK_WIDTH + BANK_WIDTH


                                     //     + ROW_WIDTH + COL_WIDTH;


                                     // Chip Select is always tied to low for


                                     // single rank devices





   //***************************************************************************


   // The following parameters are mode register settings


   //***************************************************************************


   parameter BURST_MODE            = "8",


                                     // DDR3 SDRAM:


                                     // Burst Length (Mode Register 0).


                                     // # = "8", "4", "OTF".


                                     // DDR2 SDRAM:


                                     // Burst Length (Mode Register).


                                     // # = "8", "4".





   


   //***************************************************************************


   // The following parameters are multiplier and divisor factors for PLLE2.


   // Based on the selected design frequency these parameters vary.


   //***************************************************************************


   parameter CLKIN_PERIOD          = 10000,


                                     // Input Clock Period


   parameter CLKFBOUT_MULT         = 8,


                                     // write PLL VCO multiplier


   parameter DIVCLK_DIVIDE         = 1,


                                     // write PLL VCO divisor


   parameter CLKOUT0_PHASE         = 0.0,


                                     // Phase for PLL output clock (CLKOUT0)


   parameter CLKOUT0_DIVIDE        = 1,


                                     // VCO output divisor for PLL output clock (CLKOUT0)


   parameter CLKOUT1_DIVIDE        = 2,


                                     // VCO output divisor for PLL output clock (CLKOUT1)


   parameter CLKOUT2_DIVIDE        = 32,


                                     // VCO output divisor for PLL output clock (CLKOUT2)


   parameter CLKOUT3_DIVIDE        = 8,


                                     // VCO output divisor for PLL output clock (CLKOUT3)


   parameter MMCM_VCO              = 800,


                                     // Max Freq (MHz) of MMCM VCO


   parameter MMCM_MULT_F           = 8,


                                     // write MMCM VCO multiplier


   parameter MMCM_DIVCLK_DIVIDE    = 1,


                                     // write MMCM VCO divisor





   //***************************************************************************


   // Simulation parameters


   //***************************************************************************


   parameter SIMULATION            = "FALSE",


                                     // Should be TRUE during design simulations and


                                     // FALSE during implementations





   //***************************************************************************


   // IODELAY and PHY related parameters


   //***************************************************************************


   parameter TCQ                   = 100,


   


   parameter DRAM_TYPE             = "DDR3",





   


   //***************************************************************************


   // System clock frequency parameters


   //***************************************************************************


   parameter nCK_PER_CLK           = 4,


                                     // # of memory CKs per fabric CLK





   





   //***************************************************************************


   // Debug parameters


   //***************************************************************************


   parameter DEBUG_PORT            = "OFF",


                                     // # = "ON" Enable debug signals/controls.


                                     //   = "OFF" Disable debug signals/controls.


      


   parameter RST_ACT_LOW           = 0


                                     // =1 for active low reset,


                                     // =0 for active high.


   )


  (





   // Inouts


   inout [15:0]                         ddr3_dq,


   inout [1:0]                        ddr3_dqs_n,


   inout [1:0]                        ddr3_dqs_p,





   // Outputs


   output [12:0]                       ddr3_addr,


   output [2:0]                      ddr3_ba,


   output                                       ddr3_ras_n,


   output                                       ddr3_cas_n,


   output                                       ddr3_we_n,


   output                                       ddr3_reset_n,


   output [0:0]                        ddr3_ck_p,


   output [0:0]                        ddr3_ck_n,


   output [0:0]                       ddr3_cke,


   


   


   output [1:0]                        ddr3_dm,


   


   output [0:0]                       ddr3_odt,


   





   // Inputs


   


   // Single-ended system clock
   input                                        sys_clk_i,
   // Single-ended iodelayctrl clk (reference clock)
   input                                        clk_ref_i,
   output                                       init_calib_complete,
   input  [11:0]                                device_temp_i,


                      // The 12 MSB bits of the temperature sensor transfer


                      // function need to be connected to this port. This port


                      // will be synchronized w.r.t. to fabric clock internally.

   // System reset - Default polarity of sys_rst pin is Active Low.


   // System reset polarity will change based on the option 


   // selected in GUI.


input                                        sys_rst,

//mig
  input wire [ADDR_WIDTH-1:0]                 app_addr,

  input wire [2:0]                            app_cmd,

  input wire                                  app_en,

  output wire                                  app_rdy,

  output wire [APP_DATA_WIDTH-1:0]             app_rd_data,

  output wire                                  app_rd_data_end,

  output wire                                  app_rd_data_valid,

  input wire [APP_DATA_WIDTH-1:0]             app_wdf_data,

  input wire                                  app_wdf_end,

  input wire [APP_MASK_WIDTH-1:0]             app_wdf_mask,

  output wire                                  app_wdf_rdy,

  input wire                                  app_wdf_wren,

  output wire                                  app_data_clk,
  output wire                                  app_data_clk_rst

   );





function integer clogb2 (input integer size);


    begin


      size = size - 1;


      for (clogb2=1; size>1; clogb2=clogb2+1)


        size = size >> 1;


    end


  endfunction // clogb2





  function integer STR_TO_INT;


    input [7:0] in;


    begin


      if(in == "8")


        STR_TO_INT = 8;


      else if(in == "4")


        STR_TO_INT = 4;


      else


        STR_TO_INT = 0;


    end


  endfunction








  localparam DATA_WIDTH            = 16;


  localparam RANK_WIDTH = clogb2(RANKS);


  localparam PAYLOAD_WIDTH         = (ECC_TEST == "OFF") ? DATA_WIDTH : DQ_WIDTH;


  localparam BURST_LENGTH          = STR_TO_INT(BURST_MODE);


  localparam APP_DATA_WIDTH        = 2 * nCK_PER_CLK * PAYLOAD_WIDTH;


  localparam APP_MASK_WIDTH        = APP_DATA_WIDTH / 8;





  //***************************************************************************


  // Traffic Gen related parameters (derived)


  //***************************************************************************


  localparam  TG_ADDR_WIDTH = ((CS_WIDTH == 1) ? 0 : RANK_WIDTH)


                                 + BANK_WIDTH + ROW_WIDTH + COL_WIDTH;


  localparam MASK_SIZE             = DATA_WIDTH/8;


      





  // Wire declarations

  




      



//***************************************************************************




















      


// Start of User Design top instance


//***************************************************************************


// The User design is instantiated below. The memory interface ports are


// connected to the top-level and the application interface ports are


// connected to the traffic generator module. This provides a reference


// for connecting the memory controller to system.


//***************************************************************************





  MigCore u_MigCore


      (
// Memory interface ports


       .ddr3_addr                      (ddr3_addr),


       .ddr3_ba                        (ddr3_ba),


       .ddr3_cas_n                     (ddr3_cas_n),


       .ddr3_ck_n                      (ddr3_ck_n),


       .ddr3_ck_p                      (ddr3_ck_p),


       .ddr3_cke                       (ddr3_cke),


       .ddr3_ras_n                     (ddr3_ras_n),


       .ddr3_we_n                      (ddr3_we_n),


       .ddr3_dq                        (ddr3_dq),


       .ddr3_dqs_n                     (ddr3_dqs_n),


       .ddr3_dqs_p                     (ddr3_dqs_p),


       .ddr3_reset_n                   (ddr3_reset_n),


       .init_calib_complete            (init_calib_complete),

       .ddr3_dm                        (ddr3_dm),


       .ddr3_odt                       (ddr3_odt),


// Application interface ports

       .app_addr                       (app_addr),


       .app_cmd                        (app_cmd),


       .app_en                         (app_en),


       .app_wdf_data                   (app_wdf_data),


       .app_wdf_end                    (app_wdf_end),


       .app_wdf_wren                   (app_wdf_wren),


       .app_rd_data                    (app_rd_data),


       .app_rd_data_end                (app_rd_data_end),


       .app_rd_data_valid              (app_rd_data_valid),


       .app_rdy                        (app_rdy),


       .app_wdf_rdy                    (app_wdf_rdy),


       .app_sr_req                     (1'b0),


       .app_ref_req                    (1'b0),


       .app_zq_req                     (1'b0),


       .app_sr_active                  (),


       .app_ref_ack                    (),


       .app_zq_ack                     (),


       .ui_clk                         (app_data_clk),


       .ui_clk_sync_rst                (app_data_clk_rst),

       .app_wdf_mask                   (app_wdf_mask),


      

// System Clock Ports


       .sys_clk_i                       (sys_clk_i),


// Reference Clock Ports


       .clk_ref_i                      (clk_ref_i),


       .device_temp_i                  (device_temp_i),


       .device_temp                     (),

       .sys_rst                        (sys_rst)

       );


// End of User Design top instance

//***************************************************************************




endmodule







