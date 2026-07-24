#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "dev_can_comm.h"

#define TXCOUNT  100
//#define CAN_MSG_ID              0x111
#define CAN_TX_MSG_ID1           0x18A08001
#define CAN_TX_MSG_ID4           0x18A18001
#define CAN_TX_MSG_ID5           0x18A28001
#define CAN_TX_MSG_ID6           0x18A00180
#define CAN_TX_MSG_ID7           0x18A10180
#define CAN_TX_MSG_ID8           0x18A20180
#define CAN_TX_MSG_ID9           0x18AF8001
#define CAN_TX_MSG_ID10          0x18AF0180
#define CAN_TX_MSG_ID11          0x18A30180


#define CAN_TX_MSG_OBJ1          1
#define CAN_TX_MSG_OBJ4          4
#define CAN_TX_MSG_OBJ5          5
#define CAN_TX_MSG_OBJ6          6
#define CAN_TX_MSG_OBJ7          7
#define CAN_TX_MSG_OBJ8          8
#define CAN_TX_MSG_OBJ9          9
#define CAN_TX_MSG_OBJ10         10
#define CAN_TX_MSG_OBJ11         11


#define CAN_MSG_REC_ID          0x18A00000  //0x1800C4A0

#define CANA_RX_MSG_OBJ         2
#define CANB_RX_MSG_OBJ         3
#define CANA_BASE               0x00048000
#define CANB_BASE               0x0004A000
#define CAN_MAX_BIT_DIVISOR     (13)   // The maximum CAN bit timing divisor
#define CAN_MIN_BIT_DIVISOR     (5)    // The minimum CAN bit timing divisor
#define CAN_MAX_PRE_DIVISOR     (1024) // The maximum CAN pre-divisor
#define CAN_MIN_PRE_DIVISOR     (1)    // The minimum CAN pre-divisor
#define CAN_BTR_BRP_M           (0x3F)
#define CAN_BTR_BRPE_M          (0xF0000)
#define CAN_MSG_ID_SHIFT        0U


typedef enum
{
    //! Set the message ID frame to standard.
    CAN_MSG_FRAME_STD,

    //! Set the message ID frame to extended.
    CAN_MSG_FRAME_EXT
} CAN_MsgFrameType;

typedef enum
{
    //! Transmit message object.
    CAN_MSG_OBJ_TYPE_TX,

    //! Transmit remote request message object
    CAN_MSG_OBJ_TYPE_TX_REMOTE,

    //! Receive message object.
    CAN_MSG_OBJ_TYPE_RX,

    //! Remote frame receive remote, with auto-transmit message object.
    CAN_MSG_OBJ_TYPE_RXTX_REMOTE
} CAN_MsgObjType;



void CanInit(void);
void CANTransmitMsg(void);
void sendCANAMessage(uint32_t objID);
void sendCANBMessage(uint32_t objID);
bool getCANAMessage(uint32_t objID);
bool getCANBMessage(uint32_t objID);
void CanTxDataCopy(uint16_t *p_data);

bool CANA_readMessage(uint32_t base, uint32_t objID,DevCanCommData_t *msgData);

bool CANB_readMessage(uint32_t base, uint32_t objID,DevCanCommData_t *msgData);

uint16_t MsgIdFindObjId(uint32_t msg_id);

//extern bool CAN_readMessage(uint32_t base, uint32_t objID,uint16_t *msgData);


extern uint16_t garrTXMsgData[8]; // TX Data
extern uint16_t garrCanaRXMsgData[8];
extern uint16_t garrCanbRXMsgData[8];
#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif
