#ifndef OUTPUT_PROCESS
#define OUTPUT_PROCESS
#include "common.h"

#include <ProcessScheduler.h>
#include "CommDefs.h"

#define OUTPUT_SERVICE_TIME 1000  // 1 second

class tOutput 
{
   static const uint8_t PIN_NOT_ASSIGNED = 255;
public:
  tOutput() :mPin(PIN_NOT_ASSIGNED) {}

  void SetPin(uint8_t pin, uint8_t Polarity) { mPin = pin; mPolarity = Polarity; pinMode(mPin, OUTPUT); SetState(0); }

  void Set(uint8_t State, uint16_t Timer, bool timerLongerOnly)
  {
     if (timerLongerOnly && (mTimer > Timer))
           Timer = mTimer;

     if (State) mTimer = Timer; else mTimer = 0;
     SetState(State);
  }
  void Toggle(uint16_t Timer) 
  { 
    SetState(! mState); 
    if (mState) mTimer = Timer; else mTimer = 0;
  }
  void Tick();

  uint16_t GetTimer() const { return (mTimer); }
  uint16_t GetState() const { return (mState); }

private:

  void SetState(uint8_t State);
  uint8_t mPin;
  uint16_t mTimer;
  uint8_t mState   : 1,
         mPolarity : 1,
         Reserved  : 6;
};

class tOutputProcess : public  Process
{
  public:
  tOutputProcess(Scheduler &manager) : 
    Process(manager,LOW_PRIORITY,OUTPUT_SERVICE_TIME) {}
  
  static const bool TimerLongerOnly = true;
  static const bool ForceTimer = false;

  void SetOutput(uint8_t outputId, uint8_t State, uint16_t timer, bool timerLongerOnly)
  { 
    if (outputId < NUM_OF_OUTPUTS) Output[outputId].Set(State,timer,timerLongerOnly);
  }

  void ToggleOutput(uint8_t outputId, uint16_t timer) 
  { 
    if (outputId < NUM_OF_OUTPUTS) Output[outputId].Toggle(timer);
  }
  
  uint8_t  GetOutputStateMap();
  uint8_t  GetOutputTimersStateMap();
  uint8_t  GetOutputState(uint8_t outputId) 
  { 
    if (outputId < NUM_OF_OUTPUTS) 
      return (Output[outputId].GetState());
    else return 0;
  }
  
  uint16_t GetOutputTimer(uint8_t outputId)
  { 
    if (outputId < NUM_OF_OUTPUTS) 
      return (Output[outputId].GetTimer());
    else return 0;
  }

  
  virtual void service();
  virtual void setup();
private:
  tOutput Output[NUM_OF_OUTPUTS];
};

extern tOutputProcess OutputProcess;

#endif  // OUTPUT_PROCESS
