#include <Arduino.h>
#include "B31DGMonitor.h"

B31DGCyclicExecutiveMonitor monitor;  // Monitor instance for tracking tasks

const int freqPin1 = 34;      // Input signal 1
const int freqPin2 = 35;      // Input signal 2
const int ledPin = 2;         // LED controlled by frequency sum
const int buttonPin = 4;      // Pushbutton input
const int Buttonled = 13;     // LED controlled by button press
#define SIGNAL1_PIN 19        // Output pin for square wave signal 1
#define SIGNAL2_PIN 21        // Output pin for square wave signal 2

int ledState = HIGH;               // Current LED state (on/off)
int buttonState;                   // Current state from button pin
int lastButtonState = LOW;         // Previous state for debounce tracking

unsigned long lastDebounceTime = 0;  // Time of last button toggle
unsigned long debounceDelay = 50;    // Debounce duration

unsigned long lastTime1 = 0, lastTime2 = 0, lastTime3 = 0, lastTime4 = 0, lastTime5 = 0;

float frequency1 = 0;
float frequency2 = 0;

unsigned long previousMillisSignal = 0;
unsigned long previousMillisMeasure = 0;
unsigned long previousMillisLED = 0;
const int signalInterval = 1;    // Signal generation every 1ms
const int measureInterval = 10;  // Frequency check every 10ms
const int ledInterval = 100;     // LED check every 100ms

const unsigned long period1 = 4;   // Task1 every 4ms
const unsigned long period2 = 3;   // Task2 every 3ms
const unsigned long period3 = 10;  // Task3 every 10ms
const unsigned long period4 = 10;  // Task4 (not implemented)
const unsigned long period5 = 5;   // Task5 every 5ms

void setup() {
  pinMode(SIGNAL1_PIN, OUTPUT);
  pinMode(SIGNAL2_PIN, OUTPUT);
  pinMode(freqPin1, INPUT);
  pinMode(freqPin2, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(Buttonled, OUTPUT);

  digitalWrite(ledPin, ledState);  // Initialize LED state

  monitor.startMonitoring();  // Start monitor (job tracking currently disabled)
}

// Task 1: Generate square wave signal A
void Task1() {
  // monitor.jobStarted(1);
  digitalWrite(SIGNAL1_PIN, HIGH);
  delayMicroseconds(250);
  digitalWrite(SIGNAL1_PIN, LOW);
  delayMicroseconds(50);
  digitalWrite(SIGNAL1_PIN, HIGH);
  delayMicroseconds(300);
  digitalWrite(SIGNAL1_PIN, LOW);
  // monitor.jobEnded(1);
}

// Task 2: Generate square wave signal B
void Task2() {
  // monitor.jobStarted(2);
  digitalWrite(SIGNAL2_PIN, HIGH);
  delayMicroseconds(100);
  digitalWrite(SIGNAL2_PIN, LOW);
  delayMicroseconds(50);
  digitalWrite(SIGNAL2_PIN, HIGH);
  delayMicroseconds(200);
  digitalWrite(SIGNAL2_PIN, LOW);
  // monitor.jobEnded(2);
}

// Task 3: Measure frequencies of both input signals
void Task3() {
  // monitor.jobStarted(3);

  // Redundant logic below is preserved for testing and comparison
  /*
  if (currentState1 != lastState1 && currentState1 == HIGH) {
    if (lastTime1 > 0) {
      float period1 = (currentTime - lastTime1) / 1000000.0;
      frequency1 = 1.0 / period1;
    }
    lastTime1 = currentTime;
  }
  lastState1 = currentState1;
  */

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisMeasure >= measureInterval) {
    previousMillisMeasure = currentMillis;

    // Measure frequency 1
    static int lastState1 = LOW;
    int currentState1 = digitalRead(freqPin1);
    if (currentState1 != lastState1 && currentState1 == HIGH) {
      unsigned long currentTime = micros();
      if (lastTime1 > 0) {
        float period = (currentTime - lastTime1) / 1000000.0;
        frequency1 = 1.0 / period;
      }
      lastTime1 = currentTime;
    }
    lastState1 = currentState1;

    // Measure frequency 2
    static int lastState2 = LOW;
    int currentState2 = digitalRead(freqPin2);
    if (currentState2 != lastState2 && currentState2 == HIGH) {
      unsigned long currentTime = micros();
      if (lastTime2 > 0) {
        float period = (currentTime - lastTime2) / 1000000.0;
        frequency2 = 1.0 / period;
      }
      lastTime2 = currentTime;
    }
    lastState2 = currentState2;

    // Output frequency values to serial
    Serial.print("Frequency 1: ");
    Serial.print(frequency1);
    Serial.print(" Hz\t Frequency 2: ");
    Serial.print(frequency2);
    Serial.println(" Hz");
  }

  // monitor.jobEnded(3);
}

// Task 5: Call monitor.doWork (simulated periodic task)
void Task5() {
  // monitor.jobStarted(5);
  monitor.doWork();
  // monitor.jobEnded(5);
}

// Task 6: LED on/off based on frequency sum (currently disabled)
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

// Task 7: Debounced pushbutton toggle + doWork
void Task7() {
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        ledState = !ledState;
      }
    }
  }

  digitalWrite(Buttonled, ledState);
  lastButtonState = reading;

  monitor.doWork();  // Manual call for monitor functionality
}

// Main loop: all tasks run every cycle (not scheduled)
void loop() {
  unsigned long currentMillis = millis();

  // Task1: normally runs every 4ms
  // if (currentMillis - lastTime1 >= period1) {
  //   lastTime1 = currentMillis;
      Task1();
  // }

  // Task2: normally runs every 3ms
  // if (currentMillis - lastTime2 >= period2) {
  //   lastTime2 = currentMillis;
      Task2();
  // }

  // Task3: frequency polling
  // if (currentMillis - lastTime3 >= period3) {
  //   lastTime3 = currentMillis;
      Task3();
  // }

  // Task4: frequency polling
  // if (currentMillis - lastTime4 >= period4) {
  //   lastTime4 = currentMillis;
      Task4();
  // }

  // Task6();  // disabled

  Task7();  // Poll button and toggle LED
}
