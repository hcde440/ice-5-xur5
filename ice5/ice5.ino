// Melody Xu
// HCDE 440 
// 4/23/19
// ICE 5

// Recieving  messages are in the JSON format. 

//Including libraries for esp, dht and mpl sensors, OLED, JSON and MQTT
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>   
#include <ArduinoJson.h>    
#include <ESP8266WiFi.h> 
#include <Adafruit_MPL115A2.h>
#include <Adafruit_Sensor.h>
#include "config.h"

#define mqtt_server "mediatedspaces.net"  //this is its address, unique to the server
#define mqtt_user "hcdeiot"               //this is its server login, unique to the server
#define mqtt_password "esp8266"           //this is it server password, unique to the server

#define wifi_ssid "University of Washington"   // Wifi Stuff
#define wifi_password "" //

WiFiClient espClient;             // espClient
PubSubClient mqtt(espClient);     // tie PubSub (mqtt) client to WiFi client

char mac[6]; //A MAC address as the unique user ID!

char message[201]; //201, as last character in the array is the NULL character, denoting the end of the array

char espUUID[8] = "ESP8602"; // Name of the microcontroller


Adafruit_MPL115A2 mpl; // initialize mpl pressure sensor

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Initialize oled 

// Initialize variables for temperature, humidity, and pressure

void setup() {
  // Start the serial connection
  Serial.begin(115200);

  // System status
  while(! Serial);
  Serial.print("This board is running: ");
  Serial.println(F(__FILE__));
  Serial.print("Compiled: ");
  Serial.println(F(__DATE__ " " __TIME__));


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.display(); // Initiate the display
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  setup_wifi();
  mqtt.setServer(mqtt_server, 1883); // Start the mqtt
  mqtt.setCallback(callback); //Register the callback function
  
}

/////SETUP_WIFI/////
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");  //get the unique MAC address to use as MQTT client ID, a 'truly' unique ID.
  Serial.println(WiFi.macAddress());  //.macAddress returns a byte array 6 bytes representing the MAC address
}       



void loop() {
  if (!mqtt.connected()) {  // Try connecting again
    reconnect();
  }
  mqtt.loop(); //this keeps the mqtt connection 'active'
  
}


//function to reconnect if we become disconnected from the server
void reconnect() {

  // Loop until we're reconnected
  while (!espClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    // Attempt to connect
    if (mqtt.connect(espUUID, mqtt_user, mqtt_password)) { //the connction
      Serial.println("connected");
      // Once connected, publish an announcement...
      char announce[40];
      strcat(announce, espUUID);
      strcat(announce, "is connecting. <<<<<<<<<<<");
      mqtt.publish(espUUID, announce);
      // ... and resubscribe
      //      client.subscribe("");
      mqtt.subscribe("fromMarco/tempHum");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic); //'topic' refers to the incoming topic name, the 1st argument of the callback function
  Serial.println("] ");

  DynamicJsonBuffer  jsonBuffer; //blah blah blah a DJB
  JsonObject& root = jsonBuffer.parseObject(payload); //parse it!

  if (String(topic) == "fromMarco/tempHum") {
    display.clearDisplay(); 
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(10, 0);
    display.println(); // print temp and pressure
    Serial.println("printing message");
    Serial.print("Message arrived in topic: ");
    long temp = root["Temperature"];
    long humi = root["Humidity"];
    long pres = root["Pressure"];
    String sHum = String(humi);
    String sTem = String(temp); 
    String sPre = String(pres); 
    Serial.println("Humidity " + sHum);
    Serial.println("Temperature " + sTem);
    Serial.println("Pressure " + sPre);
    Serial.print("Message:");
    
//    for (int i = 0; i < length; i++) {
//      Serial.print((char)payload[i]);
//      display.print((char)payload[i]);
//    }
    Serial.println();
    display.println("Humidity " + sHum); 
    display.println("Temperature " + sTem);// print temp and pressure
    display.println("Pressure " + sPre);
    display.display();
  }

  if (!root.success()) { //well?
    Serial.println("parseObject() failed, are you sure this message is JSON formatted.");
    return;
  }
}


