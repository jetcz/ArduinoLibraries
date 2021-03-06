/*
  Emon.cpp - Library for openenergymonitor
  Created by Trystan Lea, April 27 2010
  BADLY MODIFIED BY JETCZ - MAKE IT NONBLOCKING
  GNU GPL
  modified to use up to 12 bits ADC resolution (ex. Arduino Due)
  by boredman@boredomprojects.net 26.12.2013
  Low Pass filter for offset removal replaces HP filter 1/1/2015 - RW
  */

//#include "WProgram.h" un-comment for use on older versions of Arduino IDE
#include "EmonLib.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


//--------------------------------------------------------------------------------------
// Sets the pins to be used for voltage and current sensors
//--------------------------------------------------------------------------------------
void EnergyMonitor::voltage(unsigned int _inPinV, double _VCAL, double _PHASECAL)
{
	inPinV = _inPinV;
	VCAL = _VCAL;
	PHASECAL = _PHASECAL;
}

void EnergyMonitor::current(unsigned int _inPinI1, unsigned int _inPinI2, double _ICAL1, double _ICAL2)
{
	inPinI1 = _inPinI1;
	inPinI2 = _inPinI2;
	ICAL1 = _ICAL1;
	ICAL2 = _ICAL2;
}

//--------------------------------------------------------------------------------------
// Sets the pins to be used for voltage and current sensors based on emontx pin map
//--------------------------------------------------------------------------------------
void EnergyMonitor::voltageTX(double _VCAL, double _PHASECAL)
{
	inPinV = 2;
	VCAL = _VCAL;
	PHASECAL = _PHASECAL;
}

void EnergyMonitor::currentTX(unsigned int _channel, double _ICAL1, double _ICAL2)
{
	if (_channel == 1) inPinI1 = inPinI2 = 3;
	if (_channel == 2) inPinI1 = inPinI2 = 0;
	if (_channel == 3) inPinI1 = inPinI2 = 1;

	ICAL1 = _ICAL1;
	ICAL2 = _ICAL2;
}

//--------------------------------------------------------------------------------------
// emon_calc procedure
// Calculates realPower,apparentPower,powerFactor,Vrms,Irms,kWh increment
// From a sample window of the mains AC voltage and current.
// The Sample window length is defined by the number of half wavelengths or crossings we choose to measure.
//--------------------------------------------------------------------------------------
void EnergyMonitor::calcVI(unsigned int crossings, float vcc)
{
	//--------------------------------------------------------------------------------------
	// Variable declaration for emon_calc procedure
	//--------------------------------------------------------------------------------------
	static int sampleV;  							 //sample_ holds the raw analog read value
	static int sampleI1;
	static int sampleI2;
	static double lastFilteredV, filteredV;          //Filtered_ is the raw analog value minus the DC offset
	static double filteredI1;
	static double filteredI2;
	static double offsetV = ADC_COUNTS >> 1;;                          //Low-pass filter output
	static double offsetI1 = ADC_COUNTS >> 1;;                          //Low-pass filter output   
	static double offsetI2 = ADC_COUNTS >> 1;;
	static double phaseShiftedV;                             //Holds the calibrated phase shifted voltage.
	static double sqV, sumV, sqI1, sumI1, instP1, sumP1, sqI2, sumI2, instP2, sumP2;              //sq = squared, sum = Sum, inst = instantaneous
	static int startV;                                       //Instantaneous voltage at start of sample window.
	static bool lastVCross, checkVCross, measuring;                  //Used to measure number of times threshold is crossed.
	int SupplyVoltage = vcc;
	static unsigned int crossCount = 0;                             //Used to measure number of times threshold is crossed.
	static unsigned int numberOfSamples = 0;                        //This is now incremented  

	//-------------------------------------------------------------------------------------------------------------------------
	// 1) Do not wait here, just return if we are on sin curve far from 0
	//-------------------------------------------------------------------------------------------------------------------------

	if (!measuring)
	{
		startV = analogRead(inPinV);
		if ((startV < (ADC_COUNTS*0.55)) && (startV >(ADC_COUNTS*0.45))) { measuring = true; }
		else return;
	}

	//-------------------------------------------------------------------------------------------------------------------------
	// 2) Main measurement 
	//------------------------------------------------------------------------------------------------------------------------- 

	if (crossCount < crossings)
	{
		numberOfSamples++;                       //Count number of times looped.
		lastFilteredV = filteredV;               //Used for delay/phase compensation

		//-----------------------------------------------------------------------------
		// A) Read in raw voltage and current samples
		//-----------------------------------------------------------------------------
		sampleV = analogRead(inPinV);                 //Read in raw voltage signal
		sampleI1 = analogRead(inPinI1);                 //Read in raw current signal
		sampleI2 = analogRead(inPinI2);                 //Read in raw current signal

		//-----------------------------------------------------------------------------
		// B) Apply digital low pass filters to extract the 2.5 V or 1.65 V dc offset,
		//     then subtract this - signal is now centred on 0 counts.
		//-----------------------------------------------------------------------------
		offsetV = offsetV + ((sampleV - offsetV) / 1024.0);
		filteredV = sampleV - offsetV;
		offsetI1 = offsetI1 + ((sampleI1 - offsetI1) / 1024.0);
		filteredI1 = sampleI1 - offsetI1;
		offsetI2 = offsetI2 + ((sampleI2 - offsetI2) / 1024.0);
		filteredI2 = sampleI2 - offsetI2;

		//-----------------------------------------------------------------------------
		// C) Root-mean-square method voltage
		//-----------------------------------------------------------------------------  
		sqV = filteredV * filteredV;                 //1) square voltage values
		sumV += sqV;                                //2) sum

		//-----------------------------------------------------------------------------
		// D) Root-mean-square method current
		//-----------------------------------------------------------------------------   
		sqI1 = filteredI1 * filteredI1;                //1) square current values
		sumI1 += sqI1;                                //2) sum 
		sqI2 = filteredI2 * filteredI2;                //1) square current values
		sumI2 += sqI2;                                //2) sum 

		//-----------------------------------------------------------------------------
		// E) Phase calibration
		//-----------------------------------------------------------------------------
		phaseShiftedV = lastFilteredV + PHASECAL * (filteredV - lastFilteredV);

		//-----------------------------------------------------------------------------
		// F) Instantaneous power calc
		//-----------------------------------------------------------------------------   
		instP1 = phaseShiftedV * filteredI1;          //Instantaneous Power
		sumP1 += instP1;                               //Sum  
		instP2 = phaseShiftedV * filteredI2;          //Instantaneous Power
		sumP2 += instP2;                               //Sum  

		//-----------------------------------------------------------------------------
		// G) Find the number of times the voltage has crossed the initial voltage
		//    - every 2 crosses we will have sampled 1 wavelength 
		//    - so this method allows us to sample an integer number of half wavelengths which increases accuracy
		//-----------------------------------------------------------------------------       
		lastVCross = checkVCross;
		if (sampleV > startV) checkVCross = true;
		else checkVCross = false;
		if (numberOfSamples == 1) lastVCross = checkVCross;

		if (lastVCross != checkVCross) crossCount++;
		if (crossCount < crossings) return;
	}

	//-------------------------------------------------------------------------------------------------------------------------
	// 3) Post loop calculations
	//------------------------------------------------------------------------------------------------------------------------- 
	//Calculation of the root of the mean of the voltage and current squared (rms)
	//Calibration coefficients applied. 

	double t = ((SupplyVoltage / 1000.0) / (ADC_COUNTS));

	double V_RATIO = VCAL * t;
	Vrms = V_RATIO * sqrt(sumV / numberOfSamples);

	double I_RATIO1 = ICAL1 * t;
	double I_RATIO2 = ICAL2 * t;
	Irms1 = I_RATIO1 * sqrt(sumI1 / numberOfSamples);
	Irms2 = I_RATIO2 * sqrt(sumI2 / numberOfSamples);

	//Calculation power values
	realPower1 = V_RATIO * I_RATIO1 * sumP1 / numberOfSamples;
	apparentPower1 = Vrms * Irms1;
	powerFactor1 = realPower1 / apparentPower1;

	realPower2 = V_RATIO * I_RATIO2 * sumP2 / numberOfSamples;
	apparentPower2 = Vrms * Irms2;
	powerFactor2 = realPower2 / apparentPower2;

	//Reset accumulators
	sumV = 0;
	sumI1 = 0;
	sumP1 = 0;
	sumI2 = 0;
	sumP2 = 0;
	crossCount = 0;
	checkVCross = false;
	lastVCross = false;
	numberOfSamples = 0;
	measuring = false;
}


