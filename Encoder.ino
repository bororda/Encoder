/*
   Скетч про многоцелевую отработку поворота и нажатий энкодера с защитой от дребезга.
   Код использует аппаратные прерывания, то есть отработка энкодера ведётся всегда, 
   даже если в коде используется delay и долгие циклы.
   Поворот энкодера меняет переменную norm_counter в большую или меньшую сторону с шагом NORM_STEP
   Поворот С НАЖАТОЙ РУКОЯТКОЙ меняет переменную hold_counter в большую или меньшую сторону с шагом HOLD_STEP
   При одиночном нажатии (БЕЗ ПОВОРОТА) вызывается функция encoderPress
   При нажатии и удержании (~ секунду) вызывается функция encoderHold
   При любом нажатии вызывается функция encoderClick (вдруг пригодится)
   AlexGyver Technologies http://alexgyver.ru/
*/
// ПИНЫ ЭНКОДЕРА
#define CLK 2  // ВАЖНО! СЮДА ПОДКЛЮЧАЕТСЯ ПРЕРЫВАНИЕ!
#define DT 3
#define SW 4
// ПИНЫ ЭНКОДЕРА
#define NORM_STEP 1   // шаг изменения переменной norm_counter при вращении
#define HOLD_STEP 1   // шаг изменения переменной hold_counter при нажатии, удерживании и вращении
int norm_counter, hold_counter;
boolean DT_now, DT_last, SW_state, hold_flag, butt_flag, turn_flag;
unsigned long debounce_timer;
void setup() {
  Serial.begin (9600);
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  // настройка прерывания. Для работы достаточно одного прерывания на CLK!
  attachInterrupt(0, encoderTick, CHANGE);
  DT_last = digitalRead(CLK);  // читаем начальное положение CLK (энкодер)
}
void encoderClick() {
}
void encoderPress() {
}
void encoderHold() {
}
void encoderTurn() {
}
void encoderHoldTurn() {
}
void loop() {
  Serial.print("Turn: ");
  Serial.print(norm_counter);
  Serial.print("  HoldTurn: ");
  Serial.println(hold_counter);
  delay(1000);
}
//---------------------------------------------------------------------------
// -------------------------ОТРАБОТКА ЭНКОДЕРА-------------------------------
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
    if (!turn_flag && !hold_flag) {  // если кнопка отпущена и ручка не поворачивалась
      turn_flag = 0;
      encoderPress();
    }
    debounce_timer = millis();
  }
  if (SW_state && butt_flag && millis() - debounce_timer > 800 && !hold_flag) {
    hold_flag = 1;
    if (!turn_flag) {  // если кнопка отпущена и ручка не поворачивалась
      turn_flag = 0;
      encoderHold();
    }
  }
  if (!SW_state && butt_flag && hold_flag) {
    butt_flag = 0;
    debounce_timer = millis();
  }
  if (DT_now != DT_last) {            // если предыдущее и текущее положение CLK разные, значит был поворот
    if (digitalRead(DT) != DT_now) {  // если состояние DT отличается от CLK, значит крутим по часовой стрелке
      if (SW_state) {           // если кнопка энкодера нажата
        hold_counter += HOLD_STEP;
        encoderHoldTurn();
      } else {                  // если кнопка энкодера не нажата
        norm_counter += NORM_STEP;
        encoderTurn();
      }
    } else {                          // если совпадают, значит против часовой
      if (SW_state) {           // если кнопка энкодера нажата
        hold_counter -= HOLD_STEP;
        encoderHoldTurn();
      } else {                  // если кнопка энкодера не нажата
        norm_counter -= NORM_STEP;
        encoderTurn();
      }
    }
    turn_flag = 1;                    // флаг что был поворот ручки энкодера
  }
  DT_last = DT_now;                   // обновить значение для энкодера
}
// -------------------------ОТРАБОТКА ЭНКОДЕРА-------------------------------
//---------------------------------------------------------------------------
