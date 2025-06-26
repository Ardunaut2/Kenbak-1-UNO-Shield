#ifndef BUTTONS2_H
#define BUTTONS2_H

//word getKeysState();
extern volatile word keysState;

// buttons/keys

class Buttons
{
public:
  enum tButtons
  {
    eBit0,
    eBit1,
    eBit2,
    eBit3,
    eBit4,
    eBit5,
    eBit6,
    eBit7,

    eInputClear,
    eAddressDisplay,
    eAddressSet,
    eMemoryRead,
    eMemoryStore,
    eRunStart,
    eRunStop,
    eUnused
  };

  void Init();

  bool GetButtons(word& State, word& NewPressed, bool Wait);
  bool IsPressed(word BtnState, int Btn);
  bool GetButtonDown(word BtnState, int& Btn);

private:
  word ShiftIn();
  static byte m_pMap[];
  word m_wPrevState;
  
  word m_wPrevReading;
  unsigned long m_iTransitionTimeMS;  
};

extern Buttons buttons;
#endif
