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
    virtual void setup(const ConfigBase& config, const char* pConfigPrefix) override final;

    // Service
    virtual void service() override final;

    // Shutdown
    virtual void shutdown() override final;

private:

    // LED grid
    LEDGrid _ledGrid;

    // // Microphone
    // AnalogMicrophone _microphone;
    
};
