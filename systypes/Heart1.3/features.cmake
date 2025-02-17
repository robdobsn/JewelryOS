# Set the target Espressif chip
set(IDF_TARGET "esp32c3")

# System version
add_compile_definitions(SYSTEM_VERSION="2.1.0")

# Heart type jewelry
add_compile_definitions(FEATURE_HEART_JEWELRY)

# Enable power control function setup - this will keep the power on indefinitely if
# other power control functions are not enabled
add_compile_definitions(FEATURE_POWER_CONTROL_SETUP)

# Enable sleeping between animations
# add_compile_definitions(FEATURE_ENABLE_SLEEP_MODE)

# Enable power control function check user shutdown - this will check the power
# button for user input to shutdown the device
add_compile_definitions(FEATURE_POWER_CONTROL_USER_SHUTDOWN)

# Enable power control function for low battery shutdown
add_compile_definitions(FEATURE_POWER_CONTROL_LOW_BATTERY_SHUTDOWN)

# Enable heart LED animations
add_compile_definitions(FEATURE_HEART_ANIMATIONS)

# Add I2C bus in main
# add_compile_definitions(FEATURE_REGISTER_I2C_BUS_IN_MAIN)

# Raft components
set(RAFT_COMPONENTS
    RaftSysMods@feature-ble-central
    RaftI2C@features-genericize-bus-devices
)

# File system
set(FS_TYPE "littlefs")
set(FS_IMAGE_PATH "../Common/FSImage")
