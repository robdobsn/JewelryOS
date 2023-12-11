// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// //
// // MCMicrophone.h
// //
// // Rob Dobson 2023
// //
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// #pragma once

// #include <ConfigBase.h>
// #include <SpiramAwareAllocator.h>
// #include "driver/i2s_std.h"

// // #define INCLUDE_LAME_MP3_ENCODER

// #ifdef INCLUDE_LAME_MP3_ENCODER
// #include "MP3EncoderLAME.h"
// #endif

// class MCMicrophone
// {
// public:
//     MCMicrophone();
//     virtual ~MCMicrophone();

//     // Setup and teardown
//     void setup(ConfigBase& config, const char* pConfigPrefix);
//     void teardown();

//     // Service
//     void service();

//     // Capture
//     bool captureToFile(const char* filename, uint32_t recTimeSecs);

// private:
//     // Singleton
//     static MCMicrophone* _pThis;

//     // Is setup
//     bool _isSetup = false;

//     // I2S chanhandle
//     i2s_chan_handle_t _i2sChanHandle = nullptr;

//     // Microphone config class
//     class MicrophoneConfig
//     {
//     public:
//         // Microphone type
//         bool isPDM = false;

//         // Sample rate
//         uint32_t sampleRate = 0;

//         // I2S config
//         int i2sDataPin = -1;
//         int i2sBCLKPin = -1;
//         int i2sLRCLKPin = -1;
//         int i2sDataBitsPerSample = 16;
//         int i2sNumChannels = 1;

//         // MP3 quality
//         int mp3Quality = 5;

//     };

//     // Microphone config
//     MicrophoneConfig _micConfig;

// #ifdef INCLUDE_LAME_MP3_ENCODER
//     // MP3 encoder
//     liblame::MP3EncoderLAME _mp3Encoder;
//     liblame::AudioInfo _mp3EncoderInfo;

//     // MP3 encoder callback
//     static void mp3EncoderCallbackStatic(uint8_t* pBuf, size_t bufLen)
//     {
//         if (_pThis)
//             _pThis->mp3EncoderCallback(pBuf, bufLen);
//     }
//     void mp3EncoderCallback(uint8_t* pBuf, size_t bufLen);
// #endif

//     // Debug file handle
//     FILE* _pDebugFile = nullptr;
//     uint32_t _debugTestEncodeSampleIdx = 0;

//     // Apply setup
//     bool applySetup(const ConfigBase& configOverride, const char* pConfigPrefix);

//     // Get samples
//     uint32_t getSamples(uint8_t* pBuf, uint32_t bufLen);
// };
