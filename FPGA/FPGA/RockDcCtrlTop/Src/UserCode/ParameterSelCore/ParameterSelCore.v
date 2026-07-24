//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   ParameterSelCore.v
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
module ParameterSelCore
(
    //Global Clock
    input                           iEthRamBusClk               ,
    input       [31:0]              iEthRamBusOpAddr            ,
    output      [7:0]               oEthRamBusRdData            ,  
    //User Interface
    input       [7:0]               iFpgaParamRdData            ,
    input       [15:0]              iCpuParamRdData              
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

    localparam  RAM_BASE_ADDR       =   {
                                            {16'h0200},                         //CpuParameter Cpu Read Only
                                            {16'h0100},                         //CpuParameter Fpga Read Only
                                            {16'h0000}                          //FpgaParameter
                                        };

//******************************************************************************

//******************************************************************************
//

    reg         [15:0]              rEthRamBusOpAddrBuf [1:0]               ;
    reg         [7:0]               rEthRamBusRdData    = 8'd0              ;

//******************************************************************************

//******************************************************************************
//

always @ (posedge iEthRamBusClk) begin
    rEthRamBusOpAddrBuf[0][15:0] <= iEthRamBusOpAddr[31:16];
    rEthRamBusOpAddrBuf[1][15:0] <= rEthRamBusOpAddrBuf[0][15:0];
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iEthRamBusClk) begin
    case(rEthRamBusOpAddrBuf[1][15:0])
        RAM_BASE_ADDR[0*16+:16]     :   rEthRamBusRdData[7:0] <= iFpgaParamRdData[7:0];
        RAM_BASE_ADDR[1*16+:16]     :   rEthRamBusRdData[7:0] <= iCpuParamRdData[0*8+:8];
        RAM_BASE_ADDR[2*16+:16]     :   rEthRamBusRdData[7:0] <= iCpuParamRdData[1*8+:8];
        default                     :   rEthRamBusRdData[7:0] <= 8'd0;
    endcase
end

//******************************************************************************

//******************************************************************************
//
assign oEthRamBusRdData[7:0] = rEthRamBusRdData[7:0];

//******************************************************************************

endmodule
