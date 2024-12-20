#include <DHT.h>
#define DHTPIN 4     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 3

#include "Adafruit_VEML7700.h"
#include <SD.h>

#include <Wire.h>
#include "RTClib.h"  // Credit: Adafruit
 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
RTC_DS1307 RTC;
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino
Adafruit_VEML7700 veml = Adafruit_VEML7700();

void setup() {
  Serial.begin(9600);
  
  dht.begin();
  
  sensors.begin();
  
  veml.begin();
  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_800MS);
  veml.setLowThreshold(10000);
  veml.setHighThreshold(20000);
  veml.interruptEnable(true);

  Wire.begin();
  RTC.begin();
  // This section grabs the current datetime and compares it to
  // the compilation time.  If necessary, the RTC is updated.
  DateTime now = RTC.now();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (now.unixtime() < compiled.unixtime()) {
    Serial.println("RTC is older than compile time! Updating");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  pinMode(52, Serial);
  pinMode(53, OUTPUT);
  pinMode(11, OUTPUT);
  Serial.println("unixTime,airHum,airTemp,wind,soilHum,soilTemp,light,water on/off");

  SD.begin(53);
//  SD.remove("OUTPUT.txt"); // to clear previous data
}

bool solOn = false;
long curTime;
long prevTime;
int duration;
int on;

void loop() {
  delay(60000); 
  File myFile = SD.open("OUTPUT.txt", FILE_WRITE);

//  RTC clock
  DateTime now = RTC.now();
  myFile.print(now.unixtime(), DEC);
  myFile.print(",");
  Serial.print(now.unixtime(), DEC);
  Serial.print(",");
  
//  Air temp & moisture
  float airHum = dht.readHumidity();
  float airTemp = dht.readTemperature();
  myFile.print(airHum);
  myFile.print(",");
  myFile.print(airTemp);
  Serial.print(airHum);
  Serial.print(",");
  Serial.print(airTemp);

  // Anemometer
  int sensorValue = analogRead(A2);
  double outvoltage = sensorValue * (5.0 / 1023.0);
  int Level = 4*(11.538 * outvoltage - 7.5);
  myFile.print(",");
  myFile.print(Level);
//  Serial.print(" Voltage: ");
//  Serial.print(outvoltage);
//  Serial.print("V ");
  Serial.print(",");
  Serial.print(Level);

  // Soil Moisture
  myFile.print(",");
  myFile.print((float(analogRead(A1)) / 1023.0) * 3.3);
  Serial.print(",");
  Serial.print((float(analogRead(A1)) / 1023.0) * 3.3);

  // Soil Temp
  sensors.requestTemperatures();
  float Celcius = sensors.getTempCByIndex(0);
  myFile.print(",");
  myFile.print(Celcius);
  Serial.print(",");
  Serial.print(Celcius);

  // Light
  myFile.print(",");
  myFile.print(veml.readALS());
  Serial.print(",");  
  Serial.print(veml.readALS());

//   Solenoid, pin 11, (open: 176)
  curTime = now.unixtime();
  if (curTime - prevTime > 7200){
    on = random(1, 41);
    if (on == 1){
      duration = random(15, 36);
      analogWrite(11, 200);
      solOn = true;
      
    }
    prevTime = curTime;
  }
  if (solOn && curTime - prevTime > 60 * duration){
    analogWrite(11, LOW);
    solOn = false;
  }
  myFile.print(",");
  Serial.print(",");
  if (solOn){
    myFile.println("1");
    Serial.println("1");
  }else{
    myFile.println("0");
    Serial.println("0");
  }
  
  myFile.close();
}
