#include "ManySoftSerial.h"

ManySoftSerial printers = ManySoftSerial(8, 0b00111111);

void setup() {
  printers.begin(19200);

  uint8_t d[] = {
    0b00000000,
    0b00000111,
    0b00000110,
    0b00000000,
    0b00000111,
    0b00000100,
    0b00000101,
    0b00000010};

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

