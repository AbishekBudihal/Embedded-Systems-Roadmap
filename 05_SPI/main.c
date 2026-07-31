#include <avr/io.h>
#include <math.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <inttypes.h>
#include <avr/cpufunc.h>

#define F_CPU 32000000UL
#define MOSI 5
#define SCLK 7
#define SS 4
#define MISO 6

void Set_clk_freq_To32MHz();     //Set clk frequency to 32MHz. When initially powered, the MC runs at 2MHz clk only

void SPI_init();
int main(void)
{
	Set_clk_freq_To32MHz();
	_NOP();
	_NOP();
	PORTA.DIR=(1<<0);          //Set PORTA 0 line as output
	PORTC.DIR=(1<<MOSI)|(1<<SCLK)|(1<<SS);
	PORTE.DIRSET=(1<<PIN7_bp);   //enable transmitter pin as output for UART
	PORTE.DIRCLR=(1<<PIN6_bp);   //enable receiver pin as input for UART
	PORTQ.DIR=(1<<3);
}


/*******************************************All Sub-routines are here*************************************************/

/******************Initialization PORTC as SPI********************/

void SPI_init()
{	
	SPIC_CTRL= (1<<SPI_ENABLE_bp) | (1<<SPI_MASTER_bp) | (1<<SPI_MODE0_bp) | (1<<SPI_PRESCALER1_bp) | (1<<SPI_PRESCALER0_bp);
}
