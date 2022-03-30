#ifndef adc_ads111x_i2c_TYPES_HPP
#define adc_ads111x_i2c_TYPES_HPP

/* If you need to define types specific to your oroGen components, define them
 * here. Required headers must be included explicitly
 *
 * However, it is common that you will only import types from your library, in
 * which case you do not need this file
 */

namespace adc_ads111x_i2c {
    /** Input configuration
     *
     * X_TO_Y means that the positive side is Y and the negative X, i.e.
     * AIN0_TO_AIN3 returns the voltage from the (-) side AIN0 TO the (+) side
     * AIN3
     */
    enum Input {
        INPUT_AIN1_TO_AIN0 = 0,
        INPUT_AIN3_TO_AIN0 = 1,
        INPUT_AIN3_TO_AIN1 = 2,
        INPUT_AIN3_TO_AIN2 = 3,
        INPUT_GND_TO_AIN0  = 4,
        INPUT_GND_TO_AIN1  = 5,
        INPUT_GND_TO_AIN2  = 6,
        INPUT_GND_TO_AIN3  = 7
    };

    enum Range {
        RANGE_6144mV,
        RANGE_4096mV,
        RANGE_2048mV,
        RANGE_1024mV,
        RANGE_0512mV,
        RANGE_0256mV,
    };

    enum Rate {
        RATE_8HZ,
        RATE_16HZ,
        RATE_32HZ,
        RATE_64HZ,
        RATE_128HZ,
        RATE_250HZ,
        RATE_475HZ,
        RATE_860HZ
    };
}

#endif

