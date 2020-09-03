//AWS iot 
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "AWScredentials.h"

//connecting to the AWS iot
void connectAWSIoT();
void mqttCallback (char* topic, byte* payload, unsigned int length);

//connecting to the local wifi
char *ssid = mySSID;
char *password = myPASSWORD;

//Assigning the REST API endopoint 
const char *endpoint = myEndpoint;

//assigning the AWS communication port *NEVER use port 1883 
const int port = 8883;

//Assigning the topic to where the data is to be published and subscribed
char *pubTopic = "HydroponicsESP32/phdata";
//Uploading the ROOTca, PRIVATE and CERTIFICATE.
const char* rootCA = myRootCA;
const char* certificate = myCertificate;
const char* privateKey = myPrivateKey;

WiFiClientSecure httpsClient;
PubSubClient mqttClient(httpsClient);
//-----------------my vars------------------

char message[200];
void setup() {
    Serial.begin(115200);

    // Start WiFi
    Serial.println("Connecting to ");
    Serial.print(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected.");

    // Configuring MQTT Client
    httpsClient.setCACert(rootCA);
    httpsClient.setCertificate(certificate);
    httpsClient.setPrivateKey(privateKey);
    mqttClient.setServer(endpoint, port);
    mqttClient.setCallback(mqttCallback);

    connectAWSIoT();
    //on connection report the off state for the sprinkler, if it should be on, delta will publish
}

void connectAWSIoT() {
    while (!mqttClient.connected()) {
        if (mqttClient.connect("HydroponicsESP32")) {
            Serial.println("Connected.");
            int qos = 0; //Make sure the qos is zero in the MQTT broker of AWS
            //mqttClient.subscribe(pubTopic, qos);
            Serial.println("Subscribed.");
        } else {
            Serial.print("Failed. Error state=");
            Serial.print(mqttClient.state());
        
            delay(100); //change the delay here to manipulate the device shadow updation speed
        }
    }
    
}

void mqttCallback (char* topic, byte* payload, unsigned int length) {
    Serial.print("Received. topic=");
    Serial.println(topic);
    String rawjson;
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        message[i] = (char)payload[i];
    }
    
}

void mqttLoop() {
    if (!mqttClient.connected()) {
        Serial.println("disconnected");
        connectAWSIoT();
    }
    mqttClient.loop();
}

  
void loop() {
  mqttLoop();
  mqttClient.publish(pubTopic, "hello");
  delay(10000);
}
