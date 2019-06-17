/*
ManySoftSerial.cpp [fomerly SoftwareSerial.cpp (formerly NewSoftSerial.cpp)] - 
Multi-instance software serial library for Arduino/Wiring
-- Interrupt-driven receive and other improvements by ladyada
   (http://ladyada.net)
-- Tuning, circular buffer, derivation from class Print/Stream,
   multi-instance support, porting to 8MHz processors,
   various optimizations, PROGMEM delay tables, inverse logic and 
   direct port writing by Mikal Hart (http://www.arduiniana.org)
-- Pin change interrupt macros by Paul Stoffregen (http://www.pjrc.com)
-- 20MHz processor support by Garrett Mace (http://www.macetech.com)
-- ATmega1280/2560 support by Brett Hagman (http://www.roguerobotics.com/)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

The latest version of this library can always be found at
http://arduiniana.org.
*/

// 
// Includes
// 
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
#include "ManySoftSerial.h"
#include <util/delay_basic.h>

//
// Private methods
//

/* static */ 
inline void ManySoftSerial::tunedDelay(uint16_t delay) { 
  _delay_loop_2(delay);
}

//
// Constructor
//
ManySoftSerial::ManySoftSerial(uint8_t pin0, uint8_t mask) :
  _tx_delay(0),
  _buffer_overflow(false)
{
  setTX(pin0, mask);
}

//
// Destructor
//
ManySoftSerial::~ManySoftSerial()
{
  end();
}

void ManySoftSerial::setTX(uint8_t pin0, uint8_t mask)
{
  // First write, then set output. If we do this the other way around,
  // the pin would be output low for a short while before switching to
  // output high. Now, it is input with pullup for a short while, which
  // is fine.
  _transmitBitMask = mask;

  uint8_t port = digitalPinToPort(pin0);

  _transmitPortRegister = portOutputRegister(port);
  *_transmitPortRegister |= _transmitBitMask;

  *portModeRegister(port) |= _transmitBitMask;
}

uint16_t ManySoftSerial::subtract_cap(uint16_t num, uint16_t sub) {
  if (num > sub)
    return num - sub;
  else
    return 1;
}

//
// Public methods
//

void ManySoftSerial::begin(long speed)
{

  // Precalculate the various delays, in number of 4-cycle delays
  uint16_t bit_delay = (F_CPU / speed) / 4;

  // 12 (gcc 4.8.2) or 13 (gcc 4.3.2) cycles from start bit to first bit,
  // 15 (gcc 4.8.2) or 16 (gcc 4.3.2) cycles between bits,
  // 12 (gcc 4.8.2) or 14 (gcc 4.3.2) cycles from last bit to stop bit
  // These are all close enough to just use 15 cycles, since the inter-bit
  // timings are the most critical (deviations stack 8 times)
  _tx_delay = subtract_cap(bit_delay, 15 / 4);
}

void ManySoftSerial::write8(uint8_t dat[8])
{  
  if (_tx_delay == 0) {
    setWriteError();
    return 0;
  }

  // By declaring these as local variables, the compiler will put them
  // in registers _before_ disabling interrupts and entering the
  // critical timing sections below, which makes it a lot easier to
  // verify the cycle timings
  volatile uint8_t *reg = _transmitPortRegister;
  uint8_t reg_mask = _transmitBitMask;
  uint8_t inv_mask = ~_transmitBitMask;
  uint8_t oldSREG = SREG;
  uint16_t delay = _tx_delay;

  cli();  // turn off interrupts for a clean txmit

  // Write the start bit
  *reg &= inv_mask;

  tunedDelay(delay);

  for (uint8_t i = 0; i < 8; i++) {
    uint8_t b = dat[7 - i];
    *reg |= b & reg_mask;
    *reg &= b | inv_mask;
    tunedDelay(delay);
  }

  // restore pin to natural state
  *reg |= reg_mask;

  SREG = oldSREG; // turn interrupts back on
  tunedDelay(_tx_delay);
  
  return 1;
}

size_t ManySoftSerial::write(uint8_t b)
{
  if (_tx_delay == 0) {
    setWriteError();
    return 0;
  }

  // By declaring these as local variables, the compiler will put them
  // in registers _before_ disabling interrupts and entering the
  // critical timing sections below, which makes it a lot easier to
  // verify the cycle timings
  volatile uint8_t *reg = _transmitPortRegister;
  uint8_t reg_mask = _transmitBitMask;
  uint8_t inv_mask = ~_transmitBitMask;
  uint8_t oldSREG = SREG;
  uint16_t delay = _tx_delay;

  cli();  // turn off interrupts for a clean txmit

  // Write the start bit
  *reg &= inv_mask;

  tunedDelay(delay);

  // Write each of the 8 bits
  for (uint8_t i = 8; i > 0; --i)
  {
    if (b & 1) // choose bit
      *reg |= reg_mask; // send 1
    else
      *reg &= inv_mask; // send 0

    tunedDelay(delay);
    b >>= 1;
  }

  // restore pin to natural state
  *reg |= reg_mask;

  SREG = oldSREG; // turn interrupts back on
  tunedDelay(_tx_delay);
  
  return 1;
}

