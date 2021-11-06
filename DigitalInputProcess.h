#ifndef DIGITAL_INPUT_PROCESS
#define DIGITAL_INPUT_PROCESS
#include "common.h"

#include <ProcessScheduler.h>

#define DIGITAL_INPUT_PROCESS_SERVICE_TIME 50


//TODO:  double click
class Button
{
public:
	Button() : mActiveTime(0), mLastClickTime(0) {}

	
	static const uint8_t STATE_NO_CHANGE = 0;
	static const uint8_t STATE_CLICK = 1;
	static const uint8_t STATE_DOUBLECLICK = 2;
	static const uint8_t STATE_LONGCLICK = 3;

	uint8_t Process();
  void SetPin(uint8_t pin, uint8_t ActiveState) { mActiveState = ActiveState; mPin = pin; pinMode(mPin, INPUT_PULLUP);}
  uint8_t GetCurrentState() { return (digitalRead(mPin) == mActiveState); } 
	
private:
// a short click - SHORT_CLICK_TICKS number of "1" seen n a sequence
  static const uint8_t SHORT_CLICK_TICKS = 2;   // 100ms

// a long click - LONG_CLICK_TICKS number of "1" seen in a sequence
  static const uint8_t LONG_CLICK_TICKS  = 20;  // 1000ms


	uint8_t mPin;

  // timestamps counted in ticks, each call to Process() is a tick
  // 0 means - never seen (>255 ticks), 1 means - seen in current tick
	
	uint8_t mActiveTime;    // number of ticks the button is constantly seen as active
                          // 0 - during the last tick the button was not active
                          // 255 - overflow, button is active for an unspecified time
   uint8_t mLastClickTime; // number of ticks the last short click occured
                          // 0 - the click occured during the last tick
                          // 255 - overflow, button is active for an unspecified time
   uint8_t mActiveState;
};


//TODO: all pins, currently D2 and D3 only 

class DigitalInputProcess : public Process
{
  public:
  DigitalInputProcess(Scheduler &manager) : 
    Process(manager,LOW_PRIORITY,DIGITAL_INPUT_PROCESS_SERVICE_TIME) {}
  
  virtual void setup();
  virtual void service();
  
private:
  static const uint8_t mNumOfActiveButtons = 12;
  
  Button mButton[mNumOfActiveButtons];  
};

extern DigitalInputProcess DigitalInput;



#endif  // DIGITAL_INPUT_PROCESS
