from influxdb import InfluxDBClient
from paho.mqtt.client import Client as MQTTClient
import re
import sys
import typing

INFLUXDB_HOST     = "localhost"
INFLUXDB_PORT     = 8086
INFLUXDB_USERNAME = "sm170975"
INFLUXDB_PASSWORD = "B!5C#Cxw"
INFLUXDB_DATABASE = "sm170975"
MQTT_CLIENT_ID    = "smartin" #can be whatever I want
MQTT_USERNAME     = "sm170975"
MQTT_PASSWORD     = "B!5C#Cxw"
MQTT_HOST         = "iot.devinci.online"
MQTT_PORT         = 8883
MQTT_TOPIC_ROOT   = "sm170975"
MQTT_TOPIC        = MQTT_TOPIC_ROOT + '/+/+'
MQTT_REGEX        = MQTT_TOPIC_ROOT + '/([^/]+)/([^/]+)'

class SensorData(typing.NamedTuple): #to get the correct dtypes of the variables for the  measurement and value
sensor_name: str
measurement: str
value: float

def parse_message(topic, payload): #reads the measurement and the sensor data from the topic path structure and identifies the variables.
#The function returns the variables as NamedTuple
    match = re.match(MQTT_REGEX, topic)
    if match:
        sensor_name = match.group(1)
        measurement = match.group(2)
        return SensorData(sensor_name, measurement, float(payload))
    else:
        return None
      
def store_data(data, db_client): #gets the variables as NamedTuple as input and create a json structure which will later fits to the query in Grafana.
#The measurement is saved directly. The sensor name is stored in a tag structure and the value also nested in a field variable.
#In the last step the json structure is written to the InfluxDB database
    json_data = [
            {
                'measurement': data.measurement,
                'tags': {
                    'sensor_name': data.sensor_name
                    },
                'fields': {
                    'value': data.value
                    }
                }
             ]
    db_client.write_points(json_data)

def mqtt_connect_callback(client, userdata, flags, rc): #handle what happens when the MQTT client connects to the broker.
#If our clients connect to the broker we want to subscribe to all topics which starts with “sm170975” and print the message that the connection was established.
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)

def mqtt_message_callback(client, userdata, msg): #called every time the topic is published to
    print(msg.topic + ' ' + str(msg.payload))
    sensor_data = parse_message(msg.topic, msg.payload.decode('utf-8'))
    if sensor_data:
        store_data(sensor_data, userdata)

def main():
    influxdb_client = InfluxDBClient(INFLUXDB_HOST, INFLUXDB_PORT, INFLUXDB_USERNAME, INFLUXDB_PASSWORD, INFLUXDB_DATABASE) #First the database is initialized
    mqtt_client = MQTTClient( MQTT_CLIENT_ID, userdata=influxdb_client) #Then we create a client object
    mqtt_client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD) #and set the username and password for the MQTT client
    mqtt_client.tls_set()
    mqtt_client.on_connect = mqtt_connect_callback #We tell the client which functions are to be run on connecting
    mqtt_client.on_message = mqtt_message_callback #and on receiving a message
    mqtt_client.connect(MQTT_HOST, MQTT_PORT) #we can connect to the broker with the broker host and port
    mqtt_client.loop_forever()

if __name__ == '__main__':
    sys.exit(main())
