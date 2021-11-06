#ifndef LIGHT_WEB_CONTROL_APP
#define LIGHT_WEB_CONTROL_APP

#include "httpServer.h"
#include "http_binaries.h"
#include "WorkerProcess.h"




class tLightWebControl : public tHttpServlet
{
public:
  tLightWebControl() : tHttpServlet() {}
  
protected:

  void SendSetupFooter()
  {
      uint16_t SetupParam=0;
      GetParameter("setup",&SetupParam);
      if (! SetupParam)
      {
        pOwner->SendFlashString(PSTR("<br><small><a href=\"?setup=1\"><i>setup</i></a></small><br>"));
      }
      else
      {
        pOwner->SendFlashString(PSTR("<small><br><a href=\"?setup=0\"><i>hide setup</i></a><br>"));
      }
      pOwner->SendFlashString(PSTR("<br><a href=\"index\">strona główna</a><br>"));
  }
};


class tDefaultPageServlet :  public tLightWebControl
{
public:
   tDefaultPageServlet() : tLightWebControl() {}

  virtual bool ProcessAndResponse()
  {
    pOwner->SendFlashString(defaultPage_http_raw,defaultPage_http_raw_len);
    SendVersionAndPageClose();
    return false;
  }
};

//OutputControl(Device,Output)
class tGardenLightsServlet :  public tLightWebControl
{
public:
   tGardenLightsServlet() : tLightWebControl() {}

  virtual bool ProcessAndResponse()
  {
    pOwner->SendFlashString(gardenLightsPageHeader_http_raw,gardenLightsPageHeader_http_raw_len);
    SendSetupFooter();
    SendVersionAndPageClose();
    return false;
  }
};

class tIndoorLightsServlet :  public tLightWebControl
{
public:
   tIndoorLightsServlet() : tLightWebControl() {}

  virtual bool ProcessAndResponse()
  {
    pOwner->SendFlashString(indoorLightsPageHeader_http_raw,indoorLightsPageHeader_http_raw_len);
    SendSetupFooter();
    SendVersionAndPageClose();
    return false;
  }
};

#endif // LIGHT_WEB_CONTROL_APP
