#include <asf.h>
#include <time.h>
#include <string.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

#define BUT1_PIO			PIOD
#define BUT1_PIO_ID			ID_PIOD
#define BUT1_PIO_IDX		28
#define BUT1_PIO_IDX_MASK	(1u << BUT1_PIO_IDX) // esse j? est? pronto.

#define TRIG_PIO				PIOD
#define TRIG_PIO_ID			ID_PIOD
#define TRIG_PIO_IDX			30
#define TRIG_PIO_IDX_MASK	(1 << TRIG_PIO_IDX)

#define ECHO_PIO				PIOA
#define ECHO_PIO_ID			ID_PIOA
#define ECHO_PIO_IDX			6
#define ECHO_PIO_IDX_MASK	(1 << ECHO_PIO_IDX)

volatile char echo_flag = 0; // vari�vel global
volatile char rtt_alarm = 0;
volatile float freq = (float) 1/(0.000058*2);
volatile int tempo = 0;
volatile char but1_flag = 0;
volatile int i = 0;
volatile j = 26;
volatile int medicoes = 0;


static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);


void but1_callback(void){
	but1_flag = 1;
}



void echo_callback(void)
{	
	float t_alarme = 4/340;
	if (echo_flag == 0){
		echo_flag = 1;
		RTT_init(8620, 11*8620, RTT_MR_ALMIEN);
	} else{
		echo_flag = 0;
		tempo = rtt_read_timer_value(RTT);
	}
}


void TC1_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrup��o foi satisfeita.
	* Isso � realizado pela leitura do status do perif�rico
	**/
	volatile uint32_t status = tc_get_status(TC0, 1);

	/** Muda o estado do LED (pisca) **/
	i++;
}


void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  freq hz e interrup�c�o no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura NVIC*/
	NVIC_SetPriority(ID_TC, 4);
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}


static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT));
		rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	
}



void RTT_Handler(void) {
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		//RTT_init(4, 0, RTT_MR_RTTINCIEN);
		rtt_alarm = 1;
	}
	
	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {

	}

}


static float get_time_rtt(){
	uint ul_previous_time = rtt_read_timer_value(RTT);
}



void configure_pio_output(Pio *pio, const uint32_t ul_mask, uint32_t ul_id){
	pmc_enable_periph_clk(ul_id);
	pio_set_output(pio, ul_mask, 0, 0, 0);
}

void configure_pio_input(Pio *pio, const pio_type_t ul_type, const uint32_t ul_mask, const uint32_t ul_attribute, uint32_t ul_id){
	pmc_enable_periph_clk(ul_id);
	pio_configure(pio, ul_type, ul_mask, ul_attribute);
	pio_set_debounce_filter(pio, ul_mask, 60);
}

void configure_interruption(Pio *pio, uint32_t ul_id, const uint32_t ul_mask,  uint32_t ul_attr, void (*p_handler) (uint32_t, uint32_t), uint32_t priority){
	pio_handler_set(pio, ul_id, ul_mask , ul_attr, p_handler);
	pio_enable_interrupt(pio, ul_mask);
	pio_get_interrupt_status(pio);
	NVIC_EnableIRQ(ul_id);
	NVIC_SetPriority(ul_id, priority);
}

void io_init(void)
{
	configure_pio_input(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP|PIO_DEBOUNCE, BUT1_PIO_ID);
	configure_interruption(BUT1_PIO, BUT1_PIO_ID, BUT1_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but1_callback, 2);
	
	// Configura trig como se fosse LED
	configure_pio_output(TRIG_PIO, TRIG_PIO_IDX_MASK, TRIG_PIO_ID);

	// Configura echo como se fosse um bot�o

	//configure_pio_input(ECHO_PIO, PIO_INPUT, ECHO_PIO_IDX_MASK, PIO_DEBOUNCE, ECHO_PIO_ID);
	pmc_enable_periph_clk(ECHO_PIO_ID);
	pio_configure(ECHO_PIO, PIO_INPUT, ECHO_PIO_IDX_MASK, PIO_DEBOUNCE);
	pio_set_debounce_filter(ECHO_PIO_IDX, ECHO_PIO_IDX_MASK, 60);
	
	configure_interruption(ECHO_PIO, ECHO_PIO_ID, ECHO_PIO_IDX_MASK, PIO_IT_EDGE, echo_callback, 1);

}


void draw (int ms){
	char string[20];
	float t = (float) ms/freq;
	float distancia = (340*t*100.0)/2.0;
	float distancia_maior = distancia + 0.3;
	float distancia_menor = distancia - 0.3;
	

	//(distancia >= 400)
	if (rtt_alarm || (distancia >= 400)){
		i = 0;
		gfx_mono_generic_draw_filled_rect(0, 0, 127, 31, GFX_PIXEL_CLR);
		sprintf(string, "Erro!");
		gfx_mono_draw_string(string, 0,0, &sysfont);
		rtt_alarm = 0;
		delay_ms(200);
		gfx_mono_generic_draw_filled_rect(0, 0, 127, 31, GFX_PIXEL_CLR);
	} else{
		//128x32 pixels		
		j = 26;
		i+=22;
		
		medicoes+=1;


		if (i >= 70 || medicoes > 3){
			gfx_mono_generic_draw_filled_rect(0, 0, 127, 31, GFX_PIXEL_CLR);
			i = 0;
			medicoes = 0;
		}
		
		gfx_mono_generic_draw_rect(5, 5, 75, 26, GFX_PIXEL_SET);
		gfx_mono_draw_line(0, j-2, 1, j-2, GFX_PIXEL_SET);
		gfx_mono_draw_line(0, j-8, 1, j-8, GFX_PIXEL_SET);
		gfx_mono_draw_line(0, j-14, 1, j-14, GFX_PIXEL_SET);

		//gfx_mono_generic_draw_rect(5, 5, 75, 26, GFX_PIXEL_SET);
		// no total 21 pixels para aproveitar
		int range = 400/7;
		// nos 3 primeiros pixels de 0 at� 57 cm, depois de 57 at� 114;
		if (distancia <= 100){
			j-=0; // no meio dos limites
			gfx_mono_draw_filled_circle(i, j, 1, GFX_PIXEL_SET, GFX_WHOLE);
		} else if (distancia > 100 && distancia <= 200){
			j-= 5;
			gfx_mono_draw_filled_circle(i, j, 1, GFX_PIXEL_SET, GFX_WHOLE);
		} else if (distancia > 200 && distancia < 300){
			j-=11;
			gfx_mono_draw_filled_circle(i, j,  1, GFX_PIXEL_SET, GFX_WHOLE);
		} else if (distancia >= 300 && distancia < 400){
			j-=15;
			gfx_mono_draw_filled_circle(i, j, 1, GFX_PIXEL_SET, GFX_WHOLE);
		}
		
		//medicoes+=1;

		}
		sprintf(string, "%2.1f", distancia);
		gfx_mono_generic_draw_filled_rect(80, 9, 127, 31, GFX_PIXEL_CLR);
		gfx_mono_draw_string(string, 80,9, &sysfont);		
		//tc_stop(TC1, 1);
		
	}
	


void trig_pulse(){
	pio_set(TRIG_PIO, TRIG_PIO_IDX_MASK);
	delay_us(10);
	pio_clear(TRIG_PIO, TRIG_PIO_IDX_MASK);
}

int main (void)
{
	sysclk_init();

	io_init();

	board_init();
	delay_init();
	
	
	WDT->WDT_MR = WDT_MR_WDDIS;


  // Init OLED
	gfx_mono_ssd1306_init();


	
	while(1) {
		if (but1_flag){
			//TC_init(TC0, ID_TC1, 1, 1);
			//tc_start(TC0, 1);
			but1_flag = 0;
			trig_pulse();		
			draw(tempo);	

		}
		
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);

		
	}  
		
		
}
	

			