#define RXD2 16
#define TXD2 17
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

//------------MultiThreading-----------
TaskHandle_t AWSpHData;
TaskHandle_t WaterMonitoring;

//-----------------my vars------------------

char message[200];
void setup() {
  Serial.begin(115200);
  //UART communication setup
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial Txd is on pin: "+String(TX));
  Serial.println("Serial Rxd is on pin: "+String(RX));
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

  //create a task that will be executed in the AWSpHDataCode() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    AWSpHDataCode,   /* Task function. */
                    "AWSpHData",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &AWSpHData,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the WaterMonitoringCode() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    WaterMonitoringCode,   /* Task function. */
                    "WaterMonitoring",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &WaterMonitoring,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */
    delay(500); 
}
//AWSpHDataCode: waits for arduino mega to send ph data, then messages AWS IOT Core
void AWSpHDataCode( void * pvParameters ){
  Serial.print("AWSpHDataCode running on core ");
  Serial.println(xPortGetCoreID());
  delay(500);
  char input[6];
  while(1){
    char input[6];
    int counter = 0;
    if(Serial2.available()>0){
      Serial.println("Uart message available. Retrieving now...");
      Serial.println(Serial2.available());
      while (Serial2.available()>0) {
        input[counter]=(char)Serial2.read();
        if(Serial2.available() == 1){
          Serial.println(Serial2.available());
          input[counter+1]='\0';
        }
        counter++;
      }
    Serial.println(input);
    mqttLoop();
    mqttClient.publish(pubTopic, input);
    }
  }
}

//WaterMonitoringCode: Monitors the recevoir's water level. a dip in water levels indicates blockage
void WaterMonitoringCode( void * pvParameters ){
  while(1){
    //Serial.print("Water monitoring running on core ");
    //Serial.println(xPortGetCoreID());
    delay(700);
  }
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
  
}
