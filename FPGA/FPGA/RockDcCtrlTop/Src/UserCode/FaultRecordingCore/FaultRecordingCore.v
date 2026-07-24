module FaultRecordingCore(
    input                               iSysClk                    ,
    input                               iRstP                      ,
//*************************U_MonitorDataStorage******************************//  
    input                [32*36-1: 0]   iMonitorData               ,
    input                               iMonitorDataVld            ,
    input                               iDcDcErr                   ,
    input                [   7: 0]      iBmsShutdown               ,
    output               [  25: 0]      oMcuRdStartAddr            ,
    output                              oMcuFlashBurnEn            ,
    input                               iMcuFaultBurnFinish        ,
    //Write DDR CMD
    output                              oDdrWrCmdClk               ,
    output                              oDdrWrBegin                ,
    input                               iDdrWrRdy                  , 
    output               [   6: 0]      oDdrWrBufBaseAddr          ,     
    output                              oDdrWrBufSel               , 
    output               [   8: 0]      oDdrWrLen                  ,
    output               [  25: 0]      oDdrWrAddr                 ,
    //Write DDR Data
    input                               iDdrWrDataClk              ,
    input                [   8: 0]      iDdrWrDataAddr             ,
    input                               iDdrWrDataAddrEn           ,
    output               [  15: 0]      oDdrWrMask                 ,
    output               [ 127: 0]      oDdrWrData                 
);


MonitorDataStorage U_MonitorDataStorage(
    .iSysClk                            (iSysClk                   ),
    .iRstP                              (iRstP                     ),
    
    .iMonitorData                       (iMonitorData              ),
    .iMonitorDataVld                    (iMonitorDataVld           ),
    .iDcDcErr                           (iDcDcErr                  ),
    
    .oDdrCmdClk                         (oDdrWrCmdClk              ),
    .oDdrWrBegin                        (oDdrWrBegin               ),
    .iDdrWrRdy                          (iDdrWrRdy                 ),
    .oDdrWrBufBaseAddr                  (oDdrWrBufBaseAddr[4:0]    ),
    .oDdrWrBufSel                       (oDdrWrBufSel              ),
    .oDdrWrLen                          (oDdrWrLen[8:0]            ),
    .oDdrWrAddr                         (oDdrWrAddr[25:0]          ),
    
    .iDdrDataClk                        (iDdrWrDataClk             ),
    .iDdrWrDataAddr                     (iDdrWrDataAddr[8:0]       ),
    .iDdrWrDataAddrEn                   (iDdrWrDataAddrEn          ),
    .oDdrWrMask                         (oDdrWrMask[15:0]          ),
    .oDdrWrData                         (oDdrWrData[127:0]         ),
    
    .iBmsShutdown                       (iBmsShutdown[7:0]         ),
    .oMcuRdStartAddr                    (oMcuRdStartAddr[25:0]     ),
    .oMcuFlashBurnEn                    (oMcuFlashBurnEn           ),
    .iMcuFaultBurnFinish                (iMcuFaultBurnFinish       )

);
endmodule