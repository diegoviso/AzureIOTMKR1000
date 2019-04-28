


#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <WiFiSSLClient.h>

#include <NTPClient.h>
#include <RTCZero.h>

//#include <aREST.h>

#include <ArduinoJson.h>
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>

#define LIGHTWEIGHT 1

void azureIotClientInit();
void azureIotClientRun();
void sendPayload (char *);


// Status
int status = WL_IDLE_STATUS;

// Create aREST instance
//aREST rest = aREST();

//create timer instance
extern RTCZero rtc;
const int GMT = 12;



//IOTHubClient instance
static WiFiSSLClient sslClient;
static AzureIoTHubClient iotHubClient;


// WiFi parameters and server instanciation
char ssid[] = "";
char password[] = "";
WiFiServer server(80);


//Sensor Data struct
struct SensorData {
   const char* sensor = "MKR1000";
   int id = 1;
   int time;
   int rotation;
   int rpm;
   int pressure;
   
};
SensorData sensorData;

// Variables to be exposed to the API
int rotation;
// Declare functions to be exposed to the API
int ledControl(String command);

void setup() {
 
  Serial.begin(115200);
  initRest();
  initWifi();
  initTime() ;
  iotHubClient.begin(sslClient);
  azureIotClientInit();
  //send initial sensors readings
  pinMode(5,OUTPUT);
  digitalWrite(5,HIGH);
  
  


  
  }

void loop() {

  azureIotClientRun();
  
  serializeAndsendSensorData();

  
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }



 

}


void serializeAndsendSensorData()
{

  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["sensor"] = "MKR1000";
  root["ID"] = 1;
  root["rotation"] = analogRead(A0);
  root["rpm"] = analogRead(A1);
  root["pressure"] = analogRead(A2);

  char jsonChar[200];
  
  root.printTo(jsonChar);

  Serial.print("Payload: ");
  Serial.println(jsonChar);
  sendPayload(jsonChar);
  Serial.print("Payload Queued...\r\n");
  
 }




// Custom function accessible by the API

int ledControl(String command) {
  // Get state from command
  int state = command.toInt();
  digitalWrite(5,state);
  return 1; 
}


void initWifi(){
  // Connect to WiFi

  while (status != WL_CONNECTED) {
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  status = WiFi.begin(ssid, password);
  delay(10000);
  }
  
  Serial.println("WiFi connected");
  server.begin();

  Serial.println("Server started");
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

//initalize time sync hub
void initTime() {

    WiFiUDP ntpUdp;
    NTPClient ntpClient(ntpUdp);
    ntpClient.begin();

    while (!ntpClient.update()) {
        Serial.println("Fetching NTP epoch time failed! Waiting 5 seconds to retry.");
        delay(5000);
    }

    ntpClient.end();

    unsigned long epochTime = ntpClient.getEpochTime();

    Serial.print("Fetched NTP epoch time is: ");
    Serial.println(epochTime);
    iotHubClient.setEpochTime(epochTime);


    Serial.print("RTC clock set to: ");
    int hour = rtc.getHours() + GMT;
    if (hour>24){
      hour=hour-24;
    }
    print2digits(hour);
    Serial.print(":");
    print2digits(rtc.getMinutes());
    Serial.print(":");
    print2digits(rtc.getSeconds());
    Serial.println();
 
    
    rtc.setSeconds(1); 
    rtc.enableAlarm(rtc.MATCH_SS);
    rtc.attachInterrupt(serializeAndsendSensorData);

    
  
   
  
}


void initRest()
{
 /*  // Init variables and expose them to REST API
  rotation = 24;
  rest.variable("rotation",&rotation);
  rest.function("led",ledControl);

  // Give name and ID to device (ID should be 6 characters long)
  rest.set_id("1");
  rest.set_name("mkr1000");*/
}

void restServer(WiFiClient client) 
{
  String command = client.readStringUntil('/');
  client.println("Azure IoT Arduino Rest Server\n");
  client.println("");
  return;

}


void print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}


