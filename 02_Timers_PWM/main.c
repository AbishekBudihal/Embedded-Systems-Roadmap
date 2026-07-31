#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include <stdlib.h>

#define F_CPU 32000000UL

#define MPU_ADDR 0x68
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B

// ---------- LCD PINS ----------
#define LCD_DATA PORTA.OUT
#define LCD_DIR  PORTA.DIR

#define RS (1<<0)   // PD0
#define EN (1<<1)   // PD1

// ---------- TWI (I2C) ----------
void TWI_Init()
{
	// SDA = PC1 , SCL = PC0
	PORTC.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;
	PORTC.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc;

	// Enable TWI Master
	TWIC.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;

	// 100kHz @ 16MHz
	TWIC.MASTER.BAUD = 72;

	// Set bus state to IDLE
	TWIC.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}

// ---------- TWI Write ----------
void TWI_Write(uint8_t addr, uint8_t reg, uint8_t data)
{
	TWIC.MASTER.ADDR = addr << 1;      // Write mode
	while (!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));

	TWIC.MASTER.DATA = reg;
	while (!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));

	TWIC.MASTER.DATA = data;
	while (!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));

	TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
}


// ---------- TWI Read 16-bit ----------
int16_t TWI_Read16(uint8_t addr, uint8_t reg)
{
	uint8_t high, low;

	// Write register address
	TWIC.MASTER.ADDR = addr << 1;
	while (!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));

	TWIC.MASTER.DATA = reg;
	while (!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));

	// Repeated start for read
	TWIC.MASTER.ADDR = (addr << 1) | 1;
	while (!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm));

	high = TWIC.MASTER.DATA;
	TWIC.MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;

	while (!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm));
	low = TWIC.MASTER.DATA;

	TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;

	return (int16_t)((high << 8) | low);
}


// ---------- LCD Commands ----------
void LCD_Command(uint8_t cmd)
{
	PORTD.OUT &= ~RS;
	LCD_DATA = cmd;
	PORTD.OUT |= EN;
	_delay_us(1);
	PORTD.OUT &= ~EN;
	_delay_ms(2);
}

void LCD_Char(char c)
{
	PORTD.OUT |= RS;
	LCD_DATA = c;
	PORTD.OUT |= EN;
	_delay_us(1);
	PORTD.OUT &= ~EN;
	_delay_ms(2);
}

void LCD_String(const char *str)
{
	while(*str)
	LCD_Char(*str++);
}

void LCD_Init()
{
	LCD_DIR = 0xFF;
	PORTD.DIR |= RS | EN;

	_delay_ms(20);
	LCD_Command(0x38);   // 8-bit, 2-line
	LCD_Command(0x0C);   // Display ON
	LCD_Command(0x01);   // Clear
	LCD_Command(0x06);   // Entry mode
}

// ---------- MPU6050 Init ----------
void MPU6050_Init()
{
	TWI_Write(0x68, 0x6B, 0x00);   // Wake up MPU
}


// ---------- MAIN ----------
int main(void)
{
	char buffer[16];
	int16_t ax;

	LCD_Init();
	TWI_Init();
	MPU6050_Init();

	LCD_String("MPU6050 Ready");

	while(1)
	{
		ax = TWI_Read16(MPU_ADDR, ACCEL_XOUT_H);

		LCD_Command(0x80);   // Line 1
		sprintf(buffer, "AX: %d   ", ax);
		LCD_String(buffer);

		_delay_ms(300);
	}
}
