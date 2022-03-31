/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#include <base-logging/Logging.hpp>

using namespace adc_ads111x_i2c;
using namespace std;

Task::Task(string const& name)
    : TaskBase(name)
{
    _timeout.set(base::Time::fromMilliseconds(100));
}

Task::~Task()
{
}

static constexpr uint8_t CONF_DISABLE_COMPARATOR = 3;
static constexpr uint8_t CONF_SINGLE_SHOT = 1;
static constexpr uint8_t CONF_START_ACQUISITION = 1;


/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Task.hpp for more detailed
// documentation about them.

bool Task::configureHook()
{
    if (! TaskBase::configureHook())
        return false;

    mFD = open(_bus.get().c_str(), O_RDWR);
    if (mFD == -1) {
        LOG_ERROR_S
            << "Cannot open " << _bus.get()
            << " for reading and writing" << endl;
        return false;
    }


    int ret = ioctl(
        mFD, I2C_TIMEOUT,
        static_cast<unsigned long>(_timeout.get().toMilliseconds() / 10)
    );
    if (ret == -1) {
        close(mFD);
        return false;
    }

    mReadings = _readings.get();
    mOutput.resize(mReadings.size());
    return true;
}

static double rangeToScale(Range r) {
    switch(r) {
        case RANGE_0256mV: return 0.256 / (2 << 15);
        case RANGE_0512mV: return 0.512 / (2 << 15);
        case RANGE_1024mV: return 1.024 / (2 << 15);
        case RANGE_2048mV: return 2.048 / (2 << 15);
        case RANGE_4096mV: return 4.096 / (2 << 15);
        case RANGE_6144mV: return 6.144 / (2 << 15);
        default:
            throw std::invalid_argument("invalid Range value");
    }
}

bool Task::startHook()
{
    if (! TaskBase::startHook())
        return false;
    return true;
}
void Task::updateHook()
{
    TaskBase::updateHook();

    for (size_t i = 0; i < mReadings.size(); ++i) {
        if (!configureReading(mReadings[i])) {
            exception(IO_ERROR);
            return;
        }

        // Now busy-wait for the device to finish reading
        while(true) {
            auto r = readRegister(1);
            if (!r.first) {
                return;
            }

            if (r.second & 0x80000) {
                break;
            }
        }

        auto r = readRegister(0);
        if (!r.first) {
            return;
        }

        mOutput[i].time = base::Time::now();
        auto scale = rangeToScale(mReadings[i].range);
        mOutput[i].data = reinterpret_cast<int16_t&>(r.second) * scale;
    }

    _analog_samples.write(mOutput);
}

bool Task::configureReading(Reading const& reading) {
    uint16_t config =
        CONF_START_ACQUISITION << 15 |  reading.input << 12 | reading.rate << 9 |
        CONF_SINGLE_SHOT << 8 | reading.rate << 5 | CONF_DISABLE_COMPARATOR << 0;

    return writeRegister(1, config);
}

bool Task::writeRegister(uint8_t index, uint16_t value) {
    uint8_t buffer[3] = {
        index,
        static_cast<uint8_t>((value & 0xff00) >> 8),
        static_cast<uint8_t>(value & 0xff)
    };
    i2c_msg config_msg;
    config_msg.flags = 0;
    config_msg.addr = _address.get();
    config_msg.len = 3;
    config_msg.buf = buffer;

    i2c_rdwr_ioctl_data query;
    query.msgs = &config_msg;
    query.nmsgs = 1;
    if (ioctl(mFD, I2C_RDWR, &query) == -1) {
        LOG_ERROR_S << strerror(errno) << std::endl;
        exception(IO_ERROR);
        return false;
    }

    return true;
}

pair<bool, uint16_t> Task::readRegister(uint8_t index) {
    i2c_msg msgs[2];

    uint8_t request = index;
    msgs[0].flags = 0;
    msgs[0].addr = _address.get();
    msgs[0].len = 1;
    msgs[0].buf = &request;

    uint8_t result[2];
    msgs[1].flags = I2C_M_RD;
    msgs[1].addr = _address.get();
    msgs[1].len = 2;
    msgs[1].buf = result;

    i2c_rdwr_ioctl_data query;
    query.msgs = msgs;
    query.nmsgs = 2;
    if (ioctl(mFD, I2C_RDWR, &query) == -1) {
        LOG_ERROR_S << strerror(errno) << std::endl;
        exception(IO_ERROR);
        return make_pair(false, 0);
    }

    return make_pair(true, static_cast<uint16_t>(result[0]) << 8 | result[1]);
}

void Task::errorHook()
{
    TaskBase::errorHook();
}
void Task::stopHook()
{
    TaskBase::stopHook();
}
void Task::cleanupHook()
{
    close(mFD);

    TaskBase::cleanupHook();
}
