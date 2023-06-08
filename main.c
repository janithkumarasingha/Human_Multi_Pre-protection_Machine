#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "LCDI2C.h"


uint16_t TempReading=0; //analog Reading variable
uint16_t ReadADC(uint8_t ADCchannel); //analog reading function
void servoangle(uint8_t pin,uint8_t pinC);
void ultralengthM();

char lcddata[20];
uint8_t PeopleCounter=0;
uint8_t PeopleCounterMem=0;
#define PeopleLimit 6
#define Tempupper 40
volatile uint16_t TimerCalVal=0;// variable for collect echo data
uint16_t ultralength=0;

uint8_t hx711H=0; //Load Scale High Bits
uint16_t hx711L=0;//Load Scale Low Bits
float loadCellRead();
#define Load_data 2
#define Load_clk 3

uint8_t managerSt=0;


int main(void)
{	
	deviceAdress=0x23;
	LcdInit();
	deviceAdress=0x27;
	LcdInit();
	
	LcdSetCursor(4,0,"Welcome");
	_delay_ms(500);
	LcdCommand(LCD_CLEARDISPLAY);
	MCUCR|=(1<<ISC11)|(1<<ISC10)|(1<<ISC01)|(1<<ISC00);
	MCUCSR|=(1<<ISC2);
	GICR|=(1<<INT0)|(1<<INT1)|(1<<INT2);
	
	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));   // prescaler 128
	ADMUX |= (1<<REFS0)|(1<<REFS1);					//internal 2.56 v ref
	ADCSRA |= (1<<ADEN);                            // Turn on ADC
	PORTA|=(1<<1);
	
	
	TCCR1B|=(1<<CS12)|(1<<CS10)|(1<<WGM12);//Start timer  prescaler =1024
	TCNT1=0;
	OCR1A=31250;
	
	DDRA|=(1<<2)|(1<<3)|(1<<4);
	DDRD|=(1<<4); //D4 as output
	DDRD|=(1<<6); //D6 as output
	DDRD|=(1<<7); //D7 as output
	DDRB|=(1<<3);
	TCCR0|=(1<<WGM01);//Enable Compare match mode
	TCCR0|=(1<<CS11);//Start timer  prescaler =8
	TCNT0=0;
	OCR0=10;
	/*register value= time*(clock speed/prescale)
	register value=0.000001*(8000000/1)
	register value=10*/
	TIMSK|=(1<<OCIE0);//enable timer Compare inturrept
	
	DDRC|=(1<<Load_clk); //Load cell clock pin
	PORTC&=~(1<<Load_clk);//Clock pin low
	
	sei();
	while (1)
	{
		deviceAdress=0x27;	
		ultralengthM();
		LcdSetCursor(0,0,"Hand..");
		
		if (ultralength<20)
		{	
			TempReading=(ReadADC(0)*0.25024438); //calibrated number
			deviceAdress=0x27;
			if (TempReading<Tempupper)
			{
				LcdCommand(LCD_CLEARDISPLAY);
				LcdSetCursor(0,0,"Temperature Ok");
				_delay_ms(500);
				LcdCommand(LCD_CLEARDISPLAY);
				
				PORTA|=(1<<4);_delay_ms(1000);PORTA&=~(1<<4);
				
				
				if (PeopleCounter<PeopleLimit)
				{servoangle(1,7);
				}
				
				
				
				
				LcdSetCursor(0,0,"Press button");
				_delay_ms(500);
				
				for(uint8_t i=0;i<100;i++){
				if (!(PINA&(1<<1)))
				{
					
					LcdCommand(LCD_CLEARDISPLAY);
					LcdSetCursor(0,0,"opening");
					servoangle(1,6);
					LcdCommand(LCD_CLEARDISPLAY);
					LcdSetCursor(0,0,"Place Bag");
					for(uint8_t j=0;j<100;j++){
				float weight=loadCellRead();
				sprintf(lcddata,"%0.2fKg",weight);
				LcdSetCursor(0,1,lcddata);	
				
						
				if (weight>40)
				{
					LcdCommand(LCD_CLEARDISPLAY);
					LcdSetCursor(0,0,"Please wait");
					servoangle(0,6);
					
							PORTA|=(1<<2);
							_delay_ms(1000);
							PORTA&=~(1<<2);
					servoangle(1,6);		
					LcdCommand(LCD_CLEARDISPLAY);	
					j=99;
					break;
										
				}
					
				_delay_ms(100);		
			}
			i=99;	
		}
				
	_delay_ms(100);
}
	servoangle(0,7);
	servoangle(0,6);	
} 
	else
		{	LcdCommand(LCD_CLEARDISPLAY);
				LcdSetCursor(0,0,"Try Again");
				_delay_ms(1000);
				LcdCommand(LCD_CLEARDISPLAY);
		}

}
		
		
		if (PeopleCounterMem>PeopleCounter)
		{
				servoangle(1,6);
				PeopleCounterMem=PeopleCounter;	
				_delay_ms(1000);
				servoangle(0,6);
		}
		
		else
		{
			
			PeopleCounterMem=PeopleCounter;
			
		}
				
		deviceAdress=0x23;
		
		if (managerSt)
		{LcdSetCursor(0,0,"Manager:IN ");
		} 
		else
		{LcdSetCursor(0,0,"Manager:OUT ");
		}
		
		
		sprintf(lcddata,"Count- %02u  ",PeopleCounter);
		LcdSetCursor(0,1,lcddata);
		
		_delay_ms(10);
		if (PeopleCounter>PeopleLimit)
		{LcdSetCursor(0,2,"People limited  ");
		}
		else
		{LcdSetCursor(0,2,"               ");
		}
		
	}
}
ISR(INT0_vect){
	
	
	
	if (TIMSK&(1<<OCIE1A))
	{TIMSK&=~(1<<OCIE1A);
		if (TCNT1>0)
		{PeopleCounter++;
			TCNT1=0;
			PORTA&=~(1<<3);
		}
	}
	else
	{TIMSK|=(1<<OCIE1A);
		PORTA|=(1<<3);
	}
	
	
}
ISR(INT1_vect){
	if (TIMSK&(1<<OCIE1A))
	{TIMSK&=~(1<<OCIE1A);
		if (TCNT1>0)
		{
			if (PeopleCounter>0)
			{PeopleCounter--;
				PORTA&=~(1<<3);
			}
		}
	}
	else
	{TIMSK|=(1<<OCIE1A);
		PORTA|=(1<<3);
	}
}
ISR(TIMER1_COMPA_vect){//ultrasonic
	
	
	TIMSK&=~(1<<OCIE1A);//enable timer Compare inturrept
	PORTA&=~(1<<3);
	TCNT1=0;
}

uint16_t ReadADC(uint8_t ADCchannel)
{
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
	//single conversion mode
	ADCSRA |= (1<<ADSC);
	// wait until ADC conversion is complete
	while( ADCSRA & (1<<ADSC) );
	return ADCW;
}

void servoangle(uint8_t pin,uint8_t pinC)
{

	if (pin)
	{
		for(uint8_t j=0;j<100;j++){
			PORTD|=(1<<pinC);
			for(uint8_t i=0;i<10;i++){
				_delay_us(100);
			}
			PORTD&=~(1<<pinC);
			
		
			
			for(uint8_t i=0;i<10;i++){
				_delay_us(100);
			}
		}
	}
	else
	{for(uint8_t j=0;j<100;j++){
		
		PORTD|=(1<<pinC);
		for(uint8_t i=0;i<15;i++){
			_delay_us(100);
		}
		PORTD&=~(1<<pinC);
		
			
		for(uint8_t i=0;i<15;i++){
			_delay_us(100);
		}
	}
}




}



ISR(TIMER0_COMP_vect){//ultrasonic
	TimerCalVal++;
	TCNT0=0;
	
}


float loadCellRead(){
	hx711H=0;hx711L=0;  //clear variables
	for(uint8_t i=0;i<8;i++){  // Load cell data high 8 bits
		PORTC|=(1<<Load_clk); //Clock pin high
		_delay_us(10);
		if ((PINC&(1<<Load_data))>>Load_data)  //read data pin
		{hx711H|=(1<<(7-i));//set hx 711 varible
		}
		else
		{hx711H&=~(1<<(7-i));
		}
		PORTC&=~(1<<Load_clk); //Clock pin low
		_delay_us(5);
	}
	
	
	for(uint8_t i=0;i<16;i++){ // Load cell data low 16 bits
		PORTC|=(1<<Load_clk); //Clock pin high
		_delay_us(10);
		if ((PINC&(1<<Load_data))>>Load_data) //read data pin
		{hx711L|=(1<<(15-i));
		}
		else
		{hx711L&=~(1<<(15-i));
		}
		PORTC&=~(1<<Load_clk); //Clock pin low
		_delay_us(5);
	}
	
	hx711L=hx711L>>1; //shift bits
	
	if (hx711H&1)  //bit setup
	{hx711L|=(1<<15);
	}
	else
	{hx711L&=~(1<<15);
	}
	hx711H=hx711H>>1;
	
	return (hx711H*(65536/18029.6))+hx711L/18029.6; //load cell calibration
}

void ultralengthM(){
	
	PORTD&=~(1<<4);//TRIG pin low
	_delay_us(50);//wait 50 micro sec
	PORTD|=(1<<4);//TRIG pin high
	_delay_us(50);//wait 50 micro sec
	PORTD&=~(1<<4);////TRIG pin low
	while(!(PIND&(1<<5)))//wait for pulse
	TimerCalVal=0;//rest timer
	while((PIND&(1<<5)))////wait for pulse down
	ultralength=TimerCalVal/4.1282;//copy timer value
}

ISR(INT2_vect){
	PORTB^=(1<<3);
	if (managerSt)
	{managerSt=0;
	} 
	else
	{managerSt=1;
	}
}