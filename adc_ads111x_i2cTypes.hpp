#ifndef adc_ads111x_i2c_TYPES_HPP
#define adc_ads111x_i2c_TYPES_HPP

#include <stdint.h>

namespace adc_ads111x_i2c {
    /** Input configuration
     *
     * X_TO_Y means that the positive side is Y and the negative X, i.e.
     * AIN0_TO_AIN3 returns the voltage from the (-) side AIN0 TO the (+) side
     * AIN3
     */
    enum Input {
        INPUT_AIN1_TO_AIN0 = 0x0,
        INPUT_AIN3_TO_AIN0 = 0x1,
        INPUT_AIN3_TO_AIN1 = 0x2,
        INPUT_AIN3_TO_AIN2 = 0x3,
        INPUT_GND_TO_AIN0  = 0x4,
        INPUT_GND_TO_AIN1  = 0x5,
        INPUT_GND_TO_AIN2  = 0x6,
        INPUT_GND_TO_AIN3  = 0x7
    };

    enum Range {
        RANGE_6144mV = 0x0,
        RANGE_4096mV = 0x1,
        RANGE_2048mV = 0x2,
        RANGE_1024mV = 0x3,
        RANGE_0512mV = 0x4,
        RANGE_0256mV = 0x5
    };

    enum Rate {
        RATE_8HZ  = 0x0,
        RATE_16HZ = 0x1,
        RATE_32HZ = 0x2,
        RATE_64HZ = 0x3,
        RATE_128HZ = 0x4,
        RATE_250HZ = 0x5,
        RATE_475HZ = 0x6,
        RATE_860HZ = 0x7
    };

    struct Reading {
        Input input;
        Range range;
        Rate rate;

        /** How many times in a row the component can have read errors on this
         * input until it bails out with IO_ERROR
         */
        uint8_t acceptable_errors = 0;
    };
}

#endif

