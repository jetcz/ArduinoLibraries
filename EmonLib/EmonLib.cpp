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

void EnergyMonitor::current(unsigned int _inPinI, double _ICAL)
{
	inPinI = _inPinI;
	ICAL = _ICAL;
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

void EnergyMonitor::currentTX(unsigned int _channel, double _ICAL)
{
	if (_channel == 1) inPinI = 3;
	if (_channel == 2) inPinI = 0;
	if (_channel == 3) inPinI = 1;
	ICAL = _ICAL;
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
	static int sampleI;
	static double lastFilteredV, filteredV;          //Filtered_ is the raw analog value minus the DC offset
	static double filteredI;
	static double offsetV = ADC_COUNTS >> 1;;                          //Low-pass filter output
	static double offsetI = ADC_COUNTS >> 1;;                          //Low-pass filter output               
	static double phaseShiftedV;                             //Holds the calibrated phase shifted voltage.
	static double sqV, sumV, sqI, sumI, instP, sumP;              //sq = squared, sum = Sum, inst = instantaneous
	static int startV;                                       //Instantaneous voltage at start of sample window.
	static boolean lastVCross, checkVCross;                  //Used to measure number of times threshold is crossed.
	int SupplyVoltage = vcc;
	static unsigned int crossCount = 0;                             //Used to measure number of times threshold is crossed.
	static unsigned int numberOfSamples = 0;                        //This is now incremented  

	//-------------------------------------------------------------------------------------------------------------------------
	// 1) Do not wait here, just return if we are on sin curve far from 0
	//-------------------------------------------------------------------------------------------------------------------------
	static bool measuring = false;
	if (!measuring)
	{
		startV = analogRead(inPinV);
		if ((startV < (ADC_COUNTS*0.55)) && (startV >(ADC_COUNTS*0.45))) {}
		else return; 
	}

	//-------------------------------------------------------------------------------------------------------------------------
	// 2) Main measurement 
	//------------------------------------------------------------------------------------------------------------------------- 

	if ((crossCount < crossings))
	{
		measuring = true;
		numberOfSamples++;                       //Count number of times looped.
		lastFilteredV = filteredV;               //Used for delay/phase compensation

		//-----------------------------------------------------------------------------
		// A) Read in raw voltage and current samples
		//-----------------------------------------------------------------------------
		sampleV = analogRead(inPinV);                 //Read in raw voltage signal
		sampleI = analogRead(inPinI);                 //Read in raw current signal

		//-----------------------------------------------------------------------------
		// B) Apply digital low pass filters to extract the 2.5 V or 1.65 V dc offset,
		//     then subtract this - signal is now centred on 0 counts.
		//-----------------------------------------------------------------------------
		offsetV = offsetV + ((sampleV - offsetV) / 1024);
		filteredV = sampleV - offsetV;
		offsetI = offsetI + ((sampleI - offsetI) / 1024);
		filteredI = sampleI - offsetI;

		//-----------------------------------------------------------------------------
		// C) Root-mean-square method voltage
		//-----------------------------------------------------------------------------  
		sqV = filteredV * filteredV;                 //1) square voltage values
		sumV += sqV;                                //2) sum

		//-----------------------------------------------------------------------------
		// D) Root-mean-square method current
		//-----------------------------------------------------------------------------   
		sqI = filteredI * filteredI;                //1) square current values
		sumI += sqI;                                //2) sum 

		//-----------------------------------------------------------------------------
		// E) Phase calibration
		//-----------------------------------------------------------------------------
		phaseShiftedV = lastFilteredV + PHASECAL * (filteredV - lastFilteredV);

		//-----------------------------------------------------------------------------
		// F) Instantaneous power calc
		//-----------------------------------------------------------------------------   
		instP = phaseShiftedV * filteredI;          //Instantaneous Power
		sumP += instP;                               //Sum  

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

	double V_RATIO = VCAL *((SupplyVoltage / 1000.0) / (ADC_COUNTS));
	Vrms = V_RATIO * sqrt(sumV / numberOfSamples);

	double I_RATIO = ICAL *((SupplyVoltage / 1000.0) / (ADC_COUNTS));
	Irms = I_RATIO * sqrt(sumI / numberOfSamples);

	//Calculation power values
	realPower = V_RATIO * I_RATIO * sumP / numberOfSamples;
	apparentPower = Vrms * Irms;
	powerFactor = realPower / apparentPower;

	//Reset accumulators
	sumV = 0;
	sumI = 0;
	sumP = 0;
	crossCount = 0;
	checkVCross = 0;
	lastVCross = 0;
	numberOfSamples = 0;
	measuring = false;   
}


