Actuellement, l'ESP32 de l'e-puck2 est programmé depuis l'environnement de programmation esp-idf et étant dans le projet, il faut utiliser la commande :
> ```shell
>   make flash -p /dev/cu.usbmodemEPUCK3
> ```

Si la programmation ne s'effectue pas correctement, il faut peut être au préalable effacer totalement la flash avec la commande :
> ```shell
>   make erase_flash -p /dev/cu.usbmodemEPUCK3
> ```

Tiré des outils esp-idf, soit le fichier `/Users/danielburnier/EPFL-Mobots/Projects/esp/esp-idf/components/esptool_py/Makefile.projbuild` :

flash:
python esptool.py --chip esp32 $(ESPTOOLPY_WRITE_FLASH) --port /dev/cu.usbmodemPUCK3 --baud 115200 --before "default_reset" --after "hard_reset" write_flash

write_flash -z 
erase_flash
(ESPTOOL_ALL_FLASH_ARGS)

ESPTOOLPY_SRC := $(COMPONENT_PATH)/esptool/esptool.py
ESPTOOLPY := $(PYTHON) $(ESPTOOLPY_SRC) --chip esp32
ESPTOOLPY_SERIAL := $(ESPTOOLPY) --port $(ESPPORT) --baud $(ESPBAUD) --before $(CONFIG_ESPTOOLPY_BEFORE) --after $(CONFIG_ESPTOOLPY_AFTER)

ESPTOOLPY_WRITE_FLASH=$(ESPTOOLPY_SERIAL) write_flash $(if $(CONFIG_ESPTOOLPY_COMPRESSED),-z,-u) $(ESPTOOL_WRITE_FLASH_OPTIONS)

Au final, la commande pour programmer le firmware de l'ESP32 est :
>```shell
>python -m esptool --chip esp32 --port /dev/cu.usbmodemEPUCK3 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 /Users/danielburnier/EPFL-Mobots/Projects/esp/esp-idf/Projects/ESP32_E-Puck_2/build/bootloader/bootloader.bin 0x10000 /Users/danielburnier/EPFL-Mobots/Projects/esp/esp-idf/Projects/ESP32_E-Puck_2/build/ESP32_E-Puck_2.bin 0x8000 /Users/danielburnier/EPFL-Mobots/Projects/esp/esp-idf/Projects/ESP32_E-Puck_2/build/partitions_singleapp.bin
>```


Il faut donc installer le paquet esptool avec python :
>```shell
>   python -m pip install esptool
>```

Il faut récupérer la version de esptool afin de pouvoir downloader espfuse.py :
>```shell
>   python -m esptool version

https://github.com/espressif/esptool/blob/v4.7.0/espefuse.py

/bin/bash -c "$(curl -fsSL https://github.com/espressif/esptool/blob/v${$(python -m pip list | grep esptool)/esptool/}/espfuse.py)"

https://github.com/espressif/esptool/blob/v${${$(python -m pip list | grep esptool)}/esptool/}/espfuse.py

echo https://github.com/espressif/esptool/blob/v${$(python -m pip list | grep esptool)/esptool/}/espfuse.py



- import esptool

- esp = esptool.detect_chip(port='/dev/cu.usbmodemEPUCK3', baud=115200, connect_mode='default_reset', trace_enabled=False, connect_attempts=7)
    Connecting....
    Detecting chip type... Unsupported detection protocol, switching and trying again...
    Connecting.....
    Detecting chip type... ESP32
    <esptool.targets.esp32.ESP32ROM object at 0x10e8f7210>

- esp.DEFAULT_PORT = '/dev/cu.usbmodemEPUCK3'
- esp.get_chip_description()
- esp.get_chip_features()
