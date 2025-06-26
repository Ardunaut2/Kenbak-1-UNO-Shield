#include <Arduino.h>
#include "Buttons2.h"

byte Buttons::m_pMap[] = // eg logical eBit2 is actually bit6 from the 165
{
  0, 1, 6, 7, 4, 5, 2, 3,
  11, 12, 13, 14, 15, 9, 8, 10
};

void Buttons::Init()
{
  // set no prev state
  m_wPrevState = 0xFFFF;
  m_wPrevReading = 0xFFFF;
  m_iTransitionTimeMS = millis();
}


word Buttons::ShiftIn()
{
  // read 16 bits of button statuses
  delayMicroseconds(262);        // to mach the original ShiftIn() call duration.
  return keysState;;             // keyState is updated by the TIMER1 interrupt
}

#if 0
// Time-based debouncing, ifdef'd out because I'm not sure of the impact on execution
bool Buttons::GetButtons(word& State, word& NewPressed, bool deBounce)
{
  // get the current raw state and any that have changed to down
  unsigned long nowMS = millis();
  word ThisState = ShiftIn();
  bool deBounced = !deBounce;
  if (deBounce)
  {
      if (ThisState != m_wPrevReading)
      {
        // state change, reset the timer
        m_wPrevReading = ThisState;
        m_iTransitionTimeMS = nowMS;
      }
      else if ((nowMS - m_iTransitionTimeMS) >= 20UL)
      {
          // held for long enough
          deBounced = true;
      }
  }
  
  if (ThisState != m_wPrevState && deBounced)
  {
    NewPressed = ThisState & (~m_wPrevState);  // only those that have *changed* from OFF to ON, i.e. DOWN

    m_wPrevState = ThisState;
    State = ThisState;
    return true;
  }
  return false;
}
#else
bool Buttons::GetButtons(word& State, word& NewPressed, bool Wait)
{
  // get the current raw state and any that have changed to down
  word ThisState = ShiftIn();
  
  if (Wait)
  {
    // simple de-bounce, if requested
    delay(20);
    word DebouncedState = ShiftIn();
    if (DebouncedState != ThisState)
      return false;
  }
  
  if (ThisState != m_wPrevState)
  {
    NewPressed = ThisState & (~m_wPrevState);  // only those that have *changed* from OFF to ON, i.e. DOWN

    m_wPrevState = ThisState;
    State = ThisState;
    return true;
  }
  return false;
}
#endif

bool Buttons::IsPressed(word BtnState, int Btn)
{
  // is the Btn down (in the given state)
  return bitRead(BtnState, m_pMap[Btn]);
}

bool Buttons::GetButtonDown(word BtnState, int& Btn)
{
  // return the first Btn down in the state
  for (Btn = eBit0; Btn <= eUnused; Btn++)
  {
    if (IsPressed(BtnState, Btn))
      return true;
  }
  return false;
}


Buttons buttons = Buttons();
