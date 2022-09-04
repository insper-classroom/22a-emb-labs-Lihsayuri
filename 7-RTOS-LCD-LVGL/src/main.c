/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include <asf.h>
#include <string.h>
#include "ili9341.h"
#include "lvgl.h"
#include "touch/touch.h"
#include "clock.h"
#include "steam.h"


LV_FONT_DECLARE(dseg70);
LV_FONT_DECLARE(dseg50);
LV_FONT_DECLARE(dseg35);
LV_FONT_DECLARE(dseg20);

/************************************************************************/
/* LCD / LVGL                                                           */
/************************************************************************/

#define LV_HOR_RES_MAX          (320)
#define LV_VER_RES_MAX          (240)

/*A static or global variable to store the buffers*/
static lv_disp_draw_buf_t disp_buf;

/*Static or global buffer(s). The second buffer is optional*/
static lv_color_t buf_1[LV_HOR_RES_MAX * LV_VER_RES_MAX];
static lv_disp_drv_t disp_drv;          /*A variable to hold the drivers. Must be static or global.*/
static lv_indev_drv_t indev_drv;

static  lv_obj_t * labelBtn1;
static lv_obj_t * labelBtn2;
static lv_obj_t * labelBtn3;
static lv_obj_t * labelBtn4;
static lv_obj_t * labelBtn5;
static lv_obj_t * labelFloor;
static lv_obj_t * labelSetValue;
static 	lv_obj_t * labelClock;
static 	lv_obj_t * labelHome;
static 	lv_obj_t * labelftemp;

volatile int clk_clicked = 0;
volatile char desliga = 0;

typedef struct  {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} calendar;


SemaphoreHandle_t xSemaphoreRTC;
 
/************************************************************************/
/* RTOS                                                                 */
/************************************************************************/

#define TASK_LCD_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_LCD_STACK_PRIORITY            (tskIDLE_PRIORITY)

#define TASK_RTC_STACK_SIZE (4096 / sizeof(portSTACK_TYPE))
#define TASK_RTC_STACK_PRIORITY (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type);
void RTC_Handler(void);

volatile int rtc_seconds = 0;
volatile uint32_t current_hour, current_min, current_sec;
volatile uint32_t current_year, current_month, current_day, current_week;

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) {	}
}

extern void vApplicationIdleHook(void) { }

extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	configASSERT( ( volatile void * ) NULL );
}

/************************************************************************/
/* lvgl                                                                 */
/************************************************************************/

static void event_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		if (desliga == 0){
			desliga = 1;
		}
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}

static void menu_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}

static void clk_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
		if (clk_clicked == 0){
			clk_clicked = 1;
		} else {
			clk_clicked = 0;
		}
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}


static void up_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	char *c;
	double temp;

	if(code == LV_EVENT_CLICKED) {
		if (clk_clicked){
			current_min+=1;
			if (current_min == 60){
				current_hour+=1;
				current_min = 0;
				if (current_hour == 24){
					current_hour = 0;
				} 
			}
			rtc_set_time(RTC, current_hour, current_min, current_sec);			
		} else {
			c = lv_label_get_text(labelSetValue);
			temp = atoi(c);
			temp += 1;
			lv_label_set_text_fmt(labelSetValue, "%.1f", temp);
		}

	}

}

static void down_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	char *c;
	double temp;

	if(code == LV_EVENT_CLICKED) {
		if (clk_clicked){
			current_min-=1;
			if (current_min == -1){
				current_hour-=1;
				current_min = 59;
				if (current_hour == -1){
					current_hour = 23;
				}
			}
			rtc_set_time(RTC, current_hour, current_min, current_sec);
		} else {
			c = lv_label_get_text(labelSetValue);
			temp = atoi(c);
			temp -= 1;
			lv_label_set_text_fmt(labelSetValue, "%.1f", temp);
		}
	}

}


void config_button(lv_palette_t pallete, lv_coord_t value, lv_obj_t * btn, lv_obj_t * reference,  lv_align_t align, lv_coord_t w, lv_coord_t h, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_event_cb_t event_cb){
	static lv_style_t style;
	lv_style_init(&style);
	lv_style_set_bg_color(&style, lv_palette_main(pallete));
	lv_style_set_border_color(&style, lv_palette_main(pallete));
	lv_style_set_border_width(&style, value);
	
	lv_obj_add_event_cb(btn, event_cb, LV_EVENT_ALL, NULL);
	lv_obj_align_to(btn, reference, align, x_ofs, y_ofs);
	//lv_obj_align(btn, align, 0, 0);
	lv_obj_add_style(btn, &style, 0);
	
	lv_obj_set_width(btn, w);
	lv_obj_set_height(btn, h);
}



void lv_termostato(void) {
	static lv_style_t style;
	lv_style_init(&style);
	lv_style_set_bg_color(&style, lv_palette_main(LV_PALETTE_NONE));
	lv_style_set_border_color(&style, lv_palette_main(LV_PALETTE_NONE));
	lv_style_set_border_width(&style, 5);

	lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
	lv_obj_align(btn1, LV_ALIGN_BOTTOM_LEFT, 0, 0);
	lv_obj_add_style(btn1, &style, 0);
		
	lv_obj_set_width(btn1, 60);
	lv_obj_set_height(btn1, 60);
		
	lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
	config_button(LV_PALETTE_NONE, 5, btn2, btn1, LV_ALIGN_BOTTOM_LEFT, 60, 60, 35, -30, menu_handler);
		
	//-------------------------------------------------------------------------------------------------

	LV_IMG_DECLARE(clock);

	/*Darken the button when pressed and make it wider*/
	static lv_style_t style_pr;
	lv_style_init(&style_pr);
	lv_style_set_img_recolor_opa(&style_pr, LV_OPA_30);
	lv_style_set_img_recolor(&style_pr, lv_color_black());
	lv_style_set_transform_width(&style_pr, 10);
	lv_style_set_width(&style_pr, 20);
	lv_style_set_height(&style_pr, 20);

	/*Create an image button*/
	lv_obj_t * btn3 = lv_imgbtn_create(lv_scr_act());
	lv_obj_add_event_cb(btn3, clk_handler, LV_EVENT_ALL, NULL);
	lv_imgbtn_set_src(btn3, LV_IMGBTN_STATE_RELEASED, &clock, NULL, NULL);
	lv_obj_add_style(btn3, &style_pr, LV_STATE_PRESSED);
		
	lv_obj_align_to(btn3, btn2, LV_ALIGN_BOTTOM_LEFT, 50, 95);


	//----------------------------------------------------------------------------------------
	
	LV_IMG_DECLARE(steam);

	/*Darken the button when pressed and make it wider*/
	static lv_style_t style_pr2;
	lv_style_init(&style_pr2);
	lv_style_set_img_recolor_opa(&style_pr2, LV_OPA_30);
	lv_style_set_img_recolor(&style_pr2, lv_color_black());
	lv_style_set_transform_width(&style_pr2, 10);
	lv_style_set_width(&style_pr2, 20);
	lv_style_set_height(&style_pr2, 20);

	/*Create an image button*/
	
	lv_obj_t * clock_logo = lv_imgbtn_create(lv_scr_act());
	lv_obj_add_event_cb(clock_logo, clk_handler, LV_EVENT_ALL, NULL);
	lv_imgbtn_set_src(clock_logo, LV_IMGBTN_STATE_RELEASED, &clock, NULL, NULL);
	lv_obj_add_style(clock_logo, &style_pr, LV_STATE_PRESSED);
	
	lv_obj_align(clock_logo, LV_ALIGN_BOTTOM_RIGHT, 0, 10);
	
	lv_obj_t * steam_logo = lv_imgbtn_create(lv_scr_act());
	lv_obj_add_event_cb(steam_logo, clk_handler, LV_EVENT_ALL, NULL);
	lv_imgbtn_set_src(steam_logo, LV_IMGBTN_STATE_RELEASED, &steam, NULL, NULL);
	lv_obj_add_style(steam_logo, &style_pr2, LV_STATE_PRESSED);
	
	lv_obj_align_to(steam_logo, clock_logo, LV_ALIGN_BOTTOM_RIGHT, 50, 5);
	
	
	labelHome = lv_label_create(lv_scr_act());
	lv_obj_align(labelHome, LV_ALIGN_RIGHT_MID, -150 , 50);
	lv_label_set_text(labelHome, LV_SYMBOL_HOME);
	lv_obj_set_style_text_color(labelHome, lv_color_white(), LV_STATE_DEFAULT);
	
	
	
	//-----------------------------------------------------------------------------------------
		
	lv_obj_t * btn4 = lv_btn_create(lv_scr_act());
	config_button(LV_PALETTE_NONE, 5, btn4, btn2, LV_ALIGN_BOTTOM_LEFT, 60, 60, 110, -30, up_handler);

	lv_obj_t * btn5 = lv_btn_create(lv_scr_act());
	config_button(LV_PALETTE_NONE, 5, btn5, btn4, LV_ALIGN_BOTTOM_LEFT, 60, 60, 50, -30, down_handler);

	labelBtn1 = lv_label_create(btn1);
	lv_label_set_text(labelBtn1, "[  " LV_SYMBOL_POWER);
	lv_obj_center(labelBtn1);

	labelBtn2 = lv_label_create(btn2);
	lv_label_set_text(labelBtn2, "|  M");
	lv_obj_center(labelBtn2);
		

	labelBtn4 = lv_label_create(btn4);
	lv_label_set_text(labelBtn4, "[  " LV_SYMBOL_UP);
	lv_obj_center(labelBtn4);
		
	labelBtn5 = lv_label_create(btn5);
	lv_label_set_text(labelBtn5, LV_SYMBOL_DOWN "  ]");
	lv_obj_center(labelBtn5);
		
	labelFloor = lv_label_create(lv_scr_act());
	lv_obj_align(labelFloor, LV_ALIGN_LEFT_MID, 50 , -30);
	lv_obj_set_style_text_font(labelFloor, &dseg50, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelFloor, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelFloor, "%02d", 23);

		
	labelSetValue = lv_label_create(lv_scr_act());
	lv_obj_align(labelSetValue, LV_ALIGN_RIGHT_MID, -30 , -40);
	lv_obj_set_style_text_font(labelSetValue, &dseg50, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelSetValue, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelSetValue, "%.1f", 22.0);
		
	labelClock = lv_label_create(lv_scr_act());
	lv_obj_align(labelClock, LV_ALIGN_RIGHT_MID, -20 , -100);
	lv_obj_set_style_text_font(labelClock, &dseg35, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelClock, lv_color_white(), LV_STATE_DEFAULT);
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/



static void task_RTC(void *pvParameters) {
	calendar rtc_initial = {2018, 3, 19, 12, 15, 45 ,1};
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_SR_SEC|RTC_SR_ALARM);

	//xSemaphoreTake(xSemaphoreRTC, 1000
	for (;;) {
		rtc_get_time(RTC, &current_hour,&current_min, &current_sec);

		/* aguarda por tempo inderteminado até a liberacao do semaforo */
		if (xSemaphoreTake(xSemaphoreRTC, 1000 / portTICK_PERIOD_MS)){
			lv_label_set_text_fmt(labelClock, "%02d:%02d", current_hour, current_min);
		} else {
			lv_label_set_text_fmt(labelClock, "%02d %02d", current_hour, current_min);
		}
			
	}
	
}


static void task_lcd(void *pvParameters) {
	int px, py;
	
	//lv_ex_btn_1();
	lv_termostato();


	for (;;)  {
		if (desliga){
			lv_obj_clean(lv_scr_act());
		} 
		lv_tick_inc(50);
		lv_task_handler();
		vTaskDelay(50);
	}
}


/************************************************************************/
/* configs                                                              */
/************************************************************************/

void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);
	
	/* seccond tick */
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(xSemaphoreRTC, &xHigherPriorityTaskWoken);
		
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		// o código para irq de alame vem aqui
	}

	rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}


void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type) {
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(rtc, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.second);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 4);
	NVIC_EnableIRQ(id_rtc);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(rtc,  irq_type);
}

static void configure_lcd(void) {
	/**LCD pin configure on SPI*/
	pio_configure_pin(LCD_SPI_MISO_PIO, LCD_SPI_MISO_FLAGS);  //
	pio_configure_pin(LCD_SPI_MOSI_PIO, LCD_SPI_MOSI_FLAGS);
	pio_configure_pin(LCD_SPI_SPCK_PIO, LCD_SPI_SPCK_FLAGS);
	pio_configure_pin(LCD_SPI_NPCS_PIO, LCD_SPI_NPCS_FLAGS);
	pio_configure_pin(LCD_SPI_RESET_PIO, LCD_SPI_RESET_FLAGS);
	pio_configure_pin(LCD_SPI_CDS_PIO, LCD_SPI_CDS_FLAGS);
	
	ili9341_init();
	ili9341_backlight_on();
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT,
	};

	/* Configure console UART. */
	stdio_serial_init(CONSOLE_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	setbuf(stdout, NULL);
}



/************************************************************************/
/* port lvgl                                                            */
/************************************************************************/

void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
	ili9341_set_top_left_limit(area->x1, area->y1);   ili9341_set_bottom_right_limit(area->x2, area->y2);
	ili9341_copy_pixels_to_screen(color_p,  (area->x2 + 1 - area->x1) * (area->y2 + 1 - area->y1));
	
	/* IMPORTANT!!!
	* Inform the graphics library that you are ready with the flushing*/
	lv_disp_flush_ready(disp_drv);
}

void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data) {
	int px, py, pressed;
	
	if (readPoint(&px, &py))
		data->state = LV_INDEV_STATE_PRESSED;
	else
		data->state = LV_INDEV_STATE_RELEASED; 
	
	data->point.x = px;
	data->point.y = py;
}

void configure_lvgl(void) {
	lv_init();
	lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);
	
	lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
	disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
	disp_drv.flush_cb = my_flush_cb;        /*Set a flush callback to draw to the display*/
	disp_drv.hor_res = LV_HOR_RES_MAX;      /*Set the horizontal resolution in pixels*/
	disp_drv.ver_res = LV_VER_RES_MAX;      /*Set the vertical resolution in pixels*/

	lv_disp_t * disp;
	disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
	
	/* Init input on LVGL */
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = my_input_read;
	lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv);
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/
int main(void) {
	/* board and sys init */
	board_init();
	sysclk_init();
	configure_console();

	/* LCd, touch and lvgl init*/
	configure_lcd();
	configure_touch();
	configure_lvgl();
	
	xSemaphoreRTC = xSemaphoreCreateBinary();
	if (xSemaphoreRTC == NULL)
	printf("falha em criar o semaforo \n");

	/* Create task to control oled */
	if (xTaskCreate(task_lcd, "LCD", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create lcd task\r\n");
	}
	
	if (xTaskCreate(task_RTC, "RTC", TASK_RTC_STACK_SIZE, NULL, TASK_RTC_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test led task\r\n");
	}
	

	/* Start the scheduler. */
	vTaskStartScheduler();

	while(1){ }
}
