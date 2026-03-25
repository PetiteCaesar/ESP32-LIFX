#pragma once
#include <cstdint>
#include "Payloads.h"



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
            QUEUE_FULL
        };

        LIFX_UDP(){
            //sets the source id to a random number
            srand((uint32_t)time(nullptr));
            m_sourceId = (uint32_t)rand();
        }
        static const uint16_t port = 56700;
        
        void Begin();

        SET_RESP SetPower(Payloads::SetPower payload, const Device& device, bool requireAck = true){
            return _SetPower(payload,&device,requireAck);
        }
        SET_RESP SetPower(Payloads::SetPower payload, bool requireAck = true){
            return _SetPower(payload,nullptr,requireAck);
        }

    private:

        //true if successfully sent msg, false if failed, or 
        SET_RESP _SetPower(Payloads::SetPower payload, const Device* dev, bool requireAck);

        static uint32_t m_sourceId;
    };

    class LIFX_HTTP{
    public:
    private:
    };
}

