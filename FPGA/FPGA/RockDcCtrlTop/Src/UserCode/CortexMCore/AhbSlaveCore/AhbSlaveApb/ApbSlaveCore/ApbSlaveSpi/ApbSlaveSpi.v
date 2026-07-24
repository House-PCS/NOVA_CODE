//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   ApbSlaveSpi.v
//Original Author   :   lvyb
//Creation Date     :   
/*Description       :

    Programmer's model
        0x00 W     reg_spi_mode[1:0]   SPI mode Register
        0x04 W     reg_freq_coe[7:0]   frequency division coefficient
        0x08 W     reg_cs[7:0]         chip select
        0x0C W     reg_txd[7:0]        Data Write Register
        0x10 R     reg_rxd[7:0]        Data Read Register 
        0x14 R     wire_tx_ready       transmit message

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
module ApbSlaveSpi
(
    //ApbPort
    input                           PCLK                                    ,   // Clock
    input                           PCLKG                                   ,   // Gated Clock
    input                           PRESETn                                 ,   // Reset
    
    input                           PSEL                                    ,   // Device select
    input       [11:2]              PADDR                                   ,   // Address
    input                           PENABLE                                 ,   // Transfer control
    input                           PWRITE                                  ,   // Write control
    input       [31:0]              PWDATA                                  ,   // Write data
    
    output      [31:0]              PRDATA                                  ,   // Read data
    output                          PREADY                                  ,   // Device ready
    output                          PSLVERR                                 ,   // Device error response
    
    //SpiPort
    output                          SPI_Clk                                 ,  
    output                          SPI_CS                                  ,   
    output                          SPI_MOSI                                , 
    input                           SPI_MISO                                  
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//Signals for read/write controls

    wire                            read_enable                             ;
    wire                            write_enable                            ;
    wire                            write_enable00                          ;
    wire                            write_enable04                          ;
    wire                            write_enable08                          ;
    wire                            write_enable0C                          ;
    wire        [7:0]               wire_rxd                                ;
    wire                            wire_rx_dv                              ;
    wire                            wire_tx_ready                           ;

// Signals for Control registers
    reg         [1:0]               reg_spi_mode                            ;
    reg         [7:0]               reg_freq_coe                            ;
    reg         [7:0]               reg_cs                                  ;
    reg         [7:0]               reg_txd                                 ;
    reg         [7:0]               reg_rxd                                 ;
    reg         [31:0]              read_mux_word                           ;
    reg                             reg_tx_dv                               ;

//******************************************************************************

//******************************************************************************
//

assign PREADY   = 1'b1; // Always ready
assign PSLVERR  = 1'b0; // Always okay

//******************************************************************************

//******************************************************************************
//

// Read and write control signals
assign read_enable      = (PSEL & (~PWRITE))                                ; // assert for whole APB read transfer
assign write_enable     = (PSEL & (~PENABLE) & PWRITE)                      ; // assert for 1st cycle of write transfer
assign write_enable00   = (write_enable & (PADDR[11:2] == 10'h000))         ;
assign write_enable04   = (write_enable & (PADDR[11:2] == 10'h001))         ;
assign write_enable08   = (write_enable & (PADDR[11:2] == 10'h002))         ;
assign write_enable0C   = (write_enable & (PADDR[11:2] == 10'h003))         ;

//******************************************************************************

//******************************************************************************
//
// Write operations
always @ (posedge PCLK or negedge PRESETn) begin
    if(~PRESETn)
        reg_spi_mode <= {2{1'b0}};
    else if(write_enable00)
        reg_spi_mode <= PWDATA[1:0];
end

always @ (posedge PCLK or negedge PRESETn) begin
    if(~PRESETn)
        reg_freq_coe <= {8{1'b0}};
    else if(write_enable04)
        reg_freq_coe <= PWDATA[7:0];
end

always @ (posedge PCLK or negedge PRESETn) begin
    if(~PRESETn)
        reg_cs <= {8{1'b0}};
    else if(write_enable08)
        reg_cs <= PWDATA[7:0];
end

assign SPI_CS = reg_cs[0];

always @ (posedge PCLK or negedge PRESETn) begin
    if(~PRESETn)
        reg_txd <= {8{1'b0}};
    else if(write_enable0C)
        reg_txd <= PWDATA[7:0];
end

//******************************************************************************

//******************************************************************************
//

//Read operation
always @ (posedge PCLK or negedge PRESETn) begin
    if(~PRESETn)
        reg_rxd <= {8{1'b0}};
    else
        reg_rxd <= wire_rxd;
end

always @ (PADDR or reg_rxd) begin
    case(PADDR[4:2])
        3'h4    :   read_mux_word   <=   {{24{1'b0}}, reg_rxd}              ;
        3'h5    :   read_mux_word   <=   {{31{1'b0}}, wire_tx_ready}        ;
        default :   read_mux_word   <=   {32{1'b0}}                         ;   // x propagation
    endcase
end

assign PRDATA[31:0] = (read_enable) ? read_mux_word : {32{1'b0}};


always @ (posedge PCLKG or negedge PRESETn) begin
    if(~PRESETn)
        reg_tx_dv <= 1'b0;
    else
        reg_tx_dv <= write_enable0C;
end

//******************************************************************************

//******************************************************************************
//

SpiMaster U_SpiMaster
(
    // Control/Data Signals,
    .i_Rst_L                        (PRESETn                                ),    
    .i_Clk                          (PCLK                                   ),        
    
    // TX (MOSI) Signals
    .i_TX_Byte                      (reg_txd                                ),  // Byte to transmit on MOSI
    .i_TX_DV                        (reg_tx_dv                              ),  // Data Valid Pulse with i_TX_Byte
    .o_TX_Ready                     (wire_tx_ready                          ),  // Transmit Ready for Byte
    
    // RX (MISO) Signals
    .o_RX_DV                        (wire_rx_dv                             ),  // Data Valid pulse (1 clock cycle)
    .o_RX_Byte                      (wire_rxd                               ),  // Byte received on MISO

    // SPI Interface
    .o_SPI_Clk                      (SPI_Clk                                ),
    .i_SPI_MISO                     (SPI_MISO                               ),
    .o_SPI_MOSI                     (SPI_MOSI                               ),

    // SPI Config
    .i_SPI_Mode                     (reg_spi_mode                           ),
    .i_Clk_Cnt                      (reg_freq_coe                           )
);

//******************************************************************************

endmodule
