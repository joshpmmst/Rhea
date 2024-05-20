#pragma once

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include <driver/ledc.h>
#include <esp_log.h>

template<
ledc_timer_t LEDC_TIMER,
ledc_channel_t LEDC_CHANNEL,
gpio_num_t GPIO_NUM,
uint32_t MIN_WIDTH,
uint32_t MAX_WIDTH,
uint32_t INITIAL_WIDTH
> struct PWMMotor {
    static constexpr auto LEDC_MODE = LEDC_LOW_SPEED_MODE;
    static constexpr auto LEDC_DUTY_RES = LEDC_TIMER_14_BIT;
    static constexpr uint32_t LEDC_FREQUENCY = 50;

    static uint32_t USToDuty(const uint32_t us) {
        // 20 000 us per period, 14 bit resolution
        return (uint32_t)(us / 20'000.0f * (2 << (LEDC_TIMER_14_BIT - 1)));
    }

    uint32_t width = 0;
    bool paused = false;

    void Init() {
        width = INITIAL_WIDTH;

        ledc_timer_config_t ledc_timer{};
        ledc_timer.speed_mode       = LEDC_MODE;
        ledc_timer.timer_num        = LEDC_TIMER;
        ledc_timer.duty_resolution  = LEDC_DUTY_RES;
        ledc_timer.freq_hz          = LEDC_FREQUENCY;
        ledc_timer.clk_cfg          = LEDC_AUTO_CLK;
        ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

        ledc_channel_config_t ledc_channel{};
        ledc_channel.speed_mode     = LEDC_MODE;
        ledc_channel.channel        = LEDC_CHANNEL;
        ledc_channel.timer_sel      = LEDC_TIMER;
        ledc_channel.intr_type      = LEDC_INTR_DISABLE;
        ledc_channel.gpio_num       = GPIO_NUM;
        ledc_channel.duty           = USToDuty(width);
        ledc_channel.hpoint         = 0;
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    }

    void UpdateWidth(uint32_t new_width, const bool override = false) {
        if (!override && new_width == width) {
            return;
        }

        width = new_width > MAX_WIDTH ? MAX_WIDTH : (new_width < MIN_WIDTH ? MIN_WIDTH : new_width);

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, USToDuty(width)));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
    }

    // Animate the duty cycle from MIN_WIDTH to MAX_WIDTH.
    void Animate(uint32_t ms) {
        ledc_set_fade_with_time(
            LEDC_MODE,
            LEDC_CHANNEL,
            USToDuty(MAX_WIDTH),
            ms
        );

        ledc_fade_start(
            LEDC_MODE,
            LEDC_CHANNEL,
            LEDC_FADE_NO_WAIT
        );
    }

    void Pause() {
        ESP_ERROR_CHECK(ledc_timer_pause(LEDC_MODE, LEDC_TIMER));
        paused = true;
    }

    void Resume() {
        ESP_ERROR_CHECK(ledc_timer_resume(LEDC_MODE, LEDC_TIMER));
        paused = false;
    }
};

namespace PWMMotors {
    PWMMotor<LEDC_TIMER_0, LEDC_CHANNEL_0, GPIO_NUM_25, 1000, 1150, 1000> BottomBLDC;
    PWMMotor<LEDC_TIMER_0, LEDC_CHANNEL_1, GPIO_NUM_26, 1000, 1150, 1000> LeftBLDC;
    PWMMotor<LEDC_TIMER_0, LEDC_CHANNEL_2, GPIO_NUM_27, 1000, 1150, 1000> RightBLDC;

    PWMMotor<LEDC_TIMER_1, LEDC_CHANNEL_3, GPIO_NUM_32, 1000, 2000, 1000> LockServo;
    PWMMotor<LEDC_TIMER_2, LEDC_CHANNEL_4, GPIO_NUM_33, 1000, 2000, 1500> YawServo;

    PWMMotor<LEDC_TIMER_3, LEDC_CHANNEL_5, GPIO_NUM_14, 1000, 2000, 1300> PusherServo;

    IRAM_ATTR bool cb_ledc_fade_end_event(const ledc_cb_param_t *param, void *user_arg) {
        BaseType_t taskAwoken = pdFALSE;

        if (param->event == LEDC_FADE_END_EVT) {
            SemaphoreHandle_t counting_sem = (SemaphoreHandle_t)user_arg;
            xSemaphoreGiveFromISR(counting_sem, &taskAwoken);
        }

        return (taskAwoken == pdTRUE);
    }

    void lock_reset_task(void* unused) {
        SemaphoreHandle_t lock_reset_sem = xSemaphoreCreateCounting(1, 0);

        ledc_fade_func_install(0);
        ledc_cbs_t callbacks = {
            .fade_cb = cb_ledc_fade_end_event
        };
        ledc_cb_register(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, &callbacks, (void*)lock_reset_sem);

        while (true) {
            // Returns true if count is > 0.
            if (xSemaphoreTake(lock_reset_sem, portMAX_DELAY) == pdTRUE) {
               LockServo.UpdateWidth(1000, true);
            }

            vTaskDelay(pdMS_TO_TICKS(20));
        }

        vTaskDelete(NULL);
    }

    void Init() {
        BottomBLDC.Init();
        LeftBLDC.Init();
        RightBLDC.Init();

        LockServo.Init();
        YawServo.Init();

        PusherServo.Init();

        xTaskCreatePinnedToCore(lock_reset_task, "lock_reset_task", 2048, NULL, 12, NULL, 0);
    }
}
