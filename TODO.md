
[] I2C library being used is not RaftI2C
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
