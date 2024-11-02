/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Main entry point
//
// JewelOS
// Rob Dobson 2024
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "RaftCoreApp.h"
#include "RegisterSysMods.h"
#include "BusI2C.h"
#include "Jewelry.h"

/// @brief Main entry point for application
extern "C" void app_main(void)
{
    RaftCoreApp raftCoreApp;

    // Register SysMods from RaftSysMods library
    RegisterSysMods::registerSysMods(raftCoreApp.getSysManager());

    // Register BusI2C
#ifdef FEATURE_REGISTER_I2C_BUS_IN_MAIN
    raftBusSystem.registerBus("I2C", BusI2C::createFn);
#endif

    // Jewelry
    raftCoreApp.registerSysMod("Jewelry", Jewelry::create, true);

    // Loop forever
    while (1)
    {
        // Yield for 1 tick
        vTaskDelay(1);

        // Service the app
        raftCoreApp.loop();
    }
}
