#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"


// Definindo tudo do bot?o 1:
#define BUT1_PIO PIOD
#define BUT1_PIO_ID ID_PIOD
#define BUT1_PIO_IDX 28
#define BUT1_PIO_IDX_MASK (1u << BUT1_PIO_IDX) // esse j? est? pronto.

// Definindo tudo do LED 1:
#define LED1_PIO           PIOA                 // periferico que controla o LED
// #
#define LED1_PIO_ID        ID_PIOA                 // ID do perif?rico PIOC (controla LED)
#define LED1_PIO_IDX       0                    // ID do LED no PIO
#define LED1_PIO_IDX_MASK  (1 << LED1_PIO_IDX)   // Mascara para CONTROLARMOS o LED

// Definindo tudo do bot?o 2:
#define BUT2_PIO PIOC
#define BUT2_PIO_ID ID_PIOC
#define BUT2_PIO_IDX 31
#define BUT2_PIO_IDX_MASK (1u << BUT2_PIO_IDX) // esse j? est? pronto.

// Definindo tudo do LED 2:
#define LED2_PIO           PIOC                 // periferico que controla o LED
// #
#define LED2_PIO_ID        ID_PIOC                 // ID do perif?rico PIOC (controla LED)
#define LED2_PIO_IDX       30                    // ID do LED no PIO
#define LED2_PIO_IDX_MASK  (1 << LED2_PIO_IDX)   // Mascara para CONTROLARMOS o LED

// Definindo tudo do bot?o 3:
#define BUT3_PIO PIOA
#define BUT3_PIO_ID ID_PIOA
#define BUT3_PIO_IDX 19
#define BUT3_PIO_IDX_MASK (1u << BUT3_PIO_IDX) // esse j? est? pronto.

// Definindo tudo do LED 3:
#define LED3_PIO           PIOB                 // periferico que controla o LED
// #
#define LED3_PIO_ID        ID_PIOB                 // ID do perif?rico PIOC (controla LED)
#define LED3_PIO_IDX       2                    // ID do LED no PIO
#define LED3_PIO_IDX_MASK  (1 << LED3_PIO_IDX)   // Mascara para CONTROLARMOS o LED


volatile char but1_flag; // vari?vel global
volatile char but2_flag; // vari?vel global
volatile char but3_flag; // vari?vel global

volatile int delay;

void io_init(void);
void pisca_led(int n, int t);


void but1_callback(void)
{
	but1_flag = 1;
}

void but2_callback(void)
{
	but2_flag = 1;
}

void but3_callback(void)
{
	but3_flag = 1;
}


/************************************************************************/
/* fun??es                                                              */
/************************************************************************/



// pisca led N vez no periodo T
void pisca_led(int n, int t){
	int x = 80;
	int y = 16;
	gfx_mono_generic_draw_filled_rect(x+1, y+1, 31, 9, GFX_PIXEL_CLR);
	gfx_mono_generic_draw_rect(x, y, 32, 10, GFX_PIXEL_SET);
	for (int i=1;i<=n;){
		pio_clear(LED2_PIO, LED2_PIO_IDX_MASK);
		gfx_mono_generic_draw_vertical_line(x+i, 16, 10, GFX_PIXEL_SET);
		delay_ms(t);
		pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
		delay_ms(t);
		i++;
		if (but2_flag){
			delay = delay;
			delay_ms(200);
			return;
		}
	}
}



// Inicializa botao SW0 do kit com interrupcao
void io_init(void)
{

	// Configura led
	pmc_enable_periph_clk(LED2_PIO_ID);
	pio_configure(LED2_PIO, PIO_OUTPUT_0, LED2_PIO_IDX_MASK, PIO_DEFAULT);

	// Inicializa clock do perif?rico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);

	// Configura PIO para lidar com o pino do bot?o como entrada
	// com pull-up
	//pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_PIO, BUT1_PIO_IDX_MASK, 60);

	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT2_PIO, BUT2_PIO_IDX_MASK, 60);
	
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT3_PIO, BUT3_PIO_IDX_MASK, 60);

	// Configura interrup??o no pino referente ao botao e associa
	// fun??o de callback caso uma interrup??o for gerada
	// a fun??o de callback ? a: but_callback()
	pio_handler_set(BUT1_PIO,
	BUT1_PIO_ID,
	BUT1_PIO_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but1_callback);

	pio_handler_set(BUT2_PIO,
	BUT2_PIO_ID,
	BUT2_PIO_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but2_callback);
	
	pio_handler_set(BUT3_PIO,
	BUT3_PIO_ID,
	BUT3_PIO_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but3_callback);

	//PIO_IT_RISE_EDGE, PIO_IT_FALL_EDGE
	// Ativa interrup??o e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);

	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT2_PIO);
	
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT3_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais pr?ximo de 0 maior)
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 4); // Prioridade 4
}

void draw_frequency(int delay){
	char freq_str[20];
	sprintf(freq_str, "%d ms", delay);
	gfx_mono_draw_string(freq_str, 5,16, &sysfont);
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
void main(void)
{
	// Inicializa clock
	board_init();
	sysclk_init();
	delay_init();

	// Init OLED
	gfx_mono_ssd1306_init();

	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

	// configura botao com interrupcao
	io_init();
	
	delay = 500;
	draw_frequency(delay);
	// aplicacoes embarcadas no devem sair do while(1).
	
	while(1)
	{
		if (but1_flag || but3_flag || but2_flag){  // se qualquer bot?o for apertado, ele come?a a fazer as confer?ncias.
			for (int i = 0; i < 10000000; i++){
				if (but3_flag){
					delay += 100; 
					delay_ms(300);
					draw_frequency(delay);
					break;
				}
				if (pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK)){
					if((!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) && i >= 1000000){
						delay+= 100;
						delay_ms(300);
						draw_frequency(delay);
						break;
					} else if (pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK) && i < 1000000 ){
						delay -= 100;
						delay_ms(200);
						draw_frequency(delay);
						break;
					}
					
				}
				else{
					delay_ms(200);
					but2_flag = 0;
					pisca_led(30, delay);
					break;
				}
				
			}
			
			draw_frequency(delay);
			but1_flag = 0;
			but2_flag = 0;
			but3_flag = 0;
			
		}
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI); //utilizar somente o modo sleep mode
		
	}

}