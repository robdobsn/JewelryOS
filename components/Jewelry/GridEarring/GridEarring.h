/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Grid Earring
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "JewelryBase.h"
#include "LEDGrid.h"
// #include "AnalogMicrophone.h"

class GridEarring : public JewelryBase
{
public:

    // Constructor / Destructor
    GridEarring();
    virtual ~GridEarring();

    // Setup
    virtual void setup(const RaftJsonIF& config, DeviceManager& devMan) override final;

    // Service
    virtual void loop() override final;

    // Sleep currently handled within the loop function
    virtual uint32_t getSleepDurationUs()
    {
        return 0;
    }

    // Shutdown
    virtual void shutdown() override final;

private:

    // LED grid
    LEDGrid _ledGrid;

    // // Microphone
    // AnalogMicrophone _microphone;
    
};
