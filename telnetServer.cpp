#include "telnetServer.h"
#include "common.h"
#include "WorkerProcess.h"

bool helloHandler(Commander &Cmdr);
bool send_stateOverviewHandler(Commander &Cmdr);
bool send_OutputStateHandler(Commander &Cmdr);
bool send_SetOutputHandler(Commander &Cmdr);
bool send_ClearActions(Commander &Cmdr);
bool send_addAction(Commander &Cmdr);
bool send_GetEepromCrc(Commander &Cmdr);
bool send_GetVersion(Commander &Cmdr);
bool send_SetParams(Commander &Cmdr);
bool send_Reset(Commander &Cmdr);
bool enableLogs(Commander &Cmdr);
bool disableLogs(Commander &Cmdr);
bool trigger_ScanNodes(Commander &Cmdr);
bool send_SetDefaultTimer(Commander &Cmdr);
bool send_GetDefaultTimer(Commander &Cmdr);
bool send_ButtonPress(Commander &Cmdr);

// must be static-global (why? - only 1 telnet session may be active)
Commander cmd;

const commandList_t masterCommands[] = {
  {"enableLogs",      enableLogs,                   "enable logs on telnet console"},
  {"disableLogs",     disableLogs,                  "enable logs on telnet console"},
  {"StateOverview",   send_stateOverviewHandler,    "MESSAGE_TYPE_REQUEST_OVERVIEW_STATE dev_id"},  
  {"OutputState",     send_OutputStateHandler,      "MESSAGE_TYPE_OUTPUT_STATE_REQUEST dev_id out_id"},
  {"SetOutput",       send_SetOutputHandler,        "MESSAGE_TYPE_OUTPUT_STATE_REQUEST dev_id out_id state timer"},
  {"ButtonPress",     send_ButtonPress,             "MESSAGE_BUTTON_PRESS with a forced src id"},
  {"ClearActions",    send_ClearActions,            "MESSAGE_TYPE_CLEAR_ACTIONS dev_id"},
  {"GetEepromCrc",    send_GetEepromCrc,            "MESSAGE_TYPE_EEPROM_CRC_REQUEST dev_id"},
  {"GetVersion",      send_GetVersion,              "MESSAGE_TYPE_FW_VERSION_REQUEST dev_id"},  
  {"SetParams",       send_SetParams,               "MESSAGE_TYPE_SET_PARAMS dev_id DoubleClickTime NumOfRetransmissions MaxNumOfRetransmissions"},  
  {"SetDefTimer",     send_SetDefaultTimer,         "MESSAGE_TYPE_SET_DEFAULT_TIMER dev_id out_id defTimerValue (0=no timer)"},
  {"GetDefTimer",     send_GetDefaultTimer,         "MESSAGE_TYPE_DEFAULT_TIMER_REQUEST dev_id out_id"},
  {"Reset",           send_Reset,                   "MESSAGE_TYPE_FORCE_RESET dev_id (may be 255 - broadcast)"},
  {"ScanActiveNodes", trigger_ScanNodes,            "Scan the bus for nodes from 1 to 32"},
  {"addAction",       send_addAction,               "MESSAGE_TYPE_SET_ACTION dev_id OutId SenderID ButtonId [ Timer TriggerType ActionType OutputsMask OutputsStates ]"},
};

/*

Usage: addAction dev_id OutId SenderID ButtonId [ TriggerType ActionType Timer OutputsMask OutputsStates ]
   TriggerType -  0=CLICK,   1=LONG_CLICK,  2=DBL_CLICK   3=ANY_CLICK
   ActionType -   0=TOGGLE,  1=ON,          2=OFF


button 0 turn on 0 if 0 = 0 1 = 0

button 0 turn on 1 if 1 = 1 2 = 0

button 0 turn of 0 if 1 = 1 2 = 1

button 0 turn of 1 if 1 = 1 2 = 1

addAction 3 0 3 0 3 1 0 3 0
addAction 3 1 3 0 3 1 0 3 1
addAction 3 0 3 0 3 2 0 3 3
addAction 3 1 3 0 3 2 0 3 3
*/

tTelnetSession *pTelnetSession = NULL;

tTelnetSession::tTelnetSession(EthernetClient aEthernetClient) : tTcpSession(aEthernetClient), ResponseHandler()
{
  DEBUG_PRINTLN_3("TELNET Session started");
  cmd.begin(&mEthernetClient, masterCommands, sizeof(masterCommands));
  cmd.commandPrompt(ON); //enable the command prompt
  cmd.echo(false);     //Echo incoming characters to theoutput port
  cmd.errorMessages(ON); //error messages are enabled - it will tell us if we issue any unrecognised commands
  cmd.autoChain(ON);
  cmd.printCommandList();
  pTelnetSession = this;
  DisableLogs();
}
 
bool tTelnetSession::doProcess() 
{
  cmd.update();
  return true;
}

tTelnetSession::~tTelnetSession() { pTelnetSession = NULL; }

bool send_ClearActions(Commander &Cmdr)
{
  int Dst;
  if(Cmdr.getInt(Dst))
  {
    Worker.SendMsgClearAllActions(Dst);
  }
  else
  {
    Cmdr.println(F("Usage: ClearActions dst_dev_id"));
    return false;
  }

  return true;  
}

bool send_stateOverviewHandler(Commander &Cmdr)
{
    
  int Dst;
  if(Cmdr.getInt(Dst))
  {
    Worker.SendMsgOverviewStateRequest(Dst);
  }
  else
  {
    Cmdr.println(F("Usage: StateOverview dst_dev_id"));
    return false;
  }

  return true;
}

bool send_OutputStateHandler(Commander &Cmdr)
{
    
  int Dst;
  int OutId;  
  if(!Cmdr.getInt(Dst))
  {
    goto error;
  }
  if (! Cmdr.getInt(OutId))
  {
    goto error;
  }
  
   Worker.SendMsgOutputStateRequest(Dst,OutId);

  return true;
error:
  Cmdr.println(F("Usage: OutputState dst_dev_id output_id"));
  return false;
}

bool send_SetOutputHandler(Commander &Cmdr)
{
  int Dst;
  int OutId;  
  int State;
  int Timer = 0;  
  
  if(!Cmdr.getInt(Dst))
  {
    goto error;
  }
  if (! Cmdr.getInt(OutId))
  {
    goto error;
  }
  if (! Cmdr.getInt(State))
  {
    goto error;
  }
  if (! Cmdr.getInt(Timer))
  {
    //goto finish;
  }

  Worker.SendMsgSetOutput(Dst, OutId, State, Timer);
  return true;
error:
  Cmdr.println(F("Usage: SetOutput dst_dev_id output_id state[0/1] [timer[sec]]"));
  return false;
}

bool send_addAction(Commander &Cmdr)
{
  int RecieverID;
  int OutId;
  int SenderDevID;
  int ButtonId;
  int TriggerType = BUTTON_TRIGGER_TYPE_ANY_CLICK;
  int ActionType = BUTTON_ACTION_TYPE_TOGGLE;
  int Timer = 0xFFFF; 
  int OutputsMask = 0;
  int OutputsStates = 0;

  if(!Cmdr.getInt(RecieverID))
  {
    goto error;
  }
  if(!Cmdr.getInt(OutId))
  {
    goto error;
  }
  if(!Cmdr.getInt(SenderDevID))
  {
    goto error;
  }
  if(!Cmdr.getInt(ButtonId))
  {
    goto error;
  }
  if(!Cmdr.getInt(Timer))
  {
    goto final;
  }
  if(!Cmdr.getInt(TriggerType))
  {
    goto final;
  }
  if(!Cmdr.getInt(ActionType))
  {
    goto final;
  }
  if(!Cmdr.getInt(OutputsMask))
  {
    goto final;
  }
  if(!Cmdr.getInt(OutputsStates))
  {
    goto final;
  }

final:
  Worker.SendMsgAddAction(RecieverID, OutId, SenderDevID, ButtonId, TriggerType, ActionType, Timer, OutputsMask, OutputsStates);
  return true;
  
error:
  Cmdr.println(F("Usage: addAction dev_id OutId SenderID ButtonId [ Timer TriggerType ActionType OutputsMask OutputsStates ] "));
  Cmdr.println(F("   TriggerType -  0=CLICK,   1=LONG_CLICK,  2=DBL_CLICK    3=ANY_CLICK (default)"));
  Cmdr.println(F("   ActionType -   0=TOGGLE (default),  1=ON,          2=OFF"));
  Cmdr.println(F("   Timer = 0 - no timer, 65535(0xFFFF) - default timer (default)"));
  return false;
}

bool send_GetEepromCrc(Commander &Cmdr)
{
  int Dst;
  if(Cmdr.getInt(Dst))
  {
    Worker.SendMsgEepromCrcRequest(Dst);
  }
  else
  {
    Cmdr.println(F("Usage: StateOverview dst_dev_id"));
    return false;
  }

  return true;
}

bool send_GetVersion(Commander &Cmdr)
{
  int Dst;
  if(Cmdr.getInt(Dst))
  {
    Worker.SendMsgVersionRequest(Dst);
  }
  else
  {
    Cmdr.println(F("Usage: GetVersion dst_dev_id"));
    return false;
  }

  return true;  
}

bool send_SetParams(Commander &Cmdr)
{
  int Dst;
  int DoubleClickTime;  
  int NumOfRetransmissions;
  int MaxNumOfRetransmissions;
  
  if(!Cmdr.getInt(Dst))
  {
    goto error;
  }
  if (! Cmdr.getInt(DoubleClickTime))
  {
    goto error;
  }
  if (! Cmdr.getInt(NumOfRetransmissions))
  {
    goto error;
  }
  if (! Cmdr.getInt(MaxNumOfRetransmissions))
  {
    goto error;
  }
  
   Worker.SendMsgSetParams(Dst,DoubleClickTime,NumOfRetransmissions,MaxNumOfRetransmissions);

  return true;
error:
  Cmdr.println(F("Usage: SetParams dst_dev_id DoubleClickTime NumOfRetransmissions MaxNumOfRetransmissions"));
  return false;
}

bool send_Reset(Commander &Cmdr)
{
  int Dst;
  if(Cmdr.getInt(Dst))
  {
    Worker.SendMsgReset(Dst);
  }
  else
  {
    Cmdr.println(F("Usage: SendReset dst_dev_id"));
    return false;
  }

  return true;    
}

bool trigger_ScanNodes(Commander &Cmdr)
{
  Worker.triggerNodesScan();
}

bool enableLogs(Commander &Cmdr)
{
  pTelnetSession->EnableLogs();
}

bool disableLogs(Commander &Cmdr)
{
  pTelnetSession->DisableLogs();
}

bool send_SetDefaultTimer(Commander &Cmdr)
{
 
  int Dst;
  int OutId;
  int DefTimerValue;
  
  if(!Cmdr.getInt(Dst))
  {
    goto error;
  }
  if (! Cmdr.getInt(OutId))
  {
    goto error;
  }
  if (! Cmdr.getInt(DefTimerValue))
  {
    goto error;
  }
  
   Worker.SendMsgSetDefaultTimer(Dst,OutId,DefTimerValue);

  return true;
error:
  Cmdr.println(F("Usage: SetDefTimer dst_dev_id output_id DefaultTimer (0=no timer)"));
  return false; 
}

bool send_GetDefaultTimer(Commander &Cmdr)
{
 
  int Dst;
  int OutId;
  if(!Cmdr.getInt(Dst))
  {
    goto error;
  }
  if (! Cmdr.getInt(OutId))
  {
    goto error;
  }
  
   Worker.SendMsgDefaultTimerRequest(Dst,OutId);

  return true;
error:
  Cmdr.println(F("Usage: GetDefTimer dst_dev_id output_id"));
  return false; 
}

bool send_ButtonPress(Commander &Cmdr)
{
  uint8_t ForcedSenderId = 0;
  uint16_t ShortClick = 0;
  uint16_t LongClick = 0;
  uint16_t DoubleClick = 0;

  if(!Cmdr.getInt(ForcedSenderId))
  {
    goto error;
  }
  if(!Cmdr.getInt(ShortClick))
  {
    goto error;
  }
  
  if(!Cmdr.getInt(LongClick))
  {
    goto final;
  }
  if(!Cmdr.getInt(DoubleClick))
  {
    goto final;
  }

final:
  Worker.SendMsgButtonPress(DEVICE_ID_BROADCAST,ForcedSenderId, ShortClick, LongClick, DoubleClick);
  return true;

error:
  Cmdr.println(F("Usage: ButtonPress forcedSrcID ShortClick [LongClick DoubleClick] (bitmaps)"));
  return false; 
}


void tTelnetSession::vLog(uint8_t str)
{
    if (NULL != cmd.getOutputPort())
      cmd.getOutputPort()->write(str);
}
