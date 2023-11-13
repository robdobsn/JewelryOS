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
#include <ConfigBase.h>

// Module prefix
static const char *MODULE_PREFIX = "MAX30101";

// #define DEBUG_POLL_RESULT
#define DEBUG_HW_SETUP
#define WARN_ON_HW_SETUP_FAILURE

#define HANDLE_FIFO_FULL_ON_SERVICE_LOOP

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

void MAX30101::setup(ConfigBase& config, const char* pConfigPrefix, RaftI2CCentralIF* pI2C)
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
    _i2cAddr = config.getLong("i2cAddr", MAX30101_DEFAULT_I2C_ADDR, pConfigPrefix);

    // Get red LED intensity (used for HRM)
    _redLedIntensity = config.getLong("redLedIntensity", MAX30101_DEFAULT_RED_LED_INTENSITY, pConfigPrefix);

    // Sample average
    _sampleAverage = config.getLong("sampleAverage", MAX30101_DEFAULT_SAMPLE_AVERAGE, pConfigPrefix);

    // Sample rate
    _sampleRate = config.getLong("sampleRate", MAX30101_DEFAULT_SAMPLE_RATE, pConfigPrefix);

    // Pulse width
    _pulseWidth = config.getLong("pulseWidth", MAX30101_DEFAULT_PULSE_WIDTH, pConfigPrefix);

    // FIFO almost full threshold
    _fifoAlmostFullThreshold = config.getLong("fifoAlmostFullThreshold", MAX30101_DEFAULT_FIFO_A_THRESHOLD, pConfigPrefix);

    // ADC range
    _adcRange = config.getLong("adcRange", MAX30101_DEFAULT_ADC_RANGE, pConfigPrefix);

    // Log
#ifdef DEBUG_HW_SETUP
    LOG_I(MODULE_PREFIX, "setup i2cAddr 0x%02x redLedIntensity %0.1fmA(0x%02x) sampleAverage %d sampleRate %d pulseWidth %d fifoAlmostFullThreshold %d adcRange %d",
                (int)_i2cAddr,
                convRedLedIntensityToMA(_redLedIntensity),
                (int)_redLedIntensity,
                (int)_sampleAverage,
                (int)_sampleRate,
                (int)_pulseWidth,
                (int)_fifoAlmostFullThreshold,
                (int)_adcRange);
#endif

    // Set initialisation time
    _lastInitAttemptTimeMs = millis();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set MAX30101 register
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MAX30101::writeRegister(uint8_t regNum, uint8_t regVal)
{
    // Write register
    std::vector<uint8_t> writeBuffer = {regNum, regVal};
    uint32_t numWritten = 0;
    RaftI2CCentral::AccessResultCode rslt = _pI2C->access(_i2cAddr, writeBuffer.data(), writeBuffer.size(), nullptr, 0, numWritten);
    if (rslt != RaftI2CCentral::ACCESS_RESULT_OK)
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
    RaftI2CCentral::AccessResultCode rslt = _pI2C->access(_i2cAddr, writeBuffer.data(), writeBuffer.size(), readBuffer.data(), readBuffer.size(), numRead);
    if (rslt != RaftI2CCentral::ACCESS_RESULT_OK)
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Service
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MAX30101::service()
{
    // Check if valid
    if (_pI2C == nullptr)
        return;

    // Handle initialisation
    if (!_isInitialised)
    {
        initHardware();
        return;
    }

#ifdef HANDLE_FIFO_FULL_ON_SERVICE_LOOP
    if (Raft::isTimeout(millis(), _fifoFullCheckTimeMs, FIFO_CHECK_FULL_INTERVAL_MS))
    {
        _fifoFullCheckTimeMs = millis();
        // Check if FIFO is full
        // if (isFifoFull())
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

            // Convert the samples to uint32_t
            std::vector<uint32_t> samples;
            samples.resize(numSamples);
            for (uint32_t i = 0; i < numSamples; i++)
            {
                samples[i] = (readBuffer[i * MAX30101_BYTES_PER_SAMPLE] << 8) | 
                             (readBuffer[i * MAX30101_BYTES_PER_SAMPLE + 1] << 8) | 
                             (readBuffer[i * MAX30101_BYTES_PER_SAMPLE + 2]);
            }

            // Output data
            String debugStr;
            debugStr.reserve(200);
            for (uint32_t i = 0; i < numSamples; i++)
            {
                debugStr += String(samples[i]) + " ";
            }
            LOG_I(MODULE_PREFIX, "service i2cAddr 0x%x sampleRate %.1f/s numSamples %d writePtr %d readPtr %d samples %s", 
                        (int)_i2cAddr, 
                        convSampleRateAndAverageToHz(_sampleRate, _sampleAverage),
                        (int)numSamples, (int)writePtr, (int)readPtr,
                        debugStr.c_str());
        }
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Init
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MAX30101::initHardware()
{
    // Check if ready for init attempt
    if (!Raft::isTimeout(millis(), _lastInitAttemptTimeMs, 1000))
        return;

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
                (convPulseWidthToCode(_pulseWidth) << MAX30101_SPO2_CONFIG_LED_PW_BIT_POS) |
                (convSampleRateToCode(_sampleRate) << MAX30101_SPO2_CONFIG_SR_BIT_POS) |
                (convAdcRangeToCode(_adcRange) << MAX30101_SPO2_CONFIG_ADC_RGE_BIT_POS)))
        return;

    // Set the interrupt enable register for FIFO full
    if (!writeRegister(MAX30101_REG_INT_ENABLE_1, 1 << MAX30101_INT_EN_1_FIFO_A_FULL_BIT_POS))
        return;

    // Init complete
    _isInitialised = true;

    // // Command to get chip version data
    // const HWElemReq GetChipVersion = {{MAX30101_REV_ID}, MAX30101_BYTES_TO_READ_FOR_VERSION, HWElemReq::UNNUM, "Version", 0};

    // // Setup polling
    // BusRequestInfo busRequestInfo("MAX30101", _i2cAddr);
    // busRequestInfo.set(BUS_REQ_TYPE_POLL, AS5600GetDataCommand, _pollRatePerSec, pollResultCallbackStatic, this);
    // _pBus->addRequest(busRequestInfo);
    // _isInitialised = true;

    // LOG_I(MODULE_PREFIX, "service i2cAddr %x pollRatePerSec %d", (int)_i2cAddr, (int)_pollRatePerSec);

    // Send setup command
    // if ((_configSendTimeMs != 0) && (Raft::isTimeout(millis(), _configSendTimeMs, 100)))
    // {
        // // Command to get AS5600 data
        // const HWElemReq AS5600GetDataCommand = {{MAX30101_ROTATION_REG_NUMBER}, MAX30101_BYTES_TO_READ_FOR_ROTATION, HWElemReq::UNNUM, "Rotation", 0};

        // // Setup polling
        // BusRequestInfo busRequestInfo("AS5600", _i2cAddr);
        // busRequestInfo.set(BUS_REQ_TYPE_POLL, AS5600GetDataCommand, _pollRatePerSec, pollResultCallbackStatic, this);
        // _pBus->addRequest(busRequestInfo);
        // _isInitialised = true;
    // }
    // else
    // {
        // uint16_t confRegContents = 3 << MAX30101_CONF_HYSTERESIS_BIT_POS;
        // const HWElemReq confRegContentsCommand = {{MAX30101_CONF_REG_NUMBER, (uint8_t)(confRegContents & 0xff), uint8_t((confRegContents >> 8) & 0xff)}, 0, HWElemReq::UNNUM, "Setup", 0};
        // BusRequestInfo busRequestInfo("AS5600", _i2cAddr);
        // busRequestInfo.set(BUS_REQ_TYPE_STD, confRegContentsCommand, 0, getTemperatureResultCallbackStatic, this);
        // _pBus->addRequest(busRequestInfo);
        // _configSendTimeMs = millis();

        // const HWElemReq hwRevContentsCommand = {{MAX30101_REV_ID}, MAX30101_BYTES_TO_READ_REV_ID, HWElemReq::UNNUM, "HwRev", 0};
        // BusRequestInfo busRequestInfo("MAX30101HwRev", _i2cAddr);
        // busRequestInfo.set(BUS_REQ_TYPE_STD, hwRevContentsCommand, 0, getHwRevCallbackStatic, this);
        // _pBus->addRequest(busRequestInfo);
        // _configSendTimeMs = millis();



    // }

//     // Initialize the device
//     std::vector<uint8_t> writeBuffer = {MAX30101_REG_REV_ID};
//     std::vector<uint8_t> readBuffer = {0, 0};
//     uint32_t numRead = 0;
//     RaftI2CCentral::AccessResultCode rslt = _pI2C->access(_i2cAddr, writeBuffer.data(), writeBuffer.size(), readBuffer.data(), readBuffer.size(), numRead);
//     if (rslt == RaftI2CCentral::ACCESS_RESULT_OK)
//     {
// #ifdef DEBUG_HW_SETUP
//         LOG_I(MODULE_PREFIX, "setup OK i2cAddr 0x%02x revId 0x%02x partId 0x%02x", (int)_i2cAddr, (int)readBuffer[0], (int)readBuffer[1]);
// #endif

// //         // Reset the hardware
// //         writeBuffer = {MAX30101_REG_MODE_CONFIG, MAX30101_VAL_RESET_HRM};
// //         rslt = _pI2C->access(_i2cAddr, writeBuffer.data(), writeBuffer.size(), nullptr, 0, numRead);
// //         if (rslt == RaftI2CCentral::ACCESS_RESULT_OK)
// //         {
// // #ifdef DEBUG_HW_SETUP
// //             LOG_I(MODULE_PREFIX, "setup OK reset i2cAddr 0x%02x", (int)_i2cAddr);
// // #endif
// //         }
// //         else
// //         {
// // #ifdef WARN_ON_HW_SETUP_FAILURE
// //             LOG_E(MODULE_PREFIX, "setup FAILED reset i2cAddr 0x%02x", (int)_i2cAddr);
// // #endif
// //         }

//         // // Wait a bit for reset
//         // delay(200);

//         // Read the mode register
//         writeBuffer = {MAX30101_REG_MODE_CONFIG};
//         readBuffer = {0};
//         rslt = _pI2C->access(_i2cAddr, writeBuffer.data(), writeBuffer.size(), readBuffer.data(), readBuffer.size(), numRead);
//         if (rslt == RaftI2CCentral::ACCESS_RESULT_OK)
//         {
// #ifdef DEBUG_HW_SETUP
//             LOG_I(MODULE_PREFIX, "setup OK read mode i2cAddr 0x%02x mode 0x%02x", (int)_i2cAddr, (int)readBuffer[0]);
// #endif
//         }
//         else
//         {
// #ifdef WARN_ON_HW_SETUP_FAILURE
//             LOG_E(MODULE_PREFIX, "setup FAILED read mode i2cAddr 0x%02x", (int)_i2cAddr);
// #endif
//         }

//         // Read from the interrupt registers to clear any interrupts
//         // This is necessary because the interrupt line has a pull-up and drains around 600uA when active
//         writeBuffer = {MAX30101_REG_INT_STATUS_1};
//         readBuffer = {0, 0};
//         rslt = _pI2C->access(_i2cAddr, writeBuffer.data(), writeBuffer.size(), readBuffer.data(), readBuffer.size(), numRead);
//         if (rslt == RaftI2CCentral::ACCESS_RESULT_OK)
//         {
// #ifdef DEBUG_HW_SETUP
//             LOG_I(MODULE_PREFIX, "setup OK read int i2cAddr 0x%02x int1 0x%02x int2 0x%02x", (int)_i2cAddr, (int)readBuffer[0], (int)readBuffer[1]);
// #endif
//         }
//         else
//         {
// #ifdef WARN_ON_HW_SETUP_FAILURE
//             LOG_E(MODULE_PREFIX, "setup FAILED read int i2cAddr 0x%02x", (int)_i2cAddr);
// #endif
//         }

//         // Set the mode to HRM
//         uint8_t modeValue = MAX30101_VAL_MODE_HRM;
//         writeBuffer = {MAX30101_REG_MODE_CONFIG, modeValue};
//         rslt = _pI2C->access(_i2cAddr, writeBuffer.data(), writeBuffer.size(), nullptr, 0, numRead);
//         if (rslt == RaftI2CCentral::ACCESS_RESULT_OK)
//         {
// #ifdef DEBUG_HW_SETUP
//             LOG_I(MODULE_PREFIX, "setup OK mode selection mode 0x%02x", (int)modeValue);
// #endif
//             // Set the Red led intensity
//             writeBuffer = {MAX30101_REG_LED1_PA, _redLedIntensity};
//             rslt = _pI2C->access(_i2cAddr, writeBuffer.data(), writeBuffer.size(), nullptr, 0, numRead);

//             if (rslt == RaftI2CCentral::ACCESS_RESULT_OK)
//             {
// #ifdef DEBUG_HW_SETUP
//                 LOG_I(MODULE_PREFIX, "setup OK redLedIntensity %.1fmA(0x%02x)", convRedLedIntensityToMA(_redLedIntensity), _redLedIntensity);
// #endif
//             }
//             else
//             {
// #ifdef WARN_ON_HW_SETUP_FAILURE
//                 LOG_E(MODULE_PREFIX, "setup FAILED redLedIntensity %.1fmA(0x%02x)", convRedLedIntensityToMA(_redLedIntensity), _redLedIntensity);
// #endif
//             }

//         }
//         else
//         {
// #ifdef WARN_ON_HW_SETUP_FAILURE
//             LOG_E(MODULE_PREFIX, "setup FAILED mode selection i2cAddr 0x%02x", (int)_i2cAddr);
// #endif
//         }
//         _isInitialised = true;
//     }
//     else
//     {
// #ifdef WARN_ON_HW_SETUP_FAILURE
//         LOG_E(MODULE_PREFIX, "setup FAILED i2cAddr 0x%x", (int)_i2cAddr);
// #endif
//     }    
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Poll Result Callbacks
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// // Poll result callback
// void MAX30101::pollResultCallbackStatic(void *pCallbackData, BusRequestResult &reqResult)
// {
//     if (pCallbackData)
//         ((MAX30101 *)pCallbackData)->pollResultCallback(reqResult);
// }

// // Poll result callback
// void MAX30101::pollResultCallback(BusRequestResult &reqResult)
// {
// //     // Get received data
// //     uint8_t* pData = reqResult.getReadData();
// //     if (!pData)
// //         return;
// //     if (reqResult.getReadDataLen() == MAX30101_BYTES_TO_READ_FOR_ROTATION)
// //     {
// //         // Extract raw sensor data
// //         int32_t sensorAngle = (((uint32_t)pData[0] << 8) | pData[1]) & 0x0fff;

// //         // Check for reverse rotation
// //         if (_rotationDirectionReversed)
// //             sensorAngle = MAX30101_RAW_RANGE - sensorAngle;

// //         // Filter
// //         _angleFilter.sample(sensorAngle);

// //         // Add to sample collector
// //         if (_pSampleCollector)
// //             _pSampleCollector->addSample(sensorAngle);

// // #ifdef DEBUG_POLL_RESULT
// //         if (Raft::isTimeout(millis(), _debugLastShowTimeMs, 100))
// //         {
// //             String debugStr;
// //             Raft::getHexStrFromBytes(pData, reqResult.getReadDataLen(), debugStr);
// //             LOG_I(MODULE_PREFIX, "pollResultCallback angleDegrees %0.1f angleRadians %0.1f rawReading %d rawBytes %s", 
// //                     _angleFilter.getAverage(false, false) * MAX30101_ANGLE_CONVERSION_FACTOR_DEGREES,
// //                     _angleFilter.getAverage(false, false) * MAX30101_ANGLE_CONVERSION_FACTOR_RADIANS,
// //                     sensorAngle, 
// //                     debugStr.c_str());
// //             _debugLastShowTimeMs = millis();
// //         }
// // #endif
// //     }
// }

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Get temperature result callback
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// void MAX30101::getTemperatureResultCallbackStatic(void *pCallbackData, BusRequestResult &reqResult)
// {
//     if (pCallbackData)
//         ((MAX30101 *)pCallbackData)->getTemperatureResultCallback(reqResult);
// }

// void MAX30101::getTemperatureResultCallback(BusRequestResult &reqResult)
// {
// //     // Get received data
// //     uint8_t* pData = reqResult.getReadData();
// //     if (!pData)
// //         return;
// //     if (reqResult.getReadDataLen() == 2)
// //     {
// //         // Extract raw sensor data
// //         int32_t sensorAngle = (((uint32_t)pData[0] << 8) | pData[1]) & 0x0fff;

// //         // Check for reverse rotation
// //         if (_rotationDirectionReversed)
// //             sensorAngle = MAX30101_RAW_RANGE - sensorAngle;

// //         // Filter
// //         _angleFilter.sample(sensorAngle);

// //         // Add to sample collector
// //         if (_pSampleCollector)
// //             _pSampleCollector->addSample(sensorAngle);

// // #ifdef DEBUG_POLL_RESULT
// //         if (Raft::isTimeout(millis(), _debugLastShowTimeMs, 100))
// //         {
// //             String debugStr;
// //             Raft::getHexStrFromBytes(pData, reqResult.getReadDataLen(), debugStr);
// //             LOG_I(MODULE_PREFIX, "pollResultCallback angleDegrees %0.1f angleRadians %0.1f rawReading %d rawBytes %s", 
// //                     _angleFilter.getAverage(false, false) * MAX30101_ANGLE_CONVERSION_FACTOR_DEGREES,
// //                     _angleFilter.getAverage(false, false) * MAX30101_ANGLE_CONVERSION_FACTOR_RADIANS,
// //                     sensorAngle, 
// //                     debugStr.c_str());
// //             _debugLastShowTimeMs = millis();
// //         }


// // #endif
// //     }
// }

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Get HW Revision callback
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// void MAX30101::getHwRevCallbackStatic(void *pCallbackData, BusRequestResult &reqResult)
// {
//     if (pCallbackData)
//         ((MAX30101 *)pCallbackData)->getHwRevCallback(reqResult);
// }

// void MAX30101::getHwRevCallback(BusRequestResult &reqResult)
// {
//     // Get received data
//     uint8_t* pData = reqResult.getReadData();
//     if (!pData)
//         return;
//     if (reqResult.getReadDataLen() == MAX30101_BYTES_TO_READ_REV_ID)
//     {

//         // Extract raw sensor data
//         uint32_t hwRev = pData[0];
//         uint32_t partId =pData[1];

// #ifdef DEBUG_HW_SETUP
//         if (Raft::isTimeout(millis(), _debugLastShowTimeMs, 100))
//         {
//             LOG_I(MODULE_PREFIX, "getHwRevCallback hwRev %d partId %d", hwRev, partId);
//             _debugLastShowTimeMs = millis();
//         }
// #endif
//     }
// }
