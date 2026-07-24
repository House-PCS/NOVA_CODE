#include "F28x_Project.h"
#include "can.h"

//
// Globals
//
volatile unsigned long i;
unsigned char garrTXMsgData[8] = {0x55, 0xAA, 0x00, 0x00, 0x55, 0xAA, 0x00, 0x00}; // TX Data
unsigned char garrCanaRXMsgData[8] = {0, 0, 0, 0, 0, 0, 0, 0};         // RX Data
unsigned char garrCanbRXMsgData[8] = {0, 0, 0, 0, 0, 0, 0, 0};         // RX Data
uint32_t messageSize = sizeof(garrTXMsgData);          // Message Size (DLC)

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

        //! Receive message object.
        MSG_OBJ_TYPE_RECEIVE
}
msgObjType;

//
// Function Prototypes
//
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
        setCANABitRate(200000000, 500000);
        setCANBBitRate(200000000, 500000);


        CanaRegs.CAN_CTL.bit.Init = 0;
        CanaRegs.CAN_CTL.bit.CCE = 0;
        CanbRegs.CAN_CTL.bit.Init = 0;
        CanbRegs.CAN_CTL.bit.CCE = 0;

    //
    // Initialize the message object that will be used for sending CAN
    // messages.
    //
        setupCANAMessageObject(CAN_TX_MSG_OBJ, CAN_MSG_ID, MSG_OBJ_TYPE_TRANSMIT);
        setupCANBMessageObject(CAN_TX_MSG_OBJ, CAN_MSG_ID, MSG_OBJ_TYPE_TRANSMIT);

    //
    // Initialize the message object that will be used for receiving CAN
    // messages.
    //
        setupCANAMessageObject(CANA_RX_MSG_OBJ, CAN_MSG_REC_ID, MSG_OBJ_TYPE_RECEIVE);
        setupCANBMessageObject(CANB_RX_MSG_OBJ, CAN_MSG_REC_ID, MSG_OBJ_TYPE_RECEIVE);


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
        sendCANAMessage(CAN_TX_MSG_OBJ);
        sendCANBMessage(CAN_TX_MSG_OBJ);

        //
        // Delay 0.25 second before continuing
        //
//        DELAY_US(250000);

        //
        // Get the receive message
        //
//        getCANAMessage(CANA_RX_MSG_OBJ);
//        getCANBMessage(CANB_RX_MSG_OBJ);

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
//        if(garrCanaRXMsgData[0] == 0x55)
//        {
//            while(1);
//        }
//        if(garrCanbRXMsgData[0] == 0x55)
//        {
//            while(1);
//        }
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
    // Clear and Write out the registers to program the message object.
    //
    CAN_IF1CMD_SHADOW.all = 0;
    CanaRegs.CAN_IF1MSK.all = 0;
    CanaRegs.CAN_IF1ARB.all = 0;
    CanaRegs.CAN_IF1MCTL.all = 0;

    //
    // Set the Control, Mask, and Arb bit so that they get transferred to the
    // Message object.
    //
    CAN_IF1CMD_SHADOW.bit.Control = 1;
    CAN_IF1CMD_SHADOW.bit.Arb = 1;
    CAN_IF1CMD_SHADOW.bit.Mask = 1;
    CAN_IF1CMD_SHADOW.bit.DIR = 1;

    //
    // Set direction to transmit
    //
    if(msgType == MSG_OBJ_TYPE_TRANSMIT)
    {
        CanaRegs.CAN_IF1ARB.bit.Dir = 1;
    }

    //
    // Set Message ID (this example assumes 11 bit ID mask)
    //
    CanaRegs.CAN_IF1ARB.bit.ID = (msgID << CAN_MSG_ID_SHIFT);
    CanaRegs.CAN_IF1ARB.bit.MsgVal = 1;
    CanaRegs.CAN_IF1ARB.bit.Xtd = 1;

    //
    // Set the data length since this is set for all transfers.  This is
    // also a single transfer and not a FIFO transfer so set EOB bit.
    //
    CanaRegs.CAN_IF1MCTL.bit.DLC = messageSize;
    CanaRegs.CAN_IF1MCTL.bit.EoB = 1;

    //
    // Transfer data to message object RAM
    //
    CAN_IF1CMD_SHADOW.bit.MSG_NUM = objID;
    CanaRegs.CAN_IF1CMD.all = CAN_IF1CMD_SHADOW.all;
}


//
// setupMessageObject - Setup message object as Transmit or Receive
//
void setupCANBMessageObject(uint32_t objID, uint32_t msgID, msgObjType msgType)
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
    // Clear and Write out the registers to program the message object.
    //
    CAN_IF1CMD_SHADOW.all = 0;
    CanbRegs.CAN_IF1MSK.all = 0;
    CanbRegs.CAN_IF1ARB.all = 0;
    CanbRegs.CAN_IF1MCTL.all = 0;

    //
    // Set the Control, Mask, and Arb bit so that they get transferred to the
    // Message object.
    //
    CAN_IF1CMD_SHADOW.bit.Control = 1;
    CAN_IF1CMD_SHADOW.bit.Arb = 1;
    CAN_IF1CMD_SHADOW.bit.Mask = 1;
    CAN_IF1CMD_SHADOW.bit.DIR = 1;

    //
    // Set direction to transmit
    //
    if(msgType == MSG_OBJ_TYPE_TRANSMIT)
    {
        CanbRegs.CAN_IF1ARB.bit.Dir = 1;
    }

    //
    // Set Message ID (this example assumes 11 bit ID mask)
    //
    CanbRegs.CAN_IF1ARB.bit.ID = (msgID << CAN_MSG_ID_SHIFT);
    CanbRegs.CAN_IF1ARB.bit.MsgVal = 1;
    CanbRegs.CAN_IF1ARB.bit.Xtd = 1;

    //
    // Set the data length since this is set for all transfers.  This is
    // also a single transfer and not a FIFO transfer so set EOB bit.
    //
    CanbRegs.CAN_IF1MCTL.bit.DLC = messageSize;
    CanbRegs.CAN_IF1MCTL.bit.EoB = 1;

    //
    // Transfer data to message object RAM
    //
    CAN_IF1CMD_SHADOW.bit.MSG_NUM = objID;
    CanbRegs.CAN_IF1CMD.all = CAN_IF1CMD_SHADOW.all;
}

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
    CanaRegs.CAN_IF1CMD.all = CAN_IF1CMD_SHADOW.all;
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
}


//
// getCANMessage - Check the message object for new data.
//                 If new data, data written into array and return true.
//                 If no new data, return false.
//
bool getCANAMessage(uint32_t objID)
{
    bool status;

    //
    // Use Shadow variable for IF2CMD. IF2CMD should be written to in
    // single 32-bit write.
    //
    union CAN_IF2CMD_REG CAN_IF2CMD_SHADOW;

    //
    // Set the Message Data A, Data B, and control values to be read
    // on request for data from the message object.
    //
    CAN_IF2CMD_SHADOW.all = 0;
    CAN_IF2CMD_SHADOW.bit.Control = 1;
    CAN_IF2CMD_SHADOW.bit.DATA_A = 1;
    CAN_IF2CMD_SHADOW.bit.DATA_B = 1;

    //
    // Transfer the message object to the message object IF register.
    //
    CAN_IF2CMD_SHADOW.bit.MSG_NUM = objID;
    CanaRegs.CAN_IF2CMD.all = CAN_IF2CMD_SHADOW.all;

    //
    // Wait for busy bit to clear.
    //
    while(CanaRegs.CAN_IF2CMD.bit.Busy)
    {
    }

    //
    // See if there is new data available.
    //
    if(CanaRegs.CAN_IF2MCTL.bit.NewDat == 1)
    {
        //
        // Read out the data from the CAN registers.
        //
        uint16_t index;
        for(index = 0; index < messageSize; index++)
        {
            switch(index)
            {
                case 0:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATA.bit.Data_0;
                break;
                case 1:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATA.bit.Data_1;
                break;
                case 2:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATA.bit.Data_2;
                break;
                case 3:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATA.bit.Data_3;
                break;
                case 4:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATB.bit.Data_4;
                break;
                case 5:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATB.bit.Data_5;
                break;
                case 6:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATB.bit.Data_6;
                break;
                case 7:
                    garrCanaRXMsgData[index] = CanaRegs.CAN_IF2DATB.bit.Data_7;
                break;
            }
        }

        //
        // Populate Shadow Variable
        //
        CAN_IF2CMD_SHADOW.all = CanaRegs.CAN_IF2CMD.all;

        //
        // Clear New Data Flag
        //
        CAN_IF2CMD_SHADOW.bit.TxRqst = 1;

        //
        // Transfer the message object to the message object IF register.
        //
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

//
// getCANMessage - Check the message object for new data.
//                 If new data, data written into array and return true.
//                 If no new data, return false.
//
bool getCANBMessage(uint32_t objID)
{
    bool status;

    //
    // Use Shadow variable for IF2CMD. IF2CMD should be written to in
    // single 32-bit write.
    //
    union CAN_IF2CMD_REG CAN_IF2CMD_SHADOW;

    //
    // Set the Message Data A, Data B, and control values to be read
    // on request for data from the message object.
    //
    CAN_IF2CMD_SHADOW.all = 0;
    CAN_IF2CMD_SHADOW.bit.Control = 1;
    CAN_IF2CMD_SHADOW.bit.DATA_A = 1;
    CAN_IF2CMD_SHADOW.bit.DATA_B = 1;

    //
    // Transfer the message object to the message object IF register.
    //
    CAN_IF2CMD_SHADOW.bit.MSG_NUM = objID;
    CanbRegs.CAN_IF2CMD.all = CAN_IF2CMD_SHADOW.all;

    //
    // Wait for busy bit to clear.
    //
    while(CanbRegs.CAN_IF2CMD.bit.Busy)
    {
    }

    //
    // See if there is new data available.
    //
    if(CanbRegs.CAN_IF2MCTL.bit.NewDat == 1)
    {
        //
        // Read out the data from the CAN registers.
        //
        uint16_t index;
        for(index = 0; index < messageSize; index++)
        {
            switch(index)
            {
                case 0:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATA.bit.Data_0;
                break;
                case 1:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATA.bit.Data_1;
                break;
                case 2:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATA.bit.Data_2;
                break;
                case 3:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATA.bit.Data_3;
                break;
                case 4:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATB.bit.Data_4;
                break;
                case 5:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATB.bit.Data_5;
                break;
                case 6:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATB.bit.Data_6;
                break;
                case 7:
                    garrCanbRXMsgData[index] = CanbRegs.CAN_IF2DATB.bit.Data_7;
                break;
            }
        }

        //
        // Populate Shadow Variable
        //
        CAN_IF2CMD_SHADOW.all = CanbRegs.CAN_IF2CMD.all;

        //
        // Clear New Data Flag
        //
        CAN_IF2CMD_SHADOW.bit.TxRqst = 1;

        //
        // Transfer the message object to the message object IF register.
        //
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
