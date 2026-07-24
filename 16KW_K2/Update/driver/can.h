#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TXCOUNT  100
//#define CAN_MSG_ID              0x111
#define CAN_MSG_ID              0x18FF1234
#define CAN_MSG_REC_ID          0x1800C4A0
#define CAN_TX_MSG_OBJ          1
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

void CanInit(void);
void CANTransmitMsg(void);
void sendCANAMessage(uint32_t objID);
void sendCANBMessage(uint32_t objID);
bool getCANAMessage(uint32_t objID);
bool getCANBMessage(uint32_t objID);

extern unsigned char garrTXMsgData[8]; // TX Data
extern unsigned char garrCanaRXMsgData[8];
extern unsigned char garrCanbRXMsgData[8];
#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif
