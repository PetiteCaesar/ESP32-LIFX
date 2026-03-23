#pragma once
#include <cstdint>
namespace LIFX{

    struct Colour{
        uint16_t hue;
        uint16_t saturation;
        uint16_t brightness;
        uint16_t kelvin;
    };

    //Payloads used when sending messages to an LIFX device
    namespace Payloads{

        //Device

        struct SetPower{
            uint16_t level;//0 = off, 65535 = on
        };

        struct SetLabel{
            char label[32];//The new label on the device
        };

        struct SetLocation{
            uint8_t location[16];//the UUID of the location - Same for each device in the location
            char label[32];//The name of the location
            uint64_t updatedAt;//epoch in nanoseconds for when it was updated
        };

        struct SetGroup{
            uint8_t group[16];//UUID of the group. It should be the same for each device in the group
            char label[32];//Name of the group
            uint64_t updatedAt;//epoch in nanoseconds for when it was updated
        };
        //End device

        //Light

        struct SetColor{
            Colour colour;
            uint32_t duration;//time in milliseconds to transition to new HSBK 
        };

        struct SetWaveform{
            bool transient;
            Colour colour;
            uint32_t period;
            float cycles;
            int16_t skewRatio;
            uint8_t waveform;//0=SAW, 1=SINE, 2=HALF_SINE, 3=TRIANGLE, 4=PULSE
        };

        struct SetLightPower{
            uint16_t level;//0 = off, 65535 = on 
            uint32_t duration;//time in milliseconds to transition to new level
        };
        
        struct SetWaveformOptional{
            bool transient;
            Colour colour;
            uint32_t period;
            float cycles;
            int16_t skewRatio;
            uint8_t waveform;//0=SAW, 1=SINE, 2=HALF_SINE, 3=TRIANGLE, 4=PULSE
            bool setHue;
            bool setSaturation;
            bool setBrightness;
            bool setKelvin;
        };

        struct SetInfrared{
            uint16_t brightness;//0= no infrared, 65535 = the most infrared O_O
        };

        struct SetHevCycle{
            bool enable;//set cycles on or off
            uint32_t durationS;//duration in seconds the cycle should last for. 0 will use the default set by SetHevCycleConfiguration
        };

        struct SetHevCycleConfiguration{
            bool indication;//
            uint32_t durationS;//The default duration used by SetHevCycle;
        };
        //End Light

        //Multizone

        struct SetColorZones{
            uint8_t startIndex;
            uint8_t endIndex;
            Colour colour;
            uint32_t duration;//time in milliseconds to transition to new values
            uint8_t apply;//0=NO_APPLY, 1=APPLY, 2=APPLY_ONLY 
        };

        struct SetMultiZoneEffect{
            uint32_t instanceId;//unique number identifying this effect
            uint8_t type;//0=OFF, 1=MOVE
            uint32_t speed;//time for one cycle in milliseconds
            uint64_t duration;//The time the effect will run for in nanoseconds
            uint8_t parameters[32];//As said in the docs: "This field is 8 `4` byte fields which change meaning based on the effect that is running. When the effect is MOVE only the second field is used and is a Uint32 representing the DIRECTION enum. This field is currently ignored for all other multizone effects." The direction enum being: 0=REVERSED, 1=NOT_REVERSED
        };

        struct SetExtendedColorZones{
            uint32_t duration;//The time in milliseconds to transition to the new values
            uint8_t apply;//0=NO_APPLY, 1=APPLY, 2=NO_APPLY
            uint16_t zone_index;//the first zone to apply colours from. Use this as a starting index if you plan on changing more than 82 zones buy sending multiple messages. Eg, one at 0, and one at 82
            uint8_t coloursCount;
            Colour colours[82];//The colours to change the strip with
        };
        //End Multizone

        //Start Relay

        struct SetRPower{
            uint8_t relayIndex;//The relay on the switch starting from 0
            uint16_t level;//The new level of the relay
        };
        //End Relay

        //Tile

        //i dont even have any to test with, and theres "alot" here
    }

    namespace Response{
        struct StateService{
            uint8_t service;//1=UDP, 2-5=Reserved1-4
            uint32_t port;//Port of service, usually 56700, but not always
        };

        /*
        Major and Minor versions should be looked at like
        (major, minor).
        Each generation of devices has a different major
        */
        struct StateHostFirmware{
            uint64_t build;//timestamp of the firmware on the device as an epoch
            uint16_t versionMinor;
            uint16_t versionMajor;
        };



        /*
        RSSI can be calculated using `rssi = int(floor(10 * Log10(signal) + 0.5))`
        More info can be found at https://lan.developer.lifx.com/docs/information-messages#statewifiinfo---packet-17
        */
        struct StateWifiInfo{
            float signal;//the signal strength of the device
        };

         /*
        Major and Minor versions should be looked at like
        (major, minor).
        Each generation of devices has a different major
        */
        struct StateWifiFirmware{
            uint64_t build;//timestamp of the wifi firmware on the device as an epoch. Only relevant for the first two generations of LIFX products
            uint16_t versionMinor;
            uint16_t versionMajor;
        };

        struct StatePower{
            uint16_t level;//The level of a device (0=Off, 65535=On)
        };
    }
    
}