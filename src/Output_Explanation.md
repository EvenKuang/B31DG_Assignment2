
---

# 🧪 Output & Scheduling Explanation

This document explains the system's task scheduling, timing, monitoring outputs, and testing behavior for both the **Cyclic Executive** and **FreeRTOS-based** implementations.

---

## Task Timing Table (Cyclic Executive Example)

| Time (ms) | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | ... | 59 |
|-----------|---|---|---|---|---|---|---|---|---|---|-----|-----|----|
| **Task 1** | x |   |   |   | x |   |   |   | x |   |     |     |    |
| **Task 2** | x |   |   | x |   | x |   | x |   | x |     |     |    |
| **Task 3** | x |   |   |   |   |   |   |   |   |   | x   |     |    |
| **Task 4** | x |   |   |   |   |   |   |   |   |   | x   |     |    |
| **Task 5** | x |   | x |   | x |   | x |   | x |   | x   |     |    |
| **LED**    | Checked every minor cycle (`checkLED()`) |
| **Button** | Polled every cycle (`checkButton()`)     |

---

## Task Timing Summary (All Implementations)

| Task | Description            | Period  | Frequency | Hard Deadline? |
|------|------------------------|---------|-----------|----------------|
| T1   | Output digital signal A| 4 ms    | 250 Hz    | ✅              |
| T2   | Output digital signal B| 3 ms    | 333 Hz    | ✅              |  
| T3   | Measure F1 (input)     | 10 ms   | 100 Hz    | ✅              |
| T4   | Measure F2 (input)     | 10 ms   | 100 Hz    | ✅              |
| T5   | Call `doWork()` + LED  | 5 ms    | 200 Hz    | ✅              |
| T6   | Button + Toggle LED    | N/A     | Event     | ❌              |

---

## Output from Monitor (Example)

After running for 10 seconds with the monitor activated, the following summary is printed via Serial:

```
PERFORMANCE SUMMARY
Task #1  0/2500 violations
Task #2  0/3333 violations
Task #3  0/1000 violations
Task #4  0/1000 violations
Task #5  0/2000 violations
```

This means all tasks met their deadlines. There were **zero violations**, indicating correct timing and scheduling.

---

## Cyclic Executive (Manual Scheduling)

- Uses a fixed **major cycle** (e.g., 60ms)
- All tasks are manually assigned to time slots
- Simple, efficient, but less flexible

```
| Time (ms) | 0   | 4   | 8   | 12  | 16  | ... |
|-----------|-----|-----|-----|-----|-----|-----|
| Task1     |  ●  |     |  ●  |     |  ●  |     |
| Task2     |  ●  | ●   | ●   | ●   | ●   |     |
| Task3     |     |     |     |     |  ●  |     |
| Task4     |     |     |     |     |     | ●   |
| Task5     |     | ●   |     | ●   |     | ●   |
```

 LED checks and button polling are done in the loop (soft real-time).

---

## FreeRTOS Scheduling (Preemptive, Modular)

- Each task is created via `xTaskCreate()`
- Timed using `vTaskDelayUntil()`
- Mutex is used for shared data (F1, F2)
- LED and button logic separated into tasks

```
| Time (ms) | 0   | 3   | 4   | 5   | 6   | 10  | ... |
|-----------|-----|-----|-----|-----|-----|-----|-----|
| Task1     |  ●  |     |  ●  |     |  ●  |     |     |
| Task2     |  ●  | ●   | ●   | ●   | ●   | ●   |     |
| Task3     |     |     |     |     |     | ●   |     |
| Task4     |     |     |     |     |     |     | ●   |
| Task5     |     | ●   |     | ●   |     | ●   |     |
| Button    | Runs asynchronously on press      |
```

---

## Summary: Which Scheduler to Use?

| Feature              | Cyclic Executive     | FreeRTOS              |
|----------------------|----------------------|------------------------|
| Precise Timing       | ✅ Yes                | ✅ Yes                 |
| Code Simplicity      | 🟢 Simple but rigid   | 🟠 More modular        |
| Scalability          | ❌ Hard to extend     | ✅ Easy to add tasks   |
| Real-time Accuracy   | ✅ Manually enforced  | ✅ System enforced     |
| Button Responsiveness| ❌ Slight delay       | ✅ Faster response     |

---

> Both implementations successfully meet the 7 real-time requirements.  
> FreeRTOS provides better flexibility and modularity, while the cyclic version is more lightweight and deterministic.

---
