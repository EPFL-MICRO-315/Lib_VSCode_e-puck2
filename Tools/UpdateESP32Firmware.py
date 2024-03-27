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

    logger = logging.getLogger(__name__)
    logging.basicConfig(stream=sys.stdout, level=logging.INFO)

    if(len(sys.argv) <= 2):
        logger.error('\n\
    Not enough arguments provided: You must specify in this order: \n\
        - GDB_Server port \n\
        - SerialMonitor port \n')
        sys.exit(1)

    GDBServer_port = sys.argv[1]
    SerialMonitor_port = sys.argv[2]

    logger.info(f"\n\
    Python script {sys.argv[0]} called with parameters: \n\
         GDBServer_port = {GDBServer_port} \n\
         SerialMonitor_port = {SerialMonitor_port} \n")

# Change SerialMonitor redirection to ESP32 instead F407
    smm = SMM.SerialMonitorMode(GDBServer_port)
    smm.selMode(smm.ESP32)

# Download esptool
    if os.path.exists('esptool'):
        logger.info("\n    esptool already present but erase and reinstall\n")
        shutil.rmtree('esptool')
    logger.info("\n    Clone esptool\n")
    res = sp.run('git clone --recurse-submodules https://github.com/espressif/esptool', shell=True, text=True, capture_output=True)
    # ToDo : Check the cloning

# Program the ESP32
    cmd = f'python3 esptool/esptool.py --chip esp32 --port {SerialMonitor_port} --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x10000 ESP32_E-Puck_2.bin 0x8000 partitions_singleapp.bin'

# a) Blind variant - Until the realtime variant works
    logger.warning(f' \n\n\
    The ESP32 firmware update is done blindly in order to be able to retrieve the output messages and check the result. \n\
    This can take up to 40 seconds and a timeout is configured to regain control if necessary. \n\
    \n\
                !!!   So please wait   !!! \n')
    res = None
    try:
        res = sp.run(cmd, shell=True, capture_output=True, timeout=40)

    except sp.TimeoutExpired as t:
        logger.error(f' \n\n\
    Timeout of {t.timeout} seconds has expired! \n\
        - Check if this result message between the 2 "*** ... ***" lines explains the problem or ask for support: \n\
\n\n*****************   Begin of Result message   ***************** \n\
{res} \
\n\n*****************   End of Result message   ***************** \n')

        sys.exit(1)
 
    if res.returncode == 2:
        logger.error(f' \n\n\
    Check if the port {GDBServer_port} is correct, the ESP well powered and the USB connection is not interrupted! \n')
        sys.exit(1)
    elif res.stderr.strip().count((f"{GDBServer_port}: No such file or directory.").encode('utf-8')) == 1:
        logger.error(f' \n\n\
    Your GDB-Server port can not be opened. \n\
        - Check if the e-puck2 is well connected \n\
        - Check if the GDB-Server port ("com_port": "{GDBServer_port}") in ".vscode/settings.json" of your "LIB" project is correct! \n\
        - Check if this GDB-Server port is not already opened by another process \n')
        sys.exit(1)
    elif res.stdout.count(b'Hash of data verified.') == 3:
            logger.info('\n    ESP32 firmware well updated.\n')
    else:
            logger.error("Problem to update ESP32 firmware. Ask for support!")
            sys.exit(1)
# b) Real time variant but doesn't work !?
    # os.environ["PYTHONUNBUFFERED"] = "1"
    # Output = io.StringIO()
    # process = sp.Popen(cmd.split(), shell=True, stdout=sp.PIPE, stderr=sp.PIPE, text=True)

    # while process.poll() is None:
    #     out = process.stdout.read(1)
    #     if out != '':
    #         print(out, end='')
    #         Output.write(out)

    # # Wait for the subprocess to finish and get its return code
    # return_code = process.wait()
    # print(f"Subprocess returned with exit code: {return_code}")
    # input('Display stderr:')
    # process.stderr
    # input('Display Output:')
    # Output.getvalue()
    # Output.close()

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
