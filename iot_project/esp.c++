#include <TimeLib.h>
#include <String.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ThingSpeak.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <Servo.h>
#include "DHT.h"
#include <WiFi.h>
#include <time.h>
#include <vector>
​
int h = 17;
int m = 10;
​
// WIFI AND SERVER
​
char ssid[] = "OnePlus Nord CE 2";
char password[] = "bunny123";
​
const char* server = "mqtt3.thingspeak.com";
​
const char* mqttUserName = "HDslPTQDDTIRCCoZLi0uCgw";
​
const char* mqttPass = "PBQgihHEtPFVxe4KpkWArkRM";
​
const char* clientID = "HDslPTQDDTIRCCoZLi0uCgw";
​
int randomDataChannel = 2165262;
int channelNumber = 1;
char writeAPIKey[] = "WOVGOGAZSX4V38YY";
char readAPIKey[] = "PFWYRJ7UNRNRMFMQ";
​
int waterLevelChannel = 2165400;
int turbChannel = 2165401;
int forceChannel = 2165403;
​
int port = 1883;
​
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
​
WebServer webserver(80);
​
​
// OM2M
#define MAIN_SSID "OnePlus Nord CE 2"
#define MAIN_PASS "bunny123"
#define CSE_IP "192.168.35.137"
#define CSE_PORT 5089
#define OM2M_ORGIN "admin:admin"
#define OM2M_MN "/~/in-cse/in-name/"
#define OM2M_AE "Intelligent_toilet"
#define OM2M_DATA_CONT1 "Force_sensors/Data1"
#define OM2M_DATA_CONT2 "Force_sensors/Data2"
#define OM2M_DATA_CONT3 "Force_sensors/Data3"
#define OM2M_DATA_CONT4 "Force_sensors/Data4"
​
#define OM2M_DATA_CONT5 "IR_sensors/Data"
#define OM2M_DATA_CONT6 "Humidity_sensors/Data"
#define OM2M_DATA_CONT7 "Waterlevel_sensors/Data"
#define OM2M_DATA_CONT8 "Turbidity_sensors/Data"
#define OM2M_DATA_CONT9 "Ultrasonic_sensors/Data"
​
#define INTERVAL 15000L
​
int n = 0;
// int sum1 = 0;
// int sum2 = 0;
// int sum3 = 0;
int sum4 = 0;
// int sum5 = 0;
int sum6 = 0;
int sum7 = 0;
int sum8 = 0;
int sum9 = 0;
int sum10 = 0;
​
// int avg1;
// int avg2;
// int avg3;
int avg4;
// int avg5;
int avg6;
int avg7;
int avg8;
int avg9;
int avg10;
​
const char* ntpServer = "pool.ntp.org";
long int prev_millis = 0;
unsigned long epochTime;
HTTPClient http;
​
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}
​
void publishData();
void flush();
void handleFlush();
void sendFlushData();
void updateFlush();
void sendData();
void handlePostData();
void freshUp();
int ForceDifference(int pin4, int threshold);
void sendUsageData();
​
int flushes = 0;
int water_consumed = 0;
​
// 0 - short, 1 - medium, 2 - high consumption
int flush_type = 0;
int flushDelay = 5;
int freshnerInterval = 10;
​
​
#define DHTTYPE DHT11
#define dht_dpin 21
DHT dht(dht_dpin, DHTTYPE);
​
// #define sensor_power_pin 19
#define signal_pin 33
​
// SENSOR DATA PINS AND ALL
#define FORCE_SENSOR_PIN 35
#define FORCE_SENSOR_PIN2 555
#define FORCE_SENSOR_PIN3 12
#define FORCE_SENSOR_PIN4 13
​
Servo servo1;
Servo servo2;
#define servo_pin 18
#define servo_pin2 5
​
​
int trigPin = 14;
int echoPin = 32;
int trigPin2 = 26;
int echoPin2 = 27;
int sensorPin = 33;
long duration, cm, inches;
long duration2, cm2, inches2;
int exhaustfan = 15;
int pos2;
​
#define FORCE_THRESHOLD 2500
​
std::vector<int> water_consumed_array;
​
void check();
​
void setup() {
  Serial.begin(115200);
​
  WiFi.begin(MAIN_SSID, MAIN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }
  ThingSpeak.begin(wifiClient);
​
  webserver.on("/flush", handleFlush);
  webserver.on("/fresh", freshUp);
  webserver.on("/get-flush-data", sendFlushData);
  webserver.on("/update-flush-settings", updateFlush);
  webserver.on("/check", check);
  // webserver.on("/get-data", sendData);
  webserver.on("/post-data", handlePostData);
  webserver.on("/get-water-usage-data", sendUsageData);
  webserver.begin();
  webserver.enableCORS(true);
​
  Serial.println(WiFi.localIP());
​
  Serial.println("WiFi connected");
  mqttClient.setServer(server, port);
​
  pinMode(FORCE_SENSOR_PIN, INPUT);
  pinMode(FORCE_SENSOR_PIN2, INPUT);
  pinMode(FORCE_SENSOR_PIN3, INPUT);
  pinMode(FORCE_SENSOR_PIN4, INPUT);
  pinMode(exhaustfan, OUTPUT);
  pinMode(sensorPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  servo1.attach(servo_pin);
  servo2.attach(servo_pin2);
​
  //   WiFi.begin(MAIN_SSID, MAIN_PASS);
  // while (WiFi.status() != WL_CONNECTED) {
  //       delay(500);
  //       Serial.print("#");
  //  }
  configTime(0, 0, ntpServer);
  dht.begin();
}
​
void check(){
  webserver.send(200);
}
​
float flushReady = true;
int motorSwitch = 0;
​
void loop() {
  n++;
  if (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    while (!mqttClient.connect(clientID, mqttUserName, mqttPass)) {
      Serial.print(".");
    }
    Serial.println("Connected to MQTT");
  }
  mqttClient.loop();
​
  webserver.handleClient();
​
  // total delay - 775 + 2675 (flush) + (5000*[0,1,2])
​
  // sensor value of turbidity
  float sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);
​
  // DHT code
  float humid = dht.readHumidity();
  float temp = dht.readTemperature(true);od
  // humid = random(50,60);
  Serial.println(humid);
  if (humid >= 47) {
    if(!motorSwitch){
      for (pos2 = 180; pos2 >= 0; pos2 -= 1) {
        servo2.write(pos2);
        delay(15);
      }
      motorSwitch = 1;
      digitalWrite(exhaustfan, HIGH);
    }
  }
  else {
    if(motorSwitch){
      for (pos2 = 0; pos2 <= 180; pos2 += 1) {
        servo2.write(pos2);
        delay(15);
      }
      motorSwitch = 0;
      digitalWrite(exhaustfan, LOW);
    }
  }
​
​
​
  String dataString = "field1=" + String(humid);
  dataString += "&field2=" + String(temp);
  dataString += "&field3=" + String(sensorValue);
  String topicString = "channels/" + String(turbChannel) + "/publish";
  mqttClient.publish(topicString.c_str(), dataString.c_str());
​
  // Force Sensor
  int p4 = ForceDifference(FORCE_SENSOR_PIN, FORCE_THRESHOLD);  //wait until person sits on toilet and gets up
  Serial.println("done!!");
​
  // ultrasound sensor
  Serial.println("pot ultrasound");
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  cm = (duration / 2) / 29.1;    // Divide by 29.1 or multiply by 0.0343
  inches = (duration / 2) / 74;  // Divide by 74 or multiply by 0.0135
​
​
​
 
  
  dataString = "field6=" + String(cm);
  dataString += "&field7=" + String(inches);
  topicString = "channels/" + String(forceChannel) + "/publish";
  mqttClient.publish(topicString.c_str(), dataString.c_str());
  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.println("cm");
​
  // condition for flush
  if (sensorValue < 969 /*&& cm > 60*/) {
    flush();
    delay(flush_type * 2000);
    for (int pos = 90; pos >= 0; pos -= 1) {
      servo1.write(pos);
      delay(7.5);
    }
  }
​
​
​
  // int pos2;
  // if(cm2<10)
  // {
  // for (pos2=0;pos2<=90;pos2+=1){
  //   servo2.write(pos2);
  //   delay(7.5);
  // }
  // delay(2500);
​
​
​
  // Serial.println("fs");
  // if (millis() - prev_millis >= INTERVAL) {
  //       epochTime = getTime();
  //       time_t cur = now();
  //       int hr = hour(cur) + h;
  //       int min = minute(cur) + m;
  //       int sec = second(cur);
  //       int dy = day(cur) + 29;
  //       int mon = month(cur) + 4;
  //       int yr = year(cur) + 53;
  //       String data;
​
  //       //ultrasonic
  //       sum9 += cm;
  //       sum10 += cm2;
  //       avg9 = sum9 / n;
  //       avg10 = sum10 / n;
  //       String server = "http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String() + OM2M_MN;
  //       http.begin(server + String() + OM2M_AE + "/" + OM2M_DATA_CONT9 + "/");
  //     	http.addHeader("X-M2M-Origin", OM2M_ORGIN);
  //       http.addHeader("Content-Type", "application/json;ty=4");
  //       http.addHeader("Content-Length", "100");
  //       data =  "Sensor readings :-  Pot Ultrasonic : " + String(cm) + "cm, Door Ultrasonic : " + String(cm2) + "cm, Averages : " + " Pot Ultrasonic : " + String(avg9) + "cm, Door Ultrasonic : "+ String(avg10) + "cm, Time : " + String(hr) +":"+String(min) +":"+String(sec) +","+ String(dy) +"/"+ String(mon) +"/"+ String(yr) ;
  //       String req_data = String() + "{\"m2m:cin\": {"
​
  //         +
  //         "\"con\": \"" + data + "\","
​
  //         +
  //         "\"lbl\": \"" + "V1.0.0" + "\","
​
  //         //+ "\"rn\": \"" + "cin_"+String(i++) + "\","
​
  //         +
  //         "\"cnf\": \"text\""
​
  //         +
  //         "}}";
  //       int code = http.POST(req_data);
  //       http.end();
​
​
  //       //turbidity
  //       sum8 += sensorValue;
  //       avg8 = sum8 / n;
  //       server = "http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String() + OM2M_MN;
  //       http.begin(server + String() + OM2M_AE + "/" + OM2M_DATA_CONT8 + "/");
  //       http.addHeader("X-M2M-Origin", OM2M_ORGIN);
  //       http.addHeader("Content-Type", "application/json;ty=4");
  //       http.addHeader("Content-Length", "100");
  //       data =  "Turbidity reading : " + String(sensorValue) + " Turbidity average : " + String(avg8) + "   Time : " + String(hr) +":"+String(min) +":"+String(sec) +","+ String(dy) +"/"+ String(mon) +"/"+ String(yr) ;
  //       req_data = String() + "{\"m2m:cin\": {"
​
  //         +
  //         "\"con\": \"" + data + "\","
​
  //         +
  //         "\"lbl\": \"" + "V1.0.0" + "\","
​
  //         //+ "\"rn\": \"" + "cin_"+String(i++) + "\","
​
  //         +
  //         "\"cnf\": \"text\""
​
  //         +
  //         "}}";
  //       code = http.POST(req_data);
  //       http.end();
​
​
  //       //f4
  //       sum4 += p4;
  //       avg4 = sum4 / n;
  //       server = "http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String() + OM2M_MN;
  //       http.begin(server + String() + OM2M_AE + "/" + OM2M_DATA_CONT4 + "/");
  //       http.addHeader("X-M2M-Origin", OM2M_ORGIN);
  //       http.addHeader("Content-Type", "application/json;ty=4");
  //       http.addHeader("Content-Length", "100");
  //       data =  "Force reading : " + String(p4) + "Avg Force : " + String(avg4) +"   Time : " + String(hr) +":"+String(min) +":"+String(sec) +","+ String(dy) +"/"+ String(mon) +"/"+ String(yr) ;
  //       req_data = String() + "{\"m2m:cin\": {"
​
  //         +
  //         "\"con\": \"" + data + "\","
​
  //         +
  //         "\"lbl\": \"" + "V1.0.0" + "\","
​
  //         //+ "\"rn\": \"" + "cin_"+String(i++) + "\","
​
  //         +
  //         "\"cnf\": \"text\""
​
  //         +
  //         "}}";
  //       code = http.POST(req_data);
  //       http.end();
​
  //       //humidity
  //       sum6 += humid;
  //       avg6 = sum6 / n;
  //       server = "http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String() + OM2M_MN;
  //       http.begin(server + String() + OM2M_AE + "/" + OM2M_DATA_CONT6 + "/");
  //       http.addHeader("X-M2M-Origin", OM2M_ORGIN);
  //       http.addHeader("Content-Type", "application/json;ty=4");
  //       http.addHeader("Content-Length", "100");
  //       data =  "Humidity reading : " + String(humid) +"Avg Humidity : " + String(avg6) + "   Time : " + String(hr) +":"+String(min) +":"+String(sec) +","+ String(dy) +"/"+ String(mon) +"/"+ String(yr) ;
  //       req_data = String() + "{\"m2m:cin\": {"
​
  //         +
  //         "\"con\": \"" + data + "\","
​
  //         +
  //         "\"lbl\": \"" + "V1.0.0" + "\","
​
  //         //+ "\"rn\": \"" + "cin_"+String(i++) + "\","
​
  //         +
  //         "\"cnf\": \"text\""
​
  //         +
  //         "}}";
  //       code = http.POST(req_data);
  //       http.end();
​
  //       // //waterlevel
  //       // sum7 += water_level;
  //       // avg7 = sum7 / n;
  //       // server = "http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String() + OM2M_MN;
  //       // http.begin(server + String() + OM2M_AE + "/" + OM2M_DATA_CONT7 + "/");
  //       // http.addHeader("X-M2M-Origin", OM2M_ORGIN);
  //       // http.addHeader("Content-Type", "application/json;ty=4");
  //       // http.addHeader("Content-Length", "100");
  //       // data =  "Water level(in cm) : " + String(water_level) + "Avg Water Level : " + String(avg7) +"   Time : " + String(hr) +":"+String(min) +":"+String(sec) +","+ String(dy) +"/"+ String(mon) +"/"+ String(yr) ;
  //       // req_data = String() + "{\"m2m:cin\": {"
​
  //       //   +
  //       //   "\"con\": \"" + data + "\","
​
  //       //   +
  //       //   "\"lbl\": \"" + "V1.0.0" + "\","
​
  //       //   //+ "\"rn\": \"" + "cin_"+String(i++) + "\","
​
  //       //   +
  //       //   "\"cnf\": \"text\""
​
  //       //   +
  //       //   "}}";
  //       // code = http.POST(req_data);
  //       // http.end();
​
​
  //       Serial.println(code);
  //       prev_millis = millis();
​
  // }
​
​
  delay(100);
  // publishData();
}
​
void handlePostData() {
  String payload = webserver.arg("plain");
​
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);
​
  int ultraSound = doc["ultraSound"].as<int>();
  Serial.println(ultraSound);
​
  webserver.send(200);
}
​
void freshUp() {
  // servo motor code
  int pos2;
  for (pos2 = 0; pos2 <= 90; pos2 += 1) {
    servo2.write(pos2);
    delay(7.5);
  }
  delay(500);
​
  for (pos2 = 90; pos2 >= 0; pos2 -= 1) {
    servo2.write(pos2);
    delay(7.5);
  }
​
  DynamicJsonDocument jsonDoc(4096);
​
  jsonDoc["fresh"] = 1;
  jsonDoc["duration"] = random(1, 5);
​
  String jsonString;
  serializeJson(jsonDoc, jsonString);
​
  delay(500);
  webserver.send(200, "application/json", jsonString);
}
​
void handleFlush() {
  if (flushReady) {
    flush();
  } else {
    DynamicJsonDocument jsonDoc(4096);
    jsonDoc["flush-ready"] = 0;
​
    String jsonString;
    serializeJson(jsonDoc, jsonString);
​
    webserver.send(200, "application/json", jsonString);
  }
}
​
void flush() {
  // total delay - 2675ms
​
  // servo motor code
  Serial.println("Servo");
  for (int pos = 0; pos <= 90; pos += 1) {
    // delay of 675 ms
    servo1.write(pos);
    delay(7.5);
  }
  // delay(500);
​
  DynamicJsonDocument jsonDoc(4096);
​
  jsonDoc["flush-ready"] = 1;
  jsonDoc["flushed"] = 2 + flush_type;
  water_consumed_array.push_back(int(jsonDoc["flushed"]));
  water_consumed += int(jsonDoc["flushed"]);
  // time_t currentTime = now();
​
  // // Format the timestamp as a string
  // char timestamp[20];
  // sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02dZ", year(currentTime), month(currentTime), day(currentTime), hour(currentTime), minute(currentTime), second(currentTime));
  // Serial.println(timestamp);
​
  // jsonDoc["timestamp"] = timestamp;
​
  JsonObject sensor1 = jsonDoc.createNestedObject("turb");
  sensor1["type"] = "turbidity";
  sensor1["value"] = random(45, 145);
​
  JsonObject sensor2 = jsonDoc.createNestedObject("DHT");
  sensor2["type"] = "humidity";
  sensor2["value"] = random(10, 100);
​
  String jsonString;
  serializeJson(jsonDoc, jsonString);
​
  delay(500);
  webserver.send(200, "application/json", jsonString);
​
  flushes++;
  for (int i = 0; i < 20; i++) {
    String dataString = "field2=1";
    String topicString = "channels/" + String(waterLevelChannel) + "/publish";
    mqttClient.publish(topicString.c_str(), dataString.c_str());
    delay(75);
  }
}
​
void sendFlushData() {
  DynamicJsonDocument jsonDoc(4096);
​
  jsonDoc["flushes"] = flushes;
  jsonDoc["water-consumed"] = water_consumed;
​
  String jsonString;
  serializeJson(jsonDoc, jsonString);
​
  delay(500);
  webserver.send(200, "application/json", jsonString);
}
​
void publishData() {
  String dataString = "field1=" + String(random(90, 1000));
  dataString += "&field2=" + String(random(9, 102));
  dataString += "&field3=" + String(random(200, 500));
  String topicString = "channels/" + String(randomDataChannel) + "/publish";
  mqttClient.publish(topicString.c_str(), dataString.c_str());
​
  // ThingSpeak.setField(1, random(10, 100));
  // ThingSpeak.setField(2, random(500, 1000));
  // ThingSpeak.setField(3, random(2000, 5000));
  // int x = ThingSpeak.writeField(channelNumber, 1, random(90,1000), writeAPIKey);
  // Serial.println(x);
​
  delay(1000);
}
​
void sendData(long data) {
  HTTPClient http;
​
  // IPAddress localIP = WiFi.localIP();
  // String ipAddress = localIP.toString();
​
  String url = "http://192.168.243.94:80/post-data";
  http.begin(wifiClient, url);
  Serial.println(url);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST("{\"ultraSound\":" + String(data) + ",\"value1\":\"49.54\",\"value2\":\"1005.14\"}");
  Serial.println(httpResponseCode);
}
​
int ForceDifference(int pin4, int threshold) {
​
  // int p1 = analogRead(FORCE_SENSOR_PIN);
  // int p2 = analogRead(FORCE_SENSOR_PIN2);
  // int p3 = analogRead(FORCE_SENSOR_PIN3);
  int p4 = analogRead(FORCE_SENSOR_PIN4);
  // Serial.println(p1);
  // Serial.println(p2);
  // Serial.println(p3);
  String dataString = "field4=" + String(p4);
  String topicString = "channels/" + String(forceChannel) + "/publish";
  mqttClient.publish(topicString.c_str(), dataString.c_str());
​
  Serial.print("Force: ");
  Serial.println(p4);
  delay(1000);
​
  return p4;
}
​
void updateFlush() {
  String payload = webserver.arg("plain");
​
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);
​
  flushDelay = doc["flush-delay"].as<int>();
  freshnerInterval = doc["freshner"].as<int>();
  flush_type = doc["consumption"].as<int>();
  Serial.println("Flush Settings Updated!");
​
  webserver.send(200);
}
​
void sendUsageData() {
  DynamicJsonDocument doc(4096);
  JsonArray array = doc.createNestedArray("data");
​
  // add some values
  for (int i = 0; i < flushes; i++) {
    array.add(water_consumed_array.at(i));
  }
  // serialize the array and send the result to Serial
  String JsonString;
  serializeJson(doc, JsonString);
​
  webserver.send(200, "application/json", JsonString);
}
