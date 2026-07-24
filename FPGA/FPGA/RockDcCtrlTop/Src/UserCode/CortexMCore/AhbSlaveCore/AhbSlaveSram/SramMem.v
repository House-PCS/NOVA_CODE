//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   SramMem.v
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
module SramMem
#(
    parameter                       A_WIDTH     = 10    
)
(
    input                           iSysClk             ,
    input                           iFpgaSramCs         ,
    input       [ 3:0]              iFpgaSramWrEn       ,
    input       [29:0]              iFpgaSramAddr       ,
    input       [31:0]              iFpgaSramWrData     ,
    output      [31:0]              oFpgaSramRdData     
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
//

// Sram_4096x8 U0_Sram_4096x8 
Sram_8192x8_0 U0_Sram_8192x8
(
    .clka                           (iSysClk                                ), 
    .wea                            ((iFpgaSramWrEn[0] && iFpgaSramCs)      ), 
    .addra                          (iFpgaSramAddr[(A_WIDTH - 1):0]         ), 
    .dina                           (iFpgaSramWrData[0*8+:8]                ), 
    .douta                          (oFpgaSramRdData[0*8+:8]                )
);

// Sram_4096x8 U1_Sram_4096x8 
Sram_8192x8_1 U1_Sram_8192x8
(
    .clka                           (iSysClk                                ), 
    .wea                            ((iFpgaSramWrEn[1] && iFpgaSramCs)      ), 
    .addra                          (iFpgaSramAddr[(A_WIDTH - 1):0]         ), 
    .dina                           (iFpgaSramWrData[1*8+:8]                ), 
    .douta                          (oFpgaSramRdData[1*8+:8]                )
);

// Sram_4096x8 U2_Sram_4096x8 
Sram_8192x8_2 U2_Sram_8192x8
(
    .clka                           (iSysClk                                ), 
    .wea                            ((iFpgaSramWrEn[2] && iFpgaSramCs)      ), 
    .addra                          (iFpgaSramAddr[(A_WIDTH - 1):0]         ), 
    .dina                           (iFpgaSramWrData[2*8+:8]                ), 
    .douta                          (oFpgaSramRdData[2*8+:8]                )
);

// Sram_4096x8 U3_Sram_4096x8 
Sram_8192x8_3 U3_Sram_8192x8
(
    .clka                           (iSysClk                                ), 
    .wea                            ((iFpgaSramWrEn[3] && iFpgaSramCs)      ), 
    .addra                          (iFpgaSramAddr[(A_WIDTH - 1):0]         ), 
    .dina                           (iFpgaSramWrData[3*8+:8]                ), 
    .douta                          (oFpgaSramRdData[3*8+:8]                )
);

//******************************************************************************

endmodule
