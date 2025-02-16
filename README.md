# JewelOS - Minimal OS for Jewelry

ESP32-C3 based minimal OS for Jewelry.

To build install the Raft CLI (https://github.com/robdobsn/RaftCLI)

Then run:

raft run -p <SERIALPORT>

## Devices Log

| Date | Serial | PCB Version | Notes |
|------|--------|-------------|-------|
| 2025-02-15 | 0103 | V1.5 | Seems to allow power off without any mods? BLE performance is poor. |
| 2025-02-15 | 0104 | V1.5 | Modified to remove Q4 and D5 with a wire between CHG_IN and VBAT. Turns off ok but needs power button held down to program. |
