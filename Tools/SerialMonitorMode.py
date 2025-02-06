import logging
import sys
# import os
import subprocess as sp
# import io
from enum import Enum
from typing import NamedTuple

logging.basicConfig(format='%(levelname)-8s [%(filename)s:%(lineno)d] %(message)s', level=logging.WARNING)
logger = logging.getLogger(__name__)

class SerialMonitorMode:
    actualMode = None
    GDBServer_port = None

    class Mode(NamedTuple):
        code: str
        message: str
    
    ESP32_115200 = Mode('0', 'UART_ESP_PASSTHROUGH_115200')
    F407 = Mode('1', 'UART_407_PASSTHROUGH')
    ESP32_230400 = Mode('2', 'UART_ESP_PASSTHROUGH_230400')
    ASEBA = Mode('3', 'ASEBA_CAN_TRANSLATOR')
    TEMPORARY_OFFSET = 4

    MESSAGE_TIMEOUT = ""

    def __init__(self, port):
        self.GDBServer_port = port

    def exitErrorTimeoutEpired(self, timeout):
        logger.error(f'\n    Your selected GDB-Server port is connected but timeout of {timeout} seconds has expired then check: \n\
    - if the GDB-Server port ("com_port": "{self.GDBServer_port}") in \n\
      ".vscode/settings.json" of your "LIB" project is correct \n\
    - or ask for support if the problem persists despite meeting the requirements \n')
        sys.exit(1)

    def exitNoToolError(self):
        logger.error(f'\n    -> "arm-none-eabi-gdb" is not found. \n\
        - run this task from the "Tools: Update ESP32 firmware" task \n\
          of the "LIB" project inside VSCode for e-puck2 IDE \n\
        - or ask for support if the problem persists despite meeting the requirements \n')
        sys.exit(1)

    def exitGDBServerPortError(self):
        logger.error(f'\n    -> Your GDB-Server port "{self.GDBServer_port}" can not be opened. \n\
        - Check if the e-puck2 is well connected \n\
        - If called from VSCode task the check if the GDB-Server port ("com_port": "{self.GDBServer_port}") \n\
          in ".vscode/settings.json" of your "LIB" project is correct \n\
        - Check if this GDB-Server port is not already opened by another process \n')
        sys.exit(1)

    def exitOtherError(self, stderr):
        logger.error(f'\n    -> Problem to switch the SerialMonitor. \n\
        - Check if this result message between the 2 "*** ... ***" lines explains the problem or ask for support: \n\
    \n\n*****************   Begin of Result message   ***************** \n\
    {stderr}.decode("utf-8") \n\
    \n\n*****************   End of Result message   ***************** \n')
        sys.exit(1)

    def powerOff(self):
        try:
            res = sp.run(f'arm-none-eabi-gdb -q -ex "target extended-remote {self.GDBServer_port}" -ex "mon pwr OFF" -ex "quit"', shell=True, capture_output=True, timeout=2)
        except sp.TimeoutExpired as t:
            exitErrorTimeoutEpired(t.timeout)

    def powerOn(self):
        try:
            res = sp.run(f'arm-none-eabi-gdb -q -ex "target extended-remote {self.GDBServer_port}" -ex "mon pwr ON" -ex "quit"', shell=True, capture_output=True, timeout=2)
        except sp.TimeoutExpired as t:
            exitErrorTimeoutEpired(t.timeout)

    def selMode(self, code, temporary=False):
        actualMode = self.getMode()

        if self.actualMode.code == code:
            print(f'  ..\n  -> Actual code of mode is already {code} \n')
        else:
            try:
                if temporary:
                    res = sp.run(f'arm-none-eabi-gdb -q -ex "target extended-remote {self.GDBServer_port}" -ex "mon se {int(code)+self.TEMPORARY_OFFSET}" -ex "quit"', shell=True, capture_output=True, timeout=2)
                else:
                    res = sp.run(f'arm-none-eabi-gdb -q -ex "target extended-remote {self.GDBServer_port}" -ex "mon se {int(code)}" -ex "quit"', shell=True, capture_output=True, timeout=2)

            except sp.TimeoutExpired as t:
                errorTimeoutEpired(t.timeout, res.stderr)
            print(f'  ..\n  -> {res.stderr.decode("utf-8")}')
            if res.returncode != 0:
                exitNoToolError()
            elif res.stderr.strip().count((f"{self.GDBServer_port}: No such file or directory.").encode('utf-8')) == 1:
                self.exitGDBServerPortError()
            elif (code == self.ESP32_115200.code) & (res.stderr.strip().decode('utf-8').count(f"mode {code} -> {self.ESP32_115200.message}") == 1):
                print('  -> SerialMonitor switched to ESP32_115200 \n')
            elif (code == self.F407.code) & (res.stderr.strip().decode('utf-8').count(f"mode {code} -> {self.F407.message}") == 1):
                print('  -> SerialMonitor switched to F407 \n')
            elif (code == self.ESP32_230400.code) & (res.stderr.strip().decode('utf-8').count(f"mode {code} -> {self.ESP32_230400.message}") == 1):
                print('  -> SerialMonitor switched to ESP32_230400 \n')
            elif (code == self.ASEBA.code) & (res.stderr.strip().decode('utf-8').count(f"mode {code} -> {self.ASEBA.message}") == 1):
                print('  -> SerialMonitor switched to ASEBA \n')
            else:
                exitOtherError(res.stderr)

    def getMode(self):
        try:
            res = sp.run(f'arm-none-eabi-gdb -q -ex "target extended-remote {self.GDBServer_port}" -ex "mon get" -ex "quit"', shell=True, capture_output=True, timeout=2)

        except sp.TimeoutExpired as t:
            self.exitErrorTimeoutEpired(t.timeout)
        if res.returncode != 0:
            self.exitNoToolError()
        elif res.stderr.strip().count((f"{self.GDBServer_port}: No such file or directory.").encode('utf-8')) == 1:
            self.exitGDBServerPortError()
        elif res.stderr.strip().count(('mode 0 -> UART_ESP_PASSTHROUGH_115200').encode('utf-8')) == 1:
            mode = SerialMonitorMode.ESP32_115200
        elif res.stderr.strip().count(('mode 1 -> UART_407_PASSTHROUGH').encode('utf-8')) == 1:
            mode = SerialMonitorMode.F407
        elif res.stderr.strip().count(('mode 2 -> UART_ESP_PASSTHROUGH_230400').encode('utf-8')) == 1:
            mode = SerialMonitorMode.ESP32_230400
        elif res.stderr.strip().count(('mode 3 -> ASEBA_CAN_TRANSLATOR').encode('utf-8')) == 1:
            mode = SerialMonitorMode.ASEBA
        else:
            logger.error('\n    Problem to get the SerialMonitor switch. Ask for support! \n')
            sys.exit(1)
        self.actualMode = mode
        # return str(res.stderr.decode('utf-8'))
        return str(res.stderr.split(b'\n')[0].decode('utf-8'))

if __name__ == '__main__':
    if(len(sys.argv) == 1):
        logger.error('\n\tNo argument provided: You must specify GDB_Server port name at minima! \n')
        sys.exit(1)

    GDBServer_port = sys.argv[1]

    logger.debug(f'\nPython script "{sys.argv[0]}" called as main program with parameters: \n  GDBServer_port = {GDBServer_port}')

    smm = SerialMonitorMode(GDBServer_port)
    print(f'  ..\n  {smm.getMode()}')

    temporary = (len(sys.argv) == 4) and (sys.argv[3] == "True")
    
    if(len(sys.argv) >= 3):
        code = sys.argv[2]
        print(f'  ..\n  -> Asked : Code of mode = {code} and Temporary = {temporary}')
        if (code == smm.ESP32_115200.code):
            smm.selMode(code, temporary)
        elif (code == smm.F407.code):
            smm.selMode(code, temporary)
        elif code == smm.ESP32_230400.code:
            smm.selMode(code, temporary)
        elif code == smm.ASEBA.code:
            smm.selMode(code, temporary)
        else:
            print("\nYou must specify a code between 0..3 according to:\n\n\t0 = Programming/serial monitor of the ESP at 115200 bauds and GDB over USB,\n\t1 = Serial monitor of the main processor at 115200 bauds and GDB over USB and Bluetooth,\n\t2 = Programming/serial monitor of the ESP at 230400 baud and GDB over USB,\n\t3 = ASEBA CAN-USB translator and GDB over USB and Bluetooth\n\nAdd True or False to activate temporarly or not this mode\n\nIf it's temporarly then it will be lost when robot is power off but that's avoid to unnecessarily modify flash.\n")

