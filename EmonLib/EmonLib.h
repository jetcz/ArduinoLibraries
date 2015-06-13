/*
  Emon.h - Library for openenergymonitor
  Created by Trystan Lea, April 27 2010
  GNU GPL
  modified to use up to 12 bits ADC resolution (ex. Arduino Due)
  by boredman@boredomprojects.net 26.12.2013
  Low Pass filter for offset removal replaces HP filter 1/1/2015 - RW
  */

#ifndef EmonLib_h
#define EmonLib_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif


// to enable 12-bit ADC resolution on Arduino Due, 
// include the following line in main sketch inside setup() function:
//  analogReadResolution(ADC_BITS);
// otherwise will default to 10 bits, as in regular Arduino-based boards.
#if defined(__arm__)
#define ADC_BITS    12
#else
#define ADC_BITS    10
#endif

#define ADC_COUNTS  (1<<ADC_BITS)


class EnergyMonitor
{
public:

	void voltage(unsigned int _inPinV, double _VCAL, double _PHASECAL);
	void current(unsigned int _inPinI1, unsigned int _inPinI2, double _ICAL);

	void voltageTX(double _VCAL, double _PHASECAL);
	void currentTX(unsigned int _channel, double _ICAL);

	void calcVI(unsigned int crossings, float vcc);


	//Useful value variables
	double realPower1,
		apparentPower1,
		powerFactor1,
		Vrms,
		Irms1,
		realPower2,
		apparentPower2,
		powerFactor2,
		Irms2;

private:
	unsigned int inPinV;
	unsigned int inPinI1;
	unsigned int inPinI2;
	//Calibration coefficients
	//These need to be set in order to obtain accurate results
	double VCAL;
	double ICAL;
	double PHASECAL;

};

#endif
