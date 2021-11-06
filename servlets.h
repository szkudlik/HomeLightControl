#ifndef SERVLETS
#define SERVLETS

#include "httpServer.h"
#include "http_binaries.h"
#include "WorkerProcess.h"
#include "ResponseHandler.h"

class tjavaScriptServlet :  public tHttpServlet
{
public:
  tjavaScriptServlet() : tHttpServlet() {}
  virtual ~tjavaScriptServlet() {}

  virtual bool ProcessAndResponse();
};


/**
 * output servlet - controlling or checking state of an output
 */
class tOutputSetServlet :  public tHttpServlet
{
public:
  tOutputSetServlet() : tHttpServlet() {}
  virtual ~tOutputSetServlet() {}

  virtual bool ProcessAndResponse();  
};

/**
 * output servlet - controlling or checking state of an output
 */
class tSetTimerServlet :  public tHttpServlet
{
public:
  tSetTimerServlet() : tHttpServlet() {}
  virtual ~tSetTimerServlet() {}

  virtual bool ProcessAndResponse();  
};

class tOutputStateServlet : public tHttpServlet, public ResponseHandler
{
  public:
   tOutputStateServlet() :  tHttpServlet(),ResponseHandler(), mRequestSent(false) {};
  virtual ~tOutputStateServlet() {}
  
  virtual void vOutputStateResponseHandler(uint8_t DevID, uint8_t OutputID, uint8_t PowerState, uint16_t  TimerValue, uint16_t DefaultTimer);
  virtual bool ProcessAndResponse();
  
protected:
  uint8_t mExpectedDevID;
  uint8_t mExpectedOutputID;
  unsigned long mStartTimestamp;
  uint8_t   mPowerState;
  uint16_t  mTimerValue;
  uint16_t  mDefaultTimer;

  static uint8_t const STATUS_WAIT = 0;
  static uint8_t const STATUS_TIMEOUT = 1;
  static uint8_t const STATUS_COMPLETE = 2;
  static unsigned long const REQUEST_TIMEOUT = 5000;

  void SendOutputStateRequest(uint8_t DevID, uint8_t OutputID);
  uint8_t CheckStateRequest();
  bool ProcessOutputState(const char * caption);

  bool mRequestSent;
};



#endif
