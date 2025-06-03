#define COPI      8
#define SRCK      9
#define RCLK      A1
#define SRCLR     7

void clearSR() {
  Serial.println("SHIFT REGISTER CLEARED");
  digitalWrite(SRCLR, 0);
  digitalWrite(SRCLR, 1);
}

void clockSR() {
  digitalWrite(SRCK, 0);
  digitalWrite(SRCK, 1);
}

void clockRegister() {
  digitalWrite(RCLK, 0);
  digitalWrite(RCLK, 1);
}

void clockBit(int state) {
  digitalWrite(COPI, state);
  clockSR();
}

void setDefaults() {
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
  (void) sprintf(buffer, "running %ld", t);
  Serial.println(buffer);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("start: ");
  printMillis();

  pinMode(RCLK, OUTPUT);
  pinMode(COPI, OUTPUT);
  pinMode(SRCK, OUTPUT);
  pinMode(SRCLR, OUTPUT);
  setDefaults();
}

/*
Overall note:

The labeling on the KiCad schematic is completely screwed up. The problem
probably originated when I found that mounting the display board's 96-pin
connector to the back of the display board improved the component spacing.


On the logic board:

There are 8 buses, each 8 bits, for a total of 64 control bits.
The control bits have to be shifted out serially, MSB of each bus first.
The order of the 8-bit buses is B4, B3, B2, B1; B8, B7, B6, B5.

There are 96 connector pins numbered A1-A32, B1-B32, and C1-C32.
The entire "B" row carries power and ground, leaving 64 signal pins.
Do not confuse the display board "B" buses with the connector "B" row.
Connector pin A1 is at B4-0, A2 is at B4-1, etc; A32 is at B1-7.
Connector pin C1 is at B5-0, C2 is at B5-1, etc; C32 is at B8-7.

On the display board:

Each row ("A" or "C") in the connector runs to three of the six tubes
The "A" row runs to the minutes' ones and the seconds tubes.
The "C" row runs to the hours tubes and the minutes' tens


The shunts (tube darkener bits) are A1, A2, C31, and C32.
A1 (B5-0) darkens the hours' ones tube
A2 (B5-1) darkens the hours' tens tube (usually dark in 12 hour mode)
C31 (B1-6) darkens both minutes tubes
C32 (B1-7) darkens both seconds tubes
There is no way to individually darken minutes or seconds tubes.
*/
byte buf[8];

#define B1  3
#define B2  2
#define B3  1
#define B4  0
#define B8  4
#define B7  5
#define B6  6
#define B5  7

#define BIT(n) (1<<n)

// Everything is MSB first. The argument is a byte stored in an int.
void byteOut(int b) {
  for (int i = 0; i < 8; ++i) {
    int state = ((b << i) & 0x80) ? HIGH : LOW;
    clockBit(state);
  }
}

void bufOut(byte *pb) {
  for (int i = 0; i < 8; ++i, ++pb) {
    byteOut(*pb);
  }
  clockRegister();
}

void loop() {
  for(;;) {
    printMillis();

    for(int i = 0; i < 8; ++i) { buf[i] = 0x00; }
    bufOut(buf);
    Serial.println("All bits off, including darkener bits");
    delay(8000);

    // FACT: B1-7 turns on the "9" filament in the seconds' units.
    // B1-6 turns on the "8" filament.
    for(int i = 0; i < 8; ++i) { buf[i] = 0x00; }
    buf[B1] = BIT(6);
    bufOut(buf);
    Serial.println("One filament lit in seconds tube");
    delay(8000);

    for(int i = 0; i < 8; ++i) { buf[i] = 0x00; }
    buf[B8] = 0x81;
    bufOut(buf);
    Serial.println("Seconds darkener enabled");
    delay(8000);
  }
}
