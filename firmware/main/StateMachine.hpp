#pragma once

#include <stdint.h>

#include <esp_timer.h>

#include "Packets.hpp"
#include "Config.hpp"
#include "PWMMotors.hpp"
#include "IRSensors.hpp"

class StateMachine {
private:
    static constexpr const char* TAG = "StateMachine";
    
    static inline bool is_session_active = false;

    enum class States {
        SetYawAndMotorSpeeds,
        WaitForBall,
        Delay
    };
    static inline States state = States::SetYawAndMotorSpeeds;

    static inline esp_timer_handle_t esp_timer_handle;
    static constexpr uint64_t PERIOD_MS = 10;

    // Previous PWM values for yaw servo and 3 launching motors.
    // Used to smooth transitions between shots.
    static inline uint16_t previous_widths[4] = {0};
    static constexpr uint16_t SMOOTH_TIME_DURATION_MS = 500;
    static inline uint64_t smoothing_time_start = 0;

    static void UpdatePreviousWidths() {
        previous_widths[0] = PWMMotors::YawServo.width;
        previous_widths[1] = PWMMotors::BottomBLDC.width;
        previous_widths[2] = PWMMotors::LeftBLDC.width;
        previous_widths[3] = PWMMotors::RightBLDC.width;
    }

    static void InterpolateWidths(uint64_t time) {
        float interpolation = ((time - smoothing_time_start) / (float)SMOOTH_TIME_DURATION_MS);
        const auto& active_shot = Config::GetActiveShot();

        PWMMotors::YawServo.UpdateWidth(previous_widths[0] + interpolation * (active_shot.yaw_angle.value - previous_widths[0]));
        PWMMotors::BottomBLDC.UpdateWidth(previous_widths[1] + interpolation * (active_shot.bottom_motor.value - previous_widths[1]));
        PWMMotors::LeftBLDC.UpdateWidth(previous_widths[2] + interpolation * (active_shot.left_motor.value - previous_widths[2]));
        PWMMotors::RightBLDC.UpdateWidth(previous_widths[3] + interpolation * (active_shot.right_motor.value - previous_widths[3]));
    }

    static uint64_t GetTimeMs() {
        return esp_timer_get_time() / 1'000;
    }

    static inline uint64_t last_keepalive_time = 0;
    static constexpr uint64_t KEEPALIVE_TIMEOUT_MS = 2'000;

    static constexpr uint64_t LOCK_SERVO_ANIMATION_TIME_MS = 800;
    static inline uint64_t ball_launch_time = 0;
public:
    static void Init() {
        UpdatePreviousWidths();

        last_keepalive_time = GetTimeMs();

        const esp_timer_create_args_t timer_args = {
            .callback = &Tick,
            .arg = (void*)esp_timer_handle,
            .name = "state_machine"
        };
        esp_timer_create(&timer_args, &esp_timer_handle);
        esp_timer_start_once(esp_timer_handle, PERIOD_MS * 1'000);
    }

    static void Tick(void* unused) {
        if (!is_session_active) {
            // Stop motors.
            PWMMotors::BottomBLDC.UpdateWidth(0);
            PWMMotors::LeftBLDC.UpdateWidth(0);
            PWMMotors::RightBLDC.UpdateWidth(0);

            esp_timer_start_once(esp_timer_handle, PERIOD_MS * 1'000);
            return;
        }

        const auto time = GetTimeMs();
        switch (state) {
            case States::SetYawAndMotorSpeeds: {
                if (time - smoothing_time_start < SMOOTH_TIME_DURATION_MS) {
                    InterpolateWidths(time);
                } else {
                    if (!IRSensors::IsBallDetected()) {
                        IRSensors::StartPushingBall();
                    }
                    state = States::WaitForBall;
                }

                break;
            }
            case States::WaitForBall: {
                if (IRSensors::IsBallDetected()) {
                    // Launch ball and delay according to configured frequency.
                    state = States::Delay;
                    PWMMotors::LockServo.Animate(LOCK_SERVO_ANIMATION_TIME_MS);
                    ball_launch_time = GetTimeMs();
                } else {
                    if (time - last_keepalive_time > KEEPALIVE_TIMEOUT_MS) {
                        is_session_active = false;
                    }
                }

                break;
            }
            case States::Delay: {
                if (time - ball_launch_time > (Config::GetDelayBetweenLaunchesMs() - LOCK_SERVO_ANIMATION_TIME_MS + 200)) {
                    UpdatePreviousWidths();
                    smoothing_time_start = GetTimeMs();

                    Config::SetNextActiveShot();

                    state = States::SetYawAndMotorSpeeds;
                } else {
                    if (time - last_keepalive_time > KEEPALIVE_TIMEOUT_MS) {
                        is_session_active = false;
                    }
                }
            }
            default: { break; }
        }

        esp_timer_start_once(esp_timer_handle, PERIOD_MS * 1'000);
    }

    static void ProcessPacket(Packet packet) {
        switch (packet.GetID()) {
            case Packet::ID::AddShot: {
                if (is_session_active) {
                    break;
                }

                Config::Shot shot{
                    .yaw_angle = packet.GetValue<int8_t>(0),
                    .bottom_motor = packet.GetValue<uint8_t>(1),
                    .left_motor = packet.GetValue<uint8_t>(2),
                    .right_motor = packet.GetValue<uint8_t>(3)
                };

                Config::AddShot(shot);

                break;
            }
            case Packet::ID::RemoveShot: {
                if (is_session_active) {
                    break;
                }

                Config::RemoveShot(packet.GetValue<uint8_t>(0));
                break;
            }
            case Packet::ID::StartSession: {
                if (is_session_active || !Config::AreAnyShotsConfigured()) {
                    break;
                }

                Config::SetBallsPerMinute(packet.GetValue<uint8_t>(0));

                is_session_active = true;

                UpdatePreviousWidths();
                smoothing_time_start = GetTimeMs();

                state = States::SetYawAndMotorSpeeds;

                last_keepalive_time = GetTimeMs();
                break;
            }
            case Packet::ID::StopSession: {
                if (!is_session_active) {
                    break;
                }

                is_session_active = false;
                break;
            }
            case Packet::ID::Keepalive: {
                last_keepalive_time = GetTimeMs();

                break;
            }
            default: {
                // Should never happen.
                break;
            }
        }
    }
};
