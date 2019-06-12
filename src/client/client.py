'''
This script reads microcontroller output on serial port 
Expected data format is bytestring of individual sensor values seperated by whitespaces. Each datapoint is seperated by a newline (\n) and return carriage (\r) . Ex. (for n data points) input will be: 

b'D1 D2 D3 ... Dn\n\r'

For each sensor Di

Each datapoint is output by this script as a list of strings:

[TIMESTAMP D1 D2 D3 ... Dn]

Configuration of n is done on the microcontroller. This script should work as expected regardless of how many sensors are actually connected to the microcontroller

ERR* indicates errors; Error numbers denote the following: (FOR PRESSURE ONLY)
ERR1: Chip stuck in command mode
ERR2: Stale data (data has been already recieved)
ERR3: Sensor diagnostic fault
ERR4: Sensor missing (check the PCB if sensor is expected to be in place)

'''

'''
TODO Now that oxygen is connected and works, this script (especially the errors) should be generalized to input and output arbitrary sensor data
TODO Add logging of errors and other messages (TIMEOUT, connecting to device on address)
TODO use logic level converter to output on serial pins as well
TODO Add reset condition if 3 or more timeouts detected
  - Number of timeouts to reset could be a config option
TODO Find the reason for periodic timeouts - for some reason communication seems to stop and the port needs to be restarted in order for it to work
 - This seems to be related with the number of devices the microcontroller is serving
TODO Inspect why error numbers ERR* seem to periodically disappear (itoa problem?)
DONE A better way of reading what to do with the collected datapoints might be to have a config file in the same directory as this script: the config can allow various operations
  - print data to STDOUT
  - push data to zigbee
  - put data into SQL/Influx
  - put data into a csv file
  The output data format could also be specified in the config
  The source for each of these actions should be outside this file and imported by this file using ConfigParser (catch errors if import fails)
DONE Add timestamps to data
DONE if ACM0 does no exist, catch that exception and try other ttyACM*
DONE ignore all recieved data for the first 2 seconds
'''

'''
NOTES and thoughts
- error logging should be smart: every error occurence should not be necessarily logged since in the main while loop the error would occur many times a second. can use a timer to time error occurence frequency (is there a module for this?)
- many different things are now being done in this file that should probably be split into their own files
'''

import serial
import time
import sys
import configparser
import importlib

# temp import, this should be done dynamically
import stdout
# Output values in config file
STDOUT = 'stdout'
XBEE   = 'zigbee'
INFLUX = 'influx'

OUTPUT_FUNC = 'sendData'

def connect(config):
    connected = False
    curr_dev = 0 # ttyACM* dev number
    dev_addr = config['input']['device']
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
    vals = []
    try:
        curr_time = time.time()
        line = device.readline()
        vals = line.decode('utf-8').rsplit()
        curr_time = str(curr_time)
        curr_time = curr_time[:curr_time.find('.')+3] # truncate to 2 decimal points
        vals.insert(0, curr_time)
        return vals;
    except (serial.SerialTimeoutException, UnicodeDecodeError):
        return vals

def handleTimeout(device):
    print("TIMEOUT") #this should perhaps be logged
    device.close()
    device.open()
    return device

def doOutput(config, values):
    output = config['output']
    for module in output:
        if output[module] == "1":
            try:
                return_status = getattr(sys.modules[module], OUTPUT_FUNC)(values)
            except: # TODO Log
                continue
        
def main():
    config = configparser.ConfigParser()
    config.read("config")    
    device = connect(config)
    
    for output_option in config['output']:
        try:
            importlib.import_module(output_option)
        except ModuleNotFoundError:
            # TODO Log
            continue    
    initData(device)
    while True:
        config.read("config") # Read config for changes
        values = getData(device)
        if len(values) <= 1: # only got the currtime, most likely timeout
            # device = handleTimeout(device)
            pass
        else:
            doOutput(config, values)

# Don't stop even if device gets disconnected            
while True:
    try:
        main()
    except KeyboardInterrupt:
        sys.exit()
    except serial.SerialException:
        continue
    # except:
    #     continue
