# Set the target Espressif chip
set(IDF_TARGET "esp32c3")

# System version
add_compile_definitions(SYSTEM_VERSION="2.0.0")

# Disabled main features
# add_compile_definitions(ETHERNET_HARDWARE_OLIMEX)

# Raft components
set(RAFT_COMPONENTS
    RaftSysMods@ReWorkConfigBase
    RaftI2C@ReWorkConfigBase
)

# File system
set(FS_TYPE "littlefs")
set(FS_IMAGE_PATH "../Common/FSImage")

