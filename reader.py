import serial
'''
TODO Inspect the reason for timeouts - for some reason communication seems to stop and the port needs to be restarted in order for it to work
 - This seems to be related with the number of devices the microcontroller is serving
TODO Inspect why error numbers ERR* seem to periodically disappear
TODO if ACM0 does no exist, catch that exception and try other ttyACM* 
'''

device = serial.Serial('/dev/ttyACM0', 115200, timeout=2);

while True:
    output = ""
    try:
      line = device.readline()
    except serial.SerialTimeoutException:
        continue
    vals = line.decode('utf-8').rsplit()
    if len(vals) == 0: # most likely a timeout
        print("TIMEOUT")
        device.close()
        device.open()
        continue
    try:
        for value in vals:
            output += value
            output += " "
        print(output)
    except IndexError:
        continue
