#include "LIFX.h"
#include <string.h>
#include <lwip/netdb.h>
#include <lwip/inet.h>


#define HEADER_SIZE 36
//Could use bit fields, but this is compiler agnostic
struct LIFXHeader{
	//Frame header
	uint16_t size;
	uint16_t protocol;//protocol + addressable + tagged + origin
	uint32_t source;

	//Frame Address
	uint8_t target[8];
	uint8_t resAckReserved;//res + ack + reserved (6bit)
	uint8_t sequence;

	//Protocol Header
    uint16_t type;

	

	LIFXHeader() = default;
	LIFXHeader(uint16_t _packetType, uint32_t _payloadSize, bool _requireAck, uint32_t _sourceId, uint8_t _sequence, uint64_t _target, bool _tagged){
		//Frame header
		size = HEADER_SIZE + _payloadSize;
		protocol = 0;
		protocol |= PROTOCOL & 0x0FFF;
		protocol |= (ADDRESSABLE << 12);
		protocol |= (_tagged ? 1 : 0) << 13;
		protocol |= (ORIGIN & 0x03) << 14;
		source = _sourceId;

		//Frame address
		for(int i = 0; i < 8; i++) target[i] = (_target >> (8 * i)) & 0xFF;
		resAckReserved = 0;
		resAckReserved |= (_requireAck ? 1 : 0) << 1;
		sequence = _sequence;

		//Protocol Header
		type = _packetType;
	}
	
	//free after use
	uint8_t* Serialise(uint8_t* buf){
		memset(buf,0,HEADER_SIZE);
		//frame header
		buf[0] = size & 0xff;
		buf[1] = (size>>8) & 0xff;
		buf[2] = protocol & 0xff;
		buf[3] = (protocol >> 8) & 0xff;//rest of protocol + addressable + tagged + origin
		for(int i = 0; i < 4; i++) buf[4+i] = (source >> i*8) & 0xff;

		//frame address
		for(int i = 0; i < 8; i++) buf[8+i] = target[i];
		//next 6 reserved
		buf[22] = resAckReserved;
		buf[23] = sequence;
		//next 8 reserved
		buf[32] = type & 0xff;
		buf[33] = (type>>8) & 0xff;
		return buf;
	}

	void Deserialise(const uint8_t* buf){
		size = buf[0] | (buf[1] << 8);
		protocol = buf[2] | (buf[3] << 8);
		source = buf[4] | (buf[5] << 8) | (buf[6] << 16) | (buf[7] << 24);
		for (int i = 0; i < 8; i++) target[i] = buf[8+i];
		resAckReserved = buf[22];
		sequence = buf[23];
		type = buf[32] | (buf[33] << 8);
	}

	static LIFXHeader ParseHeader(const uint8_t* buf) {
		LIFXHeader header;
		header.Deserialise(buf);
		return header;
	} 

	inline uint64_t GetTarget() const{
		uint64_t t = 0;
		for(int i = 0; i < 8; i++) t |= (uint64_t)target[i] << i*8;
		return t;
	}

};


#pragma pack(push, 1)
typedef struct {
  /* frame */
  uint16_t size;
  uint16_t protocol:12;
  uint8_t  addressable:1;
  uint8_t  tagged:1;
  uint8_t  origin:2;
  uint32_t source;
  /* frame address */
  uint8_t  target[8];
  uint8_t  reserved[6];
  uint8_t  res_required:1;
  uint8_t  ack_required:1;
  uint8_t  :6;
  uint8_t  sequence;
  /* protocol header */
  uint64_t :64;
  uint16_t type;
  uint16_t :16;
  /* variable length payload follows */
} lx_protocol_header_t;
#pragma pack(pop)

namespace LIFX{


	LIFX_UDP::DeviceHeader GetHeaderView(const LIFXHeader& header){
		return {
			header.size,
			header.source,
			header.GetTarget(),
			header.sequence,
			header.type
		};
	}


	uint32_t LIFX::LIFX_UDP::m_sourceId = 0;
    const uint16_t LIFX_UDP::GetHeaderSize() {
        return HEADER_SIZE;
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

		xTaskCreate(UDPPollTask, "UDPPollTask",10000, this, 1,&m_pollTask);

		return UDP_SETUP_RESP::SUCCESS;
    }



	uint8_t *LIFX_UDP::GetSendHeader(uint16_t packetType, uint32_t payloadSize, bool requireAck, uint32_t sourceId, uint8_t sequence, uint64_t target, bool tagged) {
		LIFXHeader header(packetType, payloadSize, requireAck, sourceId, sequence, target,tagged);
		uint8_t* buf = new uint8_t[HEADER_SIZE + payloadSize];
		if(!buf) return nullptr;
        return header.Serialise(buf);
    }

    bool LIFX_UDP::SendPacket(const uint8_t *data, size_t len, sockaddr_in &dest)
    {
        int err = sendto(m_sock, data, len, 0, (sockaddr*)&dest, sizeof(dest));
		// printf("Sendto err: %d\n", err);
		return err >= 0;
    }

    bool LIFX_UDP::SendMessage(const uint8_t *data, size_t len, const Device *dev) {
		sockaddr_in dest;
		dest.sin_family = AF_INET;
		if(dev == nullptr || !Device::IsValid(*dev)){
			dest.sin_port = htons(UDP_PORT);
			dest.sin_addr.s_addr = htonl(INADDR_BROADCAST);
		}  else{
			dest.sin_port = htons(dev->port);
			dest.sin_addr.s_addr = htonl(dev->ipAddr);
		}
		return SendPacket(data, len, dest);
    }


	bool LIFX_UDP::DiscoverDevices() {
		if(m_deviceManager.discovering) return false;
		//GetService header
		uint8_t* data = GetSendHeader(2, 0, false, DISCOVER_SOURCE_ID, 0, 0, 1);
		bool err = SendMessage(data, HEADER_SIZE, 0);
		delete[] data;
		if(err < 0) return false;
		m_deviceManager.discovering = true;
		m_deviceManager.lastAddTime = esp_timer_get_time();
		m_deviceManager.discoveredDevices = 0;
        return true;
    }

    bool LIFX_UDP::DiscoverDevicesBlocking() {
		bool res = DiscoverDevices();
		//loop while waiting like 10ms
		while(StillDiscovering()) vTaskDelay(10/portTICK_RATE_MS);
        return res;
    }

    bool LIFX_UDP::StillDiscovering() {
        return m_deviceManager.discovering;
    }

    uint32_t LIFX_UDP::GetDiscoveredDevices() {
        return m_deviceManager.discoveredDevices;
    }

    Device LIFX_UDP::GetDevice(uint32_t id) {
		if(id >= GetDiscoveredDevices()){
			return Device {0, 0, 0, 0};
		}
        return m_deviceManager.devices[id];
    }

    void LIFX_UDP::UDPPollTask(void *data) {
		LIFX_UDP& lu = *(LIFX_UDP*)data;
		DeviceManager& dm = lu.m_deviceManager;
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(lu.m_sock, &fds);
		timeval timeout = {0,0};
		for(;;){
			fd_set readfds = fds;

			select(lu.m_sock + 1, &readfds, NULL, NULL, &timeout);

			if(dm.discovering && ((esp_timer_get_time() - dm.lastAddTime) > SOCKET_TIMEOUT_MS*1000 || dm.discoveredDevices == MAX_DEVICES)) {
				printf("Discovering stopped due to timeout or maxDevs reached\n");
				dm.discovering = false;
			}

			if (FD_ISSET(lu.m_sock, &readfds)) {
				uint8_t buffer[BUFFER_SIZE];
				sockaddr_in sourceAddr;
				socklen_t addrLen = sizeof(sourceAddr);
				int res = recvfrom(lu.m_sock, buffer, BUFFER_SIZE, 0, (sockaddr*)&sourceAddr, &addrLen);
				// auto temp = ParseHeader(buffer);
				LIFXHeader fullHeader = LIFXHeader::ParseHeader(buffer);
				
				DeviceHeader header = GetHeaderView(fullHeader);
				

				printf("Recv header seq %d, header srcId %d, target: %" PRIu64 "\n", header.sequence,header.source,header.target);

				if(dm.discovering && header.source == DISCOVER_SOURCE_ID){
			
					Device newDevice;
					newDevice.service = buffer[HEADER_SIZE];
					//seems to send the same device multiple times 
					//with different services
					//not sure what its for
					if(newDevice.service == 1){
						newDevice.id = header.target;
						memcpy(&newDevice.port, buffer + HEADER_SIZE + 1, sizeof(uint32_t));
						newDevice.ipAddr = ntohl(sourceAddr.sin_addr.s_addr);
						printf("id: %" PRIu64 ", service %" PRIu8 ", ipAddr: %" PRIu32 " port: %d\n",newDevice.id, newDevice.service, newDevice.ipAddr, newDevice.port);
						printf("Adding device at %d\n", dm.discoveredDevices);
						dm.devices[dm.discoveredDevices] = newDevice;
						dm.discoveredDevices++;
						dm.lastAddTime = esp_timer_get_time();
					}
					

				} else{
					// temp.Print();
				} 

			}
			vTaskDelay(3);
		}
    }


}

