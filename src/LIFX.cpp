#include "LIFX.h"
#include <string.h>
#include <lwip/netdb.h>
#include <lwip/inet.h>

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

	uint8_t GetSequence(){return GetSequence(packedPart);}
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
	LIFXFullHeader() = default;
	LIFXFrameHeader frameHeader;
	LIFXFrameAddress frameAddress;
	LIFXProtocolHeader protocolHeader;

	//thanks chat
	// void Print() const {
	// 	// ---- Frame Header ----
	// 	Serial.printf("=== Frame Header ===\n");
	// 	Serial.printf("Size: %u\n", frameHeader.size);
	// 	Serial.printf("Protocol: %u\n",
	// 		LIFXFrameHeader::GetProtocol(frameHeader.packedPart));
	// 	Serial.printf("Addressable: %u\n",
	// 		LIFXFrameHeader::GetAddressable(frameHeader.packedPart));
	// 	Serial.printf("Tagged: %u\n",
	// 		LIFXFrameHeader::GetTagged(frameHeader.packedPart));
	// 	Serial.printf("Origin: %u\n",
	// 		LIFXFrameHeader::GetOrigin(frameHeader.packedPart));
	// 	Serial.printf("Source: %lu\n\n", frameHeader.source);

	// 	// ---- Frame Address ----
	// 	Serial.printf("=== Frame Address ===\n");
	// 	Serial.printf("Target: %llu\n", frameAddress.target);
	// 	Serial.printf("Res Required: %u\n",
	// 		LIFXFrameAddress::GetResRequired(frameAddress.packedPart));
	// 	Serial.printf("Ack Required: %u\n",
	// 		LIFXFrameAddress::GetAckRequired(frameAddress.packedPart));
	// 	Serial.printf("Sequence: %u\n",
	// 		LIFXFrameAddress::GetSequence(frameAddress.packedPart));

	// 	// Use provided GetTargets
	// 	uint8_t targetBytes[8] = {0};
	// 	LIFXFrameAddress::GetTargets(frameAddress.target, targetBytes);

	// 	Serial.printf("Target Bytes: ");
	// 	for (int i = 0; i < 7; i++) {
	// 		Serial.printf("%u ", targetBytes[i]);
	// 	}
	// 	Serial.printf("\n\n");

	// 	// ---- Protocol Header ----
	// 	Serial.printf("=== Protocol Header ===\n");
	// 	Serial.printf("Reserved1: %llu\n", protocolHeader._reserved1);
	// 	Serial.printf("Type: %u\n", protocolHeader.type);
	// 	Serial.printf("Reserved2: %u\n", protocolHeader._reserved2);

	// 	Serial.printf("========================\n");
	// }

	
};
#pragma pack(pop)

#define HEADER_SIZE sizeof(LIFXFullHeader)


namespace LIFX{
	uint32_t LIFX::LIFX_UDP::m_sourceId = 0;

	inline LIFXFullHeader ParseHeader(uint8_t* data){
		LIFXFullHeader header;
		memcpy(&header, data, HEADER_SIZE);
		return header;
	}

    LIFX_UDP::UDP_SETUP_RESP LIFX_UDP::Begin() {
		m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (m_sock < 0) {
			return UDP_SETUP_RESP::SOCKET_CREATION_FAIL;
		}

		int broadcastEnable = 1;
		setsockopt(m_sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));


		timeval timeout;
		timeout.tv_sec = SOCKET_TIMEOUT_MS / 1000;
		timeout.tv_usec = (SOCKET_TIMEOUT_MS % 1000) * 1000;
		setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));


		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(UDP_PORT);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(m_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			return UDP_SETUP_RESP::BIND_FAIL;
		}

		return UDP_SETUP_RESP::SUCCESS;
    }

    int LIFX_UDP::DiscoverDevices() {
		//GetService header
		uint8_t* data = GetSendHeader(2,0,false,67,0);
		bool res = SendMessage(data, HEADER_SIZE, 0);
		delete[] data;

		// uint8_t buffer[BUFFER_SIZE];
		// int len = ReceivePacket(buffer, BUFFER_SIZE);
		// if(len < 0) return -1;
        return 0;
    }

    int LIFX_UDP::ReceivePacket(uint8_t* buffer, uint32_t size) {
		sockaddr_in sourceAddr;
		socklen_t addrLen = sizeof(sourceAddr);
		return recvfrom(m_sock, buffer, size, 0, (sockaddr*)&sourceAddr, &addrLen);
    }

    LIFX_UDP::UDP_RESP LIFX_UDP::WaitForAck(uint8_t seq) {
		uint8_t buffer[BUFFER_SIZE];
		int len = ReceivePacket(buffer, BUFFER_SIZE);
		if(len < 0) {
			printf("len < 0\n");
			return UDP_RESP::ACK_TIMED_OUT;
		}

		LIFXFullHeader header = ParseHeader(buffer);
		
		if(seq != header.frameAddress.GetSequence()) {
			printf("seq %d != header seq %d\n", seq, header.frameAddress.GetSequence());
			return UDP_RESP::ACK_WRONG_MSG;
		}

        return UDP_RESP::SUCCESS;
    }

    bool LIFX_UDP::SendPacket(const uint8_t *data, size_t len, sockaddr_in& dest) {
        int err = sendto(m_sock, data, len, 0, (sockaddr*)&dest, sizeof(dest));
		return err >= 0;
    }


    bool LIFX_UDP::SendMessage(const uint8_t *data, size_t len, const Device *dev) {
		sockaddr_in dest;
		dest.sin_family = AF_INET;
		if(dev == nullptr){
			dest.sin_port = htons(UDP_PORT);
			dest.sin_addr.s_addr = htonl(INADDR_BROADCAST);
		}  else{
			dest.sin_port = htons(dev->port);
			dest.sin_addr.s_addr = htonl(dev->ipAddr);
		}
		return SendPacket(data, len, dest);
    }

    uint8_t *LIFX_UDP::GetSendHeader(uint16_t packetType, uint32_t payloadSize, bool requireAck, uint8_t sequence, uint64_t target) {
		LIFXFullHeader getService{
			LIFXFrameHeader(HEADER_SIZE + payloadSize, PROTOCOL, ADDRESSABLE, TAGGED, ORIGIN, m_sourceId),
			LIFXFrameAddress(target, false, requireAck, sequence),
			LIFXProtocolHeader(packetType)
		};
        uint8_t *data = new uint8_t[HEADER_SIZE + payloadSize];
		memcpy(data, &getService, HEADER_SIZE);
        return data;
    }

    LIFX_UDP::UDP_RESP LIFX_UDP::_SetPower(Payloads::SetPower payload, const Device *dev, bool requireAck) {
        // since SetPower has just a uint16_t
        constexpr int totalPacketSize = HEADER_SIZE + 2;

		//Set power packet is #21
		uint8_t* data = GetSendHeader(21,2,requireAck,++m_sequence,Device::GetTarget(dev));
        memcpy(data + HEADER_SIZE, &payload.level, 2);

		bool res = SendMessage(data, totalPacketSize, dev);
		delete[] data;
		if(!res) return UDP_RESP::SENT_FAILED;
		if(requireAck) {
			UDP_RESP res = WaitForAck(m_sequence);
			if(res != UDP_RESP::SUCCESS){
				return UDP_RESP::ACK_TIMED_OUT;
			} 
		}
		return UDP_RESP::SUCCESS;
    }
}

