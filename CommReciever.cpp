#include <ProcessScheduler.h>
#include "Random.h"
#include "CommDefs.h"
#include <AceCRC.h>
#include "CommReciever.h"
#include "CommSender.h"
#include "WorkerProcess.h"
#include "Eeprom.h"

using namespace ace_crc::crc16ccitt_nibble;


CommRecieverProcess::CommRecieverProcess(Scheduler &manager) : Process(manager,LOW_PRIORITY,RECIEVE_CHECK_PERIOD), mRetransTableHead(0)
{
  SetState(STATE_IDLE);
}

void CommRecieverProcess::CleanIncomingBuffer()
{
  DEBUG_PRINTLN_1("Clean buffer");
  while (COMM_SERIAL.available()) COMM_SERIAL.read();
}

void CommRecieverProcess::SetState(uint8_t State)
{
  mState = State;
  switch (mState)
  {
    case STATE_IDLE:
        for (uint8_t i = 0; i < RECIEVE_NUMBER_OF_RETRANS_TABLE; i++) 
          mRetransTable[i].SenderID = DEVICE_ID_BROADCAST;  // BROADCAST means an empty slot      
        setIterations(0);
        DEBUG_PRINTLN_1("------> State Idle");
        break;
        
    case STATE_NEW_TRIGGER:
       setIterations(3);
       setPeriod(SERVICE_CONSTANTLY);
       enable();       
       DEBUG_PRINTLN_1("------> State NEW_TRIGGER");
       break;
       
    case STATE_WAIT_FOR_DATA:
      setPeriod(RECIEVE_CHECK_PERIOD);
      DEBUG_PRINTLN_1("------> State WAIT_FOR_DATA");
      break;
      
    case STATE_WAIT_FOR_IDLE:
      setPeriod(RECIEVE_IDLE_WAIT);    
      DEBUG_PRINTLN_1("------> State IDLE_WAIT");
      break;
  }
}

void CommRecieverProcess::serialEvent()
{
  if ((mState == STATE_IDLE) ||
      (mState == STATE_WAIT_FOR_IDLE))
  {
    SetState(STATE_NEW_TRIGGER);
  }  
}

void CommRecieverProcess::service()
{
  DEBUG_PRINTLN_1("SERVICE enter");
  
  switch (mState)
  {
    case STATE_NEW_TRIGGER:
      if (COMM_SERIAL.available() < sizeof(mFrame))
      {
        SetState(STATE_WAIT_FOR_DATA);
        // next iteration
        break;
      }

      // else - proceed to next step, 
    case STATE_WAIT_FOR_DATA:
      if (COMM_SERIAL.available() < sizeof(mFrame))
      {
        // timeout - drop
        DEBUG_PRINTLN_1("Timeout - drop data");
        CleanIncomingBuffer();        
      }
      else
      {
        // frame is ready
        DEBUG_PRINTLN_1("Reading frame");
        COMM_SERIAL.readBytes((char*)&mFrame,sizeof(mFrame));
        ProcessFrame();
      }
      SetState(STATE_WAIT_FOR_IDLE);
      break;

    case STATE_WAIT_FOR_IDLE:
      SetState(STATE_IDLE);
      break;      
  }

  DEBUG_PRINTLN_1("SERVICE leaving");
}

void CommRecieverProcess::ProcessFrame()
{
  // check CRC 
  crc_t crc = crc_calculate(&mFrame, sizeof(mFrame));
  if (0 != crc)
  {
  // drop frame - CRC is not valid
     DEBUG_PRINTLN_2("bad CRC - reject");    
     return;
  }
   
   
  #ifdef DEBUG_1
    // printout frame
    DEBUG_SERIAL.print(F("SenderDevId = 0x"));
    DEBUG_SERIAL.print(mFrame.SenderDevId,HEX);
    DEBUG_SERIAL.print(F(" DstDevId = 0x"));
    DEBUG_SERIAL.print(mFrame.DstDevId,HEX);
    DEBUG_SERIAL.print(F(" Seq = 0x"));
    DEBUG_SERIAL.print(mFrame.Seq,HEX);
    DEBUG_SERIAL.print(F(" MessageType = 0x"));
    DEBUG_SERIAL.print(mFrame.MessageType,HEX);
    DEBUG_SERIAL.print(F(" Data = 0x"));
    for (uint8_t i = 0; i < COMMUNICATION_PAYLOAD_DATA_SIZE; i++)
    {
      DEBUG_SERIAL.print(mFrame.Data[i],HEX);      
    }
    DEBUG_SERIAL.println();
  #endif
  
  // are we the sender?
  if (mFrame.SenderDevId ==  EEPROM.read(EEPROM_DEVICE_ID_OFFSET))
  {
    // this is a frame sent by us, mark that it has been properly recieved and drop the frame
     DEBUG_PRINTLN_2("SELF SENT FRAME RECIEVED");
     mSelfFrameMark = true;
  }

  // are we the reciever?
  if (!( (mFrame.DstDevId == DEVICE_ID_BROADCAST) || (mFrame.DstDevId == EEPROM.read(EEPROM_DEVICE_ID_OFFSET))))
  {
     // drop
     DEBUG_PRINTLN_2("Dst does not match - reject");
     return;
  }

  // check retransmissions
  for (uint8_t i = 0; i < RECIEVE_NUMBER_OF_RETRANS_TABLE; i++)
  {
     if (mRetransTable[i].SenderID == DEVICE_ID_BROADCAST) continue;  // empty table slot

     if ((mRetransTable[i].SenderID == mFrame.SenderDevId) && (mRetransTable[i].Seq == mFrame.Seq))
     {
         // a retransmission - drop
         DEBUG_PRINTLN_2("Retransmission - drop");
         return;
     }
  }
  
   // this is not an retransmission
   mRetransTable[mRetransTableHead].SenderID = mFrame.SenderDevId;
   mRetransTable[mRetransTableHead].Seq = mFrame.Seq;
   mRetransTableHead++;
   mRetransTableHead %= RECIEVE_NUMBER_OF_RETRANS_TABLE;
   //  trigger an action
   switch (mFrame.MessageType)
   {
    case MESSAGE_TYPE_OVERVIEW_STATE_REQUEST: 
        DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_OVERVIEW_STATE_REQUEST");
        Worker.HandleMsgOverviewStateRequest(mFrame.SenderDevId);
        break;
          
    case MESSAGE_TYPE_OVERVIEW_STATE_RESPONSE:
        DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_OVERVIEW_STATE_RESPONSE");
        Worker.HandleMsgOverviewStateResponse(mFrame.SenderDevId,(tMessageTypeOverviewStateResponse*) (mFrame.Data));
        break;

    case MESSAGE_TYPE_OUTPUT_STATE_REQUEST: 
        DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_OUTPUT_STATE_REQUEST");
        Worker.HandleMsgOutputStateRequest(mFrame.SenderDevId,(tMessageTypeOutputStateRequest*)(mFrame.Data));
        break;
          
    case MESSAGE_TYPE_OUTPUT_STATE_RESPONSE:
        DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_OUTPUT_STATE_RESPONSE");
        Worker.HandleMsgOutputStateResponse(mFrame.SenderDevId,(tMessageTypeOutputStateResponse*) (mFrame.Data));
        break;
        
    
    case MESSAGE_TYPE_SET_OUTPUT:
        DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_SET_OUTPUT");
        Worker.HandleMsgSetOutput(mFrame.SenderDevId,(tMessageTypeSetOutput*)(mFrame.Data));
        break;

    case MESSAGE_BUTTON_PRESS:
        DEBUG_PRINTLN_3("===================>MESSAGE_BUTTON_PRESS");
        Worker.HandleMsgButtonPress(mFrame.SenderDevId, (tMessageTypeButtonPress*)(mFrame.Data));
        break;

    case MESSAGE_TYPE_SET_ACTION:
          DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_SET_ACTION");
          Worker.HandleMsgSetAction(mFrame.SenderDevId, (tMessageTypeSetAction*)(mFrame.Data));
        break;

    case MESSAGE_TYPE_CLEAR_ACTIONS:
          DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_CLEAR_ACTIONS");
          Worker.HandleMsgClearAllActions(mFrame.SenderDevId);
        break;

    case MESSAGE_TYPE_FW_VERSION_REQUEST:
          DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_FW_VERSION_REQUEST");
          Worker.HandleMsgVersionRequest(mFrame.SenderDevId);
        break;

    case MESSAGE_TYPE_FW_VERSION_RESPONSE:
          DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_FW_VERSION_RESPONSE");
          Worker.HandleMsgVersionResponse(mFrame.SenderDevId,(tMessageTypeFwVesionResponse*)(mFrame.Data));
        break;

    case MESSAGE_TYPE_EEPROM_CRC_REQUEST:
          DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_EEPROM_CRC_REQUEST");
          Worker.HandleMsgEepromCrcRequest(mFrame.SenderDevId);
        break;

    case MESSAGE_TYPE_EEPROM_CRC_RESPONSE:
          DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_EEPROM_CRC_RESPONSE");
          Worker.HandleMsgEepromCrcResponse(mFrame.SenderDevId,(tMessageTypeEepromCRCResponse*)(mFrame.Data));
        break;
        
    case MESSAGE_TYPE_SET_PARAMS:
          DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_SET_PARAMS");
          Worker.HandleMsgSetParams(mFrame.SenderDevId,(tMessageTypeSetParams*)(mFrame.Data));
        break;

    case MESSAGE_TYPE_FORCE_RESET:
          DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_FORCE_RESET");
          cli();
          while(1); // let watchdog reboot the device
        break;
        
    case MESSAGE_TYPE_SET_DEFAULT_TIMER:
          DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_SET_DEFAULT_TIMER");
          Worker.HandleMsgSetDefaultTimer(mFrame.SenderDevId,(tMessageTypeSetDefaultTimer*)(mFrame.Data));
        break;

    case MESSAGE_TYPE_DEFAULT_TIMER_REQUEST:
          DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_DEFAULT_TIMER_REQUEST");
          Worker.HandleMsgDefaultTimerRequest(mFrame.SenderDevId,(tMessageTypeDefaultTimerRequest*)(mFrame.Data));
        break;

    case MESSAGE_TYPE_DEFAULT_TIMER_RESPONSE:
          DEBUG_PRINTLN_3("===================>MESSAGE_TYPE_DEFAULT_TIMER_RESPONSE");
          Worker.HandleMsgDefaultTimerResponse(mFrame.SenderDevId,(tMessageTypeDefaultTimerResponse*)(mFrame.Data));
        break;


  default: 
      DEBUG_PRINTLN_3("MESSAGE  unknown type, drop");
   }
}
