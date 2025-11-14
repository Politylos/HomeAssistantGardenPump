
#include <ArduinoHA.h>
#include "ESP8266WiFi.h"


#define BROKER_ADDR     IPAddress(10,0,0,4)
#define PORT            1883
#define BROKER_USERNAME     "Username" // replace with your credentials
#define BROKER_PASSWORD     "Password"


#define Relay_Pump D1
#define lvl_Sensor D2
#define rtc_mem_start 65
#define rtc_mem_len 3

const char* ssid = "wifi"; // Change this to your WiFi SSID
const char* password = "password";  // Change this to your WiFi password

typedef struct {
  bool Pump_Enable =0;
  bool Has_Water =0;
  bool Has_Watered_Today =0;
  bool Can_water =0;
  bool Is_Watering =0;
  int count =0;
} mem_save;

mem_save Pump_Data;
volatile unsigned long Start_Water = 0;
volatile unsigned long Wake_up_Time = 0;
volatile unsigned long currentMillis = 0;
bool toggleFlag = 0;
int buckets = (sizeof (mem_save) / 4);


const long utcOffsetInSeconds = 36000;

byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x3F};

WiFiClient client;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);

HAButton buttonB("TogglePump");
HASensorNumber WaterSensor("IsWatering");
HASensorNumber EmptySensor("IsEmpty");

void onButtonCommand(HAButton* sender)
{
    if (sender == &buttonB) {
        Serial.print("Button B\n");
        if (Pump_Data.Is_Watering){
          Serial.print("Stop the water\n");
          
        }else{
          Water(Pump_Data.Has_Water);
          Serial.print("Start the water\n");
        }
    }
}
bool ShouldWater(const char* data){
  uint16_t length = strlen(data);
  bool Start = false;
  const char name[] =  "ShouldWater";
  uint8_t nameLength = 11;
  uint8_t FindPos = 0;
  int i;
  for (i =0; i < length; i++){
    Serial.print(data[i]);
    if ((data[i] == '/') && (!Start) && (FindPos == nameLength)){
      Start = true;
    }
    if ((Start) && (data[i] != '/')){

      return data[i] == '1';
    }
    if (!Start){
      if (data[i] == name[FindPos]){
        FindPos++;
        if(FindPos == nameLength){
          FindPos = 11;
        }
      } else {
        FindPos = 0;
      }
    } 
  }
  return false;
}
void onMqttMessage(const char* topic, const uint8_t* payload, uint16_t length) {
    // This callback is called when message from MQTT broker is received.
    // Please note that you should always verify if the message's topic is the one you expect.
    // For example: if (memcmp(topic, "myCustomTopic") == 0) { ... }
    Serial.print("New message on topic: ");
    Serial.println(topic);
    Serial.print("Data: ");
    Serial.println((const char*)payload);
    if(ShouldWater((const char*)payload)){
      Water(Pump_Data.Has_Water);
      mqtt.publish("HasWateredToday", "1");
      Serial.print("Got Water Message\n");
    }
}

void onMqttConnected() {
    Serial.println("Connected to the broker!");

    // You can subscribe to custom topic if you need
    mqtt.subscribe("WateringToday");
    mqtt.publish("HasWateredToday", "0");
}

void onMqttDisconnected() {
    Serial.println("Disconnected from the broker!");
}

void onMqttStateChanged(HAMqtt::ConnectionState state) {
    Serial.print("MQTT state changed to: ");
    Serial.println(static_cast<int8_t>(state));
}

void Store_Data(){
  bool val = 0;
  system_rtc_mem_write(64, &val, 4);
  system_rtc_mem_write(rtc_mem_start, &Pump_Data, sizeof (mem_save)); 
  yield();
}
void Read_Data(){
  system_rtc_mem_read (rtc_mem_start, &Pump_Data,sizeof (mem_save) );
  yield();
}

void Water(bool Water_lvl){
  if (Water_lvl){
    Start_Water = millis();
    digitalWrite(Relay_Pump, 1);
    Pump_Data.count++;
    Pump_Data.Has_Watered_Today = 1;
    Pump_Data.Can_water = 0;
    Pump_Data.Is_Watering = 1;
    Serial.print("pump On\n");
  }
}
void Stop_Water(bool Force){
  if (Force){
    digitalWrite(Relay_Pump, 0);
    Pump_Data.Is_Watering = 0;
    Serial.print("pump off\n");
  }else{
    if ((millis() - Start_Water) > 60000){
      digitalWrite(Relay_Pump, 0);
      Pump_Data.Is_Watering = 0;
      Serial.print("pump off\n");
    }
  }
}

void Sleep(){
  //Store_Data();
  ESP.deepSleep(300e6); 
}

void Get_Water_lvl(){
  Pump_Data.Has_Water = 1;//digitalRead(lvl_Sensor);
}

void setup() {
    // you don't need to verify return status
    Serial.begin(9600);
    WiFi.begin(ssid, password);
    pinMode(Relay_Pump, OUTPUT);
    digitalWrite(Relay_Pump, 0);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500); 
    }
    // optional device's details
    device.setName("Water Pump");
    device.setSoftwareVersion("2.0.0");

    // optional properties
    buttonB.setIcon("mdi:water-pump");
    buttonB.setName("Toggle Pump");

    buttonB.onCommand(onButtonCommand);
    WaterSensor.setIcon("mdi:water");
    WaterSensor.setName("Watering");
    WaterSensor.setUnitOfMeasurement("");
    EmptySensor.setIcon("mdi:bucket");
    EmptySensor.setName("Has Water");
    EmptySensor.setUnitOfMeasurement("");
    mqtt.onMessage(onMqttMessage);
    mqtt.onConnected(onMqttConnected);
    mqtt.onDisconnected(onMqttDisconnected);
    mqtt.onStateChanged(onMqttStateChanged);
    mqtt.begin(BROKER_ADDR, PORT,BROKER_USERNAME,BROKER_PASSWORD);
    Wake_up_Time = millis();
}

void loop() {
    mqtt.loop();
    
    if ((millis() - currentMillis)  > 1000){
      if (!Pump_Data.Is_Watering){
        Serial.println((millis() - Wake_up_Time));
        if ((millis() - Wake_up_Time) >= 10000){

          Serial.print("going to sleep\n");
          Sleep();
        }
      }
      Get_Water_lvl();
      Stop_Water(0);
      WaterSensor.setValue(Pump_Data.Is_Watering);
      EmptySensor.setValue(Pump_Data.Has_Water);
      currentMillis = millis();
      
    }
}
