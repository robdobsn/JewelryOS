static const int HRINT = 0;
static const int LEDS_1 = 5;
static const int LEDS_2 = 3;
static const int LEDS_3 = 1;
static const int LEDS_4 = 6;
static const int I2C_SCL = 8;
static const int I2C_SDA = 10;
static const int PWR_OFF = 7;

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "esp_clk.h"

#define SETUP_HRM
#define DIABLE_HRM_IR
// #define GET_HRM_READINGS
// #define LOG_HRM_READINGS
// #define SERIAL_OUTPUT_CLOCK_SPEED
#define TEST_LIGHT_SLEEP_NO_LEDS
// #define TEST_LEDS_ALONE_IN_SLEEP
// #define USE_LED_PWM
// #define SLEEP_LIGHT

#define PWM_VALUE_FOR_ON 20

MAX30105 hrmSensor;
TwoWire i2cOne = TwoWire(0);
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute = 60;
int beatAvg = 60;
int ledsBPM = 60;
uint32_t prevLedPos = 0;
uint32_t cyclePos = 0;
const uint32_t LED_MAX = 4;
const uint32_t CYCLE_MAX = 5;
uint32_t cycleLastMs = 0;
uint32_t cycleStateTimeMs = 250;
const uint32_t LED_PULSE_SLEEP_US = 30000;
const uint32_t IDLE_SLEEP_US = 970000;

static const int cyclePosLeds[] = {LEDS_4, LEDS_3, LEDS_2, LEDS_1}; 

uint32_t calcLEDSHoldTimeMs(float bpm)
{
  return 60000.0 / bpm / CYCLE_MAX;
}

void servicePulseOut()
{
  if (cycleLastMs == 0)
  {
    cycleStateTimeMs = calcLEDSHoldTimeMs(ledsBPM);
    cyclePos = 0;
    cycleLastMs = millis();
  }

  if (millis() > cycleLastMs + cycleStateTimeMs)
  {
#ifdef USE_LED_PWM
    analogWrite(cyclePosLeds[(cyclePos+CYCLE_MAX-1)%CYCLE_MAX], 0);
    analogWrite(cyclePosLeds[cyclePos], PWM_VALUE_FOR_ON);
#else
    if (cyclePos < LED_MAX)
    {
      digitalWrite(cyclePosLeds[prevLedPos], 0);
      digitalWrite(cyclePosLeds[cyclePos], 1);
      prevLedPos = cyclePos;
    }
    else
    {
      digitalWrite(cyclePosLeds[prevLedPos], 0);
    }
#endif
    cycleLastMs = millis();
    cyclePos = (cyclePos + 1) % CYCLE_MAX;
    if (cyclePos == 0)
    {
          cycleStateTimeMs = calcLEDSHoldTimeMs(ledsBPM);
    }
  }

}

void setup() {
#ifdef SERIAL_OUTPUT_CLOCK_SPEED
  Serial.begin(115200);
  uint32_t cpu_freq = esp_clk_cpu_freq();
  Serial.print("CPU Frequency: ");
  Serial.println(cpu_freq);
#endif

  setCpuFrequencyMhz(10);

#ifdef SERIAL_OUTPUT_CLOCK_SPEED
  cpu_freq = esp_clk_cpu_freq();
  Serial.print("CPU Frequency: ");
  Serial.println(cpu_freq);
#endif


  // pinMode(EXT_VDD, OUTPUT);
  // digitalWrite(EXT_VDD, HIGH);

  pinMode(LEDS_1, OUTPUT);
  pinMode(LEDS_2, OUTPUT);
  pinMode(LEDS_3, OUTPUT);
  pinMode(LEDS_4, OUTPUT);
#ifdef USE_LED_PWM
  analogWrite(LEDS_1, 0);
  analogWrite(LEDS_2, 0);
  analogWrite(LEDS_3, 0);
  analogWrite(LEDS_4, 0);
#else
  digitalWrite(LEDS_1, 0);
  digitalWrite(LEDS_2, 0);
  digitalWrite(LEDS_3, 0);
  digitalWrite(LEDS_4, 0);
#endif

#ifdef SETUP_HRM
  delay(100);
  i2cOne.begin(I2C_SDA, I2C_SCL, 100000);

    // Initialize sensor
  if (hrmSensor.begin(i2cOne) == false)
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  //Setup to sense a nice looking saw tooth on the plotter
  byte ledBrightness = 0x1F; //Options: 0=Off to 255=50mA
  byte sampleAverage = 8; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  // hrmSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

  hrmSensor.setup(0, 4, 1, 50, 411, 4096); //Configure sensor with default settings
  hrmSensor.setPulseAmplitudeRed(0); //Turn Red LED to low to indicate sensor is running
  hrmSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

#ifdef DIABLE_HRM_IR
  hrmSensor.setPulseAmplitudeIR(0);
#endif
#endif
}

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

#ifdef SLEEP_LIGHT
  esp_sleep_enable_timer_wakeup(cyclePos < LED_MAX ? LED_PULSE_SLEEP_US : IDLE_SLEEP_US);
  esp_light_sleep_start();
#elif defined(TEST_LIGHT_SLEEP_NO_LEDS)
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_light_sleep_start();
#endif
}
