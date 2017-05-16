
void setup(void);

int16_t main(void) {
  int32_t result = -1, val = 4;
  int16_t lol = -1, newval = 4;
  int16_t x;
  do_math(val, result);
  return result;
}


int16_t do_math(int16_t x, int32_t lolol) {
  x += 2;
}