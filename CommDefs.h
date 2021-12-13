#ifndef COMMDEFS
#define COMMDEFS

#include "common.h"

/**
 * PROTOCOL
 * 
 * 1) choose new seq number (increase)
 * 2) send a frame
 *      listen for a clear wire
 * 		CSMA/CR - listen to the wire and in case of incorrect reading stop transmission. The lower ID device should win
 * 3) wait random time - random seed is a device ID 
 * 4) repeat steps 2-4 given number of times
 * 
 * recieve:
 * 1) frame witj incorrect CRC => reject
 * 2) check if pair SenderDevId/Seq was recently seen (keep a list of last pairs). If yes => reject
 * 3) execute
 */

#define CONTROLLER_DEVICE_ID 1
#define DEVICE_ID_BROADCAST 0xFF

#define NUMBER_OF_RETRANSMISSIONS 1
#define MAX_NUM_OF_RETRANSMISSIONS 5

#define FRAME_TRANSMISSION_TIME 20

// a double click - two short clicks occur before max time
#define DOUBLE_CLICK_TICKS_MAX   8   // 400ms 

// maximum number of nodes, should not be mode than 32 because of bitmaps
#define MAX_NUM_OF_NODES 32

#define NUM_OF_OUTPUTS 8

#define DEFAULT_TIMER 0xFFFF

/**
 * request the state of all outputs from a device
 * device should respoind with MESSAGE_TYPE_OVERVIEW_STATE_RESPONSE
 * 
 */
#define MESSAGE_TYPE_OVERVIEW_STATE_REQUEST 0x01

/**
 * Sent on request - actual state of all outputs
 * sent to the device requesting it by MESSAGE_TYPE_OVERVIEW_STATE_REQUEST 
 */
#define MESSAGE_TYPE_OVERVIEW_STATE_RESPONSE 0x02
typedef struct 
{
  uint8_t  PowerState;      // state of 8 outputs, 1 means it is on 
  uint8_t  TimerState;      // state of 8 timers, 1 means the output is on but there's timer pending, no timer value here
} tMessageTypeOverviewStateResponse;


/**
 * request the state of an ouput from a device
 * device should respoind with MESSAGE_TYPE_OUTPUT_STATE_REQUEST
 */
#define MESSAGE_TYPE_OUTPUT_STATE_REQUEST 0x03
typedef struct 
{
  uint8_t  OutputID;      // id of an output 
} tMessageTypeOutputStateRequest;

/**
 * Sent on request - actual state of an output
 * sent to the device requesting it by MESSAGE_TYPE_OUTPUT_STATE_RESPONSE 
 */
#define MESSAGE_TYPE_OUTPUT_STATE_RESPONSE 0x04
typedef struct 
{
  uint8_t  OutputID;        // id of an output
  uint8_t  PowerState;      // state - 1 means on
  uint16_t TimerValue;      // state of a timer. 0 means there's no timer pending
  uint16_t DefaultTimer;    // a default timer for the output. 
} tMessageTypeOutputStateResponse;

/**
 * Sent by the controller - arbitrary set state of a single output
 */
#define MESSAGE_TYPE_SET_OUTPUT 0x05
typedef struct 
{
  uint8_t  OutId;        // the output id to be set
  uint8_t  State;        // state 0 or 1 to be set, where "1" means an active state
  uint16_t Timer;        // timer when the output should be turned off. In seconds, 0 means forever, 0xFFFF means default timer value
} tMessageTypeSetOutput;

/**
 * Sent by the controller - arbitrary set state of a single output
 * this is a broadcast frame
 */
#define MESSAGE_BUTTON_PRESS 0x06
typedef struct 
{
  uint16_t ShortClick;   // bitmap of new buttons clicked
  uint16_t LongClick;    // bitmap of new long clicks (note! it will always be preceded by "short click"
  uint16_t DoubleClick;  // bitmap of new double clicks
  uint8_t  ForceSrcId;   // if != 0, the reciever will take this as a sender ID. If == 0, the real sender will be taken (for compatibility)
} tMessageTypeButtonPress;

/**
 * Set a reaction for a button pressed 
 * send by the controller to setup 
 */
#define MESSAGE_TYPE_SET_ACTION 0x07

#define BUTTON_TRIGGER_TYPE_CLICK            0
#define BUTTON_TRIGGER_TYPE_LONG_CLICK       1
#define BUTTON_TRIGGER_TYPE_DOUBLE_CLICK     2
#define BUTTON_TRIGGER_TYPE_ANY_CLICK        3

#define BUTTON_ACTION_TYPE_TOGGLE     0
#define BUTTON_ACTION_TYPE_ON         1
#define BUTTON_ACTION_TYPE_OFF        2

typedef struct 
{
  uint8_t  OutId;         // the output id to be set
  uint8_t  SenderDevID;   // the ID of the sender of a MESSAGE_BUTTON_PRESS message
  uint8_t  ButtonId    : 4, // the ID of the button triggering the action
           TriggerType : 2, // type of button action -  BUTTON_TRIGGER_TYPE*
           ActionType  : 2; // type of action - BUTTON_ACTION_TYPE*
  uint16_t Timer;         // in case of button is set to "1", by on or toggle, number of seconds after it should be turned off
  uint8_t  OutputsMask;   // bitmap of other outputs to be checked before action
  uint8_t  OutputsStates; // required states of outputs. All masked bits MUST be zero
} tMessageTypeSetAction;


/**
 * Erase all actions - clear Eeprom actions map, all actions need to be re-programmed
 */
#define MESSAGE_TYPE_CLEAR_ACTIONS 0x08


/**
 * dest node must response with MESSAGE_TYPE_FW_VERSION_RESPONSE  to the controller
 * no payload data
 */
#define MESSAGE_TYPE_FW_VERSION_REQUEST 0x09

/**
 * firmware version
 */
#define MESSAGE_TYPE_FW_VERSION_RESPONSE 0x0A
typedef struct 
{
  uint8_t Major;
  uint8_t Minor;
  uint8_t Patch;
} tMessageTypeFwVesionResponse;

/**
 * get a CRC of all eeprom
 * sent by the controller, the node must respond with MESSAGE_TYPE_EEPROM_CRC_RESPONSE
 */
#define MESSAGE_TYPE_EEPROM_CRC_REQUEST 0x0B
#define MESSAGE_TYPE_EEPROM_CRC_RESPONSE 0x0C
typedef struct 
{
  uint8_t  NumOfActions;
  uint16_t EepromCRC;
} tMessageTypeEepromCRCResponse;

/**
 * set runtime parameters
 */
#define MESSAGE_TYPE_SET_PARAMS 0x0D
typedef struct 
{
  uint8_t DoubleClickTime;              // max number of 50ms ticks for double click
  uint8_t NumOfRetransmissions;         // number of retransmissions
  uint8_t MaxNumOfRetransmissions;      // max number of retransmissions in case of collisions
} tMessageTypeSetParams;


/**
 * force the note to reset. May be sent as a broadcast
 */
#define MESSAGE_TYPE_FORCE_RESET 0x0E

#define MESSAGE_TYPE_SET_DEFAULT_TIMER 0x0F
typedef struct 
{
  uint8_t  OutputID;              // output ID 
  uint16_t DefaultTimerValue;     // timer value
} tMessageTypeSetDefaultTimer;

#define MESSAGE_TYPE_DEFAULT_TIMER_REQUEST 0x10
typedef struct 
{
  uint8_t  OutputID;      // id of an output 
} tMessageTypeDefaultTimerRequest;


#define MESSAGE_TYPE_DEFAULT_TIMER_RESPONSE 0x11
typedef struct 
{
  uint8_t  OutputID;      // id of an output
  uint16_t DefTimerValue; // default timer. 0 means there's no timer (set forever)
} tMessageTypeDefaultTimerResponse;




#define COMMUNICATION_PAYLOAD_DATA_SIZE 8
/**
 * Communication frame
 */
typedef struct
{
  uint8_t SenderDevId;    // id of the sender
  uint8_t DstDevId;       // device ID the message is sent to or broadcast
  uint8_t Seq;            // seq number. Retransmitted frame should have the same seq
  uint8_t MessageType;    // MESSAGE_TYPE*

  uint8_t Data[COMMUNICATION_PAYLOAD_DATA_SIZE];  // data structure, to be mapped on tMessageType* structure

  uint16_t crc;   // CRC, frame will be rejected if CRC is incorrect
} tCommunicationFrame;

#endif
