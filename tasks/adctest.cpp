/** Standalone test program
 */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <cstdint>
#include <iostream>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <chrono>

using namespace std;

static constexpr uint8_t CONF_DISABLE_COMPARATOR = 3;
static constexpr uint8_t CONF_SINGLE_SHOT = 1;
static constexpr uint8_t CONF_START_ACQUISITION = 1;
static constexpr int ADDRESS = 0x48;
static constexpr int SAMPLE_COUNT = 100;

static const string BUS = "/dev/i2c-2";

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
    RANGE_0256mV = 0x5,
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
    Range range = RANGE_6144mV;
    Rate rate = RATE_64HZ;
};

bool configureReading(int fd, Reading const& reading);
bool writeRegister(int fd, uint8_t index, uint16_t value) ;
pair<bool, uint16_t> readRegister(int fd, uint8_t index);
static double rangeToScale(Range r) {
    switch(r) {
        case RANGE_0256mV: return 0.256 / (1 << 15);
        case RANGE_0512mV: return 0.512 / (1 << 15);
        case RANGE_1024mV: return 1.024 / (1 << 15);
        case RANGE_2048mV: return 2.048 / (1 << 15);
        case RANGE_4096mV: return 4.096 / (1 << 15);
        case RANGE_6144mV: return 6.144 / (1 << 15);
        default:
            throw std::invalid_argument("invalid Range value");
    }
}

int main() {
    int fd = open(BUS.c_str(), O_RDWR);
    if (fd == -1) {
        cerr
            << "Cannot open " << BUS
            << " for reading and writing" << endl;
        return 1;
    }

    int ret = ioctl(fd, I2C_TIMEOUT, 10);
    if (ret == -1) {
	cerr << "timeout ioctl failed" << endl;
        close(fd);
        return 1;
    }

    auto start = chrono::steady_clock::now();
    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        Reading reading = { INPUT_GND_TO_AIN0, RANGE_6144mV, RATE_64HZ };

        if (!configureReading(fd, reading)) {
            cerr << "failed to configure" << endl;
            return 1;
        }

	// Sleep the required amount of time
	usleep(12000);

        // Now busy-wait for the device to finish reading
        while(true) {
            auto r = readRegister(fd, 1);
            if (!r.first) {
                cerr << "failed busy wait" << endl;
                return 1;
            }

            if (r.second & 0x8000) {
                break;
            }
        }

        auto r = readRegister(fd, 0);
        if (!r.first) {
            cerr << "failed to read value" << endl;
            return 1;
        }

        //std::cout << r.second << std::endl;
        auto scale = rangeToScale(reading.range);
        //std::cout << reinterpret_cast<int16_t&>(r.second) * scale << std::endl;
    }
    chrono::duration<double> diff = chrono::steady_clock::now() - start;
    cout << SAMPLE_COUNT << " samples in " << diff.count() << " seconds" << endl;
    cout << SAMPLE_COUNT/diff.count() << " samples per second" << endl;

    return 0;
}


bool configureReading(int fd, Reading const& reading) {
    uint16_t config =
        CONF_START_ACQUISITION << 15 |  reading.input << 12 | reading.range << 9 |
        CONF_SINGLE_SHOT << 8 | reading.rate << 5 | CONF_DISABLE_COMPARATOR << 0;

    return writeRegister(fd, 1, config);
}

bool writeRegister(int fd, uint8_t index, uint16_t value) {
    uint8_t buffer[3] = {
        index,
        static_cast<uint8_t>((value & 0xff00) >> 8),
        static_cast<uint8_t>(value & 0xff)
    };
    i2c_msg config_msg;
    config_msg.flags = 0;
    config_msg.addr = ADDRESS;
    config_msg.len = 3;
    config_msg.buf = buffer;

    i2c_rdwr_ioctl_data query;
    query.msgs = &config_msg;
    query.nmsgs = 1;
    if (ioctl(fd, I2C_RDWR, &query) == -1) {
        cerr << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

pair<bool, uint16_t> readRegister(int fd, uint8_t index) {
    i2c_msg msgs[2];

    uint8_t request = index;
    msgs[0].flags = 0;
    msgs[0].addr = ADDRESS;
    msgs[0].len = 1;
    msgs[0].buf = &request;

    uint8_t result[2];
    msgs[1].flags = I2C_M_RD;
    msgs[1].addr = ADDRESS;
    msgs[1].len = 2;
    msgs[1].buf = result;

    i2c_rdwr_ioctl_data query;
    query.msgs = msgs;
    query.nmsgs = 2;
    if (ioctl(fd, I2C_RDWR, &query) == -1) {
        cerr << strerror(errno) << std::endl;
        return make_pair(false, 0);
    }

    return make_pair(true, static_cast<uint16_t>(result[0]) << 8 | result[1]);
}

