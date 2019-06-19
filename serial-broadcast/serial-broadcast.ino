#include "ManySoftSerial.h"

#define ROW_BYTES 48

// states
#define WAITING 0

// messages
#define READY 0
#define BEGIN_ROW 1
#define PRINT_ROW 2

// asdf
#define CMD_PATTERN 0b11011000

ManySoftSerial b_printers = ManySoftSerial(8, 0b00111111);

uint8_t state = WAITING;

uint8_t b_buff[8];
uint8_t b_idx = 0;


void setup() {
  Serial.begin(115200);
  b_printers.begin(19200);
  b_printers.setTimeout(500);
  delay(100);
}

void loop() {
  while (!Serial.available());
  byte command = Serial.read();
  if ((command & 0b11111000) != CMD_PATTERN) {
    Serial.write(command);
    return;
  }

  switch (command & 0b0000111) {
  case 1:  // start row
    im_row(b_printers);
    Serial.write(1);
    break;

  case 2:  // write8
    Serial.write(2);
    uint8_t n = Serial.readBytes(b_buff, 8);
    if (n != 8) {
      Serial.write(0b10000000 | n);
    }
    b_printers.write8(b_buff);
    Serial.write(3);
    break;

  default:
    Serial.write(0b11111000 | command);
    break;
  }
//
//  switch (state) {
//
//  case WAITING:
//    if (!Serial.available()) return;
////    b_printers.println("go go row");
//    state = BEGIN_ROW;
//    return;
//
//  case BEGIN_ROW:
//    im_row(b_printers);
//    state = PRINT_ROW;
//    return;
//
//  case PRINT_ROW:
//    for (size_t i = 0; i < ROW_BYTES; i++) {
//      for (size_t bi = 0; bi < 8; bi++) {
//        while (Serial.available() < 8);
//        Serial.readBytes(b_buff, 8);
//        Serial.write(b_buff, 8);
////        b_buff[bi] = Serial.read();
//      }
//      b_printers.write8(b_buff);
//    }
//    state = WAITING;
//    return;
//
//  }
}

void im_row(Stream & s) {
  s.write(0x12);  // DC2
  s.write('*');   // image
  s.write(1);     // height
  s.write(ROW_BYTES);  // width
}

