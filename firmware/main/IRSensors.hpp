#pragma once

#include <stdint.h>

#include <esp_adc/adc_oneshot.h>
#include <esp_timer.h>
#include <esp_log.h>

#include "PWMMotors.hpp"

class IRSensors {
private:
    static constexpr const char* TAG = "IRSensors";

    static inline adc_oneshot_unit_handle_t adc_handle;

    static inline esp_timer_handle_t esp_timer_handle;
    static constexpr uint64_t PERIOD_MS = 10;

    // ADC1_0 - GPIO36 - Ball presence detector
    // ADC1_3 - GPIO39 - Pusher encoder
    static constexpr auto ADC_UNIT = ADC_UNIT_1;
    static constexpr auto BALL_PRESENCE_CHANNEL = ADC_CHANNEL_0;
    static constexpr auto PUSHER_ENCODER_CHANNEL = ADC_CHANNEL_3;

    static constexpr int BALL_PRESENT_THRESHOLD = 3'200;
    static constexpr int PUSHER_ENCODER_THRESHOLD = 3'150;
    static constexpr int PUSHER_ENCODER_HYSTERISIS = 100;

    static inline bool is_ball_present = false;

    enum class BallPushStates {
        NotPushing,
        WaitingForFirstSlot,
        WaitingForFirstObscured,
        WaitingForSecondSlot,
    };

    static inline BallPushStates ball_push_state = BallPushStates::NotPushing;

    static void ReadADCTick(void* unused) {
        int ball_presence_reading, encoder_reading;
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, BALL_PRESENCE_CHANNEL, &ball_presence_reading));
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, PUSHER_ENCODER_CHANNEL, &encoder_reading));

        is_ball_present = ball_presence_reading < BALL_PRESENT_THRESHOLD;

        switch (ball_push_state) {
            case BallPushStates::NotPushing: {
                break;
            }
            case BallPushStates::WaitingForFirstSlot: {
                if (encoder_reading > (PUSHER_ENCODER_THRESHOLD + PUSHER_ENCODER_HYSTERISIS)) {
                    ESP_LOGI(TAG, "WaitingForFirstSlot -> WaitingForFirstObscured");
                    ball_push_state = BallPushStates::WaitingForFirstObscured;
                }
                break;
            }
            case BallPushStates::WaitingForFirstObscured: {
                if (encoder_reading < (PUSHER_ENCODER_THRESHOLD - PUSHER_ENCODER_HYSTERISIS)) {
                    ESP_LOGI(TAG, "WaitingForFirstObscured -> WaitingForSecondSlot");
                    ball_push_state = BallPushStates::WaitingForSecondSlot;
                }
                break;
            }
            case BallPushStates::WaitingForSecondSlot: {
                if (encoder_reading > (PUSHER_ENCODER_THRESHOLD + PUSHER_ENCODER_HYSTERISIS)) {
                    ESP_LOGI(TAG, "WaitingForSecondSlot -> NotPushing");
                    ball_push_state = BallPushStates::NotPushing;
                    PWMMotors::PusherServo.Pause();
                }
                break;
            }
            default: { break; }
        }

        esp_timer_start_once(esp_timer_handle, PERIOD_MS * 1'000);
    }

public:
    static void Init() {
        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = ADC_UNIT,
        };
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

        adc_oneshot_chan_cfg_t config{};
        config.bitwidth = ADC_BITWIDTH_12;
        config.atten = ADC_ATTEN_DB_12;

        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, BALL_PRESENCE_CHANNEL, &config));
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, PUSHER_ENCODER_CHANNEL, &config));

        const esp_timer_create_args_t timer_args = {
            .callback = &ReadADCTick,
            .arg = (void*)esp_timer_handle,
            .name = "read_adc"
        };
        esp_timer_create(&timer_args, &esp_timer_handle);
        esp_timer_start_once(esp_timer_handle, PERIOD_MS * 1'000);
    }

    static bool IsBallDetected() {
        return is_ball_present;
    }

    static void StartPushingBall() {
        PWMMotors::PusherServo.UpdateWidth(1300);
        PWMMotors::PusherServo.Resume();

        ball_push_state = BallPushStates::WaitingForFirstSlot;
    }
};
