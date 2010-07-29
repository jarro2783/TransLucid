int main() {
  static const int BUFFER_SIZE = 10;
  static char buf[BUFFER_SIZE];
  buf[0] = 'a';
  return buf[2];
}
