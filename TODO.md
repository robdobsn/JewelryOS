
2025/02/15 State of Play

Manufactured V1.5 have two issues:
[] The battery is connected via a P-CH FET which is always on and this means that the power-down circuitry is always on and the battery will be drained. The solution applied in V1.8 schematic is to change the FET to a schottky diode but this may affect charging a little?
[] The selection of pin 8 for driving LEDS3 causes that pin to be pulled low at boot. This isn't an issue normally but pin 8 is a strapping pin and must be pulled high to program. This doesn't seem to stop all devices from being programmed - just some of them - and the pin can be pulled high manually to program the device. The solution applied in V1.8 is to move the LEDS3 to pin 10 and SDA to pin 8.

A bodge for the first issue is to remove diode D5. This means that the power-down circuitry works when on battery but the power button needs to be held down when programming otherwise the device will power down. This is an ok solution for the moment.

In addition in V1.8 the layout has been changed to:
[] create more space around the battery for the shell mounting points
[] improve the RF path for the antenna
[] increase the width of power traces
[] 4 layer PCB

Further hardware work:
[] Still need to resolve the way charging is done in the case - there is very little exposed metal to work with and most battery slide connectors are too big to work - was considering pogo pin connectors with very short throw and a way to introduce the earring on a slope so that it pushes up against the connector rather than going sideways against it
[] Change the sensor to a BGA type which would fit flatter to the ear
[] Consider whether having the antenna touching the ear in some cases matters

Further firmware work:
[] The evaluations|HRMAnalsis|DebugCPPAnalysis folder is intended to compare the output of the hrm algorithms run on the device with the equivalent algorithms run on the PC - it seems to match in performance generally but I think some parameters may differ now - these should be back-ported to the CPP code
[] Collection of raw samples over bluetooth using publishing works well but the samples only include the raw values - it would be good to get at the processed (filter, zero crossing, PLL, etc) parameters if possible - maybe add a device to do this and publish from that
[] The code still includes the older work using a local I2C bus and MAX30101 drivers - this can be removed
[] Investigation of reduced LED intensity or switching to IR only (is this possible) might be worth considering to save battery and reduce glow on earlobe
[] A slow version of the animated LEDs routine might be good for video creation

Older notes:
[] Clear LED pixels on power down
[] BLE connect should stop power saving mode
[] BLE disconnect after timeout and force reconnect
[] get grid message from app
[] allow message colour to be set - inc random colours
[] LED patterns through app
[] microphone
[] log heart rate
[] transfer log data over BLE
[] fix problem with LED strip transaction complete check - seems it doesn't work as on grid earrings when going to sleep after waiting for completion the pixels can get corrupted - maybe just not calling the function at all ? - there is a delay currenting inserted to solve this - look for TODO
[] fix any other TODOs
[] main processor clock rate is not set low
