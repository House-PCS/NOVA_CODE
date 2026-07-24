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
module LedLampStripCtrlCore#(
    parameter     CLK_FREQ  = 100       ,
    parameter     LED_NUM   = 16'd21
)(
    input           iSysClk             ,
    input           iCnt1Ms             ,
    input  [15:0]   iBatSoc             ,
    input           iNoEthNet           ,
    input  [7:0]    iLedStatus          ,
    input  [7:0]    iLedRValue          ,
    input  [7:0]    iLedGValue          ,
    input  [7:0]    iLedBValue          ,

    input  [7:0]    iLedMode            ,  //0-关闭，1-常亮 2-闪烁 3-呼吸 ff-显示电量（默认）5-充放电电量及状态显示
    input  [15:0]   iLedPeriod          ,
    output [1:0]    oRGBOut             
);

//-------------------------------Code Start Here--------------------------------

//******************************************************************************
// 内部参数定义
localparam  COUNT_Vsync                     =   8'd4        ;


localparam  LED_BIT_DEEP                    =   6'd16        ;
localparam  BIT_DEEP                        = LED_BIT_DEEP*3-1;



//******************************************************************************

//******************************************************************************
    reg     [7:0]               rCountVs            = 8'd0  ;
    wire                        wCountVs                    ;    
    reg     [4:0]               rSocUniformization  = 5'd3  ;
    reg     [15:0]              rAlpha              = 16'd0 ;
    reg     [1:0]               rDirBuf             = 2'b0  ;
    wire                        wDirPulse                   ;
    reg     [15:0]              rAlphaStep          = 16'd0 ;
    wire    [15:0]              wAlphaStep                  ;
    wire                        wAlphaStepClcDone           ;


//******************************************************************************


always @(posedge iSysClk ) begin
    rDirBuf[1:0] <= {rDirBuf[0],iLedStatus[0]};
end

assign wDirPulse = rDirBuf[1] ^ rDirBuf[0];

//******************************************************************************
// 每ms步长计算
UsignedDivision #(
    .WITH1              (    16             ),     // 被除数位宽
    .WITH2              (    16             )      // 除数位宽
)AlphaStepClc 
(
    .iSysClk            (iSysClk            ),   // 时钟
    .iSysRst            (1'b0               ),   // 异步复位，低有效
    .iStart             (1'b1               ),   // 开始信号，高有效
    .iDividend          (16'hFFFF           ),   // 被除数
    .iDivisor           ((iLedPeriod>>1)+1'b1   ),   // 除数 单位ms
    .oQuotient          (wAlphaStep         ),   // 商
    .oRemainder         (                   ),   // 余数
    .oDone              (wAlphaStepClcDone  ),   // 完成信号，高有效
    .oBusy              (                   )
);

always @(posedge iSysClk ) begin
    if(wAlphaStepClcDone)
        rAlphaStep <= wAlphaStep;
    else
        rAlphaStep <= rAlphaStep;
end
//******************************************************************************


//******************************************************************************
// Vsyns 计数器
always @(posedge iSysClk) begin
    if(iCnt1Ms) begin
        if(rCountVs[7:0] == (COUNT_Vsync - 8'd1))
            rCountVs[7:0] <= 8'd0;
        else
            rCountVs[7:0] <= rCountVs[7:0] + 8'd1;
    end
    else
            rCountVs[7:0] <= rCountVs[7:0];
end
    
assign wCountVs = (iCnt1Ms && (rCountVs[7:0] == (COUNT_Vsync - 8'd1)));

//******************************************************************************

//******************************************************************************
//数据Vsync来临的时候进行电量更新
// 21/1000 = 2^-6+2^-8+2^-10+2^-11
// 21/1000 = 43/2048
//0-1000的电量转化为 0 -21的灯珠个数
always @(posedge iSysClk ) begin
     if(wCountVs)begin
         if(rSocUniformization[4:0] > (LED_NUM-1'b1))
             rSocUniformization[4:0] <= LED_NUM-1'b1 ;
         else
            rSocUniformization[4:0] <= ((iBatSoc*43)>>11);
    end
end
//******************************************************************************


//******************************************************************************
//渐变系数更新
always @(posedge iSysClk ) begin
    if (wDirPulse)
        rAlpha[15:0] <= 16'd0;
    else begin
        if(iCnt1Ms)begin
            if(rAlpha[15:0] >(16'hFFFF-rAlphaStep) )
                rAlpha[15:0] <= (16'hFFFF-rAlphaStep) ;
            else
                rAlpha[15:0] <= rAlpha[15:0] + rAlphaStep;
        end
    end
end

//******************************************************************************

//******************************************************************************

wire [3:0]   wRGBout ;

Led5050Ctrl#(
    .CLK_FREQ                (100                    ),
    .LED_NUM                 (LED_NUM                )
)U_Led5050Ctrl(
    .iSysClk                 (iSysClk                ),//            
    .iRValue                 (iLedRValue             ),//[7:0]       
    .iGValue                 (iLedGValue             ),//[7:0]       
    .iBValue                 (iLedBValue             ),//[7:0]       
    .iVsync                  (wCountVs               ),//            
    .iLedMode                (iLedMode               ),//[2:0] 
    .iNoEthNet               (iNoEthNet              ),      
    .iSocUniformization      (rSocUniformization     ),//[4:0]       //SOC 值 0-100% 对应0-21个灯珠点亮的灯珠个数
    .iAlpha                  (rAlpha                 ),//[15:0]          
    .iDir                    (rDirBuf[0]             ),//            
    .oRGBOut                 (wRGBout[1:0]           ) //[1:0]       
);

Led2020Ctrl#(
    .CLK_FREQ                (100                    ),
    .LED_NUM                 (LED_NUM                )
)U_Led2020Ctrl(
    .iSysClk                 (iSysClk                ),//            
    .iRValue                 (iLedRValue             ),//[7:0]       
    .iGValue                 (iLedGValue             ),//[7:0]       
    .iBValue                 (iLedBValue             ),//[7:0]       
    .iVsync                  (wCountVs               ),//            
    .iLedMode                (iLedMode               ),//[2:0] 
    .iNoEthNet               (iNoEthNet              ),       
    .iSocUniformization      (rSocUniformization     ),//[4:0]       //SOC 值 0-100% 对应0-21个灯珠点亮的灯珠个数
    .iAlpha                  (rAlpha                 ),//[15:0]          
    .iDir                    (rDirBuf[0]             ),//            
    .oRGBOut                 (wRGBout[3:2]           ) //[1:0]       
);

assign oRGBOut[1:0] = {wRGBout[2],wRGBout[2]};

endmodule