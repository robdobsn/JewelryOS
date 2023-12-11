/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DetectHardware for Scader PCBs
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <vector>

enum HWRevision {
    HW_IS_GENERIC_BOARD = 1,
    HW_IS_HEART_EARRINGS = 2,
    HW_IS_GRID_EARRINGS_V1_1 = 3,
    };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main detection function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DetectHardware
{
public:
    static int _hardwareRevision;
    static int detectHardware();
    static int getHWRevision()
    {
        if (_hardwareRevision == -1)
            _hardwareRevision = detectHardware();
        return _hardwareRevision;
    }
    static const char* getHWRevisionStr(int hwRev)
    {
        switch (hwRev)
        {
            case HW_IS_GENERIC_BOARD:
                return "Generic";
            case HW_IS_HEART_EARRINGS:
                return "HeartEarrings";
            case HW_IS_GRID_EARRINGS_V1_1:
                return "GridEarrings";
        }
        return "Unknown";
    }
};

