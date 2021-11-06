#ifndef COMM_SENDER
#define COMM_SENDER
#include "common.h"

#include <ProcessScheduler.h>
#include "Random.h"
#include "CommDefs.h"
#include <ArduinoQueue.h>

#ifdef CONTROLLER
  #define OUTPUT_QUEUE_SIZE 140
#else
  #define OUTPUT_QUEUE_SIZE 3
#endif

#define MAX_TRANSMIT_DELAY 100

class CommSenderProcess : public Process
{
  public:
  CommSenderProcess(Scheduler &manager);

  void Enqueue(uint8_t DstDevId, uint8_t MessageType, uint8_t DataSize, void *pData);
  
  virtual void service();

private:
  typedef struct 
  {
    uint8_t DstDevId;
    uint8_t MessageType;
    uint8_t DataSize;
    uint8_t Data[COMMUNICATION_PAYLOAD_DATA_SIZE];
  } tQueueItem;

  ArduinoQueue<tQueueItem> mQueue;
  
  tCommunicationFrame mFrame;
  uint8_t mDataSize;
  Random mRandom;
  uint8_t mRetransLeft;
  bool isSending;

  bool DequeueFrame();      
};




extern CommSenderProcess CommSender;


#endif
