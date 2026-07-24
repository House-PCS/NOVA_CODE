//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   DdrArbitCore.v
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

`timescale 1ns/1ns
module DdrArbitCore
(
    input                           rst                                      ,
    input                           ddr_clk                                  ,
    input       [127:0]             app_rd_data                              ,
    input                           app_rd_data_valid                        ,
    input                           app_rdy                                  ,
    input                           app_wdf_rdy                              ,
    output      [27:0]              app_addr                                 ,
    output      [2:0]               app_cmd                                  ,
    output                          app_en                                   ,
    output      [127:0]             app_wdf_data                             ,
    output                          app_wdf_end                              ,
    output      [15:0]              app_wdf_mask                             ,
    output                          app_wdf_wren                             ,

    input       [27:0]              wr0_addr                                 ,
    input       [7:0]               wr0_buf_base                             ,
    input                           wr0_buf_sel                              ,
    input       [127:0]             wr0_buf_data                             ,
    input       [15:0]              wr0_buf_mask                             ,
    input                           wr0_burst_begin                          ,
    output                          wr0_burst_rdy                            ,
    input                           wr0_clk                                  ,
    input       [8:0]               wr0_len                                  ,
    output      [8:0]               wr0_buf_addr                             ,
    output                          wr0_buf_addr_en                          ,

    input       [27:0]              wr1_addr                                 ,
    input       [7:0]               wr1_buf_base                             , 
    input                           wr1_buf_sel                              ,
    input       [127:0]             wr1_buf_data                             ,
    input       [15:0]              wr1_buf_mask                             ,
    input                           wr1_burst_begin                          ,
    output                          wr1_burst_rdy                            ,
    input                           wr1_clk                                  ,
    input       [8:0]               wr1_len                                  ,
    output      [8:0]               wr1_buf_addr                             ,
    output                          wr1_buf_addr_en                          ,

    input       [27:0]              rd0_addr                                 ,
    input                           rd0_burst_begin                          ,
    output                          rd0_burst_rdy                            ,
    input                           rd0_clk                                  ,
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
    // input                           rd1_clk                                  ,
    // input       [8:0]               rd1_len                                  ,
    // input       [13:0]              rd1_opt                                  ,
    // input                           rd1_buf_sel                              ,
    // input       [7:0]               rd1_buf_base                             ,
    // output      [127:0]             rd1_data                                 ,
    // output      [8:0]               rd1_data_addr                            ,
    // output                          rd1_data_en                              ,
    // output      [13:0]              rd1_data_opt                             ,                         
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
    wire                            ddr_wr0_ack                             ;
    wire        [31:0]              ddr_wr0_addr                            ;
    wire                            ddr_wr0_clr                             ;
    wire        [2:0]               ddr_wr0_cmd                             ;
    wire                            ddr_wr0_cmd_en                          ;
    wire                            ddr_wr0_cmd_rdy                         ;
    wire        [127:0]             ddr_wr0_data                            ;
    wire        [15:0]              ddr_wr0_mask                            ;
    wire                            ddr_wr0_en                              ;
    wire                            ddr_wr0_end                             ;
    wire                            ddr_wr0_rdy                             ;
    wire                            ddr_wr0_req                             ;

    wire                            ddr_wr1_ack                             ;
    wire        [31:0]              ddr_wr1_addr                            ;
    wire                            ddr_wr1_clr                             ;
    wire        [2:0]               ddr_wr1_cmd                             ;
    wire                            ddr_wr1_cmd_en                          ;
    wire                            ddr_wr1_cmd_rdy                         ;
    wire        [127:0]             ddr_wr1_data                            ;
    wire        [15:0]              ddr_wr1_mask                            ;
    wire                            ddr_wr1_en                              ;
    wire                            ddr_wr1_end                             ;
    wire                            ddr_wr1_rdy                             ;
    wire                            ddr_wr1_req                             ;

    wire                            ddr_rd0_ack                             ;
    wire        [31:0]              ddr_rd0_addr                            ;
    wire                            ddr_rd0_clr                             ;
    wire        [2:0]               ddr_rd0_cmd                             ;
    wire                            ddr_rd0_cmd_en                          ;
    wire                            ddr_rd0_cmd_rdy                         ;
    wire        [127:0]             ddr_rd0_data                            ;
    wire        [8:0]               ddr_rd0_len                             ;
    wire                            ddr_rd0_req                             ;
    wire                            ddr_rd0_valid                           ;

    // wire                            ddr_rd1_ack                             ;
    // wire        [31:0]              ddr_rd1_addr                            ;
    // wire                            ddr_rd1_clr                             ;
    // wire        [2:0]               ddr_rd1_cmd                             ;
    // wire                            ddr_rd1_cmd_en                          ;
    // wire                            ddr_rd1_cmd_rdy                         ;
    // wire        [127:0]             ddr_rd1_data                            ;
    // wire        [8:0]               ddr_rd1_len                             ;
    // wire                            ddr_rd1_req                             ;
    // wire                            ddr_rd1_valid                           ;
//******************************************************************************

//******************************************************************************

DdrWrOp U_DdrWrCh0Op
(
    // Outputs
    .ddr_wr_req                     (ddr_wr0_req                            ),
    .ddr_wr_clr                     (ddr_wr0_clr                            ),
    .ddr_wr_cmd                     (ddr_wr0_cmd[2:0]                       ),
    .ddr_wr_cmd_en                  (ddr_wr0_cmd_en                         ),
    .ddr_wr_addr                    (ddr_wr0_addr[31:0]                     ),
    .ddr_wr_data                    (ddr_wr0_data[127:0]                    ),
    .ddr_wr_mask                    (ddr_wr0_mask[15:0]                     ),
    .ddr_wr_en                      (ddr_wr0_en                             ),
    .ddr_wr_end                     (ddr_wr0_end                            ),
    
    .wr_buf_addr                    (wr0_buf_addr[8:0]                      ),
    .wr_buf_addr_en                 (wr0_buf_addr_en                        ),
    // Inputs
    .ddr_clk                        (ddr_clk                                ),
    .rst                            (rst                                    ),
    .ddr_wr_ack                     (ddr_wr0_ack                            ),
    .ddr_wr_cmd_rdy                 (ddr_wr0_cmd_rdy                        ),
    .ddr_wr_rdy                     (ddr_wr0_rdy                            ),
    
    .wr_buf_data                    (wr0_buf_data[127:0]                    ),
    .wr_buf_mask                    (wr0_buf_mask[15:0]                     ),
    .wr_burst_begin                 (wr0_burst_begin                        ),
    .wr_burst_rdy                   (wr0_burst_rdy                          ),
    .wr_len                         (wr0_len[8:0]                           ),
    .wr_addr                        (wr0_addr[27:0]                         ),
    .wr_clk                         (wr0_clk                                ),
    .wr_buf_sel                     (wr0_buf_sel                            ),
    .wr_buf_base                    (wr0_buf_base[7:0]                      )
);


DdrWrOp U_DdrWrCh1Op
(
    // Outputs
    .ddr_wr_req                     (ddr_wr1_req                            ),
    .ddr_wr_clr                     (ddr_wr1_clr                            ),
    .ddr_wr_cmd                     (ddr_wr1_cmd[2:0]                       ),
    .ddr_wr_cmd_en                  (ddr_wr1_cmd_en                         ),
    .ddr_wr_addr                    (ddr_wr1_addr[31:0]                     ),
    .ddr_wr_data                    (ddr_wr1_data[127:0]                    ),
    .ddr_wr_mask                    (ddr_wr1_mask[15:0]                     ),
    .ddr_wr_en                      (ddr_wr1_en                             ),
    .ddr_wr_end                     (ddr_wr1_end                            ),
    
    .wr_buf_addr                    (wr1_buf_addr[8:0]                      ),
    .wr_buf_addr_en                 (wr1_buf_addr_en                        ),
    // Inputs
    .ddr_clk                        (ddr_clk                                ),
    .rst                            (rst                                    ),
    .ddr_wr_ack                     (ddr_wr1_ack                            ),
    .ddr_wr_cmd_rdy                 (ddr_wr1_cmd_rdy                        ),
    .ddr_wr_rdy                     (ddr_wr1_rdy                            ),
    
    .wr_buf_data                    (wr1_buf_data[127:0]                    ),
    .wr_buf_mask                    (wr1_buf_mask[15:0]                     ),
    .wr_burst_begin                 (wr1_burst_begin                        ),
    .wr_burst_rdy                   (wr1_burst_rdy                          ),
    .wr_len                         (wr1_len[8:0]                           ),
    .wr_addr                        (wr1_addr[27:0]                         ),
    .wr_clk                         (wr1_clk                                ),
    .wr_buf_sel                     (wr1_buf_sel                            ),
    .wr_buf_base                    (wr1_buf_base[7:0]                      )
);
//******************************************************************************

//******************************************************************************
//

DdrRdOp U_DdrRdCh0Op
(
    // Outputs
    .ddr_rd_req                     (ddr_rd0_req                            ),
    .ddr_rd_clr                     (ddr_rd0_clr                            ),
    .ddr_rd_cmd                     (ddr_rd0_cmd[2:0]                       ),
    .ddr_rd_cmd_en                  (ddr_rd0_cmd_en                         ),
    .ddr_rd_addr                    (ddr_rd0_addr[31:0]                     ),
    .ddr_rd_len                     (ddr_rd0_len[8:0]                       ),
    
    .rd_data_en                     (rd0_data_en                            ),
    .rd_data                        (rd0_data[127:0]                        ),
    .rd_data_addr                   (rd0_data_addr[8:0]                     ),
    .rd_data_opt                    (rd0_data_opt[13:0]                     ),
    // Inputs
    .ddr_clk                        (ddr_clk                                ),
    .rst                            (rst                                    ),
    .ddr_rd_ack                     (ddr_rd0_ack                            ),
    .ddr_rd_cmd_rdy                 (ddr_rd0_cmd_rdy                        ),
    .ddr_rd_valid                   (ddr_rd0_valid                          ),
    .ddr_rd_data                    (ddr_rd0_data[127:0]                    ),
    
    .rd_burst_begin                 (rd0_burst_begin                        ),
    .rd_burst_rdy                   (rd0_burst_rdy                          ),
    .rd_len                         (rd0_len[8:0]                           ),
    .rd_addr                        (rd0_addr[27:0]                         ),
    .rd_clk                         (rd0_clk                                ),
    .rd_buf_sel                     (rd0_buf_sel                            ),
    .rd_buf_base                    (rd0_buf_base                           ),
    .rd_opt                         (rd0_opt[13:0]                          )
);

// DdrRdOp U_DdrRdCh1Op
// (
//     // Outputs
//     .ddr_rd_req                     (ddr_rd1_req                            ),
//     .ddr_rd_clr                     (ddr_rd1_clr                            ),
//     .ddr_rd_cmd                     (ddr_rd1_cmd[2:0]                       ),
//     .ddr_rd_cmd_en                  (ddr_rd1_cmd_en                         ),
//     .ddr_rd_addr                    (ddr_rd1_addr[31:0]                     ),
//     .ddr_rd_len                     (ddr_rd1_len[8:0]                       ),
    
//     .rd_data_en                     (rd1_data_en                            ),
//     .rd_data                        (rd1_data[127:0]                        ),
//     .rd_data_addr                   (rd1_data_addr[8:0]                     ),
//     .rd_data_opt                    (rd1_data_opt[13:0]                     ),
//     // Inputs
//     .ddr_clk                        (ddr_clk                                ),
//     .rst                            (rst                                    ),
//     .ddr_rd_ack                     (ddr_rd1_ack                            ),
//     .ddr_rd_cmd_rdy                 (ddr_rd1_cmd_rdy                        ),
//     .ddr_rd_valid                   (ddr_rd1_valid                          ),
//     .ddr_rd_data                    (ddr_rd1_data[127:0]                    ),
    
//     .rd_burst_begin                 (rd1_burst_begin                        ),
//     .rd_burst_rdy                   (rd1_burst_rdy                          ),
//     .rd_len                         (rd1_len[8:0]                           ),
//     .rd_addr                        (rd1_addr[27:0]                         ),
//     .rd_clk                         (rd1_clk                                ),
//     .rd_buf_sel                     (rd1_buf_sel                            ),
//     .rd_buf_base                    (rd1_buf_base                           ),
//     .rd_opt                         (rd1_opt[13:0]                          )
// );
//******************************************************************************

//******************************************************************************
//

DdrArbitOp U_DdrArbitOp
(
    // Outputs 
    .app_addr                       (app_addr[27:0]                         ),
    .app_cmd                        (app_cmd[2:0]                           ),
    .app_en                         (app_en                                 ),
    .app_wdf_data                   (app_wdf_data[127:0]                    ),
    .app_wdf_end                    (app_wdf_end                            ),
    .app_wdf_wren                   (app_wdf_wren                           ),
    .app_wdf_mask                   (app_wdf_mask[15:0]                     ),
    // Inputs
    .ddr_clk                        (ddr_clk                                ),
    .rst                            (rst                                    ),
    .app_rdy                        (app_rdy                                ),
    .app_wdf_rdy                    (app_wdf_rdy                            ),
    .app_rd_data                    (app_rd_data[127:0]                     ),
    .app_rd_data_valid              (app_rd_data_valid                      ),

    .ddr_rd0_req                    (ddr_rd0_req                            ),
    .ddr_rd0_clr                    (ddr_rd0_clr                            ),
    .ddr_rd0_ack                    (ddr_rd0_ack                            ),
    .ddr_rd0_cmd_rdy                (ddr_rd0_cmd_rdy                        ),
    .ddr_rd0_cmd                    (ddr_rd0_cmd[2:0]                       ),
    .ddr_rd0_cmd_en                 (ddr_rd0_cmd_en                         ),
    .ddr_rd0_addr                   (ddr_rd0_addr[31:0]                     ),
    .ddr_rd0_len                    (ddr_rd0_len[8:0]                       ),
    .ddr_rd0_valid                  (ddr_rd0_valid                          ),
    .ddr_rd0_data                   (ddr_rd0_data[127:0]                    ),

    .ddr_rd1_req                    (1'b0                                   ),//(ddr_rd1_req                            ),
    .ddr_rd1_clr                    (                                       ),//(ddr_rd1_clr                            ),
    .ddr_rd1_ack                    (                                       ),//(ddr_rd1_ack                            ),
    .ddr_rd1_cmd_rdy                (                                       ),//(ddr_rd1_cmd_rdy                        ),
    .ddr_rd1_cmd                    (                                       ),//(ddr_rd1_cmd[2:0]                       ),
    .ddr_rd1_cmd_en                 (1'b0                                   ),//(ddr_rd1_cmd_en                         ),
    .ddr_rd1_addr                   (                                       ),//(ddr_rd1_addr[31:0]                     ),
    .ddr_rd1_len                    (                                       ),//(ddr_rd1_len[8:0]                       ),
    .ddr_rd1_valid                  (                                       ),//(ddr_rd1_valid                          ),
    .ddr_rd1_data                   (                                       ),//(ddr_rd1_data[127:0]                    ),

    .ddr_wr0_req                    (ddr_wr0_req                            ),
    .ddr_wr0_clr                    (ddr_wr0_clr                            ), 
    .ddr_wr0_cmd                    (ddr_wr0_cmd[2:0]                       ),
    .ddr_wr0_cmd_en                 (ddr_wr0_cmd_en                         ),
    .ddr_wr0_addr                   (ddr_wr0_addr[31:0]                     ),
    .ddr_wr0_data                   (ddr_wr0_data[127:0]                    ),
    .ddr_wr0_mask                   (ddr_wr0_mask[15:0]                     ),
    .ddr_wr0_en                     (ddr_wr0_en                             ),
    .ddr_wr0_end                    (ddr_wr0_end                            ),
    .ddr_wr0_ack                    (ddr_wr0_ack                            ),
    .ddr_wr0_cmd_rdy                (ddr_wr0_cmd_rdy                        ),
    .ddr_wr0_rdy                    (ddr_wr0_rdy                            ), 

    .ddr_wr1_req                    (ddr_wr1_req                            ),
    .ddr_wr1_clr                    (ddr_wr1_clr                            ),
    .ddr_wr1_cmd                    (ddr_wr1_cmd[2:0]                       ),
    .ddr_wr1_cmd_en                 (ddr_wr1_cmd_en                         ),
    .ddr_wr1_addr                   (ddr_wr1_addr[31:0]                     ),
    .ddr_wr1_data                   (ddr_wr1_data[127:0]                    ),
    .ddr_wr1_mask                   (ddr_wr1_mask[15:0]                     ),
    .ddr_wr1_en                     (ddr_wr1_en                             ),
    .ddr_wr1_end                    (ddr_wr1_end                            ),
    .ddr_wr1_ack                    (ddr_wr1_ack                            ),
    .ddr_wr1_cmd_rdy                (ddr_wr1_cmd_rdy                        ),
    .ddr_wr1_rdy                    (ddr_wr1_rdy                            )
);

//******************************************************************************

endmodule
