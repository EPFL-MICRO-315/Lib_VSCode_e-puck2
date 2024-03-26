import logging
import time
import platform
import shutil
import sys
import os
import subprocess as sp
import io
from enum import Enum

os_name = platform.system()

class SerialMode(Enum):
    F407 = 1
    ESP32 = 2

SerialMessage = [
    'UART_407_PASSTHROUGH',
    'UART_ESP_PASSTHROUGH'
]

def setSerialMonitorMode(mode):

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

def getSerialMonitorMode(mode):

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
    elif res.stderr.strip().count(("Switched to mode 2 : UART_ESP_PASSTHROUGH").encode('utf-8')) == 1:
            logging.info("SerialMonitor switched to ESP32.")
    else:
            logging.error("Problem to switch SerialMonitor. Ask for support!")
            sys.exit(1)


# if os_name == "Darwin":
#     monitor_urlEdit.setText("https://projects.gctronic.com/epuck2/monitor_mac.zip")
# elif os_name == "Windows":
#     monitor_urlEdit.setText("https://projects.gctronic.com/epuck2/monitor_win.zip")
# elif os_name == "Linux":
#     monitor_urlEdit.setText("https://projects.gctronic.com/epuck2/monitor_linux64bit.tar.gz")

if __name__ == '__main__':

    logging.basicConfig(stream=sys.stdout, level=logging.INFO)


    if(len(sys.argv) <= 1):
        logging.error(" No argument provided (GDB Server port name)")
        sys.exit(1)

    GDBServer_port = sys.argv[1]
    SerialMonitor_port = sys.argv[2]

    logging.info(f"\n\n\
    Python script {sys.argv[0]} called with parameters: \n\
         GDBServer_port = {GDBServer_port} \n\
         SerialMonitor_port = {SerialMonitor_port} \n")

# Change SerialMonitor redirection to ESP32 instead F407

# # Check if esptool is already installed and install it if necessary
#     res = sp.run("python3 -m pip list", shell=True, text=True, capture_output=True)
#     if res.stdout.strip().count("esptool") == 0:
#         res = sp.run("python3 -m pip install", shell=True, text=True, capture_output=True)
#         if res.stdout.strip().splitlines()[0].count("Successfully installed esptool") == 1:
#             logging.info("esptool has been successfully installed.")       
#         elif res.stdout.strip().splitlines()[0].count("Requirement already satisfied: esptool") == 1:
#             logging.info("esptool was already installed.")                        
#         else:
#             logging.error("Problem to install esptool. Ask for support!")
#             sys.exit(1)
#     else:
#         logging.info("esptool was already installed.")                        
        
# Download esptool
    if os.path.exists('esptool'):
        logging.info("esptool already present but erase and reinstall\n")
        shutil.rmtree('esptool')
    logging.info("Clone esptool\n")
    res = sp.run('git clone --recurse-submodules https://github.com/espressif/esptool', shell=True, text=True, capture_output=True)

# Program the ESP32
    # cmd = f'python3 -c esptool/esptool.py --chip esp32 --port {SerialMonitor_port} --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x10000 ESP32_E-Puck_2.bin 0x8000 partitions_singleapp.bin'
    cmd = f'python3 esptool/esptool.py --chip esp32 --port {SerialMonitor_port} --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x10000 ESP32_E-Puck_2.bin 0x8000 partitions_singleapp.bin'
    os.environ["PYTHONUNBUFFERED"] = "1"
    Output = io.StringIO()
    process = sp.Popen(cmd.split(), shell=True, stdout=sp.PIPE, stderr=sp.PIPE, text=True)

    while process.poll() is None:
        out = process.stdout.read(1)
        if out != '':
            print(out, end='')
            Output.write(out)

    # Wait for the subprocess to finish and get its return code
    return_code = process.wait()
    print(f"Subprocess returned with exit code: {return_code}")
    input('Display stderr:')
    process.stderr
    input('Display Output:')
    Output.getvalue()
    Output.close()
    # os.system(f'python3 esptool/esptool.py --chip esp32 --port {SerialMonitor_port} --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x10000 ESP32_E-Puck_2.bin 0x8000 partitions_singleapp.bin')
    #     logging.error(f' \n\n\
    # Your selected GDB-Server port is connected but timeout of {t.timeout} seconds has expired! \n\
    #     - Check if the GDB-Server port ("com_port": "{GDBServer_port}") in ".vscode/settings.json" of your "LIB" project is correct! \n')
    #     sys.exit(1)

    sys.exit()
# Change SerialMonitor redirection to F407
    try:
        res = sp.run(f'arm-none-eabi-gdb -q -ex "target extended-remote {GDBServer_port}" -ex "mon se 1" -ex "quit"', shell=True, capture_output=True, timeout=2)

    except sp.TimeoutExpired as t:
        logging.error(f' \n\n\
    Your selected GDB-Server port is connected but timeout of {t.timeout} seconds has expired! \n\
        - Check if the GDB-Server port ("com_port": "{GDBServer_port}") in ".vscode/settings.json" ofgetget_port_list your "LIB" project is correct! \n')
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
    elif res.stderr.strip().count(("Switched to mode 1 : UART_407_PASSTHROUGH").encode('utf-8')) == 1:
            logging.info("SerialMonitor switched to ESP32.\n")
    else:
            logging.error("Problem to switch SerialMonitor. Ask for support!\n")
            sys.exit(1)

# Usefull python commands:
# import subprocess as sp
# proc = sp.Popen( ['arm-none-eabi-gdb'], stdin=sp.PIPE, stdout=sp.PIPE, stderr=sp.STDOUT)
# proc.stdin.write(b'tar ext /dev/cu.usbmodemEPUCK1\n')
# proc.stdout.readline()

# import io
# myFile = io.StringIO("tar ext /dev/cu.usbmodemEPUCK1\nmon se 1\nquit\n")
# myFile.getvalue()
