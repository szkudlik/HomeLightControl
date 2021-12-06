#include "WorkerProcess.h"
#include "CommSender.h"
#include "OutputProcess.h"
#include "version.h"
#include "Eeprom.h"
#include <AceCRC.h>
#include "ResponseHandler.h"
#include "CommDefs.h"

using namespace ace_crc::crc16ccitt_nibble;



bool NodeScanTask::Process(uint32_t * pPeriod)
{
   *pPeriod = REQUEST_SENDING_PERIOD;

   if (mCurrentNodeID < MAX_NUM_OF_NODES)
   {
       // send a frame
       mCurrentNodeID++;
       Worker.SendMsgVersionRequest(mCurrentNodeID);  // staring from 1
   }
   else if (mCurrentNodeID == MAX_NUM_OF_NODES)
   {
      *pPeriod = RESPONSE_WAIT_PERIOD;
       mCurrentNodeID++;
   }
   else
   {
       // send result
       ResponseHandler::NodeScanResponse(mActiveNodesMap);
       return false;
   }

   return true;
}

void NodeScanTask::vVersionResponseHandler(uint8_t DevID, uint8_t Major, uint8_t Minor, uint8_t Patch)
{
   mActiveNodesMap |= 1 << (DevID-1);
}




// OVERVIEW STATE HANDSHAKE
bool WorkerProcess::SendMsgOverviewStateRequest(uint8_t RecieverID) 
{
#ifdef CONTROLLER
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_OVERVIEW_STATE_REQUEST");
  CommSender.Enqueue(RecieverID, MESSAGE_TYPE_OVERVIEW_STATE_REQUEST, 0, NULL);
#endif
  return true;
}

void WorkerProcess::HandleMsgOverviewStateRequest(uint8_t SenderID) 
{
  SendMsgOverviewStateResponse(SenderID,OutputProcess.GetOutputStateMap(),OutputProcess.GetOutputTimersStateMap());
}

bool WorkerProcess::SendMsgOverviewStateResponse(uint8_t RecieverID, uint8_t  PowerState, uint8_t  TimerState)
{
  tMessageTypeOverviewStateResponse Msg;
  Msg.PowerState = PowerState;
  Msg.TimerState = TimerState;
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_OVERVIEW_STATE_RESPONSE");
  CommSender.Enqueue(RecieverID, MESSAGE_TYPE_OVERVIEW_STATE_RESPONSE, sizeof(Msg), &Msg);  
  return true;
}

void WorkerProcess::HandleMsgOverviewStateResponse(uint8_t SenderID, tMessageTypeOverviewStateResponse* Message)
{
#ifdef CONTROLLER
  RespHandler.OverviewStateResponseHandler(SenderID,Message->PowerState,Message->TimerState);
#endif
}



// GET OUTPUT STATE HANDSHAKE
bool WorkerProcess::SendMsgOutputStateRequest(uint8_t RecieverID, uint8_t  OutputID) 
{
#ifdef CONTROLLER
  tMessageTypeOutputStateRequest Msg;
  Msg.OutputID = OutputID;
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_OUTPUT_STATE_REQUEST");
  CommSender.Enqueue(RecieverID, MESSAGE_TYPE_OUTPUT_STATE_REQUEST, sizeof(Msg), &Msg);
#endif
  return true;
};

void WorkerProcess::HandleMsgOutputStateRequest(uint8_t SenderID, tMessageTypeOutputStateRequest* Message)
{
  if (Message->OutputID < NUM_OF_OUTPUTS) 
  {
      uint16_t DefTimer;
      EEPROM.get(EEPROM_DEFAULT_TIMER_VALUE_OFFSET+Message->OutputID*(sizeof(uint16_t)),DefTimer);

      SendMsgOutputStateResponse(SenderID,Message->OutputID, OutputProcess.GetOutputState(Message->OutputID), OutputProcess.GetOutputTimer(Message->OutputID),DefTimer);
  }
}

bool WorkerProcess::SendMsgOutputStateResponse(uint8_t RecieverID, uint8_t  OutputID, uint8_t  PowerState, uint16_t TimerValue, uint16_t DefaultTimer)
{
  tMessageTypeOutputStateResponse Msg;
  Msg.OutputID = OutputID;
  Msg.PowerState = PowerState;
  Msg.TimerValue = TimerValue;
  Msg.DefaultTimer = DefaultTimer;
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_OUTPUT_STATE_RESPONSE");
  CommSender.Enqueue(RecieverID, MESSAGE_TYPE_OUTPUT_STATE_RESPONSE, sizeof(Msg), &Msg);  

  return true;
};

void WorkerProcess::HandleMsgOutputStateResponse(uint8_t SenderID, tMessageTypeOutputStateResponse* Message)
{
#ifdef CONTROLLER
  RespHandler.OutputStateResponseHandler(SenderID,Message->OutputID,Message->PowerState,Message->TimerValue,Message->DefaultTimer);
#endif
}

// SET OUTPUT
bool WorkerProcess::SendMsgSetOutput(uint8_t RecieverID, uint8_t  OutId, uint8_t  State, uint16_t Timer)
{
#ifdef CONTROLLER
  tMessageTypeSetOutput Message;
  Message.OutId = OutId;
  Message.State = State;
  Message.Timer = Timer;
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_SET_OUTPUT");
  CommSender.Enqueue(RecieverID, MESSAGE_TYPE_SET_OUTPUT, sizeof(Message), &Message); 
#endif  
  return true;
}

void WorkerProcess::HandleMsgSetOutput(uint8_t SenderID, tMessageTypeSetOutput* Message)
{
  OutputProcess.SetOutput(Message->OutId,Message->State,Message->Timer,tOutputProcess::ForceTimer);
}



// BUTTON  PRESS
bool WorkerProcess::SendMsgButtonPress(uint8_t RecieverID, uint8_t ForceSrcId, uint16_t ShortClick, uint16_t LongClick, uint16_t DoubleClick)
{
  tMessageTypeButtonPress Msg;
  Msg.ShortClick = ShortClick;
  Msg.LongClick  = LongClick;
  Msg.DoubleClick = DoubleClick;
  Msg.ForceSrcId = ForceSrcId;
  
  DEBUG_PRINTLN_3("===================>sending MESSAGE_BUTTON_PRESS");
  CommSender.Enqueue(RecieverID,MESSAGE_BUTTON_PRESS,sizeof(Msg),&Msg);
  return true;
};

void WorkerProcess::HandleMsgButtonPress(uint8_t SenderID, tMessageTypeButtonPress *Message)
{
#ifdef CONTROLLER 
#ifdef DEBUG_3
    RespHandler.print(F("Dev ID:"));
    RespHandler.print(SenderID,HEX);
    RespHandler.print(F(" ForcedSrc:"));
    RespHandler.print(Message->ForceSrcId,HEX);
    RespHandler.print(F(" short:"));
    RespHandler.print(Message->ShortClick,BIN);
    RespHandler.print(F(" long:"));
    RespHandler.print(Message->LongClick,BIN);
    RespHandler.print(F(" dbl:"));
    RespHandler.println(Message->DoubleClick,BIN);
#endif
#endif

  if (Message->ForceSrcId)
    SenderID = Message->ForceSrcId;
    
  // iterate through the action table
  tMessageTypeSetAction Action;
  uint8_t i = EEPROM.read(EEPROM_ACTION_TABLE_USAGE_OFFSET);
  // remeber output state BEFORE performing action set
  uint8_t OutputState = OutputProcess.GetOutputStateMap();
  while (i--)
  {
    EEPROM.get(EEPROM_ACTION_TABLE_OFFSET+(EEPROM_ACTION_TABLE_SIZE*i),Action);
    // does the sender ID match?
    if (Action.SenderDevID != SenderID) continue;
    
    // does the action type and button ID match?
    uint16_t ButtonMask = (1 << Action.ButtonId);
    if ( (BUTTON_TRIGGER_TYPE_ANY_CLICK == Action.TriggerType)    && (0 == (Message->ShortClick & ButtonMask)) && (0 == (Message->DoubleClick & ButtonMask))) continue;
    
    if ( (BUTTON_TRIGGER_TYPE_CLICK == Action.TriggerType)        && (0 == (Message->ShortClick & ButtonMask))) continue;
     
    if ( (BUTTON_TRIGGER_TYPE_LONG_CLICK == Action.TriggerType)   && (0 == (Message->LongClick & ButtonMask))) continue;
     
    if ( (BUTTON_TRIGGER_TYPE_DOUBLE_CLICK == Action.TriggerType) && (0 == (Message->DoubleClick & ButtonMask))) continue;
    
    // does the current output state match the mask?
    uint8_t OutputStateMasked = OutputState & Action.OutputsMask;
    if (OutputStateMasked != Action.OutputsStates) continue;

    if (Action.OutId  >= NUM_OF_OUTPUTS)
    {
      // drop it
      return;
    }

    uint16_t Timer = Action.Timer;
    if (DEFAULT_TIMER == Timer)
    {
      EEPROM.get(EEPROM_DEFAULT_TIMER_VALUE_OFFSET+Action.OutId*(sizeof(uint16_t)),Timer);
    }
    
    switch (Action.ActionType)
    {
      case BUTTON_ACTION_TYPE_ON: 
         // when "turn on" action is triggered bu a button, don't set a timer if it is shorter than current timer
        OutputProcess.SetOutput(Action.OutId,1,Timer,tOutputProcess::TimerLongerOnly);
        break;

      case BUTTON_ACTION_TYPE_OFF:
        OutputProcess.SetOutput(Action.OutId,0,0,tOutputProcess::ForceTimer);
        break;

      case BUTTON_ACTION_TYPE_TOGGLE:      
        OutputProcess.ToggleOutput(Action.OutId,Timer); 
        break;
    }
  }
}

bool WorkerProcess::SendMsgClearAllActions(uint8_t RecieverID)
{
#ifdef CONTROLLER
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_CLEAR_ACTIONS");
  CommSender.Enqueue(RecieverID,MESSAGE_TYPE_CLEAR_ACTIONS,0,NULL);
#endif  
  return true;
};
  
void WorkerProcess::HandleMsgClearAllActions(uint8_t SenderID)
{
    EEPROM.write(EEPROM_ACTION_TABLE_USAGE_OFFSET,0);
}

bool WorkerProcess::SendMsgAddAction(uint8_t RecieverID, uint8_t OutId, uint8_t SenderDevID, uint8_t ButtonId, uint8_t TriggerType, uint8_t ActionType, uint16_t Timer, uint8_t  OutputsMask, uint8_t  OutputsStates)
{
#ifdef CONTROLLER
  tMessageTypeSetAction Message;
  Message.OutId = OutId;
  Message.SenderDevID = SenderDevID;
  Message.ButtonId = ButtonId;
  Message.TriggerType = TriggerType;
  Message.ActionType = ActionType;
  Message.Timer = Timer;
  Message.OutputsMask = OutputsMask;
  Message.OutputsStates = OutputsStates;
  
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_SET_ACTION");
  CommSender.Enqueue(RecieverID,MESSAGE_TYPE_SET_ACTION,sizeof(Message),&Message);
#endif  
  return true;
};
  
void WorkerProcess::HandleMsgSetAction(uint8_t SenderID, tMessageTypeSetAction* Message)
{
  uint8_t ActionTableUsage = EEPROM.read(EEPROM_ACTION_TABLE_USAGE_OFFSET);
  if (ActionTableUsage < ACTION_TABLE_SIZE)
  {
    EEPROM.put(EEPROM_ACTION_TABLE_OFFSET+(EEPROM_ACTION_TABLE_SIZE*ActionTableUsage),*Message);
    ActionTableUsage++;
    EEPROM.write(EEPROM_ACTION_TABLE_USAGE_OFFSET,ActionTableUsage);    
  }
}

// EEPROM CRC Handshake
bool WorkerProcess::SendMsgEepromCrcRequest(uint8_t RecieverID)
{
#ifdef CONTROLLER
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_EEPROM_CRC_REQUEST");
  CommSender.Enqueue(RecieverID,MESSAGE_TYPE_EEPROM_CRC_REQUEST,0,NULL);
#endif  
  return true;
}

void WorkerProcess::HandleMsgEepromCrcRequest(uint8_t SenderID)
{

  int NumOfActions = EEPROM.read(EEPROM_ACTION_TABLE_USAGE_OFFSET);
  tMessageTypeSetAction Action;
  crc_t crc = crc_init();
  uint8_t i = NumOfActions;
  while (i--)
  {
    EEPROM.get(EEPROM_ACTION_TABLE_OFFSET+(EEPROM_ACTION_TABLE_SIZE*i),Action);
    crc = crc_update(crc, &Action, sizeof(Action));
  }  
  crc = crc_finalize(crc);
  SendMsgEepromCrcResponse(SenderID,NumOfActions,crc);
}

bool WorkerProcess::SendMsgEepromCrcResponse(uint8_t RecieverID,  uint8_t NumOfActions, uint16_t EepromCRC)
{
  tMessageTypeEepromCRCResponse Msg;
  Msg.NumOfActions = NumOfActions;
  Msg.EepromCRC = EepromCRC;
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_EEPROM_CRC_RESPONSE");
  CommSender.Enqueue(RecieverID,MESSAGE_TYPE_EEPROM_CRC_RESPONSE,sizeof(Msg),&Msg);
  return true;
};

void WorkerProcess::HandleMsgEepromCrcResponse(uint8_t SenderID, tMessageTypeEepromCRCResponse* Message)
{
#ifdef CONTROLLER
  RespHandler.EepromCRCResponseHandler(SenderID,Message->NumOfActions,Message->EepromCRC);
#endif
}


// VERSION HANDSHAKE
bool WorkerProcess::SendMsgVersionRequest(uint8_t RecieverID)
{
#ifdef CONTROLLER
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_FW_VERSION_REQUEST");
  CommSender.Enqueue(RecieverID,MESSAGE_TYPE_FW_VERSION_REQUEST,0,NULL);
#endif
  return true;
}

void WorkerProcess::HandleMsgVersionRequest(uint8_t SenderID)
{
  SendMsgVersionResponse(SenderID,FW_VERSION_MAJOR,FW_VERSION_MINOR,FW_VERSION_PATCH);
}

bool WorkerProcess::SendMsgVersionResponse(uint8_t RecieverID, uint8_t Major, uint8_t Minor, uint8_t Patch)
{
  tMessageTypeFwVesionResponse Msg;
  Msg.Major = Major;
  Msg.Minor = Minor;
  Msg.Patch = Patch;

  CommSender.Enqueue(RecieverID,MESSAGE_TYPE_FW_VERSION_RESPONSE,sizeof(Msg),&Msg);
};


void WorkerProcess::HandleMsgVersionResponse(uint8_t SenderID, tMessageTypeFwVesionResponse *Message)
{
#ifdef CONTROLLER
  RespHandler.VersionResponseHandler(SenderID,Message->Major,Message->Minor,Message->Patch);
#endif
}


bool WorkerProcess::SendMsgSetParams(uint8_t RecieverID, uint8_t DoubleClickTime, uint8_t NumOfRetransmissions,uint8_t MaxNumOfRetransmissions )
{
#ifdef CONTROLLER
  tMessageTypeSetParams Msg;
  Msg.DoubleClickTime = DoubleClickTime;
  Msg.NumOfRetransmissions = NumOfRetransmissions;
  Msg.MaxNumOfRetransmissions = MaxNumOfRetransmissions;
  CommSender.Enqueue(RecieverID,MESSAGE_TYPE_SET_PARAMS,sizeof(Msg),&Msg);
#endif
};

void WorkerProcess::HandleMsgSetParams(uint8_t SenderID, tMessageTypeSetParams *Message)
{
  EEPROM.write(EEPROM_DOUBLE_CLICK_TIME_OFFSET,Message->DoubleClickTime);
  EEPROM.write(EEPROM_NUM_OF_RETRANSMISSIONS,Message->NumOfRetransmissions);  
  EEPROM.write(EEPROM_MAX_NUM_OF_RETRANSMISSIONS,Message->MaxNumOfRetransmissions);  
}

bool WorkerProcess::SendMsgReset(uint8_t RecieverID)
{
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_FORCE_RESET");
  CommSender.Enqueue(RecieverID,MESSAGE_TYPE_FORCE_RESET,0,NULL);  
}


bool WorkerProcess::SendMsgSetDefaultTimer(uint8_t RecieverID, uint8_t OutputID, uint16_t DefTimerValue)
{
  tMessageTypeSetDefaultTimer Msg;
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_SET_DEFAULT_TIMER");
  Msg.OutputID = OutputID;
  Msg.DefaultTimerValue = DefTimerValue;
  CommSender.Enqueue(RecieverID,MESSAGE_TYPE_SET_DEFAULT_TIMER,sizeof(Msg),&Msg);
}

void WorkerProcess::HandleMsgSetDefaultTimer(uint8_t SenderID, tMessageTypeSetDefaultTimer *Message)
{
  if (Message->OutputID >= NUM_OF_OUTPUTS) return;
  EEPROM.put(EEPROM_DEFAULT_TIMER_VALUE_OFFSET+Message->OutputID*(sizeof(uint16_t)),Message->DefaultTimerValue);
}

bool WorkerProcess::SendMsgDefaultTimerRequest(uint8_t RecieverID, uint8_t OutputID)
{
  tMessageTypeDefaultTimerRequest Msg;
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_DEFAULT_TIMER_REQUEST");
  Msg.OutputID = OutputID;
  CommSender.Enqueue(RecieverID,MESSAGE_TYPE_DEFAULT_TIMER_REQUEST,sizeof(Msg),&Msg);  
}

void WorkerProcess::HandleMsgDefaultTimerRequest(uint8_t SenderID, tMessageTypeDefaultTimerRequest *Message)
{
  if (Message->OutputID >= NUM_OF_OUTPUTS) return;
  uint16_t DefTimer;
  EEPROM.get(EEPROM_DEFAULT_TIMER_VALUE_OFFSET+Message->OutputID*(sizeof(uint16_t)),DefTimer);
  SendMsgDefaultTimerResponse(SenderID,Message->OutputID,DefTimer);
}

bool WorkerProcess::SendMsgDefaultTimerResponse(uint8_t RecieverID, uint8_t OutputID, uint16_t DefTimerValue)
{
  tMessageTypeDefaultTimerResponse Msg;
  DEBUG_PRINTLN_3("===================>sending MESSAGE_TYPE_DEFAULT_TIMER_RESPONSE");
  Msg.OutputID = OutputID;
  Msg.DefTimerValue = DefTimerValue;
  CommSender.Enqueue(RecieverID,MESSAGE_TYPE_DEFAULT_TIMER_RESPONSE,sizeof(Msg),&Msg);  
}

void WorkerProcess::HandleMsgDefaultTimerResponse(uint8_t SenderID, tMessageTypeDefaultTimerResponse *Message)
{
#ifdef CONTROLLER
  RespHandler.DefaultTimerResponseHandler(SenderID,Message->OutputID,Message->DefTimerValue);
#endif
}

bool WorkerProcess::triggerNodesScan()
{
#ifdef CONTROLLER
   Enqueue(new NodeScanTask());
#endif
  return true;
}

void WorkerProcess::service()
{
  #ifdef CONTROLLER
   if (NULL == pCurrentWorkerTask)
   {
      if (mQueue.isEmpty())
      {
         // go to sleep
         disable();
         return;
      }

      pCurrentWorkerTask = mQueue.dequeue();
   }

   uint32_t nextPeriod;
   bool continueTask;
   continueTask = pCurrentWorkerTask->Process(&nextPeriod);
   if (continueTask)
   {
      // continue after requested time
      setPeriod(nextPeriod);
      return;
   }

   // task finished
   delete (pCurrentWorkerTask);
   pCurrentWorkerTask = NULL;
   setPeriod(SERVICE_CONSTANTLY);   // next iteration will go for next queue item or disable the task if the queue is empty
#else
   disable();
#endif
}

void WorkerProcess::Enqueue(WorkerTask *pWorkerTask)
{
   mQueue.enqueue(pWorkerTask);

   if (! isEnabled())
   {
     setPeriod(SERVICE_CONSTANTLY);
     setIterations(RUNTIME_FOREVER);
     enable();
   }
}
