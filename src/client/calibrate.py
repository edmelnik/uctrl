'''
Continuously reads calibrate.conf and sends calibration and power ON/OFF codes to O2 sensors when activated

Default values for calibration and power ensure that in the event of false activation with wrong codes
- Default code for calibration is 0000 (don't calibrate anything by default)
- Default code for power is 1111 (don't power off anything by default)
Codes need to be reset by this script after action has been taken. Success of the action is not evaluated by this script (this script won't know whether or not the sensors have turned off or have been calibrated)

Device value in the calibrate.conf should be written by the client. This seems a bit hacky..
'''

import wiringpi as wp
import time
import subprocess
import configparser

HIGH = 1
LOW = 0
CONF = "calibrate.conf"
CHK_DELAY = 100 # num cycles to wait for checking changes in addr, signal_length, pin

# From conf
SIGNAL_LENGTH = 4 # how long to hold the GPIO pin high
PIN = 7
DEVICE = "/dev/ttyACM0"


wp.wiringPiSetupSys()
wp.pinMode(PIN, 1)
wp.digitalWrite(PIN, HIGH)

k = 0

def power(code):
    pass

def calibrate(code):
    args = "echo " + code + " > " + DEVICE
    print("Sending calibration code " + code + " to address " + DEVICE) # TODO log
    wp.digitalWrite(PIN, LOW)
    epoch = time.time()
    curr_time = time.time()
    while curr_time - epoch <= SIGNAL_LENGTH:
        subprocess.Popen(args, shell=True)
        time.sleep(2)
        curr_time = time.time()
    wp.digitalWrite(PIN, HIGH)
    
def main():
    global DEVICE, PIN, SIGNAL_LENGTH
    config = configparser.ConfigParser()
    config.read(CONF)
    DEVICE = config['device']['addr']
    PIN = config['device']['pin']
    SIGNAL_LENGTH = config['device']['signal_length']
    
    while True:
        config = configparser.ConfigParser()
        config.read(CONF)
        if config['calibration']['activate'] == '1':
            calibrate(config['calibration']['code'])
        if config['power']['activate'] == '1':
            power(config['power']['code'])

        if(k==CHK_DELAY):
            DEVICE = config['device']['addr']
            SIGNAL_LENGTH = config['device']['signal_length']
            pin = config['device']['pin']
            
        k+=1
        k%=(CHK_DELAY+1)
        time.sleep(2)
        
if __name__ == '__main__':
    main()
