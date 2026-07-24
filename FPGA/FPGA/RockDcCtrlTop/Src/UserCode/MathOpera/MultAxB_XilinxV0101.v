//FILE_HEADER-----------------------------------------------------------------------------------
//NOVAstar  Copyright  (c) NOVAstar  Company Confidential
//----------------------------------------------------------------------------------------------
//FILE NAME      MultAxB_XilinxV0101.v
//DEPARTMENT     Hardware DPT
//AUTHOR         Li Peng
//----------------------------------------------------------------------------------------------
//RELEASE HISTORY
//VERSION         DATE            AUTHOR       DESCRIPTION
//1.0          2021-08-09        Li  Peng       initial   
//1.0          2024-12-06        Ren Chao       signed Mode not use add signedbit    
//
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//KEYWORDS : 
//
//----------------------------------------------------------------------------------------------
//PURPOSE :  A * B  
//----------------------------------------------------------------------------------------------
//PARAMETERS    
//PARAM  NAME         RANGE        :DESCRIPTION         :DEFAULT    :UNITS
//
//----------------------------------------------------------------------------------------------
//REUSE ISSUES       :  null
//Reset Strategy     :  null
//Clock  Domains     :  null
//Critical Timing    :  null
//Test   Features    :  null
//Asynchronous I/F   :  null
//Scan Methodology   :  null
//Instaniations      :  null
//Synthesizable      :  null
//Other              :  null
//END_HEADER------------------------------------------------------------------------------------

`timescale 1ns / 1ps 

module MultAxB_XilinxV0101 #
(
    parameter SYNTHESIS_MODE_SET = 1  , // synthrsis mode set  1: DSP 0: LUT
    parameter AWIDTH             = 18 , // Width of multiplier's 1st input
    parameter BWIDTH             = 18 , // Width of multiplier's 2nd input
    parameter PWIDTH             = 36 , // Output Width
    parameter TC                 = 0    // 1'b0 : unsigned 1'b1 : signed 
)
(
    input InClk , // Clock

    input [ AWIDTH - 1 : 0 ] InA  , // Multiplier input, max data bw 25
    input [ BWIDTH - 1 : 0 ] InB  , // Mutiplier input, max data bw 18 
    
    output [ PWIDTH - 1 : 0 ] OutP // Output,max data bw 48
);
//******************************************************************************
//                                <functions>
//******************************************************************************

//******************************************************************************
//                                <localparams>
//******************************************************************************
localparam SYNTHESIS_MODE = ( SYNTHESIS_MODE_SET ) ? "yes" : "no" ; // synthrsis mode set  1: DSP 0: LUT

//******************************************************************************
//                               <internal signals>
//******************************************************************************
reg signed [ AWIDTH - 1 : 0 ] RInA_S = { ( AWIDTH ) { 1'b0 } } ; // rigester InA   
reg signed [ BWIDTH - 1 : 0 ] RInB_S = { ( BWIDTH ) { 1'b0 } } ; // rigester InB  

reg [ AWIDTH  : 0 ] RInA_U = { ( AWIDTH + 1 ) { 1'b0 } } ; // rigester InA   
reg [ BWIDTH  : 0 ] RInB_U = { ( BWIDTH + 1 ) { 1'b0 } } ; // rigester InB  

(* use_dsp=SYNTHESIS_MODE *) reg signed [ PWIDTH + 2 - 1 : 0 ] RInP = { ( PWIDTH + 2 ) { 1'b0 } } ; // rigester InP   

reg [ PWIDTH - 1 : 0 ] RInPD1 = { ( PWIDTH ) { 1'b0 } } ; // rigester InPD1  

//******************************************************************************
//                               <module body>
//******************************************************************************

// rigester A B C 
generate
    if ( TC == 0 ) begin // unsigned
        always @ ( posedge InClk ) begin
            RInA_U <= { 1'b0 , InA } ;
            RInB_U <= { 1'b0 , InB } ;
        end
    end
    else begin  // signed
        always @ ( posedge InClk ) begin       
            RInA_S <= InA ;
            RInB_S <= InB ;
        end
    end
endgenerate

// A*B 
generate
    if ( TC == 0 ) begin // unsigned
        always @ ( posedge InClk ) begin
            RInP <= RInA_U * RInB_U ;
        end
    end
    else begin // signed
        always @ ( posedge InClk ) begin   
            RInP <= RInA_S * RInB_S ;      
        end
    end
endgenerate

// P + C
always @ ( posedge InClk )  begin
    RInPD1 <= RInP [ PWIDTH - 1 : 0 ] ;
end

assign OutP = RInPD1;

endmodule
