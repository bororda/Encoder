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
const byte DT = A5;
#define SW 3 // <- this pin to attachinterrupt
bool DT_now, DT_last, SW_state, turn_flag;
unsigned long debounce;
#define tmpIncrement 5
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
volatile int tempSet = 250; // установленная температура
int tempPrecalc;
volatile byte flag = false; //флаг для управления отображаемой температуры (tempReal или tempSet)
int tempMin = 200;          // минимальная температура
int tempMax = 450;          // максимальная температура
int tempReal;          // переменная датчика текущей температуры
int tempReal_f;
int temppwmreal = 0;        // текущее значение PWM нагревателя
int valueToDisplay = 0;
unsigned long time = 0; //переменная для хранения времени изменения температуры

//PID
float kp = 5;
float ki = 1.6;
float kd = 0.2;

float P = 0;
float prevErr = 0;
float I = 0;
float D = 0;
//PID

//Iron settings
void setup() {
  //Encoder Setup
  pinMode(CLK, INPUT); //поворот енкодера
  pinMode(DT, INPUT); //кнопка на енкодере
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
}

void loop()
{
  show();
  checkTemperature();
}

void checkTemperature() {
  if ((millis() - lastTimeCheckedTemp) >= 100) {
    P = tempSet - tempReal_f;
    I += P * 0.1;
    D = (P - prevErr) / 0.1;
    prevErr = P;
    temppwmreal = P * kp + constrain(I * ki, 0, 1023) + D * kd;
    temppwmreal = constrain(temppwmreal, 0, 1023);

    analogWrite(pinpwm, temppwmreal); //Вывод в шим порт (на транзистор) значение мощности

    int a = analogRead(tin);                   // считываем текущую температуру
    tempReal = map(a, 0, 1023, 35, 450);
    tempReal_f = tempReal * 0.2 + tempReal_f * 0.8;
    tempReal_f = constrain(tempReal_f, 20, 450);
    lastTimeCheckedTemp = millis();
  }
}

void show() {
  if (flag)
  { //Display the target temp for 2 sec after it's been set
    if (millis() - time <= 2000)
    {
      valueToDisplay = tempSet;
    }
    else if ((millis() - time > 2000))
    {
      valueToDisplay = tempReal_f;
      flag = false;
    }
  }
  else
    valueToDisplay = tempReal_f; //Default real temp to be displayed

  sevseg.setNumber(valueToDisplay, -1);
  sevseg.refreshDisplay();
}

void adjustTemp(int delta) {
  tempPrecalc = tempSet + delta;
  if (((tempPrecalc) >= tempMin) && ((tempPrecalc) <= tempMax))
  { //будущая температура в пределах мин и макс
    tempSet = tempPrecalc;
  }
  flag = true;
  time = millis();
}

void encoderTick() {
  DT_now = digitalRead(CLK);   // читаем текущее положение CLK
  // вращение ручки
  if (DT_now != DT_last && millis() - debounce > 50) {
    // если предыдущее и текущее положение CLK разные, значит был поворот
    debounce = millis();
    if (digitalRead(DT) != DT_now) {
      adjustTemp(tmpIncrement);
    }
    else { // если совпадают, значит против часовой
      adjustTemp(-tmpIncrement);
    }
  }
  DT_last = DT_now; // обновить значение для энкодера
}/*Display
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
const byte DT = A5;
#define SW 3 // <- this pin to attachinterrupt
bool DT_now, DT_last, SW_state, turn_flag;
unsigned long debounce;
#define tmpIncrement 5
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
volatile int tempSet = 250; // установленная температура
int tempPrecalc;
volatile byte flag = false; //флаг для управления отображаемой температуры (tempReal или tempSet)
int tempMin = 200;          // минимальная температура
int tempMax = 450;          // максимальная температура
int tempReal;          // переменная датчика текущей температуры
int tempReal_f;
int temppwmreal = 0;        // текущее значение PWM нагревателя
int valueToDisplay = 0;
unsigned long time = 0; //переменная для хранения времени изменения температуры

//PID
float kp = 5;
float ki = 1.6;
float kd = 0.2;

float P = 0;
float prevErr = 0;
float I = 0;
float D = 0;
//PID

//Iron settings
void setup() {
  //Encoder Setup
  pinMode(CLK, INPUT); //поворот енкодера
  pinMode(DT, INPUT); //кнопка на енкодере
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
}

void loop()
{
  show();
  checkTemperature();
}

void checkTemperature() {
  if ((millis() - lastTimeCheckedTemp) >= 100) {
    P = tempSet - tempReal_f;
    I += P * 0.1;
    D = (P - prevErr) / 0.1;
    prevErr = P;
    temppwmreal = P * kp + constrain(I * ki, 0, 1023) + D * kd;
    temppwmreal = constrain(temppwmreal, 0, 1023);

    analogWrite(pinpwm, temppwmreal); //Вывод в шим порт (на транзистор) значение мощности

    int a = analogRead(tin);                   // считываем текущую температуру
    tempReal = map(a, 0, 1023, 35, 450);
    tempReal_f = tempReal * 0.2 + tempReal_f * 0.8;
    tempReal_f = constrain(tempReal_f, 20, 450);
    lastTimeCheckedTemp = millis();
  }
}

void show() {
  if (flag)
  { //Display the target temp for 2 sec after it's been set
    if (millis() - time <= 2000)
    {
      valueToDisplay = tempSet;
    }
    else if ((millis() - time > 2000))
    {
      valueToDisplay = tempReal_f;
      flag = false;
    }
  }
  else
    valueToDisplay = tempReal_f; //Default real temp to be displayed

  sevseg.setNumber(valueToDisplay, -1);
  sevseg.refreshDisplay();
}

void adjustTemp(int delta) {
  tempPrecalc = tempSet + delta;
  if (((tempPrecalc) >= tempMin) && ((tempPrecalc) <= tempMax))
  { //будущая температура в пределах мин и макс
    tempSet = tempPrecalc;
  }
  flag = true;
  time = millis();
}

void encoderTick() {
  DT_now = digitalRead(CLK);   // читаем текущее положение CLK
  // вращение ручки
  if (DT_now != DT_last && millis() - debounce > 50) {
    // если предыдущее и текущее положение CLK разные, значит был поворот
    debounce = millis();
    if (digitalRead(DT) != DT_now) {
      adjustTemp(tmpIncrement);
    }
    else { // если совпадают, значит против часовой
      adjustTemp(-tmpIncrement);
    }
  }
  DT_last = DT_now; // обновить значение для энкодера
}
