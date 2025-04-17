/*
  Description:
    This standalone test code is used to **verify the combined functionality**
    of Tasks 1 to 6 in a simplified environment, without real-time scheduling.

    It simulates:
    - Task 1 & 2: Signal generation on GPIO19 and GPIO21
    - Task 3 & 4: Frequency measurement of two square wave inputs
    - Task 5: LED control based on frequency sum
    - Task 6: Button-based LED toggle

    Timing is managed using `millis()` and polling in `loop()`.
*/

#include <Arduino.h>


const int signalPin1 = 19;
const int signalPin2 = 21;
const int freqPin1   = 34;
const int freqPin2   = 35;
const int ledPin     = 13;
const int buttonPin  = 4;


unsigned long lastTime1 = 0, lastTime2 = 0;
float frequency1 = 0, frequency2 = 0;

unsigned long previousMillisSignal = 0;
unsigned long previousMillisMeasure = 0;
unsigned long previousMillisLED = 0;
const int signalInterval  = 1;     // Task1 & Task2: generate signal every 1ms
const int measureInterval = 10;    // Task3 & Task4: measure freq every 10ms
const int ledInterval     = 100;   // Task5 & 6: control LED every 100ms


int buttonState = HIGH;
int lastButtonState = HIGH;
bool ledState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;  // milliseconds

void setup() {
  Serial.begin(115200);

  pinMode(signalPin1, OUTPUT);
  pinMode(signalPin2, OUTPUT);
  pinMode(freqPin1, INPUT);
  pinMode(freqPin2, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  unsigned long currentMillis = millis();

  // === Task 1 & 2: Signal Generation ===
  if (currentMillis - previousMillisSignal >= signalInterval) {
    previousMillisSignal = currentMillis;

    // Task 1: Signal A (250μs, 50μs, 300μs)
    digitalWrite(signalPin1, HIGH); delayMicroseconds(250);
    digitalWrite(signalPin1, LOW);  delayMicroseconds(50);
    digitalWrite(signalPin1, HIGH); delayMicroseconds(300);
    digitalWrite(signalPin1, LOW);  delayMicroseconds(650);

    // Task 2: Signal B (100μs, 50μs, 200μs)
    digitalWrite(signalPin2, HIGH); delayMicroseconds(100);
    digitalWrite(signalPin2, LOW);  delayMicroseconds(50);
    digitalWrite(signalPin2, HIGH); delayMicroseconds(200);
    digitalWrite(signalPin2, LOW);  delayMicroseconds(650);
  }

  // === Task 3 & 4: Frequency Measurement ===
  if (currentMillis - previousMillisMeasure >= measureInterval) {
    previousMillisMeasure = currentMillis;

    // Frequency 1 (GPIO34)
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

    // Frequency 2 (GPIO35)
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

    // Print both frequencies
    Serial.print("Frequency 1: ");
    Serial.print(frequency1);
    Serial.print(" Hz\tFrequency 2: ");
    Serial.println(frequency2);
  }

  // === Task 5 & 6: LED Control + Button ===
  if (currentMillis - previousMillisLED >= ledInterval) {
    previousMillisLED = currentMillis;

    // Button logic with debounce (like Task 7)
    int reading = digitalRead(buttonPin);
    if (reading != lastButtonState) {
      lastDebounceTime = currentMillis;
    }

    if ((currentMillis - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;
        if (buttonState == LOW) {
          ledState = !ledState;  // Toggle LED state
        }
      }
    }
    lastButtonState = reading;

    // Turn LED ON if freq1 + freq2 > 1500Hz
    float totalFreq = frequency1 + frequency2;
    if (totalFreq > 1500.0) {
      digitalWrite(ledPin, HIGH);
      Serial.println("Frequency Exceeded 1500Hz: LED ON");
    } else {
      digitalWrite(ledPin, ledState ? HIGH : LOW); // Combine with toggle
      Serial.println("Frequency Below 1500Hz");
    }
  }
}
