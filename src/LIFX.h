#pragma once
#include <cstdint>
#include "Payloads.h"
#include "LIFXConfig.h"
#include <lwip/sockets.h>
#include <functional>
#include <type_traits>

namespace LIFX{
    struct Device{
        uint64_t id;
        uint8_t service;
        uint32_t port;
        uint32_t ipAddr;

        static bool IsValid(const Device& dev){
            return dev.id != 0
                && dev.service != 0
                && dev.port != 0
                && dev.ipAddr != 0;
        }
        private:
        friend class LIFX_UDP;
        static uint64_t GetTarget(const Device* dev){
            return (dev == nullptr) ? 0 : dev->id;
        } 
    };

    class LIFX_UDP{
    public:

        //public facing header
        struct DeviceHeader{
            uint16_t size;
            uint32_t source;
            uint64_t target;
            uint8_t sequence;
            uint16_t type;
        }; 

        typedef std::function<void(DeviceHeader&,uint8_t* payload, uint16_t payloadType)> GetFunction;  
        enum class UDP_RESP{
            SUCCESS,
            SENT_SUCCESS,
            SENT_FAILED,
            RECEIVE_TIMED_OUT,
            ACK_TIMED_OUT,
            ACK_WRONG_MSG
        };

        enum class UDP_SETUP_RESP{
            SOCKET_CREATION_FAIL,
            BIND_FAIL,
            SUCCESS
        };

        LIFX_UDP(){
            //sets the source id to a random number
            srand((uint32_t)time(nullptr));
            m_sourceId = (uint32_t)rand();
        }
        
        UDP_SETUP_RESP Begin();

        //returns true of query was sent, or false if already discovering or send failed
        //NOTE: Will reset currently discovered devices
        bool DiscoverDevices();
        bool DiscoverDevicesBlocking();
        bool StillDiscovering();
        //Returns the number of discovered devices
        uint32_t GetDiscoveredDevices();
        //If the id is invalid, it will return a device with
        //all zeros. Check if its invalid using Device::IsValid(dev)
        Device GetDevice(uint32_t id);

        void SetGetFunction(GetFunction& onGet){
            m_onGetFunction = onGet;
        }

        

        UDP_RESP SetPower(Payloads::SetPower payload, const Device& device, bool requireAck = true){ 
            return SendSetPacket(payload,&device,requireAck); 
        }
        //will broadcast to all devices discovered by DiscoverDevices
        UDP_RESP SetPower(Payloads::SetPower payload, bool requireAck = true){
            return SendSetPacket(payload, nullptr, requireAck);
        }

        UDP_RESP SetLightPower(Payloads::SetLightPower payload, const Device& device, bool requireAck = true){
            
            return SendSetPacket(payload, &device, requireAck);
        }
        
        //will broadcast to all devices discovered by DiscoverDevices
        UDP_RESP SetLightPower(Payloads::SetLightPower payload, bool requireAck = true){
            return SendSetPacket(payload, nullptr, requireAck);
        }


    private:
        
        static const uint16_t GetHeaderSize();

        struct DeviceManager{
            Device devices[MAX_DEVICES];
            uint16_t discoveredDevices;
            uint64_t lastAddTime;
            bool discovering;
        } m_deviceManager;
        int m_sock;
        
        bool SendPacket(const uint8_t* data, size_t len, sockaddr_in& dest);
        bool SendMessage(const uint8_t* data, size_t len, const Device* dev);

        GetFunction m_onGetFunction;

        TaskHandle_t m_pollTask = nullptr;
        static void UDPPollTask(void *data);

        //Creates the packet header. Payload offset should be at HEADER_SIZE
        //Target should be 0 when tagged is 0, as it is ignored
        //(and for some reason not having it zero when not tagged caused inconsistencies)
        uint8_t* GetSendHeader(uint16_t packetType, uint32_t payloadSize, bool requireAck, uint32_t sourceId, uint8_t sequence, uint64_t target, bool tagged);

        //payload must be LIFX::Payloads::Set... or compilation will fail
        template<typename T>
        UDP_RESP SendSetPacket(const T payload, const Device* dev, bool requireAck){
            uint8_t* data = GetSendHeader(payload.packetId,payload.GetSize(),requireAck,m_sourceId,++m_sequence,0, dev==nullptr);
            // target = 0 (broadcast only when dev==nullptr (tagged = 1))
            payload.SerialiseTo(data + GetHeaderSize());
            bool res = SendMessage(data, GetHeaderSize() + payload.GetSize(), dev);
            delete[] data;
            if(!res) return UDP_RESP::SENT_FAILED;
            return UDP_RESP::SENT_SUCCESS;
        }

        static uint32_t m_sourceId;
        uint8_t m_sequence = 0;

    };

    class LIFX_HTTP{
    public:
    private:
    };
}

