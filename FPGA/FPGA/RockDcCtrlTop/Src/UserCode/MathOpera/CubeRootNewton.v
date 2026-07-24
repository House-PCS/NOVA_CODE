`timescale 1ns / 1ps
//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   CubeRootNewton.v
//Original Author   :   gou xu hui 
//Creation Date     :   
/*Description       :   牛顿迭代法求解 f(x)=x^(1/3)-a=0
                    :   f(x)=0
                    :   f(xn)+f'(xn)*(x-xn)=0
                    :   x = xn - f(xn)/f'(xn)
                    :   带入上述方程
                    ：  x = (2*xn + a/xn^2)/3
                    :   xn1 = (2*xn + a/xn^2)/3
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
module CubeRootNewton#(
    parameter    WIDTH = 16,
    parameter    ITERATIONS = 20
)
(   input                       iSysClk     ,
    input                       iSysRst     ,
    input   signed [WIDTH-1:0]  iData       ,    //-32767 ~ 32767
    input                       iDataValid  ,
    output  signed [WIDTH-1:0]  oData       ,
    output                      oDataValid ,
    output                      oBusy	
);

//************************************************************************************
//                                 <functions>
//************************************************************************************
function integer clogb2 ( input integer depth ) ;
begin
    for ( clogb2 = 0 ; depth > 0 ; clogb2 = clogb2 + 1 ) begin 
        depth = depth >> 1 ;
    end
end
endfunction
//------------------------------------------------------------------


localparam IDLE = 3'd0, S1 = 3'd1, S2 = 3'd2, S3 = 3'd3, S4 = 3'd4 ;
localparam CNT_ITERATIONS_BW = clogb2(ITERATIONS);

reg          [                    2:0]  CurrentState  = 3'd0    ;
reg          [                    2:0]  NextState     = 3'd0    ; 
reg  signed  [            WIDTH - 1:0]  a             =   'sd0  ;
reg  signed  [            WIDTH - 2:0]  Xn            =   'sd0  ;
reg          [CNT_ITERATIONS_BW - 1:0]  CntIteraction = 'd0     ;
wire signed  [              WIDTH-1:0]  XnMulti2                ;
wire signed  [      (WIDTH - 1)*2-1:0]  XnSquare                ;
wire                                    Div1Done                ;
wire signed  [        (WIDTH - 2)*2:0]  Div1Quot1               ;
reg  signed  [        (WIDTH - 2)*2:0]  Div2Dividend            ;
wire signed  [          (WIDTH - 1):0]  Div1Quot2               ;
wire                                    Div2Done                ;
reg  signed  [            WIDTH - 1:0]  Data          = 'sd0    ;
reg                                     DataValid     = 'd0     ;
reg                                     Busy          = 'd0     ;
reg          [                    1:0]  rS1Delay      = 'd0     ;
			 
always@(posedge iSysClk)begin
  if(iSysRst)
    CurrentState <= IDLE;
  else
    CurrentState <= NextState;    
end
// xn1 = (2*xn + a/xn^2)/3
always@(*)begin
  case(CurrentState)
    IDLE: begin
	  if(iDataValid)
	    NextState <= S1;
   	  else
	    NextState <= IDLE;          	  
    end	 
    S1: begin  //计算 2xn和x^2,3拍后到S2
	  if(rS1Delay[1])
	    NextState <= S2;
	  else
	    NextState <= S1;
	end
	S2: begin //计算 a/Xn^2 + 2Xn
	  if(Div1Done)
	    NextState <= S3;
	  else
	    NextState <= S2;
	end
	S3:begin //计算（a/Xn^2 + 2Xn＿/3
	  if(Div2Done)
	    NextState <= S4;
	  else
	    NextState <= S3;
	end
	S4:begin
	  if(CntIteraction == ITERATIONS)
	    NextState <= IDLE;
	  else
	    NextState <= S1;
	end
  endcase
end

always@(posedge iSysClk)begin
  if(CurrentState == IDLE)
    a[WIDTH-1:0]  <= iData[WIDTH-1:0]; //16bit
  else
    a[WIDTH-1:0]  <= a[WIDTH-1:0];   
end

always@(posedge iSysClk)begin
  if(CurrentState == IDLE)
	Xn[WIDTH-2:0] <= iData >>> 1;    //15bit
  else if(CurrentState == S4)
    Xn[WIDTH-2:0] <= Div1Quot2;
  else
    Xn[WIDTH-2:0] <= Xn;    
end

always@(posedge iSysClk)begin //循环次数级数，每IDLE调到S1时增势 //这里还有问题，没有多次循环上
  if(CurrentState == IDLE)
    CntIteraction <= 'd0;
  else if(Div2Done)begin
    if(CntIteraction == ITERATIONS)
      CntIteraction <= CntIteraction;
    else
      CntIteraction <= CntIteraction + 1'B1;
  end
  else 
    CntIteraction <= CntIteraction;
end

MultAxB_XilinxV0101   #     //Multiplier Delay 3 Clk
(
    .SYNTHESIS_MODE_SET (1                 ), // synthrsis mode set  1: DSP 0: LUT
    .AWIDTH             (WIDTH - 1         ), // Width of multiplier's 1st input
    .BWIDTH             (WIDTH - 1         ), // Width of multiplier's 2nd input
    .PWIDTH             ((WIDTH - 1)*2  ), // Output Width
    .TC                 (1                 )  // 1'b0 : unsigned 1'b1 : signed 
)
MultAxB_XilinxV0101(
    .InClk  (iSysClk           ), // Clock

    .InA    (Xn                ), // Multiplier
    .InB    (Xn                ), // Mutiplier input, max data bw 18 

    .OutP   (XnSquare         )  // Output,max data bw 48  //29bit
);

always@(posedge iSysClk)begin
  rS1Delay[1:0] <= {rS1Delay[0],CurrentState == S1};
end

assign XnMulti2 = Xn <<< 1;
wire  Div1Star;
reg    [2:0] rCurrentState;
always@(posedge iSysClk)begin
  rCurrentState <= CurrentState;
end

assign Div1Star = (CurrentState == S2) && (rCurrentState == S1) ;

SignedDivision  #(
   . WITH1 ((WIDTH - 2)*2+1    )        ,     // 被除数位宿  29bit
   . WITH2 ((WIDTH - 2)*2+1    )              // 除数位宽  29bit
)SignedDivision1 (
    .iSysClk     (iSysClk    )    ,
    .iSysRst     (iSysRst    )    ,
    .iStart      (Div1Star   )    ,
    .iDividend   ({ {(WIDTH-3){a[WIDTH-1]}},a[WIDTH-1:0]}  )    ,
    .iDivisor    (XnSquare   )  ,
    .oQuotient   (Div1Quot1  )       , //28bit
    .oRemainder  (           )    ,
    .oDone       (Div1Done)    ,
    .oBusy       ()    
);

always@(posedge iSysClk)begin
  if(Div1Done)
    Div2Dividend  <= Div1Quot1 + {{(WIDTH-3){XnMulti2[WIDTH-1]}},XnMulti2[WIDTH-1:0]};
  else
    Div2Dividend <= Div2Dividend;
end

wire   Div2Star;
assign Div2Star = (CurrentState == S3) && (rCurrentState == S2) ;

SignedDivision   #(
   . WITH1 ((WIDTH - 2)*2+1 )    ,     // 被除数位宿 30bit
   . WITH2 (               3)          // 除数位宽
) SignedDivision2(
    .iSysClk     (iSysClk     )    ,
    .iSysRst     (iSysRst     )    ,
    .iStart      (Div2Star    )    ,
    .iDividend   (Div2Dividend)    ,
    .iDivisor    (3'sd3       )    ,
    .oQuotient   (Div1Quot2   )    , //只取16bit
    .oRemainder  (            )    ,
    .oDone       (Div2Done    )    ,
    .oBusy       (            )    
);


always@(posedge iSysClk)begin
  if( Div2Done && (CntIteraction == (ITERATIONS-1)) )begin
    Data       <=  Div1Quot2[WIDTH-1:0];
	  DataValid  <=  Div2Done ;
  end
  else begin
    Data       <=  Data;
	  DataValid  <=  1'b0     ;   
  end
end

always@(posedge iSysClk)begin
  Busy <= (CurrentState != IDLE);
end

assign oData        = Data        ;
assign oDataValid   = DataValid   ;
assign oBusy        = Busy        ;

endmodule