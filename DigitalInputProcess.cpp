#include "DigitalInputProcess.h"
#include "CommSender.h"
#include "WorkerProcess.h"
#include "Eeprom.h"
 

void DigitalInputProcess::setup()
{
    uint16_t InputPolarity;
    EEPROM.get(EEPROM_INPUT_POLARITY_OFFSET,InputPolarity);

    mButton[0].SetPin(2,((InputPolarity &  (1 << 0)) == 0) ? LOW : HIGH);
    mButton[1].SetPin(3,((InputPolarity &  (1 << 1)) == 0) ? LOW : HIGH);
    mButton[2].SetPin(4,((InputPolarity &  (1 << 2)) == 0) ? LOW : HIGH);
    mButton[3].SetPin(5,((InputPolarity &  (1 << 3)) == 0) ? LOW : HIGH);
    mButton[4].SetPin(6,((InputPolarity &  (1 << 4)) == 0) ? LOW : HIGH);
    mButton[5].SetPin(7,((InputPolarity &  (1 << 5)) == 0) ? LOW : HIGH);
    mButton[6].SetPin(8,((InputPolarity &  (1 << 6)) == 0) ? LOW : HIGH);
    mButton[7].SetPin(9,((InputPolarity &  (1 << 7)) == 0) ? LOW : HIGH);
    mButton[8].SetPin(10,((InputPolarity & (1 << 8)) == 0) ? LOW : HIGH);
    mButton[9].SetPin(11,((InputPolarity & (1 << 9)) == 0) ? LOW : HIGH);
    mButton[10].SetPin(12,((InputPolarity & (1<<10)) == 0) ? LOW : HIGH);
    mButton[11].SetPin(13,((InputPolarity & (1<<11)) == 0) ? LOW : HIGH);
}

void DigitalInputProcess::service()
{
  uint16_t ShortClick = 0;
  uint16_t LongClick = 0;
  uint16_t DoubleClick = 0;

  for (int i = 0; i < mNumOfActiveButtons; i++)
  {
    uint8_t State = mButton[i].Process();
    switch (State)
    {
      case Button::STATE_CLICK:
        ShortClick |= (1 << i);
        break;
        
      case Button::STATE_LONGCLICK:
        LongClick |= (1 << i);
        break;

      case Button::STATE_DOUBLECLICK:
        DoubleClick |= (1 << i);
        break;
    }
  }

  // do we need to send a frame?
  if ((ShortClick) | (LongClick) | (DoubleClick))
  {
      Worker.SendMsgButtonPress(DEVICE_ID_BROADCAST,0,ShortClick,LongClick,DoubleClick);
  }
}


uint8_t Button::Process()
{
  uint8_t State = STATE_NO_CHANGE;
  // check current state
  uint8_t CurrentState = GetCurrentState();

  // set mActiveTime
  if (CurrentState == HIGH)
  {
    if (mActiveTime == 0) mActiveTime = 1;
    else if (mActiveTime < 255) mActiveTime += 1;
  }
  else
  {
    mActiveTime = 0;
  }

  if (mActiveTime == SHORT_CLICK_TICKS) State = STATE_CLICK;
  if (mActiveTime == LONG_CLICK_TICKS)  State = STATE_LONGCLICK;

  // set mLastClickTime
  if (mLastClickTime < 255) mLastClickTime += 1;
  if (State == STATE_CLICK)
  {
      if (mLastClickTime <= EEPROM.read(EEPROM_DOUBLE_CLICK_TIME_OFFSET))
      {
            State = STATE_DOUBLECLICK;
      }
      else
      {
             mLastClickTime = 0;
      }
  }

  return State;
}
