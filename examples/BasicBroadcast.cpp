// Copyright (c) 2026 Lachlan Trevascus
// MIT License.
// Set your WiFi credentials before running

#include <Arduino.h>
#include <WiFi.h>
#include "LIFX.h"

#define PASSWORD "MyWifiPassword"
#define SSID "MyWifiSSID"

LIFX::LIFX_UDP lifx;

void setup(){
	Serial.begin(115200);
	WiFi.begin(SSID,PASSWORD);
	Serial.println("Connecting");
		

	uint16_t tries = 0;
	while(!WiFi.isConnected()){
		Serial.print(".");
		delay(200);
		tries++;
		if(tries == 40) {
			Serial.println("\n Failed to connect to WiFi. Restarting.");
			esp_restart();
		}
	}
    Serial.print("Connected at: ");
    Serial.println(WiFi.localIP());
    Serial.println("Starting LIFX");

    //Lifx requires that wifi is connected before it can begin
	lifx.Begin();
}

int delayTime = 2500;
unsigned long lastTime = 0;
bool on = false;

void loop(){
	if(millis() - lastTime >= delayTime){
        uint16_t level = on ? 0 : 0xffff;
        //Set the power based on the on toggle 
        Serial.printf("Sending: %u\n", (int)lifx.SetPower({level}, true));
		on = !on;
		lastTime = millis();
	}
}