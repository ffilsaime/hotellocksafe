#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#define LED_PIN0 (1 << PB0)
#define LED_PIN1 (1 << PB1)
#define LED_PIN2 (1 << PB2)

#define KEY0 (1 << PB3)
#define KEY1 (1 << PB4)
#define LED_G 0b0001
#define LED_Y 0b0110
#define LED_R 0b0100
#define LED_B 0b0011

typedef enum { RELEASED=0, PRESSED } key_t;

bool done = false;
bool locked = false;
volatile int counter = 0;
volatile uint8_t password = 0;
volatile uint8_t userinput = 0;
key_t keystate = RELEASED;
key_t keystate1 = RELEASED;
uint8_t history = 0;
uint8_t history1 = 0;

ISR(TIMER0_COMPA_vect){
  history = history << 1;
  history1 = history1 << 1;

   if ((PINB & KEY0) == 0) history = history | 0x1;// helps debounce the button0
   if ((PINB & KEY1) == 0) history1 = history1 | 0x1; //helps debounce the button1

   // checks if the state commits to 1
   if ((history & 0b111111) == 0b111111) keystate = PRESSED; // checks if button0 is on
   if ((history1 & 0b111111) == 0b111111) keystate1 = PRESSED;// checks if button1 i on

   if ((history & 0b00111111) == 0) keystate = RELEASED;//checks if button0 is released
   if ((history1 & 0b00111111) == 0) keystate1 = RELEASED;//checks if button1 is released
}

int main(void){
    OCR0A = 0x01;
    TCCR0B = 0b001; //no prescale
    TIMSK = (1 << OCIE0A);

    sei();

  while (1){
    // yellow, blue, and green should be on when the button for 0 is pressed
    if (!locked && (keystate == PRESSED) && (counter < 6)){
      done = true; // this indicates if the button was just pressed
      // the blue is going to be turned on
      DDRB = LED_PIN2;
      PORTB = LED_B;
      _delay_ms(1);
      //the green led will be turned on
      DDRB = LED_PIN0|LED_PIN1;
      PORTB = LED_G;
      _delay_ms(1);
    //  the yellow led will be turned on
      PORTB = LED_Y;
      _delay_ms(1);
    }
    // yellow, blue, and green should be on when the button for 1 is pressed
    else if(!locked && (keystate1 == PRESSED) && (counter < 6)){
      done = true;
      //the blue is going to be turned on
      DDRB = LED_PIN1|LED_PIN2;
      PORTB = LED_B;
      _delay_ms(1);
      //the green led will be turned on
      DDRB = LED_PIN0|LED_PIN1;
      PORTB = LED_G;
      _delay_ms(1);
      //  the yellow is going to be turned on
      PORTB = LED_Y;
      _delay_ms(1);
    }
    //gets the six digit code from the button0
    else if(!locked && (keystate == RELEASED) && done){
      counter += 1;
      password |= 0;
      if (counter <= 5) password = password << 1;
      done = false;
    }
    //gets the six digit code form the button1
    else if(!locked && (keystate1 == RELEASED) && done){
      counter += 1;
      password |= 1;
      if (counter <= 5) password = password << 1;
      done = false;
    }
    // if counter is at 6 the red led will be turned on
    else if (!locked && (counter == 6)){// this is locked state
      //the red led will be turned on
      DDRB = LED_PIN1|LED_PIN2;
      PORTB = LED_R;
      counter = 0;
      locked = true; //this will indicate that it's in a locked state
      history = 0;
      history1 = 0;
    }
    //unlocked state for user input turns blue and red on when button is pressed
    else if(locked && (keystate == PRESSED) && (counter < 6)){
      done = true;
      //red led is on
      DDRB = LED_PIN1|LED_PIN2;
      PORTB = LED_R;
      _delay_ms(1);
      //blue led is once
      PORTB = LED_B;
      _delay_ms(1);
    }
    //turns the blue and red led on with button1
    else if(locked && (keystate1 == PRESSED) && (counter < 6)){
      done = true;
      DDRB = LED_PIN1|LED_PIN2;
      PORTB = LED_R;
      _delay_ms(1);
      //blue led is once
      PORTB = LED_B;
      _delay_ms(1);
    }
    //in the locked state it will increase the counter and input a 0
    // into the userinput
    else if((keystate == RELEASED) && done && locked){
      counter += 1;
      userinput |= 0;
      if (counter <= 5) userinput = userinput << 1;
      done = false;
    }
    //in the locked state it will increase the counter and input a 1
    // into the userinput
    else if((keystate1 == RELEASED) && done && locked){
      counter += 1;
      userinput |= 1;
      if (counter <= 5) userinput = userinput << 1;
      done = false;
    }
    else if((counter == 6) && locked){
      if(userinput == password){
        //green and yellow are lit
        DDRB = LED_PIN0|LED_PIN1;
        PORTB = LED_G;
        _delay_ms(1);
        PORTB = LED_Y;
        _delay_ms(1);

        //resets everything
        locked = false;
        done = false;
        password = 0;
        userinput = 0;
        keystate = 0;
        keystate1 = 0;
        history = 0;
        history1 = 0;
      }
      else{
        // starts flashing yellow
        DDRB = LED_PIN1|LED_PIN0;
        PORTB = LED_Y;
        _delay_ms(1000);
        PORTB = 0;
        _delay_ms(1000);

        DDRB = LED_PIN1|LED_PIN0;
        PORTB = LED_Y;
        _delay_ms(1000);
        PORTB = 0;
        _delay_ms(1000);

        DDRB = LED_PIN1|LED_PIN0;
        PORTB = LED_Y;
        _delay_ms(1000);
        PORTB = 0;
        _delay_ms(1000);
      }
    }
    else if(locked){
      //when it's in a locked state the led will be on
      DDRB = LED_PIN1|LED_PIN2;
      PORTB = LED_R;
    }
    //the yellow and green led will always be on in an unlocked state
    else if(!locked){
      //the yellow is on
      DDRB = LED_PIN1|LED_PIN0;
      PORTB = LED_Y;
      _delay_ms(1);
      //the green is on
      PORTB = LED_G;
      _delay_ms(1);
    }
  }


  return 0;
}
