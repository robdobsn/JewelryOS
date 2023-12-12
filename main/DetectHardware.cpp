/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DetectHardware for Scader PCBs
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DetectHardware.h"
#include "esp_log.h"
#include "RaftArduino.h"

// Module
static const char *MODULE_PREFIX = "DetectHardware";

// Initial HW revision
int DetectHardware::_hardwareRevision = -1;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main detection function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int DetectHardware::detectHardware()
{
    // Default to generic
    _hardwareRevision = HW_IS_GENERIC_BOARD;

    // Check analog levels on pins 3 and 4
    // GPIO 3 is LEDS_2 on Heart Earrings, Vsense on Grid Earrings V1.1 and MIC_VDD on Grid Earrings V1.1.2
    // GPIO 4 is Vsense on Heart Earrings, PWR_OFF on Grid Earrings V1.1 and Vsense on Grid Earrings V1.1.2
    // GPIO 5 is LEDS_1 on Heart Earrings, MIC_PWR on Grid Earrings V1.1 and PIX_EN on Grid Earrings V1.1.2
    // With no battery connected but running on charger (i.e. 4.2V):
    // - Heart earrings vsensePin3 = 704 vsensePin4 = 1869
    // - Grid earrings V1.1 vsensePin3 = 3209 vsensePin4 = 4095
    // - Grid earrings V1.1.2 vsensePin3 = 528 vsensePin4 = 1583
    uint32_t vsensePin4 = analogRead(4);
    uint32_t vsensePin3 = analogRead(3);

    // Firstly identify Grid Earrings V1.1 (pin 3 is vsense in this case and should have an ADC level > 1200)
    if (vsensePin3 > 1200)
    {
        _hardwareRevision = HW_IS_GRID_EARRINGS_V1_1;
    }
    else 
    {
        // Check for Heart or Grid V1.1.2 (pin 4 is vsense in this case and should have an ADC level > 1200)
        if (vsensePin4 > 1200)
        {
            // Check for PIX_EN on GPIO 5 - do this by setting a pull-down and checking the analog level and then 
            // setting a pull-up and checking the analog level
            pinMode(5, INPUT_PULLDOWN);
            delay(5);
            uint32_t pin5PullDown = digitalRead(5);
            pinMode(5, INPUT_PULLUP);
            delay(5);
            uint32_t pin5PullUp = digitalRead(5);
            pinMode(5, INPUT);
            ESP_LOGI(MODULE_PREFIX, "detectHardware() pin5PullDown %d pin5PullUp %d", 
                        (int)pin5PullDown, (int)pin5PullUp);

            if (pin5PullDown == 0 && pin5PullUp == 1)
            {
                _hardwareRevision = HW_IS_GRID_EARRINGS_V1_1_2;
            }
            else
            {
                _hardwareRevision = HW_IS_HEART_EARRINGS;
            }
        }
    }

    ESP_LOGI(MODULE_PREFIX, "detectHardware() vsensePin3 %d vsensePin4 %d returning %s (%d)", 
                (int)vsensePin3, (int)vsensePin4, 
                getHWRevisionStr(_hardwareRevision), 
                _hardwareRevision);
    return _hardwareRevision;
}
