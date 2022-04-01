#include <SoftwareSerial.h>
#define RX 2
#define TX 3
int countTrueCommand;
int countTimeCommand;
SoftwareSerial esp8266(RX,TX);

bool left = true;
bool mid = false;
bool right = false;
int timer = 0;

void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  setupWifiModule();
  setupSensors(4, 5);
  setupSensors(6, 7);
  setupSensors(8, 9);
}

void loop() {
  Serial.println("-----------------------------");
  Serial.print("sensor left: ");
  Serial.println(getSensorData(6, 7));
  Serial.print("sensor mid: ");
  Serial.println(getSensorData(4, 5));
  Serial.print("sensor right: ");
  Serial.println(getSensorData(8, 9));

  if (timer == 15) { // why 15 ? because thingspeak free only accept request every 15sec
    if (left) {
      Serial.println("Send left");
      sendDataWifi("field1", getSensorData(6, 7));
      left = false;
      mid = true;
    } else if (mid) {
      Serial.println("Send mid");
      sendDataWifi("field2", getSensorData(4, 5));
      mid = false;
      right = true;
    } else if (right) {
      Serial.println("Send right");
      sendDataWifi("field3", getSensorData(8, 9));
      right = false;
      left = true;
    }
    timer = 0;
  }
  timer++;
  delay(1000);
}

/*--------------------------------------------------SENSORS MODULES*/

void setupSensors(const byte trigger, const byte echo) {
  pinMode(trigger, OUTPUT);
  digitalWrite(trigger, LOW);
  pinMode(echo, INPUT);
}

int getSensorData(const byte trigger, const byte echo) {
  const unsigned long measure_timeout = 25000UL; // 25ms = 8m to 340m/s
  const float sound_speed = 340.0 / 1000; //speed of sound in the air in mm/us
  
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);

  long measure = pulseIn(echo, HIGH, measure_timeout);
  int distance_cm = measure / 2.0 * sound_speed/10;

  if (distance_cm < 16) {
    return 2;
  } else if (distance_cm < 25) {
    return 1;
  }
  return 0;
}

/*--------------------------------------------------WIFI MODULE*/

void setupWifiModule() {
  String AP = "";       // WIFI NAME
  String PASS = ""; // WIFI PASSWORD

  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
}

void sendDataWifi(String field, int valSensor) {
 String API = "YOUR API KEY";   // Write API KEY
 String HOST = "api.thingspeak.com"; // programmed to communicate with thingspeak
 String PORT = "80";
 String getData = "GET /update?api_key="+ API +"&"+ field +"="+String(valSensor);

 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," + String(getData.length()+4),4,">");

 esp8266.println(getData);
 countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  bool found = false;
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
    countTimeCommand++;
  }
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
 }