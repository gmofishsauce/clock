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
