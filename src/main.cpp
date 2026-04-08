#include <Arduino.h>
#include <WiFi.h>
#include ".config.h"
#include "LIFX.h"

using namespace LIFX;

LIFX_UDP lifx;

LIFX_UDP::Device d1;
LIFX_UDP::Device d2;


void setup(){
	Serial.begin(115200);
	WiFi.begin(SSID,PASSWORD);
	Serial.println("Connecting");
		

	uint16_t tries = 0;
	while(!WiFi.isConnected()){
		Serial.print(".");
		delay(200);
		tries++;
		if(tries == 30) {
			Serial.println("\n Failed to connect to WiFi. Restarting.");
			esp_restart();
		}
	}

	lifx.Begin();
	delay(2000);
	Serial.println("Discovering");
	lifx.DiscoverDevicesBlocking();
	Serial.println("Finished discovering");

	d1 = lifx.GetDevice(0);
	d2 = lifx.GetDevice(1);
	Serial.printf("D1 valid %d\n", LIFX_UDP::Device::IsValid(d1));
	Serial.printf("D2 valid %d\n", LIFX_UDP::Device::IsValid(d2));
	delay(100);
	Serial.println("Sending get");

	delay(1000);

}

int delayTime = 2000;
int64_t lastTime = 0;
bool on = false;

void loop(){
	if(millis() - lastTime > delayTime){
		uint16_t val = on ? 0 : 0xffff;

		Serial.println("Sending: " + String(
			(int)lifx.SetLightPower({val,0},d1, true)
		));
		delay(500);
		lifx.GetResponse(LIFX::GetQuery::LightPower,[](LIFX_UDP::DeviceHeader& header, const uint8_t* data){
			Payloads::SetPower payload = LIFX::Payloads::GetResponse<LIFX::GetQuery::LightPower>(data);
			Serial.printf("\nGot resp for 116 (type): %" PRIu16 
				" should (h) be: %" PRIu16 
				" with level %" PRIu16  "\n",
				116, header.type, payload.level);
		},d1);

		lifx.SetLightPower({val,0},d2, true);
		// lifx.SetPower((struct LIFX::Payloads::SetPower){val}, true);
		on = !on;
		lastTime = millis();
	}
}



