// Copyright (c) 2026 Lachlan Trevascus
// MIT License.
#pragma once
#include <cstdint>
#include <string.h>
#include "lwip/inet.h"

namespace LIFX{


    enum class GetQuery : uint16_t{
        Service = 2,
        HostFirmware = 14,
        WifiInfo = 16,
        WifiFirmware = 18,
        Power = 20,
        Label = 23,
        Version = 32,
        Info = 34,
        Location = 48,
        Group = 51,
        // EchoRequest = 58, not implemented non empty payload
        Colour = 101,
        //Same as Power
        LightPower = 116,
        Infrared = 120,
        HevCycle = 142,
        HevCycleConfiguration = 145,
        LastHevCycleResult = 148,
        //ColourZones = 502, not implemented yet non empty payload
        //MultiZoneEffect = 507, not implemented yet non empty payload
        //ExtendedColourZones = 511, not implemented yet, multiple responses
        //RPower = 816, not implemented yet non empty payload
        DeviceChain = 701,
        Get64 = 707,
        TileEffect = 718,
        SensorGetAmbientLight = 401
    };


    inline void __writeBytes(uint8_t*& dst, const void* src, size_t size) {
        memcpy(dst, src, size);
        dst += size;
    }

    template<typename T>
    inline T convert(const uint8_t* data){
        T res = 0;
        for(int i = 0; i < sizeof(T); ++i) res |= ((T)data[i]) << i*8;
        return res; 
    }


    /*
    NOTES FOR ALL Structs:
    Deserialise and Serialise will always need/produce GetSize bytes
    which includes padding as stated in the LIFX docs.
    
    */


    struct Colour{
        uint16_t hue;
        uint16_t saturation;
        uint16_t brightness;
        uint16_t kelvin;

        static Colour Deserialise(const uint8_t* data){
            Colour c;
            c.hue = convert<uint16_t>(&data[0]);
            c.saturation = convert<uint16_t>(&data[2]);
            c.brightness = convert<uint16_t>(&data[4]);
            c.kelvin = convert<uint16_t>(&data[6]);
            return c;
        }

        void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &hue, sizeof(hue));
                __writeBytes(data, &saturation, sizeof(saturation));
                __writeBytes(data, &brightness, sizeof(brightness));
                __writeBytes(data, &kelvin, sizeof(kelvin));
            }
        static inline constexpr uint16_t GetSize() {
            return sizeof(hue) 
                + sizeof(saturation) 
                + sizeof(brightness) 
                + sizeof(kelvin);
        }
    };

    struct Tile{
        int16_t accelMeasX;
        int16_t accelMeasY;
        int16_t accelMeasZ;
        //res 2 bytes
        float userX;
        float userY;
        uint8_t width;
        uint8_t height;
        uint8_t supportedFrameBuffers;
        uint32_t deviceVersionVendor;
        uint32_t deviceVersionProduct;
        //res 4 bytes
        uint64_t firmwareBuild;
        //res 8 bytes
        uint16_t firmwareVersionMinor;
        uint16_t firmwareVersionMajor;
        //res 4 bytes

        static Tile Deserialise(const uint8_t* data){
        }
        void SerialiseTo(uint8_t* data) const{
        }
        static inline constexpr uint16_t GetSize() {
            return 54;//due to padding
        }
    };

    //Payloads used when sending messages to an LIFX device
    namespace Payloads{
        template<GetQuery Q>
        struct ResponseStruct;

        //Device
        struct SetPower{
            //no padding
            uint16_t level;//0 = off, 65535 = on
            static SetPower Deserialise(const uint8_t* data){
                SetPower p;
                p.level = data[0] | (data[1] << 8);
                return p;
            }
            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &level, sizeof(level));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(level);
            }
            static constexpr uint16_t packetId = 21;
        };
        template<>
        struct ResponseStruct<GetQuery::LightPower> {using type = SetPower;};
        template<>
        struct ResponseStruct<GetQuery::Power> {using type = SetPower;};

        struct SetLabel{
            //no padding
            char label[32];//The new label on the device

            static SetLabel Deserialise(const uint8_t* data){
                SetLabel p;
                memcpy(p.label, data, sizeof(label));
                return p;
            }

            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &label, sizeof(label));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(label);
            }
            static constexpr uint16_t packetId = 24;
        };
        template<>
        struct ResponseStruct<GetQuery::Label> {using type = SetLabel;};

        struct SetLocation{
            //no padding
            uint8_t location[16];//the UUID of the location - Same for each device in the location
            char label[32];//The name of the location
            uint64_t updatedAt;//epoch in nanoseconds for when it was updated

            static SetLocation Deserialise(const uint8_t* data){
                SetLocation p;
                memcpy(p.location, data, 16);
                memcpy(p.label, data+16, 32);
                p.updatedAt = convert<uint64_t>(&data[48]);
                return p;
            }

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
        template<>
        struct ResponseStruct<GetQuery::Location> {using type = SetLocation;};

        struct SetGroup{
            //no padding
            uint8_t group[16];//UUID of the group. It should be the same for each device in the group
            char label[32];//Name of the group
            uint64_t updatedAt;//epoch in nanoseconds for when it was updated

            static SetGroup Deserialise(const uint8_t* data){
                SetGroup p;
                memcpy(p.group, data, 16);
                memcpy(p.label, data+16, 32);
                p.updatedAt = convert<uint64_t>(&data[48]);
                return p;
            }

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
        template<>
        struct ResponseStruct<GetQuery::Group> {using type = SetGroup;};
        //End device

        //Light

        struct SetColour{
            //res 1 byte
            Colour colour;
            uint32_t duration;//time in milliseconds to transition to new HSBK
            void SerialiseTo(uint8_t* data) const{
                uint8_t pad = 0;
                __writeBytes(data, &pad, sizeof(pad));
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
            //res 1 byte
            bool transient;
            Colour colour;
            uint32_t period;
            float cycles;
            int16_t skewRatio;
            uint8_t waveform;//0=SAW, 1=SINE, 2=HALF_SINE, 3=TRIANGLE, 4=PULSE
            void SerialiseTo(uint8_t* data) const{
                uint8_t pad = 0;
                __writeBytes(data, &pad, sizeof(pad));
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
            //no padding
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
        
        //NOT IMPLEMENTED
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
            //no padding
            uint16_t brightness;//0= no infrared, 65535 = the most infrared O_O

            static SetInfrared Deserialise(const uint8_t* data){
                SetInfrared p;
                p.brightness = convert<uint16_t>(data);
                return p;
            }

            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &brightness, sizeof(brightness));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(brightness);
            }
            static constexpr uint16_t packetId = 122;
        };
        template<>
        struct ResponseStruct<GetQuery::Infrared> {using type = SetInfrared;};

        struct SetHevCycle{
            //no padding
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
            //no padding
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

        struct SetColourZones{
            //no padding
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
            //res 2 bytes
            uint32_t speed;//time for one cycle in milliseconds
            uint64_t duration;//The time the effect will run for in nanoseconds
            //res 8 bytes
            uint8_t parameters[32];//As said in the docs: "This field is 8 `4` byte fields which change meaning based on the effect that is running. When the effect is MOVE only the second field is used and is a Uint32 representing the DIRECTION enum. This field is currently ignored for all other multizone effects." The direction enum being: 0=REVERSED, 1=NOT_REVERSED

            //data must have GetSize space left
            void SerialiseTo(uint8_t* data) const{
                uint16_t res = 0;
                uint32_t res1 = 0;
                __writeBytes(data, &instanceId, sizeof(instanceId));
                __writeBytes(data, &res,sizeof(res));//res 2 
                __writeBytes(data, &type, sizeof(type));
                __writeBytes(data, &speed, sizeof(speed));
                __writeBytes(data, &duration, sizeof(duration));
                __writeBytes(data, &res1,sizeof(res1));//res 4 bytes
                __writeBytes(data, &res1,sizeof(res1));//res 4 bytes
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

        struct SetExtendedColourZones{
            //no padding
            uint32_t duration;//The time in milliseconds to transition to the new values
            uint8_t apply;//0=NO_APPLY, 1=APPLY, 2=NO_APPLY
            uint16_t zoneIndex;//the first zone to apply colours from. Use this as a starting index if you plan on changing more than 82 zones buy sending multiple messages. Eg, one at 0, and one at 82
            uint8_t coloursCount;
            Colour colours[82];//The colours to change the strip with
            

            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &duration, sizeof(duration));
                __writeBytes(data, &apply, sizeof(apply));
                __writeBytes(data, &zoneIndex, sizeof(zoneIndex));
                __writeBytes(data, &coloursCount, sizeof(coloursCount));
                for(int i = 0; i < coloursCount;i++){
                    colours[i].SerialiseTo(data);
                    data+=colours[i].GetSize();
                }
            }
            inline constexpr uint16_t GetSize() const {
                return sizeof(duration)
                    + sizeof(apply)
                    + sizeof(zoneIndex)
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
            
            static SetRPower Deserialise(const uint8_t* data){
                SetRPower p;
                p.relayIndex = data[0];
                p.level = convert<uint16_t>(&data[1]);
                return p;
            }

            void SerialiseTo(uint8_t* data) const{
                __writeBytes(data, &relayIndex, sizeof(relayIndex));
                __writeBytes(data, &level, sizeof(level));
            }
            inline constexpr uint16_t GetSize() const{
                return sizeof(relayIndex) + sizeof(level);
            }
            // static constexpr uint16_t packetId = 817;
        };
        //End Relay


        //State only (responses)

        //response to ExtendedColourZones but currently not implemented
        struct StateExtendedColourZones{
            uint16_t zoneCount;
            uint16_t zoneIndex;//the first zone to apply colours from. Use this as a starting index if you plan on changing more than 82 zones buy sending multiple messages. Eg, one at 0, and one at 82
            uint8_t coloursCount;
            Colour colours[82];//The colours to change the strip with

            static SetExtendedColourZones Deserialise(const uint8_t* data){
                SetExtendedColourZones p;
                p.duration = convert<uint32_t>(&data[0]);
                p.apply = data[4];
                p.zoneIndex = convert<uint16_t>(&data[5]);
                p.coloursCount = data[6];
                for(int i = 0; i < p.coloursCount; i++){
                    p.colours[i] = Colour::Deserialise(&data[7 + i*Colour::GetSize()]);
                }
                return p;
            }

            inline constexpr uint16_t GetSize() const {
                return sizeof(zoneCount)
                    + sizeof(zoneIndex)
                    + sizeof(coloursCount)
                    + colours[0].GetSize() * coloursCount;
            }

            // static constexpr uint16_t packetId = 512;
        };


        struct StateService{
            static StateService Deserialise(const uint8_t* data){
                StateService p;
                p.service = data[0];
                p.port = convert<uint32_t>(&data[1]);
                return p;
            }
            uint8_t service;//1=UDP, 2-5=Reserved1-4
            uint32_t port;//Port of service, usually 56700, but not always
            // static constexpr uint16_t packetId = 3;
        };
        template<>
        struct ResponseStruct<GetQuery::Service> {using type = StateService;};

        /*
        Major and Minor versions should be looked at like
        (major, minor).
        Each generation of devices has a different major
        */
        struct StateHostFirmware{
            static StateHostFirmware Deserialise(const uint8_t* data){
                StateHostFirmware p;
                p.build = convert<uint64_t>(data);
                //res 8 bytes
                p.versionMinor = convert<uint16_t>(&data[9]);
                p.versionMajor = convert<uint16_t>(&data[11]);
                return p;
            }
            uint64_t build;//timestamp of the firmware on the device as an epoch
            uint16_t versionMinor;
            uint16_t versionMajor;
            // static constexpr uint16_t packetId = 15;
        };
        template<>
        struct ResponseStruct<GetQuery::HostFirmware> {using type = StateHostFirmware;};

    
        struct StateVersion{
            uint32_t vendor;
            uint32_t product;
            static StateVersion Deserialise(const uint8_t* data){
                StateVersion p;
                p.vendor = convert<uint32_t>(&data[0]);
                p.product = convert<uint32_t>(&data[4]);
                return p;
            }
            //  static constexpr uint16_t packetId = 33;
        };
        template<>
        struct ResponseStruct<GetQuery::Version> {using type = StateVersion;};

        struct StateInfo{
            uint64_t time;
            uint64_t upTime;
            uint64_t downTime;
            static StateInfo Deserialise(const uint8_t* data){
                StateInfo p;
                p.time = convert<uint64_t>(&data[0]);
                p.upTime = convert<uint64_t>(&data[8]);
                p.downTime = convert<uint64_t>(&data[16]);
                return p;
            }
            //  static constexpr uint16_t packetId = 33;
        };
        template<>
        struct ResponseStruct<GetQuery::Info> {using type = StateInfo;};

        struct LightState{
            Colour colour;
            uint16_t power;
            char label[32];
            static LightState Deserialise(const uint8_t* data){
                LightState p;
                p.colour = Colour::Deserialise(data);
                p.power = convert<uint16_t>(&data[Colour::GetSize()]);
                memcpy(p.label, data+Colour::GetSize()+2,32);
                return p;
            }
        };
        template<>
        struct ResponseStruct<GetQuery::Colour> {using type = LightState;};

        struct StateHevCycle{
            uint32_t duration;//duration in seconds cycle set to
            uint32_t remaining;//remaining time in seconds for this cycle
            uint8_t lastPower; //"The power state before the HEV cycle started, which will be the power state once the cycle completes. This is only relevant if remaining_s is larger than 0"
            static StateHevCycle Deserialise(const uint8_t* data){
                StateHevCycle p;
                p.duration = convert<uint32_t>(data);
                p.remaining = convert<uint32_t>(data + 4);
                p.lastPower = data[8];
                return p;
            }
        };
        template<>
        struct ResponseStruct<GetQuery::Colour> {using type = LightState;};

        struct StateHevCycleConfiguration {
            uint8_t indication;//whether a short flashing indication runs at the end of the hev cycle 
            uint32_t duration;
            static StateHevCycleConfiguration Deserialise(const uint8_t* data){
                StateHevCycleConfiguration p;
                p.indication = data[0];
                p.duration = convert<uint32_t>(data+1);
                return p;
            }
        };
        template<>
        struct ResponseStruct<GetQuery::HevCycleConfiguration> {using type = StateHevCycleConfiguration;};

        struct StateLastHevCycleResult  {
            /*
                0: SUCCESS
                1: BUSY
                2: INTERRUPTED_BY_RESET
                3: INTERRUPTED_BY_HOMEKIT
                4: INTERRUPTED_BY_LAN
                5: INTERRUPTED_BY_CLOUD
                255: NONE
            */
            uint8_t result;
            static StateLastHevCycleResult Deserialise(const uint8_t* data){
                StateLastHevCycleResult p;
                p.result = data[0];
                return p;
            }
        };
        template<>
        struct ResponseStruct<GetQuery::LastHevCycleResult> {using type = StateLastHevCycleResult;};

        struct StateDeviceChain   {
            uint8_t startIndex;
            Tile tiles[16];
            uint8_t tileDevicesCount;
            static StateDeviceChain Deserialise(const uint8_t* data){
                StateDeviceChain p;
                p.startIndex = data[0];
                for(int i = 0; i < 16; i++){
                    p.tiles[i] = Tile::Deserialise(data + i*Tile::GetSize());
                }
                p.tileDevicesCount = data[Tile::GetSize()*16];
                return p;
            }
        };
        template<>
        struct ResponseStruct<GetQuery::DeviceChain> {using type = StateDeviceChain ;};

        /*
        RSSI can be calculated using `rssi = int(floor(10 * Log10(signal) + 0.5))`
        More info can be found at https://lan.developer.lifx.com/docs/information-messages#statewifiinfo---packet-17
        */
        struct StateWifiInfo{
            static StateWifiInfo Deserialise(const uint8_t* data){
                StateWifiInfo p;
                memcpy(&p.signal, data, sizeof(p.signal));
                return p;
            }
            float signal;//the signal strength of the device
            // static constexpr uint16_t packetId = 17;
        };
        template<>
        struct ResponseStruct<GetQuery::WifiInfo> {using type = StateWifiInfo;};

         /*
        Major and Minor versions should be looked at like
        (major, minor).
        Each generation of devices has a different major
        */
        struct StateWifiFirmware{
            static StateWifiFirmware Deserialise(const uint8_t* data){
                StateWifiFirmware p;
                p.build = convert<uint64_t>(data);
                //res 8 bytes
                p.versionMinor = convert<uint16_t>(&data[9]);
                p.versionMajor = convert<uint16_t>(&data[11]);
                return p;
            }
            uint64_t build;//timestamp of the wifi firmware on the device as an epoch. Only relevant for the first two generations of LIFX products
            uint16_t versionMinor;
            uint16_t versionMajor;
            // static constexpr uint16_t packetId = 19;
        };
        template<>
        struct ResponseStruct<GetQuery::WifiFirmware> {using type = StateWifiFirmware;};

        template<GetQuery Q>
        typename ResponseStruct<Q>::type GetResponse(const uint8_t* data) {
            return ResponseStruct<Q>::type::Deserialise(data);
        }

    }
}