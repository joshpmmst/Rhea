#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include <driver/uart.h>
#include <esp_log.h>

#include "UART.hpp"
#include "PWMMotors.hpp"
#include "IRSensors.hpp"
#include "StateMachine.hpp"

constexpr const char* TAG = "main";

constexpr auto UART_NUM = UART_NUM_0;
constexpr auto UART_PIN = UART_PIN_NO_CHANGE;

constexpr auto UART_BUFFER_SIZE = 1024;

extern "C"
void app_main(void) {
    UART::Init(StateMachine::ProcessPacket);
    PWMMotors::Init();
    IRSensors::Init();
    StateMachine::Init();

    // Get Ball Pusher to an initial "good" state.
    vTaskDelay(10 / portTICK_PERIOD_MS);
    if (!IRSensors::IsBallDetected()) {
        IRSensors::StartPushingBall();
    }
}
