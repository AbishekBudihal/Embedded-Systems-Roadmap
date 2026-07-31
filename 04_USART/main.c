#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include <stdlib.h>

#define F_CPU 32000000UL

void uart_portE_init(void);
void uart_check(void);


int main(void)
{
	
}


/* --- UART check --- */
void uart_check()
{
	while (!(USARTE1_STATUS & (1<<USART_DREIF_bp)));
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

/* --- UART init --- */
void uart_portE_init()
{
	USARTE1_CTRLB = (1<<USART_RXEN_bp)|(1<<USART_TXEN_bp);
	USARTE1_CTRLB |= (1<<USART_CHSIZE1_bp)|(1<<USART_CHSIZE0_bp);
	USARTE1_BAUDCTRLA=(1<<USART_BSEL5_bp)|(1<<USART_BSEL0_bp);
	USARTE1_BAUDCTRLB=(1<<USART_BSCALE3_bp)|(1<<USART_BSCALE2_bp)|(1<<USART_BSCALE1_bp)|(1<<USART_BSCALE0_bp);
}

void Set_clk_freq_To32MHz(void)
{
	CCP=CCP_IOREG_gc;
	OSC.CTRL=OSC_RC32MEN_bm;
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_RC32M_gc;
}
