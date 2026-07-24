//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   DdrWrOp.v
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
module DdrWrOp
(
    output  reg                     ddr_wr_req                              ,//给仲裁的读请求，不是给Mig的
    output                          ddr_wr_clr                              ,//给仲裁，读命令处理完了
    output      [2:0]               ddr_wr_cmd                              ,
    output                          ddr_wr_cmd_en                           ,
    output  reg [31:0]              ddr_wr_addr                             ,
    output      [127:0]             ddr_wr_data                             ,
    output      [15:0]              ddr_wr_mask                             ,
    output                          ddr_wr_en                               ,
    output                          ddr_wr_end                              ,

    output  reg [8:0]               wr_buf_addr                             ,//读用户侧Ram地址
    output                          wr_buf_addr_en                          ,//读用户侧的Ram使能
    
    input                           ddr_clk                                 ,
    input                           rst                                     ,
    input                           ddr_wr_ack                              ,//表示仲裁到来写命令
    input                           ddr_wr_cmd_rdy                          ,
    input                           ddr_wr_rdy                              ,//Mig允许写数据

    input       [127:0]             wr_buf_data                             ,
    input       [15:0]              wr_buf_mask                             ,
    input                           wr_burst_begin                          ,
    output                          wr_burst_rdy                            ,                       
    input       [8:0]               wr_len                                  ,
    input       [27:0]              wr_addr                                 ,
    input                           wr_clk                                  ,
    input                           wr_buf_sel                              ,
    input       [7:0]               wr_buf_base                              
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    localparam  [3:0]               S_IDLE              = 4'h0              ;
    localparam  [3:0]               S_WR_REQ            = 4'h1              ;
    localparam  [3:0]               S_WR_REQ_DATA       = 4'h2              ;
    localparam  [3:0]               S_WR_REQ_DATA_WAIT  = 4'h3              ;
    localparam  [3:0]               S_WR_CMD            = 4'h4              ;
    localparam  [3:0]               S_WR_DATA           = 4'h5              ;
    localparam  [3:0]               S_WR_DONE           = 4'h6              ;

//******************************************************************************

//******************************************************************************
//

    reg                  [  47: 0]      wr_cmd_pkt               =48'd0 ;
    wire                 [  47: 0]      wr_cmd_out                      ;
    wire                                wr_cmd_empty                    ;
    wire                                wr_cmd_alfull                   ;
    wire                                wr_cmd_rd_en                    ;
    reg                                 wr_cmd_wr_en             =1'd0  ;
    reg                  [   3: 0]      state                    =4'd0  ;
    reg                  [   3: 0]      state_n                  =4'd0  ;
    reg                                 wr_buf_rden              =1'd0  ;
    reg                                 wr_buf_rden_d            =1'd0  ;
    reg                  [   8: 0]      ddr_cmd_cnt              =9'd0  ;
    wire                                ddr_cmd_finish                  ; 
    reg                                 fetch_data_done          =1'd0  ;
    reg                                 fetch_data_done_d        =1'd0  ;
    reg                                 ddr_data_finish          =1'd0  ;

//******************************************************************************

//******************************************************************************

//******************************************************************************

//******************************************************************************
//
always @ (posedge wr_clk) begin   
    if(wr_burst_begin && (!wr_cmd_alfull))
        wr_cmd_wr_en <= 1'b1;
    else
        wr_cmd_wr_en <= 1'b0;                
end

always @ (posedge wr_clk) begin 
    wr_cmd_pkt[47:0]  <= {5'd0, wr_buf_base[7:0], wr_buf_sel, wr_addr[27:3], wr_len[8:0]};
end

Fifo48x16_48x16 U_WrOpCmdFifo
(
    .rst                            (rst                                    ),
    
    .wr_clk                         (wr_clk                                 ),
    .din                            (wr_cmd_pkt[47:0]                       ),
    .wr_en                          (wr_cmd_wr_en                           ),
    
    .rd_clk                         (ddr_clk                                ),
    .rd_en                          (wr_cmd_rd_en                           ),
    .dout                           (wr_cmd_out[47:0]                       ),
    
    .full                           (                                       ),
    .almost_full                    (                                       ),
    .empty                          (wr_cmd_empty                           ),
    .prog_full                      (wr_cmd_alfull                          ),
    .rd_data_count                  (                                       ),
    .wr_data_count                  (                                       )
);


assign  wr_burst_rdy = (!wr_cmd_alfull);
//******************************************************************************

//******************************************************************************
//

assign wr_cmd_rd_en                 = (state[3:0] == S_WR_DONE)             ;
assign ddr_wr_clr                   = (state[3:0] == S_WR_DONE)             ;
assign ddr_wr_cmd[2:0]              = 3'b000                                ;   //write operation
assign ddr_wr_cmd_en                = (state[3:0] == S_WR_CMD)              ;
assign ddr_wr_data[127:0]           = wr_buf_data[127:0]                    ;
assign ddr_wr_mask[15:0]            = wr_buf_mask[15:0]                     ;
assign ddr_wr_end                   = ddr_wr_en                             ;  
assign ddr_wr_en                    = (wr_buf_rden_d && (~ddr_data_finish) && ((state[3:0] == S_WR_CMD) || (state[3:0] == S_WR_DATA)));

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
        S_IDLE              :   begin
                                    if(~wr_cmd_empty)
                                        state_n[3:0] = S_WR_REQ;
                                    else
                                        state_n[3:0] = state_n[3:0];
                                end
        S_WR_REQ            :   begin
                                    if(ddr_wr_ack)//仲裁到了写命令
                                        state_n[3:0] = S_WR_REQ_DATA;
                                    else
                                        state_n[3:0] = state_n[3:0];
                                end
        S_WR_REQ_DATA       :   state_n[3:0] = S_WR_REQ_DATA_WAIT;
        S_WR_REQ_DATA_WAIT  :   state_n[3:0] = S_WR_CMD;
        S_WR_CMD            :   begin
                                    if(ddr_cmd_finish)
                                        state_n[3:0] = S_WR_DATA;
                                    else
                                        state_n[3:0] = state_n[3:0];
                                end
        S_WR_DATA           :   begin
                                    if(ddr_data_finish)
                                        state_n[3:0] = S_WR_DONE;
                                    else
                                        state_n[3:0] = state_n[3:0];
                                end
        S_WR_DONE           :   state_n[3:0] = S_IDLE;
        default             :   state_n[3:0] = S_IDLE;
    endcase
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge ddr_clk) begin
    if(state[3:0] == S_IDLE && (~wr_cmd_empty))
        ddr_wr_req <= 1'b1;
    else if(state[3:0] == S_IDLE)
        ddr_wr_req <= 1'b0;
    else if(ddr_wr_ack)
        ddr_wr_req <= 1'b0;
    else
        ddr_wr_req <= ddr_wr_req;
end

//******************************************************************************

//******************************************************************************
//
//拆成长度为1的命令
always @ (posedge ddr_clk) begin
    if(state[3:0] == S_IDLE)
        ddr_cmd_cnt[8:0] <= 9'h0; 
    else if((state[3:0] == S_WR_CMD) && ddr_wr_cmd_rdy)
        ddr_cmd_cnt[8:0] <= ddr_cmd_cnt[8:0] + 1'b1;
    else
        ddr_cmd_cnt[8:0] <= ddr_cmd_cnt[8:0];
end

assign ddr_cmd_finish = ((ddr_cmd_cnt[8:0] == (wr_cmd_out[8:0] - 1'b1)) && ddr_wr_cmd_rdy);

//******************************************************************************

//******************************************************************************
//
//拆地址
always @ (posedge ddr_clk) begin
    if(state[3:0] == S_IDLE)
        ddr_wr_addr[31:0] <= 0;
    else if((state[3:0] == S_WR_REQ) && ddr_wr_ack)
        ddr_wr_addr[31:0] <= ({wr_cmd_out[33:9]} << 3); //64 bit align
    else if(ddr_wr_cmd_en && ddr_wr_cmd_rdy)
        ddr_wr_addr[31:0] <= ddr_wr_addr[31:0] + 8;
    else
        ddr_wr_addr[31:0] <= ddr_wr_addr[31:0];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge ddr_clk) begin
    if((state[3:0] == S_IDLE) || (state[3:0] == S_WR_DONE))
        wr_buf_rden <= 1'b0;
    else if(state[3:0] == S_WR_REQ_DATA)
        wr_buf_rden <= 1'b1;
    else
        wr_buf_rden <= wr_buf_rden;
end

assign wr_buf_addr_en = (wr_buf_rden || (state[3:0] == S_WR_REQ_DATA)) && ddr_wr_rdy;

//******************************************************************************

//******************************************************************************
//读用户侧Ram地址

always @ (posedge ddr_clk) begin
    if(state[3:0] == S_IDLE)
        wr_buf_addr[7:0] <= wr_cmd_out[42:35]; 
    else if(state[3:0] == S_WR_DONE)
        wr_buf_addr[7:0] <= 0;
    else if(wr_buf_addr_en)
        wr_buf_addr[7:0] <= wr_buf_addr[7:0] + 1;
    else
        wr_buf_addr[7:0] <= wr_buf_addr[7:0];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge ddr_clk) begin
    wr_buf_addr[8] <= wr_cmd_out[34];
     
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge ddr_clk) begin
    if((state[3:0] == S_IDLE) || (state[3:0] == S_WR_DONE))
        wr_buf_rden_d <= 1'b0;
    else 
        wr_buf_rden_d <= wr_buf_rden;
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge ddr_clk) begin
    if((state[3:0] == S_IDLE) || (state[3:0] == S_WR_DONE))
        fetch_data_done <= 1'b0;
    else if(wr_buf_addr_en && ((wr_buf_addr[7:0] - wr_cmd_out[42:35]) == wr_cmd_out[8:0] - 1))
        fetch_data_done <= 1'b1;
    else
        fetch_data_done <= fetch_data_done;
end

always @ (posedge ddr_clk) begin
    if((state[3:0] == S_IDLE) || (state[3:0] == S_WR_DONE))
        fetch_data_done_d <= 1'b0;
    else if(ddr_wr_rdy)
        fetch_data_done_d <= fetch_data_done;
    else
        fetch_data_done_d <= fetch_data_done_d;
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge ddr_clk) begin
    if((state[3:0] == S_IDLE) || (state[3:0] == S_WR_DONE))
        ddr_data_finish <= 1'b0;
    else if(ddr_wr_rdy) 
        ddr_data_finish <= fetch_data_done_d;
    else
        ddr_data_finish <= ddr_data_finish;
end

//******************************************************************************

endmodule
