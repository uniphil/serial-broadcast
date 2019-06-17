#include "ManySoftSerial.h"

ManySoftSerial printers = ManySoftSerial(8);

void setup() {
  printers.begin(19200);

  //             0     1  0     0     1  0     1  1
  //             0     1  1     0     1  0     0  0
//  uint8_t k[] = {0xFF, 0, 0xFF, 0xFF, 0, 0xFF, 0, 0};
//  printers.write8(k);
  // 00001010
//  uint8_t nl[] = {0, 0, 0, 0, 0xFF, 0, 0xFF, 0};
//  printers.write8(nl);
//  printers.write8(nl);
//  printers.write8(nl);

  printers.println("\nhello\n\n\n");
}

void loop() {
}

