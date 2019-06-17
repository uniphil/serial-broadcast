#include "ManySoftSerial.h"

ManySoftSerial printers = ManySoftSerial(9, 8);

void setup() {
  printers.begin(19200);
  printers.println("oh hi\n\n\n");
}

void loop() {
}

