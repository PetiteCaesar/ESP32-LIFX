#pragma once
#include <cstdint>
#include "Payloads.h"
#include <lwip/sockets.h>


namespace LIFX{
    struct Device{
        uint64_t id;
        uint8_t service;
        uint32_t port;

        static uint64_t GetTarget(const Device* dev){
            return (dev == nullptr) ? 0 : dev->id;
        } 
    };

    class LIFX_UDP{
    public:

        enum class SET_RESP{
            SENT_SUCCESS,
            SENT_FAILED,
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
        static const uint16_t port = 56700;
        
        UDP_SETUP_RESP Begin();

        SET_RESP SetPower(Payloads::SetPower payload, const Device& device, bool requireAck = true){
            return _SetPower(payload,&device,requireAck);
        }
        SET_RESP SetPower(Payloads::SetPower payload, bool requireAck = true){
            return _SetPower(payload,nullptr,requireAck);
        }

    private:

        int m_sock;
        sockaddr_in m_broadcastDest;
        
        void ReceivePacket();
        bool SendBroadcast(const uint8_t* data, size_t len);


        //Creates the packet header. Payload offset should be at HEADER_SIZE
        uint8_t* GetSendHeader(uint16_t packetType, uint32_t payloadSize, bool requireAck, uint8_t sequence, uint64_t target);

        //true if successfully sent msg, false if failed, or 
        SET_RESP _SetPower(Payloads::SetPower payload, const Device* dev, bool requireAck);

        static uint32_t m_sourceId;
    };

    class LIFX_HTTP{
    public:
    private:
    };
}

