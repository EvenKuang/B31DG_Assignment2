/*
  Student: Yiwen Kuang (Name on Canvas: Even)
  ID: H00391696, yk2011
  Description:
    This program implements a Cyclic Executive for the ESP32 using the B31DGCyclicExecutiveMonitor
    library. The goal is to meet seven real-time requirements (RT1–RT7) using polling, spin-waiting,
    and software-level scheduling. Timing precision is achieved by using a 1ms minor cycle with
    Ticker-based interrupts.
*/

#include <Arduino.h>
#include <Ticker.h>
#include <B31DGMonitor.h>

const int MAJOR_CYCLE = 60;        // Major cycle length = LCM of all task periods (3, 4, 5, 10ms)
const int MINOR_CYCLE_MS = 1;      // Each minor cycle = 1ms, triggered by Ticker

const int signalOutPinA = 19;      // Output pin for Task 1: Signal A
const int signalOutPinB = 21;      // Output pin for Task 2: Signal B
const int freqInputPinA = 34;      // Input pin for Task 3: Frequency F1
const int freqInputPinB = 35;      // Input pin for Task 4: Frequency F2
const int ledIndicatorPin = 2;     // Output LED for Task 6: F1 + F2 > 1500Hz
const int ledTogglePin = 13;       // LED toggled by Task 7: button press
const int userButtonPin = 4;       // Input pin for pushbutton

volatile bool cycleTrigger = false;    // Flag to trigger the next 1ms scheduling cycle
Ticker schedulerTick;                  // Ticker for 1ms periodic ISR
B31DGCyclicExecutiveMonitor monitor;   // Built-in monitor to record timing stats & violations

int tick = 0;                          // Current minor cycle (0 to MAJOR_CYCLE-1)
bool ledState = false;                // Toggle state for Task 7
float F1 = 0.0, F2 = 0.0;             // Frequency measurements from Task 3 & 4

int prevA = LOW, prevB = LOW;         // For detecting rising edge (used in F1/F2)
unsigned long lastRiseA = 0, lastRiseB = 0;  // Timestamps of last signal rise

// Executed every 1ms to signal scheduler
void onTick() {
  // Sets a flag for the main loop to run tasks for the current minor cycle
  cycleTrigger = true;
}

// Task 1: Output digital waveform Signal A at 250Hz (4ms period)
void task1() {
  monitor.jobStarted(1);  // Tell monitor task started
  digitalWrite(signalOutPinA, HIGH);
  delayMicroseconds(250);
  digitalWrite(signalOutPinA, LOW);
  delayMicroseconds(50);
  digitalWrite(signalOutPinA, HIGH);
  delayMicroseconds(300);
  digitalWrite(signalOutPinA, LOW);
  monitor.jobEnded(1);    // Tell monitor task ended
}

// Task 2: Output digital waveform Signal B at 333Hz (3ms period)
void task2() {
  monitor.jobStarted(2);
  digitalWrite(signalOutPinB, HIGH);
  delayMicroseconds(100);
  digitalWrite(signalOutPinB, LOW);
  delayMicroseconds(50);
  digitalWrite(signalOutPinB, HIGH);
  delayMicroseconds(200);
  digitalWrite(signalOutPinB, LOW);
  monitor.jobEnded(2);
}

// Task 3: Poll frequency from signal A (F1) every 10ms
void task3() {
  monitor.jobStarted(3);
  int now = digitalRead(freqInputPinA);        // Read signal
  unsigned long timeNow = micros();            // Current time in µs

  if (prevA == LOW && now == HIGH) {           // Rising edge detected
    if (lastRiseA != 0) {
      float period = (timeNow - lastRiseA) / 1000000.0;  // Time since last rise in seconds
      if (period > 0.0005) F1 = 1.0 / period;            // Avoid glitch spikes
    }
    lastRiseA = timeNow;
  }
  prevA = now;
  monitor.jobEnded(3);
}

// Task 4: Poll frequency from signal B (F2) every 10ms
void task4() {
  monitor.jobStarted(4);
  int now = digitalRead(freqInputPinB);
  unsigned long timeNow = micros();

  if (prevB == LOW && now == HIGH) {
    if (lastRiseB != 0) {
      float period = (timeNow - lastRiseB) / 1000000.0;
      if (period > 0.0005) F2 = 1.0 / period;
    }
    lastRiseB = timeNow;
  }
  prevB = now;
  monitor.jobEnded(4);
}

// Task 5: Background task (non-critical workload)
void task5() {
  monitor.jobStarted(5);
  monitor.doWork();   // Simulated background processing
  monitor.jobEnded(5);
}

// Task 6: Soft RT — Check if F1 + F2 > 1500Hz, toggle LED accordingly
void checkLED() {
  if ((F1 + F2) > 1500) {
    digitalWrite(ledIndicatorPin, HIGH);
  } else {
    digitalWrite(ledIndicatorPin, LOW);
  }
}

// Task 7: Poll button, toggle another LED (event-driven, soft RT)
void checkButton() {
  static bool lastButton = HIGH;
  bool current = digitalRead(userButtonPin);

  if (lastButton == HIGH && current == LOW) { // Rising edge detected
    ledState = !ledState;
    digitalWrite(ledTogglePin, ledState);
    monitor.doWork();  // Log activity
    delay(200);        // Simple debounce
  }
  lastButton = current;
}

// Setup: Initialize pins, timer, and monitoring
void setup() {
  Serial.begin(115200);

  // Configure all pins
  pinMode(signalOutPinA, OUTPUT);
  pinMode(signalOutPinB, OUTPUT);
  pinMode(freqInputPinA, INPUT);
  pinMode(freqInputPinB, INPUT);
  pinMode(ledIndicatorPin, OUTPUT);
  pinMode(ledTogglePin, OUTPUT);
  pinMode(userButtonPin, INPUT_PULLUP);

  // Start 1ms periodic interrupt
  schedulerTick.attach_ms(MINOR_CYCLE_MS, onTick);

  // Start performance monitoring
  monitor.startMonitoring();
}

// Main Scheduler Loop
void loop() {
  if (cycleTrigger) { // Only run once per minor cycle
    cycleTrigger = false;

    // Schedule each task according to its period
    // For example: task1 runs every 4ms -> tick % 4 == 0
    if (tick % 4 == 0) task1();
    if (tick % 3 == 0) task2();
    if (tick %10 == 0) task3();
    if (tick %10 == 0) task4();
    if (tick % 5 == 0) task5();

    // Always check these two (non-periodic, soft RT)
    checkLED();
    checkButton();

    // Move to next minor cycle (roll over every 60ms)
    tick = (tick + 1) % MAJOR_CYCLE;
  }
}
