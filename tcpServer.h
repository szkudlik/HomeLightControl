#ifndef TCP_SERVER
#define TCP_SERVER
#include "common.h"
#include <Ethernet.h>

#include <ProcessScheduler.h>
#include "CommDefs.h"

class tTcpSession;
class tTcpServer
{
public:
  static tTcpServer *GetFirst() { return pFirst; }
  tTcpServer *GetNext() { return pNext; }
  
  void setup() { server.begin(); }
  EthernetClient Process() { return server.accept(); }
  virtual tTcpSession* NewSession(EthernetClient aEthernetClient) = 0;

protected:
  tTcpServer(uint16_t ServerPort) : server(ServerPort) { pNext = pFirst; pFirst = this; }

  EthernetServer server;

private:
  static tTcpServer* pFirst;
  tTcpServer *pNext;
};


class tTcpSession
{
public:
  tTcpSession(EthernetClient aEthernetClient) 
  {
    mEthernetClient = aEthernetClient; 
  }
  
  virtual ~tTcpSession() 
  {
    DEBUG_PRINTLN_3("Session terminated");
    mEthernetClient.stop();
  }

  // if return false - the session should be deleted
  bool Process();

  void SendFlashString(const char * str, size_t size);
  void SendFlashString(const char * str);

  EthernetClient mEthernetClient;
  
protected:
  // if return false - the session should be deleted
  virtual bool doProcess() = 0;

private:
   static const uint8_t BUFFER_SIZE = 100; // data portion to be sent in one frame, buffer on stack

};

class tTcpServerProcess : public Process
{
public:
  tTcpServerProcess(Scheduler &manager) : 
    Process(manager,LOW_PRIORITY,10)   
    { }
  
protected:
  virtual void setup();
  virtual void service();

  static uint8_t const NUM_OF_CONCURRENT_SESSIONS = 6;
  tTcpSession* clients[NUM_OF_CONCURRENT_SESSIONS];
};



#endif  // TCP_SERVER
