#include <Arduino.h>
#include "PINS2.h"
#include "LEDS2.h"

void LEDs::Init()
{
  ledsData = 0x00;
  m_LastData = 0x00;

  ledsControl = 0x00;  
  m_LastControl = 0x00;
}

void LEDs::Display(byte DataL, byte ControlL)
{
  // update the data leds:
  if (DataL != m_LastData)
  {
    ledsData= DataL;
    m_LastData = ledsData;
    delayMicroseconds(149);      // to mach the original ShiftOut() duration.
  }
  
  // update the control leds:  
  if (ControlL != m_LastControl)
  {
    ledsControl= ControlL;
    m_LastControl = ControlL;
  }
}

LEDs leds = LEDs();
