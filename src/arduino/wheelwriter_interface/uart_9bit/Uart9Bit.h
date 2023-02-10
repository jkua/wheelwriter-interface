// 9-bit PIO UART for the RP2040
// Designed to interface with an Intel 8051 serial port in mode 2
//
// Copyright (c) 2023 John Kua <john@kua.fm>
//
#pragma once

#include "hardware/pio.h"
#include "uart_9bit_tx_inverted.pio.h"
#include "uart_9bit_rx.pio.h"

class Uart9Bit {
public:
    Uart9Bit() {
        init_ = 0;
    }
    void init(PIO pio, uint sm_tx, uint pin_tx, uint sm_rx, uint pin_rx, uint baud) {
        pio_ = pio;

        sm_tx_ = sm_tx;
        offset_tx_ = pio_add_program(pio_, &uart_9bit_tx_inverted_program);
        uart_9bit_tx_inverted_program_init(pio_, sm_tx_, offset_tx_, pin_tx, baud);

        sm_rx_ = sm_rx;
        offset_rx_ = pio_add_program(pio_, &uart_9bit_rx_program);
        uart_9bit_rx_program_init(pio_, sm_rx_, offset_rx_, pin_rx, baud);

        init_ = 1;
    }
    void write(uint16_t word) {
        if (!init_) return;
        uart_9bit_tx_inverted_program_put_word(pio_, sm_tx_, word);
    }
    void write(const uint16_t * buffer, uint count) {
        if (!init_) return;
        uart_9bit_tx_inverted_program_put_buffer(pio_, sm_tx_, buffer, count);
    }
    uint16_t read() {
        if (!init_) return 0xffffffff;
        uint32_t fifo = uart_9bit_rx_program_get_word(pio_, sm_rx_);
        // Bits are coming from the left, so we need to shift right 32-9 bits
        return fifo >> 23;
    }
private:
    uint init_;
    PIO pio_;
    uint sm_tx_;
    uint offset_tx_;
    uint sm_rx_;
    uint offset_rx_;
};
