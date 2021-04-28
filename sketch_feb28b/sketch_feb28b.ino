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

#define chipSelect PA4
#define DEBUG 1

MPU6050 mpu6050(Wire);
BMx280I2C bmx280(I2C_ADDRESS);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
File myFile;

String sendATCommand(String cmd, bool waiting);
String waitResponse();
void clearTrash();

int sendFile(String file);
void getReadyToTransmit(String num, String file);

int numOfFile = 0, numOfFileBuf = 0;
String response = "";
String nameOfFile = "log";

char buf[12];
unsigned int turn = 0; 
long timer = 0, timers = 0, sim800 = 0, lengthOfFile;
double currentPressure = 0, groundPressure = 0, altitude = 0;
long rawAccX, rawAccY, rawAccZ;
double total, max_total;

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif
  pinMode(PC13, OUTPUT);
  pinMode(PB12, INPUT);
  pinMode(chipSelect, OUTPUT);
  
  Serial2.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
#ifdef DEBUG    
    Serial.println(F("SSD1306 allocation failed"));
#endif
    for(;;); // Don't proceed, loop forever
  }
  //delay(2000);
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.clearDisplay(); 
  display.setTextColor(WHITE);
  display.setTextSize(5);
  display.setCursor(20,8);
  display.println(utf8rus("ХАИ"));
  display.setTextSize(1);
  display.setCursor(30,50);
  display.println(utf8rus("Кафедра 501"));
  display.display();
  delay(10000);
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
#ifdef DEBUG
    Serial.println("begin() failed. check your BMx280 Interface and I2C Address.");
#endif

    display.println("ERROR! ERROR! ERROR!");
    display.display();
    //while (1);
  }
  

  bmx280.resetToDefaults();
  bmx280.writeOversamplingPressure(BMx280MI::OSRS_P_x16);
  bmx280.writeOversamplingTemperature(BMx280MI::OSRS_T_x16);
  display.println("OK!");
  display.display();
  
  bmx280.measure();
  for(int i = 0; i<150; i++)
  {
  while(!bmx280.hasValue())
  {}
  groundPressure = bmx280.getPressure64();
  bmx280.measure();
  }
  bmx280.measure();
  display.print("SD card...");
  display.display();
#ifdef DEBUG
  Serial.print("Initializing SD card...");
#endif
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
#ifdef DEBUG
    Serial.println("Card failed, or not present");
#endif
    display.println("ERROR! ERROR! ERROR!");
    display.display();
    // don't do anything more:
    while (1);
  }
#ifdef DEBUG
  Serial.println("card initialized.");
#endif
  display.println("OK!");
  display.display();
  nameOfFile += String(numOfFile);
  nameOfFile += String(".txt");
  numOfFile++;
  while(SD.exists(nameOfFile))
  {
      nameOfFile = "log";
      numOfFile++;
      nameOfFile += String(numOfFile);
      nameOfFile += String(".txt");
  }
  while(!SD.exists(nameOfFile))
  {
    if (!SD.exists(nameOfFile)) {
#ifdef DEBUG
      Serial.println("Creating file...");
#endif
      myFile = SD.open(nameOfFile, FILE_WRITE);
      myFile.close();
    } 
    if (SD.exists(nameOfFile)) {
#ifdef DEBUG      
      Serial.println("file exists.");
#endif
    } else {
#ifdef DEBUG
      Serial.println("file exist.");
#endif
    }
  }
  display.println("File name is:");
  display.setTextSize(1);
  display.println(nameOfFile);
  display.display();
  delay(1000);
  digitalWrite(PC13, HIGH);   // Initialization successful

  sim800 = millis();
  timers = millis();
  timer = millis();
  numOfFileBuf = numOfFile;

  while(numOfFileBuf > 0 && digitalRead(PB12))
    {
      display.clearDisplay(); 
      display.setTextColor(WHITE);
      display.setTextSize(2);
      display.setCursor(0,0);
      
      display.print(utf8rus("файл номер"));
      sprintf(buf, "%d", numOfFileBuf);
      display.setTextSize(4);
      display.setCursor(40,25);
      display.println(buf);
      display.display();
      sendFile("log"+(String)numOfFileBuf+".txt");
      numOfFileBuf--;
    }
    numOfFileBuf = numOfFile;
}

void loop()
{
  String dataString = "";
  
  mpu6050.update();
  
  if(((millis() - sim800) > 20000) && (digitalRead(PB12)))
  {
    while(numOfFileBuf > 0 && digitalRead(PB12))
    {
      display.clearDisplay(); 
      display.setTextColor(WHITE);
      display.setTextSize(2);
      display.setCursor(0,0);
      
      display.print(utf8rus("файл номер"));
      sprintf(buf, "%d", numOfFileBuf);
      display.setTextSize(4);
      display.setCursor(40,25);
      display.println(buf);
      display.display();
      sendFile("log"+(String)numOfFileBuf+".txt");
      numOfFileBuf--;
    }
    sim800 = millis();
  }
  
  if(millis() - timers > 200)
  {
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
  
  if(millis() - timer > 2)
  {
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
#ifdef DEBUG
    // print to the serial port too:
    Serial.println(dataString);
#endif
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
          if (n == 0x81) {n = 0xA8; break;}
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

int sendFile(String file)
{

int maxdata=1360;
int maxrounds = 2000;
int rounds;
int loops;
int lastloop;
char emailbuf;

  myFile = SD.open(file, FILE_READ);
  lengthOfFile = myFile.size();
  if(lengthOfFile < maxdata)
  {
    myFile.close();
    return 0;
  }
  loops = lengthOfFile/maxdata;
  lastloop = lengthOfFile%maxdata;
  rounds = lengthOfFile/(maxrounds*maxdata);
  
#ifdef DEBUG
  Serial.println(loops);
  Serial.println(lastloop);
  Serial.println(rounds);
  Serial.println("Writing data...");
#endif

for(int k = 0; k<rounds; k++)
{
  getReadyToTransmit((String)k, file);
  for (int i = 0; i<maxrounds;i++)
  {
    for(int j = 0; j<maxdata; j++)
    {
      myFile.seek((j+(maxdata*i))+(k * maxrounds * maxdata));
      emailbuf = myFile.peek();
      if(emailbuf == ';')
      {
         Serial2.print(emailbuf);
      }
       else
      {
        Serial2.print(emailbuf);
      }
    }
#ifdef DEBUG
    Serial.print("End of loop №");
    Serial.println(i+1);
#endif
    if(i<(maxrounds-1))
    {
        waitResponse();
        sendATCommand("AT+SMTPFT=1360", true);
    }
  }
  waitResponse();
  waitResponse();
  sendATCommand("AT+SMTPFT=0", true);
  waitResponse();
  waitResponse();
  clearTrash();
}
/*  waitResponse();
  response = (String)lastloop;
  sendATCommand("AT+SMTPFT=" + response, true);
#ifdef DEBUG
  Serial.println("Starting last loop");
#endif
  for (int i = 0; i<lastloop;i++)
  {
    myFile.seek(i+(maxdata*loops));
    emailbuf = myFile.peek();
    if(emailbuf == ';')
      {
        Serial2.print(emailbuf);
      }
      else
      {
       Serial2.print(emailbuf);
      }
  }
  waitResponse();
  sendATCommand("AT+SMTPFT=0",true);*/
  waitResponse();
  myFile.close();
}

void getReadyToTransmit(String num, String file)
{
  waitResponse();
  sendATCommand("AT+CMEE=1", true);
  clearTrash();
  sendATCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", true);
  clearTrash();
  sendATCommand("AT+SAPBR=3,1,\"APN\",\"kyivstar.net\"", true);
  clearTrash();
  sendATCommand("AT+SAPBR=3,1,\"USER\",\"igprs\"", true);
  clearTrash();
  sendATCommand("AT+SAPBR=3,1,\"PWD\",\"internet\"", true);
  clearTrash();
  sendATCommand("AT+SAPBR=1,1", true);
  clearTrash();
  sendATCommand("AT+SAPBR=2,1", true);
  waitResponse();
  clearTrash();
  sendATCommand("AT+EMAILCID=1", true);
  clearTrash();
  sendATCommand("AT+EMAILTO=120", true);
  clearTrash();
  sendATCommand("AT+EMAILSSL=1", true);
  clearTrash();
  sendATCommand("AT+SMTPSRV=\"smtp.gmail.com\",465", true);
  clearTrash();
  sendATCommand("AT+SMTPAUTH=1,\"overloadtestingblock@gmail.com\",\"19216812Zz\"", true);
  clearTrash();
  sendATCommand("AT+SMTPFROM=\"overloadtestingblock@gmail.com\",\"IvanPeredriy\"", true);
  clearTrash();
  sendATCommand("AT+SMTPRCPT=0,0,\"i.peredrii@student.khai.edu\"", true);
  clearTrash();
  sendATCommand("AT+SMTPSUB=\"Test email through GPRS\"", true);
  clearTrash();
  sendATCommand("AT+SMTPBODY=4", true);
  clearTrash();
  sendATCommand("data", true);
  clearTrash();
  sendATCommand("AT+SMTPFILE=1,\"" + num + '_' + file + "\",0", true);
  clearTrash();
  sendATCommand("AT+SMTPSEND", true);
  waitResponse();
  sendATCommand("AT+SMTPFT=1360", true);
  waitResponse();
  clearTrash();
}

String waitResponse() {                         // Функция ожидания ответа и возврата полученного результата
  String _resp = "";                            // Переменная для хранения результата
  long _timeout = millis() + 10000;             // Переменная для отслеживания таймаута (10 секунд)
  while (!Serial2.available() && millis() < _timeout)  {}; // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
  if (Serial2.available()) {                     // Если есть, что считывать...
    _resp = Serial2.readString();                // ... считываем и запоминаем
  }
  else {                                        // Если пришел таймаут, то...
#ifdef DEBUG    
    Serial.println("Timeout...");               // ... оповещаем об этом и...
#endif
  }
  return _resp;                                 // ... возвращаем результат. Пусто, если проблема
}

String sendATCommand(String cmd, bool waiting) {
  String _resp = "";                            // Переменная для хранения результата
#ifdef DEBUG
  Serial.println(cmd);                          // Дублируем команду в монитор порта
#endif
  Serial2.println(cmd);                          // Отправляем команду модулю
  if (waiting) {                                // Если необходимо дождаться ответа...
    _resp = waitResponse();                     // ... ждем, когда будет передан ответ
#ifdef DEBUG
    Serial.println(_resp);                      // Дублируем ответ в монитор порта
#endif
  }
  return _resp;                                 // Возвращаем результат. Пусто, если проблема
}

void clearTrash()
{
  delay(100);
  if(Serial2.available())
  {
    while(Serial2.available())
    {
      Serial2.read();
      delay(5);
    }
  }
}
