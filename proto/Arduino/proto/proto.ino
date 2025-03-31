// Arduino support for Nixie tube PROTOTYPE (one tube)
// 
// Wiring of pins on the LS244 buffer chip in the prototype:
// The LS244 is the 3.3v => 5v converter
//
// D8/MOSI/COPI => LS244-2  => LS244-18 => J1-1 => SER_IN (6595 pin 3)
// D9/SCK       => LS244-4  => LS244-16 => J1-2 => SRCK (6595 pin 13)
// D11/SDA      => LS244-6  => LS244-14 => J1-3 => SRCLR# (6596 pin 8)
// D7           => LS244-8  => LS244-12 => J1-4 => RCLK (6595 pin 12)
// D12/SCL      => LS244-11 => LS244-9  => J1-5 => GATE# (6595 pin 9)
//
// Mapping of Arduino logic pins to functions:
//
// D7 low => high clocks shift register content into output register
// D8 is the serial bit input to the shift register
// D9 low => high clocks the bit on D8 into the shift register
// D11 low clear holds the shift register clear
// D12 high holds the outputs disabled (nonconducting)
//
// All the bit-banging functions assume that pins D7, 8, 9, 11, and 12
// have been initialized as outputs. There are no inputs.

#include <SPI.h>

#define RCLK      7
#define COPI      8
#define SRCK      9
#define SRCLR_LOW 11
#define OUTEN_LOW 12

void enableOutput() {
  digitalWrite(OUTEN_LOW, 0);
}

void disableOutput() {
  digitalWrite(OUTEN_LOW, 1);
}

void clearSR() {
  digitalWrite(SRCLR_LOW, 0);
  //delayMicroseconds(5);
  digitalWrite(SRCLR_LOW, 1);
}

void clockSR() {
  digitalWrite(SRCK, 0);
  //delayMicroseconds(5);
  digitalWrite(SRCK, 1);
}

void clockRegister() {
  digitalWrite(RCLK, 0);
  //delayMicroseconds(5);
  digitalWrite(RCLK, 1);
}

void clockBit(int state) {
  digitalWrite(COPI, state);
  clockSR();
}

void setDefaults() {
  disableOutput();
  digitalWrite(SRCK, 1);
  digitalWrite(RCLK, 1);
  clearSR();
}

void printMillis(void) {
  long t = millis();
  printMillis(t);
}

void printMillis(long t) {
  char buffer[64];
  (void) sprintf(buffer, "Running %ld", t);
  Serial.println(buffer);
}

// BUILD_PURPOSE == 1 => slowly toggle all controls (1 / 5 seconds)
// BUILD_PURPOSE == 2 => empty setup and loop (check initialization)
// BUILD_PURPOSE == 3 => clock out a single digit and then wait
// BUILD_PURPOSE == 4 => SPI library (currently does not work)

#define BUILD_PURPOSE 4

#if BUILD_PURPOSE == 1

static int LEDstate;
void setup() {
  long t = millis();
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  printMillis(t);

}

// This version of loop() allows checking that all 5 control signals
// go to a solid low and a solid high using just a digital meter.
void loop() {
  printMillis();
  LEDstate = 1 - LEDstate;
  digitalWrite(LED_BUILTIN, LEDstate);
  digitalWrite(7, LEDstate);
  digitalWrite(8, LEDstate);
  digitalWrite(9, LEDstate);
  digitalWrite(11, LEDstate);
  digitalWrite(12, LEDstate);
  delay(5000);
}

#elif BUILD_PURPOSE == 2

// Intentionally empty setup and loop
void setup() {
}

void loop() {
}

#elif BUILD_PURPOSE == 3

// Display a digit

void setup() {
  pinMode(RCLK, OUTPUT);
  pinMode(COPI, OUTPUT);
  pinMode(SRCK, OUTPUT);
  pinMode(SRCLR_LOW, OUTPUT);
  pinMode(OUTEN_LOW, OUTPUT);

  setDefaults();
}

static int nextBit = 0;
static int bits[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
static int num_bits = sizeof(bits) / sizeof(int);

// Everything is MSB first. The argument is a byte stored in an int.
void byteOut(int b) {
  clearSR();
  clockRegister();
  for (int i = 0; i < 8; ++i) {
    int state = ((b << i) & 0x80) ? HIGH : LOW;
    clockBit(state);
  }
  clockRegister();
}

void loop() {
  delay(1000);
  enableOutput();

  if (nextBit >= num_bits) {
    nextBit = 0;
  }
  byteOut(bits[nextBit]);
  nextBit++;
}

#elif BUILD_PURPOSE == 4 // SPI library

#define MILLION (1000*1000)

static int nextBit = 0;
static uint16_t bits[] = { 0x0101, 0x0202, 0x0404, 0x0808, 0x1010, 0x2020, 0x4040, 0x8080 };
static int num_bits = sizeof(bits) / sizeof(int);

void setup() {
  pinMode(RCLK, OUTPUT);
  pinMode(COPI, OUTPUT);
  pinMode(SRCK, OUTPUT);
  pinMode(SRCLR_LOW, OUTPUT);
  pinMode(OUTEN_LOW, OUTPUT);

  clearSR();
  clockRegister();

  SPI.begin();
}

void loop() {
  delay(1000);
  enableOutput();

  //printMillis();
  SPI.beginTransaction(SPISettings(1000, MSBFIRST, SPI_MODE0));
  SPI.transfer(bits[nextBit]);
  SPI.endTransaction();

  clockRegister();
  nextBit = (nextBit + 1) % num_bits;
}

#else
#error BUILD_PURPOSE is not set
#endif


