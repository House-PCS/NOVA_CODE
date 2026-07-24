`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/06/30 20:22:55
// Design Name: 
// Module Name: ADCSample
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////




module  AdcSampleCore 
(
    input                       iSysClk                        ,
    input                       iSysClkDbug                    ,
    input                       iSysRst                        ,
    input                       iVP                            ,
    input                       iVN                            ,
    input           [15:0]      iVauxP                         ,
    input           [15:0]      iVauxN                         ,
    output          [11:0]      oFpgaTemp                      ,
    output signed   [15:0]      oLlcBusVoltageA                ,
    output signed   [15:0]      oLlcBusVoltageB                ,
    output signed   [15:0]      oBusVoltageA                   ,
    output signed   [15:0]      oBusVoltageB                   ,
    output signed   [15:0]      oBusTotalVoltage               ,
    output signed   [15:0]      oBusTotalVoltagePre            ,
    output signed   [15:0]      oResonanceCurrentA             ,
    output signed   [15:0]      oResonanceCurrentB             ,
    output signed   [15:0]      oLlcBusCurrentA                ,
    output signed   [15:0]      oLlcBusCurrentB                ,
    output signed   [15:0]      oBusTotalCurrentA              ,
    output signed   [15:0]      oBusTotalCurrentB              ,
    output signed   [15:0]      oVoltageSampVref               ,
    output signed   [15:0]      oCurrentSampVref               ,
    output signed   [15:0]      oTemp1                         ,
    output signed   [15:0]      oTemp2                         ,
    output signed   [15:0]      oTemp3                         ,
    output signed   [15:0]      oTemp4                         ,
    output signed   [15:0]      oTemp5                         ,
    output signed   [15:0]      oTemp6                         ,
    output signed   [2:0]       oTempSel                       ,
    output                      oAdcSampleSequenceDone         ,  
    input           [11:0]      iCurrentSampVrefAvera 
);
//-------------------------------Code Start Here--------------------------------

 reg      [11:0] rFpgaTemp                 = 12'd0;
 reg      [11:0] rLlcBusVoltageA           = 12'd0;
 reg      [11:0] rLlcBusVoltageB           = 12'd0;
 reg      [11:0] rBusVoltageA              = 12'd0;
 reg      [11:0] rBusVoltageB              = 12'd0;
 reg      [11:0] rBusTotalVoltage          = 12'd0;
 reg      [11:0] rBusTotalVoltagePre       = 12'd0;
 reg      [11:0] rResonanceCurrentA        = 12'd0;
 reg      [11:0] rResonanceCurrentB        = 12'd0;
 reg      [11:0] rLlcBusCurrentA           = 12'd0;
 reg      [11:0] rLlcBusCurrentB           = 12'd0;
 reg      [11:0] rBusTotalCurrentA         = 12'd0;
 reg      [11:0] rBusTotalCurrentB         = 12'd0;
 reg      [11:0] rVoltageSampVref          = 12'd0;
 reg      [11:0] rCurrentSampVref          = 12'd0;
 reg      [11:0] rTemp1                    = 12'd0;
 reg      [11:0] rTemp2                    = 12'd0;
 reg      [11:0] rTemp3                    = 12'd0;
 reg      [11:0] rTemp4                    = 12'd0;
 reg      [11:0] rTemp5                    = 12'd0;
 reg      [11:0] rTemp6                    = 12'd0;
 reg      [2:0]  rTempSel                  = 3'd0;
 reg      [6:0]  rTempSelDlyCnt            =  'd0;


    reg  [11:0] rAdc0 = 12'd0 ; //XADC 读出的 vaux0 采样数据
    reg  [11:0] rAdc1 = 12'd0 ; //XADC 读出的 vaux1 采样数据
    reg  [11:0] rAdc2 = 12'd0 ; //XADC 读出的 vaux2 采样数据
    reg  [11:0] rAdc3 = 12'd0 ; //XADC 读出的 vaux3 采样数据
    reg  [11:0] rAdc4 = 12'd0 ; //XADC 读出的 vaux4 采样数据
    reg  [11:0] rAdc5 = 12'd0 ; 
    reg  [11:0] rAdc6 = 12'd0 ; 
    reg  [11:0] rAdc7 = 12'd0 ; 
    reg  [11:0] rAdc8 = 12'd0 ; 
    reg  [11:0] rAdc9 = 12'd0 ; 
    reg  [11:0] rAdcA = 12'd0 ; 
    reg  [11:0] rAdcB = 12'd0 ; 
    reg  [11:0] rAdcC = 12'd0 ; 
    reg  [11:0] rAdcD = 12'd0 ; 
    reg  [11:0] rAdcE = 12'd0 ; 
    reg  [11:0] rAdcF = 12'd0 ; //XADC 读出的 vaux15 采样数据
    reg  [11:0]rTempSamp= 12'd0 ;


reg     [6:0]   rDrpAddr =  7'd0        ;
reg             rDrpEn   =  1'd0        ;

wire     [6:0]  wDrpAddr                ;
wire            wDrpEn                  ;
wire            wDrpDataOutRdy          ;
wire    [15:0]  wDrpDataOut             ;
wire    [4:0]   wAdcChannel             ;
wire            wAdcConvertDone         ;
wire            wAdcSampleDone          ;


wire       user_temp_alarm_out;
wire       vccint_alarm_out   ;
wire       vccaux_alarm_out   ;
wire       ot_out             ;
wire       alarm_out          ;
wire       busy_out           ;
//*****************************************************************************
//ADC标准化寄存器
reg  signed  [27:0] rNormal0LlcBusVoltageA        =28'd0;
reg  signed  [27:0] rNormal0LlcBusVoltageB        =28'd0;
reg  signed  [27:0] rNormal0BusVoltageA           =28'd0;
reg  signed  [27:0] rNormal0BusVoltageB           =28'd0;
reg  signed  [27:0] rNormal0BusTotalVoltage       =28'd0;
reg  signed  [27:0] rNormal0BusTotalVoltagePre    =28'd0;
reg  signed  [27:0] rNormal0VoltageSampVref       =28'd0;
reg  signed  [27:0] rNormal0CurrentSampVref       =28'd0;

reg  signed  [15:0] rNormalLlcBusVoltageA        =16'd0;
reg  signed  [15:0] rNormalLlcBusVoltageB        =16'd0;
reg  signed  [15:0] rNormalBusVoltageA           =16'd0;
reg  signed  [15:0] rNormalBusVoltageB           =16'd0;
reg  signed  [15:0] rNormalBusTotalVoltage       =16'd0;
reg  signed  [15:0] rNormalBusTotalVoltagePre    =16'd0;
reg  signed  [15:0] rNormalVoltageSampVref       =16'd0;
reg  signed  [15:0] rNormalCurrentSampVref       =16'd0;

reg  signed  [26:0] rNormal0ResonanceCurrentA   =27'd0;
reg  signed  [26:0] rNormal0ResonanceCurrentB   =27'd0;
reg  signed  [26:0] rNormal0LlcBusCurrentA      =27'd0;
reg  signed  [26:0] rNormal0LlcBusCurrentB      =27'd0;
reg  signed  [26:0] rNormal0BusTotalCurrentA    =27'd0;
reg  signed  [26:0] rNormal0BusTotalCurrentB    =27'd0;

reg  signed  [15:0] rNormalResonanceCurrentA    =16'd0;
reg  signed  [15:0] rNormalResonanceCurrentB    =16'd0;
reg  signed  [15:0] rNormalLlcBusCurrentA       =16'd0;
reg  signed  [15:0] rNormalLlcBusCurrentB       =16'd0;
reg  signed  [15:0] rNormalBusTotalCurrentA     =16'd0;
reg  signed  [15:0] rNormalBusTotalCurrentB     =16'd0;

reg  [2:0] rNormalAdcSampleDone;

//*****************************************************************************





always @(posedge iSysClk ) begin
    if(wAdcSampleDone == 1'b1)begin
        rResonanceCurrentB  [11:0]  <= rAdc0[11:0] ;//LLC2_HV_Curr
                                   // <= rAdc1[11:0] ;
        rBusTotalVoltage    [11:0]  <= rAdc2[11:0] ;//BUS_Relay_Behind_VOL
        rBusVoltageB        [11:0]  <= rAdc3[11:0] ;//BUCKBOOST2_BUS_VOL
                                   // <= rAdc4[11:0] ;
        rResonanceCurrentA  [11:0]  <= rAdc5[11:0] ;//LLC1_HV_Curr
        rLlcBusCurrentA     [11:0]  <= rAdc6[11:0] ;//BUCKBOOST_Curr1
        rLlcBusCurrentB     [11:0]  <= rAdc7[11:0] ;//BUCKBOOST_Curr2
                                   // <= rAdc8[11:0] ;
        rLlcBusVoltageA     [11:0]  <= rAdc9[11:0] ;//LLC1_VOL
        rBusVoltageA        [11:0]  <= rAdcA[11:0] ;//BUCKBOOST1_BUS_VOL 
        rBusTotalVoltagePre [11:0]  <= rAdcB[11:0] ;//BUS_Relay_Front_VOL
        rLlcBusVoltageB     [11:0]  <= rAdcC[11:0] ;//LLC2_VOL
        rBusTotalCurrentB   [11:0]  <= rAdcD[11:0] ;//BUS_Total_Curr2
                                   // <= rAdcE[11:0] ;
        rBusTotalCurrentA   [11:0]  <= rAdcF[11:0] ;//BUS_Total_Curr1
        
    end
    else begin
        rResonanceCurrentB  [11:0]  <= rResonanceCurrentB  [11:0];
        rBusTotalVoltage    [11:0]  <= rBusTotalVoltage    [11:0];
        rBusVoltageB        [11:0]  <= rBusVoltageB        [11:0];
        rResonanceCurrentA  [11:0]  <= rResonanceCurrentA  [11:0];
        rLlcBusCurrentA     [11:0]  <= rLlcBusCurrentA     [11:0];
        rLlcBusCurrentB     [11:0]  <= rLlcBusCurrentB     [11:0];
        rLlcBusVoltageA     [11:0]  <= rLlcBusVoltageA     [11:0];
        rBusVoltageA        [11:0]  <= rBusVoltageA        [11:0];
        rBusTotalVoltagePre [11:0]  <= rBusTotalVoltagePre [11:0];
        rLlcBusVoltageB     [11:0]  <= rLlcBusVoltageB     [11:0];
        rBusTotalCurrentB   [11:0]  <= rBusTotalCurrentB   [11:0];
        rBusTotalCurrentA   [11:0]  <= rBusTotalCurrentA   [11:0];
    end
end

always @(posedge iSysClk ) begin
    if(wAdcSampleDone == 1'b1)
        rTempSelDlyCnt <= rTempSelDlyCnt + 1'b1;
    else
        rTempSelDlyCnt <= rTempSelDlyCnt ;
end

always@(posedge iSysClk)begin
   if((wAdcSampleDone == 1'b1) && (rTempSelDlyCnt == 7'd127))
        rTempSel[2:0] <= rTempSel[2:0] + 1'b1;
    else
        rTempSel[2:0] <= rTempSel[2:0] ;
end
// wire [2:0] wTeseTempSel ;

// PwmVio your_instance_nameila (
//   .clk(iSysClk),                // input wire clk
//   .probe_out0(wTeseTempSel[2:0])  // output wire [9 : 0] probe_out0
// );

// always @(posedge iSysClk ) begin
//     rTempSel[2:0] <= wTeseTempSel[2:0] ;
// end


always @(posedge iSysClk ) begin
    if((wAdcSampleDone == 1'b1) && (rTempSelDlyCnt == 7'd126))begin
        case(rTempSel[2:0]) 
            //3'd0:    rVoltageSampVref[11:0] <= rTempSamp[11:0] ;
            3'd1:    rCurrentSampVref[11:0] <= rTempSamp[11:0] ;
            3'd2:    rTemp1          [11:0] <= rTempSamp[11:0] ;
            3'd3:    rTemp2          [11:0] <= rTempSamp[11:0] ;
            3'd4:    rTemp3          [11:0] <= rTempSamp[11:0] ;
            3'd5:    rTemp4          [11:0] <= rTempSamp[11:0] ;
            3'd6:    rTemp5          [11:0] <= rTempSamp[11:0] ;
            3'd7:    rTemp6          [11:0] <= rTempSamp[11:0] ;
            default: rVoltageSampVref[11:0] <= rTempSamp[11:0] ;
        endcase
    end
    else begin
        rVoltageSampVref[11:0] <= rVoltageSampVref[11:0] ;
        rCurrentSampVref[11:0] <= rCurrentSampVref[11:0] ;
        rTemp1          [11:0] <= rTemp1          [11:0] ;
        rTemp2          [11:0] <= rTemp2          [11:0] ;
        rTemp3          [11:0] <= rTemp3          [11:0] ;
        rTemp4          [11:0] <= rTemp4          [11:0] ;
        rTemp5          [11:0] <= rTemp5          [11:0] ;
        rTemp6          [11:0] <= rTemp6          [11:0] ;     
    end
end

//******************************************************************************
//1.XADC 数据读取寄存器地址 直接连接到通道选择输出channel_out[4:0]
//2.产生den_in一个时钟周期的高电平    

always @(posedge iSysClk ) begin
    if(wAdcConvertDone) 
        rDrpAddr[6:0] <= {2'b00,wAdcChannel[4:0]}; 
    else 
        rDrpAddr[6:0] <= 7'd0;
end 

assign  wDrpAddr[6:0] = rDrpAddr[6:0] ;

always @(posedge iSysClk ) begin
    if(wAdcConvertDone) 
        rDrpEn <= 1'b1; 
 else 
        rDrpEn <= 1'b0;
end 

assign wDrpEn = rDrpEn;
//******************************************************************************


//******************************************************************************
//
always @(posedge iSysClk ) begin
    if(wDrpDataOutRdy == 1'b1)begin
        case (wAdcChannel[4:0])
         5'h00:     rFpgaTemp[11:0] <= wDrpDataOut[15:4];
         5'h03:     rTempSamp[11:0] <= wDrpDataOut[15:4];
         5'h10:     rAdc0[11:0] <= wDrpDataOut[15:4];
         5'h11:     rAdc1[11:0] <= wDrpDataOut[15:4];
         5'h12:     rAdc2[11:0] <= wDrpDataOut[15:4];
         5'h13:     rAdc3[11:0] <= wDrpDataOut[15:4];
         5'h14:     rAdc4[11:0] <= wDrpDataOut[15:4];
         5'h15:     rAdc5[11:0] <= wDrpDataOut[15:4];
         5'h16:     rAdc6[11:0] <= wDrpDataOut[15:4];
         5'h17:     rAdc7[11:0] <= wDrpDataOut[15:4];
         5'h18:     rAdc8[11:0] <= wDrpDataOut[15:4];
         5'h19:     rAdc9[11:0] <= wDrpDataOut[15:4];
         5'h1A:     rAdcA[11:0] <= wDrpDataOut[15:4];
         5'h1B:     rAdcB[11:0] <= wDrpDataOut[15:4];
         5'h1C:     rAdcC[11:0] <= wDrpDataOut[15:4];
         5'h1D:     rAdcD[11:0] <= wDrpDataOut[15:4];
         5'h1E:     rAdcE[11:0] <= wDrpDataOut[15:4];
         5'h1F:     rAdcF[11:0] <= wDrpDataOut[15:4];
         default:begin
            rAdc0[11:0] <= rAdc0[11:0] ;
            rAdc1[11:0] <= rAdc1[11:0] ;
            rAdc2[11:0] <= rAdc2[11:0] ;
            rAdc3[11:0] <= rAdc3[11:0] ;
            rAdc4[11:0] <= rAdc4[11:0] ;
            rAdc5[11:0] <= rAdc5[11:0] ;
            rAdc6[11:0] <= rAdc6[11:0] ;
            rAdc7[11:0] <= rAdc7[11:0] ;
            rAdc8[11:0] <= rAdc8[11:0] ;
            rAdc9[11:0] <= rAdc9[11:0] ;
            rAdcA[11:0] <= rAdcA[11:0] ;
            rAdcB[11:0] <= rAdcB[11:0] ;
            rAdcC[11:0] <= rAdcC[11:0] ;
            rAdcD[11:0] <= rAdcD[11:0] ;
            rAdcE[11:0] <= rAdcE[11:0] ;
            rAdcF[11:0] <= rAdcF[11:0] ;
         end
        endcase
    end
end

//******************************************************************************

        //----------- Begin Cut here for INSTANTIATION Template ---// INST_TAG
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


 assign        oFpgaTemp          [11:0]   = rFpgaTemp          [11:0];



always @(posedge iSysClk ) begin
    rNormalAdcSampleDone[2:0] <= {rNormalAdcSampleDone[1:0],wAdcSampleDone};
end
 
assign oAdcSampleSequenceDone = rNormalAdcSampleDone[2];
//*****************************************************************************
//电压采样计算 单位是0.1V

//(ADC/4096-0.5)* (43+470*7)/1.5 = (ADC-2048)* (43+470*7)/1.5/4096  *10 （0.1V）对应A1硬件
//(ADC/4096-0.5)* (43+470*4)/1.5 = (ADC-2048)* (43+470*4)/1.5/4096  *10 （0.1V）对应A1硬件
always @(posedge iSysClk ) begin
    if(rNormalAdcSampleDone[0])begin
    rNormal0LlcBusVoltageA    [27:0] <=  rLlcBusVoltageA    [11:0] * 16'd6382;
    rNormal0LlcBusVoltageB    [27:0] <=  rLlcBusVoltageB    [11:0] * 16'd6382;
    
    rNormal0BusVoltageA       [27:0] <=  rBusVoltageA       [11:0] * 16'd6382;
    rNormal0BusVoltageB       [27:0] <=  rBusVoltageB       [11:0] * 16'd6382;
    rNormal0BusTotalVoltage   [27:0] <=  rBusTotalVoltage   [11:0] * 16'd11110;
    rNormal0BusTotalVoltagePre[27:0] <=  rBusTotalVoltagePre[11:0] * 16'd11110;
    rNormal0VoltageSampVref   [27:0] <=  rVoltageSampVref   [11:0] * 16'd6382;
    rNormal0CurrentSampVref   [27:0] <=  rCurrentSampVref   [11:0] * 16'd6382;
    end
end

always @(posedge iSysClk ) begin
    if(rNormalAdcSampleDone[1])begin
        rNormalLlcBusVoltageA    [15:0] <=  rNormal0LlcBusVoltageA    [27:12];
        rNormalLlcBusVoltageB    [15:0] <=  rNormal0LlcBusVoltageB    [27:12];
        rNormalBusVoltageA       [15:0] <=  rNormal0BusVoltageA       [27:12];
        rNormalBusVoltageB       [15:0] <=  rNormal0BusVoltageB       [27:12];
        rNormalBusTotalVoltage   [15:0] <=  rNormal0BusTotalVoltage   [27:12];
        rNormalBusTotalVoltagePre[15:0] <=  rNormal0BusTotalVoltagePre[27:12];
        rNormalVoltageSampVref   [15:0] <=  rNormal0VoltageSampVref   [27:12];
        rNormalCurrentSampVref   [15:0] <=  rNormal0CurrentSampVref   [27:12];
    end
end

//*****************************************************************************

//*****************************************************************************
//电流采样计算 单位是1mA
// 正向关系：VCCA2_Ref5 + (VP-VN) * 2.4/6.8 = (ADCp-ADCn)
// 反向计算：（ADC/4096-0.5）*12.5*6.8/2.4 == （ADC-2048）*12.5*6.8/2.4/4096  *1000 mA 
// （ADC-2048）*12.5*6.8/2.4/4096  * 1000* 1024/1024  
//   ==（ADC-2048）*8854.2 /1024
//   == （ADC *8854 -2048*8854）/1024
reg    [11:0]    rCurrentVef = 12'd2074;
always@(posedge iSysClk)begin
  if(iCurrentSampVrefAvera[11:0]>12'd495 || iCurrentSampVrefAvera[11:0]<12'd480)
      rCurrentVef[11:0] <= 12'd2048;
  else
      rCurrentVef[11:0] <= (iCurrentSampVrefAvera[11:0]*17)>>2;
end  


always @(posedge iSysClk ) begin
    if(rNormalAdcSampleDone[0])begin
        rNormal0ResonanceCurrentA[26:0] <= rResonanceCurrentA [11:0]*15'd8854-rCurrentVef[11:0]*15'd8854;//12'd2074*15'd8854;
        rNormal0ResonanceCurrentB[26:0] <= rResonanceCurrentB [11:0]*15'd8854-rCurrentVef[11:0]*15'd8854;//12'd2074*15'd8854;
        rNormal0LlcBusCurrentA   [26:0] <= rLlcBusCurrentA    [11:0]*15'd8854-rCurrentVef[11:0]*15'd8854;//12'd2074*15'd8854;
        rNormal0LlcBusCurrentB   [26:0] <= rLlcBusCurrentB    [11:0]*15'd8854-rCurrentVef[11:0]*15'd8854;//12'd2074*15'd8854;
        rNormal0BusTotalCurrentA [26:0] <= rBusTotalCurrentA  [11:0]*15'd8854-rCurrentVef[11:0]*15'd8854;//12'd2074*15'd8854;
        rNormal0BusTotalCurrentB [26:0] <= rBusTotalCurrentB  [11:0]*15'd8854-rCurrentVef[11:0]*15'd8854;//12'd2074*15'd8854;
    end
end

always @(posedge iSysClk ) begin //传感器测满4096，也只有17A
    if(rNormalAdcSampleDone[1])begin
        rNormalResonanceCurrentA[15:0] <= rNormal0ResonanceCurrentA[25:10];
        rNormalResonanceCurrentB[15:0] <= rNormal0ResonanceCurrentB[25:10];
        rNormalLlcBusCurrentA   [15:0] <= rNormal0LlcBusCurrentA   [25:10];
        rNormalLlcBusCurrentB   [15:0] <= rNormal0LlcBusCurrentB   [25:10];
        rNormalBusTotalCurrentA [15:0] <= rNormal0BusTotalCurrentA [25:10]-16'd200;
        rNormalBusTotalCurrentB [15:0] <= rNormal0BusTotalCurrentB [25:10];
    end
end

 assign        oVoltageSampVref   [15:0]   = {4'h0,rVoltageSampVref[11:0]};
 assign        oCurrentSampVref   [15:0]   = {4'h0,rCurrentSampVref[11:0]};

 assign        oLlcBusVoltageA    [15:0]   = rNormalLlcBusVoltageA    [15:0];
 assign        oLlcBusVoltageB    [15:0]   = rNormalLlcBusVoltageB    [15:0];
 assign        oBusVoltageA       [15:0]   = rNormalBusVoltageA       [15:0];
 assign        oBusVoltageB       [15:0]   = rNormalBusVoltageB       [15:0];
 assign        oBusTotalVoltage   [15:0]   = rNormalBusTotalVoltage   [15:0];
 assign        oBusTotalVoltagePre[15:0]   = rNormalBusTotalVoltagePre[15:0];

 assign        oResonanceCurrentA [15:0]   = rNormalResonanceCurrentA[15:0];
 assign        oResonanceCurrentB [15:0]   = rNormalResonanceCurrentB[15:0];
 assign        oLlcBusCurrentA    [15:0]   = rNormalLlcBusCurrentA   [15:0];
 assign        oLlcBusCurrentB    [15:0]   = rNormalLlcBusCurrentB   [15:0];
 assign        oBusTotalCurrentA  [15:0]   = rNormalBusTotalCurrentA [15:0];
 assign        oBusTotalCurrentB  [15:0]   = rNormalBusTotalCurrentB [15:0];

 assign        oTemp1             [15:0]   = rTemp1             [11:0];
 assign        oTemp2             [15:0]   = rTemp2             [11:0];
 assign        oTemp3             [15:0]   = rTemp3             [11:0];
 assign        oTemp4             [15:0]   = rTemp4             [11:0];
 assign        oTemp5             [15:0]   = rTemp5             [11:0];
 assign        oTemp6             [15:0]   = rTemp6             [11:0];
 assign        oTempSel           [2:0]    = rTempSel           [2:0] ;
endmodule