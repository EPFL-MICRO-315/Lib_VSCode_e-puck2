import logging
import sys
# import os
import subprocess as sp
# import io
from enum import Enum
from typing import NamedTuple

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.NOTSET)

class SerialMonitorMode:
    actualMode = None
    GDBServer_port = None

    class Mode(NamedTuple):
        code: str
        message: str
    
    F407 = Mode('1', 'UART_407_PASSTHROUGH')
    ESP32 = Mode('2', 'UART_ESP_PASSTHROUGH')
    ASEBA = Mode('3', 'ASEBA_CAN_TRANSLATOR')

    def __init__(self, port):
        self.GDBServer_port = port

    def selMode(self, mode):
        self.getMode()
        if self.actualMode == mode:
            logger.info(f'\n\
    Actual mode is already: {smm.getMode()} \n')
        else:
            try:
                res = sp.run(f'arm-none-eabi-gdb -q -ex "target extended-remote {self.GDBServer_port}" -ex "mon se {mode.code}" -ex "quit"', shell=True, capture_output=True, timeout=2)

            except sp.TimeoutExpired as t:
                logger.error(f'\n\
    Your selected GDB-Server port is connected but timeout of {t.timeout} seconds has expired! \n\
        - Check if the GDB-Server port ("com_port": "{self.GDBServer_port}") in ".vscode/settings.json" of your "LIB" project is correct! \n')
                sys.exit(1)
        
            if res.returncode != 0:
                logger.error(f'\n\
    "arm-none-eabi-gdb" is not found. Run this task from the "Tools: Update ESP32 firmware" task of the "LIB" project inside VSCode for e-puck2 IDE! \n\
    Ask for support if the problem persists despite meeting the requirements! \n')
                sys.exit(1)
            elif res.stderr.strip().count((f"{self.GDBServer_port}: No such file or directory.").encode('utf-8')) == 1:
                logger.error(f'\n\
    Your GDB-Server port can not be opened. \n\
        - Check if the e-puck2 is well connected \n\
        - Check if the GDB-Server port ("com_port": "{self.GDBServer_port}") in ".vscode/settings.json" of your "LIB" project is correct! \n\
        - Check if this GDB-Server port is not already opened by another process \n')
                sys.exit(1)
            elif (mode == self.ESP32) & (res.stderr.strip().decode('utf-8').count(f"Switched to mode {self.ESP32.code} : {self.ESP32.message}") == 1):
                logger.info('\n    SerialMonitor switched to ESP32. \n')
            elif (mode == self.F407) & (res.stderr.strip().decode('utf-8').count(f"Switched to mode {self.F407.code} : {self.F407.message}") == 1):
                logger.info('\n    SerialMonitor switched to F407. \n')
            elif (mode == self.ASEBA) & (res.stderr.strip().decode('utf-8').count(f"Switched to mode {self.ASEBA.code} : {self.ASEBA.message}") == 1):
                logger.info('\n    SerialMonitor switched to ASEBA. \n')
            else:
                logger.error('\n    Problem to select the SerialMonitor switch. Ask for support! \n')
                sys.exit(1)

    def getMode(self):        
        try:
            res = sp.run(f'arm-none-eabi-gdb -q -ex "target extended-remote {self.GDBServer_port}" -ex "mon get" -ex "quit"', shell=True, capture_output=True, timeout=2)

        except sp.TimeoutExpired as t:
            logger.error(f'\n\
    Your selected GDB-Server port is connected but timeout of {t.timeout} seconds has expired! \n\
        - Check if the GDB-Server port ("com_port": "{self.GDBServer_port}") in ".vscode/settings.json" of your "LIB" project is correct! \n')
            sys.exit(1)
    
        if res.returncode != 0:
            logger.error(f'\n\
    "arm-none-eabi-gdb" is not found. Run this task from the "Tools: Update ESP32 firmware" task of the "LIB" project inside VSCode for e-puck2 IDE! \n\
    Ask for support if the problem persists despite meeting the requirements! \n')
            sys.exit(1)
        elif res.stderr.strip().count((f"{self.GDBServer_port}: No such file or directory.").encode('utf-8')) == 1:
            logger.error(f'\n\
    Your GDB-Server port can not be opened. \n\
        - Check if the e-puck2 is well connected \n\
        - Check if the GDB-Server port ("com_port": "{self.GDBServer_port}") \n\
          in ".vscode/settings.json" of your "LIB" project is correct! \n\
        - Check if this GDB-Server port is not already opened by another process \n')
            sys.exit(1)
        elif res.stderr.strip().count(('Current mode : mode 1 : UART_407_PASSTHROUGH').encode('utf-8')) == 1:
            mode = SerialMonitorMode.F407
        elif res.stderr.strip().count(('Current mode : mode 2 : UART_ESP_PASSTHROUGH').encode('utf-8')) == 1:
            mode = SerialMonitorMode.ESP32
        # WARNING: For ASEBA the message is not identiqually indented !!!! Need to correct the programmer code too later !!
        elif res.stderr.strip().count(('Current mode : mode 3 :ASEBA_CAN_TRANSLATOR').encode('utf-8')) == 1:
            mode = SerialMonitorMode.ASEBA
        else:
            logger.error('\n    Problem to get the SerialMonitor switch. Ask for support! \n')
            sys.exit(1)
        self.actualMode = mode
        return mode

if __name__ == '__main__':
    if(len(sys.argv) == 1):
        logger.error('\n\
    No argument provided: You must specify GDB_Server port name at minima! \n')
        sys.exit(1)

    GDBServer_port = sys.argv[1]

    logger.info(f'\n\
    Python script "{sys.argv[0]}" called as main program with parameters: \n\
         GDBServer_port = {GDBServer_port} \n')

    smm = SerialMonitorMode(GDBServer_port)
    logger.info(f'\n\
    Actual mode : {smm.getMode()} \n')

    if(len(sys.argv) == 3):
        if sys.argv[2] == smm.F407.code:
            smm.selMode(smm.F407)
        elif sys.argv[2] == smm.ESP32.code:
            smm.selMode(smm.ESP32)
        elif sys.argv[2] == smm.ASEBA.code:
            smm.selMode(smm.ASEBA)

