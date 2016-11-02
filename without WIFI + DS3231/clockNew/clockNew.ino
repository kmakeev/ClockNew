/*
  Программа для часов на газоразрядных лампах RTC - DS3231
 */

#include "wiring_private.h"
#include "pins_arduino.h"

// #include <SoftwareSerial.h>
#include <DS1307RTC.h>
#include "demo.h"
#include <avr/wdt.h>
#include <avr/io.h>
#include <util/delay.h>


// Объявляем переменные и константы
#define NUMITEMS(arg) ((size_t) (sizeof (arg) / sizeof (arg [0])))
#define DELTA_SHIM_FOR_ANIMATION 4
#define MAX_SHIM_FOR_ANIMATION 150
#define DELAY_ANIMATION 7         // чем больше, тем медленее перебираются цифры
#define DELAY_SHOW 1


const int COM_BAUD = 9600;


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

float tempC;
bool sensorTemperatureIn = false;
bool isReadTemperature = false;
                   
boolean isAlarm = false;
bool mode_auto = true;

uint8_t a;
float b;
uint8_t i=0, j=0, z=0, y=0;

boolean up = false;         //Признак нажатия любойй из кнопок
boolean animate = false;
// boolean sec = true;      //не используется

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

//Блок переменных для воспроизведения мелодии
int freq[7][12] = {
    {65, 69, 73, 78, 82, 87, 92, 98, 104, 110, 117, 123},                     //0 = Большая октава
    {131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247},             //1 = Малая октава
    {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494},             //2 = 1-я октава
    {523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988},             //3 = 2-я октава
    {1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976}, //4 = 3-я октава
    {2093, 2218, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951}, //5 = 4-я октава
    {4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902}, //6 = 5-я октава
};


//Функции скетча

int changeButtonStatus(int buttonPin);
void setNixieNum(uint8_t tube, uint8_t num);
void DisplayNumberSet(uint8_t anod, uint8_t num1, uint8_t num2 );
void DisplayNumberSetA(uint8_t anod, uint8_t num1, uint8_t num2 );
void DisplayNumberString( uint8_t* array );
int pointsCount(char Muz[], int& curPosition);
int pointsCount(char Muz[], int& curPosition);
void Qb_PLAY(char Muz[]);
void playMusic();

tmElements_t tm;

  
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
   
     wdt_enable(WDTO_8S);        //Установка тамера для перезагрузки при подвисании программы
  
}


void loop() // выполняется циклически
{
    wdt_reset();                                //Циклический сброс таймера 
    
    //Чтение состояние кнопок 
   uint8_t btn1 = digitalRead(Pin_rt1);
   uint8_t btn2 = changeButtonStatus(Pin_rt2); 

  
     
    //Счетчик для анимации
    if (z==DELAY_ANIMATION)
    {
        j++;
        z=0;
    }
    if (j==10) {animate=false; j=0; z=0;}
    if (animate) z++;
    
   RTC.read(tm);
   Mins = tm.Minute;
   Seconds_old = Seconds;
   Seconds = tm.Second;
   hours = tm.Hour;
   
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

      if((tm.Hour>=8)&&(tm.Hour<20)) dayNight=255;
      if((tm.Hour>=20)&&(tm.Hour<22)) dayNight=40;
      if((tm.Hour>=22)&&(tm.Hour<0)) dayNight=10;
      if((tm.Hour>=0)&&(tm.Hour<8)) dayNight=0;  //1
    
    analogWrite(Led_1, dayNight);  

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
                tempC = RTC.gettemperature();         
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
        if(a < 130){                                                    //тут должно быть количество шагов анимации
            //   Serial.println("Animation step ");
            //   Serial.println("a");
            for (i=0; i<6; i++) {
                NumberArray[i] = NumberAnimationArray[a][i];         //устанавливаем значения колб для анимации
            }

            millisThis = millis();                                 //время сейчас
            // unsigned int mills = NumberAnimationDelay[a];
            if(millisThis - millisAnimation > NumberAnimationArray[a][6]*50) {  //Если время на анимацию одного шага вышло, переходим к другому
                a++;
                millisAnimation = millisThis;
            }

        } else {                                                      //Если количество шагов исчерпано, выходим в режим 1
            mode = 0;
            playMusic();                                                //Включаем музыку
        }
        break;
     
    }

    if  (timeset==0&&alarmclockset==0&&mode<4){      //Мы не в режиме установки времени, часов и не в режиме анимации, таймера, выключения.
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
            tone(Buzz_1,100, 100);
            mode++;
            mode %= 5;          //перебор всех режимов отображения 
            if (mode==4) {      //Если перешли к демо режиму
                animate=false;    //обычную анимацию отключаем
                a=0;              //переход к первому шагу анимации
                millisAnimation = millis();                  //фиксируем время начала анимации
            }
            if (mode==3) isReadTemperature = false;   //Для чтения температуры при ручной смене режима отображения
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


void DisplayNumberString( uint8_t* array ) {    //Функция для отображения строки цифр из массива array

    if (mode == 0) {              //Если показ времени то с анимацией
        DisplayNumberSetA(0,0,3);   //Выводим на 1 анод (лампы 1,4) цифры 1,4 из массива
        DisplayNumberSetA(1,1,4);   //Выводим на 2 анод (лампы 2,5) цифры 2,5 из массива
        DisplayNumberSetA(2,2,5);   //Выводим на 3 анод (лампы 3,6) цифры 3,6 из массива
    } else {                      //В других режимах нет
        DisplayNumberSet(0,array[0],array[3]);   //Выводим на 1 анод (лампы 1,4) цифры 1,4 из массива
        DisplayNumberSet(1,array[1],array[4]);   //Выводим на 2 анод (лампы 2,5) цифры 2,5 из массива
        DisplayNumberSet(2,array[2],array[5]);   //Выводим на 3 анод (лампы 3,6) цифры 3,6 из массива
    }
}

int extractNumber(int& myNumber, char Muz[], int& curPosition)
{
    int digitsNumber=0;
    int curDigit=0;
    myNumber=0;
    do
    {
        if ((Muz[curPosition]> 47) && (Muz[curPosition]<58)) // Коды ASCII цифр '0' == 48 , "9' == 57
        {
            curDigit=Muz[curPosition]-48;
            digitsNumber++;
            myNumber=myNumber*10+curDigit;
        }
        else
        {
            return digitsNumber;
        }
        curPosition++;
    }while(Muz[curPosition]!= '\0');
    return digitsNumber;
}

int pointsCount(char Muz[], int& curPosition)
{
    int pointsNumber=0;
    do
    {
        if (Muz[curPosition]== '.')
        {
            pointsNumber++;
        }
        else
        {
            return pointsNumber;
        }
        curPosition++;
    }while(Muz[curPosition]!= '\0');
    return pointsNumber;
}

void Qb_PLAY(char Muz[])
{
    static int generalOktava;
    int oktava;
    static int tempo=120; // Задание темпа или четвертных нот, которые исполняются в минуту. n от 32 до 255. По умолчанию 120
    int Nota=0;
    int  curPosition, curNota4;
    unsigned long currentNotaPauseDuration;
    unsigned long currentNotaDuration;
    unsigned long  pauseDuration;
    int takt=240000/tempo;
    bool isNota;
    bool isPause;
    int pointsNum=0;
    float generalNotaMultipl=0.875;
    static float NotaLong;
    float curMultipl;
    float tempFlo;
    float curPause;
    unsigned long tempLong;
    int i=0;
    do
    {
        isNota=false;
        isPause=false;
        oktava=generalOktava;
        switch(Muz[i]){
        case '\0':{
            return;
        }
            break;
        case 'C':{
            Nota=0;
            isNota=true;
        }
            break;
        case 'D':{
            Nota=2;
            isNota=true;
        }
            break;
        case 'E':{
            Nota=4;
            isNota=true;
        }
            break;
        case 'F':{
            Nota=5;
            isNota=true;
        }
            break;
        case 'G':{
            Nota=7;
            isNota=true;
        }
            break;
        case 'A':{
            Nota=9;
            isNota=true;
        }
            break;
        case 'B':{
            Nota=11;
            isNota=true;
        }
            break;
        case 'N':{// Nнота  Играет определенную ноту (0 - 84) в диапазоне семи октав (0 - пауза).
            curPosition=i+1;
            if (extractNumber(curNota4, Muz, curPosition)){
                i=curPosition-1;
                if (curNota4){
                    curNota4--;
                    oktava=curNota4 / 12;
                    Nota=curNota4 % 12;
                    isNota=true;
                }
                else{
                    isPause=true;
                }
            }
        }
            break;
        case 'O':{ //Oоктава Задает текущую октаву (0 - 6).
            curPosition=i+1;
            if (extractNumber(oktava, Muz, curPosition)){
                i=curPosition-1;
                generalOktava=oktava;
            }
        }
            break;
        case '>':{
            generalOktava++;
        }
            break;
        case '<':{
            generalOktava--;
        }
            break;
        case 'M':{
            switch(Muz[i+1]){
            case 'N':{ //MN  Нормаль. Каждая нота звучит 7/8 времени, заданного в команде L
                generalNotaMultipl=0.875; //  =7/8
                i++;
            }
                break;
            case 'L':{ //ML  Легато. Каждая нота звучит полный интервал времени, заданного в команде L
                generalNotaMultipl=1.0;
                i++;
            }
                break;
            case 'S':{ //MS  Стаккато. Каждая нота звучит 3/4 времени, заданного в команде L
                generalNotaMultipl=0.75;  // =3/4
                i++;
            }
                break;
            case 'F':{ //MF Режим непосредственного исполнения. Т.е. на время проигрывания ноты программа приостанавливается. Используется по умолчанию
                i++;   //Сдвигаем точку чтения и ничего не делаем.
            }
                break;

            case 'B':{ //MB проигрывние в буффер
                i++;   //Сдвигаем точку чтения и ничего не делаем.
            }
                break;
            }
        }
            break;
        case 'L':{ //Lразмер Задает длительность каждой ноты (1 - 64). L1 - целая нота, L2 - 1/2 ноты и т.д.
            curPosition=i+1;
            if (extractNumber(curNota4, Muz, curPosition)){
                i=curPosition-1;
                tempFlo=float(curNota4);
                NotaLong=1/tempFlo;
            }
        }
            break;
        case 'T':{ //Tтемп Задает темп исполнения в четвертях в минуту (32-255).По умолчанию 120
            curPosition=i+1;
            if (extractNumber(tempo, Muz, curPosition)){
                i=curPosition-1;
                takt=240000/tempo; // миллисекунд на 1 целую ноту. 240000= 60 сек * 1000 мсек/сек *4 четвертей в ноте
            }
        }
            break;
        case 'P':{ //Pпауза  Задает паузу (1 - 64). P1 - пауза в целую ноту, P2 - пауза в 1/2 ноты и т.д.
            curPosition=i+1;
            if (extractNumber(curNota4, Muz, curPosition)){
                tempFlo=float(curNota4);
                curPause=1/tempFlo;
                i=curPosition-1;
                isPause=true;
            }
        }
            break;
        case ' ':{ //Есть в некоторых текстах. Вероятно это пауза длительностью в текущую ноту
            curPause= NotaLong;
            isPause=true;
        }
            break;
        }
        if (isNota){
            switch(Muz[i+1]){
            case '#':{ // диез
                Nota++;
                i++;
            }
                break;
            case '+':{ // диез
                Nota++;
                i++;
            }
                break;
            case '-':{ // бемоль
                Nota--;
                i++;
            }
                break;
            }
            curPosition=i+1;
            if (extractNumber(curNota4, Muz, curPosition)){
                currentNotaDuration=takt/curNota4;
                i=curPosition-1;
            }
        }
        if (oktava<0) oktava=0;
        if (oktava>6) oktava=6;
        if (isNota || isPause){
            curPosition=i+1;
            pointsNum=pointsCount(Muz, curPosition);
            if (pointsNum) i=curPosition-1;
            curMultipl=1.0;
            for (int j=1; j<=pointsNum; j++) {
                curMultipl= curMultipl * 1.5;
            }
            currentNotaPauseDuration=(takt*NotaLong);
        }
        if (isNota){
            curMultipl=curMultipl*generalNotaMultipl;
            currentNotaDuration= (currentNotaPauseDuration*curMultipl);
            if (Nota<0) Nota=0;
            if (Nota>11) Nota=11;
            tempLong= freq[oktava][Nota];
            tone(Buzz_1,tempLong,currentNotaDuration);
            // DisplayNumberString(NumberArray);    //Будем при игре каждой ноты еще показывать последнее содержимое массива колб
            delay(currentNotaPauseDuration);
        }
        if (isPause){
            pauseDuration=takt*curPause*curMultipl;
            delay(pauseDuration);
        }
        i++;
    } while (Muz[i]!= '\0');
}

void playMusic()
{
    // Serial.println("Play Alarm music");
    analogWrite(Led_1, 0);                //Для нормального воспроизведения нужно выключить ШИМ выводы
    analogWrite(anods[0], 0);
    analogWrite(anods[1], 0);
    analogWrite(anods[2], 0);
    Qb_PLAY ("MST255L2O2E.L4F+L2G.L4EGGF+EL2F+L4<BP4L2>F+.L4GL2A.L4F+");
    Qb_PLAY ("AAGF+L1EL2B>EDL4EDCC<BAL2BEP4>CL4<AL2B.L4GF+<B>GF+L1E");
    Qb_PLAY ("L2B>EDL4EDCC<BAL2BEP4>CL4<AL2B.L4GF+<B>GF+L1E");

}
