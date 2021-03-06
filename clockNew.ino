/*
  Программа для часов на газоразрядных лампах с возможностью управления через приложение по WiFi (с помощью модуля esp8266)
 */

#include "wiring_private.h"
#include "pins_arduino.h"

// #include <SoftwareSerial.h>
// #include <Time.h>

#include <ArduinoJson.h>
#include <OneWire.h>
#include <TimeLib.h>
#include <DallasTemperature.h>
#include <DS1307RTC.h>
#include "demo.h"
#include <avr/wdt.h>
#include <avr/io.h>
#include <util/delay.h>



// Объявляем переменные и константы
#define DEBUG false 
#define BUFFER_SIZE 180
#define NUMITEMS(arg) ((size_t) (sizeof (arg) / sizeof (arg [0])))
#define ONE_WIRE_BUS A3                //Это вывод для подключения вывода DS, при распайке на плате RTS1703 датчика температуры                  
#define TEMPERATURE_PRECISION 9
#define DELTA_SHIM_FOR_ANIMATION 4
#define MAX_SHIM_FOR_ANIMATION 150
#define DELAY_ANIMATION 7         // чем больше, тем медленее перебираются цифры
#define DELAY_SHOW 1
#define OK 0
#define ERR 1
#define DATA 2 
#define ssid "home"
#define pass "xxxxx"
#define ntp "89.109.251.21"
#define timeZone 3
/*
//Команды для подачи HIGHT / LOW сигнала на выводы регистров в соотвевствии с используемыми выводами arduino 
const uint8_t Pin_1_a = 13;            //было                
#define Pin_1_a_OFF PORTB &= ~(1<<5);     // стало для LOW
#define Pin_1_a_ON PORTB |= (1<<5);       // стало для HIGHT
const uint8_t Pin_1_b = 12;
#define Pin_1_b_OFF PORTB &= ~(1<<4);
#define Pin_1_b_ON PORTB |= (1<<4);
const uint8_t Pin_1_c = 4;
#define Pin_1_c_OFF PORTD &= ~(1<<4);
#define Pin_1_c_ON PORTD |= (1<<4);
const uint8_t Pin_1_d = 2;
#define Pin_1_d_OFF PORTD &= ~(1<<2);
#define Pin_1_d_ON PORTD |= (1<<2);

// Анодные пины
#define Pin_a_1 11
#define Pin_a_1_OFF PORTB &= ~(1<<3);
#define Pin_a_1_ON PORTB |= (1<<3);
#define Pin_a_2 10
#define Pin_a_2_OFF PORTB &= ~(1<<2);
#define Pin_a_2_ON PORTB |= (1<<2);
#define Pin_a_3 9
#define Pin_a_3_OFF PORTB &= ~(1<<1);
#define Pin_a_3_ON PORTB |= (1<<1);
#define Pin_a_4 6
#define Pin_a_4_OFF PORTD &= ~(1<<6);
#define Pin_a_4_ON PORTD |= (1<<6);


// Настройка команд для чтения из портов состояния подключенных кнопок
#define Pin_rt1_Read (PINC & B00000001)
#define Pin_rt2_Read ((PINC & B00000010)>>1)

//Настройка команд для бипера
// const int Buzz_1 = A2;
#define Pin_Buzz_ON PORTC |= (1<<2);
#define Pin_Buzz_OFF PORTC &= ~(1<<2);

//Настройка команд для точек
// const uint8_t Pin_dot1 = 5;
#define Pin_dot1_OFF PORTD &= ~(1<<5);
#define Pin_dot1_ON PORTD |= (1<<5);
// const uint8_t Pin_dot1 = 7;
#define Pin_dot2_OFF PORTD &= ~(1<<7);
#define Pin_dot2_ON PORTD |= (1<<7);
*/
HardwareSerial &ESPport = Serial;


// SoftwareSerial esp8266(8, 9); // RX, TX
// При отладке Программный порт Serial используется для подключения к ESP8266,
// Аппаратный Serial для вывода отладочной информации.
// В боевой версии Аппаратный Serial (вывода 0-RX/1-TX используется для подключения к ESP8266,
// вывод отладочной информации в консоль заккоментирован

const int COM_BAUD = 9600;
const int NTP_PACKET_SIZE = 48;        // NTP time stamp is in the first 48 bytes of the message

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress insideThermometer;

//Блок общих переменных скетча
// К155ИД1 (1)
const uint8_t Pin_2_a = 5;                
const uint8_t Pin_2_b = 6;
const uint8_t Pin_2_c = 7;
const uint8_t Pin_2_d = 8;

// К155ИД1 (2)
const uint8_t Pin_1_a = 13;                
const uint8_t Pin_1_b = 12;
const uint8_t Pin_1_c = 4;
const uint8_t Pin_1_d = 2;

// Анодные пины
const uint8_t Pin_a_1 = 11;//колбы 1, 4
const uint8_t Pin_a_2 = 10;//колбы 2, 5
const uint8_t Pin_a_3 = 9; //колбы 3, 6       

//Пины для кнопок 
const uint8_t Pin_rt1 = A0;   //Пока будем использовать аналоговые как цифровые
const uint8_t Pin_rt2 = A1;  

//Пин для подсветки
const uint8_t Led_1 = 3; 

//Пин для бипера
const int Buzz_1 = A2;           


//Массив для управления анодами ламп
static const uint8_t anods[3] = {Pin_a_1, Pin_a_2, Pin_a_3};

//Массив с помощью которого дешефратору задаются цифры
static const uint8_t numbers[11][4] = 
{
    { 0, 0, 0, 1 }, //0
    { 0, 0, 1, 0 }, //1
    { 0, 0, 1, 1 }, //2
    { 0, 1, 0, 1 }, //3
    { 1, 0, 0, 1 }, //4
    { 1, 0, 0, 0 }, //5
    { 0, 1, 1, 1 }, //6
    { 0, 1, 1, 0 }, //7
    { 0, 1, 0, 0 }, //8
    { 0, 0, 0, 0 }, //9
    { 1, 1, 1, 1 }  //Чисто
};

// Массив для анимации, перебор всех цифр в колбе
static const uint8_t nixie_level[10] = {
    6, 7, 9, 4, 8, 5, 9, 2, 0, 3
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
uint8_t Seconds_old = 0;
uint8_t timeset = 0;
uint8_t alarmclockset = 0;
uint8_t alarmHour = 0;
uint8_t alarmMin = 0;
uint8_t dayNight = 255;

uint8_t btn1;
uint8_t btn2;
    
float tempC = 0;                    
bool sensorTemperatureIn = false;
boolean isAlarm = false;
bool esp8266in = false;
bool mode_auto = true;
// boolean play = false;

uint8_t a;
float b;
uint8_t i=0, j=0, z=0, y=0;

bool isReadTemperature = false;
boolean up = false;         //Признак нажатия любойй из кнопок
boolean animate = false;
// boolean sec = true;      //не используется


uint8_t dateTimeSet;         //Для установки времени/даты с телефона
uint8_t alarmSet;

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

long int time_;                           // Для отслеживания задержки при чтения из последовательного порта

const int delayFalse = 10;                // Длительность, меньше которой не регистрируется единоразовый клик
const int delayLongSingleClick = 1000;    // Длительность зажатия кнопки для выхода в режим увеличения громкости
const int delayDeltaDoubleClick = 800;    // Длительность между кликами, когда будет зафиксирован двойной клик

uint8_t time_hh = 0 ;
uint8_t time_mm = 0;
uint8_t time_ss = 0;
bool backward = false;
bool isTimerOn = false;
bool notSync = true;

char send_[] = {"AT+CIPSEND="};
char buffer[BUFFER_SIZE];
char *pb;

time_t t;
/*
//для динамика
const int tempo = 200;

// массив для наименований нот (до ре ми ... и т.д. в пределах двух октав) 
 const char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C','D','E','F','G','A','B' }; 
 // соответствующие нотам частоты 
 const int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956, 850, 759, 716, 638, 568, 507 }; 

// ноты мелодии   
const char notes[] = "GECgabCaCg DGECabCDED"; // пробел - это пауза 
// длительность для каждой ноты и паузы 
const uint8_t beats[] = { 4, 4, 4, 4, 1, 1, 1, 2, 1, 4, 
                          2, 4, 4, 4, 4, 1, 1, 1, 2, 1, 
                          4}; 
                          
const uint8_t length = sizeof(notes); // количество нот  
*/
int changeButtonStatus(int buttonPin);
void setNixieNum(uint8_t tube, uint8_t num);
void DisplayNumberSet(uint8_t anod, uint8_t num1, uint8_t num2 );
void DisplayNumberSetA(uint8_t anod, uint8_t num1, uint8_t num2 );
void DisplayNumberString( uint8_t* array );
void sendReply(int ch_id);
void clearSerialBuffer(void);
// void clearBuffer(void);
uint8_t sendData(String command, const int timeout, boolean debug);
void playMusic();
void playNote(char note, int duration);
void playTone(int tone, int duration); 
time_t getNtpTime();

tmElements_t tm;

String content;
  
void setup()  
{

    pinMode(Pin_2_a, OUTPUT);
    pinMode(Pin_2_b, OUTPUT);
    pinMode(Pin_2_c, OUTPUT);
    pinMode(Pin_2_d, OUTPUT);
    pinMode(Pin_1_a, OUTPUT);
    pinMode(Pin_1_b, OUTPUT);
    pinMode(Pin_1_c, OUTPUT);
    pinMode(Pin_1_d, OUTPUT);
    pinMode(Pin_a_1, OUTPUT);
    pinMode(Pin_a_2, OUTPUT);
    pinMode(Pin_a_3, OUTPUT);
    pinMode(Buzz_1, OUTPUT);
    pinMode(Led_1, OUTPUT);

    //    pinMode(Pin_dot1, OUTPUT);
    //    pinMode(Pin_dot2, OUTPUT);

    pinMode(Pin_rt1, INPUT);
    pinMode(Pin_rt2, INPUT);
    
 //   analogWrite(Led_1, 1);
 //   digitalWrite(Buzz_1, 0);
   sensors.begin();
   wdt_disable(); 
    if (sensors.getAddress(insideThermometer, 0)) {
        // sensorTemperatureIn = true;
         sensorTemperatureIn = false;
        sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
   }
   
   // Инициализируем последовательный интерфейс и ждем открытия порта:
    //  esp8266.begin(COM_BAUD);
    ESPport.begin(COM_BAUD);
    while(!ESPport){
    
      } 
    sendData("AT+RST\r\n",2000,DEBUG);
    // sendData("ATE1\r\n",500,DEBUG);  
      
     //Как точка доступа     
    sendData("AT+CWMODE=2\r\n",300,DEBUG);
    sendData("AT+CIPMUX=1\r\n",500,DEBUG);
    if (sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG)== OK){
       esp8266in = true;
     } else {
      esp8266in = false; 
     }

/*    
    // Подключение к существующей точке
    sendData("AT+CWMODE=3\r\n",1000,DEBUG); 
    // sendData("AT+CWQAP\r\n",500,DEBUG);
    // sendData("AT+CWLAP?\r\n", 1000, DEBUG);
    sendData("AT+CWDHCP=1,1\r\n", 1000, DEBUG);
    String cmd="AT+CWJAP=\"";
    cmd+=ssid;
    cmd+="\",\"";
    cmd+=pass;
    cmd+="\"";
    cmd+="\r\n";
    sendData(cmd, 3400,DEBUG);
    sendData("AT+CIFSR\r\n", 2000, DEBUG); // получаем адрес
    sendData("AT+CIPMUX=1\r\n",500,DEBUG);
    if (sendData("AT+CIPSERVER=1,80\r\n",2000,DEBUG)== OK){
       esp8266in = true;
     } else {
      esp8266in = false; 
     }
     */    
    wdt_enable(WDTO_8S);        //Установка тамера для перезагрузки при подвисании программы
  
}


void loop() // выполняется циклически
{
    wdt_reset();                                //Циклический сброс таймера 
    StaticJsonBuffer<180> jsonBuffer;
    int ch_id, packet_len;
  
    
    // Работа с WiFi модулум esp8266
    //Чтение кнопок идет до обработки полученных данных через WiFi
   btn1 = digitalRead(Pin_rt1);
   btn2 = changeButtonStatus(Pin_rt2); 

   RTC.read(tm);
   Mins = tm.Minute;
   Seconds_old = Seconds;
   Seconds = tm.Second;
   hours = tm.Hour;
   
   if (esp8266in) {
    if ((notSync)&&(hours==00)&&(Seconds==00)) {       //Запуск синхронизации времени один раз при наступлении 00 час 00
      t = getNtpTime();
      if (t !=0) {
        RTC.set(t);
        notSync = false;
        }
      }
      if ((hours==01)&&(Seconds==00)&&(!notSync)) notSync = true; 
      
      if (ESPport.available()){   // esp8266in    Если был получен успешный ответ о старте сервера и есть что читать 
        // ESPport.setTimeout(400);
        memset(buffer, 0, BUFFER_SIZE);
        ESPport.readBytesUntil('\n', buffer, BUFFER_SIZE);
        if (strncmp(buffer,"+IPD,", 5)==0) {         //Сравниваем считанное с  "+IPD,".
            //    Serial.println("Incomming connection");
            //    Serial.println(buffer);
            sscanf(buffer+5, "%d,%d", &ch_id, &packet_len);   //Считываем из буфера значения идентификатора подключения и длинну пакета.
            if (packet_len > 0){
                pb = buffer+5;
                while(*pb!=':') pb++;
                pb++;
            }
            if((strncmp(pb, "GET / ", 6) == 0) || (strncmp(pb, "GET /?", 6) == 0))
            {
                clearSerialBuffer();
                JsonObject& root = jsonBuffer.createObject();
                content = "";
                root["mode"] = mode;
                root["hh"] = hours;
                root["min"] = Mins;
                root["sec"] = Seconds;
                root["dd"] = tm.Day;
                root["mm"] = tm.Month;
                root["yy"] = tmYearToY2k(tm.Year);
                // root["tset"] = timeset;
                // root["alSet"] = alarmclockset;
                root["m_a"] = mode_auto;
                root["led"] = dayNight;
                root["tC"] = tempC;                    //(int)tempC/10
                // root["sIn"] = sensorTemperatureIn;
                root["isAl"] = isAlarm;
                root["alHour"] = alarmHour;
                root["alMin"] = alarmMin;
                // root["btn1"] = btn1;
                // root["btn2"] = btn2;
                // root["play"] = play;
                root["isT"] = isTimerOn;
                root["tHH"] = time_hh;
                root["tMM"] = time_mm;
                root["tSS"] = time_ss;
                root["tBd"] = backward;
 
                root.printTo(content);
                // sendReply(ch_id);
            } else if((strncmp(pb, "PUT / ", 6) == 0) || (strncmp(pb, "SET /?", 6) == 0))
            {
                time_ = millis();
                while( (time_ + 400) > millis())
                {
                    while(ESPport.available())
                    {
                        ESPport.readBytesUntil('\n', buffer, BUFFER_SIZE);
                        if (buffer[0] == '{')          //Признак строки с json данными
                        {
                            JsonObject& root = jsonBuffer.parseObject(buffer);
                            if (root.success()) {
                                btn1 = root["btn1"];          //Считываем их из пакета по сети
                                btn2 = root["btn2"];
                                if (btn1==1 && btn2==0) {     //Если в пакете пришла информация что ни одна из кнопок в приложении не нажата 
                                                              // (внимание btn1 и btn1 используют разные признаки), то извлекаем остальные параметры
                                  mode = root["mode"];
                                  // hours = root["hours"];
                                  // Mins = root["mins"];
                                  // Seconds = root["sec"];
                                  isAlarm = root["isAl"];
                                  alarmSet = root["alSet"];
                                  if (alarmSet==1) {
                                    alarmHour = root["alHour"];
                                    alarmMin = root["alMin"];
                                  }
                                  // alarmclockset = root["alSet"];
                                  dateTimeSet = root["tset"];
                                  if (dateTimeSet==1){
                                   tm.Hour = root["hh"];    
                                   tm.Minute = root["min"];  
                                   tm.Second = root["sec"];
                                   RTC.write(tm);
                                  } else if (dateTimeSet==2) {
                                   tm.Day = root["dd"];
                                   tm.Month = root["mm"];
                                   tm.Year = y2kYearToTm(int(root["yy"]));
                                   RTC.write(tm);
                                  }
                                  
                                  mode_auto = root["m_a"];
                                  dayNight = root["led"];
                                  // play = root["play"];
                                  backward = root["tBd"];
                                  if (!isTimerOn) {               //Если таймер в часах выключен
                                     isTimerOn = root["isT"];     //Читаем и сохраняем новое значение
                                    if (isTimerOn) {              //Если пришло вклчение, то включаем
                                      time_hh = root["tHH"];
                                      time_mm = root["tMM"];
                                      time_ss = root["tSS"];
                                    }
                                  } else if(root["isT"]) {        //Если в пакете пришла команда на выключение                                                             
                                    isTimerOn = root["isT"];      //Считываем ее, обнуляем значение таймера. Иначе ничего не делаем, т.е. оставляем таймер включенным с текущими значениями 
                                    time_ss = 0;
                                    time_mm = 0;
                                    time_hh = 0;
                                  }
                              }                            
                            } 
                             isReadTemperature = false;  //Для однократного чтения температуры при получении пакета.
                        }
                        memset(buffer, 0, BUFFER_SIZE);
                        delay(20);
                    }
                }
                content = "";
                // sendReply(ch_id);
            }
            memset(buffer, 0, BUFFER_SIZE);
            char h1[] =  "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: ";
            i=0;
            y=0;
            while (h1[i]!= 0) {
              buffer[y] = h1[i];
              i++;
              y++;
            }         
            String len = String(content.length());
            char h2[4];
            len.toCharArray(h2, 4);
            i = 0;
            while (h2[i]!= 0) {
              buffer[y] = h2[i];
              i++;
              y++;
            } 
            i = 0;
            char h3[] = "\r\n\r\n";
            while (h3[i]!= 0) {
              buffer[y] = h3[i];
              i++;
              y++;
            }
        
            ESPport.print(send_); // ответ клиенту
            ESPport.print(ch_id);
            ESPport.print(",");
            ESPport.println(y + content.length());
        
            delay(20);
            if (ESPport.find(">")) {
                //  Serial.println("Read > ");
                ESPport.print(buffer);
                ESPport.print(content);
                delay(200);
                }
         memset(buffer, 0, BUFFER_SIZE);
        }
      }    
    }
    
    //Счетчик для анимации
    if (z==DELAY_ANIMATION)
    {
        j++;
        z=0;
    }
    if (j==10) {animate=false; j=0; z=0;}
    if (animate) z++;

    if (isAlarm) {                 //если установлен будильник горят точки
     //   digitalWrite(Pin_dot1, HIGH);
     //   digitalWrite(Pin_dot2, HIGH);
        if (alarmHour==tm.Hour&&alarmMin==Mins&&alarmclockset==0) {
            isAlarm = false;
            playMusic();
        }

    }/* else {
        digitalWrite(Pin_dot1, LOW);
        digitalWrite(Pin_dot2, LOW);
    }*/
    if (dayNight!=0){                              // Единицу присылаем из проложения - это равносильно выключению, 
                                                    // В остальных случаях подсветку устанавливаем по времени 
      if((tm.Hour>=8)&&(tm.Hour<20)) dayNight=255;
      if((tm.Hour>=20)&&(tm.Hour<22)) dayNight=40;
      if((tm.Hour>=22)&&(tm.Hour<0)) dayNight=10;
      if((tm.Hour>=0)&&(tm.Hour<8)) dayNight=1;  //1
    }
    analogWrite(Led_1, dayNight);  

// Работа таймера
    //Если включен таймер
    if (isTimerOn) {
      // Если изменилась секунда
      if  (Seconds_old != Seconds) {
       //Считаем в прямом режиме
       if (!backward) {
         time_ss += 1;
         if (time_ss==60){
          time_ss = 0;
          time_mm +=1;
          if (time_mm==60){
            time_mm = 0;
            time_hh +=1;
          }
         }
       } else {
         time_ss -= 1;
        if (time_ss==255) {
          time_ss = 59;
          time_mm -= 1;
          if (time_mm == 255) {
            time_mm = 59;
            time_hh -= 1;
            if (time_hh == 255) {
              // time_hh = 99;
                playMusic();
                isTimerOn = false;
                time_ss = 0;
                time_mm = 0;
                time_hh = 0;
            }
          }
        }
       }
      }
    }

    switch(mode)
    {
    case 0:
        NumberArray[0] = tm.Hour / 10; //Первый знак часа
        NumberArray[1] = tm.Hour % 10; //Второй знак часа
        NumberArray[2] = Mins / 10; //Первый знак минут
        NumberArray[3] = Mins % 10; //Второй знак минут
        NumberArray[4] = Seconds / 10; //Первый знак секунд
        NumberArray[5] = Seconds % 10; //Второй знак секунд

        break;
    case 1:
        NumberArray[0] = tm.Day / 10; //Первый знак дня
        NumberArray[1] = tm.Day % 10; //Второй знак дня
        NumberArray[2] = tm.Month / 10; //Первый знак месяца
        NumberArray[3] = tm.Month % 10; //Второй знак месяца
        NumberArray[4] = tmYearToY2k(tm.Year) / 10; //Первый знак года
        NumberArray[5] = tmYearToY2k(tm.Year) % 10; //Второй знак года
        /* //Мигание точками при показе даты пока убрано
        if(timeset==0&&alarmclockset==0)
        {
            if ((Seconds % 10)%2==0)           ////Если знак секунды четный то включаем иначе выкл
            {
                digitalWrite(Pin_dot1, HIGH);
                digitalWrite(Pin_dot2, HIGH);
            }
            else
            {
                digitalWrite(Pin_dot1, LOW);
                digitalWrite(Pin_dot2, LOW);
            }
        }
        */
        break;

    case 2:                               //Режим отображения установок будильника
        NumberArray[0] = alarmHour / 10; //Первый знак часа
        NumberArray[1] = alarmHour % 10; //Второй знак часа
        NumberArray[2] = alarmMin / 10; //Первый знак минут
        NumberArray[3] = alarmMin % 10; //Второй знак минут
        NumberArray[4] = Seconds / 10; //Первый знак секунд
        NumberArray[5] = Seconds % 10; //Второй знак секунд

        //digitalWrite(Pin_dot1, HIGH);
        //digitalWrite(Pin_dot2, HIGH);

        /*
         //Убрано. пока нет свободных пинов для включения точек
        if (isAlarm) {
            digitalWrite(Pin_dot1, HIGH);
            digitalWrite(Pin_dot2, HIGH);
        }
        */
        break;

    case 3:                                //отображение температуры
        if(sensorTemperatureIn)
        {
            if (!isReadTemperature)
            {
                sensors.requestTemperatures();
                tempC = sensors.getTempC(insideThermometer);            // Поправка введена в связи с неточностью работы датчика
                isReadTemperature = true;
                b = (tempC - int(tempC))*100;
                //Serial.println((int)b/10);
                //Serial.println((int)b%10);
                NumberArray[0] = (int)tempC/10; //Первый
                NumberArray[1] = (int) tempC%10; //Второй
                NumberArray[2] = (int)b/10; //Первый после запятой
                NumberArray[3] = 10;        //пусто
                NumberArray[4] = 10;        //пусто
                NumberArray[5] = 10;
                //NumberArray[5] =(int)b%10; //Второй знак после запятой

            }
            
            millisThis = millis();
            if(millisThis - millisAnimation > 700) {  //Если пауза вышла двигаем колбы влево
                
                a = NumberArray[0];
                NumberArray[0] = NumberArray[1];        //пусто
                NumberArray[1] = NumberArray[2];        //пусто
                NumberArray[2] = NumberArray[3]; //Первый
                NumberArray[3] = NumberArray[4]; //Второй
                NumberArray[4] = NumberArray[5]; //Первый после запятой
                NumberArray[5] = a;
                millisAnimation = millisThis;
            }
        }
        else mode = 0;
        break;

    case 4:                      //режим анимации
        //  if(a < NUMITEMS(NumberAnimationDelay)){                   //не первышаем количество шагов анимации
        if(a < 1){                                                    //тут должно быть количество шагов анимации
            //   Serial.println("Animation step ");
            //   Serial.println("a");
            for (i=0; i<6; i++) {
                NumberArray[i] = NumberAnimationArray[a][i];         //устанавливаем значения колб для анимации
            }

            millisThis = millis();                                 //время сейчас
            // unsigned int mills = NumberAnimationDelay[a];
            if(millisThis - millisAnimation > NumberAnimationArray[a][6]) {  //Если время на анимацию одного шага вышло, переходим к другому
                a++;
                millisAnimation = millisThis;
            }

        } else {                                                      //Если количество шагов исчерпано, выходим в режим 1
            mode = 0;
            playMusic();                                                //Включаем музыку
        }
        break;
     case 5:            //Режим отображения таймера
        
        NumberArray[0] = time_hh / 10; //Первый знак часа
        NumberArray[1] = time_hh % 10; //Второй знак часа
        NumberArray[2] = time_mm / 10; //Первый знак минут
        NumberArray[3] = time_mm % 10; //Второй знак минут
        NumberArray[4] = time_ss / 10; //Первый знак секунд
        NumberArray[5] = time_ss % 10; //Второй знак секунд
        break;   
     case 6:            //Режим выключения ламп, тупо подаем 10 на дешифраторы и ничего на них не отображаем
        /*
        NumberArray[0] = 10; 
        NumberArray[1] = 10; 
        NumberArray[2] = 10; 
        NumberArray[3] = 10;    //   
        NumberArray[4] = 10;        
        NumberArray[5] = 10;
        */
        dayNight=0;
        break;
    }

    if  (timeset==0&&alarmclockset==0){      //Мы не в режиме установки времени, часов и не в режиме анимации, таймера, выключения.
        //Каждые пол часа пищим
        // if ((Mins == 0)&&sec||(Mins == 30)&&sec)
        // {
        //  tone(Buzz_1,100, 100);
        //  sec=false;
        // }                               убрал, т.к. иногда мешает
        // if ((Mins == 1)&&!sec||(Mins == 31)&&!sec)
        // {
        //   sec=true;
        // }
        //Каждые 58 секунд включаем время
     if (mode_auto&&mode < 4)     //Если выбран режим авто смены и мы не в режимах 4, 5, 6
       {
        if (Seconds==58)
        {
            mode=0;
            animate=true;
            //            printConsoleTime();
            //
           // Serial.println("Time On ");
        }
        //Включаем дату на 47 секунду
        if (Seconds==47)
        {
            mode=1;
            animate=true;
            isReadTemperature = false;                //Для однократного чтения тепературы
            //            printConsoleTime();
            //            Serial.println("Date On ");
            
        }
        //Включаем температуру на 53 секунду
        if ((Seconds==52)&(sensorTemperatureIn))
        {
            mode=3;
            animate=true;
            //            printConsoleTime();
            //            Serial.println("Date On ");
        }
       } 

        //Переключаем режимы
        if (!btn1&&!up)
        {
            up=true;
            animate=true;
            dayNight=1;
            tone(Buzz_1,100, 100);
            mode++;
            mode %= 7;          //перебор всех режимов отображения 
            if (mode==4) {      //Если перешли к демо режиму
                animate=false;    //обычную анимацию отключаем
                a=0;              //переход к первому шагу анимации
                millisAnimation = millis();                  //фиксируем время начала анимации
            }
            if (mode==3){
             // isReadTemperature = false;   //Для чтения температуры при ручной смене режима отображения
              
              mode=6;
            }
            // printConsoleTime();
            // Serial.print("Mode change to - ");
            // Serial.println(mode);
            // printConsoleTime(); // Для отладки
        }

        if (btn1&&up)
        {
            up=false;
        }
  }
    //Перебор настоек при смене времени (вход) по длительному нажатию Pin_rt2
   
    if (btn2==1) {          //Если мы в режиме смены времени то реагируем на одиночные нажатия внопки для выборя изменяемого параметра
        // Serial.println("In change");
        if (timeset!=0&&alarmclockset==0){     //Изменение параметров для установки часов
            timeset++;
            tone(Buzz_1,100, 100);
            if (timeset>=7)
            {
                timeset=1;
            }
            //  Serial.print("In time change, timeset is ");
            //  Serial.println(timeset);
        }
        if (timeset==0&&alarmclockset!=0){   //Изменение параметров для установки будильника
            alarmclockset++;
            if (alarmclockset>=3) alarmclockset = 1;
            tone(Buzz_1,100, 100);
            //  Serial.println("In alarm set, alarm is ");
            // printConsoleAlarm();
        }
    }
    if (btn2==4) {          //Отжата кнопка после долгого нажатия и мы не были в режиме смены даты времени
        if (timeset==0){
            // Serial.print("In time change begin");
            timeset = 1;            //Взаимоисключающие режимы
            mode = 0;
            alarmclockset = 0;
            //  Serial.println(timeset);
        }else{
            //  Serial.println(timeset);
            //  Serial.print("Exit on change time");
            mode = 0;
            timeset = 0;
        }
    }
    if (btn2==2) {            //двойной клик - это переход к установке будильника
        //  Serial.println("In duble click, set or change alarm");
        if (alarmclockset==0) {   //если не в режиме установки
            //    Serial.println("Set or change alarm");
            mode = 2;
            timeset = 0;
            alarmclockset = 1;
            
            //    Serial.println(alarmclockset);
        }else {               //Если мы уже были в процессе установки
            //   Serial.println("Alarm set in ");
            // printConsoleAlarm();
            //тут будет код для записи часов и мин в EPROM
            isAlarm = true;
            alarmclockset = 0;
            mode = 0;
        }

    }

    switch(alarmclockset)
    {
    // Установка будильника
    case 1:         //показываем и меняем часы
        //   digitalWrite(Pin_dot1, HIGH);         //над показом точек думаем
        //   digitalWrite(Pin_dot2, HIGH);
        mode=2;                                 //специальный режим для отображения значений из установок ЧЧ:ММ будильника
        NumberArray[2] = 10;
        NumberArray[3] = 10;
        NumberArray[4] = 10;
        NumberArray[5] = 10;
        if (!btn1&&!up)
        {
            up=true;
            alarmHour++;       // увеличиваем час смотрим что бы не больше 24
            alarmHour %=24;
            tone(Buzz_1,100, 100);
            // printConsoleAlarm();
        }
        if (btn1&&up) up=false;
        break;
    case 2:       //Показываем и меняем минуты будильника

        NumberArray[0] = 10;
        NumberArray[1] = 10;
        NumberArray[4] = 10;
        NumberArray[5] = 10;
        if (!btn1&&!up)
        {
            up=true;
            alarmMin++;
            alarmMin %=60;
            tone(Buzz_1,100, 100);
            //  printConsoleAlarm();
        }
        if (btn1&&up) up=false;
        break;
    }

    switch (timeset)
    {
    //Установка часов
    // printConsoleTime();
    case 1:
      //  digitalWrite(Pin_dot1, HIGH);
      //  digitalWrite(Pin_dot2, HIGH);
        mode=0;
        NumberArray[2] = 10;
        NumberArray[3] = 10;
        NumberArray[4] = 10;
        NumberArray[5] = 10;
        if (!btn1&&!up)
        {
            up=true;
            tm.Hour++;       // увеличиваем час смотрим что бы не больше 24
            tm.Hour %=24;
            RTC.write(tm);
            tone(Buzz_1,100, 100);
            // printConsoleTime();
        }
        if (btn1&&up) up=false;
        break;
        //Установка минут
    case 2:
     //   digitalWrite(Pin_dot1, HIGH);
     //   digitalWrite(Pin_dot2, HIGH);
        mode=0;
        NumberArray[0] = 10;
        NumberArray[1] = 10;
        NumberArray[4] = 10;
        NumberArray[5] = 10;
        if (!btn1&&!up)
        {
            up=true;
            tm.Minute++;
            tm.Minute %=60;
            RTC.write(tm);
            tone(Buzz_1,100, 100);
            // printConsoleTime();
        }
        if (btn1&&up) up=false;
        break;
        //Установка секунд
    case 3:
     //   digitalWrite(Pin_dot1, HIGH);
     //   digitalWrite(Pin_dot2, HIGH);
        mode=0;
        NumberArray[0] = 10;
        NumberArray[1] = 10;
        NumberArray[2] = 10;
        NumberArray[3] = 10;
        if (!btn1&&!up)
        {
            up=true;
            tm.Second++;
            tm.Second %=60;
            RTC.write(tm);
            tone(Buzz_1,100, 100);
            //  printConsoleTime();
        }
        if (btn1&&up) up=false;
        break;
        //Установка дня
    case 4:
        mode=1;
      //  digitalWrite(Pin_dot1, HIGH);
     //   digitalWrite(Pin_dot2, HIGH);
        NumberArray[2] = 10;
        NumberArray[3] = 10;
        NumberArray[4] = 10;
        NumberArray[5] = 10;
        if (!btn1&&!up)
        {
            up=true;
            tm.Day++;
            tm.Day%=32;
            if (tm.Day==0) tm.Day = 1;            //День нулевым быть не может
            RTC.write(tm);
            tone(Buzz_1,100, 100);
            //  printConsoleTime();
        }
        if (btn1&&up) up=false;
        break;
        // Установка месяца
    case 5:
        mode=1;
     //   digitalWrite(Pin_dot1, HIGH);
     //   digitalWrite(Pin_dot2, HIGH);
        NumberArray[0] = 10;
        NumberArray[1] = 10;
        NumberArray[4] = 10;
        NumberArray[5] = 10;
        if (!btn1&&!up)
        {
            up=true;
            tm.Month++;
            tm.Month %=13;
            if (tm.Month==0) tm.Month = 1;            //День нулевым быть не может
            RTC.write(tm);
            tone(Buzz_1,100, 100);
            //  printConsoleTime();
        }
        if (btn1&&up) up=false;
        break;
        //Установка года
    case 6:
        mode=1;
    //    digitalWrite(Pin_dot1, HIGH);
    //    digitalWrite(Pin_dot2, HIGH);
        NumberArray[0] = 10;
        NumberArray[1] = 10;
        NumberArray[2] = 10;
        NumberArray[3] = 10;
        if (!btn1&&!up)
        {
            up=true;
            tm.Year++;
            RTC.write(tm);
            tone(Buzz_1,100, 100);
            //  printConsoleTime();
        }
        if (btn1&&up) up=false;
        break;
    }
    for (i=0; i<6; i++){
        if  (NumberArray[i]!=NumberArrayOLD[i]) isChangeArray[i] = true;       //Произошло изменение значения для отображения, нужно его анимировать
        NumberArrayOLD[i] = NumberArray[i];                                   //Сохраняем текущее значение на следующий цикл как старое
    }
    
    //Отображение на индикаторы
    DisplayNumberString( NumberArray );
}


//Функция ятения значений с кнопок
/**
 * Смена текущего статуса
 * @return = 0 - ничего не произошло
 *           1 - простой клик
 *           2 - двойнок клик
 *           3 - зажата кнопка
 *           4 - отжата кнопка после долгого зажатия
 */

int changeButtonStatus(int buttonPin) {
    // Событие
    int event = 0;

    // Текущее состояние кнопки
    int currentButtonClick = digitalRead(buttonPin);
    //serial.println(currentButtonClick);

    // Текущее время
    unsigned long timeButton = millis();

    switch(currentButtonStatus) {
    
    case 0:
        // В настоящий момент кнопка не нажималась

        if(currentButtonClick==0) {
            // Зафиксировали нажатие кнопки

            currentButtonStatus = 1;
            currentButtonStatusStart1 = millis();


        } else {
            // Кнопка не нажата
            // Ничего не происходит
        }
        break;

    case 1:
        // В настоящий момент кнопка на этапе первого нажатия

        if(currentButtonClick==0) {
            // Кнопка все еще нажата

            if(timeButton - currentButtonStatusStart1 >= delayLongSingleClick) {
                // Кнопка в нажатом состоянии уже дольше времени, после которого фиксируем длительное одинарное нажатие на кнопку

                // Событие длительного давления на кнопку - продолжаем давить
                event = 3;

            }

        } else {
            // Кнопку отжали обратно

            if(timeButton - currentButtonStatusStart1 < delayFalse) {
                // Время, которое была кнопка нажата, меньше минимального времени регистрации клика по кнопке
                // Скорее всего это были какие то флюктуации
                // Отменяем нажатие
                currentButtonStatus = 0;
                event = 0;

            } else if(timeButton - currentButtonStatusStart1 < delayLongSingleClick) {
                // Время, которое была кнопка нажата, меньше времени фиксации долгого нажатия на кнопку
                // Значит это первое одноразовое нажатие
                // Дальше будем ожидать второго нажатия
                currentButtonStatus = 2;
                currentButtonStatusStart2 = millis();
            } else {
                // Время, которое была нажата кнопка, больше времени фиксации долгого единоразового нажатия
                // Значит это завершение длительного нажатия
                currentButtonStatus = 0;
                event = 4;

            }

        }

        break;

    case 2:
        // Мы находимся в фазе отжатой кнопки в ожидании повторного ее нажатия для фиксации двойного нажатия
        // или, если не дождемся - значит зафиксируем единичное нажатие


        if(currentButtonClick==0) {
            // Если кнопку снова нажали

            // Проверяем, сколько времени кнопка находилась в отжатом состоянии
            if(timeButton - currentButtonStatusStart2 < delayFalse) {
                // Кнопка была в отжатом состоянии слишком мало времени
                // Скорее всего это была какая то флюктуация дребезга кнопки
                // Возвращаем обратно состояние на первичное нажатие кнопки
                currentButtonStatus = 1;

            } else {
                // Кнопка была достаточно долго отжата, чтобы зафиксировать начало второго нажатия
                // Фиксируем
                currentButtonStatus = 3;
                currentButtonStatusStart3 = millis();
            }

        } else {
            // Если кнопка все еще отжата

            // Проверяем, не достаточно ли она уже отжата, чтобы зафиксировать разовый клик
            if(timeButton - currentButtonStatusStart2 > delayDeltaDoubleClick) {
                // Кнопка в отжатом состоянии слишком долго
                // Фиксируем одинарный клие
                currentButtonStatus = 0;
                event = 1;
            }

        }

        break;

    case 3:
        // Мы на этапе второго нажатия
        // Для подтверждения факта двойного нажатия

        if(currentButtonClick==0) {
            // Кнопка все еще зажата
            // Ничего не происходит, ждем, когда отожмут

        } else {
            // Кнопку отжали

            // Проверям, действительно ли отжали, или это дребезг кнопки
            if(timeButton - currentButtonStatusStart3 < delayFalse) {
                // Кнопку отжали слишком рано
                // Скорре всего это дребезг
                // Гинорируем его

            } else {
                // Кнопка была в нажатом состоянии уже достаточно длительное время
                // Это завершение цикла фиксации двойного нажатия
                // Сообщаем такое событие
                event = 2;
                currentButtonStatus = 0;
            }
        }

        break;

    }
    //if(event!=0){
    //  Serial.println(event);
    //  Serial.println(buttonPin);
    //}
    return event;
}
/*
void DisplayNumberSet(uint8_t anod, uint8_t num) {      //Без ШИМ


    setNixieNum(num);           //Выводим на первый шифратор Num
    if (anods[anod]==Pin_a_1) {
      Pin_a_1_ON
    } else if (anods[anod]== Pin_a_2) {
      Pin_a_2_ON
    } else if (anods[anod]== Pin_a_3) {
      Pin_a_3_ON
    } else if (anods[anod]== Pin_a_4) {
      Pin_a_4_ON
    }
    _delay_ms(DELAY_SHOW);
    if (anods[anod]== Pin_a_1) {
      Pin_a_1_OFF
    } else if (anods[anod]== Pin_a_2) {
      Pin_a_2_OFF
    } else if (anods[anod]== Pin_a_3) {
      Pin_a_3_OFF
    } else if (anods[anod]== Pin_a_4) {
      Pin_a_4_OFF
    }
}

void setNixieNum(uint8_t num) {             //Отображает цифру num на лампе   

        if (!animate) {
          if (numbers[num][0]==0) { 
              Pin_1_d_OFF
            } else  {
              Pin_1_d_ON
            }
           if (numbers[num][1]==0) { 
              Pin_1_c_OFF
            } else  {
              Pin_1_c_ON
            }
            if (numbers[num][2]==0) { 
              Pin_1_b_OFF
            } else  {
              Pin_1_b_ON
            }
            if (numbers[num][3]==0) { 
              Pin_1_a_OFF
            } else  {
              Pin_1_a_ON
            }
       
         } else {                       //Если включен режим анимации н алампу идет цифра из массива анимации
            if (numbers[nixie_level[j]][0]==0) { 
              Pin_1_d_OFF
            } else  {
              Pin_1_d_ON
            }
           if (numbers[nixie_level[j]][1]==0) { 
              Pin_1_c_OFF
            } else  {
              Pin_1_c_ON
            }
            if (numbers[nixie_level[j]][2]==0) { 
              Pin_1_b_OFF
            } else  {
              Pin_1_b_ON
            }
            if (numbers[nixie_level[j]][3]==0) { 
              Pin_1_a_OFF
            } else  {
              Pin_1_a_ON
            }
    } 
}
*/
void setNixieNum(uint8_t tube, uint8_t num) {             //Отображает цифру num на лампе из групп 1 или 2  

    for(i=0; i<4; i++)
    {
        digitalWrite(bits[tube][i], LOW);//боримся против глюков - обнуляем
        if (!animate) digitalWrite(bits[tube][i], numbers[num][i]);
        if (animate)  digitalWrite(bits[tube][i], numbers[nixie_level[j]][i]);
    }

}

void DisplayNumberSet(uint8_t anod, uint8_t num1, uint8_t num2 ) {      //Без ШИМ
    setNixieNum(0, num1);           //Выводим на первый шифратор Num1
    setNixieNum(1, num2);           //Выводим на второй шифратор Num2
    digitalWrite(anods[anod], HIGH); // Подаем кратковременно сигнал на anod
    delay(DELAY_SHOW*2);
    digitalWrite(anods[anod], LOW);   //Убираем сигнал
}



void DisplayNumberSetA(uint8_t anod, uint8_t num1, uint8_t num2 ) {
    setNixieNum(0, NumberArray[num1]);     //Выводим на первый шифратор  Num1 из массива
    setNixieNum(1, 10);              //Ничего не выводим на второй шифратор
    if (isChangeArray[num1]==true) { //Если нужно анимировать
        ShimAnimationArray[num1] = 0;
        isChangeArray[num1] = false;
    }
    analogWrite(anods[anod], ShimAnimationArray[num1]);   // Подаем ШИМ сигнал из массива значений для 6-и колб
    delay(DELAY_SHOW);
    analogWrite(anods[anod], 0);    //Убираем ШИМ мигнал

    setNixieNum(0, 10);             //Ничего не выводим на первый шифратор
    setNixieNum(1, NumberArray[num2]);    //Выводим на второй шифратор Num2 из массива с анимацией
    if (isChangeArray[num2]==true) {
        ShimAnimationArray[num2] = 0;
        isChangeArray[num2] = false;
    }
    analogWrite(anods[anod],ShimAnimationArray[num2]); // Подаем ШИМ сигнал
    delay(DELAY_SHOW);
    analogWrite(anods[anod], 0);   //Убираем ШИМ мигнал

    for (i=0; i<6; i++){
        if (ShimAnimationArray[i]<MAX_SHIM_FOR_ANIMATION) ShimAnimationArray[i]=ShimAnimationArray[i] + DELTA_SHIM_FOR_ANIMATION;    //Если ведется анимация, то увеличиваем значение ШИМ сигнала
        else ShimAnimationArray[i]=255;
    }
}

void DisplayNumberSetOff() {      //Режим Off
    digitalWrite(anods[0], LOW); // Выключаем анод
    digitalWrite(anods[1], LOW); // Выключаем анод
    digitalWrite(anods[2], LOW); // Выключаем анод
    delay(DELAY_SHOW*4);
}

void DisplayNumberString( uint8_t* array ) {    //Функция для отображения строки цифр из массива array
/*
    if ((mode == 0) and (timeset==0))  {              //Если показ времени то с анимацией
        DisplayNumberSetA(0,0,3);   //Выводим на 1 анод (лампы 1,4) цифры 1,4 из массива
        DisplayNumberSetA(1,1,4);   //Выводим на 2 анод (лампы 2,5) цифры 2,5 из массива
        DisplayNumberSetA(2,2,5);   //Выводим на 3 анод (лампы 3,6) цифры 3,6 из массива
    }  
    */if(mode == 6) {        //Если режим с выключенными лампами
        DisplayNumberSetOff(); 
    } else  
    {                      //В других режимах анимации нет

        DisplayNumberSetA(0,0,3);   //Выводим на 1 анод (лампы 1,4) цифры 1,4 из массива
        DisplayNumberSetA(1,1,4);   //Выводим на 2 анод (лампы 2,5) цифры 2,5 из массива
        DisplayNumberSetA(2,2,5);   //Выводим на 3 анод (лампы 3,6) цифры 3,6 из массива
        /*
        DisplayNumberSet(0,array[0],array[3]);   //Выводим на 1 анод (лампы 1,4) цифры 1,4 из массива
        DisplayNumberSet(1,array[1],array[4]);   //Выводим на 2 анод (лампы 2,5) цифры 2,5 из массива
        DisplayNumberSet(2,array[2],array[5]);   //Выводим на 3 анод (лампы 3,6) цифры 3,6 из массива
        */
    }
}

/*
//////////////////////Отправка ответа на GET запрос////////////////////
void sendReply(int ch_id)
{

    // Serial.println("In Send Reply");
    header =  "HTTP/1.1 200 OK\r\n";
    // header += "Content-Type: application/json\r\n";
    header += "Content-Type: text/html\r\n";
    header += "Connection: close\r\n";
    header += "Content-Length: ";
    header += (int)(content.length());
    header += "\r\n\r\n";
    //header += content;

    ESPport.print(send_); // ответ клиенту
    ESPport.print(ch_id);
    ESPport.print(",");
    ESPport.println(header.length()+content.length());

    delay(20);
    if (ESPport.find(">")) {
        //  Serial.println("Read > ");
        ESPport.print(header);
        ESPport.print(content);
        delay(200);
    } 
}
*/
//////////////////////очистка ESPport////////////////////
void clearSerialBuffer(void)
{
    time_ = millis();
    while (((time_ + 400) > millis()) && ESPport.available())
    {
        ESPport.read();
    }
}
/*
////////////////////очистка буфера////////////////////////
void clearBuffer(void) {
    for (i = 0;i<BUFFER_SIZE;i++ )
    {
        buffer[i]=0;
    }
}
*/
////////////////////Отправка данных в ESP////////////////////////
uint8_t sendData(String command, const int timeout, boolean debug)
{

    //   Serial.println("In send Data");
    //   Serial.println(command);
    memset(buffer, 0, BUFFER_SIZE);

    ESPport.print(command);           // send the read character to the esp8266
    
    time_ = millis();
    i = 0;
    while( (time_+timeout) > millis())
    {
        while(ESPport.available() && i < BUFFER_SIZE )
        {
            buffer[i] = ESPport.read(); // read the next character.
            i++;
        }
    }
    //   if (DEBUG) {Serial.print(buffer); }

    if (strstr(buffer, "OK") != 0) {
        return OK;
    } else {
        return ERR;
    }

}
void playMusic()
{
  /*
  for (uint8_t i = 0; i < length; i++) 
        {
          if (notes[i] == ' ') { 
          delay(beats[i] * tempo); 
          }
          playNote(notes[i], beats[i] * tempo);
          delay(tempo / 4);           
        }
        */
  for (i = 0; i < 10; i++) {
    playTone(800, 800);
    delay(800);
    playTone(800, 80);
    delay(2000);
      
  }
}
/*
void playNote(char note, int duration) 
{
  
  // проиграть тон, соответствующий ноте
  for (uint8_t i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}
*/

void playTone(int tone, int duration) 
{
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(Buzz_1, HIGH);
    delayMicroseconds(tone);
    digitalWrite(Buzz_1, LOW);
    delayMicroseconds(tone);      
  }
}

time_t getNtpTime()
{
  char close_[] = {"AT+CIPCLOSE=4\r\n"};
  String cmd = "AT+CIPSTART=4,\"UDP\",\"";
  cmd += ntp;
  cmd += "\",123\r\n";
  sendData(cmd, 1000, DEBUG);
  delay(20);
  memset(buffer, 0, BUFFER_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets) 
  buffer[0] = 0b11100011; // LI, Version, Mode
  buffer[1] = 0; // Stratum, or type of clock
  buffer[2] = 6; // Polling Interval
  buffer[3] = 0xEC; // Peer Clock Precision 
 // 8 bytes of zero for Root Delay & Root Dispersion
  buffer[12] = 49;
  buffer[13] = 0x4E;
  buffer[14] = 49;
  buffer[15] = 52;
  // Serial.println("Send request");
  Serial.print(send_);
  Serial.print(4);
  Serial.print(",");
  Serial.println(NTP_PACKET_SIZE);
  delay(100);
  if (Serial.find(">"))
  {
    // Serial.println("Read >");
    for (i = 0; i < NTP_PACKET_SIZE; i++) 
    {
    Serial.write(buffer[i]);
    delay(5);
    } 
    memset(buffer, 0, NTP_PACKET_SIZE);  
    // Serial.println("Server answer : ");
    i = 0;
  
    if (Serial.find("+IPD,4,48:"))
    {
      // Serial.println("Found +IPD,48:");
      time_ = millis();  
      while( (time_+400) > millis())
      {
          while((Serial.available()) && (i < NTP_PACKET_SIZE))
          {
              byte ch = Serial.read();
              // if (ch < 0x10) Serial.print('0');
              // Serial.print(ch,HEX);
              // Serial.print(' ');
              // if ( (((i+1) % 15) == 0) ) { Serial.println(); }
              buffer[i] = ch; // read the next character.
              i++;
              delay(5);           
          }
      }
      // Serial.println();
      // Serial.print("Read bytes - ");
      // Serial.println(i);
      if (i == NTP_PACKET_SIZE) {
       /*Serial.println();
       Serial.print(buffer[40],HEX);
       Serial.print(" ");
       Serial.print(buffer[41],HEX);
       Serial.print(" ");
       Serial.print(buffer[42],HEX);
       Serial.print(" ");
       Serial.print(buffer[43],HEX);
       Serial.print(" = ");
       */
      Serial.println(close_);
      unsigned long highWord = word(buffer[40], buffer[41]);
      unsigned long lowWord = word(buffer[42], buffer[43]);
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      return  secsSince1900 - 2208988800UL + 3600*timeZone;
     }      
  } else {
      // Serial.println("Not Found +IPD,48:");
      Serial.println(close_);
      return 0;
      }
  } else {
      // Serial.println("No answer to server ");
   }
  Serial.println(close_);
  return 0; 
}


