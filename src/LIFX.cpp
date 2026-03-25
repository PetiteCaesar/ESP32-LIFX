#include "LIFX.h"

#include <string.h>
#include "LIFXConfig.h"

typedef uint16_t LIFXFrameHeaderPart;
typedef uint64_t LIFXFrameAddressPart;


#pragma pack(push, 1)
struct LIFXFrameHeader{
	LIFXFrameHeader() = default;
	LIFXFrameHeader(uint16_t _size, uint16_t _protocol, bool _addressable, bool _tagged, bool _origin, uint32_t _source){
		size = _size;
		packedPart = _protocol & 0x0FFF;
		packedPart |= (_addressable ? 1 : 0) << 12;
		packedPart |= (_tagged ? 1 : 0) << 13;
		packedPart |= (_origin ? 1 : 0) << 14;
		source = _source;
	}


	uint16_t size;//msg size (including this field)
	LIFXFrameHeaderPart packedPart;//protocol, addressable, tagged, and origin
	uint32_t source;//source id, unique, set by client


	static uint16_t GetProtocol(LIFXFrameHeaderPart _packedPart){
		return _packedPart & 0x0FFF;
	}

	static bool GetAddressable(LIFXFrameHeaderPart _packedPart){
		return(_packedPart >> 12) & 1;
	}

	static uint8_t GetTagged(LIFXFrameHeaderPart _packedPart){
		return(_packedPart >> 13) & 1;
	}

	static uint8_t GetOrigin(LIFXFrameHeaderPart _packedPart){
		return(_packedPart >> 14) & 1;
	}

};
#pragma pack(pop)

#pragma pack(push, 1)
struct LIFXFrameAddress{
	LIFXFrameAddress() = default;

	LIFXFrameAddress(uint64_t _target, bool _resRequired, bool _ackRequired, uint8_t _sequence){
		target = _target;
		packedPart = 0x00;
		//first 6 bytes (48 bits) are 0
		packedPart |= (uint64_t)(_resRequired ? 1 : 0) << 48;
		packedPart |= (uint64_t)(_ackRequired ? 1 : 0) << 49;
		packedPart |= (uint64_t)(_sequence) << 56;
	}

	uint64_t target;
	LIFXFrameAddressPart packedPart;

	//targets must be 8 size
	static void GetTargets(uint64_t _target, uint8_t* targets){
		for(int i = 0; i < 7; i++){
			//2^n - 1 bits
			//2^(8*i) - 1
			targets[i] = _target >> (i*8) &0xFF;
		}
	}

	static bool GetResRequired(LIFXFrameAddressPart _packedPart){
		uint8_t byte7 = _packedPart >> (6*8) &0xFF;
		//in the 0th bit
		return byte7 & 0x01;
	}

	static bool GetAckRequired(LIFXFrameAddressPart _packedPart){
		uint8_t byte7 = _packedPart >> (6*8) &0xFF;
		//in bit 1
		return byte7 & 0x02;
	}

	static uint8_t GetSequence(LIFXFrameAddressPart _packedPart){
		return  _packedPart >> (7*8) &0xFF;
	}

};
#pragma pack(pop)

#pragma pack(push, 1)
struct LIFXProtocolHeader{
	LIFXProtocolHeader() = default;
	LIFXProtocolHeader(uint16_t _type){
		type = _type;
	}
	uint64_t _reserved1;
	uint16_t type;
	uint16_t _reserved2;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct LIFXFullHeader {
	LIFXMessage() = default;
	LIFXFrameHeader frameHeader;
	LIFXFrameAddress frameAddress;
	LIFXProtocolHeader protocolHeader;

	//thanks chat
	void Print() const {
		// ---- Frame Header ----
		Serial.printf("=== Frame Header ===\n");
		Serial.printf("Size: %u\n", frameHeader.size);
		Serial.printf("Protocol: %u\n",
			LIFXFrameHeader::GetProtocol(frameHeader.packedPart));
		Serial.printf("Addressable: %u\n",
			LIFXFrameHeader::GetAddressable(frameHeader.packedPart));
		Serial.printf("Tagged: %u\n",
			LIFXFrameHeader::GetTagged(frameHeader.packedPart));
		Serial.printf("Origin: %u\n",
			LIFXFrameHeader::GetOrigin(frameHeader.packedPart));
		Serial.printf("Source: %lu\n\n", frameHeader.source);

		// ---- Frame Address ----
		Serial.printf("=== Frame Address ===\n");
		Serial.printf("Target: %llu\n", frameAddress.target);
		Serial.printf("Res Required: %u\n",
			LIFXFrameAddress::GetResRequired(frameAddress.packedPart));
		Serial.printf("Ack Required: %u\n",
			LIFXFrameAddress::GetAckRequired(frameAddress.packedPart));
		Serial.printf("Sequence: %u\n",
			LIFXFrameAddress::GetSequence(frameAddress.packedPart));

		// Use provided GetTargets
		uint8_t targetBytes[8] = {0};
		LIFXFrameAddress::GetTargets(frameAddress.target, targetBytes);

		Serial.printf("Target Bytes: ");
		for (int i = 0; i < 7; i++) {
			Serial.printf("%u ", targetBytes[i]);
		}
		Serial.printf("\n\n");

		// ---- Protocol Header ----
		Serial.printf("=== Protocol Header ===\n");
		Serial.printf("Reserved1: %llu\n", protocolHeader._reserved1);
		Serial.printf("Type: %u\n", protocolHeader.type);
		Serial.printf("Reserved2: %u\n", protocolHeader._reserved2);

		Serial.printf("========================\n");
	}

	
};
#pragma pack(pop)

#define HEADER_SIZE sizeof(LIFXFullHeader)


namespace LIFX{


LIFX_UDP::SET_RESP LIFX_UDP::_SetPower(Payloads::SetPower payload, const Device *dev, bool requireAck){
    //temp
    return (LIFX_UDP::SET_RESP)1;
    //since SetPower has just a uint16_t
    constexpr int totalPacketSize = HEADER_SIZE + 2;
    
	LIFXFullHeader getService {
        LIFXFrameHeader(totalPacketSize,PROTOCOL,ADDRESSABLE,TAGGED,ORIGIN,m_sourceId), 
        LIFXFrameAddress(Device::GetTarget(dev),false,requireAck,67), 
        LIFXProtocolHeader(21)
    };
	uint8_t* data = new uint8_t[totalPacketSize];
	memcpy(data, &getService, HEADER_SIZE);
	memcpy(data + HEADER_SIZE, &payload.level, 2);
}




}

