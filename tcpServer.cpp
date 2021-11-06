#include "tcpServer.h"
#include "version.h"
#include "common.h"

tTcpServer* tTcpServer::pFirst = NULL;

bool tTcpSession::Process() 
{ 
  if (! mEthernetClient.connected())
  {
    return false;
  }
  return doProcess(); 
}


void tTcpSession::SendFlashString(const char * str)
{
  SendFlashString(str,strlen_P(str));
}


void tTcpSession::SendFlashString(const char * str, size_t size)
{
  char buffer[BUFFER_SIZE];
  
  while(size)
  {    
    strncpy_P(buffer,str,BUFFER_SIZE);
    str += BUFFER_SIZE;
    if (size >= BUFFER_SIZE)
    {
       mEthernetClient.write(buffer,BUFFER_SIZE);
       size -= BUFFER_SIZE; 
    }
    else
    {
       mEthernetClient.write(buffer,size);
       size = 0;
    }
  }
}

void tTcpServerProcess::setup()
{
  tTcpServer* pServer = tTcpServer::GetFirst();
  while (NULL != pServer) 
  {
    pServer->setup();
    pServer = pServer->GetNext();
  }
  
  #ifdef DEBUG_3
  DEBUG_SERIAL.print(F("server address: "));
  DEBUG_SERIAL.println(Ethernet.localIP());
  #endif
}

void tTcpServerProcess::service()
{
  tTcpServer* pServer = tTcpServer::GetFirst();
  while (NULL != pServer) 
  {
    EthernetClient newClient = pServer->Process();
    if (newClient) 
    {
      bool NotFound = true;
      #ifdef DEBUG_3
      DEBUG_SERIAL.print (F("New connection from "));
      DEBUG_SERIAL.print (newClient.remoteIP());
      #endif
      for (uint8_t i = 0; i < NUM_OF_CONCURRENT_SESSIONS; i++)
      {
        if (clients[i] == NULL)
        {
            NotFound = false;
            clients[i] = pServer->NewSession(newClient);
            break;
        }
      }
  
      if (NotFound)
      {
        DEBUG_PRINTLN_3(" rejected - no client slots");
        newClient.stop();
      }
    }
    pServer = pServer->GetNext();    
  }

  // proceed all open clients
  for (uint8_t i = 0; i < NUM_OF_CONCURRENT_SESSIONS; i++) 
  {
    if (NULL != clients[i]) 
    {
      bool KeepOpen = clients[i]->Process();
      if (! KeepOpen) 
      {
        delete (clients[i]);
        clients[i] = NULL;
      }
    }
  }  
}
