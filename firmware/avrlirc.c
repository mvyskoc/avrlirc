/****************************************************************************
 ** avrlirc.c ***************************************************************
 ****************************************************************************
 *
 *  Lirc RS232 IR interface based on AVR ATTiny 25*
 *  Copyright (C) 2016  Martin Vyskocil <m.vyskoc@seznam.cz>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * AVR ATtiny25/45/85 pinout
 *                  ----------
 *     /reset, pb5 |1        8| Vcc              
 *      xtal1, pb3 |2        7| pb2 (sck, int0, pcint2)  
 *      xtal2, pb4 |3        6| pb1 (miso, pcint1)                          
 *             GND |4        5| pb0 (mosi, pcint0)                              
 *                  ----------   
 */

//#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define BAUD_RATE 38400

#define CK0_DIV1 	1
#define CK0_DIV8	2
#define CK0_DIV64	3
#define CK0_DIV256	4
#define CK0_DIV1024	5


#define CK0_DIV CK0_DIV8
#define UART_OCR (F_CPU / (8*BAUD_RATE))

#define TX_BUFFER_SIZE 64 //Power of 2
#define TX_BUFFER_MASK (TX_BUFFER_SIZE-1)

#define TXD_BITNUM PB0
#define TXD_PORT PORTB
#define TXD_DDR DDRB

#define IR_BITNUM PB2
#define IR_PIN PINB
#define IR_PORT PORTB
#define IR_DDR DDRB

//#define RS232_TTL
#ifndef RS232_TTL
  #define TXD_SetOne()       TXD_PORT &= ~_BV(TXD_BITNUM)
  #define TXD_SetZero()      TXD_PORT |= _BV(TXD_BITNUM)
#else
  #define TXD_SetOne()       TXD_PORT |= _BV(TXD_BITNUM)
  #define TXD_SetZero()      TXD_PORT &= ~_BV(TXD_BITNUM)
#endif

#define IR_high()            (IR_PIN & _BV(IR_BITNUM))

#define EnableIRInterrupt()  GIMSK |= _BV(PCIE)
#define DisableIRInterrupt() GIMSK &= ~_BV(PCIE)

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

typedef enum {
   startbit, 
   databit0, 
   databit1, 
   databit2,
   databit3,
   databit4,
   databit5,
   databit6,
   databit7,
   stopbit0,
   stopbit1,
   stopbit2,
   stopbitn,
   txend,
   idle
} uart_state_t;

typedef struct {
   uart_state_t state;
   uint8_t tx_data;   //actualy transmitted byte
   uint8_t tx_buffer[TX_BUFFER_SIZE];
   uint8_t tx_r, tx_w; //Read, write pointer to tx_buffer
} uart_t;


typedef struct {
  uint8_t high; 
  uint16_t length;
  uint8_t timer_overflow;
  uint16_t timer;
} rc_pulse_t;

volatile uart_t uart;
volatile rc_pulse_t rc_pulse;

static void hardware_init(void)
{
   /* Disable analog comparator - save power */
    ACSR = _BV(ACD);

   /* Setup IO pins */
   TXD_DDR |= _BV(TXD_BITNUM);
   TXD_SetOne(); //Set Idle state
   
   IR_PORT |= _BV(IR_BITNUM);

   //PCIE: Pin Change Interrupt Enable
   PCMSK |= _BV(PCINT2);
   EnableIRInterrupt();

   //GIMSK = _BV(PCIE); 

   //Timer0 Init
   //OCIE0A: Timer/Counter0 Output Compare Match A Interrupt Enable
   TIMSK = _BV(OCIE0A);
   TCCR0A = _BV(WGM01); //CTC mode
   TCCR0B = (CK0_DIV << CS00); 
   OCR0A = UART_OCR;
}

static void uart_init(void)
{
   uart.state = idle;
   uart.tx_w = uart.tx_r = 0;
}

void uart_tx_char(uint8_t ch)
{
   uint8_t tmp;

   tmp = (uart.tx_w +1) & TX_BUFFER_MASK;
   if (tmp == uart.tx_r) {
      return; // drop character
   }

   uart.tx_buffer[tmp] = ch;
   uart.tx_w = tmp;
}

void uart_tx_word(uint16_t w)
{
   uart_tx_char(w & 0xff);
   uart_tx_char((w >> 8) & 0xff);
}

ISR(TIMER0_COMPA_vect)
{
   //Start bit, 8 data bits, 2xstop bit
   switch ( uart.state ) 
   {
      case startbit:
          TXD_SetZero();
          uart.state = databit0;
          break;

      case databit0...databit7:
          if (uart.tx_data & 0x01) {
            TXD_SetOne();
          } else {
            TXD_SetZero();
          }
          uart.tx_data = (uart.tx_data >> 1);
          uart.state += 1;
          break;

      case stopbit0:
          TXD_SetOne();
      case stopbit1...stopbitn:
          uart.state++;
          break;

      case idle:
          TXD_SetOne();
      case txend:
          if ( uart.tx_r != uart.tx_w ) {
            uart.tx_r = (uart.tx_r + 1) & TX_BUFFER_MASK;
            uart.tx_data = uart.tx_buffer[uart.tx_r];
            uart.state = startbit;
          } else {
            uart.state = idle;
          }
         break;
   }

   //RC pulse/space time measurement
   if ( (++rc_pulse.timer) == 0 ) {
     rc_pulse.timer_overflow = 1;
   }
}

ISR(PCINT0_vect, ISR_NOBLOCK)
//ISR(PCINT0_vect)
{
  	
  DisableIRInterrupt();
  rc_pulse.high = IR_high();
  if (rc_pulse.timer_overflow) {
     rc_pulse.length = 0xffff;
  } else {
    rc_pulse.length = rc_pulse.timer;
  }
  
  rc_pulse.timer = 0;
  rc_pulse.timer_overflow = 0;
  
  EnableIRInterrupt();

}

uint8_t prev_pulse;

void send_pulse_data(void)
{
   DisableIRInterrupt();
   if (rc_pulse.length) {
      //The Pulse MSB bit is one
      //The Space MSB bit is zero
      rc_pulse.length = MIN(0x7fff, rc_pulse.length);
      if (rc_pulse.length == 0) {
        return; 
      }

      if (!rc_pulse.high) {
         rc_pulse.length |= 0x8000;
      }
      
      uart_tx_word(rc_pulse.length);
      rc_pulse.length = 0;  //We are prepared for new measurement
   } 
   EnableIRInterrupt();
}

int main(void)
{
   cli();
   hardware_init();
   uart_init();

   sei();

   while (1) {
      send_pulse_data();
   };

}
