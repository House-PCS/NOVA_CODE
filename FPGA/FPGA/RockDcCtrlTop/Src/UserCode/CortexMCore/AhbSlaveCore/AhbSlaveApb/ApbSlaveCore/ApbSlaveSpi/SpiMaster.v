//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   SpiMaster.v
//Original Author   :   lvyb
//Creation Date     :   
/*Description       :
    Parameters:  SPI_MODE, can be 0, 1, 2, or 3.  See above.
                 Can be configured in one of 4 modes:
                 Mode | Clock Polarity (CPOL/CKP) | Clock Phase (CPHA)
                  0   |             0             |        0
                  1   |             0             |        1
                  2   |             1             |        0
                  3   |             1             |        1
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
module SpiMaster
(
   // Control/Data Signals,
   input                            i_Rst_L                                 ,   // FPGA Reset
   input                            i_Clk                                   ,   // FPGA Clock
   
   // TX (MOSI) Signals
   input        [7:0]               i_TX_Byte                               ,   // Byte to transmit on MOSI，上一层需要做的是将数据放进来
   input                            i_TX_DV                                 ,   // Data Valid Pulse with i_TX_Byte
   output reg                       o_TX_Ready                              ,   // Transmit Ready for next byte
   
   // RX (MISO) Signals
   output reg                       o_RX_DV                                 ,   // Data Valid pulse (1 clock cycle)
   output reg   [7:0]               o_RX_Byte                               ,   // Byte received on MISO

   // SPI Interface
   output reg                       o_SPI_Clk                               ,
   input                            i_SPI_MISO                              ,
   output reg                       o_SPI_MOSI                              ,

   // SPI Config
   input        [1:0]               i_SPI_Mode                              ,
   input        [7:0]               i_Clk_Cnt
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    // SPI Interface (All Runs at SPI Clock Domain)
    wire                            w_CPOL                                  ;   // Clock polarity
    wire                            w_CPHA                                  ;   // Clock phase

    reg         [1:0]               SPI_MODE                                ;
    reg         [7:0]               Clk_Cnt                                 ;
    reg         [7:0]               r_SPI_Clk_Count                         ;
    reg                             r_SPI_Clk                               ;
    reg         [4:0]               r_SPI_Clk_Edges                         ;
    reg                             r_Leading_Edge                          ;
    reg                             r_Trailing_Edge                         ;
    reg                             r_TX_DV                                 ;
    reg         [7:0]               r_TX_Byte                               ;

    reg         [2:0]               r_RX_Bit_Count                          ;
    reg         [2:0]               r_TX_Bit_Count                          ;

//******************************************************************************

//******************************************************************************
//

always @ (posedge i_Clk or negedge i_Rst_L) begin
    if(~i_Rst_L) begin
        SPI_MODE <= 0;
        Clk_Cnt  <= 8'h2;
    end
    else begin
        SPI_MODE <= i_SPI_Mode;
        Clk_Cnt  <= i_Clk_Cnt;
    end
end

//******************************************************************************

//******************************************************************************
//

// w_CPOL与w_CPHA的值由SPI选择的工作模式决定
// CPOL: Clock Polarity
// CPOL=0 means clock idles at 0, leading edge is rising edge.
// CPOL=1 means clock idles at 1, leading edge is falling edge.
assign w_CPOL  = (SPI_MODE == 2) | (SPI_MODE == 3);

// CPHA: Clock Phase
// CPHA=0 means the "out" side changes the data on trailing edge of clock
//              the "in" side captures data on leading edge of clock
// CPHA=1 means the "out" side changes the data on leading edge of clock
//              the "in" side captures data on the trailing edge of clock

assign w_CPHA  = (SPI_MODE == 1) | (SPI_MODE == 3);

//******************************************************************************

//******************************************************************************
//
// Purpose: Generate SPI Clock correct number of times when DV pulse comes
// 目的：当DV脉冲出现时，生成正确次数的SPI时钟
always @ (posedge i_Clk /*or negedge i_Rst_L*/) begin
    if(~i_Rst_L) begin
        o_TX_Ready      <= 1'b0;
        r_SPI_Clk_Edges <= 0;
        r_Leading_Edge  <= 1'b0;
        r_Trailing_Edge <= 1'b0;
        r_SPI_Clk       <= w_CPOL; // assign default state to idle state
        r_SPI_Clk_Count <= 0;
    end
    else begin
        // Default assignments
        r_Leading_Edge  <= 1'b0;
        r_Trailing_Edge <= 1'b0;

        if(i_TX_DV) begin
            o_TX_Ready      <= 1'b0;
            r_SPI_Clk_Edges <= 16;  // Total # edges in one byte ALWAYS 16
        end
        else if(r_SPI_Clk_Edges > 0) begin
            o_TX_Ready <= 1'b0;
            //if(r_SPI_Clk_Count == CLKS_PER_HALF_BIT*2-1)
            if(r_SPI_Clk_Count == Clk_Cnt) begin
                r_SPI_Clk_Edges <= r_SPI_Clk_Edges - 1'b1;
                r_Trailing_Edge <= 1'b1;
                r_SPI_Clk_Count <= 0;
                r_SPI_Clk       <= ~r_SPI_Clk;
            end
            //else if (r_SPI_Clk_Count == CLKS_PER_HALF_BIT-1)
            else if(r_SPI_Clk_Count == (Clk_Cnt >> 1)) begin
                r_SPI_Clk_Edges <= r_SPI_Clk_Edges - 1'b1;
                r_Leading_Edge  <= 1'b1;
                r_SPI_Clk_Count <= r_SPI_Clk_Count + 1'b1;
                r_SPI_Clk       <= ~r_SPI_Clk;
            end
            else begin
                r_SPI_Clk_Count <= r_SPI_Clk_Count + 1'b1;
            end
        end  
        else begin
            o_TX_Ready <= 1'b1;
        end
    end
end

//******************************************************************************

//******************************************************************************
//

// Purpose: Register i_TX_Byte when Data Valid is pulsed.
// 当数据有效时，注册i_TX_Byte字节
// Keeps local storage of byte in case higher level module changes the data
// 保持字节的本地存储，以防更高级别的模块更改数据
always @ (posedge i_Clk or negedge i_Rst_L) begin
    if(~i_Rst_L) begin
        r_TX_Byte <= 8'h00;
        r_TX_DV   <= 1'b0;
    end
    else begin
        r_TX_DV <= i_TX_DV; // 1 clock cycle delay
        if(i_TX_DV) begin
            r_TX_Byte <= i_TX_Byte;
        end
    end
end


//******************************************************************************

//******************************************************************************
//
// Purpose: Generate MOSI data
// Works with both CPHA=0 and CPHA=1

always @ (posedge i_Clk /*or negedge i_Rst_L*/) begin
    if(~i_Rst_L) begin
        o_SPI_MOSI     <= 1'b0;
        r_TX_Bit_Count <= 3'b111; // send MSb first
    end
    else begin
        // If ready is high, reset bit counts to default
        if(o_TX_Ready) begin
            r_TX_Bit_Count <= 3'b111;
        end
        // Catch the case where we start transaction and CPHA = 0
        else if(r_TX_DV & ~w_CPHA) begin
            o_SPI_MOSI     <= r_TX_Byte[3'b111];
            r_TX_Bit_Count <= 3'b110;
        end
        else if((r_Leading_Edge & w_CPHA) | (r_Trailing_Edge & (~w_CPHA))) begin
            r_TX_Bit_Count <= r_TX_Bit_Count - 1'b1;
            o_SPI_MOSI     <= r_TX_Byte[r_TX_Bit_Count];
        end
    end
end

//******************************************************************************

//******************************************************************************
//
// Purpose: Read in MISO data.

always @ (posedge i_Clk /*or negedge i_Rst_L*/) begin
    if(~i_Rst_L) begin
        o_RX_Byte      <= 8'h00;
        o_RX_DV        <= 1'b0;
        r_RX_Bit_Count <= 3'b111;
    end
    else begin
        // Default Assignments
        o_RX_DV   <= 1'b0;
        
        // Check if ready is high, if so reset bit count to default
        if(o_TX_Ready)
            r_RX_Bit_Count <= 3'b111;
        else if((r_Leading_Edge & (~w_CPHA)) | (r_Trailing_Edge & w_CPHA)) begin
            o_RX_Byte[r_RX_Bit_Count] <= i_SPI_MISO;  // Sample data
            r_RX_Bit_Count            <= r_RX_Bit_Count - 1'b1;
            if(r_RX_Bit_Count == 3'b000) begin
                o_RX_DV   <= 1'b1;   // Byte done, pulse Data Valid
            end
        end
    end
end
  

//******************************************************************************

//******************************************************************************
//
// Purpose: Add clock delay to signals for alignment.
// 为校准信号添加时钟延时

always @ (posedge i_Clk /*or negedge i_Rst_L*/)begin
    if(~i_Rst_L)
        o_SPI_Clk  <= w_CPOL;
    else
        o_SPI_Clk <= w_CPOL ? (!r_SPI_Clk) : r_SPI_Clk;    // o_spi_clk与r_spi_clk保持同步
end

//******************************************************************************

endmodule
