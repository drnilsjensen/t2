#define BIG 13*13*13*13*13*500

int test[BIG];

void main() {
  int someid = 4712;
  for (int i=0; i < BIG;++i) {
    if (test[i] == someid) {
      test[i] = 0;
    }
  }
}
