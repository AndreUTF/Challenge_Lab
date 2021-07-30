#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
#include "driverleds.h" // Projects/drivers
#include "driverbuttons.h" // Projects/drivers
osThreadId_t thread1_id, thread2_id, thread3_id, thread4_id, control_id;
osMutexId_t mutex1_id, mutex2_id;
osMessageQueueId_t message_id;

typedef struct // STRUCT
{
    uint8_t LED;   //LED
    uint16_t PWM; //PWM
} LEDPWM; // struct LEDPWM

typedef struct // STRUCT
{
    uint8_t led; //LED
    uint8_t pwm; //PWM 
} msgLEDPWM; //STRUCT

LEDPWM ledPWM1, ledPWM2, ledPWM3, ledPWM4;
msgLEDPWM msg1;

void control_task(void *arg){
  int var = 0;
  uint8_t read_bt1 = 0;
  uint8_t read_bt2 = 0;
  uint8_t led_initial = 0;
  uint8_t pwm_intital = 0;
  int change = 0;
  int change1 = 0;
  uint32_t tick;
  uint32_t tick1;
  while(1){
    read_bt1 = ButtonRead(USW1);
    read_bt2 = ButtonRead(USW2);
    osMessageQueuePut(message_id, &msg1, NULL, osWaitForever);
    if(read_bt1 == 0 && change == 0)
    {
      tick = osKernelGetTickCount();
      osDelayUntil(tick + 200);
      read_bt1 = ButtonRead(USW1);
      if(read_bt1 == 0)
      {
        tick1 = osKernelGetTickCount();
      }
      if((tick1-tick)>=200)
      {
        change = 1;
        if(msg1.led == LED1)
        {
          msg1.led = LED2;
          change = 0;
        }
        else if(msg1.led == LED2)
        {
          msg1.led = LED3;
          change = 0;
        }
        else if(msg1.led == LED3)
        {
          msg1.led = LED4;
          change = 0;
        }
        else if(msg1.led == LED4)
        {
          msg1.led = LED1;
          change = 0;
        }
      }
    }
    
    if(read_bt2 == 0 && change1 == 0)
    {
      tick = osKernelGetTickCount();
      osDelayUntil(tick + 200);
      read_bt2 = ButtonRead(USW2);
      if(read_bt2 == 0)
      {
        tick1 = osKernelGetTickCount();
      }
      if((tick1-tick)>=200)
      {
        change1 = 1;
        msg1.pwm += 1;
        if(msg1.pwm > 10)
        {
          msg1.pwm = 0;
        }
        change1 = 0;
      }
    }
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
    state = osMessageQueueGet(message_id, &msg1, NULL, osWaitForever);
    if(state == osOK){
      if(msg1.led == LED){
        osMutexAcquire(mutex1_id,osWaitForever);
        PWM = msg1.pwm;
        osMutexAcquire(mutex2_id,osWaitForever);
        LEDOn(LED);
        osMutexRelease(mutex2_id);
        tick = osKernelGetTickCount();
        osDelayUntil(tick + PWM);
        osMutexAcquire(mutex2_id,osWaitForever);
        LEDOff(LED);
        osMutexRelease(mutex2_id);
        tick = osKernelGetTickCount();
        osDelayUntil(tick + (10 - PWM));
        osMutexRelease(mutex1_id);
      }
      
    }
  } // while
} // thread

void main(void){
  ButtonInit(USW1 | USW2);
  LEDInit(LED4 | LED3 | LED2 | LED1);
  ledPWM1.LED = LED1;
  ledPWM1.PWM = 0;
  ledPWM2.LED = LED2;
  ledPWM2.PWM = 0;
  ledPWM3.LED = LED3;
  ledPWM3.PWM = 0;
  ledPWM4.LED = LED4;
  ledPWM4.PWM = 0;
  msg1.led = LED1;
  msg1.pwm = 0;
  
  osKernelInitialize();
  mutex1_id = osMutexNew(NULL);
  mutex2_id = osMutexNew(NULL);
  message_id = osMessageQueueNew(1, sizeof(msg1), NULL);
  thread1_id = osThreadNew(threadPWM, (void *)&ledPWM1, NULL);
  thread2_id = osThreadNew(threadPWM, (void *)&ledPWM2, NULL);
  thread3_id = osThreadNew(threadPWM, (void *)&ledPWM3, NULL);
  thread4_id = osThreadNew(threadPWM, (void *)&ledPWM4, NULL);
  control_id = osThreadNew(control_task, NULL, NULL);
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main
