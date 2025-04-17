#include <Arduino.h>
#include <B31DGMonitor.h>  // 假设该头文件已正确链接

B31DGCyclicExecutiveMonitor monitor;  

// 引脚定义
const int signalOutPinA = 19;
const int signalOutPinB = 21;
const int freqInputPinA = 34;
const int freqInputPinB = 35;
const int ledIndicatorPin = 2;
const int ledTogglePin = 13;
const int userButtonPin = 4;

// 频率测量结果变量
unsigned long lastEdgeTimeA = 0, lastEdgeTimeB = 0;
float measuredFreqA = 0, measuredFreqB = 0;

// 信号生成任务 A
void generateSignalA(void *pvParameters) {
    while (1) {
        digitalWrite(signalOutPinA, HIGH);
        delayMicroseconds(250);
        digitalWrite(signalOutPinA, LOW);
        delayMicroseconds(50);
        digitalWrite(signalOutPinA, HIGH);
        delayMicroseconds(300);
        digitalWrite(signalOutPinA, LOW);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// 信号生成任务 B
void generateSignalB(void *pvParameters) {
    while (1) {
        digitalWrite(signalOutPinB, HIGH);
        delayMicroseconds(100);
        digitalWrite(signalOutPinB, LOW);
        delayMicroseconds(50);
        digitalWrite(signalOutPinB, HIGH);
        delayMicroseconds(200);
        digitalWrite(signalOutPinB, LOW);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// 频率检测任务
void monitorFrequency(void *pvParameters) {
    static int prevStateA = LOW, prevStateB = LOW;
    unsigned long lastRisingTimeA = 0, lastRisingTimeB = 0;

    while (1) {
        int currentStateA = digitalRead(freqInputPinA);
        int currentStateB = digitalRead(freqInputPinB);
        unsigned long nowMicros = micros();

        // 检测输入 A 的上升沿
        if (currentStateA == HIGH && prevStateA == LOW) {
            if (lastRisingTimeA > 0) {
                float periodA = (nowMicros - lastRisingTimeA) / 10000000.0;
                if (periodA > 0) measuredFreqA = 1.0 / periodA;
            }
            lastRisingTimeA = nowMicros;
        }
        prevStateA = currentStateA;

        // 检测输入 B 的上升沿
        if (currentStateB == HIGH && prevStateB == LOW) {
            if (lastRisingTimeB > 0) {
                float periodB = (nowMicros - lastRisingTimeB) / 10000000.0;
                if (periodB > 0) measuredFreqB = 1.0 / periodB;
            }
            lastRisingTimeB = nowMicros;
        }
        prevStateB = currentStateB;

        Serial.print("FreqA: "); Serial.print(measuredFreqA);
        Serial.print(" Hz, FreqB: "); Serial.println(measuredFreqB);

        taskYIELD();  //让出 CPU  减少阻塞
    }
}

// LED 指示任务
void ledIndicatorTask(void *pvParameters) {
    while (1) {
        float freqSum = measuredFreqA + measuredFreqB;
        if (freqSum > 1500.0) {
            digitalWrite(ledIndicatorPin, HIGH);
        } else {
            digitalWrite(ledIndicatorPin, LOW);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// 按钮控制任务
// void buttonToggleTask(void *pvParameters) {
//     // static bool isLedOn = false;
//     while (1) {
//         if (digitalRead(userButtonPin) == LOW) {
//             // isLedOn = !isLedOn;
//             digitalWrite(ledTogglePin, HIGH);
//             delayMicroseconds(500);  // 稍微延迟
//             // monitor.doWork();
//             vTaskDelay(pdMS_TO_TICKS(200));  // 简单防抖
//         }
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
//      digitalWrite(ledTogglePin, LOW);
// }

void buttonToggleTask(void *pvParameters) {
  while (1) {
    int state = digitalRead(userButtonPin);
    Serial.println(state); // 打印按钮状态，0 表示按下
    if (state == LOW) {
      digitalWrite(ledTogglePin, HIGH);
      delayMicroseconds(500);
      vTaskDelay(pdMS_TO_TICKS(200)); // 防抖动
      monitor.doWork();
    } else {
      digitalWrite(ledTogglePin, LOW); // 加上释放时熄灭
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
    Serial.begin(115200);
    pinMode(signalOutPinA, OUTPUT);
    pinMode(signalOutPinB, OUTPUT);
    pinMode(freqInputPinA, INPUT);
    pinMode(freqInputPinB, INPUT);
    pinMode(ledIndicatorPin, OUTPUT);
    pinMode(ledTogglePin, OUTPUT);
    pinMode(userButtonPin, INPUT_PULLUP);

    monitor.startMonitoring(); 

    xTaskCreate(generateSignalA, "SignalGenA", 1024, NULL, 1, NULL);
    xTaskCreate(generateSignalB, "SignalGenB", 1024, NULL, 1, NULL);
    xTaskCreate(monitorFrequency, "FreqMonitor", 2048, NULL, 2, NULL);
    xTaskCreate(ledIndicatorTask, "LEDStatus", 1024, NULL, 1, NULL);
    xTaskCreate(buttonToggleTask, "ButtonCtrl", 1024, NULL, 1, NULL);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
