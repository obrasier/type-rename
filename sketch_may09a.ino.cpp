#include <Arduino.h>
#line 1 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
#line 1 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
#include <Esplora.h>

#line 3 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
int test_fuc2(unsigned int * LOL, unsigned int hi);
#line 11 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
void setup(void);
#line 16 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
void loop(void);
#line 24 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
int my_func(int y);
#line 3 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
int test_fuc2(unsigned

int * LOL, unsigned int

hi) {
  return 2;
}

void setup(void) {
  pinMode(13, OUTPUT);
}
int count = 0;

void loop(void) {
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
  Serial.println("hi!!!");
}

int my_func(int y) {
  return 2*y;
}


