/*Display
  Пины подключения индикаторов
  ANODES(CATHODES):
  D14 - a
  D15 - b
  D3 - c
  D4 - d
  D7 - e
  D8 - f
  D9 - g
  D10 - dp
    a11
   *******
  *      *
  f *      * b7
  10  *g5*
   *******
  *      *
  e *      * c4
  1  *d2*
******** # dp3
  Cathodes (dig 1 -> 3): 12, 9, 8
  CATHODES(ANODES):
  D13 - cathode 3
  D12 - cathode 2
  D11 - cathode 1
*/
// Encoder settings
#define CLK 2 // <- this pin to attachinterrupt
#define DT A5
#define SW 3 // <- this pin to attachinterrupt
bool DT_now, DT_last, SW_state, turn_flag;
unsigned long debounce;
#define tmpIncrement 2
#define tmpExtraIncrement 10
// Encoder settings

//Display settings
#include <SevSeg.h>
SevSeg sevseg;
#define numDigits 3                               //num of segments
byte digitPins[] = {11, 12, 13};                  //left to right
byte segmentPins[] = {14, 15, 4, 5, 7, 8, 9, 10}; //a to g + dg
bool resistorsOnSegments = false;                 // 'false' means resistors 330 Ohm are on digit pins
byte hardwareConfig = COMMON_CATHODE;
unsigned long lastTimeCheckedTemp = 0;
//Display settings
//Iron settings
#define tin 7               // Пин Датчика температуры IN Analog через LM358N
#define pinpwm 6            // порт нагревательного элемента(через транзистор)PWM
volatile int tempSet = 270; // установленная температура
volatile byte flag = false; //флаг для управления отображаемой температуры (tempReal или tempSet)
int tempMin = 200;          // минимальная температура
int tempMax = 480;          // максимальная температура
int tempReal;          // переменная датчика текущей температуры
int temppwmreal = 0;        // текущее значение PWM нагревателя
int valueToDisplay = 0;
unsigned long time = 0; //переменная для хранения времени изменения температуры
//Temperature boost
byte flagTempBoost = false;             //флаг управления температурным бустом
unsigned long timeTempBoost = 20000;    //время температурного буста
unsigned long timeTempBoostStarted = 0; //переменная для хранения времени начала буста
int tempBoostBackup = 0;                //переменная для хранения оригинальной температуры
//Temperature boost
//Iron settings
void setup()
{
  Serial.begin(9600);
  //Encoder Setup
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  pinMode(pinpwm, OUTPUT);
  // настройка прерывания. Для работы достаточно одного прерывания на CLK!
  attachInterrupt(digitalPinToInterrupt(CLK), encoderTick, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SW), encoderTick, CHANGE);
  DT_last = digitalRead(CLK); // читаем начальное положение CLK (энкодер)
  // Encoder Setup
  //Display Setup
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(70);
  sevseg.blank();
  //Display Setup
  getTempReal();
}
void loop()
{
  show();
  checkTemperature();
}
void checkTemperature()
{
  if ((millis() - lastTimeCheckedTemp) >= 2000)
  {
    //--------Boost check ---------------------
    if (flagTempBoost && ((millis() - timeTempBoostStarted) >= timeTempBoost))
    { // if bost time has expired
      tempBoostDisable(); // Turn off the boost
      showTargetTemp();
    }
    //--------Boost check ---------------------
    //--------Вычисление текущей темепературы относительно установенной -------------------------------------------
    if (tempReal < tempSet)
    { // Если температура паяльника ниже установленной температуры то:
      if ((tempSet - tempReal) < 16 & (tempSet - tempReal) > 6)
        temppwmreal = 190; // Проверяем разницу между у становленной температурой и текущей паяльника, и если разница меньше 10 градусов, то понижаем мощность нагрева, убираем инерцию перегрева (шим 0-255)
      else if ((tempSet - tempReal) < 7 & (tempSet - tempReal) > 3)
        temppwmreal = 150; // Понижаем мощность нагрева, убираем инерцию перегрева
      else if ((tempSet - tempReal) < 4)
        temppwmreal = 120; // Понижаем мощность нагрева, убираем инерцию перегрева
      else
        temppwmreal = 255; // Иначе Подымаем мощность нагрева на максимум для быстрого нагрева до нужной температуры
    }
    else
      temppwmreal = 0;                //Иначе (если температура паяльника равняется или выше установленной) Выключаем мощность нагрева, отключаем паяльник
    analogWrite(pinpwm, temppwmreal); //Вывод в шим порт (на транзистор) значение мощности

    getTempReal();

    lastTimeCheckedTemp = millis();
  }
}

void getTempReal() {
  int a = analogRead(tin);                   // считываем текущую температуру
  tempReal = a;
  Serial.println(a);
  tempReal = map(tempReal, 120, 1023, 20, 500); // нужно вычислить
  tempReal = constrain(tempReal, 20, 500);
  
}

void show()
{
  if (flag)
  { //Display the target temp for 2 sec after it's been set
    if (millis() - time <= 2000)
    {
      valueToDisplay = tempSet;
    }
    else if (!(millis() - time <= 2000))
    {
      valueToDisplay = tempReal;
      flag = false;
    }
  }
  else if (flagTempBoost && !flag)
  { //Boost is on and the target temp displaying went off - DISPLAY BOOST COUNTDOWN
    valueToDisplay = (timeTempBoost - (millis() - timeTempBoostStarted)) / 1000;
  }
  else
    valueToDisplay = tempReal; //Default real temp to be displayed

  sevseg.setNumber(valueToDisplay, -1);
  sevseg.refreshDisplay();
}
void showTargetTemp()
{ // enables the tempSet to be displayed for 2 secs
  flag = true;
  time = millis();
}
//Iron
void adjustTemp(int delta)
{
  if ((((tempSet - delta) > tempMin) && delta < 0) || (((tempSet + delta) <= tempMax) && delta > 0))
  { //будущая температура в пределах мин и макс
    tempSet += delta;
  }
  if (tempSet < tempMin)
    tempSet = tempMin;
  if (tempSet > tempMax)
    tempSet = tempMax;
  showTargetTemp();
}
void tempBoostEnable()
{
  timeTempBoostStarted = millis();
  tempBoostBackup = tempSet;
  tempSet = tempMax;
  flagTempBoost = true;
  showTargetTemp();
}
void tempBoostDisable()
{
  tempSet = tempBoostBackup; //switch to prev temp
  flagTempBoost = false;
}
//Iron
//Encoder
void encoderTick()
{
  DT_now = digitalRead(CLK);   // читаем текущее положение CLK
  SW_state = !digitalRead(SW); // читаем положение кнопки SW
  // отработка нажатия кнопки энкодера
  if (SW_state && millis() - debounce > 200)
  { //статус кнопки и защита от дребезга
    debounce = millis();
    encoderClick();
    turn_flag = 0; // чтобы не включился режим удержания при нажатии и повороте
  }
  if (!SW_state && !turn_flag && millis() - debounce > 1000 && millis() - debounce < 3000)
  { // кнопку отпустили и от нажатия прошло от 1 до 3 сек.
    encoderHold();
  }
  // вращение ручки
  if (DT_now != DT_last)
  { // если предыдущее и текущее положение CLK разные, значит был поворот
    turn_flag = 1;
    if (digitalRead(DT) != DT_now)
    { // если состояние DT отличается от CLK, значит крутим по часовой стрелке
      if (SW_state)
      { // если кнопка энкодера нажата
        adjustTemp(tmpExtraIncrement);
      }
      else
      { // если кнопка энкодера не нажата
        adjustTemp(tmpIncrement);
      }
    }
    else
    { // если совпадают, значит против часовой
      if (SW_state)
      { // если кнопка энкодера нажата
        adjustTemp(-tmpExtraIncrement);
      }
      else
      { // если кнопка энкодера не нажата
        adjustTemp(-tmpIncrement);
      }
    }
  }
  DT_last = DT_now; // обновить значение для энкодера
}
void encoderClick()
{
  Serial.println("A stub: no need in click function at the time");
}
void encoderHold()
{
  tempBoostEnable();
}
//Encoder

