// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// //
// // AnalogMicrophone.cpp
// //
// // Rob Dobson 2023
// //
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// #include "sdkconfig.h"
// #include <sys/stat.h>
// #include <unistd.h>
// #include "esp_err.h"
// #include "esp_system.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"
// #include "format_wav.h"
// #include "AnalogMicrophone.h"
// #include "Logger.h"
// #include "FileSystem.h"
// #include "math.h"
// #include "esp_adc/adc_continuous.h"

// // #define DEBUG_TEST_ENCODE_SINE_WAVE_FREQ 440

// #define FS_MOUNT_POINT      "/local"
// // #define SAMPLE_SIZE         (CONFIG_EXAMPLE_BIT_SAMPLE * 1024)
// // #define BYTE_RATE           (CONFIG_EXAMPLE_SAMPLE_RATE * (CONFIG_EXAMPLE_BIT_SAMPLE / 8)) * NUM_CHANNELS
// // static int16_t i2s_readraw_buff[SAMPLE_SIZE];
// // size_t bytes_read;
// // const int WAVE_HEADER_SIZE = 44;
// // const int MP3_SAMPLE_RATE = 32000;
// // const int MP3_CHANNELS = 1;


// static const char *MODULE_PREFIX = "AnalogMicrophone";

// AnalogMicrophone* AnalogMicrophone::_pThis = nullptr;

// // Constructor
// AnalogMicrophone::AnalogMicrophone()
// #ifdef INCLUDE_LAME_MP3_ENCODER
//     : _mp3Encoder(mp3EncoderCallbackStatic)
// #endif
// {
//     _pThis = this;
// }

// // Destructor
// AnalogMicrophone::~AnalogMicrophone()
// {
//     teardown();
// }

// void AnalogMicrophone::setup(ConfigBase& config, const char* pConfigPrefix)
// {
//     // Microphone type
//     _micConfig.isAnalog = config.getBool("isAnalog", false, pConfigPrefix);

//     // This class only supports analog microphones
//     if (!_micConfig.isAnalog)
//     {
//         LOG_E(MODULE_PREFIX, "setup FAILED - only analog microphones supported");
//         return;
//     }

//     // Sample rate
//     _micConfig.sampleRate = config.getLong("sampleRate", 8000, pConfigPrefix);

//     // Power and signal pins
//     _micConfig.powerPin = config.getLong("powerPin", -1, pConfigPrefix);
//     _micConfig.signalPin = config.getLong("signalPin", -1, pConfigPrefix);


// #ifdef INCLUDE_LAME_MP3_ENCODER
//     // MP3 config
//     _micConfig.mp3Quality = config.getLong("mp3Quality", 5, pConfigPrefix);
// #endif

//     // Apply setup
//     applySetup(config, pConfigPrefix);
// }

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Apply setup
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// bool AnalogMicrophone::applySetup(const ConfigBase& configOverride, const char* pConfigPrefix)
// {
//     // Teardown if already setup
//     teardown();

//     // Check pins valid
//     if (_micConfig.signalPin < 0)
//     {
//         LOG_E(MODULE_PREFIX, "setup FAILED - invalid pin signal %d", _micConfig.signalPin);
//         return false;
//     }

//     // Setup the I2S
//     if (_micConfig.isAnalog)
//     {
//         // Check if power pin used
//         if (_micConfig.powerPin >= 0)
//         {
//             // Power on
//             pinMode(_micConfig.powerPin, OUTPUT);
//             digitalWrite(_micConfig.powerPin, HIGH);
//         }

//         // Setup ADC
//         adc_continuous_handle_cfg_t adc_config = {
//             .max_store_buf_size = 1024,
//             .conv_frame_size = 256,
//         };
//         ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

//         adc_continuous_config_t dig_cfg = {
//             .sample_freq_hz = _micConfig.sampleRate,
//             .conv_mode = ADC_CONV_SINGLE_UNIT_1,
//             .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
//         };
//         adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
//         dig_cfg.pattern_num = channel_num;
//         for (int i = 0; i < channel_num; i++) {
//             adc_pattern[i].atten = EXAMPLE_ADC_ATTEN;
//             adc_pattern[i].channel = channel[i] & 0x7;
//             adc_pattern[i].unit = EXAMPLE_ADC_UNIT;
//             adc_pattern[i].bit_width = EXAMPLE_ADC_BIT_WIDTH;

//             ESP_LOGI(TAG, "adc_pattern[%d].atten is :%"PRIx8, i, adc_pattern[i].atten);
//             ESP_LOGI(TAG, "adc_pattern[%d].channel is :%"PRIx8, i, adc_pattern[i].channel);
//             ESP_LOGI(TAG, "adc_pattern[%d].unit is :%"PRIx8, i, adc_pattern[i].unit);
//         }
//         dig_cfg.adc_pattern = adc_pattern;
//         ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));        

//     }
//     else
//     {
//         // Setup I2S STD RX
//         i2s_std_config_t i2s_std_cfg  = {
//             .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(_micConfig.sampleRate),
//             // The default mono slot is the left slot (whose 'select/LR pin' of the I2S STD microphone is pulled down)
//             // Note that the cast below assumes that the enumeration i2s_data_bit_width_t has the same values as the bit width
//             .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG((i2s_data_bit_width_t)_micConfig.i2sDataBitsPerSample, I2S_SLOT_MODE_MONO),
//             .gpio_cfg = {
//                 .mclk = I2S_GPIO_UNUSED,
//                 .bclk = (gpio_num_t)_micConfig.i2sBCLKPin,
//                 .ws = (gpio_num_t)_micConfig.i2sLRCLKPin,
//                 .dout = I2S_GPIO_UNUSED,
//                 .din = (gpio_num_t)_micConfig.i2sDataPin,
//                 .invert_flags = {
//                     .mclk_inv = false,
//                     .bclk_inv = false,
//                     .ws_inv = false,
//                 },
//             },
//         };

//         // Initialize the I2S
//         ESP_ERROR_CHECK(i2s_channel_init_std_mode(_i2sChanHandle, &i2s_std_cfg));
//     }
//     esp_err_t err = ESP_OK;
//     ESP_ERROR_CHECK(err = i2s_channel_enable(_i2sChanHandle));
   
//     if (err != ESP_OK)
//     {
//         LOG_I(MODULE_PREFIX, "setup FAILED err %d", err);
//         return false;
//     }

// #ifdef INCLUDE_LAME_MP3_ENCODER
//     // MP3 encoder
//     _mp3EncoderInfo.sample_rate = _micConfig.sampleRate;
//     _mp3EncoderInfo.channels = _micConfig.i2sNumChannels;
//     _mp3EncoderInfo.bits_per_sample = _micConfig.i2sDataBitsPerSample;
//     _mp3EncoderInfo.quality = _micConfig.mp3Quality;
// #endif

//     // Ok
//     LOG_I(MODULE_PREFIX, "setup OK");
//     _isSetup = true;
//     return true;
// }

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Teardown
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// void AnalogMicrophone::teardown()
// {
//     // Check if already setup
//     if (!_isSetup)
//         return;

//     // Disable the I2S
//     if (_i2sChanHandle)
//     {
//         i2s_channel_disable(_i2sChanHandle);
//         i2s_del_channel(_i2sChanHandle);
//     }

//     // LRCLK pin input
//     if (_micConfig.i2sLRCLKPin >= 0)
//         pinMode(_micConfig.i2sLRCLKPin, INPUT);

//     // Ok
//     LOG_I(MODULE_PREFIX, "teardown OK");
//     _isSetup = false;
// }

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Loop
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// void AnalogMicrophone::loop()
// {
// }

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Capture to file
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// bool AnalogMicrophone::captureToFile(const char* filename, uint32_t recTimeSecs)
// {
//     // Check setup
//     if (!_isSetup)
//         return false;

//     // Byte rate
//     uint32_t byteRate = _micConfig.sampleRate * (_micConfig.i2sDataBitsPerSample / 8) * _micConfig.i2sNumChannels;

// #ifdef INCLUDE_LAME_MP3_ENCODER
//     // Check for MP3
//     bool isMP3 = strstr(filename, ".mp3") != NULL;

//     // Start MP3 encoder if required
//     if (isMP3)
//     {
//         // Start MP3 encoder
//         _mp3Encoder.begin(_mp3EncoderInfo);
//     }
// #else
//     bool isMP3 = false;
// #endif

//     LOG_I(MODULE_PREFIX, "Starting recording for %d seconds!", recTimeSecs);

//     // Use POSIX and C standard library functions to work with files.
//     LOG_I(MODULE_PREFIX, "Opening file %s isMP3 %s", filename, isMP3 ? "Y" : "N");

//     // Filename including file system
//     String filenameWithFS = String(FS_MOUNT_POINT) + "/" + filename;

//     // First check if file exists before creating a new file.
//     struct stat st;
//     if (stat(filenameWithFS.c_str(), &st) == 0) {
//         // Delete it if it exists
//         unlink(filenameWithFS.c_str());
//     }

//     // Create new file
//     FILE *f = fopen(filenameWithFS.c_str(), "a");
//     if (f == NULL) {
//         LOG_E(MODULE_PREFIX, "Failed to open file for writing");
//         return false;
//     }
//     _pDebugFile = f;

//     // Result
//     bool rslt = true;

//     // Start timer
//     uint32_t timeRecStarted = millis();
    
//     // Check for WAV format
//     if (!isMP3)
//     {
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
//         const wav_header_t wav_header =
//             WAV_HEADER_PCM_DEFAULT(byteRate * recTimeSecs, 16, _micConfig.sampleRate, 1);
// #pragma GCC diagnostic pop

//         // Write the header to the WAV file
//         fwrite(&wav_header, sizeof(wav_header), 1, f);
//     }

//     // Start recording
//     // TODO - make this work using service loop
//     const uint32_t BYTES_PER_ITERATION = 5000;
//     uint8_t i2s_readraw_buff[BYTES_PER_ITERATION];
//     while (!Raft::isTimeout(millis(), timeRecStarted, recTimeSecs * 1000)) 
//     {
//         // Read the RAW samples from the microphone
//         uint32_t bytesRead = getSamples(i2s_readraw_buff, BYTES_PER_ITERATION);
//         if (bytesRead != 0)
//         {
//             // LOG_I(MODULE_PREFIX, "[0] %d [1] %d [2] %d [3]%d ...", i2s_readraw_buff[0], i2s_readraw_buff[1], i2s_readraw_buff[2], i2s_readraw_buff[3]);

//             // Check format
//             if (isMP3)
//             {
// #ifdef INCLUDE_LAME_MP3_ENCODER
//                 // Encode to MP3
//                 _mp3Encoder.write(i2s_readraw_buff, bytesRead);
// #endif
//             }
//             else
//             {
//                 // Write the samples to the file
//                 fwrite(i2s_readraw_buff, bytesRead, 1, f);
//             }
//         } 
//         else 
//         {
//             LOG_E(MODULE_PREFIX, "Read Failed!");
//             break;
//         }
//     }

//     LOG_I(MODULE_PREFIX, "Recording done!");
//     _pDebugFile = nullptr;
//     fclose(f);
//     LOG_I(MODULE_PREFIX, "File written");

//     // Reopen file
//     f = fopen(filenameWithFS.c_str(), "r");
//     if (f == NULL) {
//         LOG_E(MODULE_PREFIX, "Failed to open file for reading");
//         return false;
//     }

//     // Check format
//     if (isMP3)
//     {
//         // TODO
//         LOG_I(MODULE_PREFIX, "MP3 encoder write NEEDS CHECKING");
//     }
//     else
//     {
//         // test that the starting values are consistent with a valid WAV file
//         wav_header_t header;
//         fread(&header, sizeof(header), 1, f);

//         // Print out the header information
//         LOG_I(MODULE_PREFIX, "ChunkID: %.4s", header.descriptor_chunk.chunk_id);
//         LOG_I(MODULE_PREFIX, "ChunkSize: %d", header.descriptor_chunk.chunk_size);
//         LOG_I(MODULE_PREFIX, "Format: %.4s", header.descriptor_chunk.chunk_format);
//         LOG_I(MODULE_PREFIX, "Subchunk1ID: %.4s", header.fmt_chunk.subchunk_id);
//         LOG_I(MODULE_PREFIX, "Subchunk1Size: %d", header.fmt_chunk.subchunk_size);
//         LOG_I(MODULE_PREFIX, "AudioFormat: %d", header.fmt_chunk.audio_format);
//         LOG_I(MODULE_PREFIX, "NumChannels: %d", header.fmt_chunk.num_of_channels);
//         LOG_I(MODULE_PREFIX, "SampleRate: %d", header.fmt_chunk.sample_rate);
//         LOG_I(MODULE_PREFIX, "ByteRate: %d", header.fmt_chunk.byte_rate);
//         LOG_I(MODULE_PREFIX, "BlockAlign: %d", header.fmt_chunk.block_align);
//         LOG_I(MODULE_PREFIX, "BitsPerSample: %d", header.fmt_chunk.bits_per_sample);
        
//         // Check the header
//         if (strncmp(header.descriptor_chunk.chunk_id, "RIFF", 4) != 0 ||
//             strncmp(header.descriptor_chunk.chunk_format, "WAVE", 4) != 0 ||
//             strncmp(header.fmt_chunk.subchunk_id, "fmt ", 4) != 0 ||
//             header.fmt_chunk.audio_format != 1 ||
//             header.fmt_chunk.num_of_channels != 1 ||
//             header.fmt_chunk.bits_per_sample != 16 ||
//             header.fmt_chunk.sample_rate != _micConfig.sampleRate ||
//             strncmp(header.data_chunk.subchunk_id, "data", 4) != 0 ||
//             header.data_chunk.subchunk_size != byteRate * recTimeSecs) {
//             LOG_E(MODULE_PREFIX, "Invalid WAV file");
//             rslt = false;
//         }
//     }

//     // Close
//     fclose(f);
    
//     return rslt;
// }

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // MP3 encoder callback
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// #ifdef INCLUDE_LAME_MP3_ENCODER
// void AnalogMicrophone::mp3EncoderCallback(uint8_t* pBuf, size_t bufLen)
// {
//     // LOG_I(MODULE_PREFIX, "MP3 encoder callback %d", bufLen);
//     if (_pDebugFile)
//     {
//         // Write to file
//         if (fwrite(pBuf, bufLen, 1, _pDebugFile) != 1)
//         {
//             LOG_E(MODULE_PREFIX, "MP3 encoder write FAILED");
//         }
//     }
//     // // Write to file
//     // if (pBuf && bufLen)
//     // {
//     //     // Write to file
//     //     if (FileSystem::appendFile("/local/mic.mp3", pBuf, bufLen) != bufLen)
//     //     {
//     //         LOG_E(MODULE_PREFIX, "MP3 encoder write FAILED");
//     //     }
//     // }
// }
// #endif

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Get samples
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// uint32_t AnalogMicrophone::getSamples(uint8_t* pBuf, uint32_t bufLen)
// {
// #ifdef DEBUG_TEST_ENCODE_SINE_WAVE_FREQ

//     for (uint32_t i = 0; i < bufLen; i += 2)
//     {
//         double phase = _debugTestEncodeSampleIdx * 2.0 * M_PI * DEBUG_TEST_ENCODE_SINE_WAVE_FREQ / _micConfig.sampleRate;
//         int16_t sample = (int16_t)(sin(phase) * 10000.0);
//         pBuf[i] = sample & 0xff;
//         pBuf[i + 1] = (sample >> 8) & 0xff;
//         _debugTestEncodeSampleIdx++;
//     }
//     return bufLen;

// #else
//     // Check setup
//     if (!_isSetup)
//         return 0;

//     // Read the RAW samples from the microphone
//     size_t bytesRead = 0;
//     if (i2s_channel_read(_i2sChanHandle, pBuf, bufLen, &bytesRead, 1000) == ESP_OK) 
//     {
//         return bytesRead;
//     }
// #endif
//     return 0;
// }
