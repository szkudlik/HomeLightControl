#include "ResponseHandler.h"

ResponseHandler* ResponseHandler::pFirst = NULL;
bool ResponseHandler::mLogsForced = false;
ResponseHandlerSerial RespHandler;


ResponseHandler::~ResponseHandler() 
{
  ResponseHandler *i = pFirst;
  ResponseHandler *prev = NULL;
  while(i != NULL)
  {
    if (i == this)
    {
      if (NULL == prev)        // first item?
      {
        pFirst = this->pNext; // next to this becomes first
      }
      else if (NULL == this->pNext) // last item?
      {
        prev->pNext = NULL;   // prev to this becomes last
      }
      else  // middle item
      {
        prev->pNext = this->pNext;
      }
      return; // done
    }
    prev = i;
    i = i->pNext;
  }
}

size_t ResponseHandler::write(uint8_t str) 
{ 
	for (ResponseHandler * i = pFirst; i != NULL ; i = i->pNext) 
	{
		if ((i->mLogEnbabled) | mLogsForced) i->vLog(str);
	}
}


void ResponseHandler::OverviewStateResponseHandler(uint8_t SenderID, uint8_t PowerState, uint8_t  TimerState) 
{	
	EnableLogsForce();
	RespHandler.print(F("PowerStateBitmap for device "));
	RespHandler.print(SenderID,HEX);
	RespHandler.print(F("="));
	RespHandler.print(PowerState,BIN);
	RespHandler.print(F(" with timers map="));
	RespHandler.print(TimerState,BIN);
	RespHandler.println();
	DisableLogsForce();
	for (ResponseHandler * i = pFirst; i != NULL ; i = i->pNext)  i->vOverviewStateResponseHandler(SenderID,PowerState,TimerState);
}

void ResponseHandler::OutputStateResponseHandler(uint8_t SenderID, uint8_t OutputID, uint8_t PowerState, uint16_t  TimerValue, uint16_t DefaultTimer)
{	
	EnableLogsForce();
	RespHandler.print(F("PowerState for device "));
	RespHandler.print(SenderID,HEX);
	RespHandler.print(F(" output ID "));
	RespHandler.print(OutputID,DEC);
	RespHandler.print(F("="));
	RespHandler.print(PowerState,DEC);
	RespHandler.print(F(" with timers = "));
	RespHandler.print(TimerValue,DEC);
  RespHandler.print(F(" default timer = "));
  RespHandler.print(DefaultTimer,DEC);
  
	RespHandler.println();
	DisableLogsForce();
	for (ResponseHandler * i = pFirst; i != NULL ; i = i->pNext)  i->vOutputStateResponseHandler(SenderID,OutputID,PowerState,TimerValue,DefaultTimer);
}

void ResponseHandler::EepromCRCResponseHandler(uint8_t SenderID, uint8_t NumOfActions, uint16_t EepromCRC)
{	
	EnableLogsForce();
	RespHandler.print(F("Eeprom CRC for device "));
	RespHandler.print(SenderID,HEX);
	RespHandler.print(F(" num of actions="));
	RespHandler.print(NumOfActions,DEC);
	RespHandler.print(F(" CRC="));
	RespHandler.print(EepromCRC,DEC);
	RespHandler.println();
	DisableLogsForce();
	for (ResponseHandler * i = pFirst; i != NULL ; i = i->pNext)  i->vEepromCRCResponseHandler(SenderID,NumOfActions,EepromCRC);
}

void ResponseHandler::VersionResponseHandler(uint8_t SenderID, uint8_t Major, uint8_t Minor, uint8_t Patch)
{	
	EnableLogsForce();
	RespHandler.print(F("FW Version for device "));
	RespHandler.print(SenderID,HEX);
	RespHandler.print(F("="));
	RespHandler.print(Major,DEC);    
	RespHandler.print(F("."));
	RespHandler.print(Minor,DEC);    
	RespHandler.print(F("."));
	RespHandler.print(Patch,DEC);    
	RespHandler.println();
	DisableLogsForce();

	for (ResponseHandler * i = pFirst; i != NULL ; i = i->pNext) i->vVersionResponseHandler(SenderID,Major,Minor,Patch); 
}

void ResponseHandler::NodeScanResponse(uint32_t ActiveNodesMap) 
{
	EnableLogsForce();
	RespHandler.print(F("Active node map: "));
	RespHandler.println(ActiveNodesMap,BIN);
	DisableLogsForce();

	for (ResponseHandler * i = pFirst; i != NULL ; i = i->pNext) i->vNodeScanResponse(ActiveNodesMap);     
}

void ResponseHandler::DefaultTimerResponseHandler(uint8_t SenderID,uint8_t OutputID,uint16_t DefTimerValue)
{
	EnableLogsForce();
	RespHandler.print(F("Default timer for device "));
	RespHandler.print(SenderID,HEX);
	RespHandler.print(F(" outId "));
	RespHandler.print(OutputID,DEC);    
	RespHandler.print(F("="));
	RespHandler.print(DefTimerValue,DEC);    
	RespHandler.println();
	DisableLogsForce();

	for (ResponseHandler * i = pFirst; i != NULL ; i = i->pNext) i->vDefaultTimerResponseHandler(SenderID,OutputID,DefTimerValue);       
}
