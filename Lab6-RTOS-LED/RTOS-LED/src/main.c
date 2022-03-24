/*
* Example RTOS Atmel Studio
*/

#include <asf.h>
#include "conf_board.h"

/************************************************************************/
/* DEFINES                                                              */
/************************************************************************/

#define TASK_MONITOR_STACK_SIZE            (2048/sizeof(portSTACK_TYPE))
#define TASK_MONITOR_STACK_PRIORITY        (tskIDLE_PRIORITY)
#define TASK_LED_STACK_SIZE                (1024/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY            (tskIDLE_PRIORITY)
#define TASK_LED1_STACK_SIZE			   (1024/sizeof(portSTACK_TYPE))
#define TASK_LED1_PRIORITY				   (tskIDLE_PRIORITY)
#define TASK_LED2_STACK_SIZE			   (1024/sizeof(portSTACK_TYPE))
#define TASK_LED2_PRIORITY				   (tskIDLE_PRIORITY)
#define TASK_LED3_STACK_SIZE			   (1024/sizeof(portSTACK_TYPE))
#define TASK_LED3_PRIORITY				   (tskIDLE_PRIORITY)

// Definindo tudo do LED 1:
#define LED1_PIO           PIOA                 // periferico que controla o LED
// #
#define LED1_PIO_ID        ID_PIOA                 // ID do periférico PIOC (controla LED)
#define LED1_PIO_IDX       0                    // ID do LED no PIO
#define LED1_PIO_IDX_MASK  (1 << LED1_PIO_IDX)   // Mascara para CONTROLARMOS o LED

#define LED2_PIO           PIOC                 // periferico que controla o LED
// #
#define LED2_PIO_ID        ID_PIOC                 // ID do periférico PIOC (controla LED)
#define LED2_PIO_IDX       30                    // ID do LED no PIO
#define LED2_PIO_IDX_MASK  (1 << LED2_PIO_IDX)   // Mascara para CONTROLARMOS o LED

#define LED3_PIO           PIOB                 // periferico que controla o LED
// #
#define LED3_PIO_ID        ID_PIOB                 // ID do periférico PIOC (controla LED)
#define LED3_PIO_IDX       2                    // ID do LED no PIO
#define LED3_PIO_IDX_MASK  (1 << LED3_PIO_IDX)   // Mascara para CONTROLARMOS o LED

#define LED_PIO       PIOC
#define LED_PIO_ID    ID_PIOC
#define LED_IDX       8u
#define LED_IDX_MASK  (1u << LED_IDX)

#define BUT1_PIO            PIOD
#define BUT1_PIO_ID         ID_PIOD
#define BUT1_PIO_IDX        28
#define BUT1_PIO_IDX_MASK   (1u << BUT1_PIO_IDX)

#define BUT2_PIO PIOC
#define BUT2_PIO_ID ID_PIOC
#define BUT2_PIO_IDX 31
#define BUT2_PIO_IDX_MASK (1u << BUT2_PIO_IDX) // esse já está pronto.

#define BUT3_PIO PIOA
#define BUT3_PIO_ID ID_PIOA
#define BUT3_PIO_IDX 19
#define BUT3_PIO_IDX_MASK (1u << BUT3_PIO_IDX) // esse já está pronto.



/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void pin_toggle(Pio *pio, uint32_t mask);
void LED_init(int estado);
void BTN_init_with_IRQ(Pio *pio, uint32_t ul_id, const uint32_t ul_mask, void (*p_handler) (uint32_t, uint32_t));
static void configure_console(void);
void but1_callback(void);
void but2_callback(void);
void but3_callback(void);


SemaphoreHandle_t xSemaphore;
SemaphoreHandle_t xSemaphore2;
SemaphoreHandle_t xSemaphore3;


/************************************************************************/
/* RTOS Hooks                                                           */
/************************************************************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) { }
}

extern void vApplicationIdleHook(void) {
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
}

extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) { configASSERT( ( volatile void * ) NULL ); }

/************************************************************************/
/* Funcoes                                                              */
/************************************************************************/

void but1_callback(void){
	printf("but_callback1 \n");
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
}

void but2_callback(void){
	printf("but_callback2 \n");
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphore2, &xHigherPriorityTaskWoken);
}

void but3_callback(void){
	printf("but_callback3 \n");
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphore3, &xHigherPriorityTaskWoken);
}

void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

void configure_pio_output(Pio *pio, const uint32_t ul_mask, uint32_t ul_id, int estado){
	pmc_enable_periph_clk(ul_id);
	pio_set_output(pio, ul_mask, estado, 0, 0);
}

void BTN_init_with_IRQ(Pio *pio, uint32_t ul_id, const uint32_t ul_mask, void (*p_handler) (uint32_t, uint32_t)) {
	pmc_enable_periph_clk(ul_id);
	pio_configure(pio, PIO_INPUT, ul_mask, PIO_PULLUP);
	pio_handler_set(pio, ul_id, ul_mask, PIO_IT_FALL_EDGE, p_handler);
	pio_enable_interrupt(pio, ul_mask);
	pio_get_interrupt_status(pio);
	NVIC_EnableIRQ(ul_id);
	NVIC_SetPriority(ul_id, 4); // Prioridade 4
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits = CONF_UART_STOP_BITS,
	};

	stdio_serial_init(CONF_UART, &uart_serial_options);
	setbuf(stdout, NULL);
}

/************************************************************************/
/* Tasks                                                                */
/************************************************************************/

static void task_monitor(void *pvParameters) {
	static portCHAR szList[256];

	for (;;) {
		printf("--- task ## %u\n", (unsigned int)uxTaskGetNumberOfTasks());
		vTaskList((signed portCHAR *)szList);
		printf(szList);
		vTaskDelay(1000);
	}
}

static void task_led(void *pvParameters) {
	configure_pio_output(LED_PIO, LED_IDX_MASK, LED_PIO_ID, 1);

	const TickType_t xDelay = 2000 / portTICK_PERIOD_MS;

	for (;;) {
		pin_toggle(LED_PIO, LED_IDX_MASK);
		vTaskDelay(xDelay);
	}
}



void pisca_led (Pio *pio, const uint32_t mask, int n, int t) {
	const TickType_t xDelay = t / portTICK_PERIOD_MS;
	for (int i=0;i<n;i++){
		pio_clear(pio, mask);
		vTaskDelay(t);
		pio_set(pio, mask);
		vTaskDelay(t);
	}
}

static void task_led1(void *pvParameters) {
	configure_pio_output(LED1_PIO, LED1_PIO_IDX_MASK, LED1_PIO_ID, 1);
	BTN_init_with_IRQ(BUT1_PIO, BUT1_PIO_ID, BUT1_PIO_IDX_MASK, but1_callback);
	const TickType_t xDelay = 2000 / portTICK_PERIOD_MS;

	for (;;) {
	// aguarda semáforo com timeout de  500 ms
		if( xSemaphoreTake(xSemaphore, ( TickType_t ) 500 / portTICK_PERIOD_MS) == pdTRUE ){
			pin_toggle(LED1_PIO, LED1_PIO_IDX_MASK);
		} else {
			// time out
		}
	}
}

static void task_led2(void *pvParameters) {
	configure_pio_output(LED2_PIO, LED2_PIO_IDX_MASK, LED2_PIO_ID, 1);
	BTN_init_with_IRQ(BUT2_PIO, BUT2_PIO_ID, BUT2_PIO_IDX_MASK, but2_callback);
	const TickType_t xDelay = 2000 / portTICK_PERIOD_MS;

	for (;;) {
		// aguarda semáforo com timeout de  500 ms
		if( xSemaphoreTake(xSemaphore2, ( TickType_t ) 500 / portTICK_PERIOD_MS) == pdTRUE ){
			pin_toggle(LED2_PIO, LED2_PIO_IDX_MASK);
		} else {
			// time out
		}
	}
}

static void task_led3(void *pvParameters) {
	configure_pio_output(LED3_PIO, LED3_PIO_IDX_MASK, LED3_PIO_ID, 1);
	BTN_init_with_IRQ(BUT3_PIO, BUT3_PIO_ID, BUT3_PIO_IDX_MASK, but3_callback);
	const TickType_t xDelay = 2000 / portTICK_PERIOD_MS;

	for (;;) {
		// aguarda semáforo com timeout de  500 ms
		if( xSemaphoreTake(xSemaphore3, ( TickType_t ) 500 / portTICK_PERIOD_MS) == pdTRUE ){
			pin_toggle(LED3_PIO, LED3_PIO_IDX_MASK);
			} else {
			// time out
		}
	}
}

//static void task_led1(void *pvParameters){
	//LED1_init(1);
	//const TickType_t xDelay = 3000 / portTICK_PERIOD_MS;
//
	//for(;;){
		////pin_toggle(LED1_PIO, LED1_PIO_IDX_MASK);
		//pisca_led(LED1_PIO, LED1_PIO_IDX_MASK, 3, 100);
		//vTaskDelay(xDelay);
	//}
	//
//}

/************************************************************************/
/* main                                                                */
/************************************************************************/

int main(void) {
	sysclk_init();
	board_init();

	/* Initialize the console uart */
	configure_console();

	/* Output demo information. */
	printf("-- Freertos Example --\n\r");
	printf("-- %s\n\r", BOARD_NAME);
	printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);


	/*
     We are using the semaphore for synchronisation so we create a binary
    * semaphore rather than a mutex.  We must make sure that the interrupt
    * does not attempt to use the semaphore before it is created!
    */
	xSemaphore = xSemaphoreCreateBinary();
	xSemaphore2 = xSemaphoreCreateBinary();
	xSemaphore3 = xSemaphoreCreateBinary();
 
	if (xSemaphore == NULL)
		printf("Falha em criar o semaforo \n");
	if (xSemaphore2 == NULL)
		printf("Falha em criar o semaforo \n");
	if (xSemaphore3 == NULL)
		printf("Falha em criar o semaforo \n");

	/* Create task to monitor processor activity */
	if (xTaskCreate(task_monitor, "Monitor", TASK_MONITOR_STACK_SIZE, NULL,
	TASK_MONITOR_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create Monitor task\r\n");
	}

	/* Create task to make led blink */
	if (xTaskCreate(task_led, "Led", TASK_LED_STACK_SIZE, NULL,
	TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test led task\r\n");
	}
	
	xTaskCreate(task_led1, "Led1", TASK_LED1_STACK_SIZE, NULL, TASK_LED1_PRIORITY, NULL);
	xTaskCreate(task_led2, "Led2", TASK_LED2_STACK_SIZE, NULL, TASK_LED2_PRIORITY, NULL);
	xTaskCreate(task_led3, "Led3", TASK_LED3_STACK_SIZE, NULL, TASK_LED3_PRIORITY, NULL);


	/* Start the scheduler. */
	vTaskStartScheduler();
	
	while(1){
	}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}