#include <Arduino.h>
#line 1 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
#line 1 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
#include <Esplora.h>

#line 3 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
int16_t test_fuc2(uint16_t * LOL, uint16_t hi);
#line 11 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
void setup(void);
#line 16 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
void loop(void);
#line 24 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
int16_t my_func(int16_t y);
#line 3 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
int16_t test_fuc2(uint16_t * LOL, uint16_t hi) {
  return 2;
}

void setup(void) {
  pinMode(13, OUTPUT);
}
int16_t count = 0;

void loop(void) {
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
  Serial.println("hi!!!");
}

int16_t my_func(int16_t y) {
  return 2*y;
}


