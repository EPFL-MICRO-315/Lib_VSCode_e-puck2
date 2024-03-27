import logging
import time
import platform
import shutil
import sys
import os
import subprocess as sp
import io
from enum import Enum
import SerialMonitorMode as SMM

os_name = platform.system()

# if os_name == "Darwin":
#     monitor_urlEdit.setText("https://projects.gctronic.com/epuck2/monitor_mac.zip")
# elif os_name == "Windows":
#     monitor_urlEdit.setText("https://projects.gctronic.com/epuck2/monitor_win.zip")
# elif os_name == "Linux":
#     monitor_urlEdit.setText("https://projects.gctronic.com/epuck2/monitor_linux64bit.tar.gz")

if __name__ == '__main__':

    logging.basicConfig(stream=sys.stdout, level=logging.INFO)

    if(len(sys.argv) <= 1):
        logger.error('\n\
    No argument provided: You must specify in this order: \n\
        - GDB_Server port \n\
        - SerialMonitor port \n')
        sys.exit(1)

    GDBServer_port = sys.argv[1]
    SerialMonitor_port = sys.argv[2]

    logging.info(f"\n\
    Python script {sys.argv[0]} called with parameters: \n\
         GDBServer_port = {GDBServer_port} \n\
         SerialMonitor_port = {SerialMonitor_port} \n")

# Change SerialMonitor redirection to ESP32 instead F407
    smm = SMM.SerialMonitorMode(GDBServer_port)
    smm.selMode(smm.ESP32)

# Download esptool
    if os.path.exists('esptool'):
        logging.info("\n    esptool already present but erase and reinstall\n")
        shutil.rmtree('esptool')
    logging.info("\n    Clone esptool\n")
    res = sp.run('git clone --recurse-submodules https://github.com/espressif/esptool', shell=True, text=True, capture_output=True)
    # ToDo : Check the cloning

# Program the ESP32
    # cmd = f'python3 -c esptool/esptool.py --chip esp32 --port {SerialMonitor_port} --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x10000 ESP32_E-Puck_2.bin 0x8000 partitions_singleapp.bin'
    cmd = f'python3 esptool/esptool.py --chip esp32 --port {SerialMonitor_port} --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x10000 ESP32_E-Puck_2.bin 0x8000 partitions_singleapp.bin'
    os.environ["PYTHONUNBUFFERED"] = "1"
    Output = io.StringIO()
    logging.info(f'\n    cmd = {cmd} \n')

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

# Change SerialMonitor redirection to F407
    smm.selMode(smm.F407)

# Usefull python commands:
# import subprocess as sp
# proc = sp.Popen( ['arm-none-eabi-gdb'], stdin=sp.PIPE, stdout=sp.PIPE, stderr=sp.STDOUT)
# proc.stdin.write(b'tar ext /dev/cu.usbmodemEPUCK1\n')
# proc.stdout.readline()

# import io
# myFile = io.StringIO("tar ext /dev/cu.usbmodemEPUCK1\nmon se 1\nquit\n")
# myFile.getvalue()
