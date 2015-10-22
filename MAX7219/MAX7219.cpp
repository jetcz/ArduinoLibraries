/*

 MAX7219 class
 Author: Nick Gammon
 Date:   17 March 2015


 PERMISSION TO DISTRIBUTE

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.


 LIMITATION OF LIABILITY

 The software is provided "as is", without warranty of any kind, express or implied,
 including but not limited to the warranties of merchantability, fitness for a particular
 purpose and noninfringement. In no event shall the authors or copyright holders be liable
 for any claim, damages or other liability, whether in an action of contract,
 tort or otherwise, arising from, out of or in connection with the software
 or the use or other dealings in the software.

*/

#include <SPI.h>

#include <MAX7219.h>
//#include <MAX7219_font.h>

/*


  Initialize:

	myDisplay.begin ();

  Shut down:

	myDisplay.end ();

  Write to display:

	myDisplay.sendString ("HELLO");

  Set the intensity (from 0 to 15):

	myDisplay.setIntensity (8);

  For the class to compile you need to include these three files:

	  SPI.h
	  bitBangedSPI.h
	  MAX7219.h

  You can obtain the bitBangedSPI library from:

	https://github.com/nickgammon/bitBangedSPI

*/



// destructor
MAX7219::~MAX7219()
{
	end();
} // end of destructor

void MAX7219::begin()
{
	pinMode(load_, OUTPUT);
	digitalWrite(load_, HIGH);


	SPI.begin();
	SPI.setDataMode(SPI_MODE0);
	SPI.setClockDivider(SPI_CLOCK_DIV128);

	sendToAll(MAX7219_REG_SCANLIMIT, 7);     // show 8 digits
	sendToAll(MAX7219_REG_DECODEMODE, 0);    // use bit patterns
	sendToAll(MAX7219_REG_DISPLAYTEST, 0);   // no display test
	sendToAll(MAX7219_REG_INTENSITY, 1);    // character intensity: range: 0 to 15
	sendString("");                          // clear display
	sendToAll(MAX7219_REG_SHUTDOWN, 1);      // not in shutdown mode (ie. start it up)
} // end of MAX7219::begin

void MAX7219::end()
{
	sendToAll(MAX7219_REG_SHUTDOWN, 0);  // shutdown mode (ie. turn it off)


	SPI.end();

} // end of MAX7219::end

void MAX7219::setIntensity(const byte amount)
{
	sendToAll(MAX7219_REG_INTENSITY, amount & 0xF);     // character intensity: range: 0 to 15
} // end of MAX7219::setIntensity

// send one byte to MAX7219
void MAX7219::sendByte(const byte reg, const byte data)
{
	SPI.transfer(reg);
	SPI.transfer(data);

}  // end of sendByte

void MAX7219::sendToAll(const byte reg, const byte data)
{
	digitalWrite(load_, LOW);
	for (byte chip = 0; chip < chips_; chip++)
		sendByte(reg, data);
	digitalWrite(load_, HIGH);
}  // end of sendToAll

// send one character (data) to position (pos) with or without decimal place
// pos is 0 to 7
void MAX7219::sendChar(const byte pos, const char data, const bool dp)
{

	const byte MAX7219_font[91] = {
	0b0000000, // ' '
	MAX7219::HYPHEN, // '!'
	MAX7219::HYPHEN, // '"'
	MAX7219::HYPHEN, // '#'
	MAX7219::HYPHEN, // '$'
	MAX7219::HYPHEN, // '%'
	MAX7219::HYPHEN, // '&'
	MAX7219::HYPHEN, // '''
	0b1001110,       // '('   - same as [
	0b1111000,       // ')'   - same as ]
	MAX7219::HYPHEN, // '*'
	MAX7219::HYPHEN, // '+'
	MAX7219::HYPHEN, // ','
	MAX7219::HYPHEN, // '-' - LOL *is* a hyphen
	0b0000000,       // '.'  (done by turning DP on)
	MAX7219::HYPHEN, // '/'
	0b1111110,       // '0'
	0b0110000,       // '1'
	0b1101101,       // '2'
	0b1111001,       // '3'
	0b0110011,       // '4'
	0b1011011,       // '5'
	0b1011111,       // '6'
	0b1110000,       // '7'
	0b1111111,       // '8'
	0b1111011,       // '9'
	MAX7219::HYPHEN, // ':'
	MAX7219::HYPHEN, // ';'
	MAX7219::HYPHEN, // '<'
	MAX7219::HYPHEN, // '='
	MAX7219::HYPHEN, // '>'
	MAX7219::HYPHEN, // '?'
	MAX7219::HYPHEN, // '@'
	0b1110111,       // 'A'
	0b0011111,       // 'B'
	0b1001110,       // 'C'  
	0b0111101,       // 'D'
	0b1001111,       // 'E'
	0b1000111,       // 'F'
	0b1011110,       // 'G'
	0b0110111,       // 'H'
	0b0110000,       // 'I' - same as 1
	0b0111100,       // 'J'  
	MAX7219::HYPHEN, // 'K'
	0b0001110,       // 'L'
	MAX7219::HYPHEN, // 'M'
	0b0010101,       // 'N'
	0b1111110,       // 'O' - same as 0
	0b1100111,       // 'P'
	MAX7219::HYPHEN, // 'Q'
	0b0000101,       // 'R'
	0b1011011,       // 'S'
	0b0000111,       // 'T'
	0b0111110,       // 'U'
	MAX7219::HYPHEN, // 'V'
	MAX7219::HYPHEN, // 'W'
	MAX7219::HYPHEN, // 'X'
	0b0100111,       // 'Y'
	MAX7219::HYPHEN, // 'Z'
	0b1001110,       // '['  - same as C  
	MAX7219::HYPHEN, // backslash
	0b1111000,       // ']' 
	MAX7219::HYPHEN, // '^'
	0b0001000,       // '_'
	MAX7219::HYPHEN, // '`'
	0b1110111,       // 'a'
	0b0011111,       // 'b'
	0b0001101,       // 'c'
	0b0111101,       // 'd'
	0b1001111,       // 'e'
	0b1000111,       // 'f'
	0b1011110,       // 'g'
	0b0010111,       // 'h'
	0b0010000,       // 'i' 
	0b0111100,       // 'j'
	MAX7219::HYPHEN, // 'k'
	0b0001110,       // 'l'
	MAX7219::HYPHEN, // 'm'
	0b0010101,       // 'n'
	0b1111110,       // 'o' - same as 0
	0b1100111,       // 'p'
	MAX7219::HYPHEN, // 'q'
	0b0000101,       // 'r'
	0b1011011,       // 's'
	0b0000111,       // 't'
	0b0011100,       // 'u'
	MAX7219::HYPHEN, // 'v'
	MAX7219::HYPHEN, // 'w'
	MAX7219::HYPHEN, // 'x'
	0b0100111,       // 'y'
	MAX7219::HYPHEN, // 'z'
	};  //  end of MAX7219_font

	byte converted = 0b0000001;    // hyphen as default

	// look up bit pattern if possible
	if (data >= ' ' && data <= 'z')
		converted = MAX7219_font[data - ' '];
	// 'or' in the decimal point if required
	if (dp)
		converted |= 0b10000000;

	// start sending
	digitalWrite(load_, LOW);

	// segment is in range 1 to 8
	const byte segment = 8 - (pos % 8);
	// for each daisy-chained display we need an extra NOP
	const byte nopCount = pos / 8;
	// send extra NOPs to push the data out to extra displays
	for (byte i = 0; i < nopCount; i++)
		sendByte(MAX7219_REG_NOOP, MAX7219_REG_NOOP);
	// send the segment number and data
	sendByte(segment, converted);
	// end with enough NOPs so later chips don't update
	for (int i = 0; i < chips_ - nopCount - 1; i++)
		sendByte(MAX7219_REG_NOOP, MAX7219_REG_NOOP);

	// all done!
	digitalWrite(load_, HIGH);
}  // end of sendChar

// write an entire null-terminated string to the LEDs
void MAX7219::sendString(const char * s)
{
	byte pos;

	for (pos = 0; pos < (chips_ * 8) && *s; pos++)
	{
		boolean dp = s[1] == '.';
		sendChar(pos, *s++, dp);   // turn decimal place on if next char is a dot
		if (dp)  // skip dot
			s++;
	}

	// space out rest
	while (pos < (chips_ * 8))
		sendChar(pos++, ' ');


}  // end of sendString

void MAX7219::sendString(String s)
{
   MAX7219::sendString((char*)s.c_str());     
}  // end of sendString

