#include "F28x_Project.h"
#include "can.h"


#define CAN_IF1ARB_ID_M     0x1FFFFFFFU   // `
#define CAN_IF1ARB_DIR      0x20000000U   // Message Direction
#define CAN_IF1ARB_XTD      0x40000000U   // Extended Identifier
#define CAN_IF1ARB_MSGVAL   0x80000000U   // Message Valid
#define CAN_IF1MCTL_RMTEN    0x200U    // Remote Enable
#define CAN_IF1MCTL_DLC_M    0xFU      // Data length code
#define CAN_IF1CMD_MSG_NUM_M   0xFFU       // Message Number
#define CAN_IF1MSK_MSK_S   0U
#define CAN_IF1MSK_MSK_M   0x1FFFFFFFU   // Identifier Mask
#define CAN_IF1MSK_MDIR    0x40000000U   // Mask Message Direction
#define CAN_IF1MSK_MXTD    0x80000000U   // Mask Extended Identifier
#define CAN_MSG_OBJ_FIFO                   (0x00000002U)
#define CAN_IF1MCTL_UMASK    0x1000U   // Use Acceptance Mask
#define CAN_IF1MCTL_EOB      0x80U     // End of Block
#define CAN_IF1CMD_CONTROL     0x100000U   // Access Control Bits
#define CAN_IF1CMD_ARB         0x200000U   // Access Arbitration Bits
#define CAN_IF1CMD_MASK        0x400000U   // Access Mask Bits
#define CAN_IF1CMD_DIR         0x800000U   // Write/Read Direction
#define CAN_MSG_OBJ_USE_ID_FILTER          (0x00000001U)

#define CAN_MSG_OBJ_USE_EXT_FILTER         CAN_IF1MSK_MXTD
#define CAN_MSG_OBJ_USE_DIR_FILTER         CAN_IF1MSK_MDIR

//! IF2 Arbitration Standard ID Shift Offset
#define CAN_IF2ARB_STD_ID_S             (18U)
#define CAN_IF2ARB_ID_M     0x1FFFFFFFU   // Message  Identifier
//! IF2 Arbitration Standard ID Mask
#define CAN_IF2ARB_STD_ID_M             (0x1FFC0000U)

#define CAN_IF2CMD_MSG_NUM_S   0U
#define CAN_IF2CMD_MSG_NUM_M   0xFFU       // Message Number
#define CAN_IF2CMD_BUSY        0x8000U     // Busy Flag
#define CAN_IF2CMD_DATA_B      0x10000U    // Access Data Bytes 4-7
#define CAN_IF2CMD_DATA_A      0x20000U    // Access Data Bytes 0-3
#define CAN_IF2CMD_TXRQST      0x40000U    // Access Transmission Request Bit
#define CAN_IF2CMD_CLRINTPND   0x80000U    // Clear Interrupt Pending Bit
#define CAN_IF2CMD_CONTROL     0x100000U   // Access Control Bits
#define CAN_IF2CMD_ARB         0x200000U   // Access Arbitration Bits
#define CAN_IF2CMD_MASK        0x400000U   // Access Mask Bits
#define CAN_IF2CMD_DIR         0x800000U   // Write/Read Direction




// Globals
//
volatile unsigned long i;
uint16_t garrTXMsgData[8] = {0x55, 0xAA, 0x00, 0x00, 0x55, 0xAA, 0x00, 0x00}; // TX Data
uint16_t garrCanaRXMsgData[8] = {0, 0, 0, 0, 0, 0, 0, 0};         // RX Data
uint16_t garrCanbRXMsgData[8] = {0, 0, 0, 0, 0, 0, 0, 0};         // RX Data
uint32_t messageSize = sizeof(garrTXMsgData);          // Message Size (DLC)
static uint32_t _garrIdMailbox[33];



static const uint16_t canBitValues[] =
{
    0x1100, // TSEG2 2, TSEG1 2, SJW 1, Divide 5
    0x1200, // TSEG2 2, TSEG1 3, SJW 1, Divide 6
    0x2240, // TSEG2 3, TSEG1 3, SJW 2, Divide 7
    0x2340, // TSEG2 3, TSEG1 4, SJW 2, Divide 8
    0x3340, // TSEG2 4, TSEG1 4, SJW 2, Divide 9
    0x3440, // TSEG2 4, TSEG1 5, SJW 2, Divide 10
    0x3540, // TSEG2 4, TSEG1 6, SJW 2, Divide 11
    0x3640, // TSEG2 4, TSEG1 7, SJW 2, Divide 12
    0x3740  // TSEG2 4, TSEG1 8, SJW 2, Divide 13
};

typedef enum
{
        //! Transmit message object.
        MSG_OBJ_TYPE_TRANSMIT,
        MSG_OBJ_TYPE_TRANSMIT_REMOTE,
        //! Receive message object.
        MSG_OBJ_TYPE_RECEIVE,
        MSG_OBJ_TYPE_RXTX_REMOTE
}
msgObjType;

// Function Prototypes
uint32_t setCANABitRate(uint32_t sourceClock, uint32_t bitRate);
uint32_t setCANBBitRate(uint32_t sourceClock, uint32_t bitRate);
void  setupCANAMessageObject(uint32_t objID, uint32_t msgID, msgObjType msgType);
void  setupCANBMessageObject(uint32_t objID, uint32_t msgID, msgObjType msgType);
void sendCANAMessage(uint32_t objID);
void sendCANBMessage(uint32_t objID);
bool getCANAMessage(uint32_t objID);
bool getCANBMessage(uint32_t objID);



void CanInit(void)
{
    GPIO_SetupPinMux(36, GPIO_MUX_CPU1, 6);  //GPIO36 - CANRXA
    GPIO_SetupPinMux(37, GPIO_MUX_CPU1, 6);  //GPIO37 - CANTXA
    GPIO_SetupPinOptions(36, GPIO_INPUT, GPIO_ASYNC);
    GPIO_SetupPinOptions(37, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(39, GPIO_MUX_CPU1, 6);  //GPIO39 - CANRXB
    GPIO_SetupPinMux(38, GPIO_MUX_CPU1, 6);  //GPIO38 - CANTXB
    GPIO_SetupPinOptions(39, GPIO_INPUT, GPIO_ASYNC);
    GPIO_SetupPinOptions(38, GPIO_OUTPUT, GPIO_PUSHPULL);

    //
    // Initialize the CAN-A controller
    //
    InitCANA();
    CanaRegs.CAN_RAM_INIT.bit.RAM_INIT_DONE = 1;
    CanaRegs.CAN_CTL.bit.CCE = 1;
    InitCANB();
    CanbRegs.CAN_RAM_INIT.bit.RAM_INIT_DONE = 1;
    CanbRegs.CAN_CTL.bit.CCE = 1;

    //
    // Setup CAN to be clocked off the SYSCLKOUT
    //
    ClkCfgRegs.CLKSRCCTL2.bit.CANABCLKSEL = 0;
    ClkCfgRegs.CLKSRCCTL2.bit.CANBBCLKSEL = 0;

    //
    // Set up the bit rate for the CAN bus.  This function sets up the CAN
    // bus timing for a nominal configuration.
    // In this example, the CAN bus is set to 500 kbps.
    //
    // Consult the TRM for more information about
    // CAN peripheral clocking.
    //
    setCANABitRate(200000000, 125000);
    setCANBBitRate(200000000, 125000);


    CanaRegs.CAN_CTL.bit.Init = 0;
    CanaRegs.CAN_CTL.bit.CCE = 0;
    CanbRegs.CAN_CTL.bit.Init = 0;
    CanbRegs.CAN_CTL.bit.CCE = 0;

    // Initialize the message object that will be used for sending CAN
    // messages.
    // Initialize the message object that will be used for receiving CAN
        // messages.
    setupCANAMessageObject(CANA_RX_MSG_OBJ, CAN_MSG_REC_ID, MSG_OBJ_TYPE_RECEIVE);
    setupCANBMessageObject(CANB_RX_MSG_OBJ, CAN_MSG_REC_ID, MSG_OBJ_TYPE_RECEIVE);

    setupCANAMessageObject(CAN_TX_MSG_OBJ6, CAN_TX_MSG_ID6, MSG_OBJ_TYPE_TRANSMIT);
    setupCANAMessageObject(CAN_TX_MSG_OBJ7, CAN_TX_MSG_ID7, MSG_OBJ_TYPE_TRANSMIT);
    setupCANAMessageObject(CAN_TX_MSG_OBJ8, CAN_TX_MSG_ID8, MSG_OBJ_TYPE_TRANSMIT);
    setupCANAMessageObject(CAN_TX_MSG_OBJ10, CAN_TX_MSG_ID10, MSG_OBJ_TYPE_TRANSMIT);
    setupCANAMessageObject(CAN_TX_MSG_OBJ11, CAN_TX_MSG_ID11, MSG_OBJ_TYPE_TRANSMIT);

    setupCANBMessageObject(CAN_TX_MSG_OBJ1, CAN_TX_MSG_ID4, MSG_OBJ_TYPE_TRANSMIT);
    setupCANBMessageObject(CAN_TX_MSG_OBJ4, CAN_TX_MSG_ID1, MSG_OBJ_TYPE_TRANSMIT);
    setupCANBMessageObject(CAN_TX_MSG_OBJ5, CAN_TX_MSG_ID5, MSG_OBJ_TYPE_TRANSMIT);
    setupCANBMessageObject(CAN_TX_MSG_OBJ9, CAN_TX_MSG_ID9, MSG_OBJ_TYPE_TRANSMIT);

}

#if 1
void CANTransmitMsg(void)
{
    uint16_t i = 0;
    for(i = 0; i < TXCOUNT; i++)
    {
        //
        // Send the CAN message using object number 1 (not the same thing as
        // CAN ID, which is also 1 in this example).  This function will cause
        // the message to be transmitted right away.
        //
        sendCANAMessage(CAN_TX_MSG_OBJ1);
        sendCANBMessage(CAN_TX_MSG_OBJ1);

        //
        // Delay 0.25 second before continuing
        //
        DELAY_US(250000);

        //
        // Get the receive message
        //
        getCANAMessage(CANA_RX_MSG_OBJ);
        getCANBMessage(CANB_RX_MSG_OBJ);

        //
        // Increment the value in the transmitted message data.
        //
//        ucTXMsgData[0] += 0x1;
//        ucTXMsgData[1] += 0x1;
//        ucTXMsgData[2] += 0x1;
//        ucTXMsgData[3] += 0x1;

        //
        // Reset data if exceeds a byte
        //
        if(garrCanaRXMsgData[0] == 0x55)
        {
            while(1);
        }
        if(garrCanbRXMsgData[0] == 0x55)
        {
            while(1);
        }
//        if(ucTXMsgData[2] > 0xFF)
//        {
//            ucTXMsgData[2] = 0;
//        }
//        if(ucTXMsgData[3] > 0xFF)
//        {
//            ucTXMsgData[3] = 0;
//        }
    }
}
#endif
//
// setCANABitRate - Set the CAN bit rate based on device clock (Hz)
//                 and desired bit rate (Hz)
//
uint32_t setCANABitRate(uint32_t sourceClock, uint32_t bitRate)
{
    uint32_t desiredRatio;
    uint32_t canBits;
    uint32_t preDivide;
    uint32_t regValue;
    uint16_t canControlValue;

    //
    // Calculate the desired clock rate.
    //
    desiredRatio = sourceClock / bitRate;

    //
    // Make sure that the Desired Ratio is not too large.  This enforces the
    // requirement that the bit rate is larger than requested.
    //
    if((sourceClock / desiredRatio) > bitRate)
    {
        desiredRatio += 1;
    }

    //
    // Check all possible values to find a matching value.
    //
    while(desiredRatio <= CAN_MAX_PRE_DIVISOR * CAN_MAX_BIT_DIVISOR)
    {
        //
        // Loop through all possible CAN bit divisors.
        //
        for(canBits = CAN_MAX_BIT_DIVISOR;
            canBits >= CAN_MIN_BIT_DIVISOR;
            canBits--)
        {
            //
            // For a given CAN bit divisor save the pre divisor.
            //
            preDivide = desiredRatio / canBits;

            //
            // If the calculated divisors match the desired clock ratio then
            // return these bit rate and set the CAN bit timing.
            //
            if((preDivide * canBits) == desiredRatio)
            {
                //
                // Start building the bit timing value by adding the bit timing
                // in time quanta.
                //
                regValue = canBitValues[canBits - CAN_MIN_BIT_DIVISOR];

                //
                // To set the bit timing register, the controller must be
                // placed
                // in init mode (if not already), and also configuration change
                // bit enabled.  The state of the register should be saved
                // so it can be restored.
                //
                canControlValue = CanaRegs.CAN_CTL.all;
                CanaRegs.CAN_CTL.bit.Init = 1;
                CanaRegs.CAN_CTL.bit.CCE = 1;

                //
                // Now add in the pre-scalar on the bit rate.
                //
                regValue |= ((preDivide - 1) & CAN_BTR_BRP_M) |
                            (((preDivide - 1) << 10) & CAN_BTR_BRPE_M);

                //
                // Set the clock bits in the and the bits of the
                // pre-scalar.
                //
                CanaRegs.CAN_BTR.all = regValue;

                //
                // Restore the saved CAN Control register.
                //
                CanaRegs.CAN_CTL.all = canControlValue;

                //
                // Return the computed bit rate.
                //
                return(sourceClock / ( preDivide * canBits));
            }
        }

        //
        // Move the divisor up one and look again.  Only in rare cases are
        // more than 2 loops required to find the value.
        //
        desiredRatio++;
    }
    return 0;
}


//
// setCANBBitRate - Set the CAN bit rate based on device clock (Hz)
//                 and desired bit rate (Hz)
//
uint32_t setCANBBitRate(uint32_t sourceClock, uint32_t bitRate)
{
    uint32_t desiredRatio;
    uint32_t canBits;
    uint32_t preDivide;
    uint32_t regValue;
    uint16_t canControlValue;

    //
    // Calculate the desired clock rate.
    //
    desiredRatio = sourceClock / bitRate;

    //
    // Make sure that the Desired Ratio is not too large.  This enforces the
    // requirement that the bit rate is larger than requested.
    //
    if((sourceClock / desiredRatio) > bitRate)
    {
        desiredRatio += 1;
    }

    //
    // Check all possible values to find a matching value.
    //
    while(desiredRatio <= CAN_MAX_PRE_DIVISOR * CAN_MAX_BIT_DIVISOR)
    {
        //
        // Loop through all possible CAN bit divisors.
        //
        for(canBits = CAN_MAX_BIT_DIVISOR;
            canBits >= CAN_MIN_BIT_DIVISOR;
            canBits--)
        {
            //
            // For a given CAN bit divisor save the pre divisor.
            //
            preDivide = desiredRatio / canBits;

            //
            // If the calculated divisors match the desired clock ratio then
            // return these bit rate and set the CAN bit timing.
            //
            if((preDivide * canBits) == desiredRatio)
            {
                //
                // Start building the bit timing value by adding the bit timing
                // in time quanta.
                //
                regValue = canBitValues[canBits - CAN_MIN_BIT_DIVISOR];

                //
                // To set the bit timing register, the controller must be
                // placed
                // in init mode (if not already), and also configuration change
                // bit enabled.  The state of the register should be saved
                // so it can be restored.
                //
                canControlValue = CanbRegs.CAN_CTL.all;
                CanbRegs.CAN_CTL.bit.Init = 1;
                CanbRegs.CAN_CTL.bit.CCE = 1;

                //
                // Now add in the pre-scalar on the bit rate.
                //
                regValue |= ((preDivide - 1) & CAN_BTR_BRP_M) |
                            (((preDivide - 1) << 10) & CAN_BTR_BRPE_M);

                //
                // Set the clock bits in the and the bits of the
                // pre-scalar.
                //
                CanbRegs.CAN_BTR.all = regValue;

                //
                // Restore the saved CAN Control register.
                //
                CanbRegs.CAN_CTL.all = canControlValue;

                //
                // Return the computed bit rate.
                //
                return(sourceClock / ( preDivide * canBits));
            }
        }

        //
        // Move the divisor up one and look again.  Only in rare cases are
        // more than 2 loops required to find the value.
        //
        desiredRatio++;
    }
    return 0;
}

//
// setupMessageObject - Setup message object as Transmit or Receive
//
void setupCANAMessageObject(uint32_t objID, uint32_t msgID, msgObjType msgType)
{

    //uint32_t   base id=0x12345000UL & 0X1FFFFFFF;

    //uint32_t   mask= 0x1FFFFOOQUL; 0x12345000\~ 0x123451FF   0x18000000   0x1FFFF000
    uint32_t cmdMaskReg = 0U;
    uint32_t maskReg = 0U;
    uint32_t arbReg = 0U;
    uint32_t msgCtrl = 0U;
    // Use Shadow variable for IF1CMD. IF1CMD should be written to in
    // single 32-bit write.

    union CAN_IF1CMD_REG CAN_IF1CMD_SHADOW;

    // Wait for busy bit to clear.
    while(CanaRegs.CAN_IF1CMD.bit.Busy)
    {
    }

    switch(msgType)
    {
        // Transmit message object.
        case MSG_OBJ_TYPE_TRANSMIT:
        {
            // Set message direction to transmit.
            arbReg = CAN_IF1ARB_DIR;
            break;
        }
        // Remote frame receive remote, with auto-transmit message object.
        case MSG_OBJ_TYPE_RXTX_REMOTE:
        {
            // Set message direction to Tx for remote receivers.
            arbReg = CAN_IF1ARB_DIR;
            // Set this object to auto answer if a matching identifier is seen.
            msgCtrl = (uint32_t)((uint32_t)CAN_IF1MCTL_RMTEN |
                                 (uint32_t)CAN_IF1MCTL_UMASK);
            break;
        }
        // Transmit remote request message object (CAN_MSG_OBJ_TYPE_TX_REMOTE)
        // or Receive message object (CAN_MSG_OBJ_TYPE_RX).
        default:
        {
           // Set message direction to read.
           arbReg = 0U;
           break;
        }
    }

    maskReg =  0x1FF00000 & CAN_IF1MSK_MSK_M;///
    // If the caller wants to filter on the extended ID bit then set it.
    maskReg |= CAN_IF1MSK_MXTD;
   // The caller wants to filter on the message direction field.
    maskReg |= CAN_IF1MSK_MDIR;

    arbReg |= (msgID & CAN_IF1ARB_ID_M) | CAN_IF1ARB_MSGVAL |CAN_IF1ARB_XTD;

    if((( CAN_MSG_OBJ_USE_ID_FILTER) |
           ( CAN_MSG_OBJ_USE_DIR_FILTER) |
           (CAN_MSG_OBJ_USE_EXT_FILTER)) != 0U)
   {
       msgCtrl |= CAN_IF1MCTL_UMASK;
   }

    if((msgType == CAN_MSG_OBJ_TYPE_TX) ||
            (msgType == CAN_MSG_OBJ_TYPE_RXTX_REMOTE))
    {
        msgCtrl |= ((uint32_t)8 & CAN_IF1MCTL_DLC_M);
    }


    if(CAN_MSG_OBJ_FIFO == 0U)
    {
        msgCtrl |= CAN_IF1MCTL_EOB;
    }

    // Set the Control, Arb, and Mask bit so that they get transferred to the
        // Message object.
        //
    cmdMaskReg |= CAN_IF1CMD_ARB;
    cmdMaskReg |= CAN_IF1CMD_CONTROL;
    cmdMaskReg |= CAN_IF1CMD_MASK;
    cmdMaskReg |= CAN_IF1CMD_DIR;


//    // Clear and Write out the registers to program the message object.
//    CAN_IF1CMD_SHADOW.all = 0;
//    CanaRegs.CAN_IF1MSK.all = 0;
//    CanaRegs.CAN_IF1ARB.all = 0;
//    CanaRegs.CAN_IF1MCTL.all = 0;
//
//    // Set the Control, Mask, and Arb bit so that they get transferred to the
//    // Message object.
//    CAN_IF1CMD_SHADOW.bit.Control = 1;
//    CAN_IF1CMD_SHADOW.bit.Arb = 1;
//    CAN_IF1CMD_SHADOW.bit.Mask = 1;
//    CAN_IF1CMD_SHADOW.bit.DIR = 1;
//
//    // Set direction to transmit
//    if(msgType == MSG_OBJ_TYPE_TRANSMIT)
//    {
//        CanaRegs.CAN_IF1ARB.bit.Dir = 1;
//    }
    // Set Message ID (this example assumes 11 bit ID mask)
    //CanaRegs.CAN_IF1ARB.bit.ID = (msgID << CAN_MSG_ID_SHIFT);
    //CanaRegs.CAN_IF1ARB.bit.MsgVal = 1;
   // CanaRegs.CAN_IF1ARB.bit.Xtd = 1;

    CanaRegs.CAN_IF1MSK.all = maskReg;
    CanaRegs.CAN_IF1ARB.all = arbReg;
    CanaRegs.CAN_IF1MCTL.all = msgCtrl;
    CanaRegs.CAN_IF1CMD.all = cmdMaskReg | (objID & CAN_IF1CMD_MSG_NUM_M);

    _garrIdMailbox[objID] = msgID;
    // Set the data length since this is set for all transfers.  This is
    // also a single transfer and not a FIFO transfer so set EOB bit.
//    CanaRegs.CAN_IF1MCTL.bit.DLC = messageSize;
//    CanaRegs.CAN_IF1MCTL.bit.EoB = 1;
//
//    // Transfer data to message object RAM
//    CAN_IF1CMD_SHADOW.bit.MSG_NUM = objID;
//    CanaRegs.CAN_IF1CMD.all = CAN_IF1CMD_SHADOW.all;
}

uint16_t MsgIdFindObjId(uint32_t msg_id)
{
    uint16_t i = 0;

    for(i = 1; i < 33; i++)
    {
        if(_garrIdMailbox[i] == msg_id)
        {
            return i;
        }
    }

    return 0;
}

//
// setupMessageObject - Setup message object as Transmit or Receive
//
void setupCANBMessageObject(uint32_t objID, uint32_t msgID, msgObjType msgType)
{
    //uint32_t   base id=0x12345000UL & 0X1FFFFFFF;

   //uint32_t   mask= 0x1FFFFOOQUL; 0x12345000\~ 0x123451FF   0x18000000   0x1FFFF000
   uint32_t cmdMaskReg = 0U;
   uint32_t maskReg = 0U;
   uint32_t arbReg = 0U;
   uint32_t msgCtrl = 0U;
   // Use Shadow variable for IF1CMD. IF1CMD should be written to in
   // single 32-bit write.

   union CAN_IF1CMD_REG CAN_IF1CMD_SHADOW;

   // Wait for busy bit to clear.
   while(CanbRegs.CAN_IF1CMD.bit.Busy)
   {
   }

   switch(msgType)
   {
       // Transmit message object.
       case MSG_OBJ_TYPE_TRANSMIT:
       {
           // Set message direction to transmit.
           arbReg = CAN_IF1ARB_DIR;
           break;
       }
       // Remote frame receive remote, with auto-transmit message object.
       case MSG_OBJ_TYPE_RXTX_REMOTE:
       {
           // Set message direction to Tx for remote receivers.
           arbReg = CAN_IF1ARB_DIR;
           // Set this object to auto answer if a matching identifier is seen.
           msgCtrl = (uint32_t)((uint32_t)CAN_IF1MCTL_RMTEN |
                                (uint32_t)CAN_IF1MCTL_UMASK);
           break;
       }
       // Transmit remote request message object (CAN_MSG_OBJ_TYPE_TX_REMOTE)
       // or Receive message object (CAN_MSG_OBJ_TYPE_RX).
       default:
       {
          // Set message direction to read.
          arbReg = 0U;
          break;
       }
   }

   maskReg =  0x1FF00000 & CAN_IF1MSK_MSK_M;///
   // If the caller wants to filter on the extended ID bit then set it.
   maskReg |= CAN_IF1MSK_MXTD;
  // The caller wants to filter on the message direction field.
   maskReg |= CAN_IF1MSK_MDIR;

   arbReg |= (msgID & CAN_IF1ARB_ID_M) | CAN_IF1ARB_MSGVAL |CAN_IF1ARB_XTD;

   if((( CAN_MSG_OBJ_USE_ID_FILTER) |
          ( CAN_MSG_OBJ_USE_DIR_FILTER) |
          (CAN_MSG_OBJ_USE_EXT_FILTER)) != 0U)
  {
      msgCtrl |= CAN_IF1MCTL_UMASK;
  }

   if((msgType == CAN_MSG_OBJ_TYPE_TX) ||
           (msgType == CAN_MSG_OBJ_TYPE_RXTX_REMOTE))
   {
       msgCtrl |= ((uint32_t)8 & CAN_IF1MCTL_DLC_M);
   }


   if(CAN_MSG_OBJ_FIFO == 0U)
   {
       msgCtrl |= CAN_IF1MCTL_EOB;
   }

   // Set the Control, Arb, and Mask bit so that they get transferred to the
       // Message object.
       //
   cmdMaskReg |= CAN_IF1CMD_ARB;
   cmdMaskReg |= CAN_IF1CMD_CONTROL;
   cmdMaskReg |= CAN_IF1CMD_MASK;
   cmdMaskReg |= CAN_IF1CMD_DIR;


//    // Clear and Write out the registers to program the message object.
//    CAN_IF1CMD_SHADOW.all = 0;
//    CanaRegs.CAN_IF1MSK.all = 0;
//    CanaRegs.CAN_IF1ARB.all = 0;
//    CanaRegs.CAN_IF1MCTL.all = 0;
//
//    // Set the Control, Mask, and Arb bit so that they get transferred to the
//    // Message object.
//    CAN_IF1CMD_SHADOW.bit.Control = 1;
//    CAN_IF1CMD_SHADOW.bit.Arb = 1;
//    CAN_IF1CMD_SHADOW.bit.Mask = 1;
//    CAN_IF1CMD_SHADOW.bit.DIR = 1;
//
//    // Set direction to transmit
//    if(msgType == MSG_OBJ_TYPE_TRANSMIT)
//    {
//        CanaRegs.CAN_IF1ARB.bit.Dir = 1;
//    }
   // Set Message ID (this example assumes 11 bit ID mask)
   //CanaRegs.CAN_IF1ARB.bit.ID = (msgID << CAN_MSG_ID_SHIFT);
   //CanaRegs.CAN_IF1ARB.bit.MsgVal = 1;
  // CanaRegs.CAN_IF1ARB.bit.Xtd = 1;

   CanbRegs.CAN_IF1MSK.all = maskReg;
   CanbRegs.CAN_IF1ARB.all = arbReg;
   CanbRegs.CAN_IF1MCTL.all = msgCtrl;
   CanbRegs.CAN_IF1CMD.all = cmdMaskReg | (objID & CAN_IF1CMD_MSG_NUM_M);

   _garrIdMailbox[objID] = msgID;
}
uint32_t cana_test,canb_test;
//
// sendCANMessage - Transmit data from the specified message object
//
void sendCANAMessage(uint32_t objID)
{
    //
    // Use Shadow variable for IF1CMD. IF1CMD should be written to in
    // single 32-bit write.
    //
    union CAN_IF1CMD_REG CAN_IF1CMD_SHADOW;

    //
    // Wait for busy bit to clear.
    //
    while(CanaRegs.CAN_IF1CMD.bit.Busy)
    {
    }

    //
    // Write data to transfer into DATA-A and DATA-B interface registers
    //
    uint16_t index;
    for(index = 0; index < messageSize; index++)
    {
        switch(index)
        {
            case 0:
                CanaRegs.CAN_IF1DATA.bit.Data_0 = garrTXMsgData[index];
                break;
            case 1:
                CanaRegs.CAN_IF1DATA.bit.Data_1 = garrTXMsgData[index];
                break;
            case 2:
                CanaRegs.CAN_IF1DATA.bit.Data_2 = garrTXMsgData[index];
                break;
            case 3:
                CanaRegs.CAN_IF1DATA.bit.Data_3 = garrTXMsgData[index];
                break;
            case 4:
                CanaRegs.CAN_IF1DATB.bit.Data_4 = garrTXMsgData[index];
                break;
            case 5:
                CanaRegs.CAN_IF1DATB.bit.Data_5 = garrTXMsgData[index];
                break;
            case 6:
                CanaRegs.CAN_IF1DATB.bit.Data_6 = garrTXMsgData[index];
                break;
            case 7:
                CanaRegs.CAN_IF1DATB.bit.Data_7 = garrTXMsgData[index];
                break;
        }
    }
    // Set Direction to write and set DATA-A/DATA-B to be transfered to
    // message object
    CAN_IF1CMD_SHADOW.all = 0;
    CAN_IF1CMD_SHADOW.bit.DIR = 1;
    CAN_IF1CMD_SHADOW.bit.DATA_A = 1;
    CAN_IF1CMD_SHADOW.bit.DATA_B = 1;

    // Set Tx Request Bit
    CAN_IF1CMD_SHADOW.bit.TXRQST = 1;

    // Transfer the message object to the message object specified by
    // objID.
    CAN_IF1CMD_SHADOW.bit.MSG_NUM = objID;
    CanaRegs.CAN_IF1CMD.all = CAN_IF1CMD_SHADOW.all;
    cana_test++;
}

void CanTxDataCopy(uint16_t *p_data)
{
    memcpy(garrTXMsgData, p_data, 8);
}
//
// sendCANMessage - Transmit data from the specified message object
//
void sendCANBMessage(uint32_t objID)
{
    //
    // Use Shadow variable for IF1CMD. IF1CMD should be written to in
    // single 32-bit write.
    //
    union CAN_IF1CMD_REG CAN_IF1CMD_SHADOW;

    //
    // Wait for busy bit to clear.
    //
    while(CanbRegs.CAN_IF1CMD.bit.Busy)
    {
    }

    //
    // Write data to transfer into DATA-A and DATA-B interface registers
    //
    uint16_t index;
    for(index = 0; index < messageSize; index++)
    {
        switch(index)
        {
            case 0:
                CanbRegs.CAN_IF1DATA.bit.Data_0 = garrTXMsgData[index];
                break;
            case 1:
                CanbRegs.CAN_IF1DATA.bit.Data_1 = garrTXMsgData[index];
                break;
            case 2:
                CanbRegs.CAN_IF1DATA.bit.Data_2 = garrTXMsgData[index];
                break;
            case 3:
                CanbRegs.CAN_IF1DATA.bit.Data_3 = garrTXMsgData[index];
                break;
            case 4:
                CanbRegs.CAN_IF1DATB.bit.Data_4 = garrTXMsgData[index];
                break;
            case 5:
                CanbRegs.CAN_IF1DATB.bit.Data_5 = garrTXMsgData[index];
                break;
            case 6:
                CanbRegs.CAN_IF1DATB.bit.Data_6 = garrTXMsgData[index];
                break;
            case 7:
                CanbRegs.CAN_IF1DATB.bit.Data_7 = garrTXMsgData[index];
                break;
        }
    }

    //
    // Set Direction to write and set DATA-A/DATA-B to be transfered to
    // message object
    //
    CAN_IF1CMD_SHADOW.all = 0;
    CAN_IF1CMD_SHADOW.bit.DIR = 1;
    CAN_IF1CMD_SHADOW.bit.DATA_A = 1;
    CAN_IF1CMD_SHADOW.bit.DATA_B = 1;

    //
    // Set Tx Request Bit
    //
    CAN_IF1CMD_SHADOW.bit.TXRQST = 1;

    //
    // Transfer the message object to the message object specified by
    // objID.
    //
    CAN_IF1CMD_SHADOW.bit.MSG_NUM = objID;
    CanbRegs.CAN_IF1CMD.all = CAN_IF1CMD_SHADOW.all;
    canb_test++;
}


//
// getCANMessage - Check the message object for new data.
//                 If new data, data written into array and return true.
//                 If no new data, return false.
//
bool getCANAMessage(uint32_t objID)
{
    bool status;
    // Use Shadow variable for IF2CMD. IF2CMD should be written to in
    // single 32-bit write.
    union CAN_IF2CMD_REG CAN_IF2CMD_SHADOW;

    // Set the Message Data A, Data B, and control values to be read
    // on request for data from the message object.

    CAN_IF2CMD_SHADOW.all = 0;
    CAN_IF2CMD_SHADOW.bit.Control = 1;
    CAN_IF2CMD_SHADOW.bit.DATA_A = 1;
    CAN_IF2CMD_SHADOW.bit.DATA_B = 1;
    // Transfer the message object to the message object IF register.
    CAN_IF2CMD_SHADOW.bit.MSG_NUM = objID;
    CanaRegs.CAN_IF2CMD.all = CAN_IF2CMD_SHADOW.all;

    // Wait for busy bit to clear.
    while(CanaRegs.CAN_IF2CMD.bit.Busy)
    {

    }
    // See if there is new data available.
    if(CanaRegs.CAN_IF2MCTL.bit.NewDat == 1)
    {
        // Populate Shadow Variable
        CAN_IF2CMD_SHADOW.all = CanaRegs.CAN_IF2CMD.all;
        // Clear New Data Flag
        CAN_IF2CMD_SHADOW.bit.TxRqst = 1;
        // Transfer the message object to the message object IF register.
        CAN_IF2CMD_SHADOW.bit.MSG_NUM = objID;
        CanaRegs.CAN_IF2CMD.all = CAN_IF2CMD_SHADOW.all;

        status = true;
    }
    else
    {
        status = false;
    }
    return(status);
}



//bool CAN_readMessage(uint32_t base, uint32_t objID,uint16_t *msgData)
//{
//    bool status;
//    uint16_t msgCtrl = 0U;
//
//    //
//    // Check the arguments.
//    //
//    //ASSERT(CAN_isBaseValid(base));
//   // ASSERT((objID <= 32U) && (objID != 0U));
//
//    //
//    // Set the Message Data A, Data B, and control values to be read
//    // on request for data from the message object.
//    //
//    // Transfer the message object to the message object IF register.
//    //
//  //  HWREG_BP(base + CAN_O_IF2CMD) =
//  //   ((uint32_t)CAN_IF2CMD_DATA_A | (uint32_t)CAN_IF2CMD_DATA_B |
////     (uint32_t)CAN_IF2CMD_CONTROL | (objID & CAN_IF2CMD_MSG_NUM_M) |
////     (uint32_t)CAN_IF2CMD_ARB);
//
//    CanaRegs.CAN_IF2CMD.all = ((uint32_t)CAN_IF2CMD_DATA_A | (uint32_t)CAN_IF2CMD_DATA_B |
//            (uint32_t)CAN_IF2CMD_CONTROL | (objID & CAN_IF2CMD_MSG_NUM_M) |
//            (uint32_t)CAN_IF2CMD_ARB);
//
//
//    //
//    // Wait for busy bit to clear
//    while(CanaRegs.CAN_IF2CMD.bit.Busy)//(HWREGH(base + CAN_O_IF2CMD) & CAN_IF2CMD_BUSY) == CAN_IF2CMD_BUSY)
//    {
//       ;
//    }
//    // Read out the IF control Register.
//    //msgCtrl = HWREGH(base + CAN_O_IF2MCTL);
//
//    msgCtrl = CanaRegs.CAN_IF2MCTL.all;
//
//    // See if there is new data available.
//    //
//    if(CanaRegs.CAN_IF2MCTL.bit.NewDat == 1)//(msgCtrl & CAN_IF2MCTL_NEWDAT) == CAN_IF2MCTL_NEWDAT)
//    {
//        //
//        // Read out the data from the CAN registers.
//        //
//       // CAN_readDataReg(msgData, (base + CAN_O_IF2DATA),
//       //                 ((uint32_t)msgCtrl & CAN_IF2MCTL_DLC_M));
//        uint16_t index;
//        for(index = 0; index < messageSize; index++)
//        {
//            switch(index)
//            {
//                case 0:
//                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATA.bit.Data_0;
//                break;
//                case 1:
//                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATA.bit.Data_1;
//                break;
//                case 2:
//                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATA.bit.Data_2;
//                break;
//                case 3:
//                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATA.bit.Data_3;
//                break;
//                case 4:
//                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATB.bit.Data_4;
//                break;
//
//                case 5:
//                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATB.bit.Data_5;
//                break;
//                case 6:
//                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATB.bit.Data_6;
//                break;
//                case 7:
//                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATB.bit.Data_7;
//                break;
//            }
//        }
//        status = true;
//        // Now clear out the new data flag
//
//
////        memcpy(msgData, garrCanaRXMsgData, 8);
//
//        CanaRegs.CAN_IF2CMD.all = ((uint32_t)CAN_IF2CMD_TXRQST |(objID & CAN_IF2CMD_MSG_NUM_M));
//       // HWREG_BP(base + CAN_O_IF2CMD) = ((uint32_t)CAN_IF2CMD_TXRQST |
//       //                                 (objID & CAN_IF2CMD_MSG_NUM_M));
//
//        // Wait for busy bit to clear
//        //
//        //while((HWREGH(base + CAN_O_IF2CMD) & CAN_IF2CMD_BUSY) ==
//        //               CAN_IF2CMD_BUSY)
//        while(CanaRegs.CAN_IF2CMD.bit.Busy)
//        {
//           ;
//        }
//    }
//    else
//    {
//        status = false;
//    }
//
//    return(status);
//
//}

bool CANA_readMessage(uint32_t base, uint32_t objID,DevCanCommData_t *msgData)
{
    uint32_t messageid;
    bool status;

    CanaRegs.CAN_IF2CMD.all = ((uint32_t)CAN_IF2CMD_DATA_A | (uint32_t)CAN_IF2CMD_DATA_B |
            (uint32_t)CAN_IF2CMD_CONTROL | (objID & CAN_IF2CMD_MSG_NUM_M) |
            (uint32_t)CAN_IF2CMD_ARB);


    // Wait for busy bit to clear
    while(CanaRegs.CAN_IF2CMD.bit.Busy)//(HWREGH(base + CAN_O_IF2CMD) & CAN_IF2CMD_BUSY) == CAN_IF2CMD_BUSY)
    {
       ;
    }
    // Read out the IF control Register.
    if(CanaRegs.CAN_IF2MCTL.bit.NewDat == 1)//(msgCtrl & CAN_IF2MCTL_NEWDAT) == CAN_IF2MCTL_NEWDAT)
    {
        status = true;
        uint16_t index;
        for(index = 0; index < messageSize; index++)
        {
            switch(index)
            {
                case 0:
                    msgData->data[index] = CanaRegs.CAN_IF2DATA.bit.Data_0;
                break;
                case 1:
                    msgData->data[index] = CanaRegs.CAN_IF2DATA.bit.Data_1;
                break;
                case 2:
                    msgData->data[index] = CanaRegs.CAN_IF2DATA.bit.Data_2;
                break;
                case 3:
                    msgData->data[index] = CanaRegs.CAN_IF2DATA.bit.Data_3;
                break;
                case 4:
                    msgData->data[index] = CanaRegs.CAN_IF2DATB.bit.Data_4;
                break;

                case 5:
                    msgData->data[index] = CanaRegs.CAN_IF2DATB.bit.Data_5;
                break;
                case 6:
                    msgData->data[index] = CanaRegs.CAN_IF2DATB.bit.Data_6;
                break;
                case 7:
                    msgData->data[index] = CanaRegs.CAN_IF2DATB.bit.Data_7;
                break;
            }
        }
        // Now clear out the new data flag
//        memcpy(msgData, garrCanaRXMsgData, 8);
        CanaRegs.CAN_IF2CMD.all = ((uint32_t)CAN_IF2CMD_TXRQST |(objID & CAN_IF2CMD_MSG_NUM_M));
       // HWREG_BP(base + CAN_O_IF2CMD) = ((uint32_t)CAN_IF2CMD_TXRQST |
       //                                 (objID & CAN_IF2CMD_MSG_NUM_M));

        // Wait for busy bit to clear
        //
        //while((HWREGH(base + CAN_O_IF2CMD) & CAN_IF2CMD_BUSY) ==
        //               CAN_IF2CMD_BUSY)
        while(CanaRegs.CAN_IF2CMD.bit.Busy)
        {
           ;
        }
    }
    else
    {
        status = false;
    }

    messageid = CanaRegs.CAN_IF2ARB.bit.ID;
    msgData->can_id.id = messageid;

    msgData->canx = EN_DEV_CAN0;

    if(CanaRegs.CAN_IF2ARB.bit.Xtd == EN_DEV_CAN_TYPE_EXT)
    {
        msgData->can_type =  EN_DEV_CAN_TYPE_EXT;
    }
    else if(CanaRegs.CAN_IF2ARB.bit.Xtd == EN_DEV_CAN_TYPE_STD)
    {
        msgData->can_type =  EN_DEV_CAN_TYPE_STD;
    }
    msgData->data_len =  CanaRegs.CAN_IF2MCTL.bit.DLC;//8;

    return(status);
}

bool CANB_readMessage(uint32_t base, uint32_t objID,DevCanCommData_t *msgData)
{

    uint32_t messageid;
    bool status;


    CanbRegs.CAN_IF2CMD.all = ((uint32_t)CAN_IF2CMD_DATA_A | (uint32_t)CAN_IF2CMD_DATA_B |
            (uint32_t)CAN_IF2CMD_CONTROL | (objID & CAN_IF2CMD_MSG_NUM_M) |
            (uint32_t)CAN_IF2CMD_ARB);


    // Wait for busy bit to clear
    while(CanbRegs.CAN_IF2CMD.bit.Busy)//(HWREGH(base + CAN_O_IF2CMD) & CAN_IF2CMD_BUSY) == CAN_IF2CMD_BUSY)
    {
       ;
    }
    // Read out the IF control Register.
    if(CanbRegs.CAN_IF2MCTL.bit.NewDat == 1)//(msgCtrl & CAN_IF2MCTL_NEWDAT) == CAN_IF2MCTL_NEWDAT)
    {
        uint16_t index;

        status = true;
        for(index = 0; index < messageSize; index++)
        {
            switch(index)
            {
                case 0:
                    msgData->data[index] = CanbRegs.CAN_IF2DATA.bit.Data_0;
                break;
                case 1:
                    msgData->data[index] = CanbRegs.CAN_IF2DATA.bit.Data_1;
                break;
                case 2:
                    msgData->data[index] = CanbRegs.CAN_IF2DATA.bit.Data_2;
                break;
                case 3:
                    msgData->data[index] = CanbRegs.CAN_IF2DATA.bit.Data_3;
                break;
                case 4:
                    msgData->data[index] = CanbRegs.CAN_IF2DATB.bit.Data_4;
                break;

                case 5:
                    msgData->data[index] = CanbRegs.CAN_IF2DATB.bit.Data_5;
                break;
                case 6:
                    msgData->data[index] = CanbRegs.CAN_IF2DATB.bit.Data_6;
                break;
                case 7:
                    msgData->data[index] = CanbRegs.CAN_IF2DATB.bit.Data_7;
                break;
            }
        }


        // Now clear out the new data flag

//        memcpy(msgData, garrCanaRXMsgData, 8);
        CanbRegs.CAN_IF2CMD.all = ((uint32_t)CAN_IF2CMD_TXRQST |(objID & CAN_IF2CMD_MSG_NUM_M));
       // HWREG_BP(base + CAN_O_IF2CMD) = ((uint32_t)CAN_IF2CMD_TXRQST |
       //                                 (objID & CAN_IF2CMD_MSG_NUM_M));

        // Wait for busy bit to clear
        //
        //while((HWREGH(base + CAN_O_IF2CMD) & CAN_IF2CMD_BUSY) ==
        //               CAN_IF2CMD_BUSY)
        while(CanbRegs.CAN_IF2CMD.bit.Busy)
        {
           ;
        }
    }
    else
    {
        status = false;
    }

    messageid = CanbRegs.CAN_IF2ARB.bit.ID;
    msgData->can_id.id = messageid;

    msgData->canx = EN_DEV_CAN1;
    msgData->data_len =  CanbRegs.CAN_IF2MCTL.bit.DLC;//8;

    if(CanbRegs.CAN_IF2ARB.bit.Xtd == EN_DEV_CAN_TYPE_EXT)
    {
        msgData->can_type =  EN_DEV_CAN_TYPE_EXT;
    }
    else if(CanbRegs.CAN_IF2ARB.bit.Xtd == EN_DEV_CAN_TYPE_STD)
    {
        msgData->can_type =  EN_DEV_CAN_TYPE_STD;
    }

    return(status);
}




//bool CANB_readMessage(uint32_t base, uint32_t objID,uint16_t *msgData)
//{
//    bool status;
//    uint16_t msgCtrl = 0U;
//
//    CanbRegs.CAN_IF2CMD.all = ((uint32_t)CAN_IF2CMD_DATA_A | (uint32_t)CAN_IF2CMD_DATA_B |
//            (uint32_t)CAN_IF2CMD_CONTROL | (objID & CAN_IF2CMD_MSG_NUM_M) |
//            (uint32_t)CAN_IF2CMD_ARB);
//
//
//    // Wait for busy bit to clear
//    while(CanbRegs.CAN_IF2CMD.bit.Busy)//(HWREGH(base + CAN_O_IF2CMD) & CAN_IF2CMD_BUSY) == CAN_IF2CMD_BUSY)
//    {
//       ;
//    }
//    // Read out the IF control Register.
//    //msgCtrl = HWREGH(base + CAN_O_IF2MCTL);
//    msgCtrl = CanbRegs.CAN_IF2MCTL.all;
//    // See if there is new data available.
//    if(CanbRegs.CAN_IF2MCTL.bit.NewDat == 1)//(msgCtrl & CAN_IF2MCTL_NEWDAT) == CAN_IF2MCTL_NEWDAT)
//    {
//        // Read out the data from the CAN registers.
//        //
//       // CAN_readDataReg(msgData, (base + CAN_O_IF2DATA),
//       //                 ((uint32_t)msgCtrl & CAN_IF2MCTL_DLC_M));
//        uint16_t index;
//        for(index = 0; index < messageSize; index++)
//        {
//            switch(index)
//            {
//                case 0:
//                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATA.bit.Data_0;
//                break;
//                case 1:
//                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATA.bit.Data_1;
//                break;
//                case 2:
//                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATA.bit.Data_2;
//                break;
//                case 3:
//                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATA.bit.Data_3;
//                break;
//                case 4:
//                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATB.bit.Data_4;
//                break;
//
//                case 5:
//                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATB.bit.Data_5;
//                break;
//                case 6:
//                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATB.bit.Data_6;
//                break;
//                case 7:
//                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATB.bit.Data_7;
//                break;
//            }
//        }
//        status = true;
//        // Now clear out the new data flag
//
//
////        memcpy(msgData, garrCanaRXMsgData, 8);
//
//        CanbRegs.CAN_IF2CMD.all = ((uint32_t)CAN_IF2CMD_TXRQST |(objID & CAN_IF2CMD_MSG_NUM_M));
//       // HWREG_BP(base + CAN_O_IF2CMD) = ((uint32_t)CAN_IF2CMD_TXRQST |
//       //                                 (objID & CAN_IF2CMD_MSG_NUM_M));
//
//        // Wait for busy bit to clear
//        //
//        //while((HWREGH(base + CAN_O_IF2CMD) & CAN_IF2CMD_BUSY) ==
//        //               CAN_IF2CMD_BUSY)
//        while(CanbRegs.CAN_IF2CMD.bit.Busy)
//        {
//           ;
//        }
//    }
//    else
//    {
//        status = false;
//    }
//
//    return(status);
//
//}


//
// getCANMessage - Check the message object for new data.
//                 If new data, data written into array and return true.
//                 If no new data, return false.
//
bool getCANBMessage(uint32_t objID)
{
    bool status;

    // Use Shadow variable for IF2CMD. IF2CMD should be written to in
    // single 32-bit write.
    union CAN_IF2CMD_REG CAN_IF2CMD_SHADOW;

    // Set the Message Data A, Data B, and control values to be read
    // on request for data from the message object.
    CAN_IF2CMD_SHADOW.all = 0;
    CAN_IF2CMD_SHADOW.bit.Control = 1;
    CAN_IF2CMD_SHADOW.bit.DATA_A = 1;
    CAN_IF2CMD_SHADOW.bit.DATA_B = 1;

    // Transfer the message object to the message object IF register.
    CAN_IF2CMD_SHADOW.bit.MSG_NUM = objID;
    CanbRegs.CAN_IF2CMD.all = CAN_IF2CMD_SHADOW.all;

    // Wait for busy bit to clear.
    while(CanbRegs.CAN_IF2CMD.bit.Busy)
    {
    }

    // See if there is new data available.
    if(CanbRegs.CAN_IF2MCTL.bit.NewDat == 1)
    {
        // Populate Shadow Variable
        CAN_IF2CMD_SHADOW.all = CanbRegs.CAN_IF2CMD.all;
        // Clear New Data Flag
        CAN_IF2CMD_SHADOW.bit.TxRqst = 1;
        // Transfer the message object to the message object IF register.
        CAN_IF2CMD_SHADOW.bit.MSG_NUM = objID;
        CanbRegs.CAN_IF2CMD.all = CAN_IF2CMD_SHADOW.all;

        status = true;
    }
    else
    {
        status = false;
    }

    return(status);
}
