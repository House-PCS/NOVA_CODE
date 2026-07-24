module  CommonRZ #(
  parameter NUM    =       96 
)(  input                InSysClk                        ,
    input                InVsStart                       ,//active-high
    input                InOneReqOver                    ,
	
  //32FF FFFF
    input  [31:0]        InStateOrder                    ,//The 4-bit represents a sequential function, starting at a high level
	                                                        //0: Output normally low level
                                                          //1: Output normally high level
                                                          //2: output register configuration
                                                          //3: Output image data
                                                          //4: end code
                                                          //f: The status ends
	
    input  [11:0]        InT1H                           ,//Return-to-zero code "1" high level duration, unit 8ns (system clock 125M hours)
    input  [11:0]        InT0H                           ,//Return to zero code "0" high level duration, in 8ns (125M hours on system clock)
    input  [11:0]        InTDclk                         ,//Return-to-zero code period, unit 8ns (125M time system clock)
    
    //16'd0
    input  [15:0]        InTALL1                         ,//Normally high level timing duration, unit 8ns (125M hours system clock)
    input  [15:0]        InTALL0                         ,//Normally low level timing duration, unit 8ns (125M hours on system clock)
	  
    //16'b0
    input  [15:0]        InShiftUnitNumPerSubField       ,//灯珠个数*3
    

    //1'b0
    input                InLogicalGroup64                ,//Whether the number of data groups exceeds 32 (1 indicates 64 data groups, 0 indicates less than 32 data groups)
    input                InLogicalGroup128               ,
	  
    //6'd16
    input  [ 5:0]        InSerialDotsNumPerColor         ,//Chip data bit width


    //8'd16		         
    input  [ 7:0]        InConfigLength                  ,//register length
    
    // 96'hEFFF
    input  [95:0]        InConfigRegister1               ,//Register value (low value, effective length determined by control word length)
	 
    //input  [95:0]        InData                          ,//Image data entry
    input    [NUM-1:0]   InData                          ,//Image data entry
    output               OutImageRamRdEn                 ,//Image data reading
    output               OutImageRamRdCntAddEn           ,//Image data reading
	  //output [95:0]        OutData                          //Return to zero code output
    output   [NUM-1:0]      OutData                          //Return to zero code output

);


reg         rReqStart              =  1'b0;
reg         DataReady              =  1'b0;
reg [15:0]  rDataReady             = 16'd0;
reg         Start                  =  1'b0;
reg [ 1:0]  rStart                 =  1'b0; 
wire        StartPos                      ;   
							       
reg [31:0]  rStateOrder            = 32'hffff_ffff;
							       
							       
wire        TALL0En                       ;
reg [15:0]  TALL0Cnt               = 16'd0;
reg         OutTALL0               =  1'b0;
							       
wire        TALL1En                       ;
reg [15:0]  TALL1Cnt               = 16'd0;
reg         OutTALL1               =  1'b0;
							       
wire        ConfigEn                      ;
reg [ 3:0]  rConfigDataCnt         =  4'd0;
reg [ 6:0]  rConfigCnt             =  7'd0;
reg         ConfigData             =  1'b0;
reg         ConfigDataOut          =  1'b0;
							       
wire        DataEn                        ;
reg [ 5:0]  DataCnt                =  6'd0;
reg         rImageRamRdEn          =  1'b0;
reg         rImageRamRdCntAddEn    =  1'b0;
reg [ 5:0]  SerialDotsNumPerColor  =  6'd0;
reg [15:0]  ShiftCycleCnt          = 16'd0;
wire        DataOver                      ;


wire        EndDataEn                     ;
//reg [95:0]  rDataOut               = 96'h0;
reg  [NUM-1:0]  rDataOut               =   'h0;
reg         EndDataOut             =  1'd0;
wire        EndDataOver                   ;

reg [11:0]  rCnt                   = 12'h0;
reg         rGenerate1             =  1'b0;
reg         rGenerate0             =  1'b0;
//reg [95:0]  rData                  = 96'h0;
//reg [95:0]  oData                  = 96'd0;

reg [NUM-1:0]   rData                  =   'h0;
reg [NUM-1:0]   oData                  =   'd0;


wire        TALL1Over                     ;
wire        TALL0Over                     ;
wire        ConfigDataOver                ;
wire        DataFrameOver                 ;
wire        EndOver                       ;

//
reg [11:0]  T1H                    = 12'd0;
reg [11:0]  T0H                    = 12'd0;
reg [11:0]  TDclk                  = 12'd0;
reg [15:0]  TALL1                  = 16'd0;
reg [15:0]  TALL0                  = 16'd0;
reg [ 7:0]  ConfigLength           =  8'd0;
reg [95:0] ConfigRegisterDataPowerUp = 96'd0;	
		
always @ (posedge InSysClk)
begin
  T1H[11:0]         <= InT1H[11:0]   - 1'b1;
  T0H[11:0]         <= InT0H[11:0]   - 1'b1;
  TDclk[11:0]       <= InTDclk[11:0] - 1'b1;
  TALL1[15:0]       <= InTALL1[15:0] - 1'b1;
  TALL0[15:0]       <= InTALL0[15:0] - 1'b1;
  ConfigLength[7:0] <= InConfigLength[7:0] - 1'b1;
  SerialDotsNumPerColor[5:0] <= InSerialDotsNumPerColor[5:0] - 1'b1;
  ConfigRegisterDataPowerUp[95:0] <=  InConfigRegister1[95:0];
end

always @ (posedge InSysClk)
if(InVsStart)
  rReqStart <= 1'b1;
else if(Start)
  rReqStart <= 1'b0;
    
always @ (posedge InSysClk)
begin
  if(rReqStart & InOneReqOver)
	DataReady <= 1'b1;
  else if(Start)
	DataReady <= 1'b0;
end
always @ (posedge InSysClk)
  rDataReady[15:0] <= {rDataReady[14:0],DataReady};
  
always @ (posedge InSysClk)
begin
  if(rDataReady[15])
	Start <= 1'b1;
  else if(rStateOrder[31:28] == 4'hf || InVsStart)
	Start <= 1'b0;
end

always @ (posedge InSysClk)
  rStart[1:0] <= {rStart[0],Start};

//assign  StartPos = (~rStart[0]) && Start;

//******************delay 5 SysClk***************************
reg  [19:0]  rStartPos = 20'd0;
wire         wStartPos       ;
wire         StartPos1;
wire         StartPos2;
wire         StartPos3;


assign  StartPos  = (~rStartPos[14]) && (rStartPos[13]);
assign  StartPos1 = (~rStartPos[1]) && (rStartPos[0]);
assign  StartPos2 = (~rStartPos[5]) && (rStartPos[4]);
assign  StartPos3 = (~rStartPos[9]) && (rStartPos[8]);

always@(posedge InSysClk)
  rStartPos[19:0] = {rStartPos[18:0],(~rStart[0]) && Start};
  
//***********************************************************

always @ (posedge InSysClk)
if(StartPos)
  rStateOrder[31:0] <= InStateOrder[31:0];
else if(TALL0Over || TALL1Over || ConfigDataOver || DataFrameOver || EndOver)
  rStateOrder[31:0] <= {rStateOrder[27:0],4'd0};


//TALL0
assign TALL0En =(rStateOrder[31:28] == 4'd0);
  
always @ (posedge InSysClk)
if(!Start)
  TALL0Cnt <= 15'd0;
else if(!TALL0En)
  TALL0Cnt <= 15'd0;
else 
  TALL0Cnt <= TALL0Cnt + 1'd1;
  
assign TALL0Over = (TALL0Cnt[15:0] == TALL0[15:0]);


//TALL1
assign TALL1En =(rStateOrder[31:28] == 4'd1);
  
always @ (posedge InSysClk)
if(!Start)
  TALL1Cnt <= 16'd0;
else if(!TALL1En)
  TALL1Cnt <= 16'd0;
else 
  TALL1Cnt <= TALL1Cnt + 1'd1;
  
assign TALL1Over =(TALL1Cnt[15:0] == TALL1[15:0]);


//Config  
assign ConfigEn =(rStateOrder[31:28] == 4'd2);
  
always @ (posedge InSysClk)
if(!Start)
  rConfigCnt[6:0] <= 6'h0;
else if(ConfigEn)
begin
  if(ConfigDataOver)
   rConfigCnt[6:0] <= 7'h0;
  else if(rCnt[11:0] == TDclk)
   rConfigCnt[6:0] <= rConfigCnt[6:0] + 6'h1;
  else
    rConfigCnt[6:0] <= rConfigCnt[6:0];
end
else
   rConfigCnt[6:0] <= 6'h0;

assign ConfigDataOver = (rConfigCnt[6:0] == ConfigLength[6:0]) && (rCnt[11:0] == TDclk);  

//寄存器输出
	
reg [95:0] rConfigDataInitialPowerUp = 96'd0;
always @(posedge InSysClk)
  if(StartPos)
    rConfigDataInitialPowerUp[95:0] <= ConfigRegisterDataPowerUp[95:0];  
  else if(ConfigEn && (rCnt[8:0] == TDclk))
    rConfigDataInitialPowerUp[95:0] <= {rConfigDataInitialPowerUp[94:0],1'b0};

always @(posedge InSysClk)
    ConfigData <=  rConfigDataInitialPowerUp[ConfigLength[6:0]];


//InData
assign DataEn =(rStateOrder[31:28] == 4'd3);

always @ (posedge InSysClk)
if(DataEn)
begin
  if(DataOver)
    DataCnt[5:0] <= 6'h0;
  else if(rCnt[11:0] == TDclk)
    DataCnt[5:0] <= DataCnt[5:0] + 6'h1;
  else
    DataCnt[5:0] <= DataCnt[5:0];
end 
else
  DataCnt[5:0] <= 6'h0;
   
assign DataOver = (DataCnt[5:0] == SerialDotsNumPerColor[5:0]) && (rCnt[11:0] == TDclk); 

always @ (posedge InSysClk)
if(!Start)
  ShiftCycleCnt[15:0] <= 16'h0;
else if(DataEn)
begin
  if(DataFrameOver)
  	ShiftCycleCnt[15:0] <= 16'h0;
  else if(DataOver)
  	ShiftCycleCnt[15:0] <= ShiftCycleCnt[15:0] + 16'h1;
end
else 
  ShiftCycleCnt[15:0] <= 16'h0;

assign DataFrameOver =  ((ShiftCycleCnt[15:0] == InShiftUnitNumPerSubField[15:0] - 1'b1) && DataOver);

reg  ImageRdAddrAddEnFor64Group  = 1'b0;
reg  ImageRdAddrAddEnFor128Group = 1'b0;

always @ (posedge InSysClk)
  ImageRdAddrAddEnFor64Group  <= (((rCnt[11:0] == (TDclk[11:0] - 12'd16)) && DataEn) || StartPos1);

always @ (posedge InSysClk)
  ImageRdAddrAddEnFor128Group <= ((((rCnt[11:0] == (TDclk[11:0] - 12'd4)) && DataEn) || ((rCnt[11:0] == (TDclk[11:0] - 12'd12)) && DataEn) || ((rCnt[11:0] == (TDclk[11:0] - 12'd16)) && DataEn)) || StartPos1 || StartPos2 || StartPos3);

//图像获取
always @ (*) begin
	if(((rCnt[11:0] == (TDclk[11:0] - 12'd8)) && DataEn) || (InLogicalGroup64 && ImageRdAddrAddEnFor64Group) || (InLogicalGroup128 && ImageRdAddrAddEnFor128Group))
		rImageRamRdEn <= 1'b1;
	else
		rImageRamRdEn <= 1'b0;
end

assign OutImageRamRdEn       = rImageRamRdEn;
assign OutImageRamRdCntAddEn = rImageRamRdEn;


//END
assign EndDataEn =(rStateOrder[31:28] == 4'd4);

assign	EndOver = (rCnt[11:0] == TDclk) && EndDataEn; 

always @(posedge InSysClk)
if(EndDataEn)
	EndDataOut <= rGenerate0;
else
	EndDataOut <= 1'b0;

//归零码生成
always @ (posedge InSysClk) begin
    if(DataEn || ConfigEn || EndDataEn)
        if((rCnt[11:0] == TDclk[11:0]))
            rCnt[11:0] <= 12'h0;
        else
            rCnt[11:0] <= rCnt[11:0] + 12'h1;
    else
        rCnt[11:0] <= 12'h0;
end

always @ (posedge InSysClk)
    if((rCnt[11:0] == TDclk[11:0])  || StartPos)
        rGenerate1 <= 1'b1;
    else if(rCnt[11:0] == T1H[11:0])
        rGenerate1 <= 1'b0;

always @ (posedge InSysClk)
    if((rCnt[11:0] == TDclk[11:0]) || StartPos)
        rGenerate0 <= 1'b1;
    else if(rCnt[11:0] == T0H[11:0])
        rGenerate0 <= 1'b0;
		

always @(posedge InSysClk)
    if(ConfigEn)
		if(ConfigData)
			ConfigDataOut <= rGenerate1;
		else
			ConfigDataOut <= rGenerate0;	
	else
		ConfigDataOut <= 1'b0;

always @(posedge InSysClk)
  if((DataEn &(rCnt[11:0] == TDclk[11:0]))||StartPos)begin
    rData <= InData;
  end

genvar  i;
generate
    for(i=0;i<NUM;i=i+1) 
        begin:  Data
            always @(posedge InSysClk)
                if(DataEn)
                    if(rData[i])
                        rDataOut[i] <= rGenerate1;
                    else
                        rDataOut[i] <= rGenerate0;
                else
                    rDataOut[i] <= 0;
        end
endgenerate


            // always @(posedge InSysClk)
            //     if(DataEn)
            //         if(rData)
            //             rDataOut <= rGenerate1;
            //         else
            //             rDataOut <= rGenerate0;
            //     else
            //         rDataOut <= 0;


reg rTALL0En   = 1'b0;
reg rTALL1En   = 1'b0;
reg rConfigEn  = 1'b0;
reg rDataEn    = 1'b0;
reg rEndDataEn = 1'b0;
always @(posedge InSysClk)
begin
	rTALL0En   <= TALL0En  ;
	rTALL1En   <= TALL1En  ;
	rConfigEn  <= ConfigEn ;
	rDataEn    <= DataEn   ;
	rEndDataEn <= EndDataEn;
end

always @(*)
begin
  if(rTALL0En)
    oData[NUM-1:0] = {NUM{1'b0}};
  else if(rTALL1En)
    oData[NUM-1:0] = {NUM{1'b1}};
  else if(rConfigEn)	
    oData[NUM-1:0] = {NUM{ConfigDataOut}};
  else if(rDataEn)
    oData[NUM-1:0] = rDataOut[NUM-1:0];
  else if(rEndDataEn)
    oData[NUM-1:0] = {NUM{EndDataOut}};
  else 
    oData[NUM-1:0] = {NUM{1'b0}};
end 

// always @(*)
// begin
//   if(rTALL0En)
//     oData[95:0] = {96{1'b0}};
//   else if(rTALL1En)
//     oData[95:0] = {96{1'b1}};
//   else if(rConfigEn)	
//     oData[95:0] = {96{ConfigDataOut}};
//   else if(rDataEn)
//     oData[95:0] = rDataOut[95:0];
//   else if(rEndDataEn)
//     oData[95:0] = {96{EndDataOut}};
//   else 
//     oData[95:0] = {96{1'b0}};
// end 
assign OutData = oData;

endmodule   




