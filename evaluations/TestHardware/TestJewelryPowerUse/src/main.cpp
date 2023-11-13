#include <Arduino.h>
#include <Wire.h>
// #include "MAX30105.h"
// #include "heartRate.h"
#include "esp32/clk.h"

static const int HRINT = 0;
static const int I2C_SCL = 8;
static const int I2C_SDA = 10;

// MAX30101 HRM sensor defines
static const uint8_t MAX30101_I2C_ADDRESS = 0x57;
static const uint8_t MAX30101_REG_INT_STATUS_1 = 0x00;
static const uint8_t MAX30101_REG_INT_STATUS_2 = 0x01;
static const uint8_t MAX30101_REG_INT_ENABLE_1 = 0x02;
static const uint8_t MAX30101_REG_INT_ENABLE_2 = 0x03;
static const uint8_t MAX30101_REG_MODE_CONFIG = 0x09;
static const uint8_t MAX30101_REG_SPO2_CONFIG = 0x0A;
static const uint8_t MAX30101_REG_LED1_PA = 0x0C;
static const uint8_t MAX30101_REG_LED2_PA = 0x0D;
static const uint8_t MAX30101_REG_LED3_PA = 0x0E;
static const uint8_t MAX30101_REG_LED4_PA = 0x0F;
static const uint8_t MAX30101_VAL_MODE_HRM = 0x02;
static const uint8_t MAX30101_VAL_RESET_HRM = 0x40;
static const uint8_t MAX30101_VAL_SHUTDOWN_HRM = 0x80;

static const uint8_t MAX30101_REG_PULSE_WIDTH_69US = 0;
static const uint8_t MAX30101_REG_PULSE_WIDTH_118US = 1;
static const uint8_t MAX30101_REG_PULSE_WIDTH_215US = 2;
static const uint8_t MAX30101_REG_PULSE_WIDTH_411US = 3;

// Output LEDs pins
static const int HW_LEDS_1_PIN = 5;
static const int HW_LEDS_2_PIN = 3;
static const int HW_LEDS_3_PIN = 1;
static const int HW_LEDS_4_PIN = 6;
static const int HW_POWER_REMAIN_ON_PIN = 7;

// VSENSE BAT
static const int HW_VSENSE_BAT_PIN = 4;

// #define SET_CPU_FREQ 10
#define CLEAR_OUTPUT_LEDS_ON_STARTUP
#define HRM_WITHOUT_LIBRARY
// #define ENTER_LIGHT_SLEEP
// #define SETUP_HRM_MODE
// #define SET_HRM_RED_LED_INTESITY_LEVEL 0xff
// #define SET_LED_PULSE_WIDTH MAX30101_REG_PULSE_WIDTH_69US
// #define TURN_OFF_POWER_AFTER_MS 10000
#define USE_HW_POWER_REMAIN_ON_PIN
#define DEBUG_VSENSE_READING_TO_CONSOLE
#define DEBUG_OUTPUT_TO_CONSOLE_MS 1000
// #define DEBUG_CLOCK_SPEED_TO_CONSOLE_IN_SETUP

#define LEDS_1_PWM_LEVEL 0
#define LEDS_2_PWM_LEVEL 0
#define LEDS_3_PWM_LEVEL 0
#define LEDS_4_PWM_LEVEL 0


#define DIABLE_HRM_IR
// #define GET_HRM_READINGS
// #define LOG_HRM_READINGS
// #define TEST_LEDS_ALONE_IN_SLEEP
// #define USE_LED_PWM
// #define SLEEP_LIGHT

#define PWM_VALUE_FOR_ON 20

// MAX30105 hrmSensor;
TwoWire i2cPort = TwoWire(0);
const uint8_t i2cAddr = MAX30101_I2C_ADDRESS;
// const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
// byte rates[RATE_SIZE]; //Array of heart rates
// byte rateSpot = 0;
// long lastBeat = 0; //Time at which the last beat occurred

// float beatsPerMinute = 60;
// int beatAvg = 60;
// int ledsBPM = 60;
// uint32_t prevLedPos = 0;
// uint32_t cyclePos = 0;
// const uint32_t LED_MAX = 4;
// const uint32_t CYCLE_MAX = 5;
// uint32_t cycleLastMs = 0;
// uint32_t cycleStateTimeMs = 250;
// const uint32_t LED_PULSE_SLEEP_US = 30000;
// const uint32_t IDLE_SLEEP_US = 970000;

// static const int cyclePosLeds[] = {HW_LEDS_4_PIN, HW_LEDS_3_PIN, HW_LEDS_2_PIN, HW_LEDS_1_PIN}; 

// uint32_t calcLEDSHoldTimeMs(float bpm)
// {
//   return 60000.0 / bpm / CYCLE_MAX;
// }

// void servicePulseOut()
// {
//   if (cycleLastMs == 0)
//   {
//     cycleStateTimeMs = calcLEDSHoldTimeMs(ledsBPM);
//     cyclePos = 0;
//     cycleLastMs = millis();
//   }

//   if (millis() > cycleLastMs + cycleStateTimeMs)
//   {
// #ifdef USE_LED_PWM
//     analogWrite(cyclePosLeds[(cyclePos+CYCLE_MAX-1)%CYCLE_MAX], 0);
//     analogWrite(cyclePosLeds[cyclePos], PWM_VALUE_FOR_ON);
// #else
//     if (cyclePos < LED_MAX)
//     {
//       digitalWrite(cyclePosLeds[prevLedPos], 0);
//       digitalWrite(cyclePosLeds[cyclePos], 1);
//       prevLedPos = cyclePos;
//     }
//     else
//     {
//       digitalWrite(cyclePosLeds[prevLedPos], 0);
//     }
// #endif
//     cycleLastMs = millis();
//     cyclePos = (cyclePos + 1) % CYCLE_MAX;
//     if (cyclePos == 0)
//     {
//           cycleStateTimeMs = calcLEDSHoldTimeMs(ledsBPM);
//     }
//   }

// }

//
// Low-level I2C Communication
//
uint8_t readRegister8(uint8_t address, uint8_t reg) {
  i2cPort.beginTransmission(address);
  i2cPort.write(reg);
  i2cPort.endTransmission(false);

  i2cPort.requestFrom((uint8_t)address, (uint8_t)1); // Request 1 byte
  if (i2cPort.available())
  {
    return(i2cPort.read());
  }

  return (0); //Fail

}

void writeRegister8(uint8_t address, uint8_t reg, uint8_t value) {
  i2cPort.beginTransmission(address);
  i2cPort.write(reg);
  i2cPort.write(value);
  i2cPort.endTransmission();
}

//Given a register, read it, mask it, and then set the thing
void bitMask(uint8_t reg, uint8_t mask, uint8_t thing)
{
  // Grab current register context
  uint8_t originalContents = readRegister8(i2cAddr, reg);

  // Zero-out the portions of the register we're interested in
  originalContents = originalContents & mask;

  // Change contents
  writeRegister8(i2cAddr, reg, originalContents | thing);
}

void softReset(void) {
  bitMask(MAX30101_REG_MODE_CONFIG, ~MAX30101_VAL_RESET_HRM, MAX30101_VAL_RESET_HRM);

  // Poll for bit to clear, reset is then complete
  // Timeout after 100ms
  unsigned long startTime = millis();
  while (millis() - startTime < 100)
  {
    uint8_t response = readRegister8(i2cAddr, MAX30101_REG_MODE_CONFIG);
    if ((response & MAX30101_VAL_RESET_HRM) == 0) break; //We're done!
    delay(1); //Let's not over burden the I2C bus
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup
////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
#ifdef DEBUG_OUTPUT_TO_CONSOLE_MS
  Serial.begin(115200);
#endif

#ifdef DEBUG_CLOCK_SPEED_TO_CONSOLE_IN_SETUP
  uint32_t cpu_freq = esp_clk_cpu_freq();
  Serial.print("CPU Frequency: ");
  Serial.println(cpu_freq);
#endif

#ifdef SET_CPU_FREQ
  setCpuFrequencyMhz(SET_CPU_FREQ);
#endif

#ifdef DEBUG_CLOCK_SPEED_TO_CONSOLE_IN_SETUP
  cpu_freq = esp_clk_cpu_freq();
  Serial.print("CPU Frequency: ");
  Serial.println(cpu_freq);
#endif

#ifdef USE_HW_POWER_REMAIN_ON_PIN
  pinMode(HW_POWER_REMAIN_ON_PIN, OUTPUT);
  digitalWrite(HW_POWER_REMAIN_ON_PIN, HIGH);
#endif

#ifdef OUTPUT_VSENSE_READING_TO_CONSOLE_MS
  pinMode(HW_VSENSE_BAT_PIN, INPUT);
#endif

  // pinMode(EXT_VDD, OUTPUT);
  // digitalWrite(EXT_VDD, HIGH);

#ifdef CLEAR_OUTPUT_LEDS_ON_STARTUP
  pinMode(HW_LEDS_1_PIN, OUTPUT);
  pinMode(HW_LEDS_2_PIN, OUTPUT);
  pinMode(HW_LEDS_3_PIN, OUTPUT);
  pinMode(HW_LEDS_4_PIN, OUTPUT);
// #ifdef USE_LED_PWM
//   analogWrite(HW_LEDS_1_PIN, 0);
//   analogWrite(HW_LEDS_2_PIN, 0);
//   analogWrite(HW_LEDS_3_PIN, 0);
//   analogWrite(HW_LEDS_4_PIN, 0);
// #else
  digitalWrite(HW_LEDS_1_PIN, 0);
  digitalWrite(HW_LEDS_2_PIN, 0);
  digitalWrite(HW_LEDS_3_PIN, 0);
  digitalWrite(HW_LEDS_4_PIN, 0);
// #endif
#endif

#ifdef HRM_WITHOUT_LIBRARY
  pinMode(HRINT, INPUT);
  i2cPort.begin(I2C_SDA, I2C_SCL, 100000);
  softReset();
  // Read interrupts to clear
  // This is necessary as Power ready interrupt holds until read and this uses energy through pullups
  readRegister8(i2cAddr, MAX30101_REG_INT_STATUS_1);
  readRegister8(i2cAddr, MAX30101_REG_INT_STATUS_2);
#endif

#ifdef SETUP_HRM_MODE
  // Set mode to HRM
  bitMask(MAX30101_REG_MODE_CONFIG, ~MAX30101_VAL_MODE_HRM, MAX30101_VAL_MODE_HRM);
#endif

#if defined(SET_HRM_RED_LED_INTESITY_LEVEL)
  writeRegister8(i2cAddr, MAX30101_REG_LED1_PA, SET_HRM_RED_LED_INTESITY_LEVEL);
#endif

#if defined(SET_LED_PULSE_WIDTH)
  writeRegister8(i2cAddr, MAX30101_REG_SPO2_CONFIG, SET_LED_PULSE_WIDTH);
#endif

#ifdef LEDS_1_PWM_LEVEL
  analogWrite(HW_LEDS_1_PIN, LEDS_1_PWM_LEVEL);
#endif
#ifdef LEDS_2_PWM_LEVEL
  analogWrite(HW_LEDS_2_PIN, LEDS_2_PWM_LEVEL);
#endif
#ifdef LEDS_3_PWM_LEVEL
  analogWrite(HW_LEDS_3_PIN, LEDS_3_PWM_LEVEL);
#endif
#ifdef LEDS_4_PWM_LEVEL
  analogWrite(HW_LEDS_4_PIN, LEDS_4_PWM_LEVEL);
#endif

// #ifdef SETUP_HRM
//   delay(100);
//   i2cPort.begin(I2C_SDA, I2C_SCL, 100000);

//     // Initialize sensor
//   if (hrmSensor.begin(i2cPort) == false)
//   {
//     Serial.println("MAX30105 was not found. Please check wiring/power. ");
//     while (1);
//   }

//   //Setup to sense a nice looking saw tooth on the plotter
//   byte ledBrightness = 0x1F; //Options: 0=Off to 255=50mA
//   byte sampleAverage = 8; //Options: 1, 2, 4, 8, 16, 32
//   byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
//   int sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
//   int pulseWidth = 411; //Options: 69, 118, 215, 411
//   int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

//   // hrmSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

//   hrmSensor.setup(0, 4, 1, 50, 411, 4096); //Configure sensor with default settings
//   hrmSensor.setPulseAmplitudeRed(0); //Turn Red LED to low to indicate sensor is running
//   hrmSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

// #ifdef DIABLE_HRM_IR
//   hrmSensor.setPulseAmplitudeIR(0);
// #endif
// #endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup
////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t debugLastMs = 0;

void loop()
{

#ifdef GET_HRM_READINGS

  long irValue = 0;
  // for (int loopIdx = 0; loopIdx < 500; loopIdx++)
  // {
    // Serial.println(hrmSensor.getIR()); //Send raw data to plotter
    irValue = hrmSensor.getIR();

    if (checkForBeat(irValue) == true)
    {
      //We sensed a beat!
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute < 255 && beatsPerMinute > 20)
      {
        rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
        rateSpot %= RATE_SIZE; //Wrap variable

        //Take average of readings
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }

  //   delay(10);
  // }

#ifdef LOG_HRM_READINGS
  if (millis() > debugLastMs + 250)
  {
    Serial.print("IR=");
    Serial.print(irValue);
    Serial.print(", BPM=");
    Serial.print(beatsPerMinute);
    Serial.print(", AvgBPM=");
    Serial.print(beatAvg);

    // if (irValue < 50000)
    //   Serial.print(" No finger?");

    Serial.println();
#endif
    debugLastMs = millis();
  }

#endif


  // esp_sleep_enable_timer_wakeup(1000000);
  // gpio_hold_en((gpio_num_t)EXT_VDD);
  // esp_light_sleep_start();
  // gpio_hold_en((gpio_num_t)EXT_VDD);

  // esp_sleep_enable_timer_wakeup(20000);
  // gpio_hold_en((gpio_num_t)LEDS_1);
  // esp_light_sleep_start();
  // digitalWrite(LEDS_1, HIGH);
  // esp_deep_sleep_start();

#ifdef SERVICE_PULSE_OUT
  servicePulseOut();
#endif

// #ifdef SLEEP_LIGHT
//   esp_sleep_enable_timer_wakeup(cyclePos < LED_MAX ? LED_PULSE_SLEEP_US : IDLE_SLEEP_US);
//   esp_light_sleep_start();

#if defined(ENTER_LIGHT_SLEEP)
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_light_sleep_start();
#endif

#ifdef USE_HW_POWER_REMAIN_ON_PIN
#if defined(TURN_OFF_POWER_AFTER_MS)
  if (millis() > TURN_OFF_POWER_AFTER_MS)
  {
    digitalWrite(HW_POWER_REMAIN_ON_PIN, LOW);
    // gpio_hold_en((gpio_num_t)PWR_OFF);
    // gpio_deep_sleep_hold_en();
  }
#endif
#endif

#ifdef DEBUG_OUTPUT_TO_CONSOLE_MS
  if (millis() > debugLastMs + DEBUG_OUTPUT_TO_CONSOLE_MS)
  {
    Serial.print("Status ");
#ifdef DEBUG_VSENSE_READING_TO_CONSOLE
    Serial.print("VSENSE=");
    Serial.print(analogRead(HW_VSENSE_BAT_PIN));
#endif
    Serial.println();
    debugLastMs = millis();
  }
#endif
}
