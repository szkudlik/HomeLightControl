#include "OutputProcess.h"
#include "CommDefs.h"
#include "Eeprom.h"


void tOutputProcess::setup()
{
  #ifdef CONTROLLER
    Output[0].SetPin(28,0);
    Output[1].SetPin(29,0);
    Output[2].SetPin(30,0);
    Output[3].SetPin(31,0);
    Output[4].SetPin(32,0);
    Output[5].SetPin(33,0);
    Output[6].SetPin(34,0);
    Output[7].SetPin(35,0);
  #else
    uint8_t OutputPolarity;
    EEPROM.get(EEPROM_OUTPUT_POLARITY_OFFSET,OutputPolarity);

    Output[0].SetPin(A5,((OutputPolarity &  (1 << 0)) == 0) ? 0 : 1);
    Output[1].SetPin(A4,((OutputPolarity &  (1 << 1)) == 0) ? 0 : 1);
    Output[2].SetPin(A3,((OutputPolarity &  (1 << 2)) == 0) ? 0 : 1);
    Output[3].SetPin(A2,((OutputPolarity &  (1 << 3)) == 0) ? 0 : 1);
    Output[4].SetPin(A1,((OutputPolarity &  (1 << 4)) == 0) ? 0 : 1);
    Output[5].SetPin(A0,((OutputPolarity &  (1 << 5)) == 0) ? 0 : 1);
  #endif
}

void tOutputProcess::service()
{
	for (uint8_t i = 0; i < NUM_OF_OUTPUTS; i++)
	{
    Output[i].Tick();
	}	
}


uint8_t  tOutputProcess::GetOutputStateMap()
{
  uint8_t Map = 0;
  for (uint8_t i = 0; i < NUM_OF_OUTPUTS; i++)
  {
    if (Output[i].GetState()) 
      Map |= 1 << i;
  } 

  return Map;
}

uint8_t  tOutputProcess::GetOutputTimersStateMap()
{
  uint8_t Map = 0;
  for (uint8_t i = 0; i < NUM_OF_OUTPUTS; i++)
  {
    if (Output[i].GetTimer()) 
      Map |= 1 << i;
  } 

  return Map;
}


void tOutput::Tick()
{
  if (mTimer) 
  {
    mTimer--;
    if (0 == mTimer) SetState(0);
  }
}

void tOutput::SetState(uint8_t State)
{ 
  #ifdef DEBUG_2
  DEBUG_SERIAL.print(F("========================>Setting output:"));
  DEBUG_SERIAL.print(mPin,DEC);
  DEBUG_SERIAL.print(F(" to state "));
  if (State) DEBUG_SERIAL.println(F("ACTIVE"));
        else DEBUG_SERIAL.println(F("INACTICVE"));
  #endif
  mState = State; 
  if (PIN_NOT_ASSIGNED != mPin)
  {
     if (mPolarity)
     {
        digitalWrite(mPin,(mState ? HIGH : LOW)); //reversed
     }
     else
     {
        digitalWrite(mPin,(mState ? LOW  : HIGH)); // normal
     }
  }

}
