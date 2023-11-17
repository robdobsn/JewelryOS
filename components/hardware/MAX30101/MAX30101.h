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

class RaftI2CCentralIF;
class ConfigBase;
class BusRequestResult;

#define STORE_SAMPLES_FOR_DATA_COLLECTION

class MAX30101
{
public:
    MAX30101();
    virtual ~MAX30101();

    // Setup
    void setup(ConfigBase& config, const char* pConfigPrefix, RaftI2CCentralIF* pBus);

    // Service
    void service();

    // Get pulse rate
    float getPulseRate();

    // Get address
    uint32_t getI2CAddr()
    {
        return _i2cAddr;
    }

    // Shutdown
    void shutdown();

    // Get last samples JSON
#ifdef STORE_SAMPLES_FOR_DATA_COLLECTION
    String getLastSamplesJSON()
    {
        String tmpStr = _lastSamplesJSON;
        _lastSamplesJSON = "";
        return tmpStr;
    }
#endif

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
    static const uint32_t MAX30101_DEFAULT_SAMPLE_RATE = 100;
    static const uint32_t MAX30101_DEFAULT_FIFO_A_THRESHOLD = 31;
    static const uint32_t MAX30101_DEFAULT_ADC_RANGE = 4096;
    static const uint32_t MAX30101_DEFAULT_PULSE_WIDTH = 411;

    // // MAX30101 Commands
    // static const uint32_t MAX30101_BYTES_TO_READ_ = 2;
    // static const uint32_t MAX30101_CONF_HYSTERESIS_BIT_POS = 2;
    
    // // Default poll rate
    // static const uint32_t MAX30101_DEFAULT_POLL_RATE_PER_SEC = 1000;

    // // Raw range and angle conversion factors
    // static constexpr int32_t MAX30101_RAW_RANGE = 4096;
    // static constexpr float MAX30101_ANGLE_CONVERSION_FACTOR_DEGREES = 360.0 / MAX30101_RAW_RANGE;
    // static constexpr float MAX30101_ANGLE_CONVERSION_FACTOR_RADIANS = 2.0 * 3.14159265358979323846 / MAX30101_RAW_RANGE;

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
    uint8_t convSampleRateToCode(uint32_t sampleRate)
    {
        switch(sampleRate)
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
    uint8_t convPulseWidthToCode(uint32_t pulseWidth)
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

    // Sample rate
    uint32_t _sampleRate = MAX30101_DEFAULT_SAMPLE_RATE;

    // Pulse width
    uint32_t _pulseWidth = MAX30101_REG_PULSE_WIDTH_411US;

    // FIFO almost full threshold
    uint32_t _fifoAlmostFullThreshold = MAX30101_DEFAULT_FIFO_A_THRESHOLD;

    // ADC range
    uint32_t _adcRange = MAX30101_DEFAULT_ADC_RANGE;

    // // Poll rate / sec
    // uint32_t _pollRatePerSec = MAX30101_DEFAULT_POLL_RATE_PER_SEC;

    // // Rotation direction reversed
    // bool _rotationDirectionReversed = false;

    // Is init
    bool _isInitialised = false;

    // Time of last init attempt
    uint32_t _lastInitAttemptTimeMs = 0;
    bool _lastInitAttemptWasReset = false;

    // Config send time
    uint32_t _configSendTimeMs = 0;

    // FIFO full check time
    uint32_t _fifoFullCheckTimeMs = 0;
    static const uint32_t FIFO_CHECK_FULL_INTERVAL_MS = 500;

    // I2C Bus
    RaftI2CCentralIF* _pI2C = nullptr;

    // // Numerical filter
    // AngleMovingAverage<1, MAX30101_RAW_RANGE> _angleFilter;

    // // Sample collector
    // SampleCollector<int32_t>* _pSampleCollector = nullptr;

    // Debug
    uint32_t _debugLastShowTimeMs = 0;

    // // Callbacks
    // static void pollResultCallbackStatic(void* pCallbackData, BusRequestResult& reqResult);
    // void pollResultCallback(BusRequestResult& reqResult);
    // static void getTemperatureResultCallbackStatic(void* pCallbackData, BusRequestResult& reqResult);
    // void getTemperatureResultCallback(BusRequestResult& reqResult);
    // static void getHwRevCallbackStatic(void* pCallbackData, BusRequestResult& reqResult);
    // void getHwRevCallback(BusRequestResult& reqResult);

    // Store samples for data collection
#ifdef STORE_SAMPLES_FOR_DATA_COLLECTION
    String _lastSamplesJSON;
#endif

    // Helpers
    bool writeRegister(uint8_t regNum, uint8_t regVal);
    bool readRegisters(uint8_t startRegNum, std::vector<uint8_t>& readBuffer);
    bool isFifoFull();
    void initHardware();
};
