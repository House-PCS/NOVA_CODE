//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   DdrCtrlCore.v
//Original Author   :   lvyb
//Creation Date     :   
/*Description       :

*/
//------------------------------------------------------------------------------
//Version           :   Rev 0.1
//Modifier          :   
//Modify Date       :   
/*Modify Record     :   

*/
//------------------------------------------------------------------------------
//Version           :   Rev 0.2
//Modifier          :   
//Modify Date       :   
/*Modify Record     :   

*/  
//------------------------------------------------------------------------------

`timescale 1ps/100fs
module DdrCtrlCore
(
    input                           iDdrClk                                 ,   // system clock input  100Mhz
    input                           iDdrRefClk                              ,   // reference clock input 200Mhz
    input                           iMigRst                                 ,
    input       [11:0]              iFpgaTemp                               ,
    output                          oDdrInitDone                            ,
    output                          oDdrWrOpDone                            ,
    output                          oDdrLogicClk                            ,
    //ddr3 interface 
    output      [12:0]              ddr3_addr                               ,   // output
    output      [2:0]               ddr3_ba                                 ,   // output
    output                          ddr3_cas_n                              ,   // output
    output                          ddr3_ck_n                               ,   // output
    output                          ddr3_ck_p                               ,   // output
    output                          ddr3_cke                                ,   // output
  //  output                          ddr3_cs_n                               ,   // output
    output                          ddr3_ras_n                              ,   // output
    output                          ddr3_reset_n                            ,   // output
    output                          ddr3_we_n                               ,   // output
    inout       [15:0]              ddr3_dq                                 ,   // inout 
    inout       [1:0]               ddr3_dqs_n                              ,   // inout 
    inout       [1:0]               ddr3_dqs_p                              ,   // inout 
    output      [1:0]               ddr3_dm                                 ,   // output
    output                          ddr3_odt                                ,   // output
    //App Interface
    input       [27:0]              wr0_addr                                 ,
    input       [7:0]               wr0_buf_base                             ,   //Ram基地址
    input                           wr0_buf_sel                              ,   //Ram基地址
    input       [127:0]             wr0_buf_data                             ,
    input       [15:0]              wr0_buf_mask                             ,
    input                           wr0_burst_begin                          ,
    output                          wr0_burst_rdy                            ,
    input                           wr0_clk                                  ,   //写命令时钟
    input       [8:0]               wr0_len                                  ,
    output      [8:0]               wr0_buf_addr                             ,
    output                          wr0_buf_addr_en                          ,

    input       [27:0]              wr1_addr                                 ,
    input       [7:0]               wr1_buf_base                             ,   //Ram基地址
    input                           wr1_buf_sel                              ,   //Ram基地址
    input       [127:0]             wr1_buf_data                             ,
    input       [15:0]              wr1_buf_mask                             ,
    input                           wr1_burst_begin                          ,
    output                          wr1_burst_rdy                            ,
    input                           wr1_clk                                  ,   //写命令时钟
    input       [8:0]               wr1_len                                  ,
    output      [8:0]               wr1_buf_addr                             ,
    output                          wr1_buf_addr_en                          ,

    input       [27:0]              rd0_addr                                 ,
    input                           rd0_burst_begin                          ,
    output                          rd0_burst_rdy                            ,
    input                           rd0_clk                                  ,   //读命令时钟
    input       [8:0]               rd0_len                                  ,
    input       [13:0]              rd0_opt                                  ,
    input                           rd0_buf_sel                              ,
    input       [7:0]               rd0_buf_base                             ,
    output      [127:0]             rd0_data                                 ,
    output      [8:0]               rd0_data_addr                            ,
    output                          rd0_data_en                              ,
    output      [13:0]              rd0_data_opt                             

    // input       [27:0]              rd1_addr                                 ,
    // input                           rd1_burst_begin                          ,
    // output                          rd1_burst_rdy                            ,
    // input                           rd1_clk                                  ,   //读命令时钟
    // input       [8:0]               rd1_len                                  ,
    // input       [13:0]              rd1_opt                                  ,
    // input                           rd1_buf_sel                              ,
    // input       [7:0]               rd1_buf_base                             ,
    // output      [127:0]             rd1_data                                 ,
    // output      [8:0]               rd1_data_addr                            ,
    // output                          rd1_data_en                              ,
    // output      [13:0]              rd1_data_opt                              
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    wire        [27:0]              app_addr                                ;
    wire        [2:0]               app_cmd                                 ;
    wire                            app_en                                  ;
    wire        [127:0]             app_wdf_data                            ;
    wire                            app_wdf_end                             ;
    wire        [15:0]              app_wdf_mask                            ;
    wire                            app_wdf_wren                            ; 
    wire        [127:0]             app_rd_data                             ;
    wire                            app_rd_data_end                         ;
    wire                            app_rd_data_valid                       ;
    wire                            app_rdy                                 ;
    wire                            app_wdf_rdy                             ;

    wire                            wDdrLogicClk                            ;
    wire                            wDdrLogicClkRst                         ;
    wire                            wDdrInitDone                            ;
    wire                            wDdrArbitRst                            ;

//******************************************************************************

//******************************************************************************
//

assign wDdrArbitRst     = (wDdrLogicClkRst || (!wDdrInitDone));
assign oDdrInitDone     = wDdrInitDone;
assign oDdrLogicClk     = wDdrLogicClk;
assign oDdrWrOpDone     = app_wdf_end;

//******************************************************************************

//******************************************************************************
//

MigCore U_MigCore
(
    // Memory interface ports
    .ddr3_addr                      (ddr3_addr[12:0]                        ),  //o
    .ddr3_ba                        (ddr3_ba[2:0]                           ),  //o
    .ddr3_cas_n                     (ddr3_cas_n                             ),  //o
    .ddr3_ck_n                      (ddr3_ck_n                              ),  //o
    .ddr3_ck_p                      (ddr3_ck_p                              ),  //o
    .ddr3_cke                       (ddr3_cke                               ),  //o
   // .ddr3_cs_n                      (ddr3_cs_n                              ),  //o
    .ddr3_ras_n                     (ddr3_ras_n                             ),  //o
    .ddr3_reset_n                   (ddr3_reset_n                           ),  //o
    .ddr3_we_n                      (ddr3_we_n                              ),  //o
    .ddr3_dq                        (ddr3_dq[15:0]                          ),  //io
    .ddr3_dqs_n                     (ddr3_dqs_n[1:0]                        ),  //io
    .ddr3_dqs_p                     (ddr3_dqs_p[1:0]                        ),  //io
    .ddr3_dm                        (ddr3_dm[1:0]                           ),  //o
    .ddr3_odt                       (ddr3_odt                               ),  //o
    // System Clock Ports Reference Clock Ports
    .sys_clk_i                      (iDdrClk                                ),  //i
    .clk_ref_i                      (iDdrRefClk                             ),  //i
    //App Port
    .app_addr                       (app_addr[27:0]                         ),  //i
    .app_cmd                        (app_cmd[2:0]                           ),  //i
    .app_en                         (app_en                                 ),  //i
    .app_wdf_data                   (app_wdf_data[127:0]                    ),  //i
    .app_wdf_end                    (app_wdf_end                            ),  //i
    .app_wdf_mask                   (app_wdf_mask[15:0]                     ),  //i
    .app_wdf_wren                   (app_wdf_wren                           ),  //i
    .app_rd_data                    (app_rd_data[127:0]                     ),  //o
    .app_rd_data_end                (app_rd_data_end                        ),  //o
    .app_rd_data_valid              (app_rd_data_valid                      ),  //o
    .app_rdy                        (app_rdy                                ),  //o
    .app_wdf_rdy                    (app_wdf_rdy                            ),  //o

    .app_sr_req                     (1'b0                                   ),  //i
    .app_ref_req                    (1'b0                                   ),  //i
    .app_zq_req                     (1'b0                                   ),  //i
    .app_sr_active                  (                                       ),  //o
    .app_ref_ack                    (                                       ),  //o
    .app_zq_ack                     (                                       ),  //o

    // Application interface ports
    .ui_clk                         (wDdrLogicClk                           ),  //o
    .ui_clk_sync_rst                (wDdrLogicClkRst                        ),  //o
    .init_calib_complete            (wDdrInitDone                           ),  //o
    //Temp & Rst
    .device_temp_i                  (iFpgaTemp[11:0]                        ),  //i
    .device_temp                    (                                       ),  //o
    .sys_rst                        (iMigRst                                )   //i Active High
);

//******************************************************************************
//******************************************************************************
//

DdrArbitCore U_DdrArbitCore
(
    .rst                            (wDdrArbitRst                           ),  //i
    .ddr_clk                        (wDdrLogicClk                           ),  //i

    .app_rd_data                    (app_rd_data[127:0]                     ),  //i
    .app_rd_data_valid              (app_rd_data_valid                      ),  //i
    .app_rdy                        (app_rdy                                ),  //i
    .app_wdf_rdy                    (app_wdf_rdy                            ),  //i
    .app_addr                       (app_addr[27:0]                         ),  //o
    .app_cmd                        (app_cmd[2:0]                           ),  //o
    .app_en                         (app_en                                 ),  //o
    .app_wdf_data                   (app_wdf_data[127:0]                    ),  //o
    .app_wdf_end                    (app_wdf_end                            ),  //o
    .app_wdf_mask                   (app_wdf_mask[15:0]                     ),  //o
    .app_wdf_wren                   (app_wdf_wren                           ),  //o

    .wr0_addr                           (wr0_addr                  ),//i
    .wr0_buf_base                       (wr0_buf_base              ),//i
    .wr0_buf_sel                        (wr0_buf_sel               ),//i
    .wr0_buf_data                       (wr0_buf_data              ),//i
    .wr0_buf_mask                       (wr0_buf_mask              ),//i
    .wr0_burst_begin                    (wr0_burst_begin           ),//i
    .wr0_burst_rdy                      (wr0_burst_rdy             ),
    .wr0_clk                            (wr0_clk                   ),//i
    .wr0_len                            (wr0_len                   ),//i
    .wr0_buf_addr                       (wr0_buf_addr              ),//o
    .wr0_buf_addr_en                    (wr0_buf_addr_en           ),//o

    .wr1_addr                           (wr1_addr                  ),//i
    .wr1_buf_base                       (wr1_buf_base              ),//i
    .wr1_buf_sel                        (wr1_buf_sel               ),//i
    .wr1_buf_data                       (wr1_buf_data              ),//i
    .wr1_buf_mask                       (wr1_buf_mask              ),//i
    .wr1_burst_begin                    (wr1_burst_begin           ),//i
    .wr1_burst_rdy                      (wr1_burst_rdy             ),
    .wr1_clk                            (wr1_clk                   ),//i
    .wr1_len                            (wr1_len                   ),//i
    .wr1_buf_addr                       (wr1_buf_addr              ),//o
    .wr1_buf_addr_en                    (wr1_buf_addr_en           ),//o

    .rd0_addr                           (rd0_addr                  ),//i
    .rd0_burst_begin                    (rd0_burst_begin           ),//i
    .rd0_burst_rdy                      (rd0_burst_rdy             ),
    .rd0_clk                            (rd0_clk                   ),//i
    .rd0_len                            (rd0_len                   ),//i
    .rd0_opt                            (rd0_opt                   ),//i
    .rd0_buf_sel                        (rd0_buf_sel               ),
    .rd0_buf_base                       (rd0_buf_base              ),
    .rd0_data                           (rd0_data                  ),//o
    .rd0_data_addr                      (rd0_data_addr             ),//o
    .rd0_data_en                        (rd0_data_en               ),//o
    .rd0_data_opt                       (rd0_data_opt              )//o

    // .rd1_addr                           (rd1_addr                  ),//i
    // .rd1_burst_begin                    (rd1_burst_begin           ),//i
    // .rd1_burst_rdy                      (rd1_burst_rdy             ),
    // .rd1_clk                            (rd1_clk                   ),//i
    // .rd1_len                            (rd1_len                   ),//i
    // .rd1_opt                            (rd1_opt                   ),//i
    // .rd1_buf_sel                        (rd1_buf_sel               ),
    // .rd1_buf_base                       (rd1_buf_base              ),
    // .rd1_data                           (rd1_data                  ),//o
    // .rd1_data_addr                      (rd1_data_addr             ),//o
    // .rd1_data_en                        (rd1_data_en               ),//o
    // .rd1_data_opt                       (rd1_data_opt              ) //o
);


//******************************************************************************

endmodule
