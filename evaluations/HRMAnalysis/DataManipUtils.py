

import glob
import os

class DataManipUtils:

    def __init__(self, logFilePath = ""):
        self._logFilePath = logFilePath if logFilePath != "" else os.path.join(os.getcwd(), "../../logs")
        
    def findLatestLogFile(self):
        list_of_files = glob.glob(os.path.join(self._logFilePath, '*.log'))
        latest_file = max(list_of_files, key=os.path.getctime)
        return latest_file

