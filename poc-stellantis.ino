#include <SoftwareSerial.h>
#define RX 2
#define TX 3
int countTrueCommand;
int countTimeCommand; 
boolean found = false;
SoftwareSerial esp8266(RX,TX); 

void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  setupWifiModule();
  setupSensors(4, 5);
  setupSensors(6, 7);
  setupSensors(8, 9);
}

void loop() {
  Serial.print("sensor left: ");
  Serial.println(getSensorData(6, 7));
  sendDataWifi("field1", getSensorData(6, 7));

  Serial.print("sensor middle: ");
  Serial.println(getSensorData(4, 5));
  sendDataWifi("field2", getSensorData(4, 5));

  Serial.print("sensor right: ");
  Serial.println(getSensorData(8, 9));
  sendDataWifi("field3", getSensorData(8, 9));
}

/*--------------------------------------------------SENSORS MODULES*/

void setupSensors(const byte trigger, const byte echo) {
  pinMode(trigger, OUTPUT);
  digitalWrite(trigger, LOW);
  pinMode(echo, INPUT);
}

int getSensorData(const byte trigger, const byte echo) {
  const unsigned long measure_timeout = 25000UL; // 25ms = 8m à 340m/s
  const float sound_speed = 340.0 / 1000; // Vitesse du son dans l'air en mm/us
  
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);

  long measure = pulseIn(echo, HIGH, measure_timeout);
  int distance_cm = measure / 2.0 * sound_speed/10; // 3 calcul la distance a partir du temps mesuré

  if (distance_cm < 16) {
    return 2;
  } else if (distance_cm < 25) {
    return 1;
  }
  return 0;
}

/*--------------------------------------------------WIFI MODULE*/

void setupWifiModule() {
  String AP = "INSERT WIFI NAME HERE";
  String PASS = "INSERT WIFI PASSWORD HERE";

  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
}

void sendDataWifi(String field, int valSensor) {
 String API = "INSERT API KEY HERE";
 String HOST = "api.thingspeak.com";
 String PORT = "80";
 String getData = "GET /update?api_key="+ API +"&"+ field +"="+String(valSensor);

 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," + String(getData.length()+4),4,">");

 esp8266.println(getData);
 delay(1500);
 countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);
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
  found = false;
 }
