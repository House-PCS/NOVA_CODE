//------------------------------------------------------------------------------
//Project Name      :   
//Moudle Name       :   SoftReBoot.v
//Original Author   :   Gouxuhui
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
module SoftReBoot(
  input             iClk           ,
  input             iReBootEn      , // From MCU:Enable ;  0:Disable
  input             iReBootMode      // From MCU:0:APP  ;  1:Golden
);
reg  [ 1:0]	           rReBootEn;
reg  [ 2:0] 	         IcapeWrCnt = 3'b0;

reg			               rIcapeRdWrEn=1'b1;
reg			               IcapeCs=1'b1;
wire [31:0]	           IcapeRdData;
reg  [31:0]	           IcapeWrData;
reg  [31:0]	           ReConfigAddr='d0;



ICAPE2 #(
  .DEVICE_ID(0'h3651093),     // Specifies the pre-programmed Device ID value to be used for simulation
							  //purposes.
  .ICAP_WIDTH("X32"),         // Specifies the input and output data width.
  .SIM_CFG_FILE_NAME("NONE")  // Specifies the Raw Bitstream (RBT) file to be parsed by the simulation
							  //model.
)
ICAPE2_inst (
  .O(IcapeRdData),         	// 32-bit output: Configuration data output bus
  .CLK(iClk),     			// 1-bit input: Clock Input
  .CSIB(IcapeCs),   			// 1-bit input: Active-Low ICAP Enable
  .I(IcapeWrData),         	// 32-bit input: Configuration data input bus
  .RDWRB(rIcapeRdWrEn)  			// 1-bit input: Read/Write Select input
);

always @(posedge iClk) 
	rReBootEn[1:0] <= {rReBootEn[0],iReBootEn};

always @(posedge iClk) 
	if(rReBootEn[1:0] == 2'b01)
		rIcapeRdWrEn <= 1'b0;
	else if((IcapeWrCnt[2:0] == 3'd7)&& (IcapeCs == 1'b1))
		rIcapeRdWrEn <= 1'b1;

always @(posedge iClk) 
	if(IcapeWrCnt[2:0] == 3'd7)
		IcapeCs <= 1'b1;
	else if(!rIcapeRdWrEn)
		IcapeCs <= 1'b0;

always @(posedge iClk) 
  if(IcapeCs)
    IcapeWrCnt[2:0] <= 3'd0;
  else if(IcapeWrCnt[2:0] == 3'd7)
     IcapeWrCnt[2:0] <= IcapeWrCnt[2:0];    
  else if(!IcapeCs)
    IcapeWrCnt[2:0] <= IcapeWrCnt[2:0] + 1;

always @(posedge iClk)
  if(iReBootMode == 1'b0)
    ReConfigAddr <= 32'h00640800;
  else if(iReBootMode == 1'b1)
    ReConfigAddr[31:0] <= 32'h00320800;


always @(*) 
  case(IcapeWrCnt)
  3'd0:	IcapeWrData <= 32'hFFFFFFFF;          //DUMMY
  3'd1:	IcapeWrData <= 32'h5599AA66;          //SYNCWORD
  3'd2:	IcapeWrData <= 32'h04000000;          //NOOP
  3'd3:	IcapeWrData <= 32'h0C400080;          //WBSTAR
  3'd4:	IcapeWrData <= ReConfigAddr[31:0];    //WBADDR
  3'd5:	IcapeWrData <= 32'h0C000180;          //CMD
  3'd6:	IcapeWrData <= 32'h000000F0;          //IPEOG
  3'd7:	IcapeWrData <= 32'h04000000;          //NOOP
  endcase  	

endmodule