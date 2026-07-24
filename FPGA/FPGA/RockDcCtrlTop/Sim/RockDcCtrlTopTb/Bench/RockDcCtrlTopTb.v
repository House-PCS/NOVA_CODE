//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   RockDcCtrlTopTb.v
//Original Author   :   lvyb
//Creation Date     :   2024.09.14
/*Description       :   创建RockDcCtrlTopTb的仿真顶层。

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
module RockDcCtrlTopTb;

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    parameter                       PERIOD_SYS      = 10    ;
    parameter                       PERIOD_RMII     = 20    ;
    parameter                       RMII_CH         = 2     ;

//******************************************************************************

//******************************************************************************
//
    //开发板MAC地址 00-11-22-33-44-55
    parameter  BOARD_MAC = 48'h00_11_22_33_44_55;     
    //开发板IP地址 192.168.1.10     
    parameter  BOARD_IP  = {8'd192,8'd168,8'd1,8'd10};
    //目的MAC地址 ff_ff_ff_ff_ff_ff
    parameter  DES_MAC   = 48'hff_ff_ff_ff_ff_ff;
    //目的IP地址 192.168.1.102
    parameter  DES_IP    = {8'd192,8'd168,8'd1,8'd10};
    parameter  OP_CYCLE     = 300;


//******************************************************************************

//******************************************************************************
//
    reg                             rSysClk         = 1'b0  ;


    wire    [(RMII_CH - 1):0]       wRmiiRxDv               ;
    wire    [(RMII_CH*2 - 1):0]     wRmiiRxData             ;
    wire    [(RMII_CH - 1):0]       wRmiiTxEn               ;
    wire    [(RMII_CH*2 - 1):0]     wRmiiTxData             ;
    wire    [(RMII_CH - 1):0]       wRmiiClk                ;
    wire                            wSysRst                 ;
    wire                            wSysLed                 ;

//******************************************************************************

//******************************************************************************
//

//reg define
reg           gmii_clk  = 1'd0      ;    //时钟信号


reg           tx_start_en;
reg   [7:0]   tx_data    ;
reg   [15:0]  tx_byte_num;
reg   [47:0]  des_mac    ;
reg   [31:0]  des_ip     ;

reg   [3:0]   flow_cnt   ;
reg   [13:0]  delay_cnt  ;

wire          gmii_rx_clk; //GMII接收时钟
wire          gmii_rx_dv ; //GMII接收数据有效信号
wire  [7:0]   gmii_rxd   ; //GMII接收数据
wire          gmii_tx_clk; //GMII发送时钟
wire          gmii_tx_en ; //GMII发送数据使能信号
wire  [7:0]   gmii_txd   ; //GMII发送数据
              
wire          tx_done    ; 
wire          tx_req     ;


//******************************************************************************

//******************************************************************************
//

initial begin
    forever # (PERIOD_SYS/2)
        rSysClk = ~rSysClk;
end

initial begin
    forever # (PERIOD_RMII/2)
        gmii_clk = ~gmii_clk;
end

//******************************************************************************

//******************************************************************************
//

// task Task_SysRst;
//     begin
//         repeat(20) @ (negedge rSysClk);
//             rSysRst = 1'b0;
//     end
// endtask

//******************************************************************************

//******************************************************************************
//

// initial begin
//     Task_SysRst;

// end

//******************************************************************************

//******************************************************************************
//


// assign gmii_rx_clk = gmii_clk   ;
// assign gmii_tx_clk = gmii_clk   ;
// assign gmii_rx_dv  = gmii_tx_en ;
// assign gmii_rxd    = gmii_txd   ;

always @(posedge gmii_clk or posedge wSysRst) begin
    if(wSysRst) begin
        tx_start_en <= 1'b0;
        tx_data <= 8'h33;
        tx_byte_num <= 1'b0;
        des_mac <= 1'b0;
        des_ip <= 1'b0;
        delay_cnt <= 1'b0;
        flow_cnt <= 1'b0;
    end
    else begin
        case(flow_cnt)
            'd0 : flow_cnt <= flow_cnt + 1'b1;
            'd1 : begin
                tx_start_en <= 1'b1;  //拉高开始发送使能信号
                tx_byte_num <= 16'd10;//设置发送的字节数
                flow_cnt <= flow_cnt + 1'b1;
            end
            'd2 : begin 
                tx_start_en <= 1'b0;
                flow_cnt <= flow_cnt + 1'b1;
            end    
            'd3 : begin
                if(tx_req)
                    tx_data <= tx_data + 8'h11;
                if(tx_done) begin
                    flow_cnt <= flow_cnt + 1'b1;
                    tx_data <= 8'h33;
                end    
            end
            'd4 : begin
                delay_cnt <= delay_cnt + 1'b1;
                if(delay_cnt == OP_CYCLE - 1'b1)
                    flow_cnt <= flow_cnt + 1'b1;
            end
            'd5 : begin
                tx_start_en <= 1'b1;  //拉高开始发送使能信号
                tx_byte_num <= 16'd30;//设置发送的字节数
                flow_cnt <= flow_cnt + 1'b1;               
            end
            'd6 : begin 
                tx_start_en <= 1'b0;
                flow_cnt <= flow_cnt + 1'b1;
            end 
            'd7 : begin
                if(tx_req)
                    tx_data <= tx_data + 8'h11;
                if(tx_done) begin
                    flow_cnt <= flow_cnt + 1'b1;
                    tx_data <= 8'h33;
                end  
            end
            default:;
        endcase    
    end
end

//例化UDP模块
udp                             
#(
    .BOARD_MAC                      (BOARD_MAC          ),      //参数例化
    .BOARD_IP                       (BOARD_IP           ),
    .DES_MAC                        (DES_MAC            ),
    .DES_IP                         (DES_IP             )
)u_udp
(
    .rst_n                          (!wSysRst           ),
    
    .gmii_rx_clk                    (                   ),                      //gmii_clk     
    .gmii_rx_dv                     (                   ),                      //gmii_rx_dv   
    .gmii_rxd                       (                   ),                      //gmii_rxd     
    .gmii_tx_clk                    (gmii_clk           ),
    .gmii_tx_en                     (gmii_tx_en         ),
    .gmii_txd                       (gmii_txd           ),

    .rec_pkt_done                   (                   ),
    .rec_en                         (                   ),
    .rec_data                       (                   ),
    .rec_byte_num                   (                   ),
    .tx_start_en                    (tx_start_en        ),
    .tx_data                        (tx_data            ),
    .tx_byte_num                    (tx_byte_num        ),
    .des_mac                        (des_mac            ),
    .des_ip                         (des_ip             ),
    .tx_done                        (tx_done            ),
    .tx_req                         (tx_req             ) 
); 

//******************************************************************************

//******************************************************************************
//
    // wire                            wTbRmiiRxDv             ;
    // wire  [1:0]                     wTbRmiiRxData           ;

    wire                            wTbRmiiTxEn             ;
    wire  [1:0]                     wTbRmiiTxData           ;
    reg                 rTbRmiiRxDv     = 1'd0      ;
    reg     [1:0]       rTbRmiiRxData   = 2'd0      ;
    wire                wRmiiClkTb                  ;
RmiiDrvCore
#(
    .RMII_CH                        (1                                  )
)U_RmiiDrvCore
(
    .iSysClk                        (gmii_clk                           ),
    .iRmiiClk                       (gmii_clk                           ),
    .oRmiiClk                       (wRmiiClkTb                         ),
    
    .iRmiiRxDv                      (rTbRmiiRxDv                        ),               //CRS/CRS_DV
    .iRmiiRxData                    (rTbRmiiRxData[1:0]                 ),
    .oGmiiRxDv                      (gmii_rx_dv                         ),
    .oGmiiRxData                    (gmii_rxd[7:0]                      ),
    
    .iGmiiTxEn                      (gmii_tx_en                         ),
    .iGmiiTxData                    (gmii_txd[7:0]                      ),
    .oRmiiTxEn                      (wTbRmiiTxEn                        ),
    .oRmiiTxData                    (wTbRmiiTxData[1:0]                 )
);


// assign wTbRmiiRxDv          = wRmiiTxEn[0]          ;
// assign wTbRmiiRxData[1:0]   = wRmiiTxData[1:0]      ;

// assign wRmiiRxDv[0]         = wTbRmiiTxEn           ;
// assign wRmiiRxData[1:0]     = wTbRmiiTxData[1:0]    ;

// assign wTbRmiiRxDv          = wTbRmiiTxEn           ;
// assign wTbRmiiRxData[1:0]   = wTbRmiiTxData[1:0]    ;

always @ (posedge wRmiiClkTb) begin
    rTbRmiiRxDv <= wTbRmiiTxEn;
    rTbRmiiRxData[1:0] <= wTbRmiiTxData[1:0];
end


//******************************************************************************

//******************************************************************************
//


//******************************************************************************

//******************************************************************************
//

RockDcCtrlTop
#(
    .RMII_CH                        (RMII_CH                            )
)U_RockDcCtrlTop 
(
    .iSysClkIn                      (rSysClk                            ),
    .iRmiiRxDv                      (wRmiiRxDv[(RMII_CH - 1):0]         ),
    .iRmiiRxData                    (wRmiiRxData[(RMII_CH*2 - 1):0]     ),
    .oRmiiTxEn                      (wRmiiTxEn[(RMII_CH - 1):0]         ),
    .oRmiiTxData                    (wRmiiTxData[(RMII_CH*2 - 1):0]     ),
    .oRmiiClk                       (wRmiiClk[(RMII_CH - 1):0]          ),
    .oSysRst                        (wSysRst                            ),
    .oSysLed                        (wSysLed                            )
);


//******************************************************************************

endmodule
