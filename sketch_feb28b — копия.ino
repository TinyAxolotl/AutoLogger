#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050_tockn.h>
#include <BMx280I2C.h>
#include <SD.h>


#define I2C_ADDRESS 0x76
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

MPU6050 mpu6050(Wire);
BMx280I2C bmx280(I2C_ADDRESS);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
File myFile;

int numOfFile = 0;
String nameOfFile = "logger";
const int chipSelect = 4;
long timer = 0, timers = 0;
double currentPressure = 0, groundPressure = 0, altitude = 0;
long rawAccX, rawAccY, rawAccZ;
double total, max_total;

void setup()
{
  
  Serial.begin(115200);
  pinMode(PC13, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  //delay(2000);
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.clearDisplay(); 
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(30,20);
  display.println(utf8rus("ХАИ"));
  display.setTextSize(1);
  display.setCursor(20,50);
  display.println(utf8rus("Кафедра 501"));
  display.display();
  delay(5000);
  display.clearDisplay();
  display.setCursor(0,0);
  //while (!Serial);
  display.print("MPU6050...");
  display.display();
  Wire.begin();
  mpu6050.begin();
  mpu6050.writeMPU6050(MPU6050_ACCEL_CONFIG, 0x18);
  mpu6050.calcGyroOffsets(true);
  display.println("OK!");
  display.display();
  
  display.print("BMP280...");
  display.display();
  if (!bmx280.begin())
  {
    Serial.println("begin() failed. check your BMx280 Interface and I2C Address.");
    display.println("ERROR! ERROR! ERROR!");
    display.display();
    while (1);
  }
  

  bmx280.resetToDefaults();
  bmx280.writeOversamplingPressure(BMx280MI::OSRS_P_x16);
  bmx280.writeOversamplingTemperature(BMx280MI::OSRS_T_x16);
  display.println("OK!");
  display.display();
  
  bmx280.measure();
  for(int i = 0; i<50; i++)
  {
  while(!bmx280.hasValue())
  {}
  groundPressure = bmx280.getPressure64();
  bmx280.measure();
  }
  bmx280.measure();
  display.print("SD card...");
  display.display();
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    display.println("ERROR! ERROR! ERROR!");
    display.display();
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
  display.println("OK!");
  display.display();
  nameOfFile += String(numOfFile);
  nameOfFile += String(".txt");
  numOfFile++;
  while(SD.exists(nameOfFile))
  {
      nameOfFile = "logger";
      numOfFile++;
      nameOfFile += String(numOfFile);
      nameOfFile += String(".txt");
  }
  while(!SD.exists(nameOfFile))
  {
    if (!SD.exists(nameOfFile)) {
      Serial.println("Creating file...");
      myFile = SD.open(nameOfFile, FILE_WRITE);
      myFile.close();
    } 
    if (SD.exists(nameOfFile)) {
      Serial.println("file exists.");
    } else {
      Serial.println("file exist.");
    }
  }
  display.println("File name is:");
  display.setTextSize(1);
  display.println(nameOfFile);
  display.display();
  delay(1000);
  digitalWrite(PC13, HIGH);   // Initialization successful
}

void loop()
{

  String dataString = "";
  mpu6050.update();

  if(millis() - timers > 50)
  {
    char buf[12];
    display.clearDisplay(); 
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0,0);
    display.println(utf8rus("Ускорение:"));
    sprintf(buf, "%2.3f", double(total/2048));
    display.print(buf);
    display.println(" G");
    display.println(utf8rus("максимум:"));
    sprintf(buf, "%2.3f", double(max_total/2048));
    display.print(buf);
    display.println(" G");
    display.display();
    
    timers = millis();
  }
  
  if(millis() - timer > 2){
   /* Serial.println("=======================================================");
    Serial.print("temp : ");Serial.println(mpu6050.getTemp());*/
    /*Serial.print("accX : ");Serial.print(mpu6050.getAccX());
    Serial.print("\taccY : ");Serial.print(mpu6050.getAccY());
    Serial.print("\taccZ : ");Serial.println(mpu6050.getAccZ());
    Serial.print("rawAccX : ");Serial.print(mpu6050.getRawAccX());
    Serial.print("\trawAccY : ");Serial.print(mpu6050.getRawAccY());
    Serial.print("\trawAccZ : ");Serial.println(mpu6050.getRawAccZ());*/
    dataString += String(millis());
    dataString += String(",");
    dataString += String(mpu6050.getAccX());
    dataString += String(",");
    dataString += String(mpu6050.getAccY());
    dataString += String(",");
    dataString += String(mpu6050.getAccZ());
    dataString += String(",");
    rawAccX = mpu6050.getRawAccX();
    dataString += String(rawAccX);
    dataString += String(",");
    rawAccY = mpu6050.getRawAccY();
    dataString += String(rawAccY);
    dataString += String(",");
    rawAccZ = mpu6050.getRawAccZ();
    dataString += String(rawAccZ);
    dataString += String(",");
    total = sqrt(pow(rawAccX, 2) + pow(rawAccY, 2) + pow(rawAccZ, 2));
    if(total > max_total)
    {
      max_total = total;
    }
    dataString += String(total);
    dataString += String(",");
    dataString += String(mpu6050.getGyroX());
    dataString += String(",");
    dataString += String(mpu6050.getGyroY());
    dataString += String(",");
    dataString += String(mpu6050.getGyroZ());
    dataString += String(",");
    
    if(bmx280.hasValue())
    {
      currentPressure = bmx280.getPressure64();
      //Serial.println(currentPressure);
      altitude = (44330 * (1-pow((currentPressure/groundPressure),0.1903)));
      //Serial.println(altitude);
      bmx280.measure();
    }
    dataString += String(currentPressure);
    dataString += String(",");
    dataString += String(altitude);
    dataString += String(";");

    myFile = SD.open(nameOfFile, FILE_WRITE);
    if (myFile) {
    myFile.println(dataString);
    myFile.close();
    // print to the serial port too:
    Serial.println(dataString);
    }
  /*
    Serial.print("gyroX : ");Serial.print(mpu6050.getGyroX());
    Serial.print("\tgyroY : ");Serial.print(mpu6050.getGyroY());
    Serial.print("\tgyroZ : ");Serial.println(mpu6050.getGyroZ());
  
    Serial.print("accAngleX : ");Serial.print(mpu6050.getAccAngleX());
    Serial.print("\taccAngleY : ");Serial.println(mpu6050.getAccAngleY());
  
    Serial.print("gyroAngleX : ");Serial.print(mpu6050.getGyroAngleX());
    Serial.print("\tgyroAngleY : ");Serial.print(mpu6050.getGyroAngleY());
    Serial.print("\tgyroAngleZ : ");Serial.println(mpu6050.getGyroAngleZ());
    
    Serial.print("angleX : ");Serial.print(mpu6050.getAngleX());
    Serial.print("\tangleY : ");Serial.print(mpu6050.getAngleY());
    Serial.print("\tangleZ : ");Serial.println(mpu6050.getAngleZ());
    Serial.println("=======================================================\n");*/
    timer = millis();
    
  }
}


/* Функция перекодировки русских букв из UTF-8 в Win-1251 */
String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };
  k = source.length(); i = 0;
  while (i < k) {
    n = source[i]; i++;
    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}
