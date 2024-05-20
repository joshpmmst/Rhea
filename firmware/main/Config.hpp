#pragma once

#include <stdint.h>
#include <vector>

template<
typename T,
T MIN_IN_VALUE,
T MAX_IN_VALUE,
uint16_t MIN_OUT_VALUE,
uint16_t MAX_OUT_VALUE
> struct MappedPWMValue {
    uint16_t value;

    MappedPWMValue& operator=(T v) {
        value = (v < MIN_IN_VALUE)
            ? (MIN_IN_VALUE)
            : ((v > MAX_IN_VALUE) ? MAX_IN_VALUE : v);

        value = (int16_t)(value - MIN_IN_VALUE) * (int16_t)(MAX_OUT_VALUE - MIN_OUT_VALUE) / (int16_t)(MAX_IN_VALUE - MIN_IN_VALUE) + MIN_OUT_VALUE;

        return *this;
    }

    MappedPWMValue(T v = MIN_IN_VALUE) {
        *this = v;
    }
};

class Config {
public:
    struct Shot {
        MappedPWMValue<int8_t, -30, 30, 1100, 1900> yaw_angle = 0;
        MappedPWMValue<uint8_t, 0, 100, 1000, 1150> bottom_motor = 0;
        MappedPWMValue<uint8_t, 0, 100, 1000, 1150> left_motor = 0;
        MappedPWMValue<uint8_t, 0, 100, 1000, 1150> right_motor = 0;
    };

private:
    static inline uint8_t balls_per_minute = 0;
    static constexpr uint8_t MIN_BALLS_PER_MINUTE = 5;
    static constexpr uint8_t MAX_BALLS_PER_MINUTE = 45;

    static inline std::vector<Shot> shots;
    static inline uint8_t active_shot;

public:
    static void AddShot(const Shot& shot) {
        shots.push_back(shot);
    }

    static void RemoveShot(const uint8_t index) {
        if (index < shots.size()) {
            shots.erase(shots.begin() + index);
            if (shots.empty() || index == 0) {
                active_shot = 0;
            } else if (index >= active_shot) {
                active_shot -= 1;
            }
        }
    }

    //static void SetActiveShot(const uint8_t _active_shot) {
    //    if (_active_shot < shots.size()) {
    //        active_shot = _active_shot;
    //    }
    //}

    static bool AreAnyShotsConfigured() {
        return !shots.empty();
    }

    static Shot GetActiveShot() {
        return shots.at(active_shot);
    }

    static void SetNextActiveShot() {
        active_shot = (active_shot + 1) % shots.size();
    }

    static void SetBallsPerMinute(const uint8_t _balls_per_minute) {
        balls_per_minute = 
        (_balls_per_minute > MAX_BALLS_PER_MINUTE) ? MAX_BALLS_PER_MINUTE : ((_balls_per_minute < MIN_BALLS_PER_MINUTE) ? MIN_BALLS_PER_MINUTE : _balls_per_minute);
    }

    static uint64_t GetDelayBetweenLaunchesMs() {
        return (uint64_t)(1000.0f / (((float)balls_per_minute / 60.0f)));
    }
};
