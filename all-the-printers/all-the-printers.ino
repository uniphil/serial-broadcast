#include "ManySoftSerial.h"

ManySoftSerial printers = ManySoftSerial(8, 9);

void setup() {
  printers.begin(9600);
  printers.println("hello");
}

void loop() {
}

