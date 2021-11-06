#ifndef EEPROM_DEFS
#define EEPROM_DEFS

#include <EEPROM.h>
#include "common.h"
#include "CommDefs.h"

#define ACTION_TABLE_SIZE 139   // eeprom is full
#define EEPROM_CANNARY 0xAC     // change if update changes EEPROM structure

typedef struct __tEeprom
{
  uint8_t DeviceID;  // MUST be at the 1st position
#ifndef CONTROLLER
  uint16_t InputPolarity;      // 0 means negative trigger, 1 positive trigger
#endif
  uint8_t CANNARY;   // a cannary, if is not set to EEPROM_CANNARY the eeprom should be set to default
  uint8_t DoubleClickTime;    // an externally provided double click time
  uint8_t NumOfRetransmissions;
  uint8_t MaxNumOfRetransmissions;
#ifdef CONTROLLER
  uint8_t IP[4];
  uint8_t IPMask[4];
  uint8_t Gateway[4];
  uint8_t DNS[4];
  uint8_t MAC[6];
#endif
  uint16_t DefaultTimerValue[NUM_OF_OUTPUTS];
  uint8_t ActionTableUsage;
  tMessageTypeSetAction ActionTableEntry[ACTION_TABLE_SIZE];
};

#define EEPROM_DEVICE_ID_OFFSET offsetof(__tEeprom,DeviceID)

#ifndef CONTROLLER
#define EEPROM_INPUT_POLARITY_OFFSET offsetof(__tEeprom,InputPolarity)
#else
#define EEPROM_INPUT_POLARITY_OFFSET 0
#endif

#define EEPROM_CANNARY_OFFSET offsetof(__tEeprom,CANNARY)
#define EEPROM_DOUBLE_CLICK_TIME_OFFSET offsetof(__tEeprom,DoubleClickTime)
#define EEPROM_NUM_OF_RETRANSMISSIONS offsetof(__tEeprom,NumOfRetransmissions)
#define EEPROM_MAX_NUM_OF_RETRANSMISSIONS offsetof(__tEeprom,MaxNumOfRetransmissions)

#ifdef CONTROLLER
#define EEPROM_IP offsetof(__tEeprom,IP[0])
#define EEPROM_IPMASK offsetof(__tEeprom,IPMask[0])
#define EEPROM_GATEWAY offsetof(__tEeprom,Gateway[0])
#define EEPROM_DNS offsetof(__tEeprom,DNS[0])
#define EEPROM_MAC offsetof(__tEeprom,MAC[0])
#else
#define EEPROM_IP 0
#define EEPROM_IPMASK 0
#define EEPROM_GATEWAY 0
#define EEPROM_DNS 0
#define EEPROM_MAC 0
#endif

#define EEPROM_DEFAULT_TIMER_VALUE_OFFSET offsetof(__tEeprom,DefaultTimerValue[0])
#define EEPROM_ACTION_TABLE_USAGE_OFFSET offsetof(__tEeprom,ActionTableUsage)
#define EEPROM_ACTION_TABLE_OFFSET offsetof(__tEeprom,ActionTableEntry[0])
#define EEPROM_ACTION_TABLE_SIZE sizeof(tMessageTypeSetAction)

void SetDefaultEEPromValues();

#endif //EEPROM_DEFS
