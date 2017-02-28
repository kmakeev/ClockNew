
/*
  Программа для часов на газоразрядных лампах без WiFi
  версия для 4 ламп ИН-14
 */
#include "wiring_private.h"
#include "pins_arduino.h"

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
#define BUFFER_SIZE 160
#define NUMITEMS(arg) ((size_t) (sizeof (arg) / sizeof (arg [0])))
#define ONE_WIRE_BUS A3                //Это вывод для подключения вывода DS, при распайке на плате RTS1703 датчика температуры                  
#define TEMPERATURE_PRECISION 9
#define DELTA_SHIM_FOR_ANIMATION 4
#define MAX_SHIM_FOR_ANIMATION 150
#define DELAY_ANIMATION 7         // чем больше, тем медленее перебираются цифры
#define DELAY_SHOW 2
#define OK 0
#define ERR 1
#define DATA 2 


//Команды для подачи HIGHT / LOW сигнала на выводы регистров в соотвевствии с используемыми выводами arduino 
const uint8_t Pin_1_a = 13;               //было                
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

static const uint8_t bits[1][4] = 
{ 
    {Pin_1_d, Pin_1_c, Pin_1_b, Pin_1_a},// К155ИД1 (1)
};

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
// const uint8_t Pin_rt1 = A0;   //Пока будем использовать аналоговые как цифровые
// const uint8_t Pin_rt2 = A1;  

//Пин для подсветки
const uint8_t Led_1 = 3; 

//Настройка команд для бипера
const int Buzz_1 = A2;
#define Pin_Buzz_ON PORTC |= (1<<2);
#define Pin_Buzz_OFF PORTC &= ~(1<<2);

//Настройка команд для точек
// const uint8_t Pin_dot1 = 5;
#define Pin_dot1_OFF PORTD &= ~(1<<5);
#define Pin_dot1_ON PORTD |= (1<<5);
// const uint8_t Pin_dot1 = 7;
#define Pin_dot2_OFF PORTD &= ~(1<<7);
#define Pin_dot2_ON PORTD |= (1<<7);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
// DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
// DeviceAddress insideThermometer;

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
    6, 7, 9, 4, 8, 5, 9, 2, 0, 3
};

//Массив данных для 4 колб
uint8_t NumberArray[4]={0,0,0,0};
bool isChangeArray[4]={false, false, false, false};
uint8_t ShimAnimationArray[4]={255,255,255,255};
uint8_t NumberArrayOLD[4]={0,0,0,0};


//Массив для управления анодами ламп
static const uint8_t anods[4] = {Pin_a_4, Pin_a_3, Pin_a_2, Pin_a_1};

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
bool sensorTemperatureIn = true;
bool isReadTemperature = false;
boolean isAlarm = false;
bool mode_auto = true;
// boolean play = false;

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


int changeButtonStatus();
void setNixieNum(uint8_t num);
void DisplayNumberSet(uint8_t anod, uint8_t num);
void DisplayNumberSetA(uint8_t anod, uint8_t num);
void DisplayNumberString( uint8_t* array );
void Qb_PLAY(char Muz[]);
int extractNumber(int& myNumber, char Muz[], int& curPosition);
int pointsCount(char Muz[], int& curPosition);
void playMusic();


tmElements_t tm;

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


void DisplayNumberString( uint8_t* array ) {    //Функция для отображения строки цифр из массива array
   // Serial.println("In DisplayNumberString");
   // Serial.println(array[3]);
      if (mode == 0) {                //Если показ времени то с анимацией
        DisplayNumberSetA(0,0);   //Выводим на 1 анод 1 цифрy из массива
        DisplayNumberSetA(1,1);   //Выводим на 2 анод 2 цифрy из массива
        DisplayNumberSetA(2,2);   //Выводим на 3 анод 3 цифрy из массива
        DisplayNumberSetA(3,3);   //Выводим на 4 анод 4 цифрy из массива
    } else {                      //В других режимах нет
        DisplayNumberSet(0,array[0]);   //Выводим на 1 анод 1 цифрy из массива
        DisplayNumberSet(1,array[1]);   //Выводим на 1 анод 1 цифрy из массива
        DisplayNumberSet(2,array[2]);   //Выводим на 1 анод 1 цифрy из массива
        DisplayNumberSet(3,array[3]);   //Выводим на 1 анод 1 цифрy из массива
    }
}

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

void DisplayNumberSetA(uint8_t anod, uint8_t num) {

    setNixieNum(NumberArray[num]);     //Выводим на шифратор Num из массива
    if (isChangeArray[num]==true) { //Если нужно анимировать
        ShimAnimationArray[num] = 0;
        isChangeArray[num] = false;
    }
    analogWrite(anods[anod], ShimAnimationArray[num]);   // Подаем ШИМ сигнал из массива значений для 
    _delay_ms(DELAY_SHOW);
    analogWrite(anods[anod], 0);    //Убираем ШИМ мигнал

    for (i=0; i<4; i++){
        if (ShimAnimationArray[i]<MAX_SHIM_FOR_ANIMATION) ShimAnimationArray[i]=ShimAnimationArray[i] + DELTA_SHIM_FOR_ANIMATION;    //Если ведется анимация, то увеличиваем значение ШИМ сигнала
        else ShimAnimationArray[i]=255;
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

void playMusic()
{
    // Serial.println("Play Alarm music");
    analogWrite(Led_1, 0);                //Для нормального воспроизведения нужно выключить ШИМ выводы
    analogWrite(anods[0], 0);
    analogWrite(anods[1], 0);
    analogWrite(anods[2], 0);
    Qb_PLAY ("MNT150L2O3CGP16L16FEDL2>C<GP16L16FEDL2>C<GP16L16FEFL2D");
    Qb_PLAY ("P16L16<GGGL2>CGP32L16FEDL2>C<GP16L16FEDL2>C<GP16L16A+");
    Qb_PLAY ("AA+L1GL2G.L8<G.L16GL4A.L8A>FEDCL16CDEDP16L8<AL4BL8G.L16G");
    Qb_PLAY ("L4A.L8A>FEDCGP8L4D.P8L8<G.L16GL4A.L8A>FEDCL16CDEDP16L8<A");
    Qb_PLAY ("L4BP16L8>G.L16GL8>C.L16<A+L8G+.L16GL8F.L16D+L8D.L16CL1G");
    Qb_PLAY ("L2G.P16L16GGGL8>CP8L16<CCCL2C.");
}

void setup() {
  
  DDRB  |= B00111110;       //Установка выходных регистров
  DDRD  |= B11111100;       //Установка выходных регистров
  DDRC  &= B11111100;       //Установка регистров для чтения (A0 и A1)
  DDRC  |= B00000100;       //Установка выходного регистра А2 для проигрывания мелодии
  

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

   // sensors.begin();
   wdt_disable(); 
   // if (sensors.getAddress(insideThermometer, 0)) {
   //     sensorTemperatureIn = true;
   //     sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
   //}
   wdt_enable(WDTO_8S);        //Установка тамера для перезагрузки при подвисании программы
}

void loop(){
   
     wdt_reset();                                //Циклический сброс таймера 
     
      // Чтение кнопок 
    btn1 = Pin_rt1_Read;
    btn2 = changeButtonStatus(); 
    RTC.read(tm);
    Mins = tm.Minute;
    Seconds_old = Seconds;
    Seconds = tm.Second;
    hours = tm.Hour;
       
      //Счетчик для анимации
    if (z==DELAY_ANIMATION)
     {
          j++;
          z=0;
     }
    if (j==10) {animate=false; j=0; z=0;}
    if (animate) z++;

    if (isAlarm) {                 //если установлен будильник горят точки
     //     Pin_dot1_ON
     //     Pin_dot2_ON 
     //   digitalWrite(Pin_dot1, HIGH);
     //   digitalWrite(Pin_dot2, HIGH);
        if (alarmHour==tm.Hour&&alarmMin==Mins&&alarmclockset==0) {
            isAlarm = false;
            playMusic();
        }

    } else {
    //      Pin_dot1_OFF
    //      Pin_dot2_OFF  
       // digitalWrite(Pin_dot1, LOW);
       // digitalWrite(Pin_dot2, LOW);
    }
    if (dayNight!=0){                              // Единицу присылаем из проложения - это равносильно выключению, 
                                                    // В остальных случаях подсветку устанавливаем по времени 
      if((tm.Hour>=8)&&(tm.Hour<20)) dayNight=255;
      if((tm.Hour>=20)&&(tm.Hour<22)) dayNight=40;
      if((tm.Hour>=22)&&(tm.Hour<0)) dayNight=10;
      if((tm.Hour>=0)&&(tm.Hour<8)) dayNight=1;  //1
    }
    analogWrite(Led_1, dayNight);  
   
  //Мигание точками
          
    if ((Seconds % 10)%2==0)           ////Если знак секунды четный то включаем иначе выкл
       {
          Pin_dot1_ON
          Pin_dot2_ON
       }
        else
        {
           Pin_dot1_OFF
           Pin_dot2_OFF
        }
    switch(mode)
    {
      
               
    case 0:
    
        NumberArray[0] = tm.Hour / 10; //Первый знак часа
        NumberArray[1] = tm.Hour % 10; //Второй знак часа
        NumberArray[2] = Mins / 10; //Первый знак минут
        NumberArray[3] = Mins % 10; //Первый знак минут
        break;
    case 1:
        NumberArray[0] = tm.Day / 10; //Первый знак дня
        NumberArray[1] = tm.Day % 10; //Второй знак дня
        NumberArray[2] = tm.Month / 10; //Первый знак месяца
        NumberArray[3] = tm.Month % 10; //Второй знак месяца
        break;

    case 2:                               //Режим отображения установок будильника
        NumberArray[0] = alarmHour / 10; //Первый знак часа
        NumberArray[1] = alarmHour % 10; //Второй знак часа
        NumberArray[2] = alarmMin / 10; //Первый знак минут
        NumberArray[3] = alarmMin % 10; //Второй знак минут


        //digitalWrite(Pin_dot1, HIGH);
        //digitalWrite(Pin_dot2, HIGH);
        if (isAlarm) {
            Pin_dot1_ON
            Pin_dot2_ON
        } 
       break;

    case 3:                                //отображение температуры
        if(sensorTemperatureIn)
        {        
            if (!isReadTemperature)
            {
                // sensors.requestTemperatures();
                // tempC = sensors.getTempC(insideThermometer);            // Поправка введена в связи с неточностью работы датчика
                tempC = RTC.gettemperature()+7;   // на Ds3231 Поправка  
                isReadTemperature = true;
                b = (tempC - int(tempC))*100;
                //Serial.println((int)b/10);
                //Serial.println((int)b%10);
                NumberArray[0] = (int)tempC/10; //Первый
                NumberArray[1] = (int) tempC%10; //Второй
                NumberArray[2] = (int)b/10; //Первый после запятой
                NumberArray[3] = 10;        //пусто

                //NumberArray[5] =(int)b%10; //Второй знак после запятой

            }
            
            millisThis = millis();
            if(millisThis - millisAnimation > 700) {  //Если пауза вышла двигаем колбы влево
                
                a = NumberArray[0];
                NumberArray[0] = NumberArray[1];       
                NumberArray[1] = NumberArray[2];       
                NumberArray[2] = NumberArray[3]; 
                NumberArray[3] = a;
                millisAnimation = millisThis;
            }
        }
        else mode = 0;
        break;

    case 4:                      //режим анимации
        //  if(a < NUMITEMS(NumberAnimationDelay)){                   //не первышаем количество шагов анимации
        if(a < 130){                                                    //тут должно быть количество шагов анимации
            //   Serial.println("Animation step ");
            //   Serial.println("a");
            for (i=0; i<4; i++) {
                NumberArray[i] = NumberAnimationArray[a][i];         //устанавливаем значения колб для анимации
            }

            millisThis = millis();                                 //время сейчас
            // unsigned int mills = NumberAnimationDelay[a];
            if(millisThis - millisAnimation > NumberAnimationArray[a][4]) {  //Если время на анимацию одного шага вышло, переходим к другому
                a++;
                millisAnimation = millisThis;
            }

        } else {                                                      //Если количество шагов исчерпано, выходим в режим 1
            mode = 0;
            playMusic();                                                //Включаем музыку
        }
        break;
     case 5:            //Режим отображения таймера        
        
        // NumberArray[0] = time_mm / 10; //Первый знак минут
        // NumberArray[1] = time_mm % 10; //Второй знак минут
        // NumberArray[2] = time_ss / 10; //Первый знак минут
        // NumberArray[3] = time_ss % 10; //Второй знак минут

        break;   
     case 6:            //Режим выключения ламп, тупо подаем 10 на дешифраторы и ничего на них не отображаем
        
        NumberArray[0] = 10; 
        NumberArray[1] = 10; 
        NumberArray[2] = 10; 
        NumberArray[3] = 10;    //   
        Pin_dot1_OFF
        Pin_dot2_OFF

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
            mode %= 7;          //перебор всех режимов отображения 
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

    case 1:
      //  digitalWrite(Pin_dot1, HIGH);
      //  digitalWrite(Pin_dot2, HIGH);
     // Serial.print("In time set");
        mode=0;
        NumberArray[2] = 10;
        NumberArray[3] = 10;

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
      break;
      /*
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
        */
    case 4:
        mode=1;
      //  digitalWrite(Pin_dot1, HIGH);
     //   digitalWrite(Pin_dot2, HIGH);
        NumberArray[2] = 10;
        NumberArray[3] = 10;

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
   
    /*
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
        */
        break;
    }
    for (i=0; i<4; i++){
        if  (NumberArray[i]!=NumberArrayOLD[i]) isChangeArray[i] = true;       //Произошло изменение значения для отображения, нужно его анимировать
        NumberArrayOLD[i] = NumberArray[i];                                   //Сохраняем текущее значение на следующий цикл как старое
    }
    
    //Отображение на индикаторы
    DisplayNumberString(NumberArray);
  //  printConsoleTime();
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

int changeButtonStatus() {
    // Событие
    int event = 0;

    // Текущее состояние кнопки
    int currentButtonClick = Pin_rt2_Read;
  //  Serial.println(currentButtonClick);

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


