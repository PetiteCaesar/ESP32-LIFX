// Copyright (c) 2026 Lachlan Trevascus
// MIT License.
#pragma once
#include <cstdint>
#include "Payloads.h"
#include "LIFXConfig.h"
#include <lwip/sockets.h>
#include <functional>
#include <atomic>

namespace LIFX{

    class LIFX_UDP{
    public:


        //public facing header
        struct DeviceHeader{
            uint16_t size;
            uint32_t source;
            uint64_t target;
            uint8_t sequence;
            uint16_t type;
            sockaddr_in sourceAddr;
        }; 
        struct Device{
            Device() = default;
            inline uint64_t GetTarget() const {return target;}
            inline uint8_t GetService() const {return service;}
            inline uint32_t GetPort() const {return port;}
            inline uint32_t GetIPAddress() const {return ipAddr;}
            inline bool GetUsingIP() const {return useIp;}
            //Should commands sent to this device be sent using
            //the devices IP, or by broadcasting with its 
            //target address
            inline void UseIp(bool val) {useIp = val;}

            static bool IsValid(const Device& dev){
                return (dev.target != 0 || dev.useIp)
                    && dev.service != 0
                    && dev.port != 0
                    && (dev.ipAddr != 0 || !dev.useIp);
            }
            private:

            uint64_t target = 0;
            uint8_t service = 0;
            uint32_t port = 0;
            uint32_t ipAddr = 0;
            //true - send direct to ip
            //false - broadcast to target
            bool useIp = true;

            friend class LIFX_UDP;
            static uint64_t GetTarget(const Device* dev){
                return (dev == nullptr) ? 0 : dev->target;
            } 
        };

        enum class UDP_RESP{
            FAILED,
            SUCCESS,
        };
        enum class UDP_SETUP_RESP{
            SOCKET_CREATION_FAIL,
            BIND_FAIL,
            SUCCESS
        };

        
        #ifndef USE_RAW_FUNCTIONS
        //header, payload, payloadType
        typedef std::function<void(DeviceHeader&,const uint8_t*)> OnResponseFunction;  
        #else
        typedef void(*OnResponseFunction)(void*,DeviceHeader&, const uint8_t*) ;
        #endif  

        const char* UDPRespToString(UDP_RESP resp){
            switch(resp){
                case UDP_RESP::FAILED: return "SENT_FAILED";
                default: return "UNKNOWN";
            }
        }

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

        

        UDP_RESP SetPower(Payloads::SetPower payload, const Device& device, bool requireAck = true){ 
            return SendSetPacket(payload,&device,requireAck); 
        }
        UDP_RESP SetPower(Payloads::SetPower payload, bool requireAck = true){
            return SendSetPacket(payload, nullptr, requireAck);
        }
        UDP_RESP SetLightPower(Payloads::SetLightPower payload, const Device& device, bool requireAck = true){
            return SendSetPacket(payload, &device, requireAck);
        }
        UDP_RESP SetLightPower(Payloads::SetLightPower payload, bool requireAck = true){
            return SendSetPacket(payload, nullptr, requireAck);
        }



        UDP_RESP GetResponse(GetQuery query, OnResponseFunction func, Device& dev){
            int resp = m_responseManager.QueueResponse(func);
            if(resp > 0){
                SendGetPacket((uint16_t)query,&dev,resp);
                return UDP_RESP::SUCCESS;
            }
            return UDP_RESP::FAILED;
        }

    private:
        
        static const uint16_t GetHeaderSize();

        struct DeviceManager{
            Device devices[MAX_DEVICES];
            uint16_t discoveredDevices;
            uint64_t lastAddTime;
            bool discovering;
        } m_deviceManager;


        struct Response{
            OnResponseFunction func;
            bool created = false;
            uint32_t createdAt;
        };
        struct GetResponseManager{
            GetResponseManager(){
                for(int i = 0; i < GET_STATE_BUFFER_SIZE; ++i)
                    avail[i] = i;
                head.store(GET_STATE_BUFFER_SIZE, std::memory_order_relaxed);
            }
            void RunResponse(uint8_t seq, DeviceHeader& h,const uint8_t* payload){
                if(m_functions[seq].created){
                    m_functions[seq].func(h,payload);
                    m_functions[seq].created = false;
                    int index = head.load(std::memory_order_relaxed);
                    avail[index] = seq;
                    head.store(index+1,std::memory_order_release);
                    printf("ran resp, ind %d\n", index+1);
                } 
            }
            //seq to send otherwise -1 if cant queue
            int QueueResponse(OnResponseFunction func){
                int index = head.load(std::memory_order_relaxed);
                uint32_t timeNowMs = esp_timer_get_time()/1000;
                if(index > 0){
                    --index; head.store(index,std::memory_order_release);
                    uint8_t slot = avail[index];

                    Response& resp = m_functions[slot];
                    resp.func = func;
                    resp.created = true;
                    resp.createdAt = timeNowMs;
                    return slot;
                }

                //no space so over and check for timeout
                for(int i = 0; i < GET_STATE_BUFFER_SIZE; i++){
                    auto& slot = m_functions[i];
                    //if its timed out
                    if(slot.created && timeNowMs - slot.createdAt > SOCKET_TIMEOUT_MS){
                        slot.func = func;
                        slot.createdAt = timeNowMs;
                        return i;
                    }
                }
                return -1;
                
            }


            private:
            Response m_functions[GET_STATE_BUFFER_SIZE];
            uint8_t avail[GET_STATE_BUFFER_SIZE];
            std::atomic<int> head;
        } m_responseManager;

        int m_sock;
        
        bool SendPacket(const uint8_t* data, size_t len, sockaddr_in& dest);
        bool SendMessage(const uint8_t* data, size_t len, const Device* dev);

        TaskHandle_t m_pollTask = nullptr;
        static void UDPPollTask(void *data);
        static void OnDiscoverRecv(DeviceManager& dm, DeviceHeader& header, uint8_t* buffer);

        //Creates the packet header. Payload offset should be at HEADER_SIZE
        //Target should be 0 when tagged is 0, as it is ignored
        //(and for some reason not having it zero when not tagged caused inconsistencies)
        uint8_t* GetSendHeader(uint16_t packetType, uint32_t payloadSize, bool requireAck, uint32_t sourceId, uint8_t sequence, uint64_t target, bool tagged);

        //payload must be LIFX::Payloads::Set... or compilation will fail
        template<typename T>
        UDP_RESP SendSetPacket(const T payload, const Device* dev, bool requireAck){
            uint8_t* data = GetSendHeader(payload.packetId,payload.GetSize(),requireAck,m_sourceId,++m_sequence,dev->GetTarget(dev), dev==nullptr);

            if(!data) return UDP_RESP::FAILED;
            payload.SerialiseTo(data + GetHeaderSize());
            bool res = SendMessage(data, GetHeaderSize() + payload.GetSize(), dev);
            delete[] data;
            if(!res) return UDP_RESP::FAILED;
            return UDP_RESP::SUCCESS;
        }

        UDP_RESP SendGetPacket(uint16_t type, const Device* dev, uint8_t seq){
            uint8_t* data = GetSendHeader(type,0,false,GET_RESPONSE_SOURCE_ID,seq,dev->GetTarget(dev), dev==nullptr);

            if(!data) return UDP_RESP::FAILED;
            bool res = SendMessage(data, GetHeaderSize(), dev);
            delete[] data;
            if(!res) return UDP_RESP::FAILED;
            return UDP_RESP::SUCCESS;
        }

        uint8_t m_sequence = 0;
        static uint32_t m_sourceId;
    };

    class LIFX_HTTP{
    public:
    private:
    };
}

