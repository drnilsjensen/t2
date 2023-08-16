#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "tools.h"

// unused
char *pattern = "bcdfghjlmnrst";
cr_t idx[NUM];

int main(int argc, char *argv[]) {
  unsigned long int n = 0;
  int b = 0;
  char name[81] = "";
  char twin[81] = "";
  if (argc <= 1) return 0;
  strcpy(name, argv[1]);
  for (int i = 0; i < strlen(name); ++i) {
    char c = name[i];
    int t = 0;
    if (c == '-') t = 0;
    if (isdigit(c)) t = c - '0' + 1;
    if (islower(c)) t = c - 'a' + 1 + 10;
    n = t + n * 37;
  }
  b = (int)n;
  if (b < 0) b += INT_MAX;
  printf("%s: %d %lu %lx\n", name, b, n, n);
  vary(twin, b, 37);
  printf("int overlaps with: %s\n", (!strcmp(twin, name))? "nothing" : twin);
  return 0;
}
