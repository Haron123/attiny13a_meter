#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

// If '1' the i2c functions and
// The displaynumber function for the 4 digit display
// Get compiled, if not needed can just delete those functions.
#define DISPLAY_OUTPUT 1

// For the 4 digit display used for the output in this example :
#define CLOCK_PIN PB0
#define DATA_PIN PB3
#define I2C_PORT PORTB
//##########################

// INITIAL_ADC_PIN is used to measure VCC at the start
// ADC_PIN is used to measure current
#define INITIAL_ADC_PIN PB4
#define ADC_PIN PB2

// Value of the used shunt resistor, Change this according to what shunt you use
#define SHUNT 330

#define ADC_TOP 1023

/*
if result_format == 0 then current will be in Micro_amps
if result_format == 10 then current will be in Micro_amps / 10
if result_format == 100 then current will be in Micro_amps / 100
If result_format == 1000 then current will be output as Milli_amps

You can guess what the in betweens do, choose this however suited for ur application
*/	
#define result_format 10


#if DISPLAY_OUTPUT == 1
void startcomm();
void endcomm();
void sendByte(int8_t data);
void displayNumber(uint16_t inputnumber);
#endif

void ADC_request();
uint16_t get_voltage_ref();
uint16_t get_microv_per_adc(uint32_t ref_voltage);

// If you dont use a 4 digit display you can use CLOCK_PIN(PB0) and DATA_PIN(PB3) for ur own use cases
// Also PB1 on the attiny13a is free to use

	/* PIN SETUP
	PB4 as INPUT, has to be 1/10th of VCC this can be done via a voltage Divider
	PB2 as INPUT, has to be connected via a shunt Resistor to VCC (This also determines SHUNT value)

	PB1, PB3, PB0 Are Free to use (In this example used for a 4 Digit Display via I2C)

	https://i.imgur.com/a6SrCUI.png Heres a shitty example of my circuit
	*/	


int main(void)
{
	DDRB =  (1 << CLOCK_PIN) | (1 << DATA_PIN) | (0 << ADC_PIN) | (0 << INITIAL_ADC_PIN);

	// Setup ADC
	
	/*
	For MUX :			For REF :
	00 = PB5 (ADC0)		0 for VCC
	01 = PB2 (ADC1)		1 for 1.1v internal ref
	10 = PB4 (ADC2)
	11 = PB3 (ADC3)
	*/	

	ADMUX = (1 << REFS0) | (0b10 << MUX0); // Set voltage ref to internal 1.1v, use PB4 for ADC
	ADCSRA = (1 << ADEN) | (0b110 << ADPS0); // Prescaler to 64, turn on ADC
	ADCSRB = (0b000 << ADTS0); // set ADC mode to Free running mode 

	uint32_t voltage_drop = 0;
	uint32_t current = 0;

	uint32_t ref_voltage = get_voltage_ref();
        uint32_t microv_per_adc = get_microv_per_adc(ref_voltage);

	ADMUX = (0 << REFS0) | (0b01 << MUX0); // Set voltage ref to vcc and use PB2
	while(1)
	{
		// Get adc and calculate the current
		ADC_request();
		voltage_drop = (ref_voltage * 1000) - (microv_per_adc * ADC);
			
		voltage_drop = voltage_drop / result_format;
		current = voltage_drop / SHUNT;

		displayNumber(current);
		_delay_ms(50);
	}
}

uint16_t get_voltage_ref()
{
	ADMUX |= (1 << REFS0);

	int32_t adc = 0;

	ADC_request();
	_delay_ms(100);
	ADC_request();
	_delay_ms(50);
	adc = ADC;
	adc = adc * 10000;
	
	ADMUX &= ~(1 << REFS0);
	return (adc / 930);
}

uint16_t get_microv_per_adc(uint32_t ref_voltage)
{
	uint32_t microv = 0;

	microv = ref_voltage * 1000;
	microv = microv / 1023;
	return microv;
}

void ADC_request()
{
	ADCSRA |= (1 << ADSC);
	while(!ADCSRA & (1 << ADSC));
}
#if DISPLAY_OUTPUT == 1
void displayNumber(uint16_t inputnumber)
{
	int8_t result = 0;
	uint16_t newnumber = 0;
	newnumber += inputnumber / 1000;
	newnumber += ((inputnumber / 100) % 10) * 10;
	newnumber += ((inputnumber / 10) % 10) * 100;
	newnumber += ((inputnumber) % 10) * 1000;

	uint8_t number = newnumber % 10;

	// Send data to memory address of 4 number display
	startcomm();
	sendByte(0x03);
	for(int8_t i = 0; i < 4; i++)
	{
		result = 0xFD;

		number = newnumber % 10;

		if(number == 1)
		{
			result = 0x61;
		}
		else if(number == 2)
		{
			result = 0xDB;
		}
		else if(number == 3)
		{
			result = 0xF3;
		}
		else if(number == 4)
		{
			result = 0x67;		
		}
		else if(number == 5)
		{
			result = 0xB7;
		}
		else if(number == 6)
		{
			result = 0xBF;
		}
		else if(number == 7)
		{
			result = 0xE1;
		}
		else if(number == 8)
		{
			result = 0xFF;
		}
		else if(number == 9)
		{
			result = 0xF7;
		}
		newnumber /= 10;
		sendByte(result);
		
	}
	endcomm();
	startcomm();
	sendByte(0x11);
	endcomm();
}


void startcomm()
{
	I2C_PORT |= (1 << DATA_PIN); // Set Data High
	I2C_PORT |= (1 << CLOCK_PIN); // Set Clock high
	I2C_PORT &= ~(1 << DATA_PIN); // Set Data low
}

void endcomm()
{
	I2C_PORT &= ~(1 << DATA_PIN); // Set Data Low
	I2C_PORT |= (1 << CLOCK_PIN); // Set Clock high
	I2C_PORT |= (1 << DATA_PIN); // Set Data high
}

void sendByte(int8_t data)
{
	I2C_PORT &= ~(1 << CLOCK_PIN); // Set Clock low
	for(int8_t i = 7; i >= 0; i--)
	{
		int8_t bit = data & (1 << i);

		I2C_PORT |= (1 << DATA_PIN); // Set Data high

		if(bit == 0)
		{
			I2C_PORT &= ~(1 << DATA_PIN); // Set Data low
			
		}
			I2C_PORT |= (1 << CLOCK_PIN); // Set Clock high
			I2C_PORT &= ~(1 << CLOCK_PIN); // Set Clock low
	}
	I2C_PORT |= (1 << CLOCK_PIN); // Set Clock high
	I2C_PORT &= ~(1 << CLOCK_PIN); // Set Clock low
}
#endif
