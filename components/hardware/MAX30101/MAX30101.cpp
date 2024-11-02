/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MAX30101.cpp
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MAX30101.h"
#include "RaftI2CCentral.h"
#include <RaftUtils.h>
#include <ConfigPinMap.h>
#include <RaftJsonIF.h>
#include <driver/gpio.h>

// Module prefix
static const char *MODULE_PREFIX = "MAX30101";

#define DEBUG_HW_SETUP
// #define DEBUG_POLL_RESULT
// #define DEBUG_FIFO_DATA
// #define DEBUG_FIFO_HEX_DATA_DUMP
// #define DEBUG_FIFO_TIME_RED_IR_DUMP

#define WARN_ON_HW_SETUP_FAILURE

#define ONLY_PROCESS_FIFO_IF_FULL

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor and destructor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MAX30101::MAX30101()
{
}

MAX30101::~MAX30101()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MAX30101::setup(const RaftJsonIF& config, RaftI2CCentralIF* pI2C)
{
    // Store I2C
    _pI2C = pI2C;

    // Check I2C valid
    if (!_pI2C)
    {
        LOG_E(MODULE_PREFIX, "setup I2C not valid");
        return;
    }

    // Get address
    _i2cAddr = config.getLong("i2cAddr", MAX30101_DEFAULT_I2C_ADDR);

    // Get red LED intensity (used for HRM)
    _redLedIntensity = config.getLong("redLedIntensity", MAX30101_DEFAULT_RED_LED_INTENSITY);

    // Sample average
    _sampleAverage = config.getLong("sampleAverage", MAX30101_DEFAULT_SAMPLE_AVERAGE);

    // Sample rate
    _sampleRateHz = config.getLong("sampleRateHz", MAX30101_DEFAULT_SAMPLE_RATE_HZ);

    // Pulse width
    _pulseWidthUs = config.getLong("pulseWidthUs", MAX30101_DEFAULT_PULSE_WIDTH);

    // FIFO almost full threshold
    _fifoAlmostFullThreshold = config.getLong("fifoAlmostFullThreshold", MAX30101_DEFAULT_FIFO_A_THRESHOLD);

    // ADC range
    _adcRange = config.getLong("adcRange", MAX30101_DEFAULT_ADC_RANGE);

    // FIFO full pin
    String pinName = config.getString("fifoFullPin", "");
    _fifoFullPin = ConfigPinMap::getPinFromName(pinName.c_str());
    pinMode(_fifoFullPin, INPUT);

    // Wakeup on FIFO full
    _wakeupOnFifoFull = config.getBool("wakeupOnFifoFull", true);

    // Collect HRM samples
    _collectHRM = config.getBool("collectHRM", false);

    // Calculate interval between samples
    _sampleIntervalMs = (uint32_t) (1000 / (1.0 * _sampleRateHz / _sampleAverage));

    // Log
#ifdef DEBUG_HW_SETUP
    LOG_I(MODULE_PREFIX, "setup i2cAddr 0x%02x redLedIntensity %0.1fmA(0x%02x) sampleAverage %d sampleRate %dHz sampleIntervalMs %dms pulseWidth %duS fifoAlmostFullThreshold %d adcRange %d FIFOfullPin %d wakeupOnFifoFull %s collectHRM %s",
                (int)_i2cAddr,
                convRedLedIntensityToMA(_redLedIntensity),
                (int)_redLedIntensity,
                (int)_sampleAverage,
                (int)_sampleRateHz,
                (int)_sampleIntervalMs,
                (int)_pulseWidthUs,
                (int)_fifoAlmostFullThreshold,
                (int)_adcRange,
                (int)_fifoFullPin,
                _wakeupOnFifoFull ? "YES" : "NO",
                _collectHRM ? "YES" : "NO");
#endif

    // Set initialisation time
    _lastInitAttemptTimeMs = millis();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Service
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MAX30101::loop()
{
    // Handle initialisation
    if (!_isInitialised)
    {
        initHardware();
        return;
    }

    // Check if FIFO is full
    if (Raft::isTimeout(millis(), _fifoFullCheckTimeMs, MIN_FIFO_CHECK_FULL_INTERVAL_MS))
    {
        _fifoFullCheckTimeMs = millis();

#ifdef ONLY_PROCESS_FIFO_IF_FULL
        // Check if FIFO is full
        if (isFifoFull())
#endif
        {
            // Read the FIFO read pointer
            std::vector<uint8_t> readBuffer = {0};
            if (!readRegisters(MAX30101_REG_FIFO_RD_PTR, readBuffer))
                return;
            uint32_t readPtr = readBuffer[0];

            // Read the FIFO write pointer
            if (!readRegisters(MAX30101_REG_FIFO_WR_PTR, readBuffer))
                return;
            uint32_t writePtr = readBuffer[0];

            // Calculate number of samples available
            uint32_t numSamples = 0;
            if (writePtr >= readPtr)
                numSamples = writePtr - readPtr;
            else
                numSamples = MAX30101_FIFO_DEPTH - readPtr + writePtr;

            // Read data from the MAX30101 FIFO
            readBuffer.resize(numSamples * MAX30101_BYTES_PER_SAMPLE);
            if (!readRegisters(MAX30101_REG_FIFO_DATA, readBuffer))
                return;

            // Convert the samples to uint16_t
            std::vector<uint16_t> newSamples;
            newSamples.resize(numSamples);
            for (uint32_t i = 0; i < numSamples; i++)
            {
                uint32_t sampleValue = (readBuffer[i * MAX30101_BYTES_PER_SAMPLE] << 8) | 
                             (readBuffer[i * MAX30101_BYTES_PER_SAMPLE + 1] << 8) | 
                             (readBuffer[i * MAX30101_BYTES_PER_SAMPLE + 2]);
                if (sampleValue > 0xffff)
                    sampleValue = 0xffff;
                newSamples[i] = sampleValue;
                SampleData sampleData = {(uint16_t)sampleValue, millis() - _sampleIntervalMs * (numSamples - i - 1)};
                _sampleQueue.put(sampleData);
            }

#ifdef DEBUG_FIFO_TIME_RED_IR_DUMP
            String outStr;
            for (uint32_t i = 0; i < numSamples; i++)
            {
                outStr += String(millis() - _sampleIntervalMs * (numSamples - i - 1)) + "," + String(newSamples[i]) + "\n";
            }
            printf("%s", outStr.c_str());
#endif
            // Debug
            if (_collectHRM)
            {
                String lastSamplesJSON;
                lastSamplesJSON.reserve(numSamples * 10);
                lastSamplesJSON = "{\"s\":[";
                for (uint32_t i = 0; i < numSamples; i++)
                {
                    if (i != 0)
                        lastSamplesJSON += ",";
                    lastSamplesJSON += String(newSamples[i]);
                }
                lastSamplesJSON += "]}";
                _debugLastSamplesJSON = lastSamplesJSON;
            
#ifdef DEBUG_FIFO_DATA
                LOG_I(MODULE_PREFIX, "loop i2cAddr 0x%x sampleRate %.1f/s numSamples %d writePtr %d readPtr %d samples %s", 
                            (int)_i2cAddr, 
                            convSampleRateAndAverageToHz(_sampleRateHz, _sampleAverage),
                            (int)numSamples, (int)writePtr, (int)readPtr,
                            lastSamplesJSON.c_str());
#endif
            }
            else
            {
#ifdef DEBUG_FIFO_DATA
                LOG_I(MODULE_PREFIX, "loop i2cAddr 0x%x sampleRate %.1f/s numSamples %d writePtr %d readPtr %d", 
                            (int)_i2cAddr, 
                            convSampleRateAndAverageToHz(_sampleRateHz, _sampleAverage),
                            (int)numSamples, (int)writePtr, (int)readPtr);
#endif
            }
#ifdef DEBUG_FIFO_HEX_DATA_DUMP
            String outStr;
            Raft::getHexStrFromBytes(readBuffer.data(), readBuffer.size(), outStr);
            char tmpStr[40];
            snprintf(tmpStr, sizeof(tmpStr), "%02x%02x%02x", (int)writePtr, 0, (int)readPtr);
            LOG_I(MODULE_PREFIX, "readData %s%s", tmpStr, outStr.c_str());
#endif
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Init
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MAX30101::initHardware()
{
    // Check if ready for init attempt
    if ((!_pI2C) || (!Raft::isTimeout(millis(), _lastInitAttemptTimeMs, 1000)))
        return;
    _lastInitAttemptTimeMs = millis();

    // Check phase of init
    if (!_lastInitAttemptWasReset)
    {
        // Reset the MAX30101
        if (!writeRegister(MAX30101_REG_MODE_CONFIG, MAX30101_VAL_RESET_HRM))
            return;
        
        // Reset done
        _lastInitAttemptWasReset = true;
        return;
    }

    // If init fails then reset again
    _lastInitAttemptWasReset = false;

    // Read the interrupt registers
    // This is necessary because the interrupt line has a pull-up and drains around 600uA when active
    std::vector<uint8_t> readBuffer = {0, 0};
    if (!readRegisters(MAX30101_REG_INT_STATUS_1, readBuffer))
        return;

    // Set the mode to HRM
    if (!writeRegister(MAX30101_REG_MODE_CONFIG, MAX30101_VAL_MODE_HRM))
        return;

    // Set the Red led intensity
    if (!writeRegister(MAX30101_REG_LED1_PA, _redLedIntensity))
        return;

    // Clear the FIFO registers
    if (!writeRegister(MAX30101_REG_FIFO_WR_PTR, 0))
        return;
    if (!writeRegister(MAX30101_REG_FIFO_RD_PTR, 0))
        return;
    if (!writeRegister(MAX30101_REG_OVF_COUNTER, 0))
        return;

    // Set the sample average, FIFO configuration and FIFO threshold
    if (!writeRegister(MAX30101_REG_FIFO_CONFIG, 
                (convSampleAverageToCode(_sampleAverage) << MAX30101_FIFO_CONFIG_SMP_AVE_BIT_POS) |
                (1 << MAX30101_FIFO_CONFIG_FIFO_ROLLOVER_EN_BIT_POS) |
                (convFifoLevelToCode(_fifoAlmostFullThreshold) << MAX30101_FIFO_CONFIG_FIFO_A_FULL_BIT_POS)))
        return;

    // Set the sample rate, pulse width and ADC range
    if (!writeRegister(MAX30101_REG_SPO2_CONFIG, 
                (convPulseWidthUsToCode(_pulseWidthUs) << MAX30101_SPO2_CONFIG_LED_PW_BIT_POS) |
                (convSampleRateHzToCode(_sampleRateHz) << MAX30101_SPO2_CONFIG_SR_BIT_POS) |
                (convAdcRangeToCode(_adcRange) << MAX30101_SPO2_CONFIG_ADC_RGE_BIT_POS)))
        return;

    // Set the interrupt enable register for FIFO full
    if (!writeRegister(MAX30101_REG_INT_ENABLE_1, _wakeupOnFifoFull ? 1 << MAX30101_INT_EN_1_FIFO_A_FULL_BIT_POS : 0))
        return;

    // If wakeup on FIFO full then set GPIO pin to enable wakeup
    if (_wakeupOnFifoFull)
    {
        gpio_wakeup_enable((gpio_num_t)_fifoFullPin, GPIO_INTR_LOW_LEVEL);
    }

    // Init complete
    _isInitialised = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shutdown
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MAX30101::shutdown()
{
    // Check initialised
    if (!_isInitialised)
        return;

    // Write shutdown code to mode register
    writeRegister(MAX30101_REG_MODE_CONFIG, MAX30101_VAL_SHUTDOWN_HRM);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set MAX30101 register
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MAX30101::writeRegister(uint8_t regNum, uint8_t regVal)
{
    // Write register
    std::vector<uint8_t> writeBuffer = {regNum, regVal};
    uint32_t numWritten = 0;
    RaftRetCode rslt = _pI2C->access(_i2cAddr, writeBuffer.data(), writeBuffer.size(), nullptr, 0, numWritten);
    if (rslt != RAFT_OK)
    {
        LOG_E(MODULE_PREFIX, "writeRegister FAILED i2cAddr 0x%x regNum 0x%x regVal 0x%x", (int)_i2cAddr, (int)regNum, (int)regVal);
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read MAX30101 registers
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MAX30101::readRegisters(uint8_t startRegNum, std::vector<uint8_t>& readBuffer)
{
    // Read register
    std::vector<uint8_t> writeBuffer = {startRegNum};
    uint32_t numRead = 0;
    RaftRetCode rslt = _pI2C->access(_i2cAddr, writeBuffer.data(), writeBuffer.size(), readBuffer.data(), readBuffer.size(), numRead);
    if (rslt != RAFT_OK)
    {
        LOG_E(MODULE_PREFIX, "readRegister FAILED i2cAddr 0x%x startRegNum 0x%x", (int)_i2cAddr, (int)startRegNum);
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if FIFO is full
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MAX30101::isFifoFull()
{
    // Read the FIFO interrupt register
    std::vector<uint8_t> readBuffer = {0};
    if (!readRegisters(MAX30101_REG_INT_STATUS_1, readBuffer))
        return false;

    // Check if FIFO is full
    if (readBuffer[0] & (1 << MAX30101_INT_STATUS_1_FIFO_A_FULL_BIT_POS))
        return true;
    return false;
}
