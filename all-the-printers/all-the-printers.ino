#include "ManySoftSerial.h"

ManySoftSerial printers = ManySoftSerial(8, 0b00111111);

void setup() {
  printers.begin(19200);

  //             0     1  0     0     1  0     1  1
  uint8_t d[] = {
    0b00000000,
    0b00000111,
    0b00000110,
    0b00000000,
    0b00000111,
    0b00000100,
    0b00000101,
    0b00000010};
//  printers.write8(k);
  // 00001010
//  uint8_t nl[] = {0, 0, 0, 0, 0xFF, 0, 0xFF, 0};
//  printers.write8(nl);
//  printers.write8(nl);
//  printers.write8(nl);

               // 1 1 0 1 0 0 1 1
               // 1 1 0 0 1 0 1 1
  for (int i = 0; i < 2; i++) {
    printers.write8(d);
    printers.write8(d);
    printers.write8(d);
    printers.write8(d);
    printers.write8(d);
    printers.write8(d);
    printers.write8(d);
    printers.write8(d);
    printers.write8(d);
    printers.write8(d);
    printers.write8(d);
    printers.write8(d);
    printers.write8(d);
  }

  printers.println("\n");
}

void loop() {
}

