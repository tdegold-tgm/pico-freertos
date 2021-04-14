# Pico - RTOS

[TOC]

## Erklärung FreeRTOS

> FreeRTOS is a market-leading real-time operating system (RTOS) for microcontrollers and small microprocessors.

### Tasks
Tasks stellen Funktionen dar, welche periodisch aufgerufen und ausgeführt werden. Dann bekommen sie Zugriff auf den Prozessor und können Aufgabe xyz erledigen. 

Ein Sonderfall dabei ist der IDLE-Task. Dieser wird beim Initialisieren des Schedulers erstellt und läuft mit der kleinstmöglichen Priorität. Dies stellt sicher, dass immer mindestens ein Task läuft, welcher aber nur so lange wie nötig Zugriff auf den Prozessor hat, da jeder andere Task eine höhere Priorität hat und damit vom Scheduler in *running* gesetzt wird [11].

### Scheduler
Der Scheduler ist ein Teil des FreeRTOS-Kernesl, welcher für das Managen der Tasks zuständig ist. Er befördert Tasks in die jeweiligen States und gibt/nimmt ihnen Rechte auf den Prozessor zuzugreifen [12].

Über die Scheduling Policy kann eingestellt werden, welcher Algorithmus vom Scheduler verwendet wird. Dabei wird unterschieden zwishen den, im Folgenden erläuterten, Varianten. Diese lassen sich konfigurieren, indem man die KOnfigurationsparameter `configUSE_PREEMPTION` und `configUSE_TIME_SLICING` entsprechend setzt.

#### Pre-emtive Scheduling mit Time Slicing
`configUSE_PREEMPTION`: 1
`configUSE_TIME_SLICING`: 1

Dieser algorithmus arbeitet mit fixen Prioritäten. D.h., dass der Scheduler die Prioritäten der Tasks nicht ändert, aber die Tasks auch nicht davon abhält, ihre Prioritäten (und die anderere Tasks) selbst zu verändern. Das "pre-emptive" bedeutet, dass ein Task sofort in den *running*-State befördert wird, sollte er eine höhere Prioriät als der aktuelle *running*-Task haben. Die "Zeitscheiben" sorgen dafür, dass Tasks mit der gleichen Priorität abwechselnd auf den Prozessor zugreifen und sich Rechenleistung teilen. 

#### Pre-emptive Scheduling ohne Time Slicing
`configUSE_PREEMPTION`: 1
`configUSE_TIME_SLICING`: 0

Dieser Algorithmus verhält sich gleich wie der vorherige, allerdings teilen sich Tasks mit gleicher (und höchster) Priorität nicht den Prozessor. Anstatt von einem Task auf den anderen zu wechseln wählt der Scheduler einen Task mit höchster Prioriät aus und übergibt den *running*-State nur wenn

1. ein Task mir höherer Priorität erstellt wird oder
2. ein Task in den *blocked*- oder *suspended*-State gesetzt wird.

#### Co-Operatives Scheduling
`configUSE_PREEMPTION`: 0
`configUSE_TIME_SLICING`: any value

Bei Co-Operativem Scheduling wird ein "Re-Schedule" ausschließlich dann durchgeführt, wenn der *running*-Task in den *blocked*-State gelangt oder explizit nach einem verlangt. Dies kann er, indem er die Methode `taskYield()`aufruft.

### States
Dieser Teil befasst sich mit den einzelnen Status, in welchen sich Tasks befinden können [10].

Der Task, der sich im *running*-State befindet hat derzeit Zugriff auf den Prozessor. Wenn es sich um einen Prozessor mit einem Kern handelt, kann es logischerweise nur einen *running*-Task geben.

Alle Tasks, welche nicht blockieren, also weder *blocked* noch *suspended* sind, aber vom Scheduler keinen Zugriff auf den Prozessor haben, also *running* sind, befinden sich im *ready*-State. Sie sind bereit ausgeführt zu werden und warten darauf, vom Scheduler in *running* befördert zu werden.

![](https://i.imgur.com/IFkkHkG.png)

Im *blocked*-State befinden sich alle Tasks, welche auf interne oder externe Events arbeiten. Zum Beispiel kann sich ein Task durch den Aufruf von `vTaskDelay` selbst für eine bestimmte Zeit blockieren. Wenn diese Zeit abgelaufen ist wird er wieder auf *ready* gesetzt und wartet auf den Scheduler. Sollte ein bestimmtes (internes oder externes) Event nach einer bestimmten Zeit (timeout) nicht stattfinden, wird der Task automatisch auf *ready* gesetzt.

Tasks die sich in *suspend* befinden blockieren ebenfalls, haben aber keinen Timeout. Sie werden nicht auf *ready* gesetzt, außer das wird explizit mit `vTaskResume` erzwungen.


## Deyploment

Zunächst wird dfolgendes Repo gecloned, welches ein Pico-`blink`-Projekt und die Anleitung für einen Docker-Container enthält, welcher FreeRTOS deployed und damit den Code in eine .uf2-Datei konvertiert.

```shell=
git clone https://github.com/PicoCPP/RPI-pico-FreeRTOS
cd RPI-pico-FreeRTOS/
cp CMakeLists.txt src/
docker build -t pico-freertos .
```

Folgender (fragwürdig aussehender) Befehl startet den Container, setzt die benötigten Volumes und lässt den Code in eine .uf2-Datei kompilieren.

```shell=
docker run --rm -v ${PWD}/include:/code/include -v ${PWD}/src:/code/src -v ${PWD}/build:/code/build -it pico-freertos:latest bash -c "cd /code && mkdir -p build && cp -uv src/CMakeLists.txt . && cd build && cmake .. && make clean && make"
```

Danach kann in `build/` die `hello-world.uf2` auf den Pico verschoben werden. Das Programm startet dann automatisch.



## Custom Task
Für diese Übung erstellen wir einen zweiten Task und bauen die beiden so um, dass einer die LED am Pico anschaltet und ein "LED ON!" ausgibt und der zweite die LED ausschaltet und ein "LED OFF!" ausgibt. Folgender Code ist dafür notwendig:

### main.cpp

### **LED anschalten Task**

```cpp=
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
```

### **LED ausschalten Task**

```cpp=

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
```

Dabei ist `WAIT` ein Integer, welcher die Dauer des Wartens beschreibt. In unserem Fall wird er mit `#define WAIT = 1000` auf eine Sekunde gesetzt. Die Funktion `nonBlockingDelay` zählt die abzuwartende Zeit herunter.

### **nonBlockingDelay**
```cpp=
#define nonBlockingDelay(x) \
  TickType_t currentTick = xTaskGetTickCount(); \
  while(xTaskGetTickCount() - currentTick < x)
```

Mit der Funktion `xTaskGetTickCount` gibt die Anzahl der Ticks zurück, welche seit Starten des Schedulers vergangen sind [6]. Somit wird in der `while`-Schleife abgefragt, wann die vergangene Dauer kleiner als `WAIT` ist. Es wird also eine Sekunge gewartet, dann erhöht der Task die Priorität des anderen Task um 1, somit setzt ihn der Scheduler in den *blcoked*-State und setzt den zweiten Task auf *running*.

Task zwei schaltet die LED aus und wartet ebenfalls eine Sekunde. Dann verringert er seinen eigene Priorität um 2, somit ist Task 1 wieder hochprior und der Scheduler blocked Task 2 und setzt Task 1 auf *running*.


### **Task Handler**

Damit die Prioritäten gesetzt werden können, müssen die Tasks mit einem Task-Handler initialisiert werden. Diese werden mit 

```cpp=
TaskHandle_t xTaskSetHighHandle = NULL;
TaskHandle_t xTaskSetLowHandle = NULL;
```

erstellt.

### **main-Funktion**
Das erste, was in der `main` passiert ist, dass die Funktion `stdio_init_all()` aufgerufen wird. Somit werden alle Standard-Inpurt-Output-Typen festgerlegt, welche für die Kommunikation über UART und USB benötigt werden [7].

Danach werden die zwei Tasks erstellt und in einen `BaseType_t` gespeichert.

```cpp=
task{1,2} = xTaskCreate(
            vTask{SetHigh, SetLow},         /* Function that implements the task. */
            "Turn LED {on, off}",           /* Text name for the task. */
            1024,                           /* Stack size in words, not bytes. */
            ( void * ) 1,                   /* Parameter passed into the task. */
            3,                              /* Priority at which the task is created. */
            &xTask{SetHigh, SetLow}Handle );  
```

**WICHTIG:** Da die Prioritäten der Taks während der Laufzeit verändert werden, müssen die Tasks mit unterschiedlichen Prioritäten starten. In unserem Fall startet Task 1 mit 3 und Task 2 mit 2. Die maximal-zulässige Priorität wird ebenfalls in der `FreeRTOSConfig.h` definiert. Dort kann sie mit 

```c=
#define configMAX_PRIORITIES <value>
```

gesetzt. Aufzupassen ist, wenn man mit low-power-uC arbeitet und der Parameter 

```c=
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
```

gesetzt ist. Dann ist die maximale Anzahl der Prioritäten 32 [6].

Nachdem die Tasks erstellt wurden muss ein Scheduler aktiviert werden. Dies geschieht über den Funktionsaufruf `vTaskStartScheduler();`. 

Nun wird, wie üblich, eine tautologische `while`-Schleife geschrieben, sodass das Programm endlos läuft. Innerhalt dieser Schleife wird die Funktion `configASSERT(0)` aufgerufen.

```cpp=
vTaskStartScheduler();
while(1)
{
    configASSERT(0); /* We should never get here */
}
```

`configASSERT`ist eine Funktion, welche unter FreeRTOS statt ihrem C-Äquivalent `assert` verwendet wird, da `assert` nicht in allen Kompilern definiert ist, mit denen FreeRTOS kompiliert wird [8]

Diese methode wird im `FreeRTOSConfig.h` definiert und soll im Fehlerfall aufgerufen werden.


## Ausgabe auf tty

Da der Pico auf seine eigene tty schreibt, kann diese mit einem Programm wie *minicom* oder *picocom* ausgelesen werden [9].

Mit

```shell=
minicom -b 115200 -o -D /dev/ttyACM0
```

lässt sich mittels `minicom` sehr einfach die Abfrage von `ttyACM0` ausführen: 

![](https://i.imgur.com/y8tYP8D.gif)

**Troubleshooting**

Wenn man nicht weiß, auf welche tty der Pico schreibt kann dies "leicht" herausgefunden werden. Mann muss den Pico am Gerät anstecken und die .uf2 flashen, dann

```bash=
ls /dev > list
```

nun den Pico ausstecken und 

```bash=
diff < (ls /dev) list
```

Als Ergebnis sollte dann die tty ersichtlich sein, auf die der Pico schreibt (ttyACM0 und ttyUSB0 sind die am häufigsten vorkommenden)


## Ausgabe am Pico
Nachdem die .uf2 geflashed wurde sollte die LED am Pico im ein-sekunden-Takt blinken.

![](https://i.imgur.com/YISmPaG.gif)




## Quellen 

[1] “This page describes the RTOS vTaskDelay() FreeRTOS API function which is part of the RTOS task control API. FreeRTOS is a professional grade, small footprint, open source RTOS for microcontrollers.” https://www.freertos.org/a00127.html (accessed Apr. 07, 2021).
[2] “This page describes the RTOS xTaskCreate() FreeRTOS API function which is part of the RTOS task control API. FreeRTOS is a professional grade, small footprint, open source RTOS for microcontrollers.” https://www.freertos.org/a00125.html (accessed Apr. 07, 2021).
[3] “Writing RTOS tasks in FreeRTOS - implementing tasks as forever loops.” https://www.freertos.org/implementing-a-FreeRTOS-task.html (accessed Apr. 07, 2021).
[4] “PicoCPP/RPI-pico-FreeRTOS.” https://github.com/PicoCPP/RPI-pico-FreeRTOS (accessed Apr. 07, 2021).
[5] “FreeRTOS example for Raspberry Pico : embedded.” https://www.reddit.com/r/embedded/comments/l8umle/freertos_example_for_raspberry_pico/ (accessed Apr. 07, 2021).
[6] "The FreeRTOS™ Reference Manual" https://freertos.org/fr-content-src/uploads/2018/07/FreeRTOS_Reference_Manual_V10.0.0.pdf (accessed Apr. 08, 2021)
[7] "Raspberry Pi Pico SDK Documentation for stdio_init_all()" https://raspberrypi.github.io/pico-sdk-doxygen/group__pico__stdio.html#gadd999f115d6f239056f3b15bfacb3726 (accessed Apr. 08, 2021)
[8] "Mastering the FreeRTOS™ Real Time Kernel" https://www.freertos.org/fr-content-src/uploads/2018/07/161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf (accessed Apr. 08, 2021)
[9] "Arch Wiki | Working with the serial console" https://wiki.archlinux.org/index.php/Working_with_the_serial_console (accessed Apr. 08, 2021)
[10] "Task States" https://www.freertos.org/RTOS-task-states.html (accessed Apr. 08, 2021)
[11] "IDLE Task" https://www.freertos.org/RTOS-idle-task.html (accessed Apr. 08, 2021)
[12] "FreeRTOS Scheduling" https://www.freertos.org/implementation/a00005.html (accessed Apr. 08, 2021)