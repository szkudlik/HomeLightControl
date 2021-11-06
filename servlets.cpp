#include "servlets.h"

bool tjavaScriptServlet::ProcessAndResponse()
{
	pOwner->SendFlashString(javascript_js_raw,javascript_js_raw_len);
	return false;
}

bool tOutputSetServlet::ProcessAndResponse()
{
	uint16_t Device;  // device iD
	uint16_t Output;  // output iD
	uint16_t State;  // 1 - on, 0 - off
	uint16_t Timer = 0;  // no timer; TODO: default timer
	bool ParametersOK = true;
	ParametersOK &= GetParameter("Dev",&Device);
	ParametersOK &= GetParameter("Out",&Output);    
	ParametersOK &= GetParameter("State",&State);
	GetParameter("Timer",&Timer);    // optional

	if (! ParametersOK)
	{
	  SendResponse400();
	  return false;
	}

	// execute   
	#ifdef DEBUG_3
		RespHandler.print(F("==>HTTP set output, dev="));
		RespHandler.print(Device,DEC);	
		RespHandler.print(F(" Out="));
		RespHandler.print(Output,DEC);	
		RespHandler.print(F(" State="));
		RespHandler.print(State,DEC);	
		RespHandler.print(F(" Timer="));
		RespHandler.println(Timer,DEC);	
	#endif
	Worker.SendMsgSetOutput(Device, Output, State, Timer);
	SendResponse200();

return false;
}

bool tSetTimerServlet::ProcessAndResponse()
{
	uint16_t Device;  // device iD
	uint16_t Output;  // output iD
	uint16_t Timer = 0;  // new timer
	bool ParametersOK = true;
	ParametersOK &= GetParameter("Dev",&Device);
	ParametersOK &= GetParameter("Out",&Output);    
	GetParameter("Timer",&Timer);    // optional

	if (! ParametersOK)
	{
	  SendResponse400();
	  return false;
	}

	// execute

	#ifdef DEBUG_3
		RespHandler.print(F("==>HTTP set timer, dev="));
		RespHandler.print(Device,DEC);	
		RespHandler.print(F(" Out="));
		RespHandler.print(Output,DEC);	
		RespHandler.print(F(" Timer="));
		RespHandler.println(Timer,DEC);	
	#endif
	Worker.SendMsgSetDefaultTimer(Device, Output, Timer);
	SendResponse200();

	return false;
}



void tOutputStateServlet::vOutputStateResponseHandler(uint8_t DevID, uint8_t OutputID, uint8_t PowerState, uint16_t  TimerValue, uint16_t DefaultTimer)
  {
    if (DevID != mExpectedDevID) return;
    if (OutputID != mExpectedOutputID) return;
    mPowerState = PowerState;
    mTimerValue = TimerValue;
    mDefaultTimer = DefaultTimer;
  }

void tOutputStateServlet::SendOutputStateRequest(uint8_t DevID, uint8_t OutputID)
{
  mPowerState = 255;
  mStartTimestamp = millis();
  mExpectedDevID = DevID;
  mExpectedOutputID = OutputID;
  
  Worker.SendMsgOutputStateRequest(DevID,OutputID);
}


bool tOutputStateServlet::ProcessOutputState(const char * caption)
{
  uint8_t Status = CheckStateRequest();
  if (STATUS_WAIT == Status)    
     return false; // keep waiting

  pOwner->SendFlashString(PSTR("<r>"));
  pOwner->SendFlashString(caption);
  pOwner->SendFlashString(PSTR(" </r>"));

  if (STATUS_TIMEOUT == Status) 
  {
    pOwner->SendFlashString(PSTR("???<br>"));
    return true; // give up
  }

  pOwner->SendFlashString(PSTR("<script>OutputControl(\""));
  pOwner->mEthernetClient.print(mExpectedDevID,DEC);
  pOwner->SendFlashString(PSTR("\",\""));
  pOwner->mEthernetClient.print(mExpectedOutputID,DEC);
  pOwner->SendFlashString(PSTR("\",\""));
  pOwner->mEthernetClient.print(mPowerState,DEC);
  pOwner->SendFlashString(PSTR("\",\""));
  pOwner->mEthernetClient.print(mTimerValue,DEC);
  pOwner->SendFlashString(PSTR("\",\""));
  pOwner->mEthernetClient.print(mDefaultTimer,DEC);
  pOwner->SendFlashString(PSTR("\");</script>"));

  return true; // done
}

uint8_t tOutputStateServlet::CheckStateRequest()
{
  if ((millis() - mStartTimestamp) > REQUEST_TIMEOUT) return STATUS_TIMEOUT;
  if (mPowerState == 255) return STATUS_WAIT;
  return STATUS_COMPLETE;
}

bool tOutputStateServlet::ProcessAndResponse()
{
   uint16_t Device;  // device iD
   uint16_t Output;  // output iD
   if (false == mRequestSent)
   {
      bool ParametersOK = true;
      ParametersOK &= GetParameter("Dev",&Device);
      ParametersOK &= GetParameter("Out",&Output);

      if (! ParametersOK)
      {
        SendResponse400();
        return false;
      }

      #ifdef DEBUG_3
         RespHandler.print(F("==>HTTP output state get, Dev="));
         RespHandler.print(Device,DEC);
         RespHandler.print(F(" Out="));
         RespHandler.println(Output,DEC);
      #endif
      SendOutputStateRequest(Device,Output);
      mRequestSent = true;
      return true;   // wait for result
   }

   uint8_t Status = CheckStateRequest();
   switch (Status)
   {
      case STATUS_WAIT:
         return true; // keep waiting

      case STATUS_COMPLETE:
         break;

      case STATUS_TIMEOUT:
      default:
         SendResponse400();
         return false;
   }

   // format JSON response
   pOwner->SendFlashString(PSTR("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"Device\": "));
   pOwner->mEthernetClient.print(mExpectedDevID,DEC);
   pOwner->SendFlashString(PSTR(", \"Output\": "));
   pOwner->mEthernetClient.print(mExpectedOutputID,DEC);
   pOwner->SendFlashString(PSTR(", \"State\": "));
   pOwner->mEthernetClient.print(mPowerState,DEC);
   pOwner->SendFlashString(PSTR(", \"Timer\": "));
   pOwner->mEthernetClient.print(mTimerValue,DEC);
   pOwner->SendFlashString(PSTR(", \"DefaultTimer\": "));
   pOwner->mEthernetClient.print(mDefaultTimer,DEC);
   pOwner->SendFlashString(PSTR("}\r\n"));

   return false;   
}
