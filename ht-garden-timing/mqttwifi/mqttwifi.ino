
#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define USE_SERIAL Serial

const char* ssid = "SmoovPOS";
const char* password = "Sm00vp@ss";

//relay

int ledPin =  2;      // the number of the LED pin
int ledState = LOW;             // ledState used to set the LED
unsigned long previousMillis = 0;        // will store last time LED was updated
long milliseconds = 1000;
long OnTimeMinutesDefault = 20; 
long NumberRunOnDay = 8; 
long OnTimeMinutes = OnTimeMinutesDefault; 
long OffTimeMinutes = ((24/NumberRunOnDay)*60)-OnTimeMinutes; 
long OnTime = OnTimeMinutes*60*milliseconds;           // milliseconds of on-time
long OffTime = OffTimeMinutes*60*milliseconds;          // milliseconds of off-time

//end relay
//socket io
int isAction = 0;
//

char* topic = "esp8266_arduino_out";
char* topicIn = "esp8266_arduino_in";
char* server = "m13.cloudmqtt.com";

WiFiClient wifiClient;

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  String value="";
  for (unsigned int i = 0; i < length; i++) { value.concat((char)payload[i]);}
  USE_SERIAL.println("Message receiving topic:");
  USE_SERIAL.println(topic);
  USE_SERIAL.println("Message body:");
  USE_SERIAL.println(value);

  //listen and callback
  if(value == String(3)){
    isAction = 1;
  }
  else{
    //on off relay
    String part01 = getValue(value,';',0);
    String part02 = getValue(value,';',1);
    if(part01 == String(2)){
      if(part02 == String(0)){
        isAction = 2;//on relay
      }
      else{
        isAction = 3;//off relay
      }
    }
    
    //timer runing
    if(part01 == String(1)){
      //part02 relaynumber
      String part03 = getValue(value,';',2);//ontime
      String part04 = getValue(value,';',3);//typeofftime
      String part05 = getValue(value,';',4);//offtime

      OnTimeMinutes = part03.toInt();
      if(OnTimeMinutes<=0){
        OnTimeMinutes = OnTimeMinutesDefault;
      }
      OnTime = OnTimeMinutes*60*milliseconds;
      
      //offtime
      OffTimeMinutes = part05.toInt();
      if(OffTimeMinutes<=0){
        OffTimeMinutes = OnTimeMinutesDefault;
      }
      
      if(String(part04) == String(0)){
        NumberRunOnDay = OffTimeMinutes;
        OffTimeMinutes = ((24/NumberRunOnDay)*60)-OnTimeMinutes; 
      }
      OffTime = OffTimeMinutes*60*milliseconds;

      USE_SERIAL.println("==ontime");
      USE_SERIAL.println(OnTime/60/milliseconds);
      USE_SERIAL.println("==offtime *");
      USE_SERIAL.println(OffTime/60/milliseconds);
    }
  }
}

PubSubClient client(server, 10488, callback, wifiClient);


/*
void callback(char topic, byte payload, unsigned int length) {
  Serial.println(payload);
}
*/



String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

void setup() {
    pinMode(4, OUTPUT);  
    pinMode(5, OUTPUT);  
    
  USE_SERIAL.begin(115200);
  delay(10);
  
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.print("Connecting to ");
  USE_SERIAL.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    USE_SERIAL.print(".");
  }
  USE_SERIAL.println("");
  USE_SERIAL.println("WiFi connected");  
  USE_SERIAL.println("IP address: ");
  USE_SERIAL.println(WiFi.localIP());

  
  
  if (reconnect()) {
    USE_SERIAL.println("Connected to MQTT broker");
    USE_SERIAL.print("Topic is: ");
    USE_SERIAL.println(topic);
    
    if (client.publish(topic, "3;1")) {
      USE_SERIAL.println("Publish ok");
    }
    else {
      USE_SERIAL.println("Publish failed");
    }
  }
  else {
    USE_SERIAL.println("MQTT connect failed");
    USE_SERIAL.println("Will reset and try again...");
    abort();
  }
}

long lastReconnectAttempt = 0;

boolean reconnect() {
  // Generate client name based on MAC address and last 8 bits of microsecond counter
  String clientName;
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);

  USE_SERIAL.print("Connecting to ");
  USE_SERIAL.print(server);
  USE_SERIAL.print(" as ");
  USE_SERIAL.println(clientName);
  
  if (client.connect((char*) clientName.c_str(), "uwezwnhk", "b8bIQGbfIFSs")) {
    /*
    Serial.println("Connected.");
    client.publish("OUT_CpxfXFArcRC8LtHt", "ARDUINO: IN_6xNKYbjdGdqBLpAK");
    */
    client.subscribe(topicIn);    
  }else {
    USE_SERIAL.println("Connection Error.");
  }
  return client.connected();
}

void loop() {

  if (!client.connected()){
     long now = millis();
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        // Attempt to reconnect
        if (reconnect()) {
          lastReconnectAttempt = 0;
        }
      }
  }else{
    client.loop();
  }

  unsigned long currentMillis = millis();
  
  switch (isAction) {
    case 1:
      //listen and callback
      //do something when var equals 1
      sendStatus();
      break;
    case 2:
      //do something when var equals 2
      ledState = LOW;  // turn it on
      previousMillis = currentMillis;   // Remember the time
      digitalWrite(4, ledState);    // Update the actual LED
      digitalWrite(5, ledState);    // Update the actual LED
      sendStatus();
      USE_SERIAL.println("===turn it on");
      USE_SERIAL.println(OffTime);
      break;
    case 3:
      //do something when var equals 2
      ledState = HIGH;  // turn it off
      previousMillis = currentMillis;   // Remember the time
      digitalWrite(4, ledState);    // Update the actual LED
      digitalWrite(5, ledState);    // Update the actual LED
      sendStatus();
      USE_SERIAL.println("===turn it on");
      USE_SERIAL.println(OffTime);
      break;
    default: 
      // if nothing else matches, do the default
      // default is optional
    break;
  }
  isAction = 0;
  
  if(previousMillis == 0)
  {  
    delay(10000);
    ledState = LOW;  // turn it on
    previousMillis = currentMillis;   // Remember the time
    digitalWrite(4, ledState);    // Update the actual LED
    digitalWrite(5, ledState);    // Update the actual LED
    USE_SERIAL.println("===turn it on");
    USE_SERIAL.println(OffTime);
  }
  if((ledState == LOW) && (currentMillis - previousMillis >= OnTime))
  {
    ledState = HIGH;  // Turn it off
    previousMillis = currentMillis;  // Remember the time
    digitalWrite(4, ledState);    // Update the actual LED
    digitalWrite(5, ledState);    // Update the actual LED
    sendStatus();
    USE_SERIAL.println("===turn it off");
    USE_SERIAL.println(OnTime);
  }
  else if ((ledState == HIGH) && (currentMillis - previousMillis >= OffTime))
  {
    ledState = LOW;  // turn it on
    previousMillis = currentMillis;   // Remember the time
    digitalWrite(4, ledState);    // Update the actual LED
    digitalWrite(5, ledState);    // Update the actual LED
    sendStatus();
    USE_SERIAL.println("===turn it on");
    USE_SERIAL.println(OffTime);
  }
  /*
  static int counter = 0;
  
  String payload = "{\"micros\":";
  payload += micros();
  payload += ",\"counter\":";
  payload += counter;
  payload += "}";

  
  
  if (client.connected() && counter == 0){
    Serial.print("Sending payload: ");
    Serial.println(payload);
    
    if (client.publish(topic, (char*) payload.c_str())) {
      Serial.println("Publish ok");
    }
    else {
      Serial.println("Publish failed");
    }
    ++counter;
  }else{
    client.loop();
  }
  delay(5000);
  */
}
void sendStatus(){
  if(ledState == LOW){
      if (client.publish(topic, "3;1")) {
        USE_SERIAL.println("Publish ok");
      }
      else {
        USE_SERIAL.println("Publish failed");
      }
  }
  else{
      if (client.publish(topic, "3;0")) {
        USE_SERIAL.println("Publish ok");
      }
      else {
        USE_SERIAL.println("Publish failed");
      }
  }
}
// http://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
