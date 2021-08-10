#include <TM74HC595Display.h>
#include <TimerOne.h>

int SCLK = 7;
int RCLK = 6;
int DIO = 5;

TM74HC595Display disp(SCLK, RCLK, DIO);
unsigned char LED_0F[29];

/*
  Максимально быстрый универсальный код для обработки энкодера
  Работает на перывании (используется одно)
  Тут код построен на bitRead(PIND..) - только для Arduino NANO!
*/
const int buttonPin = 4;     // номер входа, подключенный к кнопке

#define ENC_A 2       // пин энкодера
#define ENC_B 3       // пин энкодера
#define ENC_TYPE 1    // тип энкодера, 0 или 1
#include "GyverEncoder.h"
Encoder enc1(ENC_A, ENC_B);

volatile long encCounter;
volatile long actualCounter;
volatile boolean state0, lastState, turnFlag;
unsigned long dispIsrTimer, countTimer;

float margin = 1.2;

void setup() {
  Serial.begin(9600);
  attachInterrupt(0, counter, CHANGE);
  pinMode(buttonPin, INPUT_PULLUP);
}
void counter() {
  state0 = bitRead(PIND, ENC_A);
  if (state0 != lastState) {
#if (ENC_TYPE == 1)
    turnFlag = !turnFlag;
    if (turnFlag)
      encCounter += (bitRead(PIND, ENC_B) != lastState) ? -1 : 1;
#else
    encCounter += (bitRead(PIND, ENC_B) != lastState) ? -1 : 1;
#endif
    lastState = state0;
  }
}

float ticksToMeters(long value) {
  return value * 0.262 / 1000;
}

long metersToTicks(float value) {
  return value * 1000 / 0.262;
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
    Serial.println("RESET");

    encCounter = metersToTicks(margin);
    actualCounter = encCounter;

    disp.clear();
    disp.dispFloat(ticksToMeters(actualCounter), 2);
  }

  if (millis() - countTimer > 50) {   // каждые 50 миллисекунд ("прозрачный" аналог delay)
    if (actualCounter != encCounter) {
      actualCounter = encCounter;
      disp.clear();   
      disp.dispFloat(ticksToMeters(actualCounter), 2);
      Serial.println(actualCounter);
    }
    countTimer = millis();            // сбросить таймер
  }
  disp_isr();                         // динамическая индикация
}

void disp_isr() {
  if (micros() - dispIsrTimer > 1500) {       // таймер динамической индикации 
    disp.timerIsr();                         // "пнуть" дисплей
    dispIsrTimer = micros();                 // сбросить таймер
  }
}
