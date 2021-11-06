#ifndef RESPONSE_HANDLER
#define RESPONSE_HANDLER
#include "common.h"
#include "CommDefs.h"
#include "Print.h"

class ResponseHandler : public Print
{
public:
  ResponseHandler() : mLogEnbabled(true) { pNext = pFirst ; pFirst = this; }
  
  virtual ~ResponseHandler();

  void EnableLogs() { mLogEnbabled = true; }
  void DisableLogs() { mLogEnbabled = false; }
  static void EnableLogsForce() { mLogsForced = true; }
  static void DisableLogsForce() { mLogsForced = false; }
  
  virtual size_t write(uint8_t str);
  
  static void OverviewStateResponseHandler(uint8_t SenderID, uint8_t PowerState, uint8_t  TimerState);  
  static void OutputStateResponseHandler(uint8_t SenderID, uint8_t OutputID, uint8_t PowerState, uint16_t  TimerValue, uint16_t DefaultTimer);
  static void EepromCRCResponseHandler(uint8_t SenderID, uint8_t NumOfActions, uint16_t EepromCRC);
  static void VersionResponseHandler(uint8_t SenderID, uint8_t Major, uint8_t Minor, uint8_t Patch);
  static void NodeScanResponse(uint32_t ActiveNodesMap);
  static void DefaultTimerResponseHandler(uint8_t SenderID,uint8_t OutputID,uint16_t DefTimerValue);

protected:
  
  
  virtual void vOverviewStateResponseHandler(uint8_t DevID, uint8_t PowerState, uint8_t  TimerState) {}
  virtual void vOutputStateResponseHandler(uint8_t DevID, uint8_t OutputID, uint8_t PowerState, uint16_t  TimerValue, uint16_t DefaultTimer) {}
  virtual void vEepromCRCResponseHandler(uint8_t DevID, uint8_t NumOfActions, uint16_t EepromCRC) {}
  virtual void vVersionResponseHandler(uint8_t DevID, uint8_t Major, uint8_t Minor, uint8_t Patch) {}
  virtual void vDefaultTimerResponseHandler(uint8_t DevID,uint8_t OutputID,uint16_t DefTimerValue) {}
  
  virtual void vLog(uint8_t str){}
  virtual void vNodeScanResponse(uint32_t ActiveNodesMap) {}
  
private:
  bool mLogEnbabled;
  static bool mLogsForced;
  static ResponseHandler* pFirst;
  ResponseHandler* pNext;
};

class ResponseHandlerSerial : public ResponseHandler
{
public:
  ResponseHandlerSerial() : ResponseHandler() {}
  virtual ~ResponseHandlerSerial() {};

#ifdef CONTROLLER
protected: 
  virtual void vLog(uint8_t str) { DEBUG_SERIAL.write(str); }
#endif

};

extern ResponseHandlerSerial RespHandler;
#endif // RESPONSE_HANDLER
