#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "GPIO.hpp"
#include <array>

#define nonBlockingDelay(x) \
  TickType_t currentTick = xTaskGetTickCount(); \
  while(xTaskGetTickCount() - currentTick < x)

#define WAIT 1000


static pico_cpp::GPIO_Pin ledPin(25,pico_cpp::PinType::Output);
TaskHandle_t xTaskSetHighHandle = NULL;
TaskHandle_t xTaskSetLowHandle = NULL;

void vTaskSetHigh( void * pvParameters )
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. 
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    */
    for( ;; )
    {
            ledPin.set_high();
            printf("LED ON!\n");
            
            nonBlockingDelay(WAIT);
            vTaskPrioritySet( xTaskSetLowHandle, uxTaskPriorityGet(NULL)+1 );
    }
}

void vTaskSetLow( void * pvParameters )
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. 
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    */
    for( ;; )
    {  
            ledPin.set_low();
            printf("LED OFF!\n");
            
            nonBlockingDelay(WAIT);
            vTaskPrioritySet( xTaskSetLowHandle, uxTaskPriorityGet(NULL)-2 );
    }
}

int main() 
{
    stdio_init_all();
    BaseType_t task1, task2;
    
    /* Create tasks */
    task1 = xTaskCreate(
            vTaskSetHigh,           /* Function that implements the task. */
            "Turn LED on",          /* Text name for the task. */
            1024,                   /* Stack size in words, not bytes. */
            ( void * ) 1,           /* Parameter passed into the task. */
            3,                      /* Priority at which the task is created. */
            &xTaskSetHighHandle );  

    task2 = xTaskCreate(
            vTaskSetLow,            /* Function that implements the task. */
            "Turn LED of",          /* Text name for the task. */
            1024,                   /* Stack size in words, not bytes. */
            ( void * ) 1,           /* Parameter passed into the task. */
            2,                      /* Priority at which the task is created. */
            &xTaskSetLowHandle ); 

    vTaskStartScheduler();

    while(1)
    {
        configASSERT(0);            /* We should never get here */
    }

}
