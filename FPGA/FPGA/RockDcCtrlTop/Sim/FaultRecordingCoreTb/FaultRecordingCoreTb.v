//`timescale 1ps/100fs
`timescale 1ns/1ps
module FaultRecordingCoreTb();


//-----------------------------DDR3 MODE---------------------------------
  //--------------Clk-----------------------------
    reg                                 wSysClk2/*100Mhz*/         =1;
    reg                                 wSysClk3/*200Mhz*/         =1;
    wire                                wDdrRdOpClk                ;
    wire                                wDdrWrOpClk                ;
    always #5 wSysClk2/*100Mhz*/ = ~wSysClk2/*100Mhz*/;
    always #2.5 wSysClk3/*200Mhz*/ = ~wSysClk3/*200Mhz*/;
    assign wDdrRdOpClk = wSysClk2/*100Mhz*/;
    assign wDdrWrOpClk = wSysClk2/*100Mhz*/;
 //--------------------------------------------------
    wire                                wDdrInitDone               ;
    wire                                wDdrWrOpDone               ;
    wire                                wDdrMonitorDataVld         ;
    wire                                wDdrWrOpBurstBegin1        ;
    wire                                wDdrWrOpBurstRdy1          ;
    wire                 [   4: 0]      wDdrWrBufBaseAddr1         ;
    wire                                wDdrWrBufSel1              ;
    wire                 [   8: 0]      wDdrWrOpLen1               ;
    wire                 [  25: 0]      wDdrWrOpAddr1              ;
    wire                 [   8: 0]      wDdrWrDataAddr1            ;
    wire                                wDdrWrDataAddrEn1          ;
    wire                 [  15: 0]      wDdrWrOpDataMask1          ;
    wire                 [ 127: 0]      wDdrWrOpData1              ;
    wire                                wDdrRdOpBurstBegin1        ;
    wire                                wDdrRdOpBurstRdy1          ;
    wire                 [   4: 0]      wDdrRdBufBaseAddr1         ;
    wire                                wDdrRdBufSel1              ;
    wire                 [   8: 0]      wDdrRdOpLen1               ;
    wire                 [  25: 0]      wDdrRdOpAddr1              ;
    wire                                wDdrLogicClk               ;
    wire                 [   8: 0]      wDdrRdDataAddr1            ;
    wire                                wDdrRdDataEn1              ;
    wire                 [ 127: 0]      wDdrRdOpData1              ;

//**************************************************************************//

  // Reset Generation

  //**************************************************************************//
 localparam RESET_PERIOD = 200000/1000/1000; //in pSec 
    reg                                 sys_rst                    ;

  initial begin
    sys_rst = 1'b1;
    #RESET_PERIOD
    sys_rst = 1'b0;
   end

  //**************************************************************************//

  // Clock Generation

  //**************************************************************************//
 parameter CLKIN_PERIOD          = 10000/1000;
 parameter REFCLK_FREQ           = 200.0;
 localparam real REFCLK_PERIOD = (1000000.0/(2*REFCLK_FREQ))/1000;
    reg                                 sys_clk_i      = 1'b1          ;
    reg                                 clk_ref_i      = 1'b1          ;

  always sys_clk_i = #(CLKIN_PERIOD/2.0) ~sys_clk_i;

  always clk_ref_i = #REFCLK_PERIOD ~clk_ref_i;
  //**************************************************************************//

    reg                  [   7: 0]      rDdrInitDone             =8'd0;
    reg                  [  31: 0]      rMonitorData             =32'd0;
    reg                                 rMonitorDataVld          ='d0 ;

always@(posedge sys_clk_i/*100Mhz*/)begin
    rDdrInitDone[7:0] <= {rDdrInitDone[6:0],wDdrInitDone};
end

always@(posedge sys_clk_i/*100Mhz*/)begin
    if(rDdrInitDone[7])begin
        if(rMonitorData[31:0] == 'd147500)
            rMonitorData[31:0] <= rMonitorData[31:0];
        else
            rMonitorData[31:0] <= rMonitorData[31:0] + 1'b1;
    end
    else 
        rMonitorData[31:0] <= 32'd0;    
end

always@(posedge sys_clk_i/*100Mhz*/)begin
    //if(rMonitorData[31:0] >= 'd147455)
    if(rMonitorData[31:0] >= 'd4096)
        rMonitorDataVld <= 1'b0;        
    else if(rDdrInitDone[7])
        rMonitorDataVld <= 1'b1;
end

reg       rDdrMonitorDataVld = 'd0;
always@(posedge sys_clk_i)begin
    if(rMonitorData[31:0] == 'd4500)begin
        rDdrMonitorDataVld <= ~rDdrMonitorDataVld;
    end
end

 FaultRecordingCore U_FaultRecordingCore(
    .iSysClk                            (sys_clk_i/*100Mhz*/        ),
    .iRstP                              (!wDdrInitDone             ),
//*************************U_MonitorDataStorage******************************//  
    .iMonitorData                       (rMonitorData[31:0]        ),
    .iMonitorDataVld                    (rMonitorDataVld           ),
    .iDcDcErr                           (                          ),
    .iBmsShutdown                       (                          ),
    .oMcuRdStartAddr                    (                          ),
    //Write DDR CMD
    .iDdrWrCmdClk                       (wDdrWrOpClk               ),
    .oDdrWrBegin                        (wDdrWrOpBurstBegin1       ),
    .iDdrWrRdy                          (wDdrWrOpBurstRdy1         ),
    .oDdrWrBufBaseAddr                  (wDdrWrBufBaseAddr1[4:0]   ),
    .oDdrWrBufSel                       (wDdrWrBufSel1             ),
    .oDdrWrLen                          (wDdrWrOpLen1[8:0]         ),
    .oDdrWrAddr                         (wDdrWrOpAddr1[25:0]       ),
    //Write DDR Data
    .iDdrWrDataClk                      (wDdrLogicClk              ),
    .iDdrWrDataAddr                     (wDdrWrDataAddr1[8:0]      ),
    .iDdrWrDataAddrEn                   (wDdrWrDataAddrEn1         ),
    .oDdrWrMask                         (wDdrWrOpDataMask1[15:0]   ),
    .oDdrWrData                         (wDdrWrOpData1[127:0]      ),
//***********************************End*************************************//  
//****************************U_MonitorDataRead******************************//  
    .iDdrMonitorDataVld                 (rDdrMonitorDataVld        ),
    //Read DDR Cmd
    .iDdrRdCmdClk                       (wDdrRdOpClk               ),
    .oDdrRdBegin                        (wDdrRdOpBurstBegin1       ),
    .iDdrRdRdy                          (wDdrRdOpBurstRdy1         ),
    .oDdrRdBufBaseAddr                  (wDdrRdBufBaseAddr1[4:0]   ),
    .oDdrRdBufSel                       (wDdrRdBufSel1             ),
    .oDdrRdLen                          (wDdrRdOpLen1[8:0]         ),
    .oDdrRdAddr                         (wDdrRdOpAddr1[25:0]       ),
    //Read DDR Data
    .iDdrRdDataClk                      (wDdrLogicClk              ),
    .iDdrRdDataAddr                     (wDdrRdDataAddr1[8:0]      ),
    .iDdrRdDataEn                       (wDdrRdDataEn1             ),
    .oDdrRdData                         (wDdrRdOpData1[127:0]      ) 
//***********************************End*************************************//  
);

DdrCtrlCore U_DdrCtrlCore
(
    .iDdrClk                            (sys_clk_i/*100Mhz*/       ),////100MHz
    .iDdrRefClk                         (clk_ref_i/*200Mhz*/       ),////200MHz
    .iMigRst                            (sys_rst                   ),
    .iFpgaTemp                          (12'h190                   ),//
    .oDdrInitDone                       (wDdrInitDone              ),//o
    .oDdrWrOpDone                       (wDdrWrOpDone              ),//o
    .oDdrLogicClk                       (wDdrLogicClk              ),//o

 //---------------DDR Ports------------------
    // .ddr3_addr                          (ddr3_addr_fpga            ),
    // .ddr3_ba                            (ddr3_ba_fpga              ),
    // .ddr3_cas_n                         (ddr3_cas_n_fpga           ),
    // .ddr3_ck_n                          (ddr3_ck_n_fpga            ),
    // .ddr3_ck_p                          (ddr3_ck_p_fpga            ),
    // .ddr3_cke                           (ddr3_cke_fpga             ),
    // .ddr3_ras_n                         (ddr3_ras_n_fpga           ),
    // .ddr3_reset_n                       (ddr3_reset_n              ),
    // .ddr3_we_n                          (ddr3_we_n_fpga            ),
    // .ddr3_dq                            (ddr3_dq_fpga              ),
    // .ddr3_dqs_n                         (ddr3_dqs_n_fpga           ),
    // .ddr3_dqs_p                         (ddr3_dqs_p_fpga           ),
    // .ddr3_dm                            (ddr3_dm_fpga              ),
    // .ddr3_odt                           (ddr3_odt_fpga             ), 
//-------------------------------------------
   
    .wr0_addr                            ({2'b00,wDdrWrOpAddr1[26:0]}),//(),//(wDdrWrOpAddr0[27:0]       ),//i
    .wr0_buf_base                        ( {3'd0,wDdrWrBufBaseAddr1[4:0]}),//(),//(8'd0                      ),//i
    .wr0_buf_sel                         (wDdrWrBufSel1             ),//(),//(1'd0                      ),//i
    .wr0_buf_data                        (wDdrWrOpData1             ),//(),//(wDdrWrOpBurstBegin0       ),//i
    .wr0_buf_mask                        (wDdrWrOpDataMask1         ),//(),//(wDdrWrOpBurstRdy0         ),
    .wr0_burst_begin                     (wDdrWrOpBurstBegin1       ),//(),//(wDdrWrOpClk               ),//i
    .wr0_burst_rdy                       (wDdrWrOpBurstRdy1         ),//(),//(9'd1                      ),//i
    .wr0_clk                             (wDdrWrOpClk               ),//(),//(                          ),//o
    .wr0_len                             (wDdrWrOpLen1              ),//(),//(                          ),//o
    .wr0_buf_addr                        (wDdrWrDataAddr1           ),//(),//(wDdrWrOpData0[127:0]      ),//i
    .wr0_buf_addr_en                     (wDdrWrDataAddrEn1         ),//(),//(wDdrWrOpDataMask0[15:0]   ),//i

    .wr1_addr                           (),//({2'b00,wDdrWrOpAddr1[26:0]}),
    .wr1_buf_base                       (),//( {3'd0,wDdrWrBufBaseAddr1[4:0]}),
    .wr1_buf_sel                        (),//(wDdrWrBufSel1             ),
    .wr1_buf_data                       (),//(wDdrWrOpData1             ),
    .wr1_buf_mask                       (),//(wDdrWrOpDataMask1         ),
    .wr1_burst_begin                    (),//(wDdrWrOpBurstBegin1       ),
    .wr1_burst_rdy                      (),//(wDdrWrOpBurstRdy1         ),
    .wr1_clk                            (),//(wDdrWrOpClk               ),
    .wr1_len                            (),//(wDdrWrOpLen1              ),
    .wr1_buf_addr                       (),//(wDdrWrDataAddr1           ),
    .wr1_buf_addr_en                    (),//(wDdrWrDataAddrEn1         ),

    .rd0_clk                            (wDdrRdOpClk               ),//i
    .rd0_addr                           ({2'b00,wDdrRdOpAddr1[26:0]}),//i
    .rd0_burst_begin                    (wDdrRdOpBurstBegin1       ),//i
    .rd0_burst_rdy                      (wDdrRdOpBurstRdy1         ),
    .rd0_len                            (wDdrRdOpLen1              ),
    .rd0_opt                            (14'd0                     ),
    .rd0_buf_sel                        (wDdrRdBufSel1             ),
    .rd0_buf_base                       ({3'd0,wDdrRdBufBaseAddr1[4:0]}        ),
    .rd0_data                           (wDdrRdOpData1             ),//o
    .rd0_data_addr                      (wDdrRdDataAddr1           ),//o
    .rd0_data_en                        (wDdrRdDataEn1             ),//o
    .rd0_data_opt                       (                          ), //o

    .rd1_clk                           (),//(wDdrRdOpClk               ),//i
    .rd1_addr                          (),//({1'b0,wDdrRdOpAddr1[26:0]}),//i
    .rd1_burst_begin                   (),//(wDdrRdOpBurstBegin1       ),//i
    .rd1_burst_rdy                     (),//(wDdrRdOpBurstRdy1         ),
    .rd1_len                           (),//(wDdrRdOpLen1              ),
    .rd1_opt                           (),//(14'd0                     ),
    .rd1_buf_sel                       (),//(wDdrRdBufSel1             ),
    .rd1_buf_base                      (),//({3'd0,wDdrRdBufBaseAddr1[4:0]}        ),
    .rd1_data                          (),//(wDdrRdOpData1             ),//o
    .rd1_data_addr                     (),//(wDdrRdDataAddr1           ),//o
    .rd1_data_en                       (),//(wDdrRdDataEn1             ),//o
    .rd1_data_opt                      ()//(                          ) //o
);





endmodule