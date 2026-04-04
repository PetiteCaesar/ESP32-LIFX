#pragma once
#include <cstdint>
#include <string.h>
#include "lwip/inet.h"

namespace LIFX{

    inline void __writeBytes(uint8_t*& dst, const void* src, size_t size) {
        memcpy(dst, src, size);
        dst += size;
    }

    struct Colour{
        uint16_t hue;
        uint16_t saturation;
        uint16_t brightness;
        uint16_t kelvin;
        void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &hue, sizeof(hue));
                __writeBytes(data, &saturation, sizeof(saturation));
                __writeBytes(data, &brightness, sizeof(brightness));
                __writeBytes(data, &kelvin, sizeof(kelvin));
            }
        inline constexpr uint16_t GetSize() const{
            return sizeof(hue) 
                + sizeof(saturation) 
                + sizeof(brightness) 
                + sizeof(kelvin);
        }
    };



    //Payloads used when sending messages to an LIFX device
    namespace Payloads{

        //Device

        struct SetPower{
            uint16_t level;//0 = off, 65535 = on
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &level, sizeof(level));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(level);
            }
            static constexpr uint16_t packetId = 21;
        };

        struct SetLabel{
            char label[32];//The new label on the device
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &label, sizeof(label));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(label);
            }
            static constexpr uint16_t packetId = 24;
        };

        struct SetLocation{
            uint8_t location[16];//the UUID of the location - Same for each device in the location
            char label[32];//The name of the location
            uint64_t updatedAt;//epoch in nanoseconds for when it was updated
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &location, sizeof(location));
                __writeBytes(data, &label, sizeof(label));
                __writeBytes(data, &updatedAt, sizeof(updatedAt));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(location)+sizeof(label)+sizeof(updatedAt);
            }
            static constexpr uint16_t packetId = 49;
        };

        struct SetGroup{
            uint8_t group[16];//UUID of the group. It should be the same for each device in the group
            char label[32];//Name of the group
            uint64_t updatedAt;//epoch in nanoseconds for when it was updated
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &group, sizeof(group));
                __writeBytes(data, &label, sizeof(label));
                __writeBytes(data, &updatedAt, sizeof(updatedAt));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(group)+sizeof(label)+sizeof(updatedAt);
            }
            static constexpr uint16_t packetId = 52;
        };
        //End device

        //Light

        struct SetColor{
            Colour colour;
            uint32_t duration;//time in milliseconds to transition to new HSBK
            void SerialiseTo(uint8_t* data) const{
                colour.SerialiseTo(data);
                data += colour.GetSize();
                __writeBytes(data, &duration, sizeof(duration));
            }
            inline constexpr uint16_t GetSize() const{
                return colour.GetSize() + sizeof(duration);
            } 
            static constexpr uint16_t packetId = 102;
        };

        struct SetWaveform{
            bool transient;
            Colour colour;
            uint32_t period;
            float cycles;
            int16_t skewRatio;
            uint8_t waveform;//0=SAW, 1=SINE, 2=HALF_SINE, 3=TRIANGLE, 4=PULSE
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &transient, sizeof(transient));
                colour.SerialiseTo(data);
                data += colour.GetSize();
                __writeBytes(data, &period, sizeof(period));
                __writeBytes(data, &cycles, sizeof(cycles));
                __writeBytes(data, &skewRatio, sizeof(skewRatio));
                __writeBytes(data, &waveform, sizeof(waveform));
                
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(transient) 
                    + colour.GetSize() 
                    + sizeof(period) 
                    + sizeof(cycles) 
                    + sizeof(skewRatio) 
                    + sizeof(waveform);
            } 
            static constexpr uint16_t packetId = 102;
        };

        struct SetLightPower{
            uint16_t level;//0 = off, 65535 = on 
            uint32_t duration;//time in milliseconds to transition to new level
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &level, sizeof(level));
                __writeBytes(data, &duration, sizeof(duration));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(level) + sizeof(duration);
            }
            static constexpr uint16_t packetId = 117;
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
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &brightness, sizeof(brightness));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(brightness);
            }
            static constexpr uint16_t packetId = 122;
        };

        struct SetHevCycle{
            bool enable;//set cycles on or off
            uint32_t durationS;//duration in seconds the cycle should last for. 0 will use the default set by SetHevCycleConfiguration
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &enable, sizeof(enable));
                __writeBytes(data, &durationS, sizeof(durationS));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(enable) + sizeof(durationS);
            }
            static constexpr uint16_t packetId = 143;
        };

        struct SetHevCycleConfiguration{
            bool indication;//
            uint32_t durationS;//The default duration used by SetHevCycle;
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &indication, sizeof(indication));
                __writeBytes(data, &durationS, sizeof(durationS));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(indication) + sizeof(durationS);
            }
            static constexpr uint16_t packetId = 146;
        };
        //End Light

        //Multizone

        struct SetColorZones{
            uint8_t startIndex;
            uint8_t endIndex;
            Colour colour;
            uint32_t duration;//time in milliseconds to transition to new values
            uint8_t apply;//0=NO_APPLY, 1=APPLY, 2=APPLY_ONLY 
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &startIndex, sizeof(startIndex));
                __writeBytes(data, &endIndex, sizeof(endIndex));
                colour.SerialiseTo(data);
                data+=colour.GetSize();
                __writeBytes(data, &duration, sizeof(duration));
                __writeBytes(data, &apply, sizeof(apply));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(startIndex) 
                    + sizeof(endIndex) 
                    + colour.GetSize() 
                    + sizeof(duration) 
                    + sizeof(apply);
            }
            static constexpr uint16_t packetId = 501;
        };

        struct SetMultiZoneEffect{
            uint32_t instanceId;//unique number identifying this effect
            uint8_t type;//0=OFF, 1=MOVE
            uint32_t speed;//time for one cycle in milliseconds
            uint64_t duration;//The time the effect will run for in nanoseconds
            uint8_t parameters[32];//As said in the docs: "This field is 8 `4` byte fields which change meaning based on the effect that is running. When the effect is MOVE only the second field is used and is a Uint32 representing the DIRECTION enum. This field is currently ignored for all other multizone effects." The direction enum being: 0=REVERSED, 1=NOT_REVERSED
            
            //data must have GetSize space left
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &instanceId, sizeof(instanceId));
                __writeBytes(data, &type, sizeof(type));
                __writeBytes(data, &speed, sizeof(speed));
                __writeBytes(data, &duration, sizeof(duration));
                __writeBytes(data, &parameters, sizeof(parameters));
            }
            inline constexpr uint16_t GetSize() const {
                return sizeof(instanceId)
                    + sizeof(type)
                    + sizeof(speed)
                    + sizeof(duration)
                    + sizeof(parameters);
            }
            static constexpr uint16_t packetId = 508;
        };

        struct SetExtendedColorZones{
            uint32_t duration;//The time in milliseconds to transition to the new values
            uint8_t apply;//0=NO_APPLY, 1=APPLY, 2=NO_APPLY
            uint16_t zone_index;//the first zone to apply colours from. Use this as a starting index if you plan on changing more than 82 zones buy sending multiple messages. Eg, one at 0, and one at 82
            uint8_t coloursCount;
            Colour colours[82];//The colours to change the strip with
            
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &duration, sizeof(duration));
                __writeBytes(data, &apply, sizeof(apply));
                __writeBytes(data, &zone_index, sizeof(zone_index));
                __writeBytes(data, &coloursCount, sizeof(coloursCount));
                for(int i = 0; i < coloursCount;i++){
                    colours[i].SerialiseTo(data);
                    data+=colours[i].GetSize();
                }


            }
            inline constexpr uint16_t GetSize() const {
                return sizeof(duration)
                    + sizeof(apply)
                    + sizeof(zone_index)
                    + sizeof(coloursCount)
                    + colours[0].GetSize() * coloursCount;
            }

            static constexpr uint16_t packetId = 510;
        };
        //End Multizone

        //Start Relay

        struct SetRPower{
            uint8_t relayIndex;//The relay on the switch starting from 0
            uint16_t level;//The new level of the relay
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &relayIndex, sizeof(relayIndex));
                __writeBytes(data, &level, sizeof(level));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(relayIndex) + sizeof(level);
            }
            static constexpr uint16_t packetId = 817;
        };
        //End Relay

        //Tile

        //i dont even have any to test with, and theres "allot" here
    }

    namespace Response{
        struct StateService{
            uint8_t service;//1=UDP, 2-5=Reserved1-4
            uint32_t port;//Port of service, usually 56700, but not always
            in_addr deviceIP;//the devices IP
            static constexpr uint16_t packetId = 3;
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
            static constexpr uint16_t packetId = 15;
        };



        /*
        RSSI can be calculated using `rssi = int(floor(10 * Log10(signal) + 0.5))`
        More info can be found at https://lan.developer.lifx.com/docs/information-messages#statewifiinfo---packet-17
        */
        struct StateWifiInfo{
            float signal;//the signal strength of the device
            static constexpr uint16_t packetId = 17;
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
            static constexpr uint16_t packetId = 19;
        };

        // struct StatePower{
        //     uint16_t level;//The level of a device (0=Off, 65535=On)
        //     static constexpr uint16_t packetId = 22;
        // };

        // struct StateLabel{
        //     char label[32];//The devices label
        //     static constexpr uint16_t packetId = 25;
        // };
    }
    
}