#include "OutputProcess.h"
#include "CommDefs.h"


void tOutputProcess::setup()
{
  #ifdef CONTROLLER
    Output[0].SetPin(28);
    Output[1].SetPin(29);
    Output[2].SetPin(30);
    Output[3].SetPin(31);
    Output[4].SetPin(32);
    Output[5].SetPin(33);
    Output[6].SetPin(34);
    Output[7].SetPin(35);
  #else
    Output[0].SetPin(A5);
    Output[1].SetPin(A4);
    Output[2].SetPin(A3);
    Output[3].SetPin(A2);
    Output[4].SetPin(A1);
    Output[5].SetPin(A0);
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
     digitalWrite(mPin,(State ? LOW : HIGH));
}
