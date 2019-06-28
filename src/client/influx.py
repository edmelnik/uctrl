##see client.py for input details but "values" is passed as a list from client.py
##this output device just gets that in raw for to the cloud
##[TIMESTAMP D1 D2 D3 ... Dn]

##download the influx db python client with "pip install influxdb"

##todo
## Decide if we want to hardcode these as needed, or configure. Probably configure, but at that point it's like what we have now and we basically write the whole shebang
##   might have to add influx config file path to config, or all this shit below. otherwise it'll be a big shit sandwhich of data
## rewrite this all in the form of serieshelper example (smh) https://influxdb-python.readthedocs.io/en/latest/examples.html

from influxdb import InfluxDBClient

###____   SPECIFIC PROJECT CONFIG   ____###
PROJECT_NAME = "o2_uctrl"
TAG_o2 = "4o2avg"
TAG_pressure = "pressure"

###____   INFLUX SERVER CONFIG   ____###
#Uncomment to use Sandbox Server config
HOST = '35.243.148.141'
PORT = 8086
USERNAME = ""
PASSWORD = ""
DATABASE = "influxout_vanilla"

###____   INFLUX SERVER CONFIG   ____###
#Uncomment to use Production InfluxDB Server config
#HOST = 'app.psnergy.com'
#PORT = 8086
#USERNAME = "influxout_vanilla"
#PASSWORD = "emacssucks"
#DATABASE = influxout_vanilla

###____   INFLUX SERVER CONFIG   ____###
#Uncomment to use Local InfluxDB instance config
#HOST = 'localhost'
#PORT = 8086
#USERNAME = ""
#PASSWORD = ""
#DATABASE = influxout_vanilla

def buildClient():
    client = InfluxDBClient(host=HOST, port=PORT, username = USERNAME, password = PASSWORD)
    return(client)

client = buildClient()
client.switch_database(DATABASE)

# This list would be useful in the future to detect and tag ERR, STS and CAL messages
nanlist = []
for num in range(0, 9):
    nanlist.append("ERR"+str(num))
for num in range(0,5):
    nanlist.append("STS"+str(num))
for num in range(0,3):
    nanlist.append("CAL"+str(num))
nanlist.append("CAL")

def buildJson(values):
    parsed_vals = []
    if values[1] == "O":
        tag = TAG_o2
        for i in range(2, len(values)):
            if values[i].isdigit():
                parsed_vals.append(float(float(values[i])/100))
            else:
                parsed_vals.append(float(0))
    elif values[1] == "P":
        tag = TAG_pressure
        for i in range(2, len(values)):
            try:
                parsed_vals.append(float(values[i]))
            except ValueError:
                parsed_vals.append(float(0))
    json_body = [

        {
            "measurement": PROJECT_NAME,
            "tags": {
                "label": tag
            },
            "fields": {
                "d1": parsed_vals[0],
                "d2": parsed_vals[1],
                "d3": parsed_vals[2],
                "d4": parsed_vals[3]
            }
        }
    ]
    return(json_body)

def sendData(values):
    try:
        client.write_points(buildJson(values))
        return 1
    except IndexError:
        return -1
