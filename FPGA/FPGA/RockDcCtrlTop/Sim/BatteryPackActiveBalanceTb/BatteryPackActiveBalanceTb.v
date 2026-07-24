`timescale 1ns / 1ps
//****************************************VSCODE PLUG-IN**********************************//
//----------------------------------------------------------------------------------------
// IDE :                   VSCODE     
// VSCODE plug-in version: Verilog-Hdl-Format-4.1.20260207
// VSCODE plug-in author : Jiang Percy
//----------------------------------------------------------------------------------------
//****************************************Copyright (c)***********************************//
// Copyright(C)            Please Write Company name
// All rights reserved     
// File name:              
// Last modified Date:     2026/03/16 15:55:24
// Last Version:           V1.0
// Descriptions:           
//----------------------------------------------------------------------------------------
// Created by:             Please Write You Name 
// Created date:           2026/03/16 15:55:24
// mail      :             Please Write mail 
// Version:                V1.0
// TEXT NAME:              BatteryPackActiveBalance.v
// PATH:                   E:\RocDCDCCtrl\RockDcCtrlTop\Sim\BatteryPackActiveBalanceTb\BatteryPackActiveBalance.v
// Descriptions:           
//                         
//----------------------------------------------------------------------------------------
//****************************************************************************************//

module BatteryPackActiveBalanceTb();



reg                         SysClk                     =1'd0;   
reg     signed   [15:0]     wEqualizationEnable         ;
reg              [31:0]     wBatVoltage                 ;
reg              [31:0]     wBatVoltageAverage          ;
wire     signed   [31:0]    wActiveBalanceVoltageOffset ;
reg     signed   [15:0]     wDCOutputTotalPower         ;
wire     signed  [31:0]     wBalanceVoltageRealPre      ;

// 时钟生成
always #5 SysClk = ~SysClk;

BatteryPackActiveBalance #(
    .CLK_FREQ  (100)  
    )
U_BatteryPackActiveBalance(
    .iSysClk                        (SysClk                     ) ,
    .iEqualizationEnable            (wEqualizationEnable        ) ,
    .iBatVoltage                    (wBatVoltage                ) ,
    .iBatVoltageAverage             (wBatVoltageAverage         ) ,
    .oActiveBalanceVoltageOffset    (wActiveBalanceVoltageOffset) ,               
    .iDCOutputTotalPower            (wDCOutputTotalPower        ) , 
    .oBalanceVoltageRealPre         (wBalanceVoltageRealPre     )      
    );

parameter    BATV = 3200;
parameter    BATVA= 2300;
wire   [31:0] m,n;  
initial begin
    SysClk = 0;
    wEqualizationEnable = 0;
    wBatVoltage=0;
    wBatVoltageAverage=0;
    #100;

    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 0;
    wEqualizationEnable=-1;
    #100;
  $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 0;
    wEqualizationEnable=-2;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 0;
    wEqualizationEnable=1;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 0;
    wEqualizationEnable=2;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;


    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 2048;
    wEqualizationEnable=-1;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 2048;
    wEqualizationEnable=-2;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 2048;
    wEqualizationEnable=1;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 2048;
    wEqualizationEnable=2;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;


    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = -2048;
    wEqualizationEnable=-1;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = -2048;
    wEqualizationEnable=-2;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = -2048;
    wEqualizationEnable=1;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = -2048;
    wEqualizationEnable=2;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;


    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 4095;
    wEqualizationEnable=-1;
    #100;
     $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 4095;
    wEqualizationEnable=-2;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 4095;
    wEqualizationEnable=1;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 4095;
    wEqualizationEnable=2;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;

    

    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 4096;
    wEqualizationEnable=-1;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 4096;
    wEqualizationEnable=-2;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 4096;
    wEqualizationEnable=1;
    #100;
   $display("wBatVoltage=%0d\nBatVoltageAverage=%0d\nDCOutputTotalPower=%0d\nEqualizationEnable=%0d\BalanceVoltageRealPre=%0d\nActiveBalanceVoltageOffset=%0d\n",wBatVoltage,wBatVoltageAverage,wDCOutputTotalPower,wEqualizationEnable,wBalanceVoltageRealPre,wActiveBalanceVoltageOffset);
    wBatVoltage = BATV;
    wBatVoltage = BATV;
    wBatVoltageAverage=BATVA;
    wDCOutputTotalPower = 4096;
    wEqualizationEnable=2;
    #100;

    $display("===== 遍历完成 =====");
    $finish;
end

// function void iterate_mn(int  ActiveBalanceMode,int  DCOutputTotalPower,int step = 1);
//     begin
//         $display("\n===== �?始遍�? offset = %0d（m/n步长=%0d�? =====", ActiveBalanceMode, step);
//         // 遍历m�?2300~3200，按步长递增
//         for (m = BATVA; m <= BATV; m = m + step) begin
//             // 遍历n�?2300~3200，按步长递增
//             for (n = BATVA; n <= BATV; n = n + step) begin
//                 // 赋�?�给目标信号
//                 wActiveBalanceVoltageOffset = ActiveBalanceMode;
//                 wDCOutputTotalPower = DCOutputTotalPower;
//                 wBatVoltage = m;
//                 wBatVoltageAverage = n;
//                 #10; // 每个组合保持10ns
                
//                 // 可�?�：打印当前值（验证用，全遍历时注释�?
//                // $display("offset=%0d, m=%0d, n=%0d", offset, m, n);
//             end
//         end
//         //$display("===== offset = %0d 遍历完成 =====", offset);
//     end
// endfunction

endmodule