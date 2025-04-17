/*
  Description:
    This is a transitional testing file used to explore how to integrate the
    B31DGCyclicExecutiveMonitor into a basic functional sketch without using a
    scheduler (no FreeRTOS / no cyclic executive).

    Key features:
    - Task 1 & Task 2: Signal generation for GPIO19 and GPIO21.
    - Task 3: Frequency measurement of square wave inputs.
    - Task 5: Call to monitor.doWork().
    - Task 7: Pushbutton logic with LED toggle and monitor call.

    Several parts of the code are commented out or duplicated:
    - They represent experiments with timing or task structuring.
    - Some are intentionally disabled to avoid false violations in monitor.
    - These blocks are kept for future refinement and debugging purposes.

    ❗ Note: This version does not implement precise task scheduling,
    so calls to monitor.jobStarted()/jobEnded() are commented to avoid incorrect output.
*/

#include <Arduino.h>
#include "B31DGMonitor.h"

B31DGCyclicExecutiveMonitor monitor;

const int freqPin1 = 34;       // Input signal 1
const int freqPin2 = 35;       // Input signal 2
const int ledPin = 2;          // Frequency-triggered LED
const int buttonPin = 4;       // Pushbutton
const int Buttonled = 13;      // LED controlled by button
#define SIGNAL1_PIN 19         // Output square wave A
#define SIGNAL2_PIN 21         // Output square wave B

int ledState = HIGH;
int buttonState;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

unsigned long lastTime1 = 0, lastTime2 = 0; // Rising edge tracking
float frequency1 = 0;
float frequency2 = 0;

unsigned long previousMillisSignal = 0;
unsigned long previousMillisMeasure = 0;
unsigned long previousMillisLED = 0;
const int signalInterval = 1;      // Signal generation interval (ms)
const int measureInterval = 10;    // Frequency polling interval (ms)
const int ledInterval = 100;       // LED update interval (ms)

const unsigned long period1 = 4;
const unsigned long period2 = 3;
const unsigned long period3 = 10;
const unsigned long period4 = 10;
const unsigned long period5 = 5;

void setup() {
  pinMode(SIGNAL1_PIN, OUTPUT);
  pinMode(SIGNAL2_PIN, OUTPUT);
  pinMode(freqPin1, INPUT);
  pinMode(freqPin2, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(Buttonled, OUTPUT);

  digitalWrite(ledPin, ledState);

  // Activate monitor (though job tracking is currently disabled)
  monitor.startMonitoring();
}

// Task 1: Generate signal A (GPIO19)
void Task1() {
  // monitor.jobStarted(1);  // disabled: not using strict scheduling yet
  digitalWrite(SIGNAL1_PIN, HIGH); delayMicroseconds(250);
  digitalWrite(SIGNAL1_PIN, LOW);  delayMicroseconds(50);
  digitalWrite(SIGNAL1_PIN, HIGH); delayMicroseconds(300);
  digitalWrite(SIGNAL1_PIN, LOW);
  // monitor.jobEnded(1);
}

// Task 2: Generate signal B (GPIO21)
void Task2() {
  // monitor.jobStarted(2);
  digitalWrite(SIGNAL2_PIN, HIGH); delayMicroseconds(100);
  digitalWrite(SIGNAL2_PIN, LOW);  delayMicroseconds(50);
  digitalWrite(SIGNAL2_PIN, HIGH); delayMicroseconds(200);
  digitalWrite(SIGNAL2_PIN, LOW);
  // monitor.jobEnded(2);
}

// Task 3: Frequency measurement (input 1 & 2)
void Task3() {
  // monitor.jobStarted(3); // disabled: task timing not enforced

  static int lastState1 = LOW, lastState2 = LOW;
  int currentState1 = digitalRead(freqPin1);
  int currentState2 = digitalRead(freqPin2);
  unsigned long currentMillis = millis();

  // === Block below was an earlier test, now commented out ===
  /*
  if (currentState1 != lastState1 && currentState1 == HIGH) {
    if (lastTime1 > 0) {
      float period1 = (micros() - lastTime1) / 1000000.0;
      frequency1 = 1.0 / period1;
    }
    lastTime1 = micros();
  }
  lastState1 = currentState1;

  if (currentState2 != lastState2 && currentState2 == HIGH) {
    if (lastTime2 > 0) {
      float period2 = (micros() - lastTime2) / 1000000.0;
      frequency2 = 1.0 / period2;
    }
    lastTime2 = micros();
  }
  lastState2 = currentState2;
  */

  // Active version: inside 10ms polling interval
  if (currentMillis - previousMillisMeasure >= measureInterval) {
    previousMillisMeasure = currentMillis;

    static int lastState1 = LOW;
    int currentState1 = digitalRead(freqPin1);
    if (currentState1 == HIGH && lastState1 == LOW) {
      unsigned long now = micros();
      if (lastTime1 > 0) {
        float period = (now - lastTime1) / 1000000.0;
        frequency1 = 1.0 / period;
      }
      lastTime1 = now;
    }
    lastState1 = currentState1;

    static int lastState2 = LOW;
    int currentState2 = digitalRead(freqPin2);
    if (currentState2 == HIGH && lastState2 == LOW) {
      unsigned long now = micros();
      if (lastTime2 > 0) {
        float period = (now - lastTime2) / 1000000.0;
        frequency2 = 1.0 / period;
      }
      lastTime2 = now;
    }
    lastState2 = currentState2;

    Serial.print("Frequency 1: ");
    Serial.print(frequency1);
    Serial.print(" Hz\t Frequency 2: ");
    Serial.println(frequency2);
  }

  // monitor.jobEnded(3);
}

// Task 5: Call monitor.doWork()
void Task5() {
  // monitor.jobStarted(5);
  monitor.doWork();  // Call monitor method (not time-bound)
  // monitor.jobEnded(5);
}

// Task 6: Frequency sum LED control (disabled)
// Possibly replaced by button LED in Task7
/*
void Task6() {
  float totalFrequency = frequency1 + frequency2;
  if (totalFrequency > 1500.0) {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED ON: Frequency Exceeded 1500Hz");
  } else {
    digitalWrite(ledPin, LOW);
    Serial.println("LED OFF: Frequency Below 1500Hz");
  }
}
*/

// Task 7: Pushbutton with debounce and LED toggle
void Task7() {
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();  // reset debounce timer
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        ledState = !ledState;  // toggle LED
      }
    }
  }

  digitalWrite(Buttonled, ledState);  // update LED state
  lastButtonState = reading;

  monitor.doWork();  // still call monitor here for consistency
}

// Main loop: All tasks are run unconditionally
// task timing comments left for future scheduling
void loop() {
  unsigned long currentMillis = millis();

  // Task 1: signal A
  // if (currentMillis - lastTime1 >= period1) {
  //   lastTime1 = currentMillis;
      Task1();
  // }

  // Task 2: signal B
  // if (currentMillis - lastTime2 >= period2) {
  //   lastTime2 = currentMillis;
      Task2();
  // }

  // Task 3: frequency check
  // if (currentMillis - lastTime3 >= period3) {
  //   lastTime3 = currentMillis;
      Task3();
  // }

  // Task 4 would go here (same as Task3 with different signal)

  // Task6();  // disabled in current logic

  Task7();    // button handling always active
}
