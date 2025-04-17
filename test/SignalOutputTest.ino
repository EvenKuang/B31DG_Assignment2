/*
  Description:
    This file is used to **independently test** the signal generation patterns
    for Task 1 and Task 2 in the machine monitor system.

    It outputs two square wave signals with specific timing:
    - Signal 1 (on GPIO 19): HIGH 250µs, LOW 50µs, HIGH 300µs, LOW
    - Signal 2 (on GPIO 21): HIGH 100µs, LOW 50µs, HIGH 200µs, LOW

    This test is useful for debugging waveform output using an oscilloscope
    or logic analyzer before integrating into the full system.
*/

#include <B31DGMonitor.h>  // Required by course setup (even if unused in test)


#define SIGNAL1_PIN 19  // First signal output (Task 1 equivalent)
#define SIGNAL2_PIN 21  // Second signal output (Task 2 equivalent)

void setup() {
  // Configure both pins as output
  pinMode(SIGNAL1_PIN, OUTPUT);
  pinMode(SIGNAL2_PIN, OUTPUT);
}

void loop() 
  // Generate Signal 1 (Task 1 test pattern)
  digitalWrite(SIGNAL1_PIN, HIGH);
  delayMicroseconds(250);   // Pulse HIGH for 250µs
  digitalWrite(SIGNAL1_PIN, LOW);
  delayMicroseconds(50);    // Short LOW
  digitalWrite(SIGNAL1_PIN, HIGH);
  delayMicroseconds(300);   // Second pulse HIGH for 300µs
  digitalWrite(SIGNAL1_PIN, LOW);

  // Generate Signal 2 (Task 2 test pattern)
  digitalWrite(SIGNAL2_PIN, HIGH);
  delayMicroseconds(100);   // Pulse HIGH for 100µs
  digitalWrite(SIGNAL2_PIN, LOW);
  delayMicroseconds(50);    // Short LOW
  digitalWrite(SIGNAL2_PIN, HIGH);
  delayMicroseconds(200);   // Second pulse HIGH for 200µs
  digitalWrite(SIGNAL2_PIN, LOW);

  // No delay here: loop will repeat as fast as possible
}
