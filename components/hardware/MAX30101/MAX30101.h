/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MAX30101 Heart Rate Sensor
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <vector>
#include <RaftUtils.h>
#include <ThreadSafeQueue.h>

class RaftI2CCentralIF;
class RaftJsonIF;
class BusRequestResult;

// #define DEBUG_COLLECT_HRM_SAMPLES

class MAX30101
{
public:
    MAX30101();
    virtual ~MAX30101();

    // Setup
    void setup(const RaftJsonIF& config, RaftI2CCentralIF* pBus);

    // Loop
    void loop();

    // Get pulse rate
    float getPulseRate();

    // Get address
    uint32_t getI2CAddr()
    {
        return _i2cAddr;
    }

    // Shutdown
    void shutdown();

    // Check if wakeup on FIFO full enabled
    bool wakeupOnGPIO()
    {
        return _wakeupOnFifoFull;
    }

    // Get sample from queue
    bool getSample(uint16_t& hrmValue, uint32_t& sampleTimeMs)
    {
        SampleData sampleData;
        bool isValid = _sampleQueue.get(sampleData);
        if (isValid)
        {
            hrmValue = sampleData._hrmValue;
            sampleTimeMs = sampleData._sampleTimeMs;
        }
        return isValid;
    }

    // Get num debug samples available
    uint32_t debugAreSamplesAvailable()
    {
        return _debugLastSamplesJSON.length() > 0;
    }

    // Get last samples JSON
    String debugGetLastSamplesJSON()
    {
        String tmpStr = _debugLastSamplesJSON;
        _debugLastSamplesJSON = "";
        return tmpStr;
    }

private:
    // MAX30101 Address and Registers
    static const uint8_t MAX30101_DEFAULT_I2C_ADDR = 0x57;
    static const uint8_t MAX30101_REG_INT_STATUS_1 = 0x00;
    static const uint8_t MAX30101_REG_INT_STATUS_2 = 0x01;
    static const uint8_t MAX30101_REG_INT_ENABLE_1 = 0x02;
    static const uint8_t MAX30101_REG_INT_ENABLE_2 = 0x03;
    static const uint8_t MAX30101_REG_FIFO_WR_PTR = 0x04;
    static const uint8_t MAX30101_REG_OVF_COUNTER = 0x05;
    static const uint8_t MAX30101_REG_FIFO_RD_PTR = 0x06;
    static const uint8_t MAX30101_REG_FIFO_DATA = 0x07;
    static const uint8_t MAX30101_REG_FIFO_CONFIG = 0x08;
    static const uint8_t MAX30101_REG_MODE_CONFIG = 0x09;
    static const uint8_t MAX30101_REG_SPO2_CONFIG = 0x0A;
    static const uint8_t MAX30101_REG_LED1_PA = 0x0C;
    static const uint8_t MAX30101_REG_LED2_PA = 0x0D;
    static const uint8_t MAX30101_REG_LED3_PA = 0x0E;
    static const uint8_t MAX30101_REG_LED4_PA = 0x0F;
    static const uint8_t MAX30101_REG_MULTI_LED_CTRL1 = 0x11;
    static const uint8_t MAX30101_REG_MULTI_LED_CTRL2 = 0x12;
    static const uint8_t MAX30101_REG_TEMP_INTG = 0x1F;
    static const uint8_t MAX30101_REG_TEMP_FRAC = 0x20;
    static const uint8_t MAX30101_REG_TEMP_CONFIG = 0x21;
    static const uint8_t MAX30101_REG_REV_ID = 0xFE;
    static const uint8_t MAX30101_REG_PART_ID = 0xFF;

    // Values for mode register
    static const uint8_t MAX30101_VAL_MODE_HRM = 0x02;
    static const uint8_t MAX30101_VAL_RESET_HRM = 0x40;
    static const uint8_t MAX30101_VAL_SHUTDOWN_HRM = 0x80;

    // Values for sample average
    static const uint8_t MAX30101_REG_SMP_AVE_1 = 0;
    static const uint8_t MAX30101_REG_SMP_AVE_2 = 1;
    static const uint8_t MAX30101_REG_SMP_AVE_4 = 2;
    static const uint8_t MAX30101_REG_SMP_AVE_8 = 3;
    static const uint8_t MAX30101_REG_SMP_AVE_16 = 4;
    static const uint8_t MAX30101_REG_SMP_AVE_32 = 5;

    // Values for pulse width
    static const uint8_t MAX30101_REG_PULSE_WIDTH_69US = 0;
    static const uint8_t MAX30101_REG_PULSE_WIDTH_118US = 1;
    static const uint8_t MAX30101_REG_PULSE_WIDTH_215US = 2;
    static const uint8_t MAX30101_REG_PULSE_WIDTH_411US = 3;

    // Values for sample rate
    static const uint8_t MAX30101_REG_SMP_RATE_50 = 0;
    static const uint8_t MAX30101_REG_SMP_RATE_100 = 1;
    static const uint8_t MAX30101_REG_SMP_RATE_200 = 2;
    static const uint8_t MAX30101_REG_SMP_RATE_400 = 3;
    static const uint8_t MAX30101_REG_SMP_RATE_800 = 4;
    static const uint8_t MAX30101_REG_SMP_RATE_1000 = 5;
    static const uint8_t MAX30101_REG_SMP_RATE_1600 = 6;
    static const uint8_t MAX30101_REG_SMP_RATE_3200 = 7;

    // Values for ADC range
    static const uint8_t MAX30101_REG_ADC_RGE_2048 = 0;
    static const uint8_t MAX30101_REG_ADC_RGE_4096 = 1;
    static const uint8_t MAX30101_REG_ADC_RGE_8192 = 2;
    static const uint8_t MAX30101_REG_ADC_RGE_16384 = 3;

    // Bit positions
    static const uint8_t MAX30101_FIFO_CONFIG_SMP_AVE_BIT_POS = 5;
    static const uint8_t MAX30101_FIFO_CONFIG_FIFO_ROLLOVER_EN_BIT_POS = 4;
    static const uint8_t MAX30101_FIFO_CONFIG_FIFO_A_FULL_BIT_POS = 0;
    static const uint8_t MAX30101_SPO2_CONFIG_LED_PW_BIT_POS = 0;
    static const uint8_t MAX30101_SPO2_CONFIG_SR_BIT_POS = 2;
    static const uint8_t MAX30101_SPO2_CONFIG_ADC_RGE_BIT_POS = 5;
    static const uint8_t MAX30101_INT_EN_1_FIFO_A_FULL_BIT_POS = 7;
    static const uint8_t MAX30101_INT_STATUS_1_FIFO_A_FULL_BIT_POS = 7;
    
    // Bytes to read for various registers
    static const uint32_t MAX30101_BYTES_TO_READ_REV_ID = 2;

    // FIFO dimensions (assumes only 1 LED channel active)
    static const uint32_t MAX30101_FIFO_DEPTH = 32;
    static const uint32_t MAX30101_BYTES_PER_SAMPLE = 3;

    // Default values
    static const uint32_t MAX30101_DEFAULT_RED_LED_INTENSITY = 0x1F;
    static const uint32_t MAX30101_DEFAULT_SAMPLE_AVERAGE = 8;
    static const uint32_t MAX30101_DEFAULT_SAMPLE_RATE_HZ = 100;
    static const uint32_t MAX30101_DEFAULT_FIFO_A_THRESHOLD = 31;
    static const uint32_t MAX30101_DEFAULT_ADC_RANGE = 4096;
    static const uint32_t MAX30101_DEFAULT_PULSE_WIDTH = 411;

    // Convert sample average to code
    uint8_t convSampleAverageToCode(uint32_t sampleAverage)
    {
        switch(sampleAverage)
        {
            case 1: return MAX30101_REG_SMP_AVE_1;
            case 2: return MAX30101_REG_SMP_AVE_2;
            case 4: return MAX30101_REG_SMP_AVE_4;
            case 8: return MAX30101_REG_SMP_AVE_8;
            case 16: return MAX30101_REG_SMP_AVE_16;
            case 32: return MAX30101_REG_SMP_AVE_32;
            default: return MAX30101_REG_SMP_AVE_4;
        }
    }
    
    // Convert sample rate to code
    uint8_t convSampleRateHzToCode(uint32_t sampleRateHz)
    {
        switch(sampleRateHz)
        {
            case 50: return MAX30101_REG_SMP_RATE_50;
            case 100: return MAX30101_REG_SMP_RATE_100;
            case 200: return MAX30101_REG_SMP_RATE_200;
            case 400: return MAX30101_REG_SMP_RATE_400;
            case 800: return MAX30101_REG_SMP_RATE_800;
            case 1000: return MAX30101_REG_SMP_RATE_1000;
            case 1600: return MAX30101_REG_SMP_RATE_1600;
            case 3200: return MAX30101_REG_SMP_RATE_3200;
            default: return MAX30101_REG_SMP_RATE_100;
        }
    }

    // Convert pulse width to code
    uint8_t convPulseWidthUsToCode(uint32_t pulseWidth)
    {
        switch(pulseWidth)
        {
            case 69: return MAX30101_REG_PULSE_WIDTH_69US;
            case 118: return MAX30101_REG_PULSE_WIDTH_118US;
            case 215: return MAX30101_REG_PULSE_WIDTH_215US;
            case 411: return MAX30101_REG_PULSE_WIDTH_411US;
            default: return MAX30101_REG_PULSE_WIDTH_411US;
        }
    }

    // Convert FIFO level to code
    uint8_t convFifoLevelToCode(uint32_t fifoLevel)
    {
        fifoLevel = Raft::clamp(fifoLevel, 17, 32);
        return 32 - fifoLevel;
    }

    // Convert ADC range to code
    uint8_t convAdcRangeToCode(uint32_t adcRange)
    {
        switch(adcRange)
        {
            case 2048: return MAX30101_REG_ADC_RGE_2048;
            case 4096: return MAX30101_REG_ADC_RGE_4096;
            case 8192: return MAX30101_REG_ADC_RGE_8192;
            case 16384: return MAX30101_REG_ADC_RGE_16384;
            default: return MAX30101_REG_ADC_RGE_4096;
        }
    }

    // Convert to mA
    float convRedLedIntensityToMA(uint8_t _redLedIntensity)
    {
        return _redLedIntensity/5.0;
    }

    // Convert sample rate and average to Hertz
    float convSampleRateAndAverageToHz(uint32_t sampleRate, uint32_t sampleAverage)
    {
        if (sampleAverage != 0)
            return sampleRate * 1.0 / sampleAverage;
        return 0;
    }

    // I2C address
    uint32_t _i2cAddr = MAX30101_DEFAULT_I2C_ADDR;

    // Red LED intensity
    uint32_t _redLedIntensity = MAX30101_DEFAULT_RED_LED_INTENSITY;

    // Sample average
    uint32_t _sampleAverage = MAX30101_DEFAULT_SAMPLE_AVERAGE;

    // Sample rate Hz
    uint32_t _sampleRateHz = MAX30101_DEFAULT_SAMPLE_RATE_HZ;

    // Pulse width
    uint32_t _pulseWidthUs = MAX30101_REG_PULSE_WIDTH_411US;

    // FIFO almost full threshold
    uint32_t _fifoAlmostFullThreshold = MAX30101_DEFAULT_FIFO_A_THRESHOLD;

    // ADC range
    uint32_t _adcRange = MAX30101_DEFAULT_ADC_RANGE;

    // FIFO full pin
    int _fifoFullPin = -1;

    // Wakeup on FIFO full
    bool _wakeupOnFifoFull = false;

    // Is init
    bool _isInitialised = false;

    // Time of last init attempt
    uint32_t _lastInitAttemptTimeMs = 0;
    bool _lastInitAttemptWasReset = false;

    // Config send time
    uint32_t _configSendTimeMs = 0;

    // FIFO full check time
    uint32_t _fifoFullCheckTimeMs = 0;
    static const uint32_t MIN_FIFO_CHECK_FULL_INTERVAL_MS = 10;

    // Sample interval
    uint32_t _sampleIntervalMs = 20;

    // I2C Bus
    RaftI2CCentralIF* _pI2C = nullptr;

    // Flags to control logging of HRM samples
    bool _collectHRM = false;

    // Queue of sample data
    class SampleData
    {
    public:
        uint16_t _hrmValue;
        uint32_t _sampleTimeMs;
    };
    ThreadSafeQueue<SampleData> _sampleQueue;

    // Debug store JSON samples for data collection
    String _debugLastSamplesJSON;

    // Debug
    uint32_t _debugLastShowTimeMs = 0;

    // Helpers
    bool writeRegister(uint8_t regNum, uint8_t regVal);
    bool readRegisters(uint8_t startRegNum, std::vector<uint8_t>& readBuffer);
    bool isFifoFull();
    void initHardware();
};
