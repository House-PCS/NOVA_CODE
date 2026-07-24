//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   DdrArbitOp.v
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
module DdrArbitOp
(
    //output
    output      [27:0]              app_addr                                ,
    output      [2:0]               app_cmd                                 ,
    output                          app_en                                  ,
    output      [127:0]             app_wdf_data                            ,
    output                          app_wdf_end                             ,
    output                          app_wdf_wren                            ,
    output      [15:0]              app_wdf_mask                            ,
    //input
    input                           ddr_clk                                 ,
    input                           rst                                     ,
    input                           app_rdy                                 ,
    input                           app_wdf_rdy                             ,
    input       [127:0]             app_rd_data                             ,
    input                           app_rd_data_valid                       ,

    input                           ddr_rd0_req                             ,
    input                           ddr_rd0_clr                             ,
    output                          ddr_rd0_ack                             ,
    output                          ddr_rd0_cmd_rdy                         ,
    input       [2:0]               ddr_rd0_cmd                             ,
    input                           ddr_rd0_cmd_en                          ,
    input       [31:0]              ddr_rd0_addr                            ,
    input       [8:0]               ddr_rd0_len                             ,
    output                          ddr_rd0_valid                           ,
    output      [127:0]             ddr_rd0_data                            ,

    input                           ddr_rd1_req                             ,
    input                           ddr_rd1_clr                             ,
    output                          ddr_rd1_ack                             ,
    output                          ddr_rd1_cmd_rdy                         ,
    input       [2:0]               ddr_rd1_cmd                             ,
    input                           ddr_rd1_cmd_en                          ,
    input       [31:0]              ddr_rd1_addr                            ,
    input       [8:0]               ddr_rd1_len                             ,
    output                          ddr_rd1_valid                           ,
    output      [127:0]             ddr_rd1_data                            ,

    input                           ddr_wr0_req                             ,
    input                           ddr_wr0_clr                             ,
    input       [2:0]               ddr_wr0_cmd                             ,
    input                           ddr_wr0_cmd_en                          ,
    input       [31:0]              ddr_wr0_addr                            ,
    input       [127:0]             ddr_wr0_data                            ,
    input       [15:0]              ddr_wr0_mask                            ,
    input                           ddr_wr0_en                              ,
    input                           ddr_wr0_end                             ,
    output                          ddr_wr0_ack                             ,
    output                          ddr_wr0_cmd_rdy                         ,
    output                          ddr_wr0_rdy                             ,

    input                           ddr_wr1_req                             ,
    input                           ddr_wr1_clr                             ,
    input       [2:0]               ddr_wr1_cmd                             ,
    input                           ddr_wr1_cmd_en                          ,
    input       [31:0]              ddr_wr1_addr                            ,
    input       [127:0]             ddr_wr1_data                            ,
    input       [15:0]              ddr_wr1_mask                            ,
    input                           ddr_wr1_en                              ,
    input                           ddr_wr1_end                             ,
    output                          ddr_wr1_ack                             ,
    output                          ddr_wr1_cmd_rdy                         ,
    output                          ddr_wr1_rdy                             
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    localparam  [6:0]               S_IDLE              = 7'h0              ;
    localparam  [6:0]               S_RD0_GET           = 7'h1              ;
    localparam  [6:0]               S_RD0_DONE          = 7'h2              ;
    localparam  [6:0]               S_WR0_GET           = 7'h3              ;
    localparam  [6:0]               S_WR0_DONE          = 7'h4              ;
    localparam  [6:0]               S_RD1_GET           = 7'h5              ;
    localparam  [6:0]               S_RD1_DONE          = 7'h6              ;
    localparam  [6:0]               S_WR1_GET           = 7'h7              ;
    localparam  [6:0]               S_WR1_DONE          = 7'h8              ;
//******************************************************************************

//******************************************************************************
//

    reg         [7:0]               state               = 8'd0              ;
    reg         [7:0]               state_n             = 8'd0              ;
    reg         [8:0]               rd_len              = 9'd0              ;
    reg         [3:0]               rd_id               = 4'd0              ;
    wire        [15:0]              rd_cmd                                  ;
    wire        [15:0]              rd_cmd_suspend                          ;
    wire                            rd_cmd_wren                             ;
    wire                            rd_cmd_clr                              ;
    reg         [8:0]               data_cnt            = 9'd0              ;
    reg                             ddr_rd0_ack_d       = 1'd0              ;
    reg                             ddr_rd1_ack_d       = 1'd0              ;
//******************************************************************************

//******************************************************************************
//

assign app_addr             = (state[3:0] == S_RD0_GET) ? ddr_rd0_addr[27:0] :
                              (state[3:0] == S_WR0_GET) ? ddr_wr0_addr[27:0] :
                              (state[3:0] == S_RD1_GET) ? ddr_rd1_addr[27:0] :
                              (state[3:0] == S_WR1_GET) ? ddr_wr1_addr[27:0] : 28'd0;
    
assign app_cmd              = (state[3:0] == S_RD0_GET || state[3:0] == S_RD1_GET) ? 3'b001 : 3'b000;
    
assign app_en               = (state[3:0] == S_RD0_GET) ? ddr_rd0_cmd_en :
                              (state[3:0] == S_WR0_GET) ? ddr_wr0_cmd_en :
                              (state[3:0] == S_RD1_GET) ? ddr_rd1_cmd_en :
                              (state[3:0] == S_WR1_GET) ? ddr_wr1_cmd_en : 1'b0;

assign app_wdf_data[127:0]  = (state[3:0] == S_WR0_GET) ? ddr_wr0_data[127:0] : 
                              (state[3:0] == S_WR1_GET) ? ddr_wr1_data[127:0] : 128'd0;

assign app_wdf_wren         = (state[3:0] == S_WR0_GET) ? ddr_wr0_en          : 
                              (state[3:0] == S_WR1_GET) ? ddr_wr1_en          : 1'b0;

assign app_wdf_end          = (state[3:0] == S_WR0_GET) ? ddr_wr0_end         : 
                              (state[3:0] == S_WR1_GET) ? ddr_wr1_end         : 1'b0;

assign app_wdf_mask[15:0]   = (state[3:0] == S_WR0_GET) ? ddr_wr0_mask[15:0]  : 
                              (state[3:0] == S_WR1_GET) ? ddr_wr1_mask[15:0]  : 16'h0;                                     

//******************************************************************************

//******************************************************************************
//

assign ddr_rd0_ack          = ((state[3:0] == S_RD0_GET) && ddr_rd0_req);//仲裁到读命令了
assign ddr_rd0_cmd_rdy      = ((state[3:0] == S_RD0_GET) && app_rdy);//可以给Mig中写读命令了
assign ddr_rd0_valid        = ((rd_cmd_suspend[12:9] == 4'h0) && app_rd_data_valid); 
assign ddr_rd0_data[127:0]  = app_rd_data[127:0];

assign ddr_rd1_ack          = ((state[3:0] == S_RD1_GET) && ddr_rd1_req);//仲裁到读命令了
assign ddr_rd1_cmd_rdy      = ((state[3:0] == S_RD1_GET) && app_rdy);//可以给Mig中写读命令了
assign ddr_rd1_valid        = ((rd_cmd_suspend[12:9] == 4'h1) && app_rd_data_valid);  
assign ddr_rd1_data[127:0]  = app_rd_data[127:0];

assign ddr_wr0_ack          = ((state[3:0] == S_WR0_GET) && ddr_wr0_req);//仲裁到写命令了
assign ddr_wr0_cmd_rdy      = ((state[3:0] == S_WR0_GET) && app_rdy);    //可以给Mig中写写命令了
assign ddr_wr0_rdy          = ((state[3:0] == S_WR0_GET) && app_wdf_rdy);//可以给Mig写入写数据了 

assign ddr_wr1_ack          = ((state[3:0] == S_WR1_GET) && ddr_wr1_req);//仲裁到写命令了
assign ddr_wr1_cmd_rdy      = ((state[3:0] == S_WR1_GET) && app_rdy);    //可以给Mig中写写命令了
assign ddr_wr1_rdy          = ((state[3:0] == S_WR1_GET) && app_wdf_rdy);//可以给Mig写入写数据了
//******************************************************************************

//******************************************************************************
//

always @ (posedge ddr_clk) begin
    if(state[3:0] == S_IDLE)
        rd_len[8:0] <= 9'h0;
    else if(ddr_rd0_ack)
        rd_len[8:0] <= ddr_rd0_len[8:0];
    else if(ddr_rd1_ack) 
        rd_len[8:0] <= ddr_rd1_len[8:0];
    else
        rd_len[8:0] <= rd_len[8:0];
end

always @ (posedge ddr_clk) begin  //通过ID去识别
    if(state[3:0] == S_IDLE)
        rd_id[3:0] <= 4'h0;
    else if(ddr_rd0_ack)
        rd_id[3:0] <= 4'h0;  
    else if(ddr_rd1_ack)
        rd_id[3:0] <= 4'h1;  
    else 
        rd_id[3:0] <= rd_id[3:0];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge ddr_clk) begin
    ddr_rd0_ack_d <= ddr_rd0_ack;
end

always @ (posedge ddr_clk) begin
    ddr_rd1_ack_d <= ddr_rd1_ack;
end

assign rd_cmd_wren  = ddr_rd0_ack_d || ddr_rd1_ack_d;

//******************************************************************************

//******************************************************************************
//

assign rd_cmd[15:0] = {3'h0, rd_id[3:0], rd_len[8:0]};

//******************************************************************************

//******************************************************************************
//

Fifo16x32_16x32 U_RdOpCmdSuspendFifo
(
    .srst                           (rst                                    ),
    .clk                            (ddr_clk                                ),
    
    .din                            (rd_cmd[15:0]                           ),
    .wr_en                          (rd_cmd_wren                            ),
    
    .rd_en                          (rd_cmd_clr                             ),
    .dout                           (rd_cmd_suspend[15:0]                   ),
    
    .full                           (                                       ),
    .almost_full                    (                                       ),
    .empty                          (                                       ),
    .data_count                     (                                       )
);

//******************************************************************************

//******************************************************************************

always @ (posedge ddr_clk) begin
    if(rst)
        data_cnt[8:0] <= 9'h0;
    else if(rd_cmd_clr)
        data_cnt[8:0] <= 9'h0;
    else if(app_rd_data_valid)
        data_cnt[8:0] <= data_cnt[8:0] + 1;
    else
        data_cnt[8:0] <= data_cnt[8:0];
end

assign rd_cmd_clr   = (app_rd_data_valid && (data_cnt[8:0] == (rd_cmd_suspend[8:0] - 1)));

//******************************************************************************

//******************************************************************************
//

always @ (posedge ddr_clk) begin     
    if(rst)              
        state[3:0] <= S_IDLE;
    else 
        state[3:0] <= state_n[3:0];
end

always @ (*) begin
    state_n[3:0] = state[3:0];
    case(state[3:0])
        S_IDLE      :   begin
                            if(ddr_rd0_req)
                                state_n[3:0] = S_RD0_GET;
                            else if(ddr_wr0_req)
                                state_n[3:0] = S_WR0_GET;
                            else if(ddr_rd1_req)
                                state_n[3:0] = S_RD1_GET;
                            else if(ddr_wr1_req)
                                state_n[3:0] = S_WR1_GET;                            
                            else
                                state_n[3:0] = state_n[3:0];
                        end
        S_RD0_GET   :   begin
                            if(ddr_rd0_clr)
                                state_n[3:0] = S_RD0_DONE;
                            else
                                state_n[3:0] = state_n[3:0];
                        end
        S_RD0_DONE  :   begin
                            if(ddr_wr0_req)
                                state_n[3:0] = S_WR0_GET;
                            else if(ddr_rd0_req)
                                state_n[3:0] = S_RD0_GET;
                            else if(ddr_rd1_req)
                                state_n[3:0] = S_RD1_GET;
                            else if(ddr_wr1_req)
                                state_n[3:0] = S_WR1_GET;    
                            else
                                state_n[3:0] = S_IDLE;
                        end
        S_WR0_GET   :   begin
                            if(ddr_wr0_clr)
                                state_n[3:0] = S_WR0_DONE;
                            else
                                state_n[3:0] = state_n[3:0];
                        end
        S_WR0_DONE  :   begin
                            if(ddr_rd0_req) 
                                state_n[3:0] = S_RD0_GET;
                            else if(ddr_wr0_req)
                                state_n[3:0] = S_WR0_GET;
                            else if(ddr_rd1_req)
                                state_n[3:0] = S_RD1_GET;
                            else if(ddr_wr1_req)
                                state_n[3:0] = S_WR1_GET; 
                            else
                                state_n[3:0] = S_IDLE;
                        end
        S_RD1_GET   :   begin
                            if(ddr_rd1_clr)
                                state_n[3:0] = S_RD1_DONE;
                            else
                                state_n[3:0] = state_n[3:0];
                        end       
        S_RD1_DONE  :   begin
                            if(ddr_rd0_req)
                                state_n[3:0] = S_RD0_GET;
                            else if(ddr_wr0_req)
                                state_n[3:0] = S_WR0_GET;
                            else if(ddr_wr1_req)
                                state_n[3:0] = S_WR1_GET;
                            else if(ddr_rd1_req)
                                state_n[3:0] = S_RD1_GET;    
                            else
                                state_n[3:0] = S_IDLE;
                        end 
        S_WR1_GET   :   begin
                            if(ddr_wr1_clr)
                                state_n[3:0] = S_WR1_DONE;
                            else
                                state_n[3:0] = state_n[3:0];
                        end
        S_WR1_DONE  :   begin
                            if(ddr_rd0_req) 
                                state_n[3:0] = S_RD0_GET;
                            else if(ddr_wr0_req)
                                state_n[3:0] = S_WR0_GET;                               
                            else if(ddr_rd1_req)
                                state_n[3:0] = S_RD1_GET;
                            else if(ddr_wr1_req)
                                state_n[3:0] = S_WR1_GET; 
                            else
                                state_n[3:0] = S_IDLE;
                        end
        default     :   state_n[3:0] = S_IDLE;
    endcase
end

//******************************************************************************

endmodule
