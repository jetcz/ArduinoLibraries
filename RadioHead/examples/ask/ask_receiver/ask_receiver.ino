// ask_receiver.pde
// -*- mode: C++ -*-
// Simple example of how to use RadioHead to receive messages
// with a simple ASK transmitter in a very simple way.
// Implements a simplex (one-way) receiver with an Rx-B1 module

#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile
const int RESET_ETH_SHIELD_PIN = 14;
const int RADIO_RX_PIN = 17;
const int RADIO_CTRL_PIN = 0;
RH_ASK driver(2000, RADIO_RX_PIN, 0);

void setup()
{
    Serial.begin(9600);	// Debugging only
    if (!driver.init())
         Serial.println("init failed");
         resetEthShield(RESET_ETH_SHIELD_PIN);
}

void loop()
{
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);

    if (driver.recv(buf, &buflen)) // Non-blocking
    {
	int i;

	// Message with a good checksum received, dump it.
	driver.printBuffer("Got:", buf, buflen);
    }
}
void resetEthShield(int pin) {
	pinMode(pin, OUTPUT);
	Serial.println(F("Reseting Ethernet Shield"));
	digitalWrite(pin, LOW);
	delay(1);
	digitalWrite(pin, HIGH);
	delay(250);
}
