/*
  Student: Yiwen Kuang (Name on Canvas: Even)
  ID: H00391696, yk2011
  Description:
    RTOS-based implementation of the monitor system using FreeRTOS on ESP32.
    Fully satisfies all 7 real-time requirements using independent tasks and mutex protection.

    Includes timing compensation for B31DGCyclicExecutiveMonitor due to RTOS startup delay.
    Also includes stack usage measurement using uxTaskGetStackHighWaterMark().
*/

#include <Arduino.h>
#include <B31DGMonitor.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/semphr.h"

#define SIGNAL_OUT_A   19
#define SIGNAL_OUT_B   21
#define FREQ_INPUT_A   34
#define FREQ_INPUT_B   35
#define LED_INDICATOR  2
#define LED_TOGGLE     13
#define BUTTON_INPUT   4

// Monitor: Add timing offset to correct FreeRTOS scheduler delay
B31DGCyclicExecutiveMonitor monitor(1479); // microseconds offset

float F1 = 0.0;
float F2 = 0.0;
SemaphoreHandle_t freqMutex;  // Mutex to protect F1/F2 access

bool ledToggleState = false;  // Button-controlled LED

TaskHandle_t Task1Handle, Task2Handle, Task3Handle, Task4Handle, Task5Handle, TaskButtonHandle;

// Utility Function: Generate one waveform sequence for task 1/2
void generateWave(int pin, int h1, int l1, int h2) {
  digitalWrite(pin, HIGH); 
  delayMicroseconds(h1);
  digitalWrite(pin, LOW);  
  delayMicroseconds(l1);
  digitalWrite(pin, HIGH); 
  delayMicroseconds(h2);
  digitalWrite(pin, LOW);
}

// Task 1: Output signal A every 4ms
void Task1(void *pv) {
  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(4);
  for (;;) {
    monitor.jobStarted(1);
    generateWave(SIGNAL_OUT_A, 250, 50, 300);
    monitor.jobEnded(1);

    // Monitor stack usage (optional)
    UBaseType_t stackFree = uxTaskGetStackHighWaterMark(NULL);
    Serial.printf("[T1] Stack remaining: %u words\n", stackFree);

    vTaskDelayUntil(&lastWake, period);
  }
}

// Task 2: Output signal B every 3ms
void Task2(void *pv) {
  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(3);
  for (;;) {
    monitor.jobStarted(2);
    generateWave(SIGNAL_OUT_B, 100, 50, 200);
    monitor.jobEnded(2);

    UBaseType_t stackFree = uxTaskGetStackHighWaterMark(NULL);
    Serial.printf("[T2] Stack remaining: %u words\n", stackFree);

    vTaskDelayUntil(&lastWake, period);
  }
}

// Task 3: Poll frequency F1 every 10ms
void Task3(void *pv) {
  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(10);
  int prev = LOW;
  unsigned long lastRise = 0;

  for (;;) {
    monitor.jobStarted(3);
    int curr = digitalRead(FREQ_INPUT_A);
    unsigned long now = micros();

    if (prev == LOW && curr == HIGH) {
      if (lastRise != 0) {
        float p = (now - lastRise) / 1000000.0;
        if (p > 0.0005) {
          xSemaphoreTake(freqMutex, portMAX_DELAY);
          F1 = 1.0 / p;
          xSemaphoreGive(freqMutex);
        }
      }
      lastRise = now;
    }
    prev = curr;
    monitor.jobEnded(3);

    UBaseType_t stackFree = uxTaskGetStackHighWaterMark(NULL);
    Serial.printf("[T3] Stack remaining: %u words\n", stackFree);

    vTaskDelayUntil(&lastWake, period);
  }
}

// Task 4: Poll frequency F2 every 10ms
void Task4(void *pv) {
  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(10);
  int prev = LOW;
  unsigned long lastRise = 0;

  for (;;) {
    monitor.jobStarted(4);
    int curr = digitalRead(FREQ_INPUT_B);
    unsigned long now = micros();

    if (prev == LOW && curr == HIGH) {
      if (lastRise != 0) {
        float p = (now - lastRise) / 1000000.0;
        if (p > 0.0005) {
          xSemaphoreTake(freqMutex, portMAX_DELAY);
          F2 = 1.0 / p;
          xSemaphoreGive(freqMutex);
        }
      }
      lastRise = now;
    }
    prev = curr;
    monitor.jobEnded(4);

    UBaseType_t stackFree = uxTaskGetStackHighWaterMark(NULL);
    Serial.printf("[T4] Stack remaining: %u words\n", stackFree);

    vTaskDelayUntil(&lastWake, period);
  }
}

// Task 5: Monitor doWork + LED every 5ms
void Task5(void *pv) {
  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(5);
  for (;;) {
    monitor.jobStarted(5);
    monitor.doWork();

    xSemaphoreTake(freqMutex, portMAX_DELAY);
    float sum = F1 + F2;
    xSemaphoreGive(freqMutex);

    digitalWrite(LED_INDICATOR, (sum > 1500) ? HIGH : LOW);
    monitor.jobEnded(5);

    UBaseType_t stackFree = uxTaskGetStackHighWaterMark(NULL);
    Serial.printf("[T5] Stack remaining: %u words\n", stackFree);

    vTaskDelayUntil(&lastWake, period);
  }
}

// Task 6: Button press detection and LED toggle
void TaskButton(void *pv) {
  bool lastButton = HIGH;
  for (;;) {
    bool current = digitalRead(BUTTON_INPUT);
    if (lastButton == HIGH && current == LOW) {
      ledToggleState = !ledToggleState;
      digitalWrite(LED_TOGGLE, ledToggleState);
      monitor.doWork();
      vTaskDelay(pdMS_TO_TICKS(200)); // debounce
    }
    lastButton = current;

    UBaseType_t stackFree = uxTaskGetStackHighWaterMark(NULL);
    Serial.printf("[T6] Stack remaining: %u words\n", stackFree);

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Setup: Pin init + task creation
void setup() {
  Serial.begin(115200);

  pinMode(SIGNAL_OUT_A, OUTPUT);
  pinMode(SIGNAL_OUT_B, OUTPUT);
  pinMode(FREQ_INPUT_A, INPUT);
  pinMode(FREQ_INPUT_B, INPUT);
  pinMode(LED_INDICATOR, OUTPUT);
  pinMode(LED_TOGGLE, OUTPUT);
  pinMode(BUTTON_INPUT, INPUT_PULLUP);

  freqMutex = xSemaphoreCreateMutex();  // Create mutex to protect F1/F2

  monitor.startMonitoring();  // Start task monitoring (with offset compensation)

  // Create tasks with priority and stack sizes
  xTaskCreate(Task1, "Task1", 2048, NULL, 3, &Task1Handle);
  xTaskCreate(Task2, "Task2", 2048, NULL, 3, &Task2Handle);
  xTaskCreate(Task3, "Task3", 2048, NULL, 2, &Task3Handle);
  xTaskCreate(Task4, "Task4", 2048, NULL, 2, &Task4Handle);
  xTaskCreate(Task5, "Task5", 2048, NULL, 1, &Task5Handle);
  xTaskCreate(TaskButton, "Button", 2048, NULL, 1, &TaskButtonHandle);
}

// === Main loop not used in RTOS mode ===
void loop() {
  // Empty — tasks are managed by the FreeRTOS kernel
}
