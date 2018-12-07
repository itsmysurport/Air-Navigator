#include <DHT.h>
#include <ESP8266WiFi.h>
#include "ArduinoJson-v5.13.4.h"

#define cmd 3
#define DHTPIN 4  
#define DHTTYPE DHT11

//Blasin

enum {
  ONE = 1, TWO= 2, THREE = 4
};

String apiWriteKey = "UZYH9JHGU13YOZ1V";
String apiReadKey = "Q9SK7D86J6N40PH6";

const char *ssid = "AndroidHotspot7919";
const char *pass = "kdhas210";

const char* server = "api.thingspeak.com";

WiFiClient client;
bool bConnent;

String inputString, tokenString, command;
int tokenIndex;
//End

DHT dht(DHTPIN, DHTTYPE);

int start = 0;
long duration;
int distance;
const int trigPin = 2;      //D4
const int echoPin = 0;      //D3
const int Button = 5;       //D1
const int upLed = 14;       //D5
const int downLed = 12;     //D6
const int dgLed = 13;       //D7

void setup() {
  Serial.begin(115200);
  
  Serial.printf("Connecting to %s\n", ssid);

  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, pass);
  
  // put your setup code here, to run once:
  WriteToServer(0, 0);
  start = 0;
  ESP.wdtDisable();
  dht.begin();
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(Button, INPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(upLed, OUTPUT); 
  pinMode(downLed, OUTPUT); 
  pinMode(dgLed, OUTPUT); 
  digitalWrite(upLed, LOW);
  digitalWrite(downLed, LOW);
  digitalWrite(dgLed, LOW);
}

void loop() {
  // Blasin
  if (!bConnent) {
    Serial.println("No Connected");
    delay(2000);
    return;
  }
  //END
  
  // DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  Serial.println("H : " + String(humidity));
  Serial.println("T : " + String(temperature) + "\n");
    
  if(humidity > 60)     digitalWrite(dgLed, HIGH);
  else                  digitalWrite(dgLed, LOW);
  delay(2000);
  whenStart();
  
  
  if (start == 1)
  {
    bribri();
    delay(15000);
  }
  delay(50);
}



void whenStart()
{
  int val1, val2;
  RecieveToServer(val1, val2);
  if(val1)
  {
    digitalWrite(upLed, HIGH);
    start = 1;
  }
  else
  {
    digitalWrite(upLed, LOW);
    start = 0;
  }
  
  if(digitalRead(Button)==LOW) // 7번 핀이 LOW 일 때, 버튼을 눌렀을 때
  {
    delay(30); // delay함수를 통해 처음 버튼을 눌렀을 때 생기는 잡신호들을 무시
    Serial.println("Cliked");
    if(start == 0)
    {
      digitalWrite(upLed, HIGH);
      start = 1;
      WriteToServer(cmd, 0);
    }
    else
    {
      digitalWrite(upLed, LOW);
      start = 0;
      WriteToServer(0, 0);   
    }
    while(digitalRead(Button) == LOW); // 스위치를 누르면 깜빡거리는 버그를 없애줌      
    delay(30);
  }
}

void bribri()
{
  int fire, flag;     // fire는 불몇층에서 났는지, flag는 사람 몇층에 있는지
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  
  RecieveToServer(fire, flag);         //서버의 현재상태 읽어들임
  
  if(distance <= 10)
  {
    Serial.print("People HERE !!!!\tDistance: ");
    Serial.println(distance);
    flag |= THREE;
    Serial.print("Flag : ");
    Serial.println(flag);
  }
  else
  {
    flag &= !THREE;
  }
  WriteToServer(fire, flag);
}


//Blasin
void WriteToServer(int data1, int data2) {
  if (!client.connect(server, 80)) return;

  String postStr = apiWriteKey;

  postStr += "&field1=";
  postStr += String(data1);

  postStr += "&field2=";
  postStr += String(data2);

  postStr += "\r\n\r\n";

  client.println("POST /update HTTP/1.1");
  client.println("Host: api.thingspeak.com");
  client.println("Connection: close");
  client.println("X-THINGSPEAKAPIKEY: " + apiWriteKey);
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.print(postStr.length());
  client.print("\n\n");
  client.print(postStr);

  Serial.println("Send");
  client.stop();
}

void RecieveToServer(int& value1, int& value2) {
  value1 = value2 = 0;

  if (!client.connect(server, 80)) return;
  
  StaticJsonBuffer<200> jsonBuffer;
  String url = "/channels/644914/feeds.json";

  client.println("GET " + url + "?api_key=" + apiReadKey + "&results=1 HTTP/1.1");
  client.println("Host: api.thingspeak.com\n");

  delay(500);

  String readString = client.readString();
  
  readString.remove(1, readString.indexOf("\"feeds") - 1);
  readString.remove(0, readString.indexOf("[") + 1);
  readString.remove(readString.lastIndexOf("]"));
  
  JsonObject& root = jsonBuffer.parseObject(readString);

  if (!root.success()) {
    Serial.println("Recieve Fail");
    return;
  }

  value1 = root["field1"];
  value2 = root["field2"];

  Serial.println("Recieve");
  client.stop();
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
  case WIFI_EVENT_STAMODE_CONNECTED:
    Serial.println("WiFi Connented");
    bConnent = true;
    break;
    
  case WIFI_EVENT_STAMODE_DISCONNECTED:
    Serial.println("WiFi Disconnented");
    bConnent = false;
    break;
  }
}
//END
