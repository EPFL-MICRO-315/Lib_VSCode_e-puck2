Voici les résultats de la commande `python -m serial.tools.list_ports -v e-puck2` sur les divers OS :

1. Windows 11
    ```
    COM3
        desc: USB Serial Device (COM3)
        hwid: USB VID:PID=0483:5740 SER=301 LOCATION=1-2.3
    COM4
        desc: USB Serial Device (COM4)
        hwid: USB VID:PID=1D50:6018 SER=EPUCK LOCATION=1-1:x.2
    COM5
        desc: USB Serial Device (COM5)
        hwid: USB VID:PID=1D50:6018 SER=EPUCK LOCATION=1-1:x.0
    3 ports found
    ```
2. Windows 10
    ```
    COM3
        desc: USB Serial Device (COM3)
        hwid: USB VID:PID=1D50:6018 SER=EPUCK LOCATION=1-2.4:x.0
    COM4
        desc: USB Serial Device (COM4)
        hwid: USB VID:PID=1D50:6018 SER=EPUCK LOCATION=1-2.4:x.2
    COM5
        desc: USB Serial Device (COM5)
        hwid: USB VID:PID=0483:5740 SER=301 LOCATION=1-2.5:x.2
    3 ports found
    ````
3. Linux Fedora 38
    ```
    Filtered list with regexp: 'e-puck2'
    /dev/ttyACM0        
        desc: e-puck2 - e-puck2 GDB Server
        hwid: USB VID:PID=1D50:6018 SER=EPUCK LOCATION=1-2.1:1.0
    /dev/ttyACM1        
        desc: e-puck2 - e-puck2 Serial Monitor
        hwid: USB VID:PID=1D50:6018 SER=EPUCK LOCATION=1-2.1:1.2
    /dev/ttyACM2        
        desc: e-puck2 STM32F407
        hwid: USB VID:PID=0483:5740 SER=301 LOCATION=1-2.2:1.0
    3 ports found
    ```
3. MacOS 15.2
    ```
    ...
    /dev/cu.usbmodem3011
        desc: e-puck2 STM32F407
        hwid: USB VID:PID=0483:5740 SER=301 LOCATION=20-2.2
    /dev/cu.usbmodemEPUCK1
        desc: e-puck2
        hwid: USB VID:PID=1D50:6018 SER=EPUCK LOCATION=20-2.1
    /dev/cu.usbmodemEPUCK3
        desc: e-puck2
        hwid: USB VID:PID=1D50:6018 SER=EPUCK LOCATION=20-2.1
    ...
    x ports found
    ```
Il est donc tout à fait possible de discriminer les ports séries de l'e-puck2 avec cette commande, quel que soit l'OS.
Par contre il n'est pas possible sur tous les OS de retrouver nominativement les ports série comme ils sont pourtant défini dans les firmwares de l'e-puck2, à savoir :
1. e-puck2 GDB Server, identifiable par SER=EPUCK et 
2. e-puck2 Serial Monitor
3. e-puck2 STM32F407

Et c'est bien toujours le device SER=EPUCK ayant la plus petite LOCATION qui est utilisé pour la communication GDB Server avec l'e-puck2, l'autre étant pour Serial Monitor.
