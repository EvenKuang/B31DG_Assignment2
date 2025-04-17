/* B31DGCyclicExecutiveMonitor C version
   Suggestion: Package the following code in a library separated from your main program
*/

#include "sdkconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include <rom/ets_sys.h>
#include <esp_timer.h>

#define NUMBER_TASKS 5
#define TEST_TIME 10000000

bool bActive = false;

unsigned long timeStart = 0;  
unsigned long timeRelease[NUMBER_TASKS];
unsigned long timeNextDeadlines[NUMBER_TASKS];

unsigned long counterJobs[NUMBER_TASKS];
unsigned long taskViolations[NUMBER_TASKS];
unsigned long timeFirstEnd[NUMBER_TASKS];
unsigned long timeFirstBegin[NUMBER_TASKS];

unsigned long offset = 0;

unsigned long taskRateRequirements[NUMBER_TASKS] = 
         {    4000,  // 1 - digital output signal 1
              3000,  // 2 - digital output signal 2 
             10000,  // 3 - frequency monitor 1
             10000,  // 4 - frequency monitor 2
              5000,  // 5 - doWork
         };
     
void printSummary();              


void jobStarted(int task);    // Call this method whenever your program starts executing an instance of task #task
void jobEnded(int task);      // Call this method whenever your program completed the execution of an instance of task #task
unsigned long startMonitoring(int offset);   // Call this method once, i.e. in the setup/main function of your program, to activate the monitoring. It returns the time the monitor started to monitor
unsigned long getTimeStart(); // Returns the time the monitor started to monitor (also returned by startMonitoring
bool isActive() {return bActive; }
void doWork(); // waste CPU


unsigned long getTimeStart()
{
    return timeStart; 
}
    
unsigned long startMonitoring(int offset)
{
    bActive = true;
    // find the next deadlines
    timeStart = esp_timer_get_time() + offset;

    for (int i=0; i< NUMBER_TASKS; i++) {
      timeFirstEnd[i] = 0;
      timeFirstBegin[i] = 0;
      timeRelease[i] = timeStart;
      counterJobs[i] = 0;
      taskViolations[i]= 0;
      timeNextDeadlines[i] =  timeStart + taskRateRequirements[i];
    }

    return timeStart;
}

void jobStarted(int taskNumber)
{
  if (!bActive) return;
  
  taskNumber--;

  unsigned long now = esp_timer_get_time();
  if (timeFirstBegin[taskNumber]==0) {
      timeFirstBegin[taskNumber] = now;
  }

  if ( (now - timeStart) > TEST_TIME) {
    printSummary();
    ets_delay_us(1000000);
    exit(0);
  }


  if (now < timeRelease[taskNumber]) {
    taskViolations[taskNumber]++;
  } 
}

void jobEnded(int taskNumber)
{
  if (!bActive) return;
  
  unsigned long now = esp_timer_get_time();

  taskNumber--;

  if (timeFirstEnd[taskNumber]==0) {
      timeFirstEnd[taskNumber] = now;
  }

  if ( (now - timeStart) > TEST_TIME) {
    printSummary();
    exit(0);
  }

  counterJobs[taskNumber]++;

  if (now > timeNextDeadlines[taskNumber]) {
    taskViolations[taskNumber]++; 
  }

  timeRelease[taskNumber] = timeNextDeadlines[taskNumber];
  timeNextDeadlines[taskNumber] += taskRateRequirements[taskNumber];
}


void printSummary()
{
    printf("PERFORMANCE SUMMARY\n");
    printf("Start monitoring at ");
    printf("%lu",timeStart);
    printf("\n");
    for (int i=0; i< NUMBER_TASKS; i++) {
      printf("Task #");
      printf("%d",(i+1));
      printf("  ");
      printf("%lu",taskViolations[i]);
      printf("/");
      printf("%lu",counterJobs[i]);
      printf(" violations");
      printf("  First job from ");
      printf("%lu",timeFirstBegin[i]);
      printf(" to ");
      printf("%lu",timeFirstEnd[i]);
      printf("\n");
    }

}


void doWork()
{
    ets_delay_us(500);
}
