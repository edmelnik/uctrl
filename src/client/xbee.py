import serial
import time
import sys
from digi.xbee.devices import XBeeDevice
from digi.xbee.io import IOLine, IOMode

def connect():
    zigbee_device = XBeeDevice('/dev/ttyUSB0', 230400)
    REMOTE_NODE_ID = "Sender"
    zigbee_device.open()
    xbee_network = zigbee_device.get_network()
    remote_device = xbee_network.discover_device(REMOTE_NODE_ID)
    return(zigbee_device)
zigbee_device=connect()

def sendData(values):
    output = ""
    try:
        for value in values:
            output += value
            output += " "
            zigbee_device.send_data(remote_device, output)
        return 1
    except IndexError:
        return -1
##def error(return -1 from xbee)
##    zigbee_device.close()
##    time.sleep(1)
##    zigbee_device.open()
    
