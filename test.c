uint16_t do_math(int16_t x, uint32_t lolol) {
  x += 5;
}

int16_t main(void) {
  uint32_t result = -1, val = 4;
  uint16_t lol = -1, newval = 4;
  int16_t x;
  do_math(val, result);
  return result;
}
