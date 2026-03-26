// #include <Arduino.h>
// #include <WiFi.h>
// #include <AsyncUDP.h>
// #include ".config.h"



// typedef uint16_t LIFXFrameHeaderPart;

// struct LIFXFrameHeader{
// 	LIFXFrameHeader() = default;
// 	LIFXFrameHeader(uint16_t _size, uint16_t _protocol, bool _addressable, bool _tagged, bool _origin, uint32_t _source){
// 		size = _size;
// 		packedPart = _protocol & 0x0FFF;
// 		packedPart |= (_addressable ? 1 : 0) << 12;
// 		packedPart |= (_tagged ? 1 : 0) << 13;
// 		packedPart |= (_origin ? 1 : 0) << 14;
// 		source = _source;
// 	}


// 	uint16_t size;//msg size (including this field)
// 	LIFXFrameHeaderPart packedPart;//protocol, addressable, tagged, and origin
// 	uint32_t source;//source id, unique, set by client


// 	static uint16_t GetProtocol(LIFXFrameHeaderPart _packedPart){
// 		return _packedPart & 0x0FFF;
// 	}

// 	static bool GetAddressable(LIFXFrameHeaderPart _packedPart){
// 		return(_packedPart >> 12) & 1;
// 	}

// 	static uint8_t GetTagged(LIFXFrameHeaderPart _packedPart){
// 		return(_packedPart >> 13) & 1;
// 	}

// 	static uint8_t GetOrigin(LIFXFrameHeaderPart _packedPart){
// 		return(_packedPart >> 14) & 1;
// 	}

// };

// typedef uint64_t LIFXFrameAddressPart;

// struct LIFXFrameAddress{
// 	LIFXFrameAddress() = default;

// 	LIFXFrameAddress(uint64_t _target, bool _resRequired, bool _ackRequired, uint8_t _sequence){
// 		target = _target;
// 		packedPart = 0x00;
// 		//first 6 bytes (48 bits) are 0
// 		packedPart |= (uint64_t)(_resRequired ? 1 : 0) << 48;
// 		packedPart |= (uint64_t)(_ackRequired ? 1 : 0) << 49;
// 		packedPart |= (uint64_t)(_sequence) << 56;
// 	}

// 	uint64_t target;
// 	LIFXFrameAddressPart packedPart;

// 	//targets must be 8 size
// 	static void GetTargets(uint64_t _target, uint8_t* targets){
// 		for(int i = 0; i < 7; i++){
// 			//2^n - 1 bits
// 			//2^(8*i) - 1
// 			targets[i] = _target >> (i*8) &0xFF;
// 		}
// 	}

// 	static bool GetResRequired(LIFXFrameAddressPart _packedPart){
// 		uint8_t byte7 = _packedPart >> (6*8) &0xFF;
// 		//in the 0th bit
// 		return byte7 & 0x01;
// 	}

// 	static bool GetAckRequired(LIFXFrameAddressPart _packedPart){
// 		uint8_t byte7 = _packedPart >> (6*8) &0xFF;
// 		//in bit 1
// 		return byte7 & 0x02;
// 	}

// 	static uint8_t GetSequence(LIFXFrameAddressPart _packedPart){
// 		return  _packedPart >> (7*8) &0xFF;
// 	}

// };

// #pragma pack(push, 1)
// struct LIFXProtocolHeader{
// 	LIFXProtocolHeader() = default;
// 	LIFXProtocolHeader(uint16_t _type){
// 		type = _type;
// 	}
// 	uint64_t _reserved1;
// 	uint16_t type;
// 	uint16_t _reserved2;
// };
// #pragma pack(pop)

// #pragma pack(push, 1)
// struct LIFXMessage {
// 	LIFXMessage() = default;
// 	LIFXFrameHeader frameHeader;
// 	LIFXFrameAddress frameAddress;
// 	LIFXProtocolHeader protocolHeader;

// 	//thanks chat
// 	void Print() const {
// 		// ---- Frame Header ----
// 		Serial.printf("=== Frame Header ===\n");
// 		Serial.printf("Size: %u\n", frameHeader.size);
// 		Serial.printf("Protocol: %u\n",
// 			LIFXFrameHeader::GetProtocol(frameHeader.packedPart));
// 		Serial.printf("Addressable: %u\n",
// 			LIFXFrameHeader::GetAddressable(frameHeader.packedPart));
// 		Serial.printf("Tagged: %u\n",
// 			LIFXFrameHeader::GetTagged(frameHeader.packedPart));
// 		Serial.printf("Origin: %u\n",
// 			LIFXFrameHeader::GetOrigin(frameHeader.packedPart));
// 		Serial.printf("Source: %lu\n\n", frameHeader.source);

// 		// ---- Frame Address ----
// 		Serial.printf("=== Frame Address ===\n");
// 		Serial.printf("Target: %llu\n", frameAddress.target);
// 		Serial.printf("Res Required: %u\n",
// 			LIFXFrameAddress::GetResRequired(frameAddress.packedPart));
// 		Serial.printf("Ack Required: %u\n",
// 			LIFXFrameAddress::GetAckRequired(frameAddress.packedPart));
// 		Serial.printf("Sequence: %u\n",
// 			LIFXFrameAddress::GetSequence(frameAddress.packedPart));

// 		// Use provided GetTargets
// 		uint8_t targetBytes[8] = {0};
// 		LIFXFrameAddress::GetTargets(frameAddress.target, targetBytes);

// 		Serial.printf("Target Bytes: ");
// 		for (int i = 0; i < 7; i++) {
// 			Serial.printf("%u ", targetBytes[i]);
// 		}
// 		Serial.printf("\n\n");

// 		// ---- Protocol Header ----
// 		Serial.printf("=== Protocol Header ===\n");
// 		Serial.printf("Reserved1: %llu\n", protocolHeader._reserved1);
// 		Serial.printf("Type: %u\n", protocolHeader.type);
// 		Serial.printf("Reserved2: %u\n", protocolHeader._reserved2);

// 		Serial.printf("========================\n");
// 	}

	
// };
// #pragma pack(pop)

// constexpr int basePacketSize = sizeof(LIFXMessage);

// #pragma pack(push, 1)
// struct StateServicePacket{
// 	uint8_t service;
// 	uint32_t port;
// };
// #pragma pack(pop)


// #pragma pack(push, 1)
// struct StatePowerMsg {
// 	uint16_t _level;
// };
// #pragma pack(pop)

// struct SendMSG{
// 	uint8_t* data;
// 	size_t len;
// };

// //dont forget to free when done!!!!!!!
// uint8_t* GetStatePowerMsg(uint16_t level, uint64_t target){
// 	const int totalPacketSize = basePacketSize + sizeof(StatePowerMsg);
// 	LIFXFrameHeader fh(totalPacketSize,1024,true,false,0,2);
// 	LIFXFrameAddress fa(target,true,false,67);
// 	LIFXProtocolHeader ph(21);//Set power packet
// 	LIFXMessage getService {fh, fa, ph};
// 	uint8_t* data = new uint8_t[totalPacketSize];
// 	memcpy(data, &getService, basePacketSize);
	
// 	StatePowerMsg statePower {level};
// 	memcpy(data + basePacketSize, &statePower, sizeof(StatePowerMsg));
// 	return data;
// }

// uint8_t* GetLightPowerMsg(uint16_t level, uint32_t duration, uint64_t target){
// 	static uint8_t seq = 0;
// 	seq++;
// 	#pragma pack(push, 1)
// 	struct {
// 		uint16_t _level;
// 		uint32_t _duration;
// 	} payload {level, duration};
// 	#pragma pack(pop)

// 	const int totalPacketSize = basePacketSize + sizeof(payload);
// 	LIFXFrameHeader fh(totalPacketSize,1024,true,false,0,seq);
// 	LIFXFrameAddress fa(target,true,false,0);
// 	LIFXProtocolHeader ph(117);//SetLightPower 
// 	LIFXMessage getService {fh, fa, ph};
// 	uint8_t* data = new uint8_t[totalPacketSize];
// 	memcpy(data, &getService, basePacketSize);
	
// 	memcpy(data + basePacketSize, &payload, sizeof(payload));
// 	return data;
// }

// SendMSG GetSetColourMsg(uint16_t hue, uint16_t saturation, uint16_t brightness, uint16_t kelvin, uint32_t duration, uint64_t target){
// 	#pragma pack(push, 1)
// 	struct {
// 		uint8_t res;
// 		uint16_t _hue; 
// 		uint16_t _saturation; 
// 		uint16_t _brightness; 
// 		uint16_t _kelvin; 
// 		uint32_t _duration;
// 	} payload {0,hue, saturation, brightness, kelvin, duration};
// 	#pragma pack(pop)

// 	const int totalPacketSize = basePacketSize + sizeof(payload);
// 	LIFXFrameHeader fh(totalPacketSize,1024,true,false,0,2);
// 	LIFXFrameAddress fa(target,false,false,1);
// 	LIFXProtocolHeader ph(102);//SetColor
// 	LIFXMessage getService {fh, fa, ph};
// 	uint8_t* data = new uint8_t[totalPacketSize];
// 	memcpy(data, &getService, basePacketSize);
	
// 	memcpy(data + basePacketSize, &payload, sizeof(payload));
// 	return {data, totalPacketSize};
// }


// #include <lwip/sockets.h>
// #include <lwip/netdb.h>

// constexpr uint16_t port = 56700;
// #define UDP_PORT 56700
// #define BUFFER_SIZE 256
// #define SOCKET_TIMEOUT_MS 2000

// int sock;
// struct sockaddr_in dest;
// IPAddress broadcast;

// IPAddress getBroadcast(IPAddress ip, IPAddress subnet) {
//   IPAddress broadcast;
//   for (int i = 0; i < 4; i++) {
//     broadcast[i] = ip[i] | (~subnet[i]);
//   }
//   return broadcast;
// }

// void sendBroadcast(const uint8_t* data, size_t len) {

// 	int err = sendto(sock, data, len, 0, (struct sockaddr*)&dest, sizeof(dest));

// 	// if (err < 0) {
// 	// 	Serial.println("Send failed");
// 	// } else {
// 	// 	Serial.printf("Sent %d bytes to %s\n", err, broadcast.toString().c_str());
// 	// }
// }


// void receivePacket() {
// 	uint8_t buffer[BUFFER_SIZE];

// 	struct sockaddr_in sourceAddr;
// 	socklen_t addrLen = sizeof(sourceAddr);
	
// 	int len = recvfrom(sock, buffer, BUFFER_SIZE, 0,
// 						(struct sockaddr*)&sourceAddr, &addrLen);

// 	if (len < 0) {
// 		//Serial.println("No data (timeout)");
// 		return;
// 	}

// 	char ipStr[16];
// 	inet_ntoa_r(sourceAddr.sin_addr, ipStr, sizeof(ipStr));
	
// 	Serial.printf("Received %d bytes from %s:%d\n",len,ipStr,ntohs(sourceAddr.sin_port));

// }

// void setup() {
// 	Serial.begin(115200);
// 	WiFi.begin(SSID,PASSWORD);
// 	Serial.println("Connecting");
	
// 	uint16_t tries = 0;
// 	while(!WiFi.isConnected()){
// 		Serial.print(".");
// 		delay(200);
// 		tries++;
// 		if(tries == 30) {
// 			Serial.println("\n Failed to connect to WiFi. Restarting.");
// 			esp_restart();
// 		}
// 	}

// 	//Serial.println("Connected to wifi");

// 	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
// 	if (sock < 0) {
// 		Serial.println("Socket creation failed");
// 		return;
// 	}

// 	int broadcastEnable = 1;
// 	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));


// 	struct timeval timeout;
// 	timeout.tv_sec = SOCKET_TIMEOUT_MS / 1000;
// 	timeout.tv_usec = (SOCKET_TIMEOUT_MS % 1000) * 1000;
// 	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));


// 	struct sockaddr_in addr;
// 	addr.sin_family = AF_INET;
// 	addr.sin_port = htons(UDP_PORT);
// 	addr.sin_addr.s_addr = htonl(INADDR_ANY);

// 	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
// 		Serial.println("Bind failed");
// 		return;
// 	}

// 	IPAddress ip = WiFi.localIP();
// 	IPAddress subnet = WiFi.subnetMask();
// 	broadcast = getBroadcast(ip, subnet);

// 	dest.sin_family = AF_INET;
// 	dest.sin_port = htons(UDP_PORT);
// 	dest.sin_addr.s_addr = inet_addr(broadcast.toString().c_str());



// }

// //https://lan.developer.lifx.com/docs/packet-contents

// uint8_t buffer[255];
// int delayTime = 5000;
// int64_t lastTime = -delayTime;
// bool on = false;
// void loop() {
// 	if(millis() - lastTime > delayTime){
// 		// float temp = temperatureRead();
// 		// Serial.print("CPU Temp: ");
// 		// Serial.print(temp);
// 		// Serial.println(" C");
// 		// uint8_t* msg = GetLightPowerMsg(on ? 0 : 32767, 1000, 0);
// 		// sendBroadcast(msg, basePacketSize + 6);
// 		// receivePacket();
// 		// delete[] msg;

// 		SendMSG msg = GetSetColourMsg(47331, 62000,on ? 0 : 65535,6500,10000,0);
// 		// SendMSG msg = GetSetColourMsg(47331, millis()%65535,on ? 0 : 1000,2700,0,0);
// 		sendBroadcast(msg.data, msg.len);
// 		//receivePacket();
// 		delete[] msg.data;
// 		lastTime = millis();
// 		on = !on;
// 	}
// }




#include <Arduino.h>
#include <WiFi.h>
#include ".config.h"
#include "LIFX.h"

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
		if(tries == 30) {
			Serial.println("\n Failed to connect to WiFi. Restarting.");
			esp_restart();
		}
	}

	lifx.Begin();
}

int delayTime = 300;
int64_t lastTime = -delayTime;
bool on = false;

void loop(){
	// if(millis() - lastTime > delayTime){
	// 	lifx.SetPower((struct LIFX::Payloads::SetPower){on ? 0 : 65535}, true);
	// 	on = !on;
	// 	lastTime = millis();
	// }
}



