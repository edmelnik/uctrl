'''
Continuously reads calibrate.conf and sends calibration and power ON/OFF codes to O2 sensors when activated

Default values for calibration and power ensure that in the event of false activation with wrong codes
- Default code for calibration is 0000 (don't calibrate anything by default)
- Default code for power is 1111 (don't power off anything by default)
Codes are reset by this script after action has been taken. Success of the action is not evaluated by this script (this script won't know whether or not the sensors have turned off or have been calibrated)

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
# SIGNAL_LENGTH = 4 # how long to hold the GPIO pin high
# PIN = 7
# DEVICE = "/dev/ttyACM0"

def power(code):
    pass

def calibrate(code, device, pin, signal_length):
    args = "echo " + code + " > " + device
    print("Sending calibration code " + code + " to address " + device) # TODO log
    wp.digitalWrite(pin, LOW)
    epoch = time.time()
    curr_time = time.time()
    while curr_time - epoch <= signal_length:
        subprocess.Popen(args, shell=True)
        time.sleep(2)
        curr_time = time.time()
    wp.digitalWrite(pin, HIGH)
    
def main():        
    config = configparser.ConfigParser()
    config.read(CONF)
    device = config['device']['addr']
    pin = int(config['device']['pin'])
    signal_length = int(config['device']['signal_length'])

    wp.wiringPiSetupSys()
    wp.pinMode(pin, 1)
    wp.digitalWrite(pin, HIGH)
    k=0
    while True:
        config = configparser.ConfigParser()
        config.read(CONF)
        if config['calibration']['activate'] == '1':
            code = config['calibration']['code']
            calibrate(code, device, pin, signal_length)
            # Reset config activation to 0
            config['calibration']['activate'] = '0'
            config['calibration']['code'] = '0000'
            with open(CONF, 'w') as updated_conf:
                config.write(updated_conf)
        if config['power']['activate'] == '1':
            power(config['power']['code'])

        if k==CHK_DELAY:
            device = config['device']['addr']
            signal_length = int(config['device']['signal_length'])
            pin = int(config['device']['pin'])
            
        k+=1
        k%=(CHK_DELAY+1)
        time.sleep(2)
        
# TODO Add try/except block here
while True:
    main()
