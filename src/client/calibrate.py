'''
Continuously reads calibrate.conf and sends calibration and power ON/OFF codes to O2 sensors when activated

Default values for calibration and power ensure that in the event of false activation
- Default code for calibration is 0000 (don't calibrate anything by default)
- Default code for power is 1111 (don't power off anything by default)

'''
wp.wiringPiSetupSys()                                                           
wp.pinMode(PIN, 1)                                                              
wp.digitalWrite(PIN, HIGH)                                                      
                                                                                
def calibrate(sensors):                                                         
    echostr = ''                                                                
    echostr+='"'                                                                
    for i in sensors:                                                           
        if i == 1:                                                              
            echostr+="1"                                                        
        else:                                                                   
            echostr+="0"                                                        
    echostr+='"'                                                                
    args = 'echo '+ echostr + ' > /dev/ttyACM0'                                 
    print("sending signal " + echostr + "..\n")                                 
    wp.digitalWrite(PIN, LOW)                                                   
    epoch = time.time()                                                         
    curr_time = time.time()                                                     
    while curr_time - epoch <= DELAY:                                           
        subprocess.Popen(args, shell=True)                                      
        time.sleep(2)                                                           
        curr_time = time.time()                                                 
    wp.digitalWrite(PIN, HIGH)
    
while True:                                                                     
    inp = str(input("enter calibration code: "))                                
    if(len(inp)<4):                                                             
        continue                                                                
    echolist = []                                                               
    for i in range(len(inp)):                                                   
        if inp[i] == "1":                                                       
            echolist.append(1)                                                  
        elif inp[i] == "0":                                                     
            echolist.append(0)                                                  
        else:                                                                   
            continue                                                            
    calibrate(echolist)   
