#include <Arduino.h>

class MAX7219
  {
  // pins
  const byte chips_;
  const byte load_;

  void sendByte (const byte reg, const byte data);
  void sendToAll (const byte reg, const byte data);

  // registers
  enum { 
       MAX7219_REG_NOOP        = 0x0,
      // codes 1 to 8 are digit positions 1 to 8
       MAX7219_REG_DECODEMODE  = 0x9,
       MAX7219_REG_INTENSITY   = 0xA,
       MAX7219_REG_SCANLIMIT   = 0xB,
       MAX7219_REG_SHUTDOWN    = 0xC,
       MAX7219_REG_DISPLAYTEST = 0xF,
      }; // end of enum


  public:
    // constructor
    MAX7219 (const byte chips, 
             const byte load
			)
       : chips_ (chips), load_ (load) { }
    
    ~MAX7219 ();  // destructor
    void begin ();
    void end ();

    void sendChar (const byte pos, const char data, const bool dp = false);
    void sendString (const char * s);
    void sendString (String s);
    void setIntensity (const byte amount);  // 0 to 15

  static const byte HYPHEN = 0b0000001;

  }; // end of class MAX7219
