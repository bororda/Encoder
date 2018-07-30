/*Display
  Пины подключения индикаторов
  ANODES(CATHODES):
  D0 - a
  D1 - b
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
#define SW A4
boolean DT_now, DT_last, SW_state, hold_flag, butt_flag, turn_flag;
unsigned long debounce_timer;
#define tmpIncrement 2
#define tmpExtraIncrement 10
// Encoder settings

//Display settings
#include <SevSeg.h>
SevSeg sevseg;
#define numDigits 3 //num of segments
byte digitPins[] = {11, 12, 13};  //left to right
byte segmentPins[] = {14, 15, 3, 4, 7, 8, 9, 10}; //a to g + dg
bool resistorsOnSegments = false; // 'false' means resistors 330 Ohm are on digit pins
byte hardwareConfig = COMMON_CATHODE;
unsigned long timeToCheckTemp = 0;
//Display settings
//Iron settings
#define tin 0 // Пин Датчика температуры IN Analog через LM358N
#define pinpwm 5// порт нагревательного элемента(через транзистор)PWM
volatile int tempSet = 270; // установленная температура
volatile byte flag = false; //флаг для управления отображаемой температуры (tempReal или tempSet)
int tempMin = 200; // минимальная температура
int tempMax = 480; // максимальная температура
int tempReal = 20; // переменная датчика текущей температуры
int temppwmreal = 0; // текущее значение PWM нагревателя
int tempToDisplay = 0;
unsigned long time = 0;//переменная для хранения времени изменения температуры
  //Temperature boost
byte flagTempBoost = false; //флаг управления температурным бустом
unsigned long timeTempBoost = 15000; //время температурного буста
unsigned long timeTempBoostStarted = 0; //переменная для хранения времени начала буста
int tempBoostBackup = 0; //переменная для хранения оригинальной температуры
  //Temperature boost
//Iron settings
void setup() {
  Serial.begin (9600);
  //Encoder Setup
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  // настройка прерывания. Для работы достаточно одного прерывания на CLK!
  attachInterrupt(digitalPinToInterrupt(CLK), encoderTick, CHANGE);
  DT_last = digitalRead(CLK); // читаем начальное положение CLK (энкодер)
  // Encoder Setup
  //Display Setup
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(70);
  sevseg.blank();
  //Display Setup
}
void loop() {
  show();
  checkTemperature();
}
void checkTemperature() {
  if (!(millis() - timeToCheckTemp <= 2000)) {
    //--------Boost check ---------------------
	if(flagTempBoost && ((millis() - timeTempBoostStarted) >= timeTempBoost)) {
	  tempSet = tempBoostBackup;
	  flagTempBoost = false;
	}
	//--------Boost check ---------------------
    //--------Вычисление текущей темепературы относительно установенной -------------------------------------------
    if (tempReal < tempSet ) { // Если температура паяльника ниже установленной температуры то:
      if ((tempSet - tempReal) < 16 & (tempSet - tempReal) > 6 ) temppwmreal = 150; // Проверяем разницу между у становленной температурой и текущей паяльника, и если разница меньше 10 градусов, то понижаем мощность нагрева, убираем инерцию перегрева (шим 0-255)
      else if ((tempSet - tempReal) < 7 & (tempSet - tempReal) > 3) temppwmreal = 120;// Понижаем мощность нагрева, убираем инерцию перегрева
      else if ((tempSet - tempReal) < 4 ) temppwmreal = 90;// Понижаем мощность нагрева, убираем инерцию перегрева
      else temppwmreal = 230; // Иначе Подымаем мощность нагрева на максимум для быстрого нагрева до нужной температуры
    }
    else temppwmreal = 0;//Иначе (если температура паяльника равняется или выше установленной) Выключаем мощность нагрева, отключаем паяльник
    analogWrite(pinpwm, temppwmreal); //Вывод в шим порт (на транзистор) значение мощности

    tempReal = analogRead(tin);// считываем текущую температуру
    tempReal = map(tempReal, 750, 1023, 20, 500); // нужно вычислить
    tempReal = constrain(tempReal, 20, 500);
  }
}
void show() {
  if (flag) {
    flag = false;
    time = millis();
  };
  if (millis() - time <= 2000) {
    tempToDisplay = tempSet;
  }
  else tempToDisplay = tempReal;
  timeToCheckTemp = millis();
  
  sevseg.setNumber(tempToDisplay, -1);
  sevseg.refreshDisplay();
}
//Iron
void adjustTemp(int delta){
  if (((tempSet - delta) >= tempMin) && ((tempSet + delta) <= tempMax)) { //будущая температура в пределах мин и макс
    tempSet += delta;
    flag = true;
  }
}
void tempBoostEnable(){
  timeTempBoostStarted = millis();
  flagTempBoost = true;
  tempBoostBackup = tempSet;
  tempSet = tempMax;
}
//Iron
//Encoder
void encoderTick() {
  DT_now = digitalRead(CLK);          // читаем текущее положение CLK
  SW_state = !digitalRead(SW);        // читаем положение кнопки SW
  // отработка нажатия кнопки энкодера
  if (SW_state && !butt_flag && millis() - debounce_timer > 200) {
    hold_flag = 0;
    butt_flag = 1;
    turn_flag = 0;
    debounce_timer = millis();
    encoderClick();
  }
  if (!SW_state && butt_flag && millis() - debounce_timer > 200 && millis() - debounce_timer < 500) {
    butt_flag = 0;
    if (!turn_flag && !hold_flag) { // если кнопка отпущена и ручка не поворачивалась
      turn_flag = 0;
      encoderPress();
    }
    debounce_timer = millis();
  }
  if (SW_state && butt_flag && millis() - debounce_timer > 800 && !hold_flag) {
    hold_flag = 1;
    if (!turn_flag) { // если кнопка отпущена и ручка не поворачивалась
      turn_flag = 0;
      encoderHold();
	  //tempBoostEnable(); //TEMPORAL STUB for the temperature boost functionality
    }
  }
  if (!SW_state && butt_flag && hold_flag) {
    butt_flag = 0;
    debounce_timer = millis();
  }
  if (DT_now != DT_last) {            // если предыдущее и текущее положение CLK разные, значит был поворот
    if (digitalRead(DT) != DT_now) {  // если состояние DT отличается от CLK, значит крутим по часовой стрелке
      if (SW_state) {          // если кнопка энкодера нажата
        adjustTemp(10);
      } else {                  // если кнопка энкодера не нажата
        adjustTemp(2);
      }
    } else {                          // если совпадают, значит против часовой
      if (SW_state) {          // если кнопка энкодера нажата
        adjustTemp(-10);
      } else {                  // если кнопка энкодера не нажата
        adjustTemp(-2);
      }
    }
    turn_flag = 1;                    // флаг что был поворот ручки энкодера
  }
  DT_last = DT_now;                  // обновить значение для энкодера
}
void encoderClick() {
}
void encoderPress() {
}
void encoderHold() {
  Serial.println("TEMPORAL STUB for the temperature boost functionality");
  Serial.println("To enable temperature boost functionality uncomment the call to tempBoostEnable() function in encoderTick()");
}
//Encoder
