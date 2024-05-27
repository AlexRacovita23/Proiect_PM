#include <LiquidCrystal.h> // includes the LiquidCrystal Library
#include <dht.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>


// PIN Initialization
const int trigPin = 8;
const int echoPin = 9;
const int dhtPin = 3;
const int chipSelect = 10;
const int recordButtonPin = 5;
const int changeMeasPin = 4;

// Sensor Initialization
LiquidCrystal_I2C lcd_i2c(0x27, 16, 2);
#define DHTTYPE DHT22
dht DHT;
File dataFile;

// Sensor variables
int recordButtonState = 0, lastMeasState = LOW, currentMeasState;
unsigned long pressedTime, releasedTime, pressDuration;
const int SHORT_PRESS = 500;
bool displayMode = false;
float hum, temp,distanceCm, distanceInch, soundSpeed;
long duration;
int readDHT22, count = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  lcd_i2c.begin();
  lcd_i2c.backlight();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(recordButtonPin, INPUT);
  pinMode(changeMeasPin, INPUT);
  

  if (!SD.begin(chipSelect)) {
    Serial.println("Card initialization failed!");
    while(1);
    return;
  }
  Serial.println("Card initialized.");

  dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Distance in cms,Distance in inches,Recorded temperature,Recorded humidity");
    dataFile.close();
  }
}
void loop() {
  lcd_i2c.clear();
  //DHT22 Data Read 
  readDHT22 = DHT.read22(dhtPin);
  temp = DHT.temperature;
  hum = DHT.humidity;

  // HC-SR04 signal trigger
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  //Velocity = 331.4 + 0.6 x Temperature + 0.0124 x Relative_Humidity
  // Distance computation
  duration = pulseIn(echoPin, HIGH);
  soundSpeed = 331.4 + (0.6 * temp) + (0.0124 * hum);
  // Divided by two as the signal duration is for both transmission and reception
  distanceCm = duration * soundSpeed / 20000;
  distanceInch = distanceCm * 0.393700787;
  
  // LCD print
  if(!displayMode) {
    lcd_i2c.setCursor(0, 0);
    lcd_i2c.print(distanceCm, 2);
    delay(10);

    lcd_i2c.setCursor(13, 0);
    lcd_i2c.print(" cm\0");
    delay(10);

    lcd_i2c.setCursor(0, 1);
    lcd_i2c.print(distanceInch, 2);
    delay(10);

    lcd_i2c.setCursor(11, 1);
    lcd_i2c.print(" inch\0");
    delay(10);
  }

  // Button read
  recordButtonState = digitalRead(recordButtonPin);
  currentMeasState = digitalRead(changeMeasPin);

  if (recordButtonState == HIGH) {
    writeDataToCSV((String)distanceCm + "," + distanceInch + "," + temp + "," + hum);
  }

  if(lastMeasState == HIGH && currentMeasState == LOW)        // button is pressed
    pressedTime = millis();
  else if(lastMeasState == LOW && currentMeasState == HIGH) { // button is released
    releasedTime = millis();
    pressDuration = releasedTime - pressedTime;
    Serial.println(pressDuration);
    if(pressDuration < SHORT_PRESS){
      lcd_i2c.clear();
      displayMode = true;
      String line = dataFile.readStringUntil('\n');
      char str[16];
      line.toCharArray(str, line.length());
      Serial.println(str);
      char delim[] = ",";
      char *word = strtok(str, delim);
      Serial.println(word);

      lcd_i2c.setCursor(0, 0);
      lcd_i2c.print(word);
      delay(10);

      lcd_i2c.setCursor(13, 0);
      lcd_i2c.print(" cm\0");
      delay(10);

      word = strtok(NULL, delim);
      Serial.println(word);
      lcd_i2c.setCursor(0, 1);
      lcd_i2c.print(word);
      delay(10);

      lcd_i2c.setCursor(11, 1);
      lcd_i2c.print(" inch\0");
      delay(10);
      
      }
    else{
      lcd_i2c.clear();
      displayMode = false;
    }
  }

  lastMeasState = currentMeasState;
  delay(200);
}

void writeDataToCSV(String data) {
  dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(data);
    dataFile.close();
  }
}
