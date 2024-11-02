/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Jewelry
//
// Rob Dobson 2023-24
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RaftSysMod.h"
#include "PowerControl.h"
#include "JewelryBase.h"

class APISourceInfo;

class Jewelry : public RaftSysMod
{
public:
    Jewelry(const char *pModuleName, RaftJsonIF& sysConfig);
    virtual ~Jewelry();

    // Create function (for use by SysManager factory)
    static RaftSysMod* create(const char* pModuleName, RaftJsonIF& sysConfig)
    {
        return new Jewelry(pModuleName, sysConfig);
    }

protected:
    // Setup
    virtual void setup() override final;

    // Loop (called frequently)
    virtual void loop() override final;

    // Add endpoints
    virtual void addRestAPIEndpoints(RestAPIEndpointManager& pEndpoints) override final;

    // Status
    virtual String getStatusJSON() const override final;

    // Get named value
    virtual double getNamedValue(const char* valueName, bool& isValid) override final;

private:

    // Debug
    uint32_t _lastDebugTimeMs = 0;

    // Power control
    PowerControl _powerControl;

    // Jewelry
    JewelryBase* _pJewelry = nullptr;

    // Helper functions
    RaftRetCode apiControl(const String &reqStr, String &respStr, const APISourceInfo& sourceInfo);

    // TODO - remove
    int battPC = 0;

    // Debug
    static constexpr const char *MODULE_PREFIX = "Jewelry";
};
