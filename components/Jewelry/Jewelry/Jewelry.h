/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Jewelry
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "SysModBase.h"
#include "PowerControl.h"
#include "JewelryBase.h"

class Jewelry : public SysModBase
{
public:
    Jewelry(const char *pModuleName, ConfigBase &defaultConfig, ConfigBase *pGlobalConfig, ConfigBase *pMutableConfig);
    ~Jewelry() override;

protected:

    // Setup
    virtual void setup() override final;

    // Service (called frequently)
    virtual void service() override final;

private:

    // Debug
    uint32_t _lastDebugTimeMs = 0;

    // Power control
    PowerControl _powerControl;

    // Heart earring
    JewelryBase* _pJewelry = nullptr;
};
