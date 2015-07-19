/*
  // March 2014 - TMRh20 - Updated along with High Speed RF24 Library fork
  // Parts derived from examples by J. Coliz <maniacbug@ymail.com>
  */
/**
 * Example for efficient call-response using ack-payloads
 *
 * This example continues to make use of all the normal functionality of the radios including
 * the auto-ack and auto-retry features, but allows ack-payloads to be written optionlly as well.
 * This allows very fast call-response communication, with the responding radio never having to
 * switch out of Primary Receiver mode to send back a payload, but having the option to if wanting
 * to initiate communication instead of respond to a commmunication.
 */



#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 
RF24 radio(7, 8);

// Topology
const uint64_t pipes[2] = { 0x24CDABCD71LL, 0x244d52687CLL };              // Radio pipe addresses for the 2 nodes to communicate.


//structure holding data for scheduling relays
struct Payload
{
	int AirTemp; //must divide by 10
	int AirHum; //must divide by 10
	int AirHumidex; //must divide by 10
	int SoilTemp; //must divide by 10
	int SoilHum; //must divide by 10
	int Light; //must divide by 10
	unsigned int RainTips;
	unsigned long Uptime;
	int FreeRam;
	int Vcc;

	void print(){
		Serial.print(F("Air Temperature: "));
		Serial.print(AirTemp / 10.0, 1);
		Serial.println(F("C"));
		Serial.print(F("Air Humidity: "));
		Serial.print(AirHum / 10.0, 1);
		Serial.println(F("%RH"));
		Serial.print(F("Air Humidex: "));
		Serial.print(AirHumidex / 10.0, 1);
		Serial.println(F("C"));
		Serial.print(F("Soil Temperature: "));
		Serial.print(SoilTemp / 10.0, 1);
		Serial.println(F("C"));
		Serial.print(F("Soil Humidity: "));
		Serial.print(SoilHum / 10.0, 1);
		Serial.println(F("%RH"));
		Serial.print(F("Light: "));
		Serial.print(Light / 10.0, 1);
		Serial.println(F("%"));
		Serial.print(F("Rain Tips: "));
		Serial.print(RainTips);
		Serial.println();
		Serial.print(F("Uptime: "));
		Serial.print(Uptime);
		Serial.println(F("s"));
		Serial.print(F("Free RAM: "));
		Serial.print(FreeRam);
		Serial.println(F("B"));
		Serial.print(F("Vcc: "));
		Serial.print(Vcc);
		Serial.println(F("mV"));
		Serial.println();
	}

}; typedef struct Payload Payload;

Payload p;




void setup(){

	Serial.begin(57600);
	printf_begin();
	printf("\n\rRF24/examples/GettingStarted/\n\r");

	p.AirTemp = 256;
	p.AirHum = 615;
	p.AirHumidex = 263;
	p.SoilTemp = 221;
	p.SoilHum = 802;
	p.Light = 991;
	p.RainTips = 15;
	p.Uptime = 5453411635;
	p.FreeRam = 2111;
	p.Vcc = 4990;

	// Setup and configure rf radio

	radio.begin();
	radio.setAutoAck(1);                    // Ensure autoACK is enabled
	//radio.setChannel(24);
	radio.setCRCLength(RF24_CRC_8);
	radio.setDataRate(RF24_250KBPS);
	radio.setPALevel(RF24_PA_MAX);
	radio.enableAckPayload();               // Allow optional ack payloads
	radio.setRetries(1, 15);                 // Smallest time between retries, max no. of retries
	radio.setPayloadSize(22);                // Here we are sending 1-byte payloads to test the call-response speed

	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(1, pipes[1]);

	radio.startListening();                 // Start listening
	radio.printDetails();                   // Dump the configuration of the rf unit for debugging
}

void loop(void) {
	unsigned long a = micros();
	radio.stopListening();                                  // First, stop listening so we can talk.

	printf("Now sending floats as payload. ");
	byte gotByte;

	if (!radio.write(&p, sizeof(p))){
		printf("failed.\n\r");
	}
	else{

		if (!radio.available()){
			printf("Blank Payload Received\n\r");
		}
	}
	unsigned long b = micros();
	printf("roundtrip %d us\n\r", int((b - a)));

	// Try again later
	delay(2000);


}
