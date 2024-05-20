#pragma once

#include <stdint.h>

struct Packet {
    enum class ID : uint8_t {
        AddShot,
        RemoveShot,
        StartSession,
        StopSession,
        Keepalive,
        Max
    };

    static constexpr uint8_t HEADER = 0x47;
    static constexpr uint8_t FOOTER = 0xED;
    static constexpr uint8_t CONTENT_LENGTH = 5;
    // header + id + content + footer
    static constexpr uint8_t PACKET_LENGTH = sizeof(HEADER) + 1 + CONTENT_LENGTH + sizeof(FOOTER);

    uint8_t* data = nullptr;

    ID GetID() const {
        return (ID)data[1];
    }

    template<typename T>
    T GetValue(uint8_t index) const {
        return *((T*)&data[1 + 1 + index]);
    }
};
