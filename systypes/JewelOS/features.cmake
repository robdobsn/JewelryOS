# Set the target Espressif chip
set(IDF_TARGET "esp32c3")

# System version
add_compile_definitions(SYSTEM_VERSION="2.0.1")

# Enable sleeping between animations
# add_compile_definitions(FEATURE_ENABLE_SLEEP_MODE)

# Enable power control function keeping the board alive
# add_compile_definitions(FEATURE_POWER_CONTROL_KEEP_ALIVE)

# Enable heart LED animations
# add_compile_definitions(FEATURE_HEART_ANIMATIONS)

# Disabled main features
# add_compile_definitions(ETHERNET_HARDWARE_OLIMEX)

# Raft components
set(RAFT_COMPONENTS
    RaftSysMods@main
    RaftI2C@main
)

# File system
set(FS_TYPE "littlefs")
set(FS_IMAGE_PATH "../Common/FSImage")

