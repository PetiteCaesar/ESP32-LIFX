#pragma once
#include <cstdint>




namespace LIFX{
    struct Light{
        uint64_t id;
        uint8_t service;
        uint32_t port;
    };

    class LIFX_UDP{
    public:

        static const uint16_t port = 56700;
        
        void Begin();

    private:

    };

    class LIFX_HTTP{
    public:
    private:
    };
}

