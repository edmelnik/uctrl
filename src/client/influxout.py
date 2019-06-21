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
PROJECT_NAME = "o2-u-ctrl"
TAG = "4o2avg"

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

def buildJson(values):
    json_body = [
        {
            "measurement": PROJECT_NAME,
            "tags": {
                "label": TAG
            },
            "time": values[0],
            "fields": {
                "d1": values[1],
                "d2": values[2],
                "d3": values[3],
                "d4": values[4]
            }
        }
    ]
    return(json_body)

def sendData(values):
    try:
        client = buildClient()
        client.switch_database(DATABASE)
        client.write_points(buildJson(values))
        return 1
    except IndexError:
        return -1