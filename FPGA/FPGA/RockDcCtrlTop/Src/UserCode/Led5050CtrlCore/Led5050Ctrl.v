//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   LedLampStripCtrlCore.v
//Original Author   :   xcy Nova003107
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
module Led5050Ctrl#(
    parameter           CLK_FREQ  = 100       ,
    parameter           LED_NUM   = 16'd21
)(
    input               iSysClk                 ,
    input   [7:0]       iRValue                 ,
    input   [7:0]       iGValue                 ,
    input   [7:0]       iBValue                 ,
    input               iVsync                  ,
    input   [7:0]       iLedMode                ,
    input               iNoEthNet               ,//无网口通信时，亮一个灯，为了快速定位FPGA是否加载成功
    input   [4:0]       iSocUniformization      ,//SOC 值 0-100% 对应0-21个灯珠点亮的灯珠个数
    input   [15:0]      iAlpha                  ,    
    input               iDir                    ,
    output  [1:0]       oRGBOut 
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
// 内部参数定义
localparam  T1H                             =   12'd70      ;  //Return-to-zero code "1" high level duration, unit 10ns (system clock 125M hours)
localparam  T0H                             =   12'd30      ;  //Return to zero code "0" high level duration, in 10ns   (125M hours on system clock)
localparam  TDclk                           =   12'd100     ; 

localparam  LED_BIT_DEEP                    =       6'd8    ;
localparam  SHIFT_UNIT_NUM_PER_SUB_FIELD    =   LED_NUM*3   ;

//******************************************************************************


//******************************************************************************
//
reg     [7:0]       rLedRValue          = 8'd0  ;
reg     [7:0]       rLedGValue          = 8'd0  ;
reg     [7:0]       rLedBValue          = 8'd0  ;

reg     [7:0]       rLedBValueBuf       = 8'h00 ;
reg     [7:0]       rLedRValueBuf       = 8'h00 ;
reg     [7:0]       rLedGValueBuf       = 8'h00 ;

reg     [7:0]       rLedBitCnt          = 8'd0  ;
reg     [23:0]      rRGBbitDataPre      = 24'd0 ;
reg     [23:0]      rRGBbitData         = 24'd0 ;
reg     [7:0]       rLedNumCnt          = 8'd0  ;

wire                wLedBitCntEnd               ;
wire                wRGBbitData                 ;
wire                wImageRamRdEn               ;
wire    [23:0]      wLedBValueBuf;
wire    [23:0]      wLedRValueBuf;
wire    [23:0]      wLedGValueBuf;
//******************************************************************************


//******************************************************************************
//

always @(posedge iSysClk ) begin
    rLedRValue[7:0] <= iRValue[7:0];
    rLedGValue[7:0] <= iGValue[7:0];
    rLedBValue[7:0] <= iBValue[7:0];
end

assign  wLedRValueBuf[23:0] = (rLedRValue * iAlpha) ;
assign  wLedGValueBuf[23:0] = (rLedGValue * iAlpha) ;
assign  wLedBValueBuf[23:0] = (rLedBValue * iAlpha) ;

always @(posedge iSysClk ) begin
   rLedRValueBuf <=  wLedRValueBuf[23:16] ;
   rLedGValueBuf <=  wLedGValueBuf[23:16] ;
   rLedBValueBuf <=  wLedBValueBuf[23:16] ;
end
//******************************************************************************

always @(posedge iSysClk ) begin
    if(iNoEthNet)begin
        if(rLedNumCnt[7:0]<(LED_NUM-'d1))
            rRGBbitDataPre[23:0] <= {24{1'b0}};
        else
            rRGBbitDataPre[23:0] <= {8'h80,8'h80,8'h80};
    end
    else begin 
        case (iLedMode)
            8'h00:begin//关闭
                rRGBbitDataPre[23:0] <= {24{1'b0}};
                end 
            8'h01:begin//常亮
                rRGBbitDataPre[23:0] <= {rLedGValue,rLedRValue,rLedBValue};
            end
            8'h02:begin//闪烁
                if(iDir)
                    rRGBbitDataPre[23:0] <= {rLedGValue,rLedRValue,rLedBValue};
                else
                    rRGBbitDataPre[23:0] <= {24{1'b0}};
            end
            8'h03:begin//呼吸
                if (iDir) begin// 上升阶段
                    rRGBbitDataPre[7:0]     <= rLedBValueBuf[7:0];
                    rRGBbitDataPre[15:8]    <= rLedRValueBuf[7:0];
                    rRGBbitDataPre[23:16]   <= rLedGValueBuf[7:0]; 
                end else begin // 下降阶段
                    rRGBbitDataPre[7:0]     <= (rLedBValue[7:0] - rLedBValueBuf[7:0]);
                    rRGBbitDataPre[15:8]    <= (rLedRValue[7:0] - rLedRValueBuf[7:0]);
                    rRGBbitDataPre[23:16]   <= (rLedGValue[7:0] - rLedGValueBuf[7:0]);
                end
            end
    
            8'hff:begin//电量显示
                if(rLedNumCnt[7:0]<(LED_NUM-iSocUniformization-1'b1))
                    rRGBbitDataPre[23:0] <= {24{1'b0}};
                else
                    rRGBbitDataPre[23:0] <= {rLedGValue,rLedRValue,rLedBValue};
            end
        
            default: begin
                // if(rLedNumCnt[7:0]<(LED_NUM-5'd10-1'b1))
                //     rRGBbitDataPre[23:0] <= {24{1'b0}};
                // else
                //     rRGBbitDataPre[23:0] <= {rLedGValue,rLedRValue,rLedBValue};
                rRGBbitDataPre[23:0] <= {24{1'b0}};
            end
        endcase
    end
end

//******************************************************************************
//一个灯珠送完计数
always @(posedge iSysClk ) begin
    if(iVsync)begin
        rLedBitCnt[7:0] <=8'd0;
    end else begin
        if(rLedBitCnt[7:0] == LED_BIT_DEEP*8'd3)
            rLedBitCnt[7:0] <=8'd0;
        else if(wImageRamRdEn)
            rLedBitCnt[7:0] <= rLedBitCnt[7:0] + 1'b1;
    end     
end

assign wLedBitCntEnd = (rLedBitCnt[7:0] == LED_BIT_DEEP*8'd3);

// 灯珠个数计数
always @(posedge iSysClk ) begin
    if(iVsync)begin
        rLedNumCnt[7:0] <=8'b0;
    end else begin
        if(rLedNumCnt[7:0] == (LED_NUM))
            rLedNumCnt[7:0] <= 8'd0;
        else if(wLedBitCntEnd)
            rLedNumCnt[7:0] <= rLedNumCnt[7:0] + 1'b1;
    end     
end

// assign wLedNumCntEnd = wLedBitCntEnd && (rLedNumCnt[7:0] == LED_NUM);
//******************************************************************************


//******************************************************************************
//计数及数据ROM
always @(posedge iSysClk ) begin
    if(wLedBitCntEnd || iVsync)
        rRGBbitData[23:0] <= rRGBbitDataPre[23:0];
    else if(wImageRamRdEn)
        rRGBbitData[23:0] <= {rRGBbitData[22:0],1'b0};
end


assign wRGBbitData = rRGBbitData[23];

//******************************************************************************


CommonRZ #(
    .NUM    (  2        )
)U_CommonRZ(   
    .InSysClk                   (iSysClk                        )   ,
    .InVsStart                  (iVsync                         )   ,//active-high
    .InOneReqOver               (1'b1                           )   ,
    .InStateOrder               (32'h3FFFFFFF                   )   ,//状态机跳转 3 2 F
	
    .InT1H                      (T1H                            )   ,//Return-to-zero code "1" high level duration, unit 8ns (system clock 125M hours)
    .InT0H                      (T0H                            )   ,//Return to zero code "0" high level duration, in 8ns (125M hours on system clock)
    .InTDclk                    (TDclk                          )   ,//Return-to-zero code period, unit 8ns (125M time system clock)
    
    .InTALL1                    (16'd0                          )   ,//Normally high level timing duration, unit 8ns (125M hours system clock)
    .InTALL0                    (16'd0                          )   ,//Normally low level timing duration, unit 8ns (125M hours on system clock)
    .InShiftUnitNumPerSubField  (SHIFT_UNIT_NUM_PER_SUB_FIELD   )   ,//灯珠个数*3
    .InLogicalGroup64           (1'b0                           )   ,//Whether the number of data groups exceeds 32 (1 indicates 64 data groups, 0 indicates less than 32 data groups)
    .InLogicalGroup128          (1'b0                           )   ,
	  
    .InSerialDotsNumPerColor    (LED_BIT_DEEP                   )   ,//Chip data bit width

    .InConfigLength             (8'd0                           )   ,//register length //8'd16	
    .InConfigRegister1          (96'h7FFF                       )   ,//Register value (low value, effective length determined by control word length)

    .InData                     ({2{wRGBbitData} }              )   ,//Image data entry
    .OutImageRamRdEn            (wImageRamRdEn                  )   ,//Image data reading
    .OutImageRamRdCntAddEn      (                               )   ,//Image data reading
	
    .OutData                    (oRGBOut[1:0]                   )    //Return to zero code output

);

endmodule