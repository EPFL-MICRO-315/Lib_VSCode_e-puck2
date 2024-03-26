import logging
import sys
# import os
import subprocess as sp
# import io
from enum import Enum

logger = logging.getLogger(__name__)

SerialMessage = [
    'UART_407_PASSTHROUGH',
    'UART_ESP_PASSTHROUGH'
]

class SerialMonitorMode:
  def __init__(self, port):
    self.GDBServer_port = port

  def myfunc(self):
    print("Hello my name is " + self.name)

def _is_GDBServer_port_defined():
    if GDBServer_port is None:
        logging.error(f' \n\n\
    "GDBServer_port" is undefined. Call init(port) before to use any functions! \n')
        sys.exit(1)


def setSerialMonitorMode(mode):
    _is_GDBServer_port_defined()

    try:
        res = sp.run(f'arm-none-eabi-gdb -q -ex "target extended-remote {GDBServer_port}" -ex "mon se 2" -ex "quit"', shell=True, capture_output=True, timeout=2)

    except sp.TimeoutExpired as t:
        logging.error(f' \n\n\
    Your selected GDB-Server port is connected but timeout of {t.timeout} seconds has expired! \n\
        - Check if the GDB-Server port ("com_port": "{GDBServer_port}") in ".vscode/settings.json" of your "LIB" project is correct! \n')
        sys.exit(1)
 
    if res.returncode != 0:
        logging.error(f' \n\n\
    "arm-none-eabi-gdb" is not found. Run this task from the "Tools: Update ESP32 firmware" task of the "LIB" project inside VSCode for e-puck2 IDE! \n\
    Ask for support if the problem persists despite meeting the requirements! \n')
        sys.exit(1)
    elif res.stderr.strip().count((f"{GDBServer_port}: No such file or directory.").encode('utf-8')) == 1:
        logging.error(f' \n\n\
    Your GDB-Server port can not be opened. \n\
        - Check if the e-puck2 is well connected \n\
        - Check if the GDB-Server port ("com_port": "{GDBServer_port}") in ".vscode/settings.json" of your "LIB" project is correct! \n\
        - Check if this GDB-Server port is not already opened by another process \n')
        sys.exit(1)
    elif res.stderr.strip().count(("Switched to mode 2 : UART_ESP_PASSTHROUGH").encode('utf-8')) == 1:
            logging.info("SerialMonitor switched to ESP32.")
    else:
            logging.error("Problem to switch SerialMonitor. Ask for support!")
            sys.exit(1)

def getSerialMonitorMode():
    _is_GDBServer_port_defined()
    
    try:
        res = sp.run(f'arm-none-eabi-gdb -q -ex "target extended-remote {GDBServer_port}" -ex "mon get" -ex "quit"', shell=True, capture_output=True, timeout=2)

    except sp.TimeoutExpired as t:
        logging.error(f' \n\n\
    Your selected GDB-Server port is connected but timeout of {t.timeout} seconds has expired! \n\
        - Check if the GDB-Server port ("com_port": "{GDBServer_port}") in ".vscode/settings.json" of your "LIB" project is correct! \n')
        sys.exit(1)
 
    if res.returncode != 0:
        logging.error(f' \n\n\
    "arm-none-eabi-gdb" is not found. Run this task from the "Tools: Update ESP32 firmware" task of the "LIB" project inside VSCode for e-puck2 IDE! \n\
    Ask for support if the problem persists despite meeting the requirements! \n')
        sys.exit(1)
    elif res.stderr.strip().count((f"{GDBServer_port}: No such file or directory.").encode('utf-8')) == 1:
        logging.error(f' \n\n\
    Your GDB-Server port can not be opened. \n\
        - Check if the e-puck2 is well connected \n\
        - Check if the GDB-Server port ("com_port": "{GDBServer_port}") in ".vscode/settings.json" of your "LIB" project is correct! \n\
        - Check if this GDB-Server port is not already opened by another process \n')
        sys.exit(1)
    elif res.stderr.strip().count(("Current mode : mode 1 : UART_407_PASSTHROUGH").encode('utf-8')) == 1:
        mode = SerialMode.F407
        return mode
    elif res.stderr.strip().count(("Current mode : mode 2 : UART_ESP_PASSTHROUGH").encode('utf-8')) == 1:
        mode = SerialMode.ESP32
        return mode
    else:
        logging.error("Problem to switch SerialMonitor. Ask for support!")
        sys.exit(1)

