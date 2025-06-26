/* Kenbakuino_UNO.ino  Ardunaut Jun 26,2025
 *   -Replacing       by
 *      Pins.h        Pins2.h
 *      LEDS.h        LEDS2.h
 *      LEDS.cpp      LEDS2.cpp
 *      Buttons.h     Buttons2.h
 *      Buttons.cpp   Buttons2.cpp
 *   -Added in main sketch:
 *      TIMER1 Interrupt Service Routine to read buttons and update leds.
*/

// ==================================================================
// ==================================================================
//      K E N B A K u i n o
//      Mark Wilson 2011
//  Software emulation of a KENBAK-1.
//  Released under Creative Commons Attribution, CC BY
//  (kiwimew at gmail dot com)
//  Sep 2011: Initial version.
//  Jan 2012: Changes to compile under v1.0 if the IDE (Arduino.h, changes to Wire)
//  May 2014: Corrected control switch order in schematic, 
//            typos in documents (no code changes)
//  Jun 2015: Changes to compile under v1.6.4 if the IDE, (#define for deprecated prog_uchar)  
//  Dec 2018: Corrected SDA/SCL pins, clarified that RTC is a DS1307 module/breakout (i.e. with XTAL)
//            GitHub. Serial read/write memory
//  May 2019: Added Auto-run program at start-up
//            Added Clock consts for no RTC or RTC with no SRAM
//  Jun 2019: Added system extensions to read/write EEPROM
//  May 2021: Fixed right-shift sign extension and issues with multi-bit roll instructions (see CPU_LEGACY_SHIFT_ROLL)
//  Sep 2022: Fixed program counter increment to happen after instruction executed (see CPU_LEGACY_PROGRAM_COUNTER)
//  Nov 2024: Turn RUN LED off when HALT encountered or STOP pressed (see MCP_LEGACY_RUN_LED)
// ==================================================================

// WTF.  We need to include these in the "main sketch" if we want to include them in "tabs"?
#include <Arduino.h>
#include <Wire.h>

#include "PINS2.h"
#include "Config.h"
#include "Clock.h"
#include "LEDS2.h"
#include "Buttons2.h"
#include "CPU.h"
#include "MCP.h"
#include "Memory.h"

#define COLMASK 0b11110000  //(to be used in the PORTD access)

/*------- KeyState key bits, following ken_pins.txt  order ------------------- */
enum {key0, key1, key6, key7, key4, key5, key2, key3,
  keyStop, keyStart, noKey, keyClear, keyDisplay, keySet, keyRead, keyStore
};

/*------- ledsData bits, following ken_pins.txt  order ------------------- */
 enum {led0, led1, led2, led3, led4, led5, led6, led7,
  };

/*------- ledsControl bits, following MCP.h  order ------------------- */
 enum {inputLed, addressLed, memoryLed, runLed,  noneLed
  };

 byte ledsData = 0x00 ;              // declared as extern in LEDS2.h
 byte ledsControl = 0x00 ;           // declared as extern in LEDS2.h
 volatile word keysState = 0xFFFF;   // declared as extern in Buttons2.h
 byte counter=0;
 
ExtendedCPU cpu = ExtendedCPU();

void setup() 
{
  Serial.begin(38400);
  PORTD = PORTD & ~COLMASK;      // all column data LOW.).  
  pinMode(LEDS1, OUTPUT);
  pinMode(LEDS2, OUTPUT);
  pinMode(LEDS3, OUTPUT);
  pinMode(KEYS1, INPUT_PULLUP);
  pinMode(KEYS2, INPUT_PULLUP);
  pinMode(KEYS3, INPUT_PULLUP);
  pinMode(KEYS4, INPUT_PULLUP);
  pinMode(RUNLED, OUTPUT);      // Not needed, uses analogWrite()
  
  cli();                   // Disable interrupts ---------------------------------
// Set TIMER1 interrupts at 1.244KHz (to avoid flickering leds at max count speed):
  TCCR1A = 0;              // set entire TCCR1A register to 0
  TCCR1B = 0;              // same for TCCR1B
  TCNT1  = 0;              //initialize counter value to 0
//  OCR1A = 249;             // = 16.000.000 / (1000*64) - 1 (1KHz)  // Sets compare match register
  OCR1A = 200;             // = 16.000.000 / (1244*64) - 1 (1.244KHz)  // Sets compare match register
//  OCR1A = 124;             // = 16.000.000 / (2000*64) - 1 (2KHz)  // Sets compare match register
  TCCR1B |= (1 << WGM12);              // Turns on CTC mode.
  TCCR1B |= (1 << CS11) | (1 << CS10); // Sets CS11 and CS10 bits for 64 prescaler.
  TIMSK1 |= (1 << OCIE1A);             // Enables the timer compare interrupt.

  sei();                   // Enable interrupts ---------------------------------
  delay(10);               // time to refresh keysState     


  clock.Init();
  buttons.Init();
  config.Init();
  leds.Init();
  cpu.Init();
  memory.Init();
  mcp.Init();
}
 
void loop() 
{
  mcp.Loop();
}

/*------------------------------------------------------------------------------
TIMER1 Interrupt Service Routine
------------------------------------------------------------------------------*/
ISR(TIMER1_COMPA_vect){            //timer1 interrupt 1KHz
static byte column = 0b00010000;   //pointing to the first column (left side)

  digitalWrite(LEDS1,LOW);
  digitalWrite(LEDS2,LOW);
  digitalWrite(LEDS3,LOW);
  
  if(column == 0b10000000) column = 0b00010000; //points to the first column
  else column = column << 1;   // point to next column
  DDRD = DDRD & ~COLMASK;      // sets all column pins as inputs
  DDRD = PORTD | column;       // select the new column as output LOW.

  // here update the leds values and read the pushbuttons:
  if (bitRead(column, 4)== 1){
    bitWrite(keysState, key7, !digitalRead(KEYS1));
    bitWrite(keysState, key3, !digitalRead(KEYS2));
    bitWrite(keysState, keyStart, !digitalRead(KEYS3));
    bitWrite(keysState, keyStop, !digitalRead(KEYS4));
    digitalWrite(LEDS1,bitRead(ledsData, led7));
    digitalWrite(LEDS2,bitRead(ledsData, led3));
    digitalWrite(LEDS3,bitRead(ledsControl, inputLed));
  }
  if (bitRead(column, 5)== 1){
    bitWrite(keysState, key6, !digitalRead(KEYS1));
    bitWrite(keysState, key2, !digitalRead(KEYS2));
    bitWrite(keysState, keyStore, !digitalRead(KEYS3));
    bitWrite(keysState, keyDisplay, !digitalRead(KEYS4));
    digitalWrite(LEDS1,bitRead(ledsData, led6));
    digitalWrite(LEDS2,bitRead(ledsData, led2));
    digitalWrite(LEDS3,bitRead(ledsControl, addressLed));
  }
  if (bitRead(column, 6)== 1){
    bitWrite(keysState, key5, !digitalRead(KEYS1));
    bitWrite(keysState, key1, !digitalRead(KEYS2));
    bitWrite(keysState, keyRead, !digitalRead(KEYS3));
    bitWrite(keysState, keySet, !digitalRead(KEYS4));
    digitalWrite(LEDS1,bitRead(ledsData, led5));
    digitalWrite(LEDS2,bitRead(ledsData, led1));
    digitalWrite(LEDS3,bitRead(ledsControl, memoryLed));    
  }
  if (bitRead(column, 7)== 1){
    bitWrite(keysState, key4, !digitalRead(KEYS1));
    bitWrite(keysState, key0, !digitalRead(KEYS2));
    bitWrite(keysState, keyClear, !digitalRead(KEYS3));
    digitalWrite(LEDS1,bitRead(ledsData, led4));
    digitalWrite(LEDS2,bitRead(ledsData, led0));

    if(bitRead(ledsControl,runLed))
    {
      // the Run LED can do PWM, use the upper 4 bits.
      analogWrite(RUNLED,(0xFF-(ledsControl & 0xF0)));    // intensity
    }
    else
    {
      digitalWrite(RUNLED, LOW);      // Turn off  RUN Led.
    }
  }

}
