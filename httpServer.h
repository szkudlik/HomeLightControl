#ifndef HTTP_SERVER
#define HTTP_SERVER
#include "common.h"
#include "tcpServer.h"
#include "ResponseHandler.h"

class tHttpSession;

class tHttpServlet
{
  public:
   tHttpServlet() : pOwner(NULL) {}
   virtual ~tHttpServlet() {}

   virtual bool ProcessAndResponse() = 0;
 
  protected:
    tHttpSession *pOwner;

    void SendResponse400();
    void SendResponse200();
    void SendVersionAndPageClose();

    bool GetParameter(const char * Param) { return (NULL != FindParameter(Param,NULL,NULL)); }
    bool GetParameter(const char * Param, unsigned *pValue);


  private:
   char * tHttpServlet::FindParameter(const char * Param,  char **ppValue, uint8_t *pValueLen);
   friend class tHttpSession; void SetOwner(tHttpSession *owner) { pOwner = owner; }
};


class tHttpSession : public tTcpSession, public ResponseHandler
{
public:
  tHttpSession(EthernetClient aEthernetClient);
  virtual ~tHttpSession() { if (pServlet) delete pServlet; }

  String RequestBuffer;

protected:
   virtual bool doProcess();

private:  
   /**
    * Pointer to servlet providing data
    */
   tHttpServlet *pServlet;
   static const uint8_t HTTP_MAX_REQUEST_BUFFER_SIZE = 200;
};



class tHttpServer : public tTcpServer
{
public:
  tHttpServer() : tTcpServer(80) {}

protected:
  virtual tTcpSession* NewSession(EthernetClient aEthernetClient) { return new tHttpSession(aEthernetClient); }
};

tHttpServlet * ServletFactory(String *pRequestBuffer);

#endif  // HTTP_SERVER
