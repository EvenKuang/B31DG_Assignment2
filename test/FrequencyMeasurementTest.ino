/*
  Description:
    This file is used to **independently test** the frequency measurement logic 
    for Task 3 and Task 4, as well as LED control logic (Task 5).
    
    - Two input signals are read from GPIO 34 and 35.
    - Their frequencies are measured using polling (no interrupts).
    - If the sum of the two frequencies exceeds 1500 Hz, a built-in LED (GPIO 2) is turned ON.
    - The measured frequencies and LED status are printed to Serial Monitor.

    This is useful for validating signal input and measurement accuracy 
    before integrating into the full system.
*/

#include <Arduino.h>

const int freqPin1 = 34;  // Input signal for frequency F1
const int freqPin2 = 35;  // Input signal for frequency F2
const int ledPin   = 2;   // Onboard LED (or external LED) for threshold indication

unsigned long lastTime1 = 0, lastTime2 = 0;  // Last rising edge time
float frequency1 = 0, frequency2 = 0;        // Measured frequencies

void setup() {
  Serial.begin(115200);              // Start serial communication for output
  pinMode(freqPin1, INPUT);          // Set frequency input pins
  pinMode(freqPin2, INPUT);
  pinMode(ledPin, OUTPUT);           // Set LED pin as output
}

void loop() {
  measureFrequency();  // Measure both input signal frequencies
  controlLED();        // Turn LED on/off based on frequency sum
}

// Measure the frequencies of two square wave inputs
void measureFrequency() {
  static int lastState1 = LOW, lastState2 = LOW;  // Remember previous signal states
  int currentState1 = digitalRead(freqPin1);
  int currentState2 = digitalRead(freqPin2);
  unsigned long currentTime = micros();           // Current time in microseconds

  // Frequency 1
  if (currentState1 == HIGH && lastState1 == LOW) {  // Detect rising edge
    if (lastTime1 > 0) {
      float period1 = (currentTime - lastTime1) / 1000000.0;  // Convert µs to s
      if (period1 > 0) frequency1 = 1.0 / period1;             // Calculate frequency
    }
    lastTime1 = currentTime;
  }
  lastState1 = currentState1;

  // Frequency 2
  if (currentState2 == HIGH && lastState2 == LOW) {
    if (lastTime2 > 0) {
      float period2 = (currentTime - lastTime2) / 1000000.0;
      if (period2 > 0) frequency2 = 1.0 / period2;
    }
    lastTime2 = currentTime;
  }
  lastState2 = currentState2;

  // Print the measured frequencies to Serial Monitor
  Serial.print("F1: ");
  Serial.print(frequency1);
  Serial.print(" Hz, F2: ");
  Serial.print(frequency2);
  Serial.println(" Hz");
}

// Turn LED on if F1 + F2 > 1500Hz
void controlLED() {
  float totalFrequency = frequency1 + frequency2;
  if (totalFrequency > 1500.0) {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED ON: F1 + F2 > 1500 Hz");
  } else {
    digitalWrite(ledPin, LOW);
    Serial.println("LED OFF: F1 + F2 <= 1500 Hz");
  }
}
