/*
 * IEEE_Conversion.c
 *
 * Created: 11/13/2025 12:17:13 PM
 * Author : ABISHEK
 */ 


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
void uart_portE_init();
void uart_Check();
void DAC_init();
void DAC_Allchannelszero();


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

/**********************Controller to DAC Write Packet i.e DAC Intialization*********************************/
void DAC_init()
{
	unsigned char byte_1, byte_2,byte_3, byte_4,byte_5,byte_6,byte_7,byte_8,byte_9; 
	PORTC_OUT= (0<<SS);          //Pulling SS Pin low to select DAC(i.e. slave)
	_NOP();
	_NOP();

/**********************Write Packet****************************/
	SPIC_DATA=0x03; //Write-0, Read-1, (23rd to 16th bit)00000011=0x03,i.e.SPI Config Register, Higher Byte(HB),Lower Byte(LB)
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	byte_1=SPIC_DATA;
	SPIC_DATA=0x0A;       // Here, 0x0A is the HB and defined as SDO-Enabled, ALMOUT-Enabled, TEMPALM-Enabled. These are recommended by IC manufacturers
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	byte_2=SPIC_DATA;
	SPIC_DATA=0xBC;        //Here, 0x84 is the LB. See the HB details on above
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	byte_3=SPIC_DATA;
	PORTC_OUT=(1<<SS);     //Pulling SS/CS Pin high, disabled DAC as a slave
	_NOP();
	_NOP();
	_delay_us(20);
/**********************Write Packet****************************/
	PORTC_OUT=(0<<SS);      //Pulling SS Pin low to select DAC(i.e slave)
	_NOP();
	_NOP();
	SPIC_DATA=0x04;       // Write-0, Read-1, (23rd to 16th bit)00000100=0x04, i.e.GEN Config Register
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	byte_4=SPIC_DATA;
	SPIC_DATA=0x3F;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	byte_5=SPIC_DATA;
	SPIC_DATA=0x00;
	while(!(SPIC_STATUS & (1<SPI_IF_bp)));
	byte_6=SPIC_DATA;
	PORTC_OUT=(1<<SS);    //Pulling SS/CS Pin high, disabled DAC as a slave
	_NOP();
	_NOP();
	_delay_us(20);
/**********************Write Packet****************************/
	PORTC_OUT=(0<<SS);     //Pulling SS Pin low to select DAC(Le slave)
	_NOP();
	_NOP();
	SPIC_DATA=0x09;      //DACPWDWN Register and loaded with 0xF00F; enabling 8 DAC channels
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	byte_7=SPIC_DATA;
	SPIC_DATA=0x00;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	byte_8=SPIC_DATA;
	SPIC_DATA=0x00;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	byte_9=SPIC_DATA;
	PORTC_OUT=(1<<SS);     //Pulling SS/CS Pin high, disabled DAC as a slave
	_NOP();
	_NOP();
	_delay_us(20);

/************************Write Packet**************************/
	PORTC_OUT=(0<<SS);      //Pulling SS Pin low to select AHRS(i.e. slave)
	_NOP();
	_NOP();
	SPIC_DATA=0x0B;     //DACRANGE register: set with range of 0-5V it is default
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	byte_7=SPIC_DATA;
	SPIC_DATA=0xAA;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	byte_8=SPIC_DATA;
	SPIC_DATA=0xAA;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	byte_9=SPIC_DATA;
	PORTC_OUT=(1<<SS); //Pulling SS/CS Pin high, disabled DAC as a slave
	_NOP();
	_NOP();
	_delay_us(20);
	PORTC_OUT=(0<<SS);    //Pulling SS Pin low to select AHRS(i.e. slave)
	_NOP();
	_NOP();
	SPIC_DATA=0x0C;       //DACRANGE register: set with range of 0-5V it is default
	while(!(SPIC_STATUS & (1<SPI_IF_bp)));
	byte_7=SPIC_DATA;
	SPIC_DATA=0xAA;
	while(!(SPIC_STATUS & (1<SPI_IF_bp)));
	byte_8=SPIC_DATA;
	SPIC_DATA=0xAA;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	byte_9=SPIC_DATA;
	PORTC_OUT=(1<<SS);    //Pulling SS/CS Pin high, disabled DAC as a slave
	_NOP();
	_NOP();
	_delay_us(20);
}

/******************To make all DAC channels output to 'OV', apply 0x7FFF on each channel, where 7F is the MSByte and FF is LSByte******************/ 
void DAC_Allchannelszero()
{
	unsigned char Receive_null;
	PORTC_OUT=(0<<SS);    //Pulling SS Pin low to select DAC (ie slave)
	_NOP();
	SPIC_DATA=0x14;    //Select Channel DACO
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	Receive_null=SPIC_DATA;
	SPIC_DATA=0x7F;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	Receive_null=SPIC_DATA;
	SPIC_DATA=0xFF;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	Receive_null=SPIC_DATA;
	PORTC_OUT=(1<<SS);     //Pulling SS/CS Pin high, disabled DAC as a slave
	_NOP();
	_delay_us(20);   // recommended delay >=10us
	PORTC_OUT=(0<<SS);    //Pulling SS Pin low to select DAC (ie slave)
	_NOP();
	SPIC_DATA=0x15;    //Select Channel DACI
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	Receive_null=SPIC_DATA;
	SPIC_DATA=0x7F;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	Receive_null=SPIC_DATA;
	SPIC_DATA=0xFF;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	Receive_null=SPIC_DATA; 
	PORTC_OUT=(1<<SS);    //Pulling SS/CS Pin high, disabled DAC as a slave
	_NOP();
	_delay_us(20); 
	PORTC_OUT=(0<<SS);    //Pulling SS Pin low to select DAC (i.e slave) 
	_NOP();
	SPIC_DATA=0x16;    //Select Channel DAC2
	while(!(SPIC_STATUS & (1<<SPI_IF_bp))); 
	Receive_null=SPIC_DATA; 
	SPIC_DATA=0x7F; 
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	Receive_null=SPIC_DATA;
	SPIC_DATA=0xFF;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	Receive_null=SPIC_DATA;
	PORTC_OUT=(1<<SS);     //Pulling SS/CS Pin high, disabled DAC as a slave
	_NOP();
	_delay_us(20);
	PORTC_OUT=(0<<SS);    //Pulling SS Pin low to select DAC (i.e slave)
	_NOP();
	SPIC_DATA=0x17;     //Select Channel DAC3
	while(!(SPIC_STATUS & (1<<SPI_IF_bp))); 
	Receive_null=SPIC_DATA; 
	while(!(SPIC_STATUS & (1<<SPI_IF_bp))); 
	Receive_null=SPIC_DATA;
	SPIC_DATA=0xFF; 
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	SPIC_DATA=0x7F;
	Receive_null=SPIC_DATA;
	PORTC_OUT=(1<<SS);    //Pulling SS/CS Pin high, disabled DAC as a slave
	_NOP();
	_delay_us(20);
	PORTC_OUT=(0<<SS);   // Pulling SS Pin low to select DAC(i.e slave)
	_NOP();
	SPIC_DATA=0x18;    //Select Channel DAC4 
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	Receive_null=SPIC_DATA;
	SPIC_DATA=0x7F;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	Receive_null=SPIC_DATA;
	SPIC_DATA=0xFF;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	Receive_null=SPIC_DATA;
	PORTC_OUT=(1<<SS);     //Pulling SS/CS Pin high, disabled DAC as a slave
	_NOP();
	_delay_us(20);
	PORTC_OUT=(0<<SS);     // Pulling SS Pin low to select AHRS(i.e slave)
	_NOP();
	SPIC_DATA=0x19;      //Select Channel DAC5
	Receive_null=SPIC_DATA;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp))); 
	Receive_null=SPIC_DATA;
	SPIC_DATA=0x7F;
	while(!(SPIC_STATUS &(1<<SPI_IF_bp)));
	SPIC_DATA=0xFF;
	while(!(SPIC_STATUS & (1<<SPI_IF_bp)));
	Receive_null=SPIC_DATA;
	PORTC_OUT=(1<<SS);      //Pulling SS/CS Pin high, disabled DAC as a slave
	_NOP();
	_delay_us(20);
}

/****************************Set MC clock to 32MHz*********************************/
void Set_clk_freq_To32MHz()
{
	CCP=CCP_IOREG_gc;
	OSC.CTRL=OSC_RC32MEN_bm;
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_RC32M_gc;
}
