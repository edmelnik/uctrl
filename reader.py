import serial
import time

'''
This script reads microcontroller output on serial port 
Expected data format is bytestring of individual sensor values seperated by whitespaces. Each datapoint is seperated by a newline (\n) and return carriage (\r) . Ex. (for n data points) input will be: 

b'D1 D2 D3 ... Dn\n\r'

For each sensor Di

Configuration of of n is done on the microcontroller. The number of actual sensors on the PCB should be reflected in the NUM_SENSORS global in this script.

ERR* indicates errors; Error numbers denote the following:

ERR1: Chip stuck in command mode
ERR2: Stale data (data has been already recieved)
ERR3: Sensor diagnostic fault
ERR4: Sensor missing (check the PCB if sensor is expected to be in place)

'''

'''
DONE if ACM0 does no exist, catch that exception and try other ttyACM*
DONE ignore all recieved data for the first 2 seconds
TODO Find the reason for periodic timeouts - for some reason communication seems to stop and the port needs to be restarted in order for it to work
 - This seems to be related with the number of devices the microcontroller is serving
TODO Inspect why error numbers ERR* seem to periodically disappear
'''

# device = serial.Serial('/dev/ttyACM0', 9600, timeout=2);

def connect():
    connected = False
    curr_dev = 0 # ttyACM* dev number
    dev_addr = "/dev/ttyACM"
    curr_dev_addr = ""
    while not connected:
        curr_dev_addr = dev_addr + str(curr_dev)
        try:
            device = serial.Serial(curr_dev_addr, 9600, timeout=2)
            connected = True
        except (serial.SerialException, FileNotFoundError) as e:
            curr_dev += 1
            curr_dev %= 10
    return device

def initData(device):
    epoch = time.time()
    curr_time = time.time()
    while curr_time - epoch <= 2:
        getData(device)
        curr_time = time.time()
        
def getData(device):
    try:
        line = device.readline()
        vals = line.decode('utf-8').rsplit()
        return vals;
    except (serial.SerialTimeoutException, UnicodeDecodeError):
        return vals

def handleTimeout(device):
    print("TIMEOUT") #this should perhaps be logged
    device.close()
    device.open()
    return device

def printData(values):
    output = ""
    try:
        for value in values:
            output += value
            output += " "
        print(output)
    except IndexError:
        pass
        
def main():
    device = connect()
    initData(device)
    while True:
        values = getData(device)
        if len(values) == 0:
            device = handleTimeout(device)
        else:
            # do something with data (printing for now)
            printData(values)
            
main()
