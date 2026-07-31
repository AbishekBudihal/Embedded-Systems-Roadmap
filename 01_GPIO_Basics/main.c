#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <inttypes.h>
#include <avr/cpufunc.h>

#define F_CPU 32000000UL

#define MPU_ADDR 0x68
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B

// ---------- LCD PINS ----------
#define LCD_DPRT PORTC.OUT
#define LCD_RSON PORTE.OUT |=(1<<0)
#define LCD_RSOFF PORTE.OUT &=~(1<<0)
#define LCD_RWON PORTE.OUT |=(1<<1)
#define LCD_RWOFF PORTE.OUT &=~(1<<1)
#define LCD_EON PORTE.OUT &=~(1<<6)
#define LCD_EOFF PORTE.OUT &=~(1<<6)


void Set_clk_freq_To32MHz();
void lcdData(unsigned char data);
void lcdCommand(unsigned char cmnd);
void lcd_init();
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_print(char *str);
void MPU6050_Init();
void TWI_Init();
int16_t TWI_Read16(uint8_t addr, uint8_t reg);
void TWI_Write(uint8_t addr, uint8_t reg, uint8_t data);




// ---------- MAIN ----------
int main(void)
{
	Set_clk_freq_To32MHz();
	_NOP();
	_NOP();
	
	TWI_Init();
	MPU6050_Init();
	char buffer[16];
	int16_t ax;
	PORTC.DIR |=(1<<0);
	PORTC.DIR |=(1<<1);
	PORTC.DIR |=(1<<2);
	PORTC.DIR |=(1<<3);
	PORTC.DIR |=(1<<4);
	PORTC.DIR |=(1<<5);
	PORTC.DIR |=(1<<6);
	PORTC.DIR |=(1<<7);
	
	PORTE.DIR |=(1<<0);
	PORTE.DIR |=(1<<1);
	PORTE.DIR |=(1<<6);
	PORTE.DIR |=(1<<7);
	lcd_init();
	_delay_ms(20);
	lcdCommand(0x01);
	_delay_ms(50);
	lcd_gotoxy(1,1); 
	lcd_print("Altitude Measurement");
	lcd_gotoxy(1,2); 
	lcd_print("Yaw, Pitch, Roll");
	lcdCommand(0x0C);
	_delay_us(60);
	_delay_ms(5000);
	lcdCommand(0x01);
	_delay_ms(20);
	while(1)
	{
		ax = TWI_Read16(MPU_ADDR, ACCEL_XOUT_H);
		lcdCommand(0x80);   // Line 1
		itoa(ax, buffer,10);
		lcd_print("AX:");
		lcd_print(buffer);
		_delay_ms(300);
	}
}


void Set_clk_freq_To32MHz()
{
	CCP=CCP_IOREG_gc;
	OSC.CTRL=OSC_RC32MEN_bm;
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_RC32M_gc;
}

//---------------------------LCD-----------------------------------
void lcdCommand(unsigned char cmnd)
{
	PORTC.OUT=cmnd;
	LCD_RSOFF;
	LCD_RWOFF;
	LCD_EON;
	_delay_us(1);
	LCD_EOFF;
	_delay_us(100);
}

void lcdData(unsigned char data)
{
	PORTC.OUT=data;
	LCD_RSON;
	LCD_RWOFF;
	LCD_EON;
	_delay_us(1);
	LCD_EOFF;
	_delay_us(100);
}

void lcd_init()
{
	PORTC.DIR=0xFF;
	LCD_EOFF;
	_delay_us(2000);
	lcdCommand(0x38);
	lcdCommand(0x0E);
	lcdCommand(0x01);
	_delay_us(2000);
	lcdCommand(0x06);
}

void lcd_gotoxy(unsigned char x, unsigned char y)
{
	unsigned char firstCharAdr[]={0x80,0xC0,0x94,0xD4};
	lcdCommand(firstCharAdr[y-1]+ x-1);
	_delay_us(100);
}

void lcd_print(char *str)
{
	unsigned char i=0;
	while(str[i]!=0)
	{
		lcdData(str[i]);
		i++;
	}
}



// ---------- MPU6050 Init ----------
void MPU6050_Init()
{
	TWI_Write(0x68, 0x6B, 0x00);   // Wake up MPU
}



//----------------------i2c------------------------------------------
void TWI_Init()
{
	// SDA = PF0, SCL = PF1
	PORTF.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;
	PORTF.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc;

	// Enable TWI Master on TWIF
	TWIF.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;

	// 100 kHz @ 32 MHz
	TWIF.MASTER.BAUD = 150;  // correct for 32MHz

	// Set bus to IDLE
	TWIF.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}

// ---------- TWI Write ----------
void TWI_Write(uint8_t addr, uint8_t reg, uint8_t data)
{
	TWIF.MASTER.ADDR = addr << 1;
	while (!(TWIF.MASTER.STATUS & TWI_MASTER_WIF_bm));

	TWIF.MASTER.DATA = reg;
	while (!(TWIF.MASTER.STATUS & TWI_MASTER_WIF_bm));

	TWIF.MASTER.DATA = data;
	while (!(TWIF.MASTER.STATUS & TWI_MASTER_WIF_bm));

	TWIF.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
}

// ---------- TWI Read 16-bit ----------
int16_t TWI_Read16(uint8_t addr, uint8_t reg)
{
	uint8_t high, low;

	TWIF.MASTER.ADDR = addr << 1;
	while (!(TWIF.MASTER.STATUS & TWI_MASTER_WIF_bm));

	TWIF.MASTER.DATA = reg;
	while (!(TWIF.MASTER.STATUS & TWI_MASTER_WIF_bm));

	TWIF.MASTER.ADDR = (addr << 1) | 1;
	while (!(TWIF.MASTER.STATUS & TWI_MASTER_RIF_bm));

	high = TWIF.MASTER.DATA;
	TWIF.MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;

	while (!(TWIF.MASTER.STATUS & TWI_MASTER_RIF_bm));
	low = TWIF.MASTER.DATA;

	TWIF.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;

	return (high << 8) | low;
}

