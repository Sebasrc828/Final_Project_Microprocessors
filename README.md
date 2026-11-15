# FreeROTS_01

Reloj digital con teclado matricial, LED, buzzer y salida por puerto serie virtual USB (CDC), construido sobre un **STM32F407VET6** con **STM32CubeMX**, **STM32CubeIDE**, **HAL de ST** y **FreeRTOS** (CMSIS-RTOS v2).

Proyecto del reto *Programación en Alto Nivel — Sistema embebido con STM32F407VET6, HAL de ST y FreeRTOS*.

## Contenido

- [Hardware](#hardware)
- [Modos de operación](#modos-de-operación)
- [Tareas de FreeRTOS](#tareas-de-freertos)
- [Estructura del repositorio](#estructura-del-repositorio)
- [Requisitos](#requisitos)
- [Abrir, compilar y ejecutar](#abrir-compilar-y-ejecutar)
- [Versiones de software y bibliotecas](#versiones-de-software-y-bibliotecas)
- [Historial de cambios](#historial-de-cambios)

## Hardware

| Elemento | Detalle |
| --- | --- |
| Microcontrolador | STM32F407VET6 (LQFP100), placa DevEBox STM32F4VE |
| Reloj del sistema | HSE 8 MHz → PLL → SYSCLK 168 MHz |
| Reloj del RTC | LSE externo (PC14 / PC15) |
| Time base de HAL | TIM6 (SysTick queda para FreeRTOS) |
| Depuración | SWD (PA13 SWDIO, PA14 SWCLK) |

### Conexiones

| Periférico | Función | Pin STM32 | Recurso |
| --- | --- | --- | --- |
| Teclado matricial 4×4 | Filas R1–R4 | PE0, PE1, PE2, PE3 | GPIO entrada con pull-up |
| Teclado matricial 4×4 | Columnas C1–C4 | PE4, PE5, PE6, PE7 | GPIO salida |
| LED | Indicador | PA2 | GPIO salida |
| Buzzer | Alarma sonora (activación por GPIO, sin PWM) | PB1 | GPIO salida |
| USB (dispositivo CDC) | Puerto serie virtual para visualizar la hora/menús | PA11 (DM), PA12 (DP) | USB_OTG_FS, clase CDC |
| RTC | Hora y fecha, formato 24 h | — | RTC calendario (LSE) |

Mapa de teclas del teclado:

```
 1  2  3  A
 4  5  6  B
 7  8  9  C
 *  0  #  D
```

## Modos de operación

El firmware define los siguientes modos de visualización (`DisplayMode_t` en `Core/Src/main.c`):

`MODE_CLOCK_ONLY`, `MODE_TIMER_SET`, `MODE_TIMER_RUNNING`, `MODE_HOUR_SET`, `MODE_CRONOMETER`, `MODE_ALARM`.

## Tareas de FreeRTOS

Tareas creadas con `osThreadNew()` (CMSIS-RTOS v2 sobre FreeRTOS), definidas en `Core/Src/main.c` y configuradas en `FreeROTS_01.ioc`.

| Tarea | Prioridad (CMSIS) | Valor | Pila | Periodo (`osDelay`) |
| --- | --- | --- | --- | --- |
| `TaskSM` | `osPriorityNormal` | 24 | 1024 palabras (4096 B) | 50 ms (refresco del monitor serial) |
| `TaskLED` | `osPriorityBelowNormal` | 16 | 256 palabras (1024 B) | 100 ms |
| `TaskHOURCHANGE` | `osPriorityNormal1` | 25 | 512 palabras (2048 B) | 200 ms |
| `TaskCRONOMETER` | `osPriorityBelowNormal1` | 17 | 256 palabras (1024 B) | 1000 ms |
| `TaskTEMP` | `osPriorityBelowNormal2` | 18 | 256 palabras (1024 B) | 1000 ms |
| `TaskBUZZER` | `osPriorityNormal2` | 26 | 256 palabras (1024 B) | 100 ms (250 ms mientras suena) |

Configuración de FreeRTOS (`Core/Inc/FreeRTOSConfig.h`): FreeRTOS V10.3.1, puerto `GCC/ARM_CM4F`, gestor de memoria `heap_4`, expropiación activada (`configUSE_PREEMPTION = 1`), tick de 1000 Hz, `configTOTAL_HEAP_SIZE = 15360` bytes.

## Estructura del repositorio

```
FreeROTS_01/
├── Core/
│   ├── Inc/                  # main.h, FreeRTOSConfig.h, stm32f4xx_hal_conf.h, stm32f4xx_it.h
│   ├── Src/                  # main.c, freertos.c, stm32f4xx_it.c, stm32f4xx_hal_msp.c, ...
│   └── Startup/              # startup_stm32f407vetx.s
├── USB_DEVICE/               # Aplicación y destino USB CDC generados por CubeMX
│   ├── App/
│   └── Target/
├── Drivers/                  # HAL de ST (STM32F4xx_HAL_Driver) y CMSIS
├── Middlewares/
│   ├── ST/STM32_USB_Device_Library/
│   └── Third_Party/FreeRTOS/ # FreeRTOS V10.3.1 + CMSIS_RTOS_V2
├── FreeROTS_01.ioc           # Configuración de STM32CubeMX
├── STM32F407VETX_FLASH.ld    # Linker script (Flash)
├── STM32F407VETX_RAM.ld      # Linker script (RAM)
├── FreeROTS_01 Debug.launch  # Configuración de depuración (J-Link)
├── FreeROTS_01 Debug.cfg     # Configuración de depuración (OpenOCD / ST-LINK)
├── .project / .cproject / .mxproject / .settings/   # Metadatos del proyecto de CubeIDE
├── CHANGELOG.md
└── README.md
```

Todo lo necesario para compilar está dentro del repositorio; las rutas del proyecto son relativas al workspace (`${workspace_loc:/FreeROTS_01}`) y no dependen de ningún computador específico.

## Requisitos

- **STM32CubeIDE** (incluye la toolchain `arm-none-eabi-gcc` y el soporte de depuración).
- **STM32CubeMX** 6.18.1 (opcional; solo para modificar el `.ioc` fuera de CubeIDE).
- Tarjeta con **STM32F407VET6** y un programador/depurador (J-Link o ST-LINK) conectado por SWD.
- Cable USB a la tarjeta para el puerto serie virtual (CDC) y un monitor serial en el computador (por ejemplo, el terminal de CubeIDE, PuTTY o Tera Term).

## Abrir, compilar y ejecutar

1. Clonar el repositorio:
   ```bash
   git clone https://github.com/<usuario>/FreeROTS_01.git
   ```
2. Abrir **STM32CubeIDE** y elegir un workspace cualquiera.
3. Importar el proyecto: **File → Open Projects from File System…** → seleccionar la carpeta clonada `FreeROTS_01` → **Finish**.
   (Alternativa: **File → Import → General → Existing Projects into Workspace**.)
4. Compilar: seleccionar el proyecto y pulsar **Project → Build Project** (icono del martillo). La configuración activa es **Debug**; el binario se genera en `Debug/FreeROTS_01.elf`.
5. Conectar la tarjeta por SWD y por USB.
6. Ejecutar/depurar: **Run → Debug** (usa `FreeROTS_01 Debug.launch`, configurada para J-Link con el dispositivo `STM32F407VE`). Si se usa ST-LINK, crear una configuración de depuración de tipo *STM32 C/C++ Application* con ST-LINK (OpenOCD o GDB server de ST) apuntando a `Debug/FreeROTS_01.elf`.
7. Abrir el monitor serial sobre el puerto COM/`ttyACM` que aparece al conectar el USB de la tarjeta para ver la salida del reloj.

Para regenerar el código desde CubeMX, abrir `FreeROTS_01.ioc` (doble clic en CubeIDE) y usar **Project → Generate Code**. El código propio está dentro de los bloques `USER CODE BEGIN / END` y se conserva (`KeepUserCode=true`).

## Versiones de software y bibliotecas

| Componente | Versión / origen |
| --- | --- |
| STM32CubeMX | 6.18.1 |
| STM32Cube Firmware | STM32Cube FW_F4 V1.28.3 (HAL, CMSIS, USB Device Library) |
| FreeRTOS | V10.3.1 (incluido en `Middlewares/Third_Party/FreeRTOS`) con capa CMSIS-RTOS v2 |
| Toolchain | GNU Tools for STM32 (`arm-none-eabi-`) incluida en STM32CubeIDE |

Las licencias de las bibliotecas de terceros se encuentran dentro de sus respectivas carpetas (`Drivers/`, `Middlewares/`).

## Historial de cambios

Ver [CHANGELOG.md](CHANGELOG.md) y el historial de commits del repositorio.
