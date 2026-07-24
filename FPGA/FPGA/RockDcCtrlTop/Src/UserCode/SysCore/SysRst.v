//------------------------------------------------------------------------------
//Project Name      :    
//Moudle Name       :   SysRst.v
//Original Author   :   lvyb
//Creation Date     :       
/*Description       :   

*/
//------------------------------------------------------------------------------
//Version           :   Rev 0.1
//Modifier          :   lvyb
//Modify Date       :     
/*Modify Record     :   系统复位默认时间50ms

*/
//------------------------------------------------------------------------------
//Version           :   Rev 0.2
//Modifier          :   lvyb
//Modify Date       :    
/*Modify Record     :   

*/  
//------------------------------------------------------------------------------
`timescale 1ns/1ns
module SysRst
#(
    parameter       CLK_FREQ    = 100//Mhz
)
(
    //global clock
    input           iClk         ,
    input           iPllLocked   ,
    input           iCnt1Us      ,
    //user interface
    output          oRstP        ,
    output          oRstN    
);

//-------------------------------Code Start Here----------------------------------
//******************************************************************************
`ifdef SIMULATION_EN
    localparam  RESET_TIME  = 20'd50;   //(Period_SysClk * RESET_TIME)
`else
    localparam  RESET_TIME  = 20'd50000 ;
`endif


//******************************************************************************

//******************************************************************************
//
reg                [   7: 0]              rRstCnt                   =8'd0 ;
reg                                       rRstP                     =1 ;
reg                                       rRstN                     =0 ;
reg                [   2: 0]              rPllLockedSync            =3'b111;


//******************************************************************************

//******************************************************************************
//Asynchronous Reset Synchronous Release
always @ (posedge iClk or negedge iPllLocked ) begin
    if(!iPllLocked)
      rPllLockedSync[2:0] <= 3'b111;
    else 
      rPllLockedSync[2:0] <= {rPllLockedSync[1:0],1'b0};
end


always @ (posedge iClk or posedge rPllLockedSync[2] ) begin
    if(rPllLockedSync[2])begin
        rRstCnt[7:0] <= 8'd0;
        rRstP <= 1'b1;
        rRstN <= 1'b0;
    end
    else  begin
        if(iCnt1Us) begin
            if(rRstCnt[7:0] != 8'hFF)begin
                rRstCnt[7:0] <= rRstCnt[7:0] + 19'd1;
                rRstP <= 1'b1; 
                rRstN <= 1'b0;           
            end
            else begin
                rRstCnt[7:0] <= rRstCnt[7:0];
                rRstP <= 1'b0;
                rRstN <= 1'b1;    
            end
        end
        else begin
            rRstCnt[7:0] <= rRstCnt[7:0];
            rRstP <= rRstP;
            rRstN <= rRstN;
        end
    end
end
//******************************************************************************

//******************************************************************************

assign oRstP = rRstP;
assign oRstN = rRstN;
//******************************************************************************

endmodule


