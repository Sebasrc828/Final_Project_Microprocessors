/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// Modos de visualización de la pantalla / consola
typedef enum {
  MODE_CLOCK_ONLY = 0,
  MODE_TIMER_SET,
  MODE_TIMER_RUNNING,
  MODE_HOUR_SET,
  MODE_CRONOMETER,
  MODE_ALARM
} DisplayMode_t;

// Valores del timepo a editar
typedef enum {
  EDIT_NONE = 0,
  EDIT_HOURS,
  EDIT_MINUTES,
  EDIT_SECONDS
} TimerEditField_t;

typedef enum {
  BUZZER_OFF = 0,
  BUZZER_ON
} BuzzerSound_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

/* Definitions for TaskSM */
osThreadId_t TaskSMHandle;
const osThreadAttr_t TaskSM_attributes = {
  .name = "TaskSM",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskLED */
osThreadId_t TaskLEDHandle;
const osThreadAttr_t TaskLED_attributes = {
  .name = "TaskLED",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for TaskHOURCHANGE */
osThreadId_t TaskHOURCHANGEHandle;
const osThreadAttr_t TaskHOURCHANGE_attributes = {
  .name = "TaskHOURCHANGE",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for TaskCRONOMETER */
osThreadId_t TaskCRONOMETERHandle;
const osThreadAttr_t TaskCRONOMETER_attributes = {
  .name = "TaskCRONOMETER",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal1,
};
/* Definitions for TaskTEMP */
osThreadId_t TaskTEMPHandle;
const osThreadAttr_t TaskTEMP_attributes = {
  .name = "TaskTEMP",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal2,
};
/* Definitions for TaskBUZZER */
osThreadId_t TaskBUZZERHandle;
const osThreadAttr_t TaskBUZZER_attributes = {
  .name = "TaskBUZZER",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal2,
};
/* USER CODE BEGIN PV */

// Mapa de teclas del teclado matricial 4x4
const char KEYPAD_MAP[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

volatile DisplayMode_t modo_pantalla = MODE_CLOCK_ONLY;
volatile BuzzerSound_t Sonar_buzzer = BUZZER_OFF;
volatile TimerEditField_t campo_edicion = EDIT_NONE;

// Variables compartidas para el Temporizador (TaskTEMP)
volatile uint8_t  temporizador_activo = 0;   // 1: contando, 0: detenido
volatile uint32_t temporizador_cuenta = 0;  // Segundos restantes
volatile uint32_t temporizador_ajuste = 0;  // Segundos acumulados durante el ingreso manual

// Banderas de comunicación con TaskHOURCHANGE
volatile uint8_t  cambiar_hora_solicitado = 0;
volatile uint8_t  nueva_hora = 0;
volatile uint8_t  nuevo_minuto = 0;
volatile uint8_t  nuevo_segundo = 0;

// Variables de edición independientes para HH:MM:SS
volatile uint32_t set_hrs = 0;
volatile uint32_t set_min = 0;
volatile uint32_t set_sec = 0;

volatile uint8_t Noche = 0;

/* USER CODE BEGIN PV */
// Variables compartidas para el Cronómetro (TaskCRONOMETER)
volatile uint8_t  cronometro_activo = 0;   // 1: contando, 0: pausado/detenido
volatile uint32_t cronometro_cuenta = 0;  // Segundos transcurridos acumulados
/* USER CODE END PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
void StartTaskSM(void *argument);
void StartTaskLED(void *argument);
void StartTaskHOURCHANGE(void *argument);
void StartTaskCRONOMETER(void *argument);
void StartTaskTEMP(void *argument);
void StartTaskBUZZER(void *argument);

/* USER CODE BEGIN PFP */
char Keypad_Read(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile uint32_t counter = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of TaskSM */
  TaskSMHandle = osThreadNew(StartTaskSM, NULL, &TaskSM_attributes);

  /* creation of TaskLED */
  TaskLEDHandle = osThreadNew(StartTaskLED, NULL, &TaskLED_attributes);

  /* creation of TaskHOURCHANGE */
  TaskHOURCHANGEHandle = osThreadNew(StartTaskHOURCHANGE, NULL, &TaskHOURCHANGE_attributes);

  /* creation of TaskCRONOMETER */
  TaskCRONOMETERHandle = osThreadNew(StartTaskCRONOMETER, NULL, &TaskCRONOMETER_attributes);

  /* creation of TaskTEMP */
  TaskTEMPHandle = osThreadNew(StartTaskTEMP, NULL, &TaskTEMP_attributes);

  /* creation of TaskBUZZER */
  TaskBUZZERHandle = osThreadNew(StartTaskBUZZER, NULL, &TaskBUZZER_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
  // Si la marca mágica ya está grabada en el registro de respaldo, no reescribir la hora
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == 0x32F2)
    {
      return;
    }
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  // Escribir la marca mágica para que no se reinicie la hora en los siguientes bootouts
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);
  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, C1_Pin|C2_Pin|C3_Pin|C4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : R3_Pin R4_Pin R1_Pin R2_Pin */
  GPIO_InitStruct.Pin = R3_Pin|R4_Pin|R1_Pin|R2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : C1_Pin C2_Pin C3_Pin C4_Pin */
  GPIO_InitStruct.Pin = C1_Pin|C2_Pin|C3_Pin|C4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Buzzer_Pin */
  GPIO_InitStruct.Pin = Buzzer_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Buzzer_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
char Keypad_Read(void)
{
  GPIO_TypeDef* col_ports[4] = {GPIOE, GPIOE, GPIOE, GPIOE};
  uint16_t col_pins[4] = {C1_Pin, C2_Pin, C3_Pin, C4_Pin};

  GPIO_TypeDef* row_ports[4] = {GPIOE, GPIOE, GPIOE, GPIOE};
  uint16_t row_pins[4] = {R1_Pin, R2_Pin, R3_Pin, R4_Pin};

  // Poner todas las columnas en nivel ALTO por defecto
  for (int c = 0; c < 4; c++) {
    HAL_GPIO_WritePin(col_ports[c], col_pins[c], GPIO_PIN_SET);
  }

  for (int c = 0; c < 4; c++) {
    // Activar columna actual (BAJO)
    HAL_GPIO_WritePin(col_ports[c], col_pins[c], GPIO_PIN_RESET);

    for (int r = 0; r < 4; r++) {
      // Si la fila responde en BAJO, hay pulsación en (r, c)
      if (HAL_GPIO_ReadPin(row_ports[r], row_pins[r]) == GPIO_PIN_RESET) {
        osDelay(20); // Antirrebote (Debounce)

        if (HAL_GPIO_ReadPin(row_ports[r], row_pins[r]) == GPIO_PIN_RESET) {
          // Esperar liberación de la tecla antes de retornar
          while (HAL_GPIO_ReadPin(row_ports[r], row_pins[r]) == GPIO_PIN_RESET) {
            osDelay(10);
          }
          // Restaurar columna antes de salir
          HAL_GPIO_WritePin(col_ports[c], col_pins[c], GPIO_PIN_SET);
          return KEYPAD_MAP[r][c];
        }
      }
    }
    // Restaurar columna actual a ALTO
    HAL_GPIO_WritePin(col_ports[c], col_pins[c], GPIO_PIN_SET);
  }

  return '\0'; // Ninguna tecla presionada
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartTaskSM */
/**
  * @brief  Function implementing the TaskSM thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTaskSM */
void StartTaskSM(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 5 */

  	RTC_TimeTypeDef t;
	RTC_DateTypeDef d;

	// tamaño de datos para identificar en el serial
	char usb_buffer[128];

	//vlbe auxiliar de impresión en serial
	uint16_t len = 0;

	// Declaración del temporizador de refresco
	uint32_t ultimo_refresco = 0;

	// Contador de dígitos ingresados en el campo activo (0, 1 o 2)
	  uint8_t digitos_ingresados = 0;

  /* Infinite loop */
  for(;;)
  {
	  char tecla = Keypad_Read();

	  if (tecla != '\0')
	      {
	        // ==========================================
	        // 1. MENÚ PRINCIPAL (MODE_CLOCK_ONLY)
	        // ==========================================
	        if (modo_pantalla == MODE_CLOCK_ONLY)
	        {
	          if (tecla == 'A') // Activar Luz de Noche (LED por 3s)
	          {
	            Noche = 1;
	          }
	          else if (tecla == 'B') // Entrar a Ajustar Hora del RTC
	          {
	            HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
	            HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);

	            modo_pantalla = MODE_HOUR_SET;
	            campo_edicion = EDIT_HOURS;
	            set_hrs = t.Hours;
	            set_min = t.Minutes;
	            set_sec = t.Seconds;
	            digitos_ingresados = 0;
	          }
	          else if (tecla == 'C') // Entrar a Ajustar Temporizador
	          {
	            modo_pantalla = MODE_TIMER_SET;
	            campo_edicion = EDIT_HOURS;
	            set_hrs = 0; set_min = 0; set_sec = 0;
	            digitos_ingresados = 0;
	          }
	          else if (tecla == 'D') // Entrar a Modo Cronómetro
	          {
	            modo_pantalla = MODE_CRONOMETER;
	            cronometro_activo = 0; // Inicia pausado
	            cronometro_cuenta = 0; // Inicia en 00:00:00
	          }
	        }

	        // ==========================================
	        // 2. MODO CRONÓMETRO (MODE_CRONOMETER)
	        // ==========================================
	        else if (modo_pantalla == MODE_CRONOMETER)
	        {
	          if (tecla == '*') // Iniciar / Pausar
	          {
	            cronometro_activo = !cronometro_activo;
	          }
	          else if (tecla == 'A') // Reiniciar a 0
	          {
	            cronometro_cuenta = 0;
	          }
	          else if (tecla == '#') // Salir al reloj principal
	          {
	            cronometro_activo = 0;
	            cronometro_cuenta = 0;
	            modo_pantalla = MODE_CLOCK_ONLY;
	          }
	        }

	        // ==========================================
	        // 3. MODOS DE EDICIÓN (TIMER O HORA RTC)
	        // ==========================================
	        else if (modo_pantalla == MODE_TIMER_SET || modo_pantalla == MODE_HOUR_SET)
	        {
	          // --- Selección de Campo a Modificar ---
	          if (tecla == 'A')
	          {
	            campo_edicion = EDIT_HOURS;
	            digitos_ingresados = 0;
	          }
	          else if (tecla == 'B')
	          {
	            campo_edicion = EDIT_MINUTES;
	            digitos_ingresados = 0;
	          }
	          else if (tecla == 'C')
	          {
	            campo_edicion = EDIT_SECONDS;
	            digitos_ingresados = 0;
	          }

	          // --- Ingreso de Dígitos ('0' - '9') ---
	          else if (tecla >= '0' && tecla <= '9')
	          {
	            uint32_t val = tecla - '0';

	            if (campo_edicion == EDIT_HOURS)
	            {
	              if (digitos_ingresados == 0 || digitos_ingresados >= 2)
	              {
	                set_hrs = val;
	                digitos_ingresados = 1;
	              }
	              else
	              {
	                set_hrs = (set_hrs * 10) + val;
	                digitos_ingresados = 2;
	              }

	              if (modo_pantalla == MODE_HOUR_SET && set_hrs > 23) set_hrs = 23;
	              else if (set_hrs > 99) set_hrs = 99;
	            }
	            else if (campo_edicion == EDIT_MINUTES)
	            {
	              if (digitos_ingresados == 0 || digitos_ingresados >= 2)
	              {
	                set_min = val;
	                digitos_ingresados = 1;
	              }
	              else
	              {
	                set_min = (set_min * 10) + val;
	                digitos_ingresados = 2;
	              }

	              if (set_min > 59) set_min = 59;
	            }
	            else if (campo_edicion == EDIT_SECONDS)
	            {
	              if (digitos_ingresados == 0 || digitos_ingresados >= 2)
	              {
	                set_sec = val;
	                digitos_ingresados = 1;
	              }
	              else
	              {
	                set_sec = (set_sec * 10) + val;
	                digitos_ingresados = 2;
	              }

	              if (set_sec > 59) set_sec = 59;
	            }
	          }

	          // --- Confirmar ('*') ---
	          else if (tecla == '*')
	          {
	            if (modo_pantalla == MODE_TIMER_SET)
	            {
	              uint32_t total = (set_hrs * 3600) + (set_min * 60) + set_sec;
	              if (total > 0)
	              {
	                temporizador_cuenta = total;
	                temporizador_activo = 1;
	                modo_pantalla = MODE_TIMER_RUNNING;
	              }
	            }
	            else if (modo_pantalla == MODE_HOUR_SET)
	            {
	              nueva_hora = (uint8_t)set_hrs;
	              nuevo_minuto = (uint8_t)set_min;
	              nuevo_segundo = (uint8_t)set_sec;
	              cambiar_hora_solicitado = 1;
	              modo_pantalla = MODE_CLOCK_ONLY;
	            }
	            campo_edicion = EDIT_NONE;
	            digitos_ingresados = 0;
	          }

	          // --- Cancelar ('#') ---
	          else if (tecla == '#')
	          {
	            modo_pantalla = MODE_CLOCK_ONLY;
	            campo_edicion = EDIT_NONE;
	            digitos_ingresados = 0;
	          }
	        }

	        // ==========================================
	        // 4. TIMER EN EJECUCIÓN
	        // ==========================================
	        else if (modo_pantalla == MODE_TIMER_RUNNING && tecla == '#')
	        {
	          temporizador_activo = 0;
	          temporizador_cuenta = 0;
	          modo_pantalla = MODE_CLOCK_ONLY;
	        }
	      }

	      // ==========================================
	      // REFRESCO DEL MONITOR SERIAL (CADA 1000 ms)
	      // ==========================================
	      if ((osKernelGetTickCount() - ultimo_refresco) >= 1000)
	      {
	        ultimo_refresco = osKernelGetTickCount();

	        HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
	        HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);

	        if (modo_pantalla == MODE_CLOCK_ONLY)
	        {
	          len = snprintf(usb_buffer, sizeof(usb_buffer),
	                         "\033[2K\rHora: %02d:%02d:%02d | ['A': Light, 'B': Clock, 'C': Timer, 'D': Crono]",
	                         t.Hours, t.Minutes, t.Seconds);
	        }
	        else if (modo_pantalla == MODE_CRONOMETER)
	        {
	          uint32_t hrs = cronometro_cuenta / 3600;
	          uint32_t min = (cronometro_cuenta % 3600) / 60;
	          uint32_t sec = cronometro_cuenta % 60;
	          const char *estado = cronometro_activo ? "RUN " : "STOP";

	          len = snprintf(usb_buffer, sizeof(usb_buffer),
	                         "\033[2K\rCrono [%s]: %02lu:%02lu:%02lu | ['*': Start/Pause, 'A': Reset, '#': Esc]",
	                         estado, hrs, min, sec);
	        }
	        else if (modo_pantalla == MODE_TIMER_SET || modo_pantalla == MODE_HOUR_SET)
	        {
	          char sh[8], sm[8], ss[8];
	          snprintf(sh, sizeof(sh), (campo_edicion == EDIT_HOURS)   ? "[%02lu]" : "%02lu", set_hrs);
	          snprintf(sm, sizeof(sm), (campo_edicion == EDIT_MINUTES) ? "[%02lu]" : "%02lu", set_min);
	          snprintf(ss, sizeof(ss), (campo_edicion == EDIT_SECONDS) ? "[%02lu]" : "%02lu", set_sec);

	          const char *titulo = (modo_pantalla == MODE_TIMER_SET) ? "Set Timer" : "Set Clock";

	          len = snprintf(usb_buffer, sizeof(usb_buffer),
	                         "\033[2K\r%s: %s:%s:%s | [A:H, B:M, C:S | '*': Ok, '#': Esc]",
	                         titulo, sh, sm, ss);
	        }
	        else // MODE_TIMER_RUNNING
	        {
	          uint32_t hrs = temporizador_cuenta / 3600;
	          uint32_t min = (temporizador_cuenta % 3600) / 60;
	          uint32_t sec = temporizador_cuenta % 60;

	          len = snprintf(usb_buffer, sizeof(usb_buffer),
	                         "\033[2K\rTimer: %02lu:%02lu:%02lu | [#: Cancel]",
	                         hrs, min, sec);
	        }

	        CDC_Transmit_FS((uint8_t*)usb_buffer, len);
	      }

	      osDelay(50); // Tasa de refresco del monitor serial
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTaskLED */
/**
* @brief Function implementing the TaskLED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskLED */
void StartTaskLED(void *argument)
{
  /* USER CODE BEGIN StartTaskLED */
	uint32_t conteo = 0;
	uint8_t luz = 0;
  /* Infinite loop */
  for(;;)
  {
	  if (Noche == 1)
	  	      {
	  	        // Al activar el led, guardamos el tiempo exacto de inicio
	  	        if (!luz)
	  	        {
	  	          luz = 1;
	  	          conteo = osKernelGetTickCount();
	  	        }

	  	        // Alumbra durante 3 segundos
	  	        if ((osKernelGetTickCount() - conteo) < 3000)
	  	        {
	  	          HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	  	        }
	  	        else
	  	        {
	  	          // Transcurridos los 3 segundos, se apaga
	  	          HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	  	          Noche = 0;
	  	          luz = 0;
	  	        }
	  	      }
	  	      else
	  	      {
	  	        // Garantizar pin en LOW y ceder el control de la CPU a las demás tareas
	  	        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	  	        luz = 0;
	  	      }
	  	        osDelay(100);
  }
  /* USER CODE END StartTaskLED */
}

/* USER CODE BEGIN Header_StartTaskHOURCHANGE */
/**
* @brief Function implementing the TaskHOURCHANGE thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskHOURCHANGE */
void StartTaskHOURCHANGE(void *argument)
{
  /* USER CODE BEGIN StartTaskHOURCHANGE */
  /* Infinite loop */
  for(;;)
  {
	if (cambiar_hora_solicitado)
	    {
	      RTC_TimeTypeDef sTime = {0};

	      sTime.Hours   = nueva_hora;
	      sTime.Minutes = nuevo_minuto;
	      sTime.Seconds = nuevo_segundo;
	      sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	      sTime.StoreOperation = RTC_STOREOPERATION_RESET;

	      // Actualizar el RTC en formato binario
	      HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

	      // Limpiar bandera
	      cambiar_hora_solicitado = 0;
	    }
	    osDelay(200); // Revisa cada 200 ms
  }
  /* USER CODE END StartTaskHOURCHANGE */
}

/* USER CODE BEGIN Header_StartTaskCRONOMETER */
/**
* @brief Function implementing the TaskCRONOMETER thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskCRONOMETER */
void StartTaskCRONOMETER(void *argument)
{
  /* USER CODE BEGIN StartTaskCRONOMETER */
  /* Infinite loop */
  for(;;)
  {
	  if (cronometro_activo)
	      {
	        cronometro_cuenta++;
	      }

	      osDelay(1000); // Incrementar exactamente 1 segundo
  }
  /* USER CODE END StartTaskCRONOMETER */
}

/* USER CODE BEGIN Header_StartTaskTEMP */
/**
* @brief Function implementing the TaskTEMP thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskTEMP */
void StartTaskTEMP(void *argument)
{
  /* USER CODE BEGIN StartTaskTEMP */
  /* Infinite loop */
  for(;;)
  {
	  // Si el temporizador está activo y es mayor a 0
	  if (temporizador_activo && temporizador_cuenta > 0)
	      {
	        temporizador_cuenta--;

	        // Al llegar a 0, detener el temporizador
	        if (temporizador_cuenta == 0)
	        {
	          temporizador_activo = 0;
	          Sonar_buzzer = BUZZER_ON;
	          modo_pantalla = MODE_CLOCK_ONLY; // Regresar a la pantalla de reloj
	          // Aquí se podrá notificar a TaskBUZZER en el futuro
	        }
	      }

	      osDelay(1000); // Decremento exacto cada 1000 ms (1 segundo)
  }
  /* USER CODE END StartTaskTEMP */
}

/* USER CODE BEGIN Header_StartTaskBUZZER */
/**
* @brief Function implementing the TaskBUZZER thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskBUZZER */
void StartTaskBUZZER(void *argument)
{
  /* USER CODE BEGIN StartTaskBUZZER */
	uint32_t tiempo_inicio = 0;
	 uint8_t sonando = 0;
  /* Infinite loop */
  for(;;)
  {
	  if (Sonar_buzzer == BUZZER_ON)
	      {
	        // Al activar el buzzer, guardamos el tiempo exacto de inicio
	        if (!sonando)
	        {
	          sonando = 1;
	          tiempo_inicio = osKernelGetTickCount();
	        }

	        // Parpadear / Conmutar el pin durante 3000 ms (3 segundos)
	        if ((osKernelGetTickCount() - tiempo_inicio) < 3000)
	        {
	          HAL_GPIO_TogglePin(Buzzer_GPIO_Port, Buzzer_Pin);
	          osDelay(250); // Tono intermitente
	        }
	        else
	        {
	          // Transcurridos los 3 segundos, apagar buzzer y reiniciar banderas
	          HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
	          Sonar_buzzer = BUZZER_OFF;
	          sonando = 0;
	        }
	      }
	      else
	      {
	        // Garantizar pin en LOW y ceder el control de la CPU a las demás tareas
	        HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
	        sonando = 0;
	        osDelay(100); // <-- Esencial para no congelar el RTOS
	      }
  }
  /* USER CODE END StartTaskBUZZER */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
