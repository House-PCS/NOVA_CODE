//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   DdrRdOp.v
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
module DdrRdOp
(
    output  reg                     ddr_rd_req                              ,//给仲裁的读请求，不是给Mig的
    output                          ddr_rd_clr                              ,//给仲裁，读命令处理完了
    output      [2:0]               ddr_rd_cmd                              ,
    output                          ddr_rd_cmd_en                           ,
    output  reg [31:0]              ddr_rd_addr                             ,
    output      [8:0]               ddr_rd_len                              ,
    
    output  reg                     rd_data_en                              ,
    output  reg [127:0]             rd_data                                 ,
    output  reg [8:0]               rd_data_addr                            ,//Ram地址
    output  reg [13:0]              rd_data_opt                             ,
    
    input                           ddr_clk                                 ,
    input                           rst                                     ,
    input                           ddr_rd_ack                              ,//仲裁到读命令了
    input                           ddr_rd_cmd_rdy                          ,//mig可以接收命令
    input                           ddr_rd_valid                            ,
    input       [127:0]             ddr_rd_data                             ,

    input                           rd_clk                                  ,
    input                           rd_burst_begin                          ,
    output                          rd_burst_rdy                            ,
    input       [8:0]               rd_len                                  ,
    input       [27:0]              rd_addr                                 ,
    input                           rd_buf_sel                              ,
    input       [7:0]               rd_buf_base                             ,
    input       [13:0]              rd_opt                                   
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    localparam  [3:0]               S_IDLE              = 4'h0              ;
    localparam  [3:0]               S_RD_REQ            = 4'h1              ;
    localparam  [3:0]               S_RD_CMD            = 4'h2              ;
    localparam  [3:0]               S_RD_DONE           = 4'h3              ; 

//******************************************************************************

//******************************************************************************
//
    reg         [47:0]              rd_cmd_pkt          = 48'd0             ;
    wire        [47:0]              rd_cmd_out                              ;
    wire        [47:0]              rd_cmd_suspend                          ;
    reg                             rd_cmd_wr_en        = 1'b0              ;
    wire                            rd_cmd_rd_en                            ;
    wire                            rd_cmd_sus_rd_en                        ;
    reg                             rd_cmd_sus_rd_en_r                      ;
    wire                            rd_cmd_empty                            ;
    wire                            rd_cmd_alfull                           ;

    wire                            ddr_cmd_finish                          ;
    reg         [3:0]               state               = 4'd0              ;
    reg         [3:0]               state_n             = 4'd0              ;
    reg         [8:0]               ddr_cmd_cnt         = 9'd0              ;
    reg                             ddr_rd_valid_r      = 1'd0              ;
    reg         [127:0]             ddr_rd_data_r                           ;
//******************************************************************************

//******************************************************************************
//

//assign rd_cmd_pkt[47:0] = {rd_opt[13:0], rd_addr[27:3], rd_len[8:0]}        ;
//assign rd_cmd_pkt[47:0] = {5'd0,rd_buf_sel,rd_buf_base[7:0], rd_addr[27:3], rd_len[8:0]}        ;

always @ (posedge rd_clk) begin   
    if(rd_burst_begin && (!rd_cmd_alfull))
        rd_cmd_wr_en <= 1'b1;
    else
        rd_cmd_wr_en <= 1'b0;                
end

always @ (posedge rd_clk) begin 
    rd_cmd_pkt[47:0]  <= {5'd0,rd_buf_base[7:0],rd_buf_sel, rd_addr[27:3], rd_len[8:0]};
end

//******************************************************************************

//******************************************************************************
//

Fifo48x16_48x16 U_RdOpCmdFifo
(
    .rst                            (rst                                    ),

    .wr_clk                         (rd_clk                                 ),
    .din                            (rd_cmd_pkt[47:0]                       ),
    .wr_en                          (rd_cmd_wr_en                           ),

    .rd_clk                         (ddr_clk                                ),
    .rd_en                          (rd_cmd_rd_en                           ),
    .dout                           (rd_cmd_out[47:0]                       ),
    
    .full                           (                                       ),
    .almost_full                    (rd_cmd_alfull                          ),
    .empty                          (rd_cmd_empty                           ),
    .rd_data_count                  (                                       ),
    .wr_data_count                  (                                       )
 );

assign  rd_burst_rdy = (!rd_cmd_alfull);
//******************************************************************************

//******************************************************************************
//

Fifo48x16_48x16 U_RdOpCmdSuspendFifo
(
    .rst                            (rst                                    ),

    .wr_clk                         (rd_clk                                 ),
    .din                            (rd_cmd_pkt[47:0]                       ),
    .wr_en                          (rd_cmd_wr_en                         ),

    .rd_clk                         (ddr_clk                                ),
    .rd_en                          (rd_cmd_sus_rd_en                       ),
    .dout                           (rd_cmd_suspend[47:0]                   ),
    
    .full                           (                                       ),
    .almost_full                    (                                       ),
    .empty                          (rd_cmd_sus_empty                       ),
    .rd_data_count                  (                                       ),
    .wr_data_count                  (                                       )
);

//******************************************************************************

//******************************************************************************
//

assign rd_cmd_rd_en     = (state[3:0] == S_RD_DONE)                         ;

assign ddr_rd_cmd[2:0]  = 3'b001                                            ;   //read operation
assign ddr_rd_cmd_en    = (state[3:0] == S_RD_CMD)                          ;
assign ddr_rd_clr       = (state[3:0] == S_RD_DONE)                         ;
assign ddr_rd_len[8:0]  = rd_cmd_out[8:0]                                   ;

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
                            if (~rd_cmd_empty)
                                state_n[3:0] = S_RD_REQ;
                            else
                                state_n[3:0] = state_n[3:0];
                        end
        S_RD_REQ    :   begin
                            if(ddr_rd_ack) //仲裁到读命令了，可以下发读命令了
                                state_n[3:0] = S_RD_CMD;
                            else
                                state_n[3:0] = state_n[3:0];
                        end
        S_RD_CMD    :   begin
                            if(ddr_cmd_finish)
                                state_n[3:0] = S_RD_DONE;
                            else
                                state_n[3:0] = state_n[3:0];
                        end
        S_RD_DONE   :   state_n[3:0] = S_IDLE;
        default     :   state_n[3:0] = S_IDLE;
    endcase
end


//******************************************************************************

//******************************************************************************
//

always @ (posedge ddr_clk) begin
    if(state[3:0] == S_IDLE && (~rd_cmd_empty))
        ddr_rd_req <= 1'b1;
    else if(state[3:0] == S_IDLE)
        ddr_rd_req <= 1'b0;
    else if(ddr_rd_ack)//仲裁到读命令了，下发给仲裁的读命令清除了
        ddr_rd_req <= 1'b0;
    else
        ddr_rd_req <= ddr_rd_req;
end

//******************************************************************************

//******************************************************************************
//把1个长度为Len的命令，拆分为Len个长度为1的命令
always @ (posedge ddr_clk) begin
    if(state[3:0] == S_IDLE)
        ddr_cmd_cnt[8:0] <= 9'h0; 
    else if(state[3:0] == S_RD_CMD && ddr_rd_cmd_rdy)
        ddr_cmd_cnt[8:0] <= ddr_cmd_cnt[8:0] + 1;
    else
        ddr_cmd_cnt[8:0] <= ddr_cmd_cnt[8:0];
end

assign ddr_cmd_finish = ((ddr_cmd_cnt[8:0] == (rd_cmd_out[8:0] - 1)) && ddr_rd_cmd_rdy);
 
//******************************************************************************

//******************************************************************************
// 

//将起始地址按照128bit拆分，与命令匹配
always @ (posedge ddr_clk) begin
    if(state[3:0] == S_IDLE)
        ddr_rd_addr[31:0] <= 0;
    else if((state[3:0] == S_RD_REQ) && ddr_rd_ack)
        ddr_rd_addr[31:0] <= ({rd_cmd_out[33:9]} << 3); //先右移3，再左移3，为了把低3bit全部清0
    else if(ddr_rd_cmd_en && ddr_rd_cmd_rdy)
        ddr_rd_addr[31:0] <= ddr_rd_addr[31:0] + 8; //8*16=128bit
    else
        ddr_rd_addr[31:0] <= ddr_rd_addr[31:0];
end

//******************************************************************************

//******************************************************************************
//
always @ (posedge ddr_clk) begin
    ddr_rd_valid_r     <= ddr_rd_valid;
    ddr_rd_data_r[127:0] <= ddr_rd_data[127:0];
end

always @ (posedge ddr_clk) begin
    rd_data_en     <= ddr_rd_valid_r;
    rd_data[127:0] <= ddr_rd_data_r[127:0];
end

//******************************************************************************

//******************************************************************************
always @ (posedge ddr_clk) begin
    if(rst)
        rd_data_addr[7:0] <= 8'h00;
    else if(rd_cmd_sus_rd_en_r)
        rd_data_addr[7:0] <= rd_cmd_suspend[42:35];
    else if(rd_data_en)
        rd_data_addr[7:0] <= rd_data_addr[7:0] + 1;
    else
        rd_data_addr[7:0] <= rd_data_addr[7:0];
end

always @ (posedge ddr_clk) begin
    rd_data_addr[8] <= rd_cmd_suspend[34];
end
//******************************************************************************

//******************************************************************************
//

assign rd_cmd_sus_rd_en     = (rd_data_en && (rd_data_addr[7:0] - rd_cmd_suspend[42:35]== (rd_cmd_suspend[8:0] - 1)));

always @ (posedge ddr_clk) begin
    rd_cmd_sus_rd_en_r <= rd_cmd_sus_rd_en;
end

//assign rd_data_opt[13:0]    = rd_cmd_suspend[47:34];

//******************************************************************************

endmodule
