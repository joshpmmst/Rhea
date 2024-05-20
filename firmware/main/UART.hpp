#pragma once

#include <stdint.h>

#include <esp_log.h>
#include <esp_timer.h>
#include <driver/uart.h>

#include "Packets.hpp"

class UART {
private:
    static constexpr const char* TAG = "UART";
    static constexpr auto UART_NUM = UART_NUM_0;
    static constexpr auto UART_PIN = UART_PIN_NO_CHANGE;

    static constexpr auto UART_BUFFER_SIZE = 1024;

    static inline uint8_t rx_buffer[Packet::PACKET_LENGTH] = {0};
    static inline uint8_t rx_buffer_pos = 0;
    static inline bool waiting_for_header = true;

    static inline esp_timer_handle_t esp_timer_handle;
    static constexpr uint64_t PERIOD_MS = 10;

    using PacketCallbackFn = void(*)(Packet packet);
    static inline PacketCallbackFn packet_callback = nullptr;

public:
    static void Init(PacketCallbackFn callback) {
        ESP_LOGI(TAG, "Init()");

        uart_config_t uart_config = {
            .baud_rate = 115'200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        };

        uart_driver_install(UART_NUM, UART_BUFFER_SIZE * 2, UART_BUFFER_SIZE * 2, 20, nullptr, 0);
        uart_param_config(UART_NUM, &uart_config);
        uart_set_pin(UART_NUM, UART_PIN, UART_PIN, UART_PIN, UART_PIN);

        packet_callback = callback;

        const esp_timer_create_args_t timer_args = {
            .callback = &RxThread,
            .arg = (void*)esp_timer_handle,
            .name = "uart_rx"
        };
        esp_timer_create(&timer_args, &esp_timer_handle);
        esp_timer_start_once(esp_timer_handle, PERIOD_MS * 1'000);
    }

private:
    static void RxThread(void* unused) {
        uint8_t byte;
        while (true) {
            int num_read = uart_read_bytes(UART_NUM, &byte, 1, 1 / portTICK_PERIOD_MS);
            if (!(num_read > 0)) {
                break;
            }

            // Check for overflow.
            if (rx_buffer_pos > sizeof(rx_buffer)) {
                rx_buffer_pos = 0;
            }

            if (waiting_for_header) {
                if (byte == Packet::HEADER) {
                    rx_buffer[rx_buffer_pos++] = byte;
                    waiting_for_header = false;
                }
            } else {
                rx_buffer[rx_buffer_pos++] = byte;
                if (rx_buffer_pos == sizeof(rx_buffer)) {
                    if (IsCurrentBufferValid()) {
                        Packet p{.data = rx_buffer};
                        packet_callback(p);

                        rx_buffer_pos = 0;
                        waiting_for_header = true;
                    } else {
                        int8_t next_header_pos = -1;
                        for (auto i = 1; i < sizeof(rx_buffer); ++i) {
                            if (rx_buffer[i] == Packet::HEADER) {
                                next_header_pos = i;
                                break;
                            }
                        }

                        if (next_header_pos == -1) {
                            // No new header found within current data.
                            rx_buffer_pos = 0;
                            waiting_for_header = true;
                        } else {
                            // New header was found within packet's length.
                            // Shift it and all the data after it to the start.
                            for (auto i = 0; i <= sizeof(rx_buffer) - next_header_pos; i++) {
                                rx_buffer[i] = rx_buffer[next_header_pos + i];
                            }
                            rx_buffer_pos -= next_header_pos;
                        }
                    }
                }
            }
        }
        
        esp_timer_start_once(esp_timer_handle, PERIOD_MS * 1'000);
    }

    static bool IsCurrentBufferValid() {
        return
            rx_buffer[0] == Packet::HEADER
            && (rx_buffer[1] < (uint8_t)Packet::ID::Max)
            && rx_buffer[sizeof(rx_buffer) - 1] == Packet::FOOTER;
    }
};
