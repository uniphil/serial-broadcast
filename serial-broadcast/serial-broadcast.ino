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
#define CHUNK_BYTES 8

ManySoftSerial b_printers = ManySoftSerial(8, 0b00111111);

uint8_t state = WAITING;

uint8_t b_buff[CHUNK_BYTES * 8];
uint8_t b_idx = 0;


void setup() {
  Serial.begin(500000);
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
    return;

  case 2:  // write8
    Serial.write(2);
    uint8_t n = Serial.readBytes(b_buff, CHUNK_BYTES * 8);
    if (n != CHUNK_BYTES * 8) {
      Serial.write(0b10000000 | n);
      return;
    }
    for (int o = 0; o < CHUNK_BYTES; o++) {
      b_printers.write8((uint8_t*)(b_buff + o * 8));
    }
    Serial.write(3);
    return;

  default:
    Serial.write(0b11111000 | command);
    return;
  }
}

void im_row(Stream & s) {
  s.write(0x12);  // DC2
  s.write('*');   // image
  s.write(1);     // height
  s.write(ROW_BYTES);  // width
}

