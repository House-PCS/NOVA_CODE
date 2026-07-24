//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   AdcSampleCore.v
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
module AdcSampleCoreV1_0
(
    //Global Clock
    input                           iSysClk                                 ,
    input                           iSysRst                                 ,
    input                           iVP                                     ,
    input                           iVN                                     ,
    input       [15:0]              iVauxP                                  ,
    input       [15:0]              iVauxN                                  ,

    output      [11:0]              oFpgaTemp                               ,
    output      [11:0]              oTempSamp                               ,
    output      [191:0]             oAdcData                                     
);

//-------------------------------Code Start Here--------------------------------


//******************************************************************************
//

    wire        [6:0]               wDrpAddr                                ;
    wire                            wDrpEn                                  ;
    wire        [15:0]              wDrpDataOut                             ;
    wire                            wDrpDataOutRdy                          ;
    wire        [4:0]               wAdcChannel                             ;
    wire                            wAdcConvertDone                         ;
    wire                            wAdcSampleDone                          ;
    reg         [11:0]              rFpgaTemp           = 12'd0             ;
    reg         [11:0]              rTempSamp           = 12'd0             ;
    reg         [(16*12 - 1):0]     rAdcData            = 191'd0            ;

//******************************************************************************

//******************************************************************************
//

assign wDrpAddr[6:0] = {2'd0, wAdcChannel[4:0]};
assign wDrpEn = wAdcConvertDone;

//******************************************************************************

//******************************************************************************
//
/*
    wire    [31:0]      probe0  ;
    wire    [31:0]      probe1  ;

assign probe0 = {8'd0, wDrpDataOut[15:0], wAdcChannel[4:0], wAdcConvertDone, wDrpDataOutRdy, wAdcSampleDone};
assign probe1 = 32'd0;


ila_0 U_ila_0 
(   
    .clk        (iSysClk            ), 
    .probe0     (probe0             ), 
    .probe1     (probe1             )
);
*/
//******************************************************************************

//******************************************************************************
//

XadcCore U_XadcCore
(
    .reset_in                       (iSysRst                                ),

    .dclk_in                        (iSysClk                                ),
    .di_in                          (16'd0                                  ),
    .dwe_in                         (1'd0                                   ),

    .daddr_in                       (wDrpAddr[6:0]                          ),
    .den_in                         (wDrpEn                                 ),
    .do_out                         (wDrpDataOut[15:0]                      ),
    .drdy_out                       (wDrpDataOutRdy                         ),


    .channel_out                    (wAdcChannel[4:0]                       ),
    .eoc_out                        (wAdcConvertDone                        ),

    .vp_in                          (iVP                                    ),
    .vn_in                          (iVN                                    ),
    .vauxp0                         (iVauxP[0]                              ),
    .vauxn0                         (iVauxN[0]                              ),
    .vauxp1                         (iVauxP[1]                              ),
    .vauxn1                         (iVauxN[1]                              ),
    .vauxp2                         (iVauxP[2]                              ),
    .vauxn2                         (iVauxN[2]                              ),
    .vauxp3                         (iVauxP[3]                              ),
    .vauxn3                         (iVauxN[3]                              ),
    .vauxp4                         (iVauxP[4]                              ),
    .vauxn4                         (iVauxN[4]                              ),
    .vauxp5                         (iVauxP[5]                              ),
    .vauxn5                         (iVauxN[5]                              ),
    .vauxp6                         (iVauxP[6]                              ),
    .vauxn6                         (iVauxN[6]                              ),
    .vauxp7                         (iVauxP[7]                              ),
    .vauxn7                         (iVauxN[7]                              ),
    .vauxp8                         (iVauxP[8]                              ),
    .vauxn8                         (iVauxN[8]                              ),
    .vauxp9                         (iVauxP[9]                              ),
    .vauxn9                         (iVauxN[9]                              ),
    .vauxp10                        (iVauxP[10]                             ),
    .vauxn10                        (iVauxN[10]                             ),
    .vauxp11                        (iVauxP[11]                             ),
    .vauxn11                        (iVauxN[11]                             ),
    .vauxp12                        (iVauxP[12]                             ),
    .vauxn12                        (iVauxN[12]                             ),
    .vauxp13                        (iVauxP[13]                             ),
    .vauxn13                        (iVauxN[13]                             ),
    .vauxp14                        (iVauxP[14]                             ),
    .vauxn14                        (iVauxN[14]                             ),
    .vauxp15                        (iVauxP[15]                             ),
    .vauxn15                        (iVauxN[15]                             ),

    .busy_out                       (                                       ),
    .eos_out                        (wAdcSampleDone                         ),
    .ot_out                         (                                       ),

    .vccaux_alarm_out               (                                       ),
    .vccint_alarm_out               (                                       ),
    .user_temp_alarm_out            (                                       ),
    .alarm_out                      (                                       ) 
);

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(wDrpDataOutRdy && (wAdcChannel[4:0] == 5'd0))
        rFpgaTemp[11:0] <= wDrpDataOut[15:4];
    else
        rFpgaTemp[11:0] <= rFpgaTemp[11:0] ;
end

//******************************************************************************

//******************************************************************************
//

always @ (posedge iSysClk) begin
    if(wDrpDataOutRdy && (wAdcChannel[4:0] == 5'd3))
        rTempSamp[11:0] <= wDrpDataOut[15:4];
    else
        rTempSamp[11:0] <= rTempSamp[11:0] ;
end

//******************************************************************************

//******************************************************************************
//

genvar i;

generate
    for(i = 0; i < 16; i = i + 1) begin

        always @ (posedge iSysClk) begin
            if(wDrpDataOutRdy && (wAdcChannel[4:0] == (5'd16 + i)))
                rAdcData[i*12+:12] <= wDrpDataOut[15:4];
            else
                rAdcData[i*12+:12] <= rAdcData[i*12+:12];
        end
    end
endgenerate

//******************************************************************************

//******************************************************************************
//

assign oFpgaTemp[11:0]              = rFpgaTemp[11:0]                       ;
assign oTempSamp[11:0]              = rTempSamp[11:0]                       ;
assign oAdcData[(16*12 - 1):0]      = rAdcData[(16*12 - 1):0]               ;

//******************************************************************************

endmodule
