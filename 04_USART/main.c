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

void Set_clk_freq_To32MHz();     //Set clk frequency to 32MHz. When initially powered, the MC runs at 2MHz clk only

void uart_portE_init();
void uart_Check();

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
	
/******************Initialization PORTED as UART*********************/

void uart_portE_init()
{
//		Initialization of PORTED as UART to baudrate is at 115200, 8-bits,no parity, stop bit 1
	USARTE1_CTRLB = (1<<USART_RXEN_bp)|(1<<USART_TXEN_bp);
	USARTE1_CTRLB |= (1<<USART_CHSIZE1_bp)|(1<<USART_CHSIZE0_bp);
	USARTE1_BAUDCTRLA=(1<<USART_BSEL5_bp)|(1<<USART_BSEL0_bp);       //For BSEL value at 115200 baudrate with 12 as per data given in the datasheet
	USARTE1_BAUDCTRLB=(1<<USART_BSCALE3_bp)|(1<<USART_BSCALE2_bp)|(1<<USART_BSCALE1_bp)|(1<<USART_BSCALE0_bp);  //For BSCALE value at 115200 baudrate with -1 as per data given in the datasheet
}

/********************Sample data to computer*************************/
void uart_Check()
{
	while (!(USARTE1_STATUS & (1<<USART_DREIF_bp)));                  //send ascii character 'C' to pc
	USARTE1_DATA = 'C';
	while (!(USARTE1_STATUS & (1<<USART_DREIF_bp)));
	USARTE1_DATA = '&';
	while (!(USARTE1_STATUS & (1<<USART_DREIF_bp)));
	USARTE1_DATA = 'G';
	while (!(USARTE1_STATUS & (1<<USART_DREIF_bp)));
	USARTE1_DATA = 0x0D;
	while (!(USARTE1_STATUS & (1<<USART_DREIF_bp)));
	USARTE1_DATA = 0x0A;
}
