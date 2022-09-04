/**
* 5 semestre - Eng. da Computação - Insper
* Rafael Corsi - rafael.corsi@insper.edu.br
*
* Projeto 0 para a placa SAME70-XPLD
*
* Objetivo :
*  - Introduzir ASF e HAL
*  - Configuracao de clock
*  - Configuracao pino In/Out
*
* Material :
*  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
*/


#include "asf.h"

//defines                                                              


//Definindo tudo do LED da placa: 
#define LED_PIO           PIOC                 // periferico que controla o LED
// #
#define LED_PIO_ID        ID_PIOC                 // ID do periférico PIOC (controla LED)
#define LED_PIO_IDX       8                    // ID do LED no PIO
#define LED_PIO_IDX_MASK  (1 << LED_PIO_IDX)   // Mascara para CONTROLARMOS o LED

// Definindo tudo do botão SW300 da placa:
#define BUT_PIO PIOA
#define BUT_PIO_ID ID_PIOA
#define BUT_PIO_IDX 11
#define BUT_PIO_IDX_MASK (1u << BUT_PIO_IDX) // esse já está pronto.

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

#define _PIO_DEFAULT             (0u << 0)
/*  The internal pin pull-up is active. */
#define _PIO_PULLUP              (1u << 0)
/*  The internal glitch filter is active. */
#define _PIO_DEGLITCH            (1u << 1)
/*  The internal debouncing filter is active. */
#define _PIO_DEBOUNCE            (1u << 3)


void init(void);

void _pio_set(Pio *p_pio, const uint32_t ul_mask)
{
	 p_pio->PIO_SODR = ul_mask;
}


void _pio_clear(Pio *p_pio, const uint32_t ul_mask)
{
	p_pio->PIO_CODR = ul_mask;
}


void _pio_pull_up(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_pull_up_enable)
{
	if (ul_pull_up_enable){
		p_pio -> PIO_PUER = ul_mask;
	} else{
		p_pio ->PIO_PUDR = ul_mask;
	}

 }

void _pio_set_input(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_attribute)
{	
	_pio_pull_up(p_pio, ul_mask, (ul_attribute & _PIO_PULLUP) );
	
	if (ul_attribute & PIO_DEGLITCH) {
		p_pio->PIO_IFSCDR = ul_mask; // quando tá disable, ou seja, IFSCSR é 0, ativa o deglitch
	} else if (ul_attribute & PIO_DEBOUNCE) { //quando tá enable, ou seja, IFSCSR é 1, ativa o debounce
				p_pio->PIO_IFSCER = ul_mask;
		}
		
		
	if (ul_attribute & (_PIO_DEBOUNCE|_PIO_DEGLITCH)){
		p_pio -> PIO_IFER = ul_mask;
		} else{
		p_pio -> PIO_IFDR = ul_mask;
	}
	
	p_pio->PIO_PER = ul_mask; // Quando o PIO_PER = 1: : Enables the PIO to control the corresponding pin (disables peripheral control of the pin).
	p_pio->PIO_ODR = ul_mask;  //Quando o PIO_ODR = 1: Disables the output on the I/O line, ou seja, é um INPUT. 
}


void _pio_set_output(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_default_level, const uint32_t ul_multidrive_enable, const uint32_t ul_pull_up_enable)
{
	_pio_pull_up(p_pio, ul_mask, ul_pull_up_enable); // Ativa ou não o Pull- up.
	
	// Define a saída inicial do pino
	if (ul_default_level){
		//Se o PIO_SODR = 1: Sets the data to be driven on the I/O line.
		_pio_set(p_pio, ul_mask);
	} else{
		// Se o PIO_CODR = 1: Clears the data to be driven on the I/O line.
		_pio_clear(p_pio, ul_mask);
	}
	
	// Ativa ou não o multidrive:
	if (ul_multidrive_enable){
		p_pio -> PIO_MDER = ul_mask;
	} else{
		p_pio -> PIO_MDDR = ul_mask;
	}
	
	p_pio->PIO_OER = ul_mask;  //Quando o PIO_OER = 1: Enables the output on the I/O line. 
	p_pio->PIO_PER = ul_mask; // Quando o PIO_PER = 1: : Enables the PIO to control the corresponding pin (disables peripheral control of the pin).
	
}


uint32_t _pio_get(Pio *p_pio, const pio_type_t ul_type, const uint32_t ul_mask)
{
	uint32_t input_or_output;

	if (ul_type == PIO_OUTPUT_0) {
		
		input_or_output = p_pio->PIO_ODSR; //  write operations set and clear the Output Data Status Register (PIO_ODSR).
		} else {
		input_or_output = p_pio->PIO_PDSR;  // The level on each I/O line can be read through PIO_PDSR. -> Input
	}

	if ((input_or_output & ul_mask) == 0) {  // aplica a mascara para pegar o pino exato que quero ver o status.
		return 0;
	} else {
		return 1;
	}
}

void _delay_ms(int ms){

	for (int i = 0; i < ms*150000 ; i++){  
		asm("NOP");
	}	
}

// Função de inicialização do uC
void init(void)
{
	// Initialize the board clock
	sysclk_init();

	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// Ativa o PIO na qual o LED foi conectado
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(LED1_PIO_ID);
	pmc_enable_periph_clk(LED2_PIO_ID);
	pmc_enable_periph_clk(LED3_PIO_ID);


	
	// Inicializa PIO do botao
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);


	
	//Inicializa PC8 como saída
	_pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 0, 0, 0);
	_pio_set_output(LED1_PIO, LED1_PIO_IDX_MASK, 0, 0, 0);
	_pio_set_output(LED2_PIO, LED2_PIO_IDX_MASK, 0, 0, 0);
	_pio_set_output(LED3_PIO, LED3_PIO_IDX_MASK, 0, 0, 0);


	
	// configura pino ligado ao botão como entrada com um pull-up.
	_pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK,  _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(BUT1_PIO, BUT1_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(BUT2_PIO, BUT2_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(BUT3_PIO, BUT3_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);


}


// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
	init();

	// super loop
	// aplicacoes embarcadas não devem sair do while(1).
	
	while(1){
		int resultPio;
		int resultPioBut1;
		int resultPioBut2;
		int resultPioBut3;

		//int resultPioBut2;
		resultPio = _pio_get(BUT_PIO, PIO_INPUT, BUT_PIO_IDX_MASK);
		resultPioBut1 = _pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK);
		resultPioBut2 = _pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK);
		resultPioBut3 = _pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK);

		if (!resultPio){
			for (int l = 0; l < 5; l++ )
			{
				_pio_set(PIOC, LED_PIO_IDX_MASK);      // Coloca 1 no pino LED
				_delay_ms(100);                        // Delay por software de 200 ms
				_pio_clear(PIOC, LED_PIO_IDX_MASK);    // Coloca 0 no pino do LED
				_delay_ms(100);                        // Delay por software de 200 ms
			}

			_pio_set(PIOC, LED_PIO_IDX_MASK);      // Coloca 1 no pino LED
		}


		else if (!resultPioBut1){
			for (int i = 0; i < 5; i++ )
			{
				_pio_set(PIOA, LED1_PIO_IDX_MASK);      // Coloca 1 no pino LED
				_delay_ms(100);                        // Delay por software de 200 ms
				_pio_clear(PIOA, LED1_PIO_IDX_MASK);    // Coloca 0 no pino do LED
				_delay_ms(100);                        // Delay por software de 200 ms
			} 
			
			_pio_set(PIOA, LED1_PIO_IDX_MASK);      // Coloca 1 no pino LED

		}
		
		
	  else if (!resultPioBut2){
			for (int j = 0; j < 5; j++)
			{
				_pio_set(PIOC, LED2_PIO_IDX_MASK);      // Coloca 1 no pino LED
				_delay_ms(100);                        // Delay por software de 200 ms
				_pio_clear(PIOC, LED2_PIO_IDX_MASK); // Coloca 0 no pino do LED
				_delay_ms(100);                        // Delay por software de 200 ms
			}
					
			_pio_set(PIOC, LED2_PIO_IDX_MASK);      // Coloca 1 no pino LED

		}
	
	 else if(!resultPioBut3){
			for (int z = 0; z < 5; z++ )
			{
				_pio_set(PIOB, LED3_PIO_IDX_MASK);      // Coloca 1 no pino LED
				_delay_ms(100);                        // Delay por software de 200 ms
				_pio_clear(PIOB, LED3_PIO_IDX_MASK);    // Coloca 0 no pino do LED
				_delay_ms(100);                        // Delay por software de 200 ms
			}
			
			_pio_set(PIOB, LED3_PIO_IDX_MASK);      // Coloca 1 no pino LED

		}
		
		
	}
	
	return 0;
}

