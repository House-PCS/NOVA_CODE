//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AhbLiteDecoder.v
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
module AhbLiteDecoder
(
    input       [31:0]              iAhbAddr                                ,
    output      [15:0]              oAhbSelMap                              
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//
    wire        [15:0]              wAhbSelMap                              ;

//******************************************************************************

//******************************************************************************
//

assign wAhbSelMap[0] = (iAhbAddr[31:28] == 4'h0);       //Rom+Rom               
                                                                            //0x0000_0000 --> 0x0000_4000   --> 32KB
assign wAhbSelMap[1] = (iAhbAddr[31:28] == 4'h2);       //DDR               
                                                                            //0x2000_0000 --> 0x2fff_ffff   --> 2Gb = 256MB
assign wAhbSelMap[2] = (iAhbAddr[31:16] == 16'h3000);   //AhbSlaveUdp       
                                                                //UdpRxDataCache
                                                                            //0x3000_0000 --> 0x3000_07ff   --> 2KB
                                                                //UdpTxDataCache
                                                                            //0x3000_1000 --> 0x3000_17ff   --> 2KB

assign wAhbSelMap[3] = (iAhbAddr[31:16] == 16'h3001);   //BmsParamCache     (映射FPGA地址32'h0100_0000)
                                                                            //0x3001_0000 --> 0x3001_07ff   --> 2KB
assign wAhbSelMap[4] = (iAhbAddr[31:16] == 16'h3002);   //FpgaParamCache    (映射FPGA地址32'h0200_0000)
                                                                            //0x3002_0000 --> 0x3002_07ff   --> 2KB

assign wAhbSelMap[5] = (iAhbAddr[31:16] == 16'h4000);   //AhbBaseAddr
                                                                //AhbSlaveGpio
                                                                            //0x4000_0000 --> 0x4000_001f   --> 32B


assign wAhbSelMap[6] = (iAhbAddr[31:16] == 16'h4100);   //ApbBaseAddr       
                                                                //ApbSlaveTimer
                                                                            //0x4100_0000 --> 0x4100_0fff   --> Timer
                                                                //ApbSlaveUart
                                                                            //0x4100_1000 --> 0x4100_1fff   --> Uart

assign wAhbSelMap[15] = (wAhbSelMap[6:0] == 7'd0);

//******************************************************************************

//******************************************************************************
//

assign wAhbSelMap[14:7] = 8'd0;

//******************************************************************************

//******************************************************************************
//

assign oAhbSelMap[15:0] = wAhbSelMap[15:0];

//******************************************************************************

endmodule
