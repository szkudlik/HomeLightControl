#ifndef TELNET_SERVER
#define TELNET_SERVER
#include "common.h"
#include "tcpServer.h"
#include "ResponseHandler.h"
#include <Commander.h>

class tTelnetSession : public tTcpSession, public ResponseHandler
{
public:
  tTelnetSession(EthernetClient aEthernetClient);
  virtual ~tTelnetSession();

protected:
  virtual bool doProcess();

  virtual void vLog(uint8_t str);
};



class tTelnetServer : public tTcpServer
{
public:
  tTelnetServer() : tTcpServer(23) {}

protected:
  virtual tTcpSession* NewSession(EthernetClient aEthernetClient) { return new tTelnetSession(aEthernetClient); }
};



#endif  // WORKER_PROCESS
