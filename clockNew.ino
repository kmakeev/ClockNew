/*
  Программа для часов на газоразрядных лампах с возможностью управления через приложение по WiFi (с помощью модуля esp8266)
 */
 
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Time.h>
#include <DS1307RTC.h>


#define DEBUG true 
#define BUFFER_SIZE 200

#define NUMITEMS(arg) ((size_t) (sizeof (arg) / sizeof (arg [0])))
#define ONE_WIRE_BUS 0                    
#define TEMPERATURE_PRECISION 9
#define DELTA_SHIM_FOR_ANIMATION 4
#define MAX_SHIM_FOR_ANIMATION 150
#define DELAY_ANIMATION 7         // чем больше, тем медленее перебираются цифры
#define DELAY_SHOW 1
#define OK 0
#define ERR 1
#define DATA 2 


char buffer[BUFFER_SIZE];
char *pb;

SoftwareSerial esp8266(8, 9); // RX, TX
const int COM_BAUD = 9600; 

// StaticJsonBuffer<220> jsonBuffer;
// DynamicJsonBuffer  jsonBuffer;


// Объявляем переменные и константы
//Блок общих переменных скетча
// К155ИД1 (1)
uint8_t Pin_1_a = 6;                
uint8_t Pin_1_b = 8;
uint8_t Pin_1_c = 7;
uint8_t Pin_1_d = 5;

// К155ИД1 (2)
uint8_t Pin_2_a = 13;                
uint8_t Pin_2_b = 12;
uint8_t Pin_2_c = 4;
uint8_t Pin_2_d = 1;

// Анодные пины
uint8_t Pin_a_1 = 10;//колбы 1, 4
uint8_t Pin_a_2 = 11;//колбы 2, 5
uint8_t Pin_a_3 = 9; //колбы 3, 6       

//Пины для точек
uint8_t Pin_dot1 = A3;   //Пока будем использовать аналоговые как цифровые
uint8_t Pin_dot2 = A2;   

//Пины для кнопок 
uint8_t Pin_rt1 = A0;   //Пока будем использовать аналоговые как цифровые
uint8_t Pin_rt2 = A1;  

//Пин для подсветки
uint8_t Led_1 = 3; 

//Пин для бипера
int Buzz_1 = 2;           

//Массив для управления анодами ламп
static const uint8_t anods[3] = {Pin_a_1, Pin_a_2, Pin_a_3};


//Массив с помощью которого дешефратору задаются цифры
static const uint8_t numbers[11][4] = 
{
    { 0, 0, 0, 0 }, //0
    { 1, 0, 0, 1 }, //1
    { 1, 0, 0, 0 }, //2
    { 0, 1, 1, 1 }, //3
    { 0, 1, 1, 0 }, //4
    { 0, 1, 0, 1 }, //5
    { 0, 1, 0, 0 }, //6
    { 0, 0, 1, 1 }, //7
    { 0, 0, 1, 0 }, //8
    { 0, 0, 0, 1 }, //9
    { 1, 1, 1, 1 }  //Чисто
};

// Массив для анимации, перебор всех цифр в колбе
static const uint8_t nixie_level[10] = {
    1, 2, 6, 7, 5, 0, 4, 9, 8, 3
};


static const uint8_t bits[2][4] = 
{ 
    {Pin_1_d, Pin_1_c, Pin_1_b, Pin_1_a},// К155ИД1 (1)
    {Pin_2_d, Pin_2_c, Pin_2_b, Pin_2_a }// К155ИД1 (2)
}; 

//Массив данных для 6 колб
uint8_t NumberArray[6]={0,0,0,0,0,0};
bool isChangeArray[6]={false, false, false, false, false,false};
uint8_t ShimAnimationArray[6]={255,255,255,255,255,255};
uint8_t NumberArrayOLD[6]={0,0,0,0,0,0};

uint8_t mode = 0;
uint8_t hours = 0;
uint8_t Mins = 0;
uint8_t Seconds  = 0;
uint8_t timeset = 0;
uint8_t alarmclockset = 0;
uint8_t alarmHour = 0;
uint8_t alarmMin = 0;
uint8_t dayNight = 255;
float tempC = 0;                    //(int)tempC/10
bool sensorTemperatureIn = false;
boolean isAlarm = false;

boolean play = false;

uint8_t btn1 = 0;
uint8_t btn2 = 0;

uint8_t a;

uint8_t i=0, j=0, z=0;
bool isReadTemperature = false;


//boolean ok =false;
boolean up = false;
boolean animate = false;
boolean sec = true;
tmElements_t tm;


unsigned long millisAnimation;               //Время начала шага анимации
unsigned long millisThis;                    //Время сейчас

//Блок переменных для работы с кнопкой
uint8_t currentButtonStatus = 0;              // 0 - Кнопка не нажата
// 1 - Кнопка нажата первый раз
// 2 - Кнопка отжата после двойного нажатия
// 3 - Событие длительного давления на кнопку
// 4 - завершение длительного нажатия


unsigned long currentButtonStatusStart1;  // Кол-во милисекунд от начала работы программы, когда начался статус 1
unsigned long currentButtonStatusStart2;  // Кол-во милисекунд от начала работы программы, когда начался статус 2    
unsigned long currentButtonStatusStart3;  // Кол-во милисекунд от начала работы программы, когда начался статус 3


const int delayFalse = 10;                // Длительность, меньше которой не регистрируется единоразовый клик
const int delayLongSingleClick = 1000;    // Длительность зажатия кнопки для выхода в режим увеличения громкости
const int delayDeltaDoubleClick = 800;    // Длительность между кликами, когда будет зафиксирован двойной клик


String content;
String header;

void setup()  
{
  // Инициализируем последовательный интерфейс и ждем открытия порта:

  esp8266.begin(COM_BAUD);
  Serial.begin(COM_BAUD);
  while(!Serial){
    
  }
  Serial.println();
//  Serial.println("Start");
//  Serial.print("Reset module - ");
  if (sendData("AT+RST\r\n",3400,DEBUG)==OK){
    Serial.println("ok");
  } else {
    Serial.println("error");
  }
  sendData("ATE0\r\n",400,DEBUG);  
//  Serial.println("Configure as access point - ");        
  if (sendData("AT+CWMODE=2\r\n",300,DEBUG)==OK){
    Serial.println("ok");
  } else {
    Serial.println("error");
  }
//  Serial.println("Configure for multiple connections "); 
  if (sendData("AT+CIPMUX=1\r\n",300,DEBUG)==OK){
    Serial.println("ok");
  } else {
    Serial.println("error");   
  }
//  Serial.println("Turn on server on port 80  "); 
  if (sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG)==OK){// turn on server on port 80
    Serial.println("ok");
  } else {
    Serial.println("error");  
  }
 // sendData("AT+CIPSTO=2",300, DEBUG); // Таймаут сервера 2 секунды
//  Serial.println("Waiting for page request  "); 
//  Serial.println("Get ip address  "); 
  if (sendData("AT+CIFSR", 300, DEBUG)==OK){ // узнаём адрес
    Serial.print("ok");
  } else {
    Serial.print("error");  
  }

  pinMode(Led_1, OUTPUT);

}
 
void loop() // выполняется циклически
{
  StaticJsonBuffer<220> jsonBuffer;
  int ch_id, packet_len;

  esp8266.readBytesUntil('\n', buffer, BUFFER_SIZE);
  if (strncmp(buffer,"+IPD,", 5)==0) {         //Сравниваем считанное с  "+IPD,".     
 //    Serial.println("Incomming connection");
     Serial.println(buffer);
     sscanf(buffer+5, "%d,%d", &ch_id, &packet_len);   //Считываем из буфера значения идентификатора подключения и длинну пакета.
   if (packet_len > 0){
    pb = buffer+5;
    while(*pb!=':') pb++;
    pb++;
    }
    if((strncmp(pb, "GET / ", 6) == 0) || (strncmp(pb, "GET /?", 6) == 0))
       {
        Serial.println("Found Get");
        clearSerialBuffer();
        JsonObject& root = jsonBuffer.createObject();
        content = "";
        root["mode"] = mode;
        root["hours"] = hours;
        root["mins"] = Mins;
        root["sec"] = Seconds;
        root["timeset"] = timeset;
        root["alarmSet"] = alarmclockset;
        root["Led"] = dayNight;
        root["tC"] = tempC;                    //(int)tempC/10
        root["sensIn"] = sensorTemperatureIn;
        root["isAlarm"] = isAlarm;
        root["alHour"] = alarmHour;
        root["alMin"] = alarmMin;
        root["btn1"] = btn1;
        root["btn2"] = btn2;
        root["play"] = play;
        root.printTo(content); 
        sendReply(ch_id);
    } else if((strncmp(pb, "PUT / ", 6) == 0) || (strncmp(pb, "SET /?", 6) == 0))
       {
        Serial.println("Found PUT");
        long int time = millis();

        while( (time+400) > millis())
        {
         while(esp8266.available())
         {   
          esp8266.readBytesUntil('\n', buffer, BUFFER_SIZE);
          
          Serial.println();
          Serial.print(buffer);  
          if (buffer[0] == '{')          //Признак строки с json данными
          {
            JsonObject& root = jsonBuffer.parseObject(buffer);
            if (root.success()) {
              Serial.println("Parsing OK");
              mode = root["mode"];
              hours = root["hours"];
              Mins = root["mins"];
              Seconds = root["sec"];
              timeset = root["timeset"];
              alarmclockset = root["alarmSet"];
              dayNight = root["Led"];
              tempC = root["tC"];                    //(int)tempC/10
              sensorTemperatureIn = root["sensIn"];
              isAlarm = root["isAlarm"];
              alarmHour = root["alHour"];
              alarmMin = root["alMin"];
              btn1 = root["btn1"];
              btn2 = root["btn2"];
              play = root["play"];

              
            } else {
              Serial.println();
              Serial.println("Parsing ERROR");
            }
          }
          clearBuffer();
          delay(20);
         }
        }
        content = "";
        sendReply(ch_id);
       }
  clearBuffer();
  }

  analogWrite(Led_1, dayNight);  
        
}

//////////////////////Отправка ответа на GET запрос////////////////////
void sendReply(int ch_id)
{


 Serial.println("In Send Reply");
  header =  "HTTP/1.1 200 OK\r\n";
 // header += "Content-Type: application/json\r\n";
  header += "Content-Type: text/html\r\n";
  header += "Connection: close\r\n";  
  header += "Content-Length: ";
  header += (int)(content.length());
  header += "\r\n\r\n";
  //header += content;

  esp8266.print("AT+CIPSEND="); // ответ клиенту
  esp8266.print(ch_id);
  esp8266.print(",");
  esp8266.println(header.length()+content.length());

  clearBuffer();
  
  long int time = millis();
  i = 0;
  while( (time+100) > millis())
   {
    while(esp8266.available() && i < BUFFER_SIZE )
     {   
       buffer[i] = esp8266.read(); // read the next character.
       i++;
     }
   }  
  // Serial.print(buffer); 
  
  if (strstr(buffer, ">") != 0) {
      Serial.println("Read > ");
      esp8266.print(header);
      esp8266.print(content);
    } else {
      Serial.println(" NOT Read > ");
      esp8266.println("+++");
    }
}

 //////////////////////очистка ESPport////////////////////
void clearSerialBuffer(void)
{
       while ( esp8266.available() > 0 )
       {
         esp8266.read();
       }
}

////////////////////очистка буфера////////////////////////
void clearBuffer(void) {
       for (i =0;i<BUFFER_SIZE;i++ )
       {
         buffer[i]=0;
       }
}

////////////////////Отправка данных в ESP////////////////////////
uint8_t sendData(String command, const int timeout, boolean debug)
{

 //   Serial.println("In send Data");
    // Serial.println(command);
    clearBuffer();
  
    esp8266.print(command);           // send the read character to the esp8266
    
    long int time = millis();
    i = 0;
    while( (time+timeout) > millis())
    {
      while(esp8266.available() && i < BUFFER_SIZE )
      {   
        buffer[i] = esp8266.read(); // read the next character.
        i++;
        }
      }  
   if (DEBUG) {Serial.print(buffer); } 
   
   if (strstr(buffer, "OK") != 0) {
      return OK;
    } else {
      return ERR;
    }
        
}
