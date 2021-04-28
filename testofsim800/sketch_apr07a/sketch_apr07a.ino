#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>
#include <SD.h>

#define CHIPSELECT PA4
//#define DEBUG 1


String sendATCommand(String cmd, bool waiting);
String waitResponse();
void clearTrash();

void sendFile(String file);

String response = "";
String nameOfFile = "logger30.txt";
File myFile;

void setup()
{
  
  pinMode(CHIPSELECT, OUTPUT);
#ifdef DEBUG
  Serial.begin(115200);
#endif
  //Serial.println(“Begin”);
  Serial2.begin(4800);

  // here init of mpu, smth else

#ifdef DEBUG 
  while(!Serial)
  {};
  Serial.println("START!");
#endif
  
  if (!SD.begin(CHIPSELECT)) {
    Serial.println("Card failed, or not present");
  }

  delay(20000);
  sendFile(nameOfFile);
}

void loop()
{
  sendFile(nameOfFile);
#ifdef DEBUG
  if (Serial2.available())
    Serial.write(Serial2.read());
  if (Serial.available())
    Serial2.write(Serial.read());
 #endif
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

void sendFile(String file)
{
int maxdata=1360;
int loops;
int lastloop;
char emailbuf;

if (SD.exists(file)) {
      Serial.println("file exists.");
    } else {
      Serial.println("file exist.");
    }

  myFile = SD.open(file, FILE_READ);

  delay(500);
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
  sendATCommand("AT+SMTPFILE=1,\"" + file + "\",0", true);
  clearTrash();
  sendATCommand("AT+SMTPSEND", true);
  waitResponse();
  delay(10000);

  // We need second response, converted it digits.
  /*Serial.print("Raw: ");
  response = waitResponse();
  Serial.println(response);
  response = response.substring(response.indexOf(':')+4);
  maxdata = response.toInt();
  Serial.println(maxdata);
  sendATCommand("AT+SMTPFT=" + response, true);*/
  //maxdata = 1360;
  sendATCommand("AT+SMTPFT=1360", true);
  waitResponse();
  clearTrash();
  loops = myFile.size()/maxdata;
  lastloop = myFile.size()%maxdata;
  
#ifdef DEBUG
  Serial.println(loops);
  Serial.println(lastloop);
  Serial.println("Writing data...");
#endif
  
  for (int i = 0; i<loops;i++)
  {
    for(int j = 0; j<maxdata; j++)
    {
    myFile.seek(j+(maxdata*i));
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
    /*waitResponse();
    Serial.print("Raw: ");
    response = waitResponse();
    Serial.println(response);
    response = response.substring(response.indexOf(':')+4);
    maxdata = response.toInt();
    Serial.println(maxdata);
    sendATCommand("AT+SMTPFT=" + response, true);*/
#ifdef DEBUG
    Serial.print("End of loop №");
    Serial.println(i+1);
#endif
    if(i<(loops-1))
    {
        waitResponse();
        sendATCommand("AT+SMTPFT=1360", true);
    }
  }
  waitResponse();
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
  sendATCommand("AT+SMTPFT=0",true);
  waitResponse();
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
