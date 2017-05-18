#include <Arduino.h>
#line 1 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
#line 1 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
#include <Esplora.h>

#line 3 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
int test_fuc2(const int LOL, const int hi);
#line 11 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
void setup(void);
#line 16 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
void loop(void);
#line 24 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"
int my_func(int y);
#line 3 "/home/owen/Arduino/sketch_may09a/sketch_may09a.ino"


template<typename T, typename S>
class bar {
};

class blah {
  operator int() {
    return 0;
  }
};

bar<double, double> b;

int arr[1];



class foo {
  foo() : a(m3), m5(&m0) {}
  const int m0 = 0;
  mutable int m1;
  volatile int m2;
  int m3;
  static int m4;
  int * m6;
  int &a;
  const int * const m5;
  int arr[10];

  int baz(int a) {
    return 0;
  }

  int bar(int a) const {
    return 0;
  }
};

const volatile int mmm3 = 0;

volatile int * xxxx;
int test_fuc2(const int *LOL, const int hi) {
  return 2;
}

void setup(void) {
  // pinMode(13, OUTPUT);
}
int count = 0;

void loop(void) {
  auto asda = 1;
  static unsigned int yx;
  // digitalWrite(13, HIGH);
  // delay(1000);
  // digitalWrite(13, LOW);
  // delay(1000);
  // // Serial.println("hi!!!");

  unsigned int a = static_cast<int>('a');
  int a2 = (int)('a');
}

int my_func(int y) {
  return 2*y;
}


int main() {
  return EXIT_SUCCESS;
}

