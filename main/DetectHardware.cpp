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
    // Pin 3 is Vsense on Grid Earrings and LEDS_2 on Heart Earrings
    // Pin 4 is Vsense on Heart Earrings and PWR_OFF on Grid Earrings
    // With no battery connected but running on charger (i.e. 4.2V):
    // - Heart earrings vsensePin4 = 1869 vsensePin3 = 704
    // - Grid earrings vsensePin4 = 4095 vsensePin3 = 3209
    uint32_t vsensePin4 = analogRead(4);
    uint32_t vsensePin3 = analogRead(3);

    // Check which makes sense
    if (vsensePin3 < 1200)
    {
        if (vsensePin4 < 1200)
            _hardwareRevision = HW_IS_HEART_EARRINGS;
    }
    else 
    {
        _hardwareRevision = HW_IS_GRID_EARRINGS_V1_1;
    }

    ESP_LOGI(MODULE_PREFIX, "detectHardware() vsenseHeart %d vsenseGrid %d returning %s (%d)", 
                (int)vsensePin4, (int)vsensePin3,
                getHWRevisionStr(_hardwareRevision), 
                _hardwareRevision);
    return _hardwareRevision;
}
