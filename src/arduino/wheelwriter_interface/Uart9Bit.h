// 9-bit PIO UART for the RP2040
// Designed to interface with an Intel 8051 serial port in mode 2
//
// Copyright (c) 2023 John Kua <john@kua.fm>
//
#pragma once

#include "hardware/pio.h"
#include "uart_9bit_tx.pio.h"

class Uart9Bit {
public:
    Uart9Bit() {
        init_ = 0;
    }
    void init(PIO pio, uint sm, uint pin, uint baud) {
        pio_ = pio;
        sm_ = sm;
        offset_ = pio_add_program(pio_, &uart_9bit_tx_program);
        uart_9bit_tx_program_init(pio_, sm_, offset_, pin, baud);
        init_ = 1;
    }
    void write(uint16_t word) {
        if (!init_) return;
        uart_9bit_tx_program_put_word(pio_, sm_, word);
    }
    void write(const uint16_t * buffer, uint count) {
        if (!init_) return;
        uart_9bit_tx_program_put_buffer(pio_, sm_, buffer, count);
    }
private:
    uint init_;
    PIO pio_;
    uint sm_;
    uint offset_;
};
