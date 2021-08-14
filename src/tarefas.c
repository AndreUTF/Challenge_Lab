#include <stdint.h>
#include <stdbool.h>
#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "driverbuttons.h"
#include "cmsis_os2.h" // CMSIS-RTOS
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"

osThreadId_t thread1_id, thread2_id, thread3_id, thread4_id, control_id;
osMutexId_t mutex1_id, mutex2_id;
osMessageQueueId_t message_id;

typedef struct // STRUCT
{
    uint8_t LED;   //LED
    uint16_t PWM; //PWM
    bool isActive; //isActive
} LEDPWM; // struct LEDPWM

typedef struct // STRUCT
{
    uint8_t led; //LED
    uint8_t pwm; //PWM
    bool change_led; 
    bool change_pwm;
    bool read_queue; 
} msgLEDPWM; //STRUCT

LEDPWM ledPWM1, ledPWM2, ledPWM3, ledPWM4;
msgLEDPWM msg1;
bool change_led, change_pwm, read_queue;

void control_task(void *arg){
  int var = 0;
  int state = 0;
  uint8_t read_bt1 = 0;
  uint8_t read_bt2 = 0;
  while(1){
    //osThreadFlagsWait(0x0011, NULL, osWaitForever);
    if(msg1.change_led == true){
      if(msg1.led == LED1)
      {
        msg1.led = LED2;
        ledPWM1.isActive = false;
        ledPWM2.isActive = true;
        ledPWM3.isActive = false;
        ledPWM4.isActive = false;
      }
      else if(msg1.led == LED2)
      {
        msg1.led = LED3;
        ledPWM1.isActive = false;
        ledPWM2.isActive = false;
        ledPWM3.isActive = true;
        ledPWM4.isActive = false;
      }
      else if(msg1.led == LED3)
      {
        msg1.led = LED4;
        ledPWM1.isActive = false;
        ledPWM2.isActive = false;
        ledPWM3.isActive = false;
        ledPWM4.isActive = true;
      }
      else if(msg1.led == LED4)
      {
        msg1.led = LED1;
        ledPWM1.isActive = true;
        ledPWM2.isActive = false;
        ledPWM3.isActive = false;
        ledPWM4.isActive = false;
      }
      msg1.change_led = false;
    }
    if(msg1.change_pwm == true){
      if(msg1.led == LED1){
        ledPWM1.PWM += 1;
        if(ledPWM1.PWM > 10)
        {
          ledPWM1.PWM = 0;
        }
        msg1.pwm = ledPWM1.PWM;
        msg1.change_pwm = false;
      }
      else if(msg1.led == LED2){
        ledPWM2.PWM += 1;
        if(ledPWM2.PWM > 10)
        {
          ledPWM2.PWM = 0;
        }
          
        msg1.pwm = ledPWM2.PWM;
        msg1.change_pwm = false;
      }
      else if(msg1.led == LED3){
        ledPWM3.PWM += 1;
        if(ledPWM3.PWM > 10)
        {
          ledPWM3.PWM = 0;
        }
        msg1.pwm = ledPWM3.PWM;
        msg1.change_pwm = false;
      }
      else if(msg1.led == LED4){
        ledPWM4.PWM += 1;
        if(ledPWM4.PWM > 10)
        {
          ledPWM4.PWM = 0;
        }
        msg1.pwm = ledPWM4.PWM;
        msg1.change_pwm = false;
      }    
    }
    osMessageQueuePut(message_id, &msg1, NULL, 0);
  }
}

void threadPWM(void *arg){
  uint32_t tick;
  LEDPWM* foo;
  uint8_t LED;
  uint16_t PWM;
  uint8_t state;
  while(1){
    foo = (LEDPWM*) arg;
    LED = foo->LED;
    PWM = foo->PWM;
      if(foo->isActive == true){
        PWM = msg1.pwm;
        tick = osKernelGetTickCount();
        osMutexAcquire(mutex2_id,osWaitForever);
        LEDOn(LED);
        osMutexRelease(mutex2_id);
        osDelayUntil(tick + (8*PWM));
        
        tick = osKernelGetTickCount();
        osMutexAcquire(mutex2_id,osWaitForever);
        LEDOff(LED);
        osMutexRelease(mutex2_id);
        osDelayUntil(tick + 8*(10-PWM));
        
        tick = osKernelGetTickCount();
        osMutexAcquire(mutex2_id,osWaitForever);
        LEDOn(LED);
        osMutexRelease(mutex2_id);
        osDelayUntil(tick + PWM);
        
        
        tick = osKernelGetTickCount();
        osMutexAcquire(mutex2_id,osWaitForever);
        LEDOff(LED);
        osMutexRelease(mutex2_id);
        osDelayUntil(tick + (10 - PWM));
      }
      else {
        tick = osKernelGetTickCount();
        osMutexAcquire(mutex2_id,osWaitForever);
        LEDOn(LED);
        osMutexRelease(mutex2_id);
        osDelayUntil(tick + PWM);
        
        tick = osKernelGetTickCount();
        osMutexAcquire(mutex2_id,osWaitForever);
        LEDOff(LED);
        osMutexRelease(mutex2_id);
        osDelayUntil(tick + (10 - PWM));
      }
      if(msg1.read_queue == true){
        state = osMessageQueueGet(message_id, &msg1, NULL, osWaitForever);
        if(state == osOK){
          if(msg1.led == LED1){
            ledPWM1.PWM = msg1.pwm;
          }
          if(msg1.led == LED2){
            ledPWM2.PWM = msg1.pwm;
          }
          if(msg1.led == LED3){

            ledPWM3.PWM = msg1.pwm;
          }
          if(msg1.led == LED4){
            ledPWM4.PWM = msg1.pwm;
          }
        }
        read_queue = false;
      }
    //}
  } // while
} // thread

void GPIOJ_Handler(void){
  int bt1_read = ButtonRead(USW1);
  int bt2_read = ButtonRead(USW2);
  if(bt1_read == 0 && bt2_read != 0){
    msg1.change_led = true;
    msg1.change_pwm = false;
    ButtonIntClear(USW1);
  }
  else if(bt1_read != false && bt2_read == 0){
    msg1.change_led = false;
    msg1.change_pwm = true;
    ButtonIntClear(USW2);
  }
}

void main(void){
  ButtonInit(USW1);
  ButtonInit(USW2);
  ButtonIntEnable(USW1);
  ButtonIntEnable(USW2);
  LEDInit(LED4 | LED3 | LED2 | LED1);
  ledPWM1.LED = LED1;
  ledPWM1.PWM = 1;
  ledPWM1.isActive = true;
  ledPWM2.LED = LED2;
  ledPWM2.PWM = 2;
  ledPWM2.isActive = false;
  ledPWM3.LED = LED3;
  ledPWM3.PWM = 4;
  ledPWM3.isActive = false;
  ledPWM4.LED = LED4;
  ledPWM4.PWM = 6;
  ledPWM4.isActive = false;
  msg1.led = LED1;
  msg1.pwm = 0;
  
  osKernelInitialize();
  mutex1_id = osMutexNew(NULL);
  mutex2_id = osMutexNew(NULL);
  message_id = osMessageQueueNew(5, sizeof(msg1), NULL);
  thread1_id = osThreadNew(threadPWM, (void *)&ledPWM1, NULL);
  thread2_id = osThreadNew(threadPWM, (void *)&ledPWM2, NULL);
  thread3_id = osThreadNew(threadPWM, (void *)&ledPWM3, NULL);
  thread4_id = osThreadNew(threadPWM, (void *)&ledPWM4, NULL);
  control_id = osThreadNew(control_task, NULL, NULL);
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main