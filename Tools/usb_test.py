import usb.core
import usb.util

# Rechercher tous les périphériques USB connectés
devices = usb.core.find(find_all=True)

if not devices:
    print("Aucun périphérique USB trouvé.")
else:
    for device in devices:
        print(f"Bus: {device.bus}, Adresse: {device.address}")
        print(f"ID du fabricant: {hex(device.idVendor)}, ID du produit: {hex(device.idProduct)}")
        
        try:
            # Accéder aux descripteurs
            print("  Descripteur du périphérique:")
            print(f"    Classe: {device.bDeviceClass}")
            print(f"    Sous-classe: {device.bDeviceSubClass}")
            print(f"    Protocole: {device.bDeviceProtocol}")
            print(f"    Numéro de série: {usb.util.get_string(device, device.iSerialNumber)}")

            # Accéder aux configurations
            for cfg in device:
                print(f"  Configuration {cfg.bConfigurationValue}:")
                for intf in cfg:
                    print(f"    Interface {intf.bInterfaceNumber}, Classe: {intf.bInterfaceClass}")
                    for ep in intf:
                        print(f"      Endpoint {ep.bEndpointAddress}, Type: {usb.util.endpoint_type(ep.bmAttributes)}")
        except usb.core.USBError as e:
            print(f"Erreur lors de l'accès aux descripteurs: {e}")
        print()
