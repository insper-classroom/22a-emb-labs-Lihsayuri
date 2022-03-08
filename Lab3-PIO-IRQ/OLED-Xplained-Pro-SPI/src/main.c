#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"


// Definindo tudo do botão 1:
#define BUT1_PIO PIOD
#define BUT1_PIO_ID ID_PIOD
#define BUT1_PIO_IDX 28
#define BUT1_PIO_IDX_MASK (1u << BUT1_PIO_IDX) // esse já está pronto.

// Definindo tudo do LED 1:
#define LED1_PIO           PIOA                 // periferico que controla o LED
// #
#define LED1_PIO_ID        ID_PIOA                 // ID do periférico PIOC (controla LED)
#define LED1_PIO_IDX       0                    // ID do LED no PIO
#define LED1_PIO_IDX_MASK  (1 << LED1_PIO_IDX)   // Mascara para CONTROLARMOS o LED

// Definindo tudo do botão 2:
#define BUT2_PIO PIOC
#define BUT2_PIO_ID ID_PIOC
#define BUT2_PIO_IDX 31
#define BUT2_PIO_IDX_MASK (1u << BUT2_PIO_IDX) // esse já está pronto.

// Definindo tudo do LED 2:
#define LED2_PIO           PIOC                 // periferico que controla o LED
// #
#define LED2_PIO_ID        ID_PIOC                 // ID do periférico PIOC (controla LED)
#define LED2_PIO_IDX       30                    // ID do LED no PIO
#define LED2_PIO_IDX_MASK  (1 << LED2_PIO_IDX)   // Mascara para CONTROLARMOS o LED

// Definindo tudo do botão 3:
#define BUT3_PIO PIOA
#define BUT3_PIO_ID ID_PIOA
#define BUT3_PIO_IDX 19
#define BUT3_PIO_IDX_MASK (1u << BUT3_PIO_IDX) // esse já está pronto.

// Definindo tudo do LED 3:
#define LED3_PIO           PIOB                 // periferico que controla o LED
// #
#define LED3_PIO_ID        ID_PIOB                 // ID do periférico PIOC (controla LED)
#define LED3_PIO_IDX       2                    // ID do LED no PIO
#define LED3_PIO_IDX_MASK  (1 << LED3_PIO_IDX)   // Mascara para CONTROLARMOS o LED


volatile char but_flag; // variável global
volatile int frequencia;
volatile int but1_pressed;

void io_init(void);
void pisca_led(int n, int t);


void but_callback(void)
{
	but_flag = 1;
}

/************************************************************************/
/* funções                                                              */
/************************************************************************/



// pisca led N vez no periodo T
void pisca_led(int n, int t){
	int x = 80;
	int y = 16;
	gfx_mono_generic_draw_filled_rect(x+1, y+1, 31, 9, GFX_PIXEL_CLR);
	gfx_mono_generic_draw_rect(x, y, 32, 10, GFX_PIXEL_SET);
	for (int i=1;i<=n;){
		if (but1_pressed){
			pio_clear(LED2_PIO, LED2_PIO_IDX_MASK);
			gfx_mono_generic_draw_vertical_line(x+i, 16, 10, GFX_PIXEL_SET);
			delay_ms(t);
			pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
			delay_ms(t);
			i++;
			if(!pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK)){
				but1_pressed = 0;
				frequencia = frequencia;
				delay_ms(700);
				return;
			}
		}
		if(!pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK)){
			but1_pressed = 0;
			frequencia = frequencia;
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

	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	//pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_PIO, BUT1_PIO_IDX_MASK, 60);

	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT2_PIO, BUT2_PIO_IDX_MASK, 60);
	
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT3_PIO, BUT3_PIO_IDX_MASK, 60);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT1_PIO,
	BUT1_PIO_ID,
	BUT1_PIO_IDX_MASK,
	PIO_IT_EDGE,
	but_callback);

	pio_handler_set(BUT2_PIO,
	BUT2_PIO_ID,
	BUT2_PIO_IDX_MASK,
	PIO_IT_EDGE,
	but_callback);
	
	pio_handler_set(BUT3_PIO,
	BUT3_PIO_ID,
	BUT3_PIO_IDX_MASK,
	PIO_IT_EDGE,
	but_callback);

	//PIO_IT_RISE_EDGE, PIO_IT_FALL_EDGE
	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);

	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT2_PIO);
	
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT3_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 4); // Prioridade 4
}

void draw_frequency(frequencia){
	char freq_str[20];
	sprintf(freq_str, "%d ms", frequencia);
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
	// Escreve na tela um circulo e um texto
	//gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	//gfx_mono_draw_string("Yay :)", 50,16, &sysfont);


	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

	// configura botao com interrupcao
	io_init();
	
	frequencia = 500;
	draw_frequency(frequencia);
	// super loop
	// aplicacoes embarcadas no devem sair do while(1).
	int but2_pressed = 0;
	
	while(1)
	{
		if (but_flag){
			//while((!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) || (!pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK)) || (!pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK)) ){
				for (int i = 0; i < 10000000; i++){
					if (!pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK) && i < 1000000){
						frequencia-=100;
						delay_ms(200);
						draw_frequency(frequencia);
						break;
					} else if((!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) && i >= 1000000 && pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK) ){
						frequencia+=100;
						delay_ms(700);
						draw_frequency(frequencia);
						break;
					} else if ((pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) && i < 1000000 && pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK) && pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK)){
						frequencia-=100;
						delay_ms(200);
						draw_frequency(frequencia);
						break;
					} else if (!pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK)){
						but1_pressed = 1;
						delay_ms(200);
						pisca_led(30, frequencia);
						break;
					}
				
				}
				
				
				draw_frequency(frequencia);
			//}
			
			//but_flag = 0;
		}
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI); //utilizar somente o modo sleep mode
	}
}


