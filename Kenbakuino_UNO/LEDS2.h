#ifndef LEDS2_H
#define LEDS2_H

extern byte ledsData;  
extern byte ledsControl;

class LEDs
{
public:
  void Init();
  void Display(byte Data, byte Control);

private:
  byte m_LastData;
  byte m_LastControl;
  
};

extern LEDs leds;
#endif
